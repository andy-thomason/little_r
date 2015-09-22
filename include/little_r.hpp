
#include "parser.hpp"

#include <sstream>

namespace little_r {
  class little_r {
  public:
    little_r() {
    }

    bool unit_test() {
      if (false) {
        std::wfstream istr("../test/R-tests/arith.R");
        lexer lex(istr);
        do {
          lex.next();
          std::cout << "[" << lex.id() << "]\n";
          if (lex.tok() == tt::error) {
            std::cout << "error\n";
            return false;
          }
        } while(lex.tok() != tt::end_of_input);
      }

      if (false) {
        std::wfstream istr("../test/R-tests/arith.R");
        parser p(istr);
      }

      if (true) {
        std::wistringstream istr(L"1 + b");
        parser p(istr);
      }

      return true;
    }
  private:
  };
}
