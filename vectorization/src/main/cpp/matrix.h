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

#include <base/logging.h>

#include <array>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <span>

namespace samples::vectorization {

template <size_t Rows, size_t Columns, typename T = float>
class Matrix {
 public:
  Matrix() = default;
  Matrix(T (&&cells)[Rows][Columns]) {
    for (size_t row = 0; row < Rows; row++) {
      for (size_t column = 0; column < Columns; column++) {
        (*this)[row, column] = cells[row][column];
      }
    }
  }

  // Convenience constructor for vectors so callers don't need to use
  // `{{x}, {y}, {z}}`.
  Matrix(const std::array<T, Rows> cells)
    requires(Columns == 1)
      : cells_(cells) {}

  [[nodiscard, clang::always_inline]] constexpr const T* _Nonnull data() const {
    return cells_.data();
  }

  [[nodiscard, clang::always_inline]] constexpr T* _Nonnull data() {
    return cells_.data();
  }

  [[nodiscard, clang::always_inline]] constexpr T& get(size_t row,
                                                       size_t column) {
    return cells_[column * Rows + row];
  }

  [[nodiscard, clang::always_inline]] constexpr const T& get(
      size_t row, size_t column) const {
    return cells_[column * Rows + row];
  }

  [[nodiscard, clang::always_inline]] constexpr T& operator[](size_t row,
                                                              size_t column) {
    return get(row, column);
  }

  [[nodiscard, clang::always_inline]] constexpr const T& operator[](
      size_t row, size_t column) const {
    return get(row, column);
  }

  [[nodiscard, clang::always_inline]] constexpr const std::span<const T> column(
      size_t column) const {
    return std::span{&get(0, column), Rows};
  }

  [[nodiscard, clang::always_inline]] constexpr std::span<T> column(
      size_t column) {
    return std::span{&get(0, column), Rows};
  }

  bool operator==(const Matrix<Rows, Columns, T>& rhs) const {
    return cells_ == rhs.cells_;
  }

  friend std::ostream& operator<<(std::ostream& stream,
                                  const Matrix<Rows, Columns, T>& m) {
    stream << "{" << std::endl;
    for (size_t row = 0; row < Rows; row++) {
      stream << "\t{";
      for (size_t column = 0; column < Columns; column++) {
        stream << m[row, column];
        if (column != Columns - 1) {
          stream << ", ";
        }
      }
      stream << "}" << std::endl;
    }
    stream << "}";
    return stream;
  }

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
  template <size_t OtherRows, size_t OtherColumns>
  Matrix<Rows, OtherColumns, T> operator*(
      const Matrix<OtherRows, OtherColumns, T>& rhs) const
    requires(OtherRows == Columns)
  {
    auto m_lhs =
        __builtin_matrix_column_major_load(data(), Rows, Columns, Rows);
    auto m_rhs = __builtin_matrix_column_major_load(rhs.data(), OtherRows,
                                                    OtherColumns, OtherRows);
    auto m_result = m_lhs * m_rhs;

    Matrix<Rows, OtherColumns, T> result;
    __builtin_matrix_column_major_store(m_result, result.data(), Rows);
    return result;
  }

 private:
  std::array<T, Rows * Columns> cells_ = {};
};

// Enables automatic deduction of definitions like `Matrix m{{1, 0}, {0, 1}}`
// without needing to specify `Matrix<2, 2, int>`.
template <size_t Rows, size_t Columns, typename T>
Matrix(T (&&)[Rows][Columns]) -> Matrix<Rows, Columns, T>;

#if __NDK_MAJOR__ >= 28
#error "TODO: `template <typename T = float>` each of these"
#endif
using Mat4 = Matrix<4, 4>;
using Vec4 = Matrix<4, 1>;

}  // namespace samples::vectorization
