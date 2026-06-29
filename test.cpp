#include "src/raon.h"
#include <fstream>
#include <iostream>
#include <sstream>

int main(void) {
  std::ifstream file{"./example.raon"};
  if (!file.is_open()) {
    std::cerr << "Failed to open raon file\n";
    return 1;
  }

  std::stringstream strbuf;
  strbuf << file.rdbuf();

  std::string contents = strbuf.str();

  auto ast = raon_parse(VEC_DEFAULT_ALLOCATOR, contents.data(), contents.size());
  if (!ast) {
    std::cerr << "Failed to parse raon file\n";
    return 1;
  }

  std::cout << "C++ test ran sucessfully.\n";
}
