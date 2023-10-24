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

#ifndef WASM_RT_NO_SANDBOX_DECLARATIONS_H_
#define WASM_RT_NO_SANDBOX_DECLARATIONS_H_

#define FUNC_PROLOGUE
#define FUNC_EPILOGUE

#if WABT_BIG_ENDIAN
#error "Big endian not supported in --disable-sandbox mode."
#endif

extern u32 fputs(u64, u64);
extern char stderr;
extern u32 write(u32, u64, u64);

static inline int wasm_rt_trap(const char* msg, u64 n) {
    write(2, (u64)msg, n);
    __builtin_abort();
    return 0;
}

#define TRAP(x) wasm_rt_trap(#x "\n", sizeof(#x)+1)
#define UNREACHABLE (void)wasm_rt_trap("UNREACHABLE\n", 12)

#ifndef ALTERNATIVE_LOAD_STORE

#define DEFINE_LOAD(name, t1, t2, t3)                       \
  static inline t3 name(u64 addr) {                         \
    t1 result;                                              \
    __builtin_memcpy(&result, (const void*)addr, sizeof(t1)); \
    __asm__("" ::"r"(result));                             \
    return (t3)(t2)result;                                  \
  }

#define DEFINE_STORE(name, t1, t2)                     \
  static inline void name(u64 addr, t2 value) {        \
    t1 wrapped = (t1)value;                            \
    __builtin_memcpy((void*)addr, &wrapped, sizeof(t1)); \
  }

#else

// Alternative, simpler, but apparently equally (non-)performant
// versions of the LOAD- and STORE primitives.
#define DEFINE_LOAD(name, t1, t2, t3)             \
  static inline t3 name(u64 addr) {               \
    return (t3)(t2)*(t1*)addr;                    \
  }

#define DEFINE_STORE(name, t1, t2)                \
  static inline void name(u64 addr, t2 value) {   \
    *(t1*)addr = (t1)value;                       \
  }

#endif

DEFINE_LOAD(i32_load, u32, u32, u32)
DEFINE_LOAD(i64_load, u64, u64, u64)
DEFINE_LOAD(f32_load, f32, f32, f32)
DEFINE_LOAD(f64_load, f64, f64, f64)
DEFINE_LOAD(i32_load8_s, s8, s32, u32)
DEFINE_LOAD(i64_load8_s, s8, s64, u64)
DEFINE_LOAD(i32_load8_u, u8, u32, u32)
DEFINE_LOAD(i64_load8_u, u8, u64, u64)
DEFINE_LOAD(i32_load16_s, s16, s32, u32)
DEFINE_LOAD(i64_load16_s, s16, s64, u64)
DEFINE_LOAD(i32_load16_u, u16, u32, u32)
DEFINE_LOAD(i64_load16_u, u16, u64, u64)
DEFINE_LOAD(i64_load32_s, s32, s64, u64)
DEFINE_LOAD(i64_load32_u, u32, u64, u64)
DEFINE_STORE(i32_store, u32, u32)
DEFINE_STORE(i64_store, u64, u64)
DEFINE_STORE(f32_store, f32, f32)
DEFINE_STORE(f64_store, f64, f64)
DEFINE_STORE(i32_store8, u8, u32)
DEFINE_STORE(i32_store16, u16, u32)
DEFINE_STORE(i64_store8, u8, u64)
DEFINE_STORE(i64_store16, u16, u64)
DEFINE_STORE(i64_store32, u32, u64)

#if defined(WASM_RT_ENABLE_SIMD)
#error "SIMD not yet supported with --disable-sandbox."
#endif

#define I32_CLZ(x) ((x) ? __builtin_clz(x) : 32)
#define I64_CLZ(x) ((x) ? __builtin_clzll(x) : 64)
#define I32_CTZ(x) ((x) ? __builtin_ctz(x) : 32)
#define I64_CTZ(x) ((x) ? __builtin_ctzll(x) : 64)
#define I32_POPCNT(x) (__builtin_popcount(x))
#define I64_POPCNT(x) (__builtin_popcountll(x))

// Unchecked variants of division and remainder.
#define DIV_S(ut, min, x, y) ((ut)((x) / (y)))
#define REM_S(ut, min, x, y) \
  ((UNLIKELY((x) == min && (y) == -1)) ? 0 : (ut)((x) % (y)))

#define I32_DIV_S(x, y) DIV_S(u32, INT32_MIN, (s32)x, (s32)y)
#define I64_DIV_S(x, y) DIV_S(u64, INT64_MIN, (s64)x, (s64)y)
#define I32_REM_S(x, y) REM_S(u32, INT32_MIN, (s32)x, (s32)y)
#define I64_REM_S(x, y) REM_S(u64, INT64_MIN, (s64)x, (s64)y)

#define DIVREM_U(op, x, y) ((x)op(y))
#define DIV_U(x, y) DIVREM_U(/, x, y)
#define REM_U(x, y) DIVREM_U(%, x, y)

#define ROTL(x, y, mask) \
  (((x) << ((y) & (mask))) | ((x) >> (((mask) - (y) + 1) & (mask))))
#define ROTR(x, y, mask) \
  (((x) >> ((y) & (mask))) | ((x) << (((mask) - (y) + 1) & (mask))))

#define I32_ROTL(x, y) ROTL(x, y, 31)
#define I64_ROTL(x, y) ROTL(x, y, 63)
#define I32_ROTR(x, y) ROTR(x, y, 31)
#define I64_ROTR(x, y) ROTR(x, y, 63)

// TODO: SIMD

// Excerpt from math.h:
#define NAN __builtin_nanf("")

#define signbit(x) \
    ((sizeof(x) == sizeof(float)) ? __builtin_signbitf(x) \
    : (sizeof(x) == sizeof(double)) ? __builtin_signbit(x) \
    : __builtin_signbitl(x))



// FP helpers:
#define FMIN(x, y)                                                     \
  (UNLIKELY(__builtin_isnan(x))                                        \
       ? NAN                                                           \
       : UNLIKELY(__builtin_isnan(y))                                  \
             ? NAN                                                     \
             : (UNLIKELY((x) == 0 && (y) == 0)) ? (signbit(x) ? x : y) \
                                                : (x < y) ? x : y)

#define FMAX(x, y)                                                     \
  (UNLIKELY(__builtin_isnan(x))                                        \
       ? NAN                                                           \
       : UNLIKELY(__builtin_isnan(y))                                  \
             ? NAN                                                     \
             : (UNLIKELY((x) == 0 && (y) == 0)) ? (signbit(x) ? y : x) \
                                                : (x > y) ? x : y)

#define TRUNC_S(ut, st, ft, min, minop, max, x)                           \
  (UNLIKELY(__builtin_isnan(x))                                           \
       ? TRAP(INVALID_CONVERSION)                                         \
       : (UNLIKELY(!((x)minop(min) && (x) < (max)))) ? TRAP(INT_OVERFLOW) \
                                                     : (ut)(st)(x))

#define I32_TRUNC_S_F32(x) \
  TRUNC_S(u32, s32, f32, (f32)INT32_MIN, >=, 2147483648.f, x)
#define I64_TRUNC_S_F32(x) \
  TRUNC_S(u64, s64, f32, (f32)INT64_MIN, >=, (f32)INT64_MAX, x)
#define I32_TRUNC_S_F64(x) \
  TRUNC_S(u32, s32, f64, -2147483649., >, 2147483648., x)
#define I64_TRUNC_S_F64(x) \
  TRUNC_S(u64, s64, f64, (f64)INT64_MIN, >=, (f64)INT64_MAX, x)

#define TRUNC_U(ut, ft, max, x)                                          \
  (UNLIKELY(__builtin_isnan(x))                                          \
       ? TRAP(INVALID_CONVERSION)                                        \
       : (UNLIKELY(!((x) > (ft)-1 && (x) < (max)))) ? TRAP(INT_OVERFLOW) \
                                                    : (ut)(x))

#define I32_TRUNC_U_F32(x) TRUNC_U(u32, f32, 4294967296.f, x)
#define I64_TRUNC_U_F32(x) TRUNC_U(u64, f32, (f32)UINT64_MAX, x)
#define I32_TRUNC_U_F64(x) TRUNC_U(u32, f64, 4294967296., x)
#define I64_TRUNC_U_F64(x) TRUNC_U(u64, f64, (f64)UINT64_MAX, x)

#define TRUNC_SAT_S(ut, st, ft, min, smin, minop, max, smax, x) \
  (UNLIKELY(__builtin_isnan(x))                                       \
       ? 0                                                      \
       : (UNLIKELY(!((x)minop(min))))                           \
             ? smin                                             \
             : (UNLIKELY(!((x) < (max)))) ? smax : (ut)(st)(x))

#define I32_TRUNC_SAT_S_F32(x)                                            \
  TRUNC_SAT_S(u32, s32, f32, (f32)INT32_MIN, INT32_MIN, >=, 2147483648.f, \
              INT32_MAX, x)
#define I64_TRUNC_SAT_S_F32(x)                                              \
  TRUNC_SAT_S(u64, s64, f32, (f32)INT64_MIN, INT64_MIN, >=, (f32)INT64_MAX, \
              INT64_MAX, x)
#define I32_TRUNC_SAT_S_F64(x)                                        \
  TRUNC_SAT_S(u32, s32, f64, -2147483649., INT32_MIN, >, 2147483648., \
              INT32_MAX, x)
#define I64_TRUNC_SAT_S_F64(x)                                              \
  TRUNC_SAT_S(u64, s64, f64, (f64)INT64_MIN, INT64_MIN, >=, (f64)INT64_MAX, \
              INT64_MAX, x)

#define TRUNC_SAT_U(ut, ft, max, smax, x)                \
  (UNLIKELY(__builtin_isnan(x))                          \
     ? 0                                                 \
     : (UNLIKELY(!((x) > (ft)-1)))                       \
          ? 0                                            \
          : (UNLIKELY(!((x) < (max)))) ? smax : (ut)(x))

#define I32_TRUNC_SAT_U_F32(x) \
  TRUNC_SAT_U(u32, f32, 4294967296.f, UINT32_MAX, x)
#define I64_TRUNC_SAT_U_F32(x) \
  TRUNC_SAT_U(u64, f32, (f32)UINT64_MAX, UINT64_MAX, x)
#define I32_TRUNC_SAT_U_F64(x) TRUNC_SAT_U(u32, f64, 4294967296., UINT32_MAX, x)
#define I64_TRUNC_SAT_U_F64(x) \
  TRUNC_SAT_U(u64, f64, (f64)UINT64_MAX, UINT64_MAX, x)

#define DEFINE_REINTERPRET(name, t1, t2)           \
  static inline t2 name(t1 x) {                    \
    t2 result;                                     \
    __builtin_memcpy(&result, &x, sizeof(result)); \
    return result;                                 \
  }

DEFINE_REINTERPRET(f32_reinterpret_i32, u32, f32)
DEFINE_REINTERPRET(i32_reinterpret_f32, f32, u32)
DEFINE_REINTERPRET(f64_reinterpret_i64, u64, f64)
DEFINE_REINTERPRET(i64_reinterpret_f64, f64, u64)

static float quiet_nanf(float x) {
  uint32_t tmp;
  __builtin_memcpy(&tmp, &x, 4);
  tmp |= 0x7fc00000lu;
  __builtin_memcpy(&x, &tmp, 4);
  return x;
}

static double quiet_nan(double x) {
  uint64_t tmp;
  __builtin_memcpy(&tmp, &x, 8);
  tmp |= 0x7ff8000000000000llu;
  __builtin_memcpy(&x, &tmp, 8);
  return x;
}

static double wasm_quiet(double x) {
  if (UNLIKELY(__builtin_isnan(x))) {
    return quiet_nan(x);
  }
  return x;
}

static float wasm_quietf(float x) {
  if (UNLIKELY(__builtin_isnan(x))) {
    return quiet_nanf(x);
  }
  return x;
}

double floor(double);

static double wasm_floor(double x) {
  if (UNLIKELY(__builtin_isnan(x))) {
    return quiet_nan(x);
  }
  return floor(x);
}

float floorf(float);

static float wasm_floorf(float x) {
  if (UNLIKELY(__builtin_isnan(x))) {
    return quiet_nanf(x);
  }
  return floorf(x);
}

double ceil(double);

static double wasm_ceil(double x) {
  if (UNLIKELY(__builtin_isnan(x))) {
    return quiet_nan(x);
  }
  return ceil(x);
}

float ceilf(float);

static float wasm_ceilf(float x) {
  if (UNLIKELY(__builtin_isnan(x))) {
    return quiet_nanf(x);
  }
  return ceilf(x);
}

double trunc(double);

static double wasm_trunc(double x) {
  if (UNLIKELY(__builtin_isnan(x))) {
    return quiet_nan(x);
  }
  return trunc(x);
}

float truncf(float);

static float wasm_truncf(float x) {
  if (UNLIKELY(__builtin_isnan(x))) {
    return quiet_nanf(x);
  }
  return truncf(x);
}

float nearbyintf(float);

static float wasm_nearbyintf(float x) {
  if (UNLIKELY(__builtin_isnan(x))) {
    return quiet_nanf(x);
  }
  return nearbyintf(x);
}

double nearbyint(double);

static double wasm_nearbyint(double x) {
  if (UNLIKELY(__builtin_isnan(x))) {
    return quiet_nan(x);
  }
  return nearbyint(x);
}

float fabsf(float) __attribute_const__;

static float wasm_fabsf(float x) {
  if (UNLIKELY(__builtin_isnan(x))) {
    uint32_t tmp;
    __builtin_memcpy(&tmp, &x, 4);
    tmp = tmp & ~(1UL << 31);
    __builtin_memcpy(&x, &tmp, 4);
    return x;
  }
  return fabsf(x);
}

double fabs(double) __attribute_const__;

static double wasm_fabs(double x) {
  if (UNLIKELY(__builtin_isnan(x))) {
    uint64_t tmp;
    __builtin_memcpy(&tmp, &x, 8);
    tmp = tmp & ~(1ULL << 63);
    __builtin_memcpy(&x, &tmp, 8);
    return x;
  }
  return fabs(x);
}

double sqrt(double);

static double wasm_sqrt(double x) {
  if (UNLIKELY(__builtin_isnan(x))) {
    return quiet_nan(x);
  }
  return sqrt(x);
}

float sqrtf(float);

static float wasm_sqrtf(float x) {
  if (UNLIKELY(__builtin_isnan(x))) {
    return quiet_nanf(x);
  }
  return sqrtf(x);
}

extern u32 open(u64, u32, ...);

inline static u32 wasm_open2(u64 p, u32 f) {
  return open(p, f);
}

inline static u32 wasm_open3(u64 p, u32 f, u32 m) {
  return open(p, f, m);
}

#endif /* WASM_RT_NO_SANDBOX_DECLARATIONS_H_ */
