/*
 * Copyright (C) Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <cstdio>
#include "anim.hpp"
#include "ascii_to_geom.hpp"
#include "game_consts.hpp"
#include "our_shader.hpp"
#include "play_scene.hpp"
#include "util.hpp"
#include "welcome_scene.hpp"
#include "welcome_scene.hpp"

#include "data/ascii_art.inl"
#include "data/cube_geom.inl"
#include "data/strings.inl"
#include "data/tunnel_geom.inl"

#define WALL_TEXTURE_SIZE 64

// colors for menus
static const float MENUITEM_SEL_COLOR[] = { 1.0f, 1.0f, 0.0f };
static const float MENUITEM_COLOR[] = { 1.0f, 1.0f, 1.0f };

// obstacle colors
static const float OBS_COLORS[] = {
        0.0f, 0.0f, 0.0f, // style 0 (not used)
        0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f
};

static const char* TONE_BONUS[] = {
    "d70 f150. f250. f350. f450.",
    "d70 f200. f300. f400. f500.",
    "d70 f250. f350. f450. f550.",
    "d70 f300. f400. f500. f600.",
    "d70 f350. f450. f550. f650.",
    "d70 f400. f500. f600. f700.",
    "d70 f450. f550. f650. f750.",
    "d70 f500. f600. f700. f800.",
    "d70 f550. f650. f750. f850."
};

PlayScene::PlayScene() : Scene() {
    mOurShader = NULL;
    mTrivialShader = NULL;
    mTextRenderer = NULL;
    mShapeRenderer = NULL;
    mShipSteerX = mShipSteerZ = 0.0f;
    mFilteredSteerX = mFilteredSteerZ = 0.0f;

    mPlayerDir = glm::vec3(0.0f, 1.0f, 0.0f); // forward
    mDifficulty = 0;
    mUseCloudSave = false;

    mCubeGeom = NULL;
    mTunnelGeom = NULL;

    mObstacleCount = 0;
    mFirstObstacle = 0;
    mFirstSection = 0;
    mSteering = STEERING_NONE;
    mPointerId = -1;
    mPointerAnchorX = mPointerAnchorY = 0.0f;

    mWallTexture = NULL;

    memset(mMenuItemText, 0, sizeof(mMenuItemText));
    mMenuItemText[MENUITEM_UNPAUSE] = S_UNPAUSE;
    mMenuItemText[MENUITEM_QUIT] = S_QUIT;
    mMenuItemText[MENUITEM_START_OVER] = S_START_OVER;
    mMenuItemText[MENUITEM_RESUME] = S_RESUME;

    memset(mMenuItems, 0, sizeof(mMenuItems));
    mMenuItemCount = 0;

    mMenu = MENU_NONE;
    mMenuSel = 0;

    mSignText = NULL;
    mSignTimeLeft = 0.0f;

    mShowedHowto = false;
    mLifeGeom = NULL;

    mLives = PLAYER_LIVES;

    mRollAngle = 0.0f;

    mPlayerSpeed = 0.0f;
    mBlinkingHeart = false;
    mGameStartTime = Clock();

    mBonusInARow = 0;
    mLastCrashSection = -1;

    mFrameClock.SetMaxDelta(MAX_DELTA_T);
    mLastAmbientBeepEmitted = 0;
    mMenuTouchActive = false;

    mCheckpointSignPending = false;

    SetScore(0);

    /*
     * where do I put the program???
     */
    const char *savePath = "/mnt/sdcard/com.google.example.games.tunnel.fix";
    int len = strlen(savePath) + strlen(SAVE_FILE_NAME) + 3;
    mSaveFileName = new char[len];
    strcpy(mSaveFileName, savePath);
    strcat(mSaveFileName, "/");
    strcat(mSaveFileName, SAVE_FILE_NAME);
    LOGD("Save file name: %s", mSaveFileName);
    LoadProgress();

    if (mSavedCheckpoint) {
        // start with the menu that asks whether or not to start from the saved level
        // or start over from scratch
        ShowMenu(MENU_LEVEL);
    }
}

void PlayScene::LoadProgress() {
    // try to load save file
    mSavedCheckpoint = 0;

    LOGD("Attempting to load: %s", mSaveFileName);
    FILE *f = fopen(mSaveFileName, "r");
    bool hasLocalFile = false;
    if (f) {
        hasLocalFile = true;
        LOGD("File found. Loading data.");
        if (1 != fscanf(f, "v1 %d", &mSavedCheckpoint)) {
            LOGE("Error parsing save file.");
            mSavedCheckpoint = 0;
        } else {
            LOGD("Loaded. Level = %d", mSavedCheckpoint);
            mSavedCheckpoint = (mSavedCheckpoint / LEVELS_PER_CHECKPOINT) * LEVELS_PER_CHECKPOINT;
            LOGD("Normalized check-point: level %d", mSavedCheckpoint);
        }
        fclose(f);
    } else {
        LOGD("Save file not present.");
    }

    // check cloud save.
    LOGD("Checking cloud save data.");
    if (true) {
        LOGD("No cloud save available because we are not signed in.");
        mUseCloudSave = false;
    }

    if (mUseCloudSave && hasLocalFile) {
        // since we're using cloud save, we can delete the local progress file
        LOGD("Since we're using cloud save, deleting local progress file %s", mSaveFileName);
        if (0 != remove(mSaveFileName)) {
            LOGW("WARNING: failed to remove local progress file.");
        }
    }

    LOGD("Final decision on starting level: %d", mSavedCheckpoint);
    LOGD("Final decision on whether to use cloud: %s", mUseCloudSave ? "USE CLOUD" :
            "DO NOT USE CLOUD (failed)");
}

void PlayScene::WriteSaveFile(int level) {
    LOGD("Saving progress (level %d) to file: %s", level, mSaveFileName);
    FILE *f = fopen(mSaveFileName, "w");
    if (!f) {
        LOGE("Error writing to save game file.");
        return;
    }
    fprintf(f, "v1 %d", level);
    fclose(f);
    LOGD("Save file written.");
}

void PlayScene::SaveProgress() {
    if (mDifficulty <= mSavedCheckpoint) {
        // nothing to do
        LOGD("No need to save level, current = %d, saved = %d", mDifficulty, mSavedCheckpoint);
        return;
    } else if (!IsCheckpointLevel()) {
        LOGD("Current level %d is not a checkpoint level. Nothing to save.", mDifficulty);
        return;
    }

    mSavedCheckpoint = mDifficulty;

    // Save state locally or to the cloud, depending on configuration:
    if (mUseCloudSave) {
        LOGD("Saving progress to the cloud: level %d", mDifficulty);
        /*
         * No where to save
         */
    } else {
        LOGD("Saving progress to LOCAL FILE: level %d", mDifficulty);
        WriteSaveFile(mDifficulty);
    }

    // Show a "checkpoint saved" sign when possible. We don't show it right away
    // because will already be showing the "Level N" sign, so we just set this flag
    // to remind us to show it right after.
    mCheckpointSignPending = true;
}

static unsigned char* _gen_wall_texture() {
    static unsigned char pixel_data[WALL_TEXTURE_SIZE * WALL_TEXTURE_SIZE * 3];
    unsigned char *p;
    int x, y;
    for (y = 0, p = pixel_data; y < WALL_TEXTURE_SIZE; y++) {
        for (x = 0; x < WALL_TEXTURE_SIZE; x++, p += 3) {
            p[0] = p[1] = p[2] = 128 + ((x > 2 && y > 2) ? Random(128) : 0);
        }
    }
    return pixel_data;
}

void PlayScene::OnStartGraphics() {
    // build shaders
    mOurShader = new OurShader();
    mOurShader->Compile();
    mTrivialShader = new TrivialShader();
    mTrivialShader->Compile();

    // build projection matrix
    UpdateProjectionMatrix();

    // build tunnel geometry
    mTunnelGeom = new SimpleGeom(
            new VertexBuf(TUNNEL_GEOM, sizeof(TUNNEL_GEOM),TUNNEL_GEOM_STRIDE),
            new IndexBuf(TUNNEL_GEOM_INDICES, sizeof(TUNNEL_GEOM_INDICES)));
    mTunnelGeom->vbuf->SetColorsOffset(TUNNEL_GEOM_COLOR_OFFSET);
    mTunnelGeom->vbuf->SetTexCoordsOffset(TUNNEL_GEOM_TEXCOORD_OFFSET);

    // build cube geometry (to draw obstacles)
    mCubeGeom = new SimpleGeom(new VertexBuf(CUBE_GEOM, sizeof(CUBE_GEOM),CUBE_GEOM_STRIDE));
    mCubeGeom->vbuf->SetColorsOffset(CUBE_GEOM_COLOR_OFFSET);
    mCubeGeom->vbuf->SetTexCoordsOffset(CUBE_GEOM_TEXCOORD_OFFSET);

    // make the wall texture
    mWallTexture = new Texture();
    mWallTexture->InitFromRawRGB(WALL_TEXTURE_SIZE, WALL_TEXTURE_SIZE, false,
            _gen_wall_texture());

    // reset frame clock so the animation doesn't jump
    mFrameClock.Reset();

    // life icon geometry
    mLifeGeom = AsciiArtToGeom(ART_LIFE, LIFE_ICON_SCALE);

    // create text renderer and shape renderer
    mTextRenderer = new TextRenderer(mTrivialShader);
    mShapeRenderer = new ShapeRenderer(mTrivialShader);
}

void PlayScene::OnKillGraphics() {
    CleanUp(&mTextRenderer);
    CleanUp(&mShapeRenderer);
    CleanUp(&mOurShader);
    CleanUp(&mTrivialShader);
    CleanUp(&mTunnelGeom);
    CleanUp(&mCubeGeom);
    CleanUp(&mWallTexture);
    CleanUp(&mLifeGeom);
}

void PlayScene::DoFrame() {
    float deltaT = mFrameClock.ReadDelta();
    float previousY = mPlayerPos.y;

    // clear screen
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // rotate the view matrix according to current roll angle
    glm::vec3 upVec = glm::vec3(-sin(mRollAngle), 0, cos(mRollAngle));

    // set up view matrix according to player's ship position and direction
    mViewMat = glm::lookAt(mPlayerPos, mPlayerPos + mPlayerDir, upVec);

    // render tunnel walls
    RenderTunnel();

    // render obstacles
    RenderObstacles();

    if (mMenu) {
        RenderMenu();
        // nothing more to do
        return;
    }

    // render HUD (lives, score, etc)
    RenderHUD();

    // deduct from the time remaining to remove a sign from the screen
    if (mSignText && mSignExpires) {
        mSignTimeLeft -= deltaT;
        if (mSignTimeLeft < 0.0f) {
            mSignText = NULL;
        }
    }

    // if a "saved checkpoint" sign pending? Can we show it right now?
    if (!mSignText && mCheckpointSignPending) {
        mCheckpointSignPending = false;
        ShowSign(S_CHECKPOINT_SAVED, SIGN_DURATION);
    }

    // did we already show the howto?
    if (!mShowedHowto && mDifficulty == 0) {
        mShowedHowto = true;
        ShowSign(S_HOWTO_WITHOUT_JOY, SIGN_DURATION);
    }

    // deduct from the time remaining on the blinking heart animation
    if (mBlinkingHeart && Clock() > mBlinkingHeartExpire) {
        mBlinkingHeart = false;
    }

    // update speed
    float targetSpeed = PLAYER_SPEED + PLAYER_SPEED_INC_PER_LEVEL * mDifficulty;
    float accel = mPlayerSpeed >= 0.0f ? PLAYER_ACCELERATION_POSITIVE_SPEED :
            PLAYER_ACCELERATION_NEGATIVE_SPEED;
    if (mLives <= 0) {
        targetSpeed = 0.0f;
    }
    mPlayerSpeed = Approach(mPlayerSpeed, targetSpeed, deltaT * accel);

    // apply noise filter on steering
    mFilteredSteerX = (mFilteredSteerX * (NOISE_FILTER_SAMPLES - 1) + mShipSteerX)
            / NOISE_FILTER_SAMPLES;
    mFilteredSteerZ = (mFilteredSteerZ * (NOISE_FILTER_SAMPLES - 1) + mShipSteerZ)
            / NOISE_FILTER_SAMPLES;

    // move player
    if (mLives > 0) {
        float steerX = mFilteredSteerX, steerZ = mFilteredSteerZ;
        if (mSteering == STEERING_TOUCH) {
            // touch steering
            mPlayerPos.x = Approach(mPlayerPos.x, steerX, PLAYER_MAX_LAT_SPEED * deltaT);
            mPlayerPos.z = Approach(mPlayerPos.z, steerZ, PLAYER_MAX_LAT_SPEED * deltaT);
        } else if (mSteering == STEERING_JOY) {
            // joystick steering
            mPlayerPos.x += deltaT * steerX;
            mPlayerPos.z += deltaT * steerZ;
        }
    }
    mPlayerPos.y += deltaT * mPlayerSpeed;

    // make sure player didn't leave tunnel
    mPlayerPos.x = Clamp(mPlayerPos.x, PLAYER_MIN_X, PLAYER_MAX_X);
    mPlayerPos.z = Clamp(mPlayerPos.z, PLAYER_MIN_Z, PLAYER_MAX_Z);

    // shift sections if needed
    ShiftIfNeeded();

    // generate more obstacles!
    GenObstacles();

    // detect collisions
    DetectCollisions(previousY);

    // update ship's roll speed according to level
    static float roll_speeds[] = ROLL_SPEEDS;
    int count = sizeof(roll_speeds) / sizeof(float);
    float speed = roll_speeds[mDifficulty % count];
    mRollAngle += deltaT * speed;
    while (mRollAngle < 0) {
        mRollAngle += 2 * M_PI;
    }
    while (mRollAngle > 2 * M_PI) {
        mRollAngle -= 2 * M_PI;
    }

    // did the game expire?
    if (mLives <= 0 && Clock() > mGameOverExpire) {
        SceneManager::GetInstance()->RequestNewScene(new WelcomeScene());

    }

    // produce the ambient sound
    int soundPoint = (int)floor(mPlayerPos.y / (TUNNEL_SECTION_LENGTH/3));
    if (soundPoint % 3 != 0 && soundPoint > mLastAmbientBeepEmitted) {
        mLastAmbientBeepEmitted = soundPoint;
        SfxMan::GetInstance()->PlayTone(soundPoint % 2 ? TONE_AMBIENT_0 : TONE_AMBIENT_1);
    }
}

static float GetSectionCenterY(int i) {
    return (float)i * TUNNEL_SECTION_LENGTH;
}

static float GetSectionEndY(int i) {
    return GetSectionCenterY(i) + 0.5f * TUNNEL_SECTION_LENGTH;
}

static void _get_obs_color(int style, float *r, float *g, float *b) {
    style = Clamp(style, 1, 6);
    *r = OBS_COLORS[style * 3];
    *g = OBS_COLORS[style * 3 + 1];
    *b = OBS_COLORS[style * 3 + 2];
}

void PlayScene::RenderTunnel() {
    glm::mat4 modelMat;
    glm::mat4 mvpMat;
    int i, oi;

    mOurShader->BeginRender(mTunnelGeom->vbuf);
    mOurShader->SetTexture(mWallTexture);
    for (i = mFirstSection, oi = 0; i <= mFirstSection + RENDER_TUNNEL_SECTION_COUNT; ++i, ++oi) {
        float segCenterY = GetSectionCenterY(i);
        modelMat = glm::translate(glm::mat4(1.0), glm::vec3(0.0, segCenterY, 0.0));
        mvpMat = mProjMat * mViewMat * modelMat;

        Obstacle *o = oi >= mObstacleCount ? NULL : GetObstacleAt(oi);

        // the point light is given in model coordinates, which is 0,0,0 is ok (center of
        // tunnel section)
        if (o) {
            float red, green, blue;
            _get_obs_color(o->style, &red, &green, &blue);
            mOurShader->EnablePointLight(glm::vec3(0.0, 0.0f, 0.0f), red, green, blue);
        } else {
            mOurShader->DisablePointLight();
        }

        // render tunnel section
        mOurShader->Render(mTunnelGeom->ibuf, &mvpMat);
    }
    mOurShader->EndRender();
}

void PlayScene::RenderObstacles() {
    int i;
    int r, c;
    float red, green, blue;
    glm::mat4 modelMat;
    glm::mat4 mvpMat;

    mOurShader->BeginRender(mCubeGeom->vbuf);
    mOurShader->SetTexture(mWallTexture);

    for (i = 0; i < mObstacleCount; i++) {
        Obstacle *o = GetObstacleAt(i);
        float posY = GetSectionCenterY(mFirstSection + i);

        if (o->style == Obstacle::STYLE_NULL) {
            // don't render null obstacles
            continue;
        }

        for (r = 0; r < OBS_GRID_SIZE; r++) {
            for (c = 0; c < OBS_GRID_SIZE; c++) {
                bool isBonus = r == o->bonusRow && c == o->bonusCol;
                if (o->grid[c][r]) {
                    // set up matrices
                    modelMat = glm::translate(glm::mat4(1.0f), o->GetBoxCenter(c, r, posY));
                    modelMat = glm::scale(modelMat, o->GetBoxSize(c, r));
                    mvpMat = mProjMat * mViewMat * modelMat;

                    // set up color
                    _get_obs_color(o->style, &red, &green, &blue);
                    mOurShader->SetTintColor(red, green, blue);

                    // render box
                    mOurShader->Render(&mvpMat);
                } else if (isBonus) {
                    modelMat = glm::translate(glm::mat4(1.0f), o->GetBoxCenter(c, r, posY));
                    modelMat = glm::scale(modelMat, glm::vec3(OBS_BONUS_SIZE, OBS_BONUS_SIZE,
                            OBS_BONUS_SIZE));
                    modelMat = glm::rotate(modelMat, Clock() * 90.0f, glm::vec3(0.0f, 0.0f, 1.0f));
                    mvpMat = mProjMat * mViewMat * modelMat;
                    mOurShader->SetTintColor(SineWave(0.8f, 1.0f, 0.5f, 0.0f),
                            SineWave(0.8f, 1.0f, 0.5f, 0.0f),
                            SineWave(0.8f, 1.0f, 0.5f, 0.0f)); // shimmering color
                    mOurShader->Render(&mvpMat); // render
                }
            }
        }
    }
    mOurShader->EndRender();
}

void PlayScene::GenObstacles() {
    while (mObstacleCount < MAX_OBS) {
        // generate a new obstacle
        int index = (mFirstObstacle + mObstacleCount) % MAX_OBS;

        int section = mFirstSection + mObstacleCount;
        if (section < OBS_START_SECTION) {
            // generate an empty obstacle
            mObstacleCircBuf[index].Reset();
            mObstacleCircBuf[index].style = Obstacle::STYLE_NULL;
        } else {
            // generate a normal obstacle
            mObstacleGen.Generate(&mObstacleCircBuf[index]);
        }
        mObstacleCount++;
    }
}

void PlayScene::ShiftIfNeeded() {
    // is it time to discard a section and shift forward?
    while (mPlayerPos.y > GetSectionEndY(mFirstSection) + SHIFT_THRESH) {
        // shift to the next turnnel section
        mFirstSection++;

        // discard obstacle corresponding to the deleted section
        if (mObstacleCount > 0) {
            // discarding first object (shifting) is easy because it's a circular buffer!
            mFirstObstacle = (mFirstObstacle + 1) % MAX_OBS;
            --mObstacleCount;
        }
    }
}

void PlayScene::UpdateMenuSelFromTouch(float x, float y) {
    float sh = SceneManager::GetInstance()->GetScreenHeight();
    int item = (int)floor((y / sh) * (mMenuItemCount));
    mMenuSel = Clamp(item, 0, mMenuItemCount - 1);
}

void PlayScene::OnPointerDown(int pointerId, const struct PointerCoords *coords) {
    float x = coords->x, y = coords->y;
    if (mMenu) {
        if (coords->isScreen) {
            UpdateMenuSelFromTouch(x, y);
            mMenuTouchActive = true;
        }
    } else if (mSteering != STEERING_TOUCH) {
        mPointerId = pointerId;
        mPointerAnchorX = x;
        mPointerAnchorY = y;
        mShipAnchorX = mPlayerPos.x;
        mShipAnchorZ = mPlayerPos.z;
        mSteering = STEERING_TOUCH;
    }
}

void PlayScene::OnPointerUp(int pointerId, const struct PointerCoords *coords) {
    if (mMenu && mMenuTouchActive) {
        if (coords->isScreen) {
            mMenuTouchActive = false;
            HandleMenu(mMenuItems[mMenuSel]);
        }
    } else if (mSteering == STEERING_TOUCH && pointerId == mPointerId) {
        mSteering = STEERING_NONE;
    }
}

void PlayScene::OnPointerMove(int pointerId, const struct PointerCoords *coords) {
    float rangeY = coords->isScreen ? SceneManager::GetInstance()->GetScreenHeight() :
            (coords->maxY - coords->minY);
    float x = coords->x, y = coords->y;

    if (mMenu && mMenuTouchActive) {
        UpdateMenuSelFromTouch(x, y);
    }
    else if (mSteering == STEERING_TOUCH && pointerId == mPointerId) {
        float deltaX = (x - mPointerAnchorX) * TOUCH_CONTROL_SENSIVITY / rangeY;
        float deltaY = -(y - mPointerAnchorY) * TOUCH_CONTROL_SENSIVITY / rangeY;
        float rotatedDx = cos(mRollAngle) * deltaX - sin(mRollAngle) * deltaY;
        float rotatedDy = sin(mRollAngle) * deltaX + cos(mRollAngle) * deltaY;

        mShipSteerX = mShipAnchorX + rotatedDx;
        mShipSteerZ = mShipAnchorZ + rotatedDy;
    }
}

void PlayScene::RenderHUD() {
    float aspect = SceneManager::GetInstance()->GetScreenAspect();
    glm::mat4 orthoMat = glm::ortho(0.0f, aspect, 0.0f, 1.0f);
    glm::mat4 modelMat;
    glm::mat4 mat;

    glDisable(GL_DEPTH_TEST);

    // render score digits
    int i, unit;
    static char score_str[6];
    int score = GetScore();
    for (i = 0, unit = 10000; i < 5; i++, unit /= 10) {
        score_str[i] = '0' + (score / unit) % 10;
    }
    score_str[i] = '\0';

    mTextRenderer->SetFontScale(SCORE_FONT_SCALE);
    mTextRenderer->RenderText(score_str, SCORE_POS_X, SCORE_POS_Y);

    // render current sign
    if (mSignText) {
        modelMat = glm::mat4(1.0f);
        float t = Clock() - mSignStartTime;
        if (t < SIGN_ANIM_DUR) {
            float scale = t / SIGN_ANIM_DUR;
            modelMat = glm::scale(modelMat, glm::vec3(1.0f, scale, 1.0f));
        } else if (mSignTimeLeft < SIGN_ANIM_DUR) {
            float scale = mSignTimeLeft / SIGN_ANIM_DUR;
            modelMat = glm::scale(modelMat, glm::vec3(1.0f, scale, 1.0f));
        }

        mTextRenderer->SetMatrix(modelMat);
        mTextRenderer->SetFontScale(SIGN_FONT_SCALE);
        mTextRenderer->RenderText(mSignText, aspect * 0.5f, 0.5f);
        mTextRenderer->ResetMatrix();
    }

    // render life icons
    glLineWidth(LIFE_LINE_WIDTH);
    float lifeX = LIFE_POS_X < 0.0f ? aspect + LIFE_POS_X : LIFE_POS_X;
    modelMat = glm::translate(glm::mat4(1.0), glm::vec3(lifeX, LIFE_POS_Y, 0.0f));
    modelMat = glm::scale(modelMat, glm::vec3(1.0f, LIFE_SCALE_Y, 1.0f));
    int ubound = (mBlinkingHeart && BlinkFunc(0.2f)) ? mLives + 1 : mLives;
    for (int i = 0; i < ubound; i++) {
        mat = orthoMat * modelMat;
        mTrivialShader->RenderSimpleGeom(&mat, mLifeGeom);
        modelMat = glm::translate(modelMat, glm::vec3(LIFE_SPACING_X, 0.0f, 0.0f));
    }

    glEnable(GL_DEPTH_TEST);
}

void PlayScene::RenderMenu() {
    float aspect = SceneManager::GetInstance()->GetScreenAspect();
    glm::mat4 modelMat;
    glm::mat4 mat;

    glDisable(GL_DEPTH_TEST);

    RenderBackgroundAnimation(mShapeRenderer);

    float scaleFactor = SineWave(1.0f, MENUITEM_PULSE_AMOUNT, MENUITEM_PULSE_PERIOD, 0.0f);

    int i;
    for (i = 0; i < mMenuItemCount; i++) {
        float thisFactor = (mMenuSel == i) ? scaleFactor : 1.0f;
        float y = 1.0f - (i + 1) / ((float)mMenuItemCount + 1);
        float x = aspect * 0.5f;
        mTextRenderer->SetFontScale(thisFactor * MENUITEM_FONT_SCALE);
        mTextRenderer->SetColor(mMenuSel == i ? MENUITEM_SEL_COLOR : MENUITEM_COLOR);
        mTextRenderer->RenderText(mMenuItemText[mMenuItems[i]], x, y);
    }
    mTextRenderer->ResetColor();

    glEnable(GL_DEPTH_TEST);
}

void PlayScene::DetectCollisions(float previousY) {
    Obstacle *o = GetObstacleAt(0);
    float obsCenter = GetSectionCenterY(mFirstSection);
    float obsMin = obsCenter - OBS_BOX_SIZE;
    float curY = mPlayerPos.y;

    if (!o || !(previousY < obsMin && curY >= obsMin)) {
        // no collision
        return;
    }

    // what row/column is the player on?
    int col = o->GetColAt(mPlayerPos.x);
    int row = o->GetRowAt(mPlayerPos.z);

    if (o->grid[col][row]) {
        // crashed against obstacle
        mLives--;
        if (mLives > 0) {
            ShowSign(S_OUCH, SIGN_DURATION);
            SfxMan::GetInstance()->PlayTone(TONE_CRASHED);
        } else {
            // say "Game Over"
            ShowSign(S_GAME_OVER, SIGN_DURATION_GAME_OVER);
            SfxMan::GetInstance()->PlayTone(TONE_GAME_OVER);
            mGameOverExpire = Clock() + GAME_OVER_EXPIRE;
        }
        mPlayerPos.y = obsMin - PLAYER_RECEDE_AFTER_COLLISION;
        mPlayerSpeed = PLAYER_SPEED_AFTER_COLLISION;
        mBlinkingHeart = true;
        mBlinkingHeartExpire = Clock() + BLINKING_HEART_DURATION;

        mLastCrashSection = mFirstSection;

    } else if (row == o->bonusRow && col == o->bonusCol) {
        ShowSign(S_GOT_BONUS, SIGN_DURATION_BONUS);
        o->DeleteBonus();
        AddScore(BONUS_POINTS);
        mBonusInARow++;

        if (mBonusInARow >= 10) {
            mBonusInARow = 0;
        }

        // update difficulty level, if applicable
        int score = GetScore();
        if (mDifficulty < score / SCORE_PER_LEVEL) {
            mDifficulty = score / SCORE_PER_LEVEL;
            ShowLevelSign();
            mObstacleGen.SetDifficulty(mDifficulty);
            SfxMan::GetInstance()->PlayTone(TONE_LEVEL_UP);

            // save progress, if needed
            SaveProgress();
        } else {
            int tone = (score % SCORE_PER_LEVEL) / BONUS_POINTS - 1;
            tone = tone < 0 ? 0 :
                   tone >= static_cast<int>(sizeof(TONE_BONUS)/sizeof(char*)) ?
                   static_cast<int>(sizeof(TONE_BONUS)/sizeof(char*) - 1) : tone;
            SfxMan::GetInstance()->PlayTone(TONE_BONUS[tone]);
        }

    } else if (o->HasBonus()) {
        // player missed bonus!
        mBonusInARow = 0;
    }

    // was it a close call?
    if (!o->grid[col][row]) {
        bool isCloseCall = false;
        for (int i = -1; i <= 1 && !isCloseCall; i++) {
            for (int j = -1; j <= 1; j++) {
                int other_row = o->GetColAt(mPlayerPos.x + i * CLOSE_CALL_CALC_DELTA);
                int other_col = o->GetRowAt(mPlayerPos.z + j * CLOSE_CALL_CALC_DELTA);
                if (o->grid[other_col][other_row]) {
                    isCloseCall = true;
                    break;
                }
            }
        }

    }
}

bool PlayScene::OnBackKeyPressed() {
    if (mMenu) {
        // reset frame clock so that the animation doesn't jump:
        mFrameClock.Reset();

        // leave menu
        ShowMenu(MENU_NONE);
    } else {
        // enter pause menu
        ShowMenu(MENU_PAUSE);
    }
    return true;
}


void PlayScene::OnJoy(float joyX, float joyY) {
    if (!mSteering || mSteering == STEERING_JOY) {
        float deltaX = joyX * JOYSTICK_CONTROL_SENSIVITY;
        float deltaY = joyY * JOYSTICK_CONTROL_SENSIVITY;
        float rotatedDx = cos(-mRollAngle) * deltaX - sin(-mRollAngle) * deltaY;
        float rotatedDy = sin(-mRollAngle) * deltaX + cos(-mRollAngle) * deltaY;
        mShipSteerX = rotatedDx;
        mShipSteerZ = -rotatedDy;
        mSteering = STEERING_JOY;

        // If player is going faster than the reference speed, PLAYER_SPEED, adjust it.
        // This makes the steering react faster as the ship accelerates in more difficult
        // levels.
        if (mPlayerSpeed > PLAYER_SPEED) {
            mShipSteerX *= mPlayerSpeed / PLAYER_SPEED;
            mShipSteerZ *= mPlayerSpeed / PLAYER_SPEED;
        }
    }
}

void PlayScene::OnKeyDown(int keyCode) {
    if (mMenu) {
        if (keyCode == OURKEY_UP) {
            mMenuSel = mMenuSel > 0 ? mMenuSel - 1 : mMenuSel;
        }
        else if (keyCode == OURKEY_DOWN) {
            mMenuSel = mMenuSel + 1 < mMenuItemCount ? mMenuSel + 1 : mMenuSel;
        }
        else if (keyCode == OURKEY_ENTER) {
            HandleMenu(mMenuItems[mMenuSel]);
        }
    }
}

void PlayScene::ShowMenu(int menu) {
    mMenu = menu;
    mMenuSel = 0;
    switch (menu) {
        case MENU_PAUSE:
            mMenuItems[0] = MENUITEM_UNPAUSE;
            mMenuItems[1] = MENUITEM_QUIT;
            mMenuItemCount = 2;
            break;
        case MENU_LEVEL:
            mMenuItems[0] = MENUITEM_RESUME;
            mMenuItems[1] = MENUITEM_START_OVER;
            mMenuItemCount = 2;
            break;
        default:
            // since we're leaving the menu, reset the frame clock to avoid a skip
            // in the animation
            mFrameClock.Reset();
    }
}

void PlayScene::HandleMenu(int menuItem) {
    switch (menuItem) {
        case MENUITEM_QUIT:
            SceneManager::GetInstance()->RequestNewScene(new WelcomeScene());
            break;
        case MENUITEM_UNPAUSE:
            ShowMenu(MENU_NONE);
            break;
        case MENUITEM_RESUME:
            // resume from saved level
            mDifficulty = (mSavedCheckpoint / LEVELS_PER_CHECKPOINT) * LEVELS_PER_CHECKPOINT;
            SetScore(SCORE_PER_LEVEL * mDifficulty);
            mObstacleGen.SetDifficulty(mDifficulty);
            ShowLevelSign();
            ShowMenu(MENU_NONE);
            break;
        case MENUITEM_START_OVER:
            // start over from scratch
            ShowMenu(MENU_NONE);
            break;
    }
}

void PlayScene::ShowLevelSign() {
    static char level_str[] = "LEVEL XX";
    int level = mDifficulty + 1;
    level_str[6] = '0' + ((level > 9) ? (level / 10) % 10 : level % 10);
    level_str[7] = (level > 9) ? ('0' + level % 10) : '\0';
    level_str[8] = '\0';
    ShowSign(level_str, SIGN_DURATION);
}

void PlayScene::OnPause() {
    if (mMenu == MENU_NONE) {
        ShowMenu(MENU_PAUSE);
    }
}

void PlayScene::OnScreenResized(int width, int height) {
    UpdateProjectionMatrix();
}

void PlayScene::UpdateProjectionMatrix() {
    SceneManager *mgr = SceneManager::GetInstance();
    mProjMat = glm::perspective(RENDER_FOV, mgr->GetScreenAspect(), RENDER_NEAR_CLIP,
            RENDER_FAR_CLIP);
}

