
#ifndef OBJECTS_HPP
#define OBJECTS_HPP

#include <cstring>

namespace little_r {
  enum class ot {
    nil = 0, // nil  = null
    symbol = 1, // symbols
    list = 2, // lists of dotted pairs
    closure = 3, // closures
    env = 4, // environments
    promise = 5, // promises: [un]evaluated closure arguments
    lang = 6, // language constructs (special lists)
    special = 7, // special forms
    builtin = 8, // builtin non-special forms
    chr = 9, // "scalar" string type (internal only)
    logical = 10, // logical vectors
    integer = 13, // integer vectors
    real = 14, // real variables
    complex = 15, // complex variables
    str = 16, // string vectors
    dot = 17, // dot-dot-dot object
    any = 18, // make "any" args work
    vec = 19, // generic vectors
    expr = 20, // expressions vectors
    bytecode = 21, // byte code
    extptr = 22, // external pointer
    weakref = 23, // weak reference
    raw = 24, // raw bytes
    s4 = 25, // s4 non-vector

    news = 30,   // fresh node created in new page
    frees = 31,   // node released by gc

    funs = 99 // closure or builtin
  };

  struct sxpinfo_struct {
      ot type      :  5;
      unsigned int obj   :  1;
      unsigned int named :  2;
      unsigned int gp    : 16;
      unsigned int mark  :  1;
      unsigned int debug :  1;
      unsigned int trace :  1;  /* functions and memory tracing */
      unsigned int spare :  1;  /* currently unused */
      unsigned int gcgen :  1;  /* old generation number */
      unsigned int gccls :  3;  /* node class */
  };

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

  class obj;

  // Record to use when using the original R C code stuctures.
  struct SEXPREC {
    struct vecsxp_struct {
      size_t length;
      size_t truelength;
    };

    struct primsxp_struct {
      int offset;
    };

    struct symsxp_struct {
      obj *pname;
      obj *value;
      obj *internal;
    };

    struct listsxp_struct {
      obj *carval;
      obj *cdrval;
      obj *tagval;
    };

    struct envsxp_struct {
      obj *frame;
      obj *enclos;
      obj *hashtab;
    };

    struct closxp_struct {
      obj *formals;
      obj *body;
      obj *env;
    };

    struct promsxp_struct {
      obj *value;
      obj *expr;
      obj *env;
    };

    sxpinfo_struct sxpinfo;
    obj *attrib;
    obj *gengc_next_node;
    obj *gengc_prev_node;

    union {
       primsxp_struct primsxp;
       symsxp_struct symsxp;
       listsxp_struct listsxp;
       envsxp_struct envsxp;
       closxp_struct closxp;
       promsxp_struct promsxp;
    };
  };

  typedef SEXPREC *SEXP;

  typedef obj *objref;

  std::ostream &operator <<(std::ostream &os, const obj &rhs);

  // equivalent to reference compiler's SEXP
  class obj : private SEXPREC {
  public:
    obj(ot type=ot::list) {
      init(type, null_const(), null_const());
    }

    obj(ot type, objref elem1) {
      init(type, elem1, null_const());
    }

    obj(ot type, objref elem1, objref elem2) {
      init(type, elem1, elem2);
    }

    template <typename... elems>
    obj(ot type, objref head, elems... tail) {
      init(type, head, new obj(type, tail...));
    }

    static objref null_const() {
      static const SEXPREC value;
      return objref(&value);
    }

    void *operator new(size_t size) {
      // stand-in allocator
      return malloc(size);
    }

    void operator delete(void *) {
    }

    void *operator new(size_t size, size_t extra) {
      // stand-in allocator
      return malloc(size + extra);
    }

    void operator delete(void *, size_t) {
    }

    ot type() const { return sxpinfo.type; }
    objref head() const { return listsxp.carval; }
    objref tail() const { return listsxp.cdrval; }
    objref tag() const { return listsxp.tagval; }

    obj &set_type(ot value) { sxpinfo.type = value; return *this; }
    obj &set_head(objref value) { listsxp.carval = value; return *this; }
    obj &set_tail(objref value) { listsxp.cdrval = value; return *this; }
    obj &set_tag(objref value) { listsxp.tagval = value; return *this; }

    objref last() {
      objref p = this;
      while (tail() != null_const()) {
        p = tail();
      }
      return p;
    }

    objref append(objref val) {
      objref t = tail();
      objref extra = new obj(ot::list, val);
      t->set_tail(extra);
      return extra;
    }

    char *chr_data() { return (char*)this + sizeof(obj); }
    const char *chr_data() const { return (const char*)this + sizeof(obj); }

    static objref make_string(const std::string &str) {
      objref res = new (str.size() + 1) obj(ot::chr);
      memcpy(res->chr_data(), str.c_str(), str.size() + 1);
      return res;
    }

    static objref make_symbol(const std::string &str) {
      objref res = new (str.size() + 1) obj(ot::symbol);
      memcpy(res->chr_data(), str.c_str(), str.size() + 1);
      return res;
    }

    bool isNull() const { return sxpinfo.type == ot::nil; }
    bool isSymbol() const { return sxpinfo.type == ot::symbol; }
    bool isLogical() const { return sxpinfo.type == ot::logical; }
    bool isReal() const { return sxpinfo.type == ot::real; }
    bool isComplex() const { return sxpinfo.type == ot::complex; }
    bool isExpression() const { return sxpinfo.type == ot::expr; }
    bool isEnvironment() const { return sxpinfo.type == ot::env; }
    bool isString() const { return sxpinfo.type == ot::str; }
    bool isObject() const { return sxpinfo.obj != 0; }

    std::ostream &dump(std::ostream &os) const {
      if (this == nullptr) return os << "<nullptr>";
      switch (type()) {
        case ot::nil: return os << "NULL";
        case ot::symbol: return os << "`" << chr_data() << "\'";
        case ot::list: {
          os << "[";
          for (const obj *p = this; p != null_const(); p = p->tail()) {
            os << *head();
            if (tag() != nullptr) os << "(t=" << *tag() << ")";
            if (p->tail() != null_const()) os << ", ";
          }
          return os << "]";
        }
        case ot::lang: {
          os << "[L ";
          for (const obj *p = this; p != null_const(); p = p->tail()) {
            os << *head();
            if (tag() != nullptr) os << "+" << *tag();
            if (p->tail() != null_const()) os << ", ";
          }
          return os << "]";
        }
        default: return os << "[" << object_names[(int)type()] << " " << *head() << ", " << *tail() << "]";
      }
    }

protected:
    void init(ot type, objref head, objref tail) {
      memset((SEXPREC*)this, 0, sizeof(SEXPREC));
    }
  };

  std::ostream &operator <<(std::ostream &os, const obj &rhs) {
    rhs.dump(os);
    return os;
  }

}

#endif
