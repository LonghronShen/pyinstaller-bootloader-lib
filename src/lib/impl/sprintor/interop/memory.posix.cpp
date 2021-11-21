#if !defined(WIN32)
#include <cstdio>

#include <sprintor/interop/memory.hpp>

namespace sprintor {
namespace interop {
namespace memory {

FILE *fmemopen(void *buffer, std::size_t buffer_length, const char *mode) {
  return ::fmemopen(buffer, buffer_length, "r");
}

} // namespace memory
} // namespace interop
} // namespace sprintor
#endif