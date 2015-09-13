
#include "little_r.hpp"

int main() {
  little_r::little_r r{};

  if (!r.unit_test()) {
    return 1;
  }
  return 0;
}
