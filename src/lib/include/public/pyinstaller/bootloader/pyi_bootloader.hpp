#if defined(_MSC_VER) || (defined(__BORLANDC__) && __BORLANDC__ >= 0x0650) ||  \
    (defined(__COMO__) && __COMO_VERSION__ >= 400) /* ??? */                   \
    || (defined(__DMC__) && __DMC__ >= 0x700)      /* ??? */                   \
    || (defined(__clang__) && __clang_major__ >= 3) ||                         \
    (defined(__GNUC__) &&                                                      \
     (__GNUC__ >= 4 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)))
#pragma once
#endif

#ifndef D0040068_303C_4D8E_AA62_EE439FC8C3C4
#define D0040068_303C_4D8E_AA62_EE439FC8C3C4

#include <cstddef>
#include <string>
#include <vector>

namespace pyinstaller {
namespace bootloader {

int run_from_buffer(void *buffer, std::size_t buffer_length, std::int32_t argc,
                    char *argv[], const char *extract_path = nullptr);

std::int32_t run_from_buffer(const std::vector<char> &buffer,
                             const std::vector<std::string> &argv,
                             const std::string &extract_path = "");

} // namespace bootloader
} // namespace pyinstaller

#endif /* D0040068_303C_4D8E_AA62_EE439FC8C3C4 */
