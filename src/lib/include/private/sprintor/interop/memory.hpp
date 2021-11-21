#if defined(_MSC_VER) || (defined(__BORLANDC__) && __BORLANDC__ >= 0x0650) ||  \
    (defined(__COMO__) && __COMO_VERSION__ >= 400) /* ??? */                   \
    || (defined(__DMC__) && __DMC__ >= 0x700)      /* ??? */                   \
    || (defined(__clang__) && __clang_major__ >= 3) ||                         \
    (defined(__GNUC__) &&                                                      \
     (__GNUC__ >= 4 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)))
#pragma once
#endif

#ifndef BEDA5BDC_EC44_4DA1_B9C9_4B1649E5E5EE
#define BEDA5BDC_EC44_4DA1_B9C9_4B1649E5E5EE

#include <cstddef>
#include <cstdio>

namespace sprintor {
namespace interop {
namespace memory {

FILE *fmemopen(void *buffer, std::size_t buffer_length, const char *mode);

}
} // namespace interop
} // namespace sprintor

#endif /* BEDA5BDC_EC44_4DA1_B9C9_4B1649E5E5EE */
