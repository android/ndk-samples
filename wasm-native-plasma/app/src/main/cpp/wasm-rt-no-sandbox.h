/*
 * Copyright 2018 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef WASM_RT_NO_SANDBOX_H_
#define WASM_RT_NO_SANDBOX_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
#error "MSC not supported."
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0  // Compatibility with non-clang compilers.
#endif

#if __has_builtin(__builtin_expect)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define LIKELY(x) __builtin_expect(!!(x), 1)
#else
#define UNLIKELY(x) (x)
#define LIKELY(x) (x)
#endif

#if __has_builtin(__builtin_memcpy)
#define wasm_rt_memcpy __builtin_memcpy
#else
#error "No __builtin_memcpy."
#endif

#if __has_builtin(__builtin_unreachable)
#define wasm_rt_unreachable __builtin_unreachable
#else
#error "No __builtin_unreachable."
#endif

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#define WASM_RT_THREAD_LOCAL _Thread_local
#else
#define WASM_RT_THREAD_LOCAL
#endif

#define WASM_RT_NO_RETURN __attribute__((noreturn))

#ifdef __cplusplus
}
#endif

#endif /* WASM_RT_NO_SANDBOX_H_ */
