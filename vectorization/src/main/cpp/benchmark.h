/*
 * Copyright (C) 2024 The Android Open Source Project
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

#pragma once

#include <chrono>
#include <expected>
#include <optional>

namespace samples::vectorization {

/**
 * The available backends for matrix multiplication shown in this sample.
 */
enum class Backend : uint8_t {
  /// Auto-vectorization only.
  kAutoVectorization = 0,

  /// C++ std::simd.
  kCxxSimd = 1,

  /// Clang's arch-generic vector types.
  kClangVector = 2,

  /// Clang's built-in matrix type.
  kClangMatrix = 3,

  /// OpenMP SIMD.
  kOpenMp = 4,
};

/// Errors returned by BenchmarkMatrixMultiplication.
enum class BenchmarkError : int8_t {
  /// Indicates that the requested backend has not yet been implemented.
  kNotImplemented = -1,
  /// Indicates that the requested backend isn't supported for the device.
  kNotSupported = -2,
  /// Indicates that an unknown backend was requested.
  kUnknownBackend = -3,
};

/**
 * Benchmarks the given matrix multiply backend.
 *
 * The chosen backend will run a predetermined number of times and return the
 * average execution time.
 *
 * @param backend The backend to benchmark.
 * @return The average execution time, or an error code.
 */
[[nodiscard]] std::expected<std::chrono::nanoseconds, BenchmarkError>
BenchmarkMatrixMultiplication(Backend backend);

}  // namespace samples::vectorization
