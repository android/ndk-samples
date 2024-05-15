# Vectorization

This sample shows how to implement matrix multiplication using various
vectorization approaches.

Note: You should not reuse this matrix library in your application. It was not
written to be useful beyond the scope of this demo. If you're looking for a
matrix library, you probably want [GLM] for graphics applications, or a linear
algebra library such as BLAS for compute applications.

The sample app will benchmark each implementation and display the average run
time over 1,000,000 runs. The goal of this sample is to illustrate the trade-
offs of each implementation in terms of flexibility, readability, and
performance.

Given the relatively small problem size used here (4x4 matrices and vec4s), the
best performing implementations in this sample are the ones that can best
improve over the naive implementation without large set up costs. You should not
take the results of this sample as authoritative: if performance is important to
you, you **must** benchmark your code for workloads realistic for your app.

If you're not familiar with it [Godbolt] is an invaluable tool for examining
compiler optimizer behavior. You could also use `$NDK_BIN/clang -S -O2 -o -`
from the command line for a local workflow.

## Implementations

This sample contains the following implementations. Each of their trade-offs are
discussed briefly, but as mentioned above, you should not rely on the
performance results measured here to make a decision for your app.

### Auto-vectorization

See [auto_vectorization.h] for the implementation.

This implementation is written in generic C++ and contains no explicit SIMD. The
only vectorization that will be performed is Clang's auto-vectorization. This
makes for the most portable code and readable code, but at the cost of
performance.

See https://llvm.org/docs/Vectorizers.html for Clang's docs about
auto-vectorization.

### std::simd

This isn't actually available yet. It's an experimental part of the C++ standard
and is in development in libc++, but NDK r27 happened to catch it right in the
middle of a rewrite, so it's not currently usable.

See https://en.cppreference.com/w/cpp/experimental/simd/simd.

### Clang vectors

See [clang_vector.h] for the implementation.

This implementation uses Clang's generic vector types. This code is mostly as
portable as the auto-vectorization implementation, with the only caveat being
that it is limited by the width of the vector registers for the target hardware.
To deal with problems that don't fit in the target's vector registers, you would
need to either alter the algorithm to use a [partitioned matrix multiply], or
use Scalable Vector Extensions (AKA [SVE]).

However, the benefit of the portability trade-off is that this does outperform
the auto-vectorization implementation.

See
https://clang.llvm.org/docs/LanguageExtensions.html#vectors-and-extended-vectors.

### Clang matrices

See [matrix.h] for the implementation. This is the default implementation for
`Matrix::operator*`, so unlike the others that file contains the rest of the
`Matrix` class as well.

This implementation uses Clang's built-in matrix type. This is an experimental
feature in Clang, but it has the simplest code (because some kind Clang person
wrote the hard part) and performs the best by a wide margin. There are
implementation defined limits on the size of the matrix, but within those limits
the code is as portable as the auto-vectorization implementation. The docs say
the feature is still under development and subject to change, so be wary of
using this in production, and definitely don't use these types as part of your
ABI.

See https://clang.llvm.org/docs/LanguageExtensions.html#matrix-types for more
details.

### OpenMP SIMD

See [omp_simd.h] for the implementation.

This implementation uses OpenMP's SIMD directive. For some reason this
under-performs even the auto-vectorized implementation. There are a lot of
additional specifiers that can be added to the simd directive that would maybe
improve this implementation. Patches welcome :)

See https://www.openmp.org/spec-html/5.0/openmpsu42.html for more information.

## Alternatives not shown here

There are other approaches that could be used that aren't shown here.

### Neon

A Neon implementation would be nearly identical to the one in [clang_vector.h].
The only difference is how the vector type is specified. A lot of older Neon
sample code looks substantially different because it uses the Neon intrinsics
defined in `arm_neon.h`, but if you look at how the intrinsics in that file are
defined, all they actually do (for a little endian system, and Android does not
support big endian, so we can ignore that caveat) is use the `*` operator and
leave the correct instruction selection up to Clang.

In other words, you should probably never use the Neon-specific approach. The
generated code should be identical to code written with Clang's arch-generic
vectors. If you rewrite the [clang_vector.h] implementation to use Neon's
`float32x4_t` instead of the Clang vector, the results are identical.

### SVE

[SVE] scales SIMD to arbitrarily sized vectors, and the C extensions, while
making for less concise code than is needed for a constrained vector size like
we have here, handle windowing of data to fit the hardware vector size for you.
For problems like the small matrix multiply we do here, it's overkill. For
portability across various vector widths for the Arm CPUs that support SVE, it
can reduce the difficulty of writing SIMD code.

In practice though, most mobile hardware has no SVE support, and all the
hardware that does support SVE only has 128-bit vector lengths. In practice,
SVE's portability is currently only between mobile and server (because server
SVE implementations do sometimes have larger vector lengths).

### GPU acceleration

GPU acceleration is a better fit for large data sets. That approach isn't shown
here because it's substantially more code to set up the GPU for this
computation, and our data size is so small that the cost of GPU initialization
and streaming the data to the GPU is likely to make that a net-loss. If you want
to learn more about GPU compute, see https://vulkan-tutorial.com/Compute_Shader,
https://www.khronos.org/opengl/wiki/Compute_Shader, and
https://www.khronos.org/opencl/ (while OpenCL is not guaranteed to be available
for all Android devices, it is a very common OEM extension).

## Function multi-versioning

There are two compiler attributes that can be helpful for targeting specific
hardware features when optimizing hot code paths: [target] and [target_clones],
both of which may be referred to as "function multiversioning" or "FMV". Each
solves a slightly different but related problem.

The `target` attribute makes it easier to write multiple implementations for a
function that should be selected based on the runtime hardware. If benchmarking
shows that one implementation performs better on armv8.2 and a different
implementation performs better on armv8.0 (see the docs for more details on
specific targeting capabilities), you can write the function twice, annotate
them with the appropriate `__attribute__((target(...)))` tag, and the compiler
will auto-generate the code to select the best-fitting implementation at runtime
(it uses ifuncs under the hood, so the branch is resolved once at library load
time rather than for each call).

The `target_clones` attribute, on the other hand, allows you to write the
function once but instruct the compiler to generate multiple variants of the
function for each requested target. This means that, for example, if you've
requested both `default` and `armv8.2`, the compiler will generate a default
implementation compatible with all Android devices, as well as a second
implementation that uses instructions available in armv8.2 but not available in
the base armv8 ABI. As with the `target` attribute, Clang will automatically
select the best-fitting implementation at runtime. Using `target_clones` is the
same as using `target` with identical function bodies.

Note that with both of these approaches, testing becomes more difficult because
you will need a greater variety of hardware to test each code path. If you're
already doing fine grained targeting like this, that isn't a new problem, and
using one or both of these attributes may help you simplify your implementation.

Neither of these techniques are shown in this sample. We don't have access to
enough hardware to benchmark or verify multiple implementations, and (as of NDK
r27, at least), Clang doesn't support `target_clones` on templated functions.

[auto_vectorization.h]: src/main/cpp/auto_vectorization.h

[clang_vector.h]: src/main/cpp/clang_vector.h

[GLM]: https://github.com/g-truc/glm

[Gobolt]: https://godbolt.org/

[matrix.h]: src/main/cpp/matrix.h

[neon.h]: src/main/cpp/neon.h

[omp_simd.h]: src/main/cpp/omp_simd.h

[partitioned matrix multiply]: https://en.wikipedia.org/wiki/Block_matrix#Multiplication

[SVE]: https://developer.arm.com/Architectures/Scalable%20Vector%20Extensions

[target_clones]: https://clang.llvm.org/docs/AttributeReference.html#target-clones

[target]: https://clang.llvm.org/docs/AttributeReference.html#target
