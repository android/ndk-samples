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
#include "common.hpp"
#include "jni_util.hpp"
#include "native_engine.hpp"

static struct JniSetup _jni_setup = {0};

struct JniSetup* GetJNISetup() {
    if (!_jni_setup.env) {
        _jni_setup.env = NativeEngine::GetInstance()->GetJniEnv();
        _jni_setup.thiz = NativeEngine::GetInstance()->GetAndroidApp()->activity->clazz;
        _jni_setup.clazz = _jni_setup.env->GetObjectClass(_jni_setup.thiz);
    }
    return &_jni_setup;
}


