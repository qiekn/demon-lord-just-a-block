#pragma once

// Compile-time asset path prefix. Callers pass `CK_ASSET("fonts/...")` to
// raylib's C string APIs without allocating, and a single edit to this file
// can re-root the asset tree later (e.g. for an installed-binary layout that
// resolves against the executable directory). Keep this header std-free so
// TUs that mix `import std;` with `#include "assets.hpp"` stay clean.

#define CK_ASSET(rel) "assets/" rel

namespace ck {

// Runtime variant for paths only known at call time (e.g. concatenating with
// a dynamic level id). Result lives in a thread-local buffer; copy out if you
// need to retain it past the next call.
const char* AssetPath(const char* rel);

}  // namespace ck
