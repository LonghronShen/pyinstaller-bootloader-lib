#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include <pyinstaller/bootloader/pyi_bootloader.hpp>

std::vector<char> load_all_bytes(const std::string &path) {
  std::ifstream fs(path, std::ios_base::binary);
  std::vector<char> bytes;
  std::copy(std::istreambuf_iterator<char>(fs),
            std::istreambuf_iterator<char>(), std::back_insert_iterator(bytes));
  fs.close();
  return std::move(bytes);
}

int main(int argc, char *argv[]) {
#if _WIN32
  std::string exe_path = "./executable-loader-test-1.exe";
#else
  std::string exe_path = "./main";
#endif

  std::vector<std::string> args;

  if (argc > 1) {
    exe_path = argv[1];
    std::copy(argv + 2, argv + argc, std::back_inserter(args));
  }

  const auto &exe_binary = load_all_bytes(exe_path);

  pyinstaller::bootloader::run_from_buffer(exe_binary, args);

#if defined(_WIN32)
  system("pause");
#endif

  return 0;
}
