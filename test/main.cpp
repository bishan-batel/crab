#include "preamble.hpp"
#include "ref.hpp"
#include <vector>

void bruh() {}

void huh() { return bruh(); }

int main() {
  i32 a = 42;

  std::vector<RefMut<i32>> vec = {a, a, a, a};

  return 0;
}
