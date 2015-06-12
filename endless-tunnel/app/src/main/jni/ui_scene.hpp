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
#ifndef endlesstunnel_ui_scene_hpp
#define endlesstunnel_ui_scene_hpp

#include "ascii_to_geom.hpp"
#include "engine.hpp"
#include "shape_renderer.hpp"
#include "text_renderer.hpp"
#include "util.hpp"

#define UI_DIR_UP 0
#define UI_DIR_RIGHT 1
#define UI_DIR_DOWN 2
#define UI_DIR_LEFT 3

class UiWidget;

/* UI scene. This is a base class for all scenes that present menus, etc. */
class UiScene : public Scene {
    protected:
        // the widgets we currently have
        static const int MAX_WIDGETS = 64;
        int mWidgetCount;
        UiWidget *mWidgets[MAX_WIDGETS];

        // which widget currently has focus? If we are not in focus mode, this is -1
        int mFocusWidget;

        // is a touch pointer (a.k.a. finger) down at the moment?
        bool mPointerDown;

        // what is the default button? The default widget is the one that we assume was
        // clicked if the user presses ENTER on the DPAD when not in focus mode.
        int mDefaultButton;

        // renderers and shaders we need
        TrivialShader *mTrivialShader;
        TextRenderer *mTextRenderer;
        ShapeRenderer *mShapeRenderer;

        // if true, shows a "please wait" screen instead of the interface
        bool mWaitScreen;

        // must be implemented by subclass
        virtual void OnButtonClicked(int buttonId);
        virtual void RenderBackground();

        // transition start time
        float mTransitionStart;

        // add a new widget
        inline UiWidget* NewWidget();

        // set default button -- the button that is assumed to be clicked when the user
        // presses the the "confirm" button (usually DPAD_CENTER or the ENTER key on a
        // keyboard).
        inline void SetDefaultButton(int id) {
            mDefaultButton = id;
        }

        inline void SetWaitScreen(bool b) {
            mWaitScreen = b;
            if (mWaitScreen) {
                mTransitionStart = Clock();
            }
        }

        // add a navigation rule
        void AddNav(int fromWidgetId, int dir, int toWidgetId);

        // subclasses must override these to create their widgets
        virtual void OnCreateWidgets();

    public:
        UiScene();
        virtual ~UiScene();


        virtual void OnStartGraphics();
        virtual void OnKillGraphics();
        virtual void DoFrame();
        virtual void OnPointerDown(int pointerId, const struct PointerCoords *coords);
        virtual void OnPointerMove(int pointerId, const struct PointerCoords *coords);
        virtual void OnPointerUp(int pointerId, const struct PointerCoords *coords);
        virtual void OnKeyDown(int ourKeyCode);
        virtual void OnScreenResized(int width, int height);

        UiWidget* GetWidgetById(int id);

    private:
        void UpdateTouchFocus(const struct PointerCoords *coords);
        void DispatchButtonClick(int id);
        int FindDefaultButton();

        inline void DeleteWidgets() {
            mWidgetCount = 0;
        }
};


/* Represents a single UI widget on the screen, like a block of text or a button. */
class UiWidget {
    private:
        int mId;
        float mCenterX, mCenterY, mWidth, mHeight;
        float mBackColor[3], mTextColor[3];
        bool mHasBorder;
        bool mTransparent;
        bool mIsButton;
        bool mVisible;
        bool mEnabled;
        const char* mText;
        const char* mIconArt;
        float mIconScale;
        float mFontScale;
        bool mHasGraphics;
        int mTransition;

        // only exists between StartGraphics() and KillGraphics()
        SimpleGeom* mIconGeom;

        // which widget lies logically to each direction of this widget (for
        // dpad navigation). Indexed by the UI_DIR_* constants.
        int mNav[4];

    public:
        static const int TRANS_NONE = 0;
        static const int TRANS_SCALE = 1;
        static const int TRANS_FROM_LEFT = 2;
        static const int TRANS_FROM_RIGHT = 3;
        static const int TRANS_FROM_TOP = 4;
        static const int TRANS_FROM_BOTTOM = 5;

        inline UiWidget(int id) {
            mId = id;
            mTransition = TRANS_NONE;
            mCenterX = mCenterY = mWidth = mHeight = 0.0f;
            mIsButton = false;
            memset(mBackColor, 0, sizeof(mBackColor));
            memset(mTextColor, 0, sizeof(mTextColor));
            mTextColor[0] = mTextColor[1] = mTextColor[2] = 1.0f;
            mText = NULL;
            mIconGeom = NULL;
            mHasGraphics = false;
            mEnabled = true;
            mHasBorder = false;
            mTransparent = false;
            mVisible = true;
            mFontScale = 1.0f;
            mNav[0] = mNav[1] = mNav[2] = mNav[3] = -1;
            mIconScale = 1.0f;
            mIconArt = NULL;
        }

        inline ~UiWidget() {
            CleanUp(&mIconGeom);
        }

        inline int GetId() { return mId; }
        inline float GetCenterX() { return mCenterX; }
        inline float GetCenterY() { return mCenterY; }
        inline float GetWidth() { return mWidth; }
        inline float GetHeight() { return mHeight; }
        inline bool IsButton() { return mIsButton; }
        inline int GetNav(int dir) { return dir >= 0 && dir < 4 ? mNav[dir] : -1; }
        inline bool IsEnabled() { return mEnabled; }
        inline bool IsVisible() { return mVisible; }

        inline UiWidget* SetNav(int dir, int id) {
            if (dir >= 0 && dir < 4) {
                mNav[dir] = id;
            }
            return this;
        }
        inline UiWidget* SetTransition(int trans) {
            mTransition = trans;
            return this;
        }
        inline UiWidget* SetEnabled(bool enabled) {
            mEnabled = enabled;
            return this;
        }
        inline UiWidget* SetVisible(bool visible) {
            mVisible = visible;
            return this;
        }
        inline UiWidget* SetCenter(float x, float y) {
            mCenterX = x;
            mCenterY = y;
            return this;
        }
        inline UiWidget* SetSize(float w, float h) {
            mWidth = w;
            mHeight = h;
            return this;
        }
        inline UiWidget* SetText(const char* text) {
            mText = text;
            return this;
        }
        inline UiWidget* SetTextColor(float r, float g, float b) {
            mTextColor[0] = r;
            mTextColor[1] = g;
            mTextColor[2] = b;
            return this;
        }
        inline UiWidget* SetTextColor(const float *color) {
            mTextColor[0] = color[0];
            mTextColor[1] = color[1];
            mTextColor[2] = color[2];
            return this;
        }

        inline UiWidget* SetFontScale(float scale) {
            mFontScale = scale;
            return this;
        }

        inline UiWidget* SetBackColor(float r, float g, float b) {
            mBackColor[0] = r;
            mBackColor[1] = g;
            mBackColor[2] = b;
            return this;
        }
        inline UiWidget* SetTransparent(bool transp) {
            mTransparent = true;
            return this;
        }
        inline UiWidget* SetIsButton(bool isButton) {
            mIsButton = isButton;
            mHasBorder = true;
            return this;
        }
        inline UiWidget* SetHasBorder(bool border) {
            mHasBorder = border;
            return this;
        }
        inline UiWidget* SetIconFromAsciiArt(const char *asciiArt, float scale) {
            mIconArt = asciiArt;
            mIconScale = scale;
            if (mHasGraphics) {
                CleanUp(&mIconGeom);
                mIconGeom = AsciiArtToGeom(mIconArt, mIconScale);
            }
            return this;
        }
        inline void StartGraphics() {
            if (mIconArt) {
                mIconGeom = AsciiArtToGeom(mIconArt, mIconScale);
            }
            mHasGraphics = true;
        }
        inline void KillGraphics() {
            CleanUp(&mIconGeom);
            mHasGraphics = false;
        }
        inline bool PointBelongs(float x, float y) {
            float dx = fabsf(x - mCenterX);
            float dy = fabsf(y - mCenterY);
            return dx < 0.5f * mWidth && dy < 0.5f * mHeight;
        }

        inline bool IsClickableButton() {
            return mIsButton && mVisible && mEnabled;
        }

        static const int FOCUS_NOT_APPLICABLE = 0;
        static const int FOCUS_YES = 1;
        static const int FOCUS_NO = 2;
        void Render(TrivialShader *trivialShader, TextRenderer *textRenderer,
                ShapeRenderer *shapeRenderer, int focus, float transitionFactor);
};

UiWidget* UiScene::NewWidget() {
    MY_ASSERT(mWidgetCount + 1 < MAX_WIDGETS);
    UiWidget *newWidget = new UiWidget(mWidgetCount);
    mWidgets[mWidgetCount++] = newWidget;
    return newWidget;
}

#endif

