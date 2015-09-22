
#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <fstream>
#include <memory>
#include <vector>
#include <cstdint>
#include <cwctype>
#include <iostream>

#include "objects.hpp"

namespace little_r {
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
    and_,
    or_,
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
    not_,
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
    question,
    dotdotdot,
    uplus,
    uminus,
  };

  static const char *tok_to_str[] = {
    "undefined",
    "end_of_input",
    "error",
    "str_const",
    "num_const",
    "symbol",
    "<-",
    "\n",
    "NULL",
    "function",
    "=",
    "->",
    "[[",
    "for",
    "in",
    "if",
    "else",
    "while",
    "next",
    "break",
    "repeat",
    ">",
    ">=",
    "<",
    "<=",
    "==",
    "!=",
    "&",
    "|",
    "&&",
    "||",
    "ns_get",
    "ns_get_int",
    "+",
    "-",
    "*",
    "/",
    "%",
    "%%",
    ":",
    "!",
    "{",
    "}",
    "(",
    ")",
    "[",
    "]",
    "**",
    "^",
    "~",
    "$",
    "@",
    ",",
    ";",
    "?",
    "...",
    "+",
    "-",
  };

  inline void indent(int delta = 0) {
    static int depth;
    if (delta == -1) --depth;
    printf("%*s", depth*2, "");
    if (delta == 1) ++depth;
  }

  inline void push_debug(const char *label) {
    indent(); puts(label);
    indent(1); puts("{");
  }

  inline void pop_debug() {
    indent(-1); puts("}");
  }

  class lexer {
  public:
    lexer(std::wistream &istr) : istr(istr) {
      id_.reserve(2048);
      eof_chr = std::char_traits<wchar_t>::eof();
      chr = istr.get();
      value_ = nullptr;
    }

    tt next() {
      skip_whitespace();

      if (chr == '#') {
        id_.resize(0);
        skip_comment();
      }

      id_.resize(0);

      switch (chr) {
        case '>': consume(); tok_ = next_is('=') ? tt::ge : tt::gt; break;
        case '!': consume(); tok_ = next_is('=') ? tt::ne : tt::not_; break;
        case '=': consume(); tok_ = next_is('=') ? tt::eq : tt::eq_assign; break;
        case '&': consume(); tok_ = next_is('&') ? tt::and2 : tt::and_; break;
        case '|': consume(); tok_ = next_is('|') ? tt::or2 : tt::or_; break;
        case '{': consume(); tok_ = tt::lbrace; break;
        case '}': consume(); tok_ = tt::rbrace; break;
        case '(': consume(); tok_ = tt::lparen; break;
        case ')': consume(); tok_ = tt::rparen; break;
        case '[': consume(); tok_ = next_is('[') ? tt::lbb : tt::lbracket; break;
        case ']': consume(); tok_ = tt::rbracket; break;
        case '*': consume(); tok_ = next_is('*') ? tt::star2 : tt::star; break;
        case '+': consume(); tok_ = tt::plus; break;
        case '/': consume(); tok_ = tt::divide; break;
        case '^': consume(); tok_ = tt::caret; break;
        case '~': consume(); tok_ = tt::tilde; break;
        case '$': consume(); tok_ = tt::dollar; break;
        case '@': consume(); tok_ = tt::at; break;
        case ',': consume(); tok_ = tt::comma; break;
        case ';': consume(); tok_ = tt::semicolon; break;
        case '?': consume(); tok_ = tt::question; break;
        case '\n': consume(); tok_ = tt::newline; break;

        case '\'': case '"':  tok_ = parse_string(); break;
        case '`': parse_string(); tok_ = tt::symbol; break;
        case '%': tok_ = parse_special(); break;

        case '.': {
          consume();
          tok_ = is_digit(chr) ? parse_numeric_value() : parse_symbol();
          break;
        }

        case '<': {
          consume();
          tok_ =
            next_is('=') ? tt::le :
            next_is('-') ? tt::left_assign :
            next_is('<') ? next_is('-') ? tt::left_assign : tt::error :
            tt::lt
          ;
          break;
        }

        case '-': {
          consume();
          tok_ =
            next_is('>') ? ( next_is('>') ? tt::right_assign : tt::right_assign ) : 
            next_is('=') ? tt::left_assign :
            tt::minus
          ;
          break;
        }

        case ':': {
          consume();
          tok_ =
            next_is(':') ? ( next_is(':') ? tt::ns_get_int : tt::ns_get ) : 
            next_is('=') ? tt::left_assign :
            tt::colon
          ;
          break;
        }

        default: {
          tok_ =
            is_digit(chr) ? parse_numeric_value() :
            std::iswalpha(chr) ? parse_symbol() :
            chr == eof_chr ? tt::end_of_input :
            tt::error
          ;
          break;
        }
      }
      indent(); std::cout << "[" << id_ << "]\n";
      return tok_;
    }

    tt tok() const { return tok_; }
    std::string id() const { return id_; }
    obj *value() const { return value_; }

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
      switch (id()[0]) {
        case '.': if (is_sym("...")) return tt::dotdotdot; else break;
        case 'b': if (is_sym("break")) return tt::break_; else break;
        case 'e': if (is_sym("else")) return tt::else_; else break;
        case 'F': if (is_sym("FALSE")) return tt::num_const; else break;
        case 'f': if (is_sym("for")) return tt::for_; else if (is_sym("function")) return tt::function; else break;
        case 'i': if (is_sym("if")) return tt::if_; else if (is_sym("in")) return tt::in; else break;
        case 'I': if (is_sym("Inf")) return value_ = obj::null_const(), tt::num_const; else break;
        case 'N':
          if (is_sym("NA")) return value_ = obj::null_const(), tt::num_const;
          if (is_sym("NA_complex_")) return value_ = obj::null_const(), tt::num_const;
          if (is_sym("NA_integer_")) return value_ = obj::null_const(), tt::num_const;
          if (is_sym("NA_real_")) return value_ = obj::null_const(), tt::num_const;
          if (is_sym("Nan")) return value_ = obj::null_const(), tt::num_const;
          if (is_sym("NULL")) return value_ = obj::null_const(), tt::null_const;
          else break;
        case 'n': if (is_sym("next")) return tt::next; else break;
        case 'r': if (is_sym("repeat")) return tt::repeat; else break;
        case 'T': if (is_sym("TRUE")) return tt::num_const; else break;
        case 'w': if (is_sym("while")) return tt::while_; else break;
      }
      value_ = obj::make_symbol(id());
      return tt::symbol;
    }

    bool is_sym(const char *sym) {
      return !std::strcmp(id_.c_str(), sym);
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

      while ((chr <= max_code && is_hex_digit(chr)) || (chr == '.' && expect_dot)) {
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
          } else if (chr == 'x' || chr == 'u' || chr == 'U') {
            int num_digits = chr == 'x' ? 2 : chr == 'u' ? 4 : 8;
            skip();
            for (int i = 0; i != num_digits; ++i) {
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
    std::string id_;
    int chr;
    int eof_chr;
    obj *value_;
  };
}

#endif
