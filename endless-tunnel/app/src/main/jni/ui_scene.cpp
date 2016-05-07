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
#include "ui_scene.hpp"

#include "data/strings.inl"

// how much do buttons pulse?
#define PULSE_AMOUNT 0.01f
#define PULSE_PERIOD 0.5f

// how big (as a fraction of the button's dimension) is the border?
#define BUTTON_BORDER_SIZE 0.1f

// scale of the "please wait" sign
#define WAIT_SIGN_SCALE 1.0f

// transition duration
#define TRANSITION_DURATION 0.3f

// default button colors
const static float BUTTON_FOCUS_COLOR[] = { 1.0f, 1.0f, 0.0f };
const static float BUTTON_DISABLED_COLOR[] = { 0.3f, 0.3f, 0.3f };

UiScene::UiScene() {
    mWidgetCount = 0;
    memset(mWidgets, 0, sizeof(mWidgets));
    mFocusWidget = -1;
    mTrivialShader = NULL;
    mTextRenderer = NULL;
    mShapeRenderer = NULL;
    mDefaultButton = -1;
    mPointerDown = false;
    mWaitScreen = false;
    mTransitionStart = 0.0f;
}

UiScene::~UiScene() {
    // note: cleanup for graphics-related stuff goes in OnKillGraphics

    int i;
    for (i = 0; i < mWidgetCount; ++i) {
        CleanUp(&mWidgets[i]);
    }
    mWidgetCount = 0;
}

UiWidget* UiScene::NewWidget() {
    MY_ASSERT(mWidgetCount + 1 < MAX_WIDGETS);
    UiWidget *widget = new UiWidget(mWidgetCount);
    mWidgets[mWidgetCount++] = widget;
    return widget;
}

void UiScene::OnStartGraphics() {
    mTrivialShader = new TrivialShader();
    mTrivialShader->Compile();
    mTextRenderer = new TextRenderer(mTrivialShader);
    mShapeRenderer = new ShapeRenderer(mTrivialShader);

    for (int i = 0; i < mWidgetCount; ++i) {
        mWidgets[i]->StartGraphics();
    }
    mTransitionStart = Clock();

    if (mWidgetCount <= 0) {
        // time to create our widgets
        OnCreateWidgets();
    }
}

void UiScene::OnKillGraphics() {
    CleanUp(&mTextRenderer);
    CleanUp(&mShapeRenderer);
    CleanUp(&mTrivialShader);

    for (int i = 0; i < mWidgetCount; ++i) {
        mWidgets[i]->KillGraphics();
    }

    // remove all widgets
    DeleteWidgets();
}

void UiScene::OnScreenResized(int width, int height) {
    // screen got resized; if we have widgets and graphics, we have to recreate them
    SceneManager *mgr = SceneManager::GetInstance();
    if (mgr->HasGraphics() && mWidgetCount > 0) {
        DeleteWidgets();
        OnCreateWidgets();
    }
}


UiWidget* UiScene::GetWidgetById(int id) {
    return (id < 0 || id >= mWidgetCount) ? NULL : mWidgets[id];
}

void UiScene::DoFrame() {
    SceneManager *mgr = SceneManager::GetInstance();

    // clear screen
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    // render background
    RenderBackground();

    // if we're in wait screen mode, render the "Please Wait" sign and do nothing else
    if (mWaitScreen) {
        mTextRenderer->SetFontScale(WAIT_SIGN_SCALE);
        mTextRenderer->SetColor(1.0f, 1.0f, 1.0f);
        mTextRenderer->RenderText(S_PLEASE_WAIT, mgr->GetScreenAspect() * 0.5f, 0.5f);
        glEnable(GL_DEPTH_TEST);
        return;
    }

    // if the currently focused widget is not a clickable button, get out of focus mode
    if (mFocusWidget >= 0 && !mWidgets[mFocusWidget]->IsClickableButton()) {
        mFocusWidget = -1;
    }

    // calculate transition factor, which is 0 when we're starting the transition
    // and 1 when we've finished the transition
    float tf = Clamp((Clock() - mTransitionStart) / TRANSITION_DURATION, 0.0f, 1.0f);

    // render ALL the widgets!
    int i;
    for (i = 0; i < mWidgetCount; ++i) {
        mWidgets[i]->Render(mTrivialShader, mTextRenderer, mShapeRenderer,
                (mFocusWidget < 0) ? UiWidget::FOCUS_NOT_APPLICABLE :
                (mFocusWidget == i) ? UiWidget::FOCUS_YES : UiWidget::FOCUS_NO, tf);
    }

    glEnable(GL_DEPTH_TEST);
}

void UiScene::RenderBackground() {
    // base classes override this to draw background
}

void UiScene::OnButtonClicked(int buttonId) {
    // base classes override this to react to button clicks
}

void UiScene::UpdateTouchFocus(const struct PointerCoords *coords) {
    // translate to our coordinate system
    float h = SceneManager::GetInstance()->GetScreenHeight();
    float x = coords->x / h;
    float y = (h - coords->y) / h;

    int i;
    mFocusWidget = -1;
    for (i = 0; i < mWidgetCount; ++i) {
        UiWidget *w = mWidgets[i];
        if (w->PointBelongs(x, y) && w->IsClickableButton()) {
            mFocusWidget = i;
            return;
        }
    }
}

void UiScene::OnPointerDown(int pointerId, const struct PointerCoords *coords) {
    // If this event was generated by something that's not associated to the screen,
    // (like a trackpad), ignore it, because our UI is not driven that way.
    if (coords->isScreen && !mWaitScreen) {
        mPointerDown = true;
        UpdateTouchFocus(coords);
    }
}

void UiScene::OnPointerMove(int pointerId, const struct PointerCoords *coords) {
    if (coords->isScreen && mPointerDown && !mWaitScreen) {
        UpdateTouchFocus(coords);
    }
}

void UiScene::OnPointerUp(int pointerId, const struct PointerCoords *coords) {
    if (!coords->isScreen || mWaitScreen) {
        return;
    }

    // if we didn't know the pointer was down, this event should not trigger anything
    if (!mPointerDown) {
        return;
    }

    mPointerDown = false;
    UpdateTouchFocus(coords);

    if (mFocusWidget >= 0 && mWidgets[mFocusWidget]->IsButton()) {
        DispatchButtonClick(mFocusWidget);
        mFocusWidget = -1;
    }
}

void UiScene::DispatchButtonClick(int id) {
    UiWidget *w = GetWidgetById(id);
    if (w && w->IsClickableButton()) {
        OnButtonClicked(id);
    }
}

int UiScene::FindDefaultButton() {
    if (mDefaultButton >= 0) {
        return mDefaultButton;
    }
    int i;
    for (i = 0; i < mWidgetCount; ++i) {
        UiWidget *w = mWidgets[i];
        if (w->IsClickableButton()) {
            return i;
        }
    }
    return -1;
}

void UiScene::OnKeyDown(int ourKeyCode) {
    if (mWaitScreen) {
        return;
    }

    int navDir = -1;
    switch (ourKeyCode) {
        case OURKEY_UP:
            navDir = UI_DIR_UP;
            break;
        case OURKEY_DOWN:
            navDir = UI_DIR_DOWN;
            break;
        case OURKEY_LEFT:
            navDir = UI_DIR_LEFT;
            break;
        case OURKEY_RIGHT:
            navDir = UI_DIR_RIGHT;
            break;
        case OURKEY_ENTER:
            DispatchButtonClick(mFocusWidget >= 0 ? mFocusWidget : mDefaultButton);
            break;
    }

    if (navDir >= 0) {
        if (mFocusWidget < 0) {
            // enter focus mode
            mFocusWidget = FindDefaultButton();
        } else {
            // navigate
            int destId = -1;
            UiWidget *w = GetWidgetById(mFocusWidget);
            UiWidget *destWidget = w ? GetWidgetById(destId = w->GetNav(navDir)) : NULL;
            if (destWidget && destWidget->IsClickableButton()) {
                // navigate to that widget
                mFocusWidget = destId;
            }
        }
    }
}

void UiScene::AddNav(int fromWidgetId, int dir, int toWidgetId) {
    UiWidget *from = GetWidgetById(fromWidgetId);
    UiWidget *to = GetWidgetById(toWidgetId);
    if (from && to) {
        from->SetNav(dir, toWidgetId);
    }
}

static void _apply_transition(int trans, float f, float *x, float *y, float *w, float *h,
        float *fontScale) {
    float maxX = SceneManager::GetInstance()->GetScreenAspect();
    switch (trans) {
        case UiWidget::TRANS_SCALE:
            *w = *w * f;
            *h = *h * f;
            *fontScale = *fontScale * f;
            break;
        case UiWidget::TRANS_FROM_LEFT:
            *x = Interpolate(0.0f, *x - *w * 0.5f, 1.0f, *x, f);
            break;
        case UiWidget::TRANS_FROM_RIGHT:
            *x = Interpolate(0.0f, maxX + *w * 0.5f, 1.0f, *x, f);
            break;
        case UiWidget::TRANS_FROM_TOP:
            *y = Interpolate(0.0f, 1.0f + *h * 0.5f, 1.0f, *y, f);
            break;
        case UiWidget::TRANS_FROM_BOTTOM:
            *y = Interpolate(0.0f, *y - *h * 0.5f, 1.0f, *y, f);
            break;
    }
}

void UiWidget::Render(TrivialShader *trivialShader, TextRenderer *textRenderer,
        ShapeRenderer *shapeRenderer, int focus, float transitionFactor) {
    if (!mVisible) {
        // that was easy.
        return;
    }

    bool pulse = IsClickableButton() && (focus != FOCUS_NO);
    float factor = pulse ? SineWave(1.0f - PULSE_AMOUNT, 1.0f + PULSE_AMOUNT,
            PULSE_PERIOD, 0.0f) : 1.0f;
    const float *color = (mIsButton && focus == FOCUS_YES) ? BUTTON_FOCUS_COLOR :
            (mIsButton && !mEnabled) ? BUTTON_DISABLED_COLOR : mTextColor;
    float borderSize = 0.0f;
    float x = mCenterX;
    float y = mCenterY;
    float w = mWidth;
    float h = mHeight;
    float fontScale = mFontScale;

    _apply_transition(mTransition, transitionFactor, &x, &y, &w, &h, &fontScale);

    // Note: right now, we don't support buttons that have borders AND are transparent.
    // They will be rendered incorrectly (the background will be the border color).

    if (mHasBorder || (focus == FOCUS_YES && !mTransparent)) {
        // draw border
        shapeRenderer->SetColor(color);
        shapeRenderer->RenderRect(x, y, w * factor, h * factor);
        borderSize = BUTTON_BORDER_SIZE;
    }

    // draw background
    if (mIsButton && !mTransparent) {
        shapeRenderer->SetColor(mBackColor);
        shapeRenderer->RenderRect(x, y, w * factor * (1.0f - borderSize),
                h * factor * (1.0f - borderSize));
    }

    // draw text
    if (mText) {
        textRenderer->SetColor(color);
        textRenderer->SetFontScale(fontScale * factor);
        textRenderer->RenderText(mText, x, y);
    }
}

void UiScene::OnCreateWidgets() {}

