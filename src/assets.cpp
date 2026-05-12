#include "assets.hpp"

#include <cstdio>

namespace ck {

const char* AssetPath(const char* rel) {
  thread_local char buf[512];
  std::snprintf(buf, sizeof(buf), "assets/%s", rel);
  return buf;
}

}  // namespace ck
