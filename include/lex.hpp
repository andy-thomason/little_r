
#include <string>
#include <fstream>
#include <memory>
#include <vector>
#include <cstdint>
#include <cwctype>
#include <iostream>

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
    comma,
    semicolon,
  };

  class lexer {
  public:
    lexer(std::wistream &istr) : istr(istr) {
      id_.reserve(2048);
      eof_chr = std::char_traits<wchar_t>::eof();
      chr = istr.get();
    }

    tt next() {
      skip_whitespace();

      if (chr == '#') {
        id_.resize(0);
        skip_comment();
      }

      id_.resize(0);

      switch (chr) {
        case '>': consume(); return tok_ = next_is('=') ? tt::ge : tt::gt;
        case '!': consume(); return tok_ = next_is('=') ? tt::ne : tt::not;
        case '=': consume(); return tok_ = next_is('=') ? tt::eq : tt::eq_assign;
        case '&': consume(); return tok_ = next_is('&') ? tt::and2 : tt::and;
        case '|': consume(); return tok_ = next_is('|') ? tt::or2 : tt::or;
        case '{': consume(); return tok_ = tt::lbrace;
        case '}': consume(); return tok_ = tt::rbrace;
        case '(': consume(); return tok_ = tt::lparen;
        case ')': consume(); return tok_ = tt::rparen;
        case '[': consume(); return tok_ = next_is('[') ? tt::lbb : tt::lbracket;
        case ']': consume(); return tok_ = tt::rbracket;
        case '*': consume(); return tok_ = next_is('*') ? tt::star2 : tt::star;
        case '+': consume(); return tok_ = tt::plus;
        case '/': consume(); return tok_ = tt::divide;
        case '^': consume(); return tok_ = tt::caret;
        case '~': consume(); return tok_ = tt::tilde;
        case '$': consume(); return tok_ = tt::dollar;
        case '@': consume(); return tok_ = tt::at;
        case ',': consume(); return tok_ = tt::comma;
        case ';': consume(); return tok_ = tt::semicolon;
        case '\n': consume(); return tok_ = tt::newline;

        case '\'': case '"':  return tok_ = parse_string();
        case '`': parse_string(); return tt::symbol;
        case '%': return tok_ = parse_special();

        case '.': {
          consume();
          return tok_ = is_digit(chr) ? parse_numeric_value() : parse_symbol();
        }

        case '<': {
          consume();
          return tok_ =
            next_is('=') ? tt::le :
            next_is('-') ? tt::left_assign :
            next_is('<') ? next_is('-') ? tt::left_assign : tt::error :
            tt::lt
          ;
        }

        case '-': {
          consume();
          return tok_ =
            next_is('>') ? ( next_is('>') ? tt::right_assign : tt::right_assign ) : 
            next_is('=') ? tt::left_assign :
            tt::minus
          ;
        }

        case ':': {
          consume();
          return tok_ =
            next_is(':') ? ( next_is(':') ? tt::ns_get_int : tt::ns_get ) : 
            next_is('=') ? tt::left_assign :
            tt::colon
          ;
        }

        default: {
          return tok_ =
            is_digit(chr) ? parse_numeric_value() :
            std::iswalpha(chr) ? parse_symbol() :
            chr == eof_chr ? tt::end_of_input :
            tt::error
          ;
        }
      }
    }

    tt tok() const { return tok_; }
    std::wstring id() const { return id_; }

  private:
    static bool is_digit(int c) {
      return c >= '0' && c <= '9';
    }

    static bool is_hex_digit(int c) {
      return (c >= '0' && c <= '9') || ((c&~32) >= 'A' && (c&~32) <= 'F');
    }

    void consume() {
      id_.push_back(chr);
      chr = istr.get();
    }

    void skip() {
      chr = istr.get();
    }

    bool next_is(int c) {
      if (chr == c) {
        consume();
        return true;
      }
      return false;
    }

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
      while (iswalnum(chr) || chr == '.' || chr == '_') {
        consume();
      }
      return tt::symbol;
    }

    tt parse_numeric_value() {
      bool expect_dot = true;
      int exp_code = 'E';
      int max_code = '9';
      if (chr == '0') {
        consume();
        if (chr == 'x' || chr == 'X') {
          consume();
          exp_code = 'P';
          max_code = 'f';
        }
      }

      while (chr <= max_code && is_hex_digit(chr) || (chr == '.' && expect_dot)) {
        expect_dot = chr != '.';
        consume();
      }

      if ((chr & ~32) == exp_code) {
        consume();
        if (chr == '+' || chr == '-') consume();
        if (!is_digit(chr)) return tt::error;
        do {
          consume();
        } while (is_digit(chr));
      } else if (exp_code == 'p' && !expect_dot) {
        // can't have 0x1p8
        return tt::error;
      }
      if (next_is('L')) {
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
      skip();
      while (chr != terminator && chr != eof_chr) {
        if (chr == '\\') {
          skip();
          if (chr == '0') {
            skip();
            for (int i = 0; i != 3 && chr >= '0' && chr <= '7'; ++i) {
              skip();
            }
            if (chr >= '0' && chr <= '7') {
              skip();
              if (chr >= '0' && chr <= '7') {
                skip();
              }
            }
          } else if (chr == 'x') {
            skip();
            for (int i = 0; i != 2; ++i) {
              if (!is_hex_digit(chr)) return tt::error;
              skip();
            }
          } else if (chr == 'u') {
            skip();
            for (int i = 0; i != 4; ++i) {
              if (!is_hex_digit(chr)) return tt::error;
              skip();
            }
          } else if (chr == 'U') {
            skip();
            for (int i = 0; i != 8; ++i) {
              if (!is_hex_digit(chr)) return tt::error;
              skip();
            }
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
            }
            skip();
          }
        } else {
           skip();
        }
      }
      next_is(terminator);
      return tt::str_const;
    }

    void skip_whitespace() {
      id_.resize(0);
      while (chr == ' ' || chr == '\t' || chr == '\f') {
        consume();
      }
    }

    std::wistream &istr;
    tt tok_;
    std::wstring id_;
    int chr;
    int eof_chr;
  };
}
