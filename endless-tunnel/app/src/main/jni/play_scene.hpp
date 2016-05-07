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
#ifndef endlesstunnel_play_scene_h
#define endlesstunnel_play_scene_h

#include "engine.hpp"
#include "obstacle_generator.hpp"
#include "obstacle.hpp"
#include "sfxman.hpp"
#include "shape_renderer.hpp"
#include "text_renderer.hpp"
#include "util.hpp"

class OurShader;

/* This is the gameplay scene -- the scene that shows the player flying down
 * the infinite tunnel, dodging obstacles, collecting bonuses and being awesome. */
class PlayScene : public Scene {
    public:
        PlayScene();
        virtual void OnStartGraphics();
        virtual void OnKillGraphics();
        virtual void DoFrame();
        virtual void OnPointerDown(int pointerId, const struct PointerCoords *coords);
        virtual void OnPointerUp(int pointerId, const struct PointerCoords *coords);
        virtual void OnPointerMove(int pointerId, const struct PointerCoords *coords);
        virtual bool OnBackKeyPressed();
        virtual void OnScreenResized(int width, int height);
        virtual void OnJoy(float joyX, float joyY);
        virtual void OnKeyDown(int keyCode);
        virtual void OnPause();

    protected:
        // shaders
        OurShader *mOurShader;
        TrivialShader *mTrivialShader;

        // the wall texture
        Texture *mWallTexture;

        // shape and text renderers we use when rendering the HUD
        ShapeRenderer *mShapeRenderer;
        TextRenderer *mTextRenderer;

        // matrices
        glm::mat4 mViewMat, mProjMat;

        // player's position and direction
        glm::vec3 mPlayerPos, mPlayerDir;

        // lives left
        int mLives;

        // player's score. As a trivial form of protection (just to give crackers a
        // hard time), we *actually* store the score encrypted in mEncryptedScore, but have a
        // fake variable mFakeScore that stores a copy of it. This serves as a honeypot to
        // an attacker who's trying to crack the game using a memory editor.
        unsigned mFakeScore;
        unsigned mEncryptedScore;

        // current difficulty level
        int mDifficulty;

        // should we use cloud save? If not, we will save progress to local data only.
        bool mUseCloudSave;

        // greatest checkpoint level attained by player (loaded from file)
        int mSavedCheckpoint;

        // vertex buffer and index buffer to render tunnel
        SimpleGeom *mTunnelGeom;

        // vertex buffer to render obstacles
        SimpleGeom *mCubeGeom;

        // what is the first tunnel section that we are rendering
        int mFirstSection;

        // circular buffer of obstacles (mObstacleCircBuf[mFirstObstacle...])
        // There is exactly one obstacle for each tunnel section:
        // obstacle 0 is at section mFirstSection
        // obstacle 1 is at section mFirstSection + 1
        // and so on and so forth.
        static const int MAX_OBS = RENDER_TUNNEL_SECTION_COUNT * 2;
        int mFirstObstacle;
        int mObstacleCount;
        Obstacle mObstacleCircBuf[MAX_OBS];

        // obstacle generator
        ObstacleGenerator mObstacleGen;

        // touch pointer ID and anchor position (where touch started)
        static const int STEERING_NONE = 0, STEERING_TOUCH = 1, STEERING_JOY = 2;
        int mSteering;  // is player steering at the moment? If so, how?
        int mPointerId;  // if so, what's the pointer ID
        float mPointerAnchorX, mPointerAnchorY; // where the drag started
        float mShipAnchorX, mShipAnchorZ; // x,z of ship when drag started
        float mShipSteerX, mShipSteerZ; // target x,z of ship (when using touch control) or
                                        // velocity vector (when using joystick)

        // moving average filter for input (on mShipSteerX and mShipSteerY)
        static const int NOISE_FILTER_SAMPLES = 5;
        float mFilteredSteerX, mFilteredSteerZ;

        // frame clock -- it computes the deltas between successive frames so we can
        // update stuff properly
        DeltaClock mFrameClock;

        // sign (string) that we're currently showing (NULL if none)
        const char *mSignText;
        bool mSignExpires; // does the sign expire after a while?
        float mSignTimeLeft; // for how much longer the sign will still be on screen
        float mSignStartTime; // time when sign was shown

        // did we already show the instructions?
        bool mShowedHowto;

        // are we showing a menu? If so, what menu?
        static const int MENU_NONE = 0;
        static const int MENU_PAUSE = 1; // pause menu
        static const int MENU_LEVEL = 2; // select starting level
        int mMenu;

        // identifiers for each menu item
        static const int MENUITEM_UNPAUSE = 0;
        static const int MENUITEM_QUIT = 1;
        static const int MENUITEM_START_OVER = 2;
        static const int MENUITEM_RESUME = 3;
        static const int MENUITEM_COUNT = 4;

        // text for each menu item
        const char *mMenuItemText[MENUITEM_COUNT];

        // menu items on current menu
        static const int MENUITEMS_MAX = 4;
        int mMenuItems[MENUITEMS_MAX];
        int mMenuItemCount; // # of menu items
        int mMenuSel; // index of selected menu item

        // is user touching the screen to select menu? are they using the buttons?
        bool mMenuTouchActive;

        // heart geom (to display # lives)
        SimpleGeom *mLifeGeom;

        // current roll angle, in degrees, counterclockwise from original
        float mRollAngle;

        // current speed
        float mPlayerSpeed;

        // are we showing the "just lost a heart" animation? If so, when does it expire?
        bool mBlinkingHeart;
        float mBlinkingHeartExpire;

        // when should the game expire? This will be set after the game is over (mLives <= 0)
        // and indicates when we should return to the main screen
        float mGameOverExpire;

        // time when game started
        float mGameStartTime;

        // how many bonuses were collected without missing one?
        int mBonusInARow;

        // what was the section number of the last obstacle with which the player crashed?
        int mLastCrashSection;


        // last subsection were an ambient sound was emitted
        int mLastAmbientBeepEmitted;

        // name of the save file
        char *mSaveFileName;

        // pending to show a "checkpoint saved" sign?
        bool mCheckpointSignPending;

        // get current score
        int GetScore() {
            return (int)(mEncryptedScore ^ 0x600673);
        }

        // set current score
        void SetScore(int s) {
            mFakeScore = (unsigned)s;
            mEncryptedScore = mFakeScore ^ 0x600673;
        }

        // add to current score
        void AddScore(int s) {
            SetScore(GetScore() + s);
        }

        // generate new obstacles as needed
        void GenObstacles();

        // renders the tunnel walls
        void RenderTunnel();

        // renders the obstacles
        void RenderObstacles();

        // renders the HUD (score, lives, etc)
        void RenderHUD();

        // renders the currently active menu
        void RenderMenu();

        // Shift tunnel sections if needed (this means discarding the ones the
        // player has already past and generating the obstacles for the new ones
        // that came into view)
        void ShiftIfNeeded();

        // detect if the player hit obstacles or got the bonus
        void DetectCollisions(float previousY);

        // shows a text sign on the middle of the screen
        void ShowSign(const char* sign, float timeout) {
            mSignTimeLeft = timeout;
            mSignText = sign;
            mSignExpires = true;
            mSignStartTime = Clock();
        }
        void ShowSign(const char* sign) {
            mSignText = sign;
            mSignExpires = false;
            mSignStartTime = Clock();
        }
        Obstacle* GetObstacleAt(int i) {
            return &mObstacleCircBuf[(mFirstObstacle + i) % MAX_OBS];
        }

        // shows the given menu
        void ShowMenu(int menu);

        // handle the fact that the given menu item was selected
        void HandleMenu(int menuItem);

        // updates which menu item is selected based on where the screen was touched
        void UpdateMenuSelFromTouch(float x, float y);

        // writes to the local save file
        void WriteSaveFile(int level);

        // loads progress from the local save file and/or cloudsave
        void LoadProgress();

        // saves progress to the local save file and/or cloudsave
        void SaveProgress();

        // returns whether or not this level is a "checkpoint level" (that is,
        // where progress should be saved)
        bool IsCheckpointLevel() {
            return 0 == mDifficulty % LEVELS_PER_CHECKPOINT;
        }

        // shows the sign that tells the player they've reached a new level.
        // (like "LEVEL 5").
        void ShowLevelSign();

        // update projection matrix
        void UpdateProjectionMatrix();
};

#endif

