
#include "parser.hpp"

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
          std::wcout << "[" << lex.id() << "]\n";
          if (lex.tok() == tt::error) {
            std::wcout << "error\n";
            return false;
          }
        } while(lex.tok() != tt::end_of_input);
      }

      if (true) {
        std::wfstream istr("../test/R-tests/arith.R");
        lexer lex(istr);
      }
      return true;
    }
  private:
  };
}
