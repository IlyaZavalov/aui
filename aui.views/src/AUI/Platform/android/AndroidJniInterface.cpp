// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.


#include <jni.h>
#include <AUI/Thread/AThread.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Logging/ALogger.h>

static void runOnGLThread(std::function<void()> callback) {
    AUI_NULLSAFE(AWindow::current())->getThread()->enqueue(std::move(callback));
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_AuiView_handleRedraw(JNIEnv *env, jclass clazz) {
    AUI_NULLSAFE(AThread::current()->getCurrentEventLoop())->loop();
    AUI_NULLSAFE(dynamic_cast<AWindow*>(AWindow::current()))->AWindow::redraw();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_AuiView_handleResize(JNIEnv *env, jclass clazz, jint width, jint height) {
    runOnGLThread([=] {
        AUI_NULLSAFE(AWindow::current())->setSize({width, height});
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_AuiView_handleLongPress(JNIEnv *env, jclass clazz, jint x, jint y) {
    runOnGLThread([=] {
        AUI_NULLSAFE(AWindow::current())->onGesture({x, y}, ALongPressEvent{});
    });
}

static glm::ivec2 gestureOriginPos{0, 0};
static glm::ivec2 scrollPrevValue{0, 0};

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_AuiView_handlePointerButtonDown(JNIEnv *env, jclass clazz, jint x,
                                                          jint y, jint pointerId) {
    ALogger::info("Test") << "down " << x << ", " << y << " " << pointerId;
    scrollPrevValue = gestureOriginPos = {x, y};
    runOnGLThread([=] {
        AUI_NULLSAFE(AWindow::current())->onPointerPressed({{x, y}, APointerIndex::finger(pointerId)});
    });
}
extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_AuiView_handlePointerButtonUp(JNIEnv *env, jclass clazz, jint x,
                                                        jint y, jint pointerId) {
    ALogger::info("Test") << "up " << x << ", " << y << " " << pointerId;
    runOnGLThread([=] {
        AUI_NULLSAFE(AWindow::current())->onPointerReleased({{x, y}, APointerIndex::finger(pointerId)});
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_aui_android_AuiView_handlePointerMove(JNIEnv *env, jclass clazz, jint x,
                                                    jint y, jint pointerId) {
    ALogger::info("Test") << "move " << x << ", " << y << " " << pointerId;
    runOnGLThread([=] {
        AUI_NULLSAFE(AWindow::current())->onPointerMove({x, y}, APointerIndex::finger(pointerId));
    });
}
