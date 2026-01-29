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

#include "benchmark.h"

#include <base/logging.h>
#include <stdint.h>

#include <expected>
#include <functional>

#include "auto_vectorization.h"
#include "clang_vector.h"
#include "matrix.h"
#include "omp_simd.h"

constexpr uint32_t kNumRuns = 1'000'000;

namespace samples::vectorization {

Vec4 result;

/**
 * Benchmarks a given matrix multiply operation.
 *
 * The multiply is given here as a callback to try to keep Clang from folding,
 * unrolling, or inlining inconsistently across each benchmarked implementation.
 * We want Clang to do as much as possible to optimize *within* the multiply
 * function itself, but inconsistent optimization of the benchmark code itself
 * could skew results.
 *
 * @param position A position vector.
 * @param translation A translation vector.
 * @param func The multiplication function to use.
 * @return The average duration per call in nanoseconds.
 */
[[nodiscard, clang::noinline]] std::chrono::nanoseconds Benchmark(
    Vec4<>& position, Mat4<>& translation,
    std::function<Vec4<>(const Vec4<>&, const Mat4<>&)> func) {
  // TODO: Move to a unit test.
  auto test = func(position, translation);
  auto expected = Vec4{{20, 10, 10, 1}};
  CHECK_EQ(test, expected);

  auto begin = std::chrono::steady_clock::now();

  // This is another attempt to prevent Clang from optimizing the benchmark
  // harness inconsistently.
#pragma clang loop unroll(disable)
  for (auto i = 0U; i < kNumRuns; i++) {
    result = func(position, translation);
  }

  auto end = std::chrono::steady_clock::now();

  return (end - begin) / kNumRuns;
}

[[nodiscard]] std::expected<std::chrono::nanoseconds, BenchmarkError>
BenchmarkMatrixMultiplication(Backend backend) {
  Vec4 position{{10.0f, 10.0f, 10.0f, 1.0f}};
  Mat4 translation{{
      {1.0f, 0.0f, 0.0f, 10.0f},
      {0.0f, 1.0f, 0.0f, 0.0f},
      {0.0f, 0.0f, 1.0f, 0.0f},
      {0.0f, 0.0f, 0.0f, 1.0f},
  }};

  switch (backend) {
    case Backend::kAutoVectorization:
      LOG(INFO) << "Benchmarking auto-vectorization";
      return Benchmark(position, translation, [](Vec4<> p, Mat4<> t) {
        return MultiplyWithAutoVectorization(t, p);
      });
    case Backend::kCxxSimd:
#if __NDK_MAJOR__ >= 31
#error check if std::simd works yet
#endif
      // The libc++ in NDK r27 has only a skeleton implementation of std::simd.
      // Some things we can do without, but it doesn't actually have operator*,
      // which is sort of essential :)
      LOG(INFO) << "Benchmarking std::simd";
      return std::unexpected{BenchmarkError::kNotImplemented};
    case Backend::kClangVector:
      LOG(INFO) << "Benchmarking Clang vectors";
      return Benchmark(position, translation, [](Vec4<> p, Mat4<> t) {
        return MultiplyWithClangVectors(t, p);
      });
    case Backend::kClangMatrix:
      LOG(INFO) << "Benchmarking Clang matrices";
      return Benchmark(position, translation, [](Vec4<> p, Mat4<> t) {
        // This is the default implementation since it's the fastest.
        return t * p;
      });
    case Backend::kOpenMp:
      LOG(INFO) << "Benchmarking OpenMP SIMD";
      return Benchmark(position, translation, [](Vec4<> p, Mat4<> t) {
        return MultiplyWithOpenMP(t, p);
      });
    default:
      return std::unexpected{BenchmarkError::kUnknownBackend};
  }
}

}  // namespace samples::vectorization
