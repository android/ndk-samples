# third_party

This directory contains third-party code used by the samples. Currently this
directory contains only absl, which provides things like `LOG(ERROR)` and
`CHECK()`.

Note that absl does **not** have a stable ABI at the time of writing. As such,
if you are a middleware developer who distributes native libraries in binary
form (whether `.so` or `.a`), you probably cannot use absl. See [Advice for
middleware vendors] for more details.

[Advice for middleware vendors]:
  https://developer.android.com/ndk/guides/middleware-vendors
