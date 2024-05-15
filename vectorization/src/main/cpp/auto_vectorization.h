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

#include <stdint.h>

#include "matrix.h"

namespace samples::vectorization {

/**
 * Multiplies two compatible matrices and returns the result.
 *
 * @tparam T The type of each matrix cell.
 * @tparam M The number of rows in the left operand and the result.
 * @tparam N The number of columns in the left operand, and the rows in the
 * right operand.
 * @tparam P The number of columns in the right operand and the result.
 * @param lhs The left operand.
 * @param rhs The right operand.
 * @return The result of lhs * rhs.
 */
template <typename T, size_t M, size_t N, size_t P>
Matrix<M, P, T> MultiplyWithAutoVectorization(const Matrix<M, N, T>& lhs,
                                              const Matrix<N, P, T>& rhs) {
  // This may look like an unfair benchmark because this implementation uses the
  // less vector friendly one than the others, however, using the vector
  // friendly algorithm here actually made performance worse.
  //
  // This is a good illustration of why it's important to benchmark your own
  // code and not rely on what someone else tells you about which works best: it
  // depends.
  //
  // It's probably also worth mentioning that if what you need is *consistent*
  // performance across compiler versions, the only real choice you have is
  // writing assembly. Even the instruction intrinsics (at least for Neon) are
  // subject to the compiler's instruction selection. That will be overkill for
  // most users, since it's substantially more difficult to write and maintain,
  // but is how you'll see some code bases deal with this (codecs in particular
  // are willing to make that trade-off).
  Matrix<M, P, T> result;
  for (auto i = 0U; i < M; i++) {
    for (auto j = 0U; j < P; j++) {
      T sum = {};
      for (auto k = 0U; k < N; k++) {
        sum += lhs.get(i, k) * rhs[k, j];
      }
      result[i, j] = sum;
    }
  }
  return result;
}

}  // namespace samples::vectorization
