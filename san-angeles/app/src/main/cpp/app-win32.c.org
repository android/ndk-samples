/* San Angeles Observation OpenGL ES version example
 * Copyright 2004-2005 Jetro Lauha
 * All rights reserved.
 * Web: http://iki.fi/jetro/
 *
 * This source is free software; you can redistribute it and/or
 * modify it under the terms of EITHER:
 *   (1) The GNU Lesser General Public License as published by the Free
 *       Software Foundation; either version 2.1 of the License, or (at
 *       your option) any later version. The text of the GNU Lesser
 *       General Public License is included with this source in the
 *       file LICENSE-LGPL.txt.
 *   (2) The BSD-style license that is included with this source in
 *       the file LICENSE-BSD.txt.
 *
 * This source is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files
 * LICENSE-LGPL.txt and LICENSE-BSD.txt for more details.
 *
 * $Id: app-win32.c,v 1.6 2005/02/24 20:29:00 tonic Exp $
 * $Revision: 1.6 $
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#ifdef UNDER_CE
#include <aygshell.h>
#endif

#include <stdio.h>

#include "importgl.h"

#include "app.h"


int gAppAlive = 1;

static HINSTANCE sInstance;

static const _TCHAR sAppName[] =
    _T("San Angeles Observation OpenGL ES version example (Win32)");
static HWND sWnd;
static int sWindowWidth = WINDOW_DEFAULT_WIDTH;
static int sWindowHeight = WINDOW_DEFAULT_HEIGHT;
static EGLDisplay sEglDisplay = EGL_NO_DISPLAY;
static EGLConfig sEglConfig;
static EGLContext sEglContext = EGL_NO_CONTEXT;
static EGLSurface sEglSurface = EGL_NO_SURFACE;


static void checkGLErrors()
{
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        _TCHAR errorString[32];
        _stprintf(errorString, _T("0x%04x"), error);
        MessageBox(NULL, errorString, _T("GL Error"), MB_OK);
    }
}


static void checkEGLErrors()
{
    EGLint error = eglGetError();
    if (error != EGL_SUCCESS)
    {
        _TCHAR errorString[32];
        _stprintf(errorString, _T("0x%04x"), error);
        MessageBox(NULL, errorString, _T("EGL Initialization Error"), MB_OK);
    }
}


static BOOL initEGL(HWND wnd)
{
    static const EGLint configAttribs[] =
    {
#if (WINDOW_BPP == 16)
        EGL_RED_SIZE,       5,
        EGL_GREEN_SIZE,     5,
        EGL_BLUE_SIZE,      5,
#elif (WINDOW_BPP == 32)
        EGL_RED_SIZE,       8,
        EGL_GREEN_SIZE,     8,
        EGL_BLUE_SIZE,      8,
#else
#error WINDOW_BPP must be 16 or 32
#endif
        EGL_DEPTH_SIZE,     16,
        EGL_ALPHA_SIZE,     EGL_DONT_CARE,
        EGL_STENCIL_SIZE,   EGL_DONT_CARE,
        EGL_SURFACE_TYPE,   EGL_WINDOW_BIT,
        EGL_NONE
    };
    EGLBoolean success;
    EGLint numConfigs;
    EGLint majorVersion;
    EGLint minorVersion;
#ifdef PVRSDK
    HDC dc;
#endif // PVRSDK

#ifndef DISABLE_IMPORTGL
    int importGLResult;
    importGLResult = importGLInit();
    if (!importGLResult)
        return FALSE;
#endif // !DISABLE_IMPORTGL

#ifdef PVRSDK
    dc = GetDC(sWnd);
    sEglDisplay = eglGetDisplay(dc);
#else
    sEglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
#endif // !PVRSDK
    success = eglInitialize(sEglDisplay, &majorVersion, &minorVersion);
    if (success != EGL_FALSE)
        success = eglGetConfigs(sEglDisplay, NULL, 0, &numConfigs);
    if (success != EGL_FALSE)
        success = eglChooseConfig(sEglDisplay, configAttribs,
                                  &sEglConfig, 1, &numConfigs);
    if (success != EGL_FALSE)
    {
        sEglSurface = eglCreateWindowSurface(sEglDisplay, sEglConfig,
                                             wnd, NULL);
        if (sEglSurface == EGL_NO_SURFACE)
            success = EGL_FALSE;
    }
    if (success != EGL_FALSE)
    {
        sEglContext = eglCreateContext(sEglDisplay, sEglConfig, NULL, NULL);
        if (sEglContext == EGL_NO_CONTEXT)
            success = EGL_FALSE;
    }
    if (success != EGL_FALSE)
        success = eglMakeCurrent(sEglDisplay, sEglSurface,
                                 sEglSurface, sEglContext);

    if (success == EGL_FALSE)
        checkEGLErrors();

    return success;
}


static void deinitEGL()
{
    eglMakeCurrent(sEglDisplay, NULL, NULL, NULL);
    eglDestroyContext(sEglDisplay, sEglContext);
    eglDestroySurface(sEglDisplay, sEglSurface);
    eglTerminate(sEglDisplay);
#ifndef DISABLE_IMPORTGL
    importGLDeinit();
#endif // !DISABLE_IMPORTGL
}


static LRESULT CALLBACK wndProc(HWND wnd, UINT message,
                                WPARAM wParam, LPARAM lParam)
{
    RECT rc;
    int useDefWindowProc = 0;

    switch (message)
    {
    case WM_CLOSE:
        DestroyWindow(wnd);
        gAppAlive = 0;
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        gAppAlive = 0;
        break;

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE || wParam == VK_RETURN)
            gAppAlive = 0;
        useDefWindowProc = 1;
        break;

    case WM_KEYUP:
        useDefWindowProc = 1;
        break;

    case WM_SIZE:
        GetClientRect(sWnd, &rc);
        sWindowWidth = rc.right;
        sWindowHeight = rc.bottom;
        break;

    default:
        useDefWindowProc = 1;
    }

    if (useDefWindowProc)
        return DefWindowProc(wnd, message, wParam, lParam);
    return 0;
}


int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance,
                   LPTSTR cmdLine, int cmdShow)
{
    MSG msg;
    WNDCLASS wc;
    DWORD windowStyle;
    int windowX, windowY;

    // not referenced:
    prevInstance = prevInstance;
    cmdLine = cmdLine;


    sInstance = instance;

    // register class
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = (WNDPROC)wndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = sInstance;
    wc.hIcon = NULL;
    wc.hCursor = 0;
    wc.hbrBackground = GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = sAppName;
    if (!RegisterClass(&wc))
        return FALSE;

    // init instance
    windowStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
#ifdef UNDER_CE
    sWindowWidth = GetSystemMetrics(SM_CXSCREEN);
    sWindowHeight = GetSystemMetrics(SM_CYSCREEN);
    windowX = windowY = 0;
#else
    windowStyle |= WS_OVERLAPPEDWINDOW;
    windowX = CW_USEDEFAULT;
    windowY = 0;
#endif
    sWnd = CreateWindow(sAppName, sAppName, windowStyle,
                        windowX, windowY,
                        sWindowWidth, sWindowHeight,
                        NULL, NULL, instance, NULL);
    if (!sWnd)
        return FALSE;

    ShowWindow(sWnd, cmdShow);

#ifdef UNDER_CE
    SHFullScreen(sWnd,
                 SHFS_HIDETASKBAR | SHFS_HIDESIPBUTTON | SHFS_HIDESTARTICON);
    MoveWindow(sWnd, 0, 0, sWindowWidth, sWindowHeight, TRUE);
#endif

    UpdateWindow(sWnd);

    if (!initEGL(sWnd))
        return FALSE;

    appInit(sWindowWidth, sWindowHeight);

    while (gAppAlive)
    {
        while (PeekMessage(&msg, sWnd, 0, 0, PM_NOREMOVE))
        {
            if (GetMessage(&msg, sWnd, 0, 0))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
                gAppAlive = 0;
        }

        if (gAppAlive)
        {
            appRender(GetTickCount(), sWindowWidth, sWindowHeight);
            checkGLErrors();
            eglSwapBuffers(sEglDisplay, sEglSurface);
            checkEGLErrors();
        }
    }

    appDeinit();
    deinitEGL();

    return 0;
}
