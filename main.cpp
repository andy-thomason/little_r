
#include <string>
#include <fstream>
#include <memory>
#include <vector>
#include <cstdint>
#include <cwctype>
//#include <boost/interprocess>

namespace little_r {
  static const char *object_names[] = {
    "NULL",    // NULL
    "symbol",    //a variable name
    "pairlist",    //a pairlist object (mainly internal)
    "closure",    //a function
    "environment",    //an environment
    "promise",    //an object used to implement lazy evaluation
    "language",    //an R language construct
    "special",    //an internal function that does not evaluate its arguments
    "builtin",    //an internal function that evaluates its arguments
    "char",    //a ‘scalar’ string object (internal only) ***
    "logical",    //a vector containing logical values
    "integer",    //a vector containing integer values
    "double",    //a vector containing real values
    "complex",    //a vector containing complex values
    "character",    //a vector containing character values
    "...",    //the special variable length argument ***
    "any",    //a special type that matches all types: there are no objects of this type
    "expression",    //an expression object
    "list",    //a list
    "bytecode",    //byte code (internal only) ***
    "externalptr",    //an external pointer object
    "weakref",    //a weak reference object
    "raw",    //a vector containing bytes
    "S4",    //an S4 object which is not a simple object
  };

  enum class obj_types {
    obj_NULL,
    obj_symbol,
    obj_pairlist,
    obj_closure,
    obj_environment,
    obj_promise,
    obj_language,
    obj_special,
    obj_builtin,
    obj_char,
    obj_logical,
    obj_integer,
    obj_double,
    obj_complex,
    obj_character,
    obj_dot_dot_dot,
    obj_any,
    obj_expression,
    obj_list,
    obj_bytecode,
    obj_externalptr,
    obj_weakref,
    obj_raw,
    obj_S4,
  };

  class obj;

  typedef std::pair<std::string, obj *> pair;

  class obj {
  public:
    virtual obj_types type() const = 0;
    virtual size_t size() const = 0;
    virtual std::string to_string() const = 0;
    virtual ~obj() {
    }

    obj &attr(const std::string &name, obj &object) {
      for (auto p : attributes_) {
        if (p.first == name) { p.second = &object; return *this; }
      }
      attributes_.push_back(pair(name, &object));
      return *this;
    }
  private:
    std::vector<pair> attributes_;
    size_t ref_count_ = 0;
  };

  class symbol : public obj {
  public:
    symbol(const char *name) : name_(name) {
    }

    obj_types type() const { return obj_types::obj_symbol; }
    size_t size() const { return 1; }
    std::string to_string() const { return name_; }

  private:
    std::string name_;
  };

  class environment : public obj {
  public:
  private:
    std::vector<pair> frame_;
    environment *enclosure_;
  };

  class list {
  public:
    obj_types type() const { return obj_types::obj_list; }
    size_t size() const { return data_.size(); }
  private:
    std::vector<obj *> data_;
    std::string to_string() const { return "[]"; }
  };

  enum class tt {
    undefined,
    end_of_input,
    error,
    str_const,
    num_const,
    symbol,
    left_assign,
    newline,
    null_const,
    function,
    eq_assign,
    right_assign,
    lbb,
    for_,
    in,
    if_,
    else_,
    while_,
    next,
    break_,
    repeat,
    gt,
    ge,
    lt,
    le,
    eq,
    ne,
    and,
    or,
    and2,
    or2,
    ns_get,
    ns_get_int,
    plus,
    minus,
    star,
    divide,
    modulus,
    special,
    colon,
    not,
    lbrace,
    rbrace,
    lparen,
    rparen,
    lbracket,
    rbracket,
    star2,
    caret,
    tilde,
    dollar,
    at,
  };

  class lexer {
  public:
    lexer(std::istream &istr) : istr(istr) {
      id.reserve(2048);
      eof_chr = std::char_traits<wchar_t>::eof();
      chr = istr.get();
      chr = istr.get();
      chr = istr.get();
      chr = istr.get();
    }

    tt next() {
      skip_whitespace();

      if (chr == '#') {
        id.resize(0);
        skip_comment();
      }

      id.resize(0);

      switch (chr) {
        case '.': tok = is_digit(istr.peek()) ? parse_numeric_value() : parse_symbol();
        case '\'': case '"': case '`': return tok = parse_string();
        case '%': return tok = parse_special();

        case '<': {
          return tok =
            next_is('=') ? tt::le :
            next_is('-') ? tt::left_assign :
            next_is('<') ? next_is('-') ? tt::left_assign : tt::error :
            tt::lt
          ;
        }

        case '-': {
          return tok =
            next_is('>') ? ( next_is('>') ? tt::right_assign : tt::right_assign ) : 
            next_is('=') ? tt::left_assign :
            tt::minus
          ;
        }

        case ':': {
          return tok =
            next_is(':') ? ( next_is(':') ? tt::ns_get_int : tt::ns_get ) : 
            next_is('=') ? tt::left_assign :
            tt::colon
          ;
        }

        case '>': return tok = next_is('=') ? tt::ge : tt::gt;
        case '!': return tok = next_is('=') ? tt::ne : tt::not;
        case '=': return tok = next_is('=') ? tt::eq : tt::eq_assign;
        case '&': return tok = next_is('&') ? tt::and2 : tt::and;
        case '|': return tok = next_is('|') ? tt::or2 : tt::or;
        case '{': return tok = tt::lbrace;
        case '}': return tok = tt::rbrace;
        case '(': return tok = tt::lparen;
        case ')': return tok = tt::rparen;
        case '[': return tok = next_is('[') ? tt::lbb : tt::lbracket;
        case ']': return tok = tt::rbracket;
        case '*': return tok = next_is('*') ? tt::star2 : tt::star;
        case '+': return tok = tt::plus;
        case '/': return tok = tt::divide;
        case '^': return tok = tt::caret;
        case '~': return tok = tt::tilde;
        case '$': return tok = tt::dollar;
        case '@': return tok = tt::at;

        default: {
          return tok =
            is_digit(chr) ? parse_numeric_value() :
            std::iswalpha(chr) ? parse_symbol() :
            tt::error
          ;
        }
      }
    }

    operator tt() { return tok; }

  private:
    static bool is_digit(int c) {
      return c >= '0' && c <= '9';
    }

    void consume() {
      id.push_back(chr);
      chr = istr.get();
      peek = istr.peek();
    }

    bool next_is(int c) {
      if (istr.peek() == c) {
        consume();
        return true;
      }
      return false;
    }

    /*bool next_is(int c, int c2) {
      if (peek == c || peek == c2) {
        consume();
        return true;
      }
      return false;
    }*/

    void skip_comment() {
      while (chr == '#') {
        do {
          consume();
        } while(chr != eof_chr && chr != '\n');
        if (chr != eof_chr) {
          consume();
        }
      }
    }

    tt parse_symbol() {
      consume();
      while (iswalnum(peek) || peek == '.' || peek == '_') {
        consume();
      }
      return tt::symbol;
    }

    tt parse_numeric_value() {
      bool expect_dot = true;
      if (chr == '0' && (peek == 'x' || peek == 'X')) {
        //
      } else {
        while (is_digit(peek) || (peek == 'e' || peek == 'E') || (peek == '.' && expect_dot)) {
          expect_dot = peek != '.';
          consume();
        }
        if (peek == 'e' || peek == 'E') {
          consume();
          if (!is_digit(peek)) return tt::error;
          do {
            consume();
          } while (is_digit(peek));
        }
        if (next_is('L')) {
          //ival = std::atoll(id.c_str());
        } else {
          //dval = std::atof(id.c_str());
        }
      }
      return tt::num_const;
    }

    tt parse_special() {
      consume();
      while (chr != '%') {
        if (chr == eof_chr) return tt::error;
        consume();
      }
      consume();
      return tt::special;
    }

    tt parse_string() {
      int terminator = chr == '"' ? '"' : '\'';
      consume();
      while (chr != terminator && chr != eof_chr) {
        if (chr == '\\') {
          consume();
          if (chr == '0') {
          } else {
            switch (chr) {
              case 'a': chr = '\a'; break;
              case 'b': chr = '\b'; break;
              case 'f': chr = '\f'; break;
              case 'n': chr = '\n'; break;
              case 'r': chr = '\r'; break;
              case 't': chr = '\t'; break;
              case 'v': chr = '\v'; break;
              case '\\': break;
              case '"': break;
              case '\'': break;
              case '`': break;
              case ' ': break;
              case '\n': break;
              default: {
                throw std::runtime_error("invalid escape char");
              }
              consume();
            }
          }
        } else {
           consume();
        }
      }
      next_is(terminator);
      return tt::str_const;
    }

    void skip_whitespace() {
      while (chr == ' ' || chr == '\t' || chr == '\f') {
        consume();
      }
    }

    std::istream &istr;
    tt tok;
    std::string id;
    double as_double;
    std::int64_t as_int;
    int chr;
    int peek;
    int eof_chr;
  };

  class little_r {
  public:
    little_r() {
      lexer lex(std::ifstream("/Users/Andy/Documents/GitHub/r-source/tests/arith.R"));
      do {
        lex.next();
      } while(lex != tt::error && lex != tt::end_of_input);
    }
  };
}

int main() {
  little_r::little_r r{};
}
