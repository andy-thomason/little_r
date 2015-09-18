
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

  // equivalent to reference compiler's SEXP
  class obj {
  public:
    obj(ot type=ot::list, obj *elem1=null_const(), obj *elem2=null_const()) {
      init(type, elem1, elem2);
    }

    obj(obj *elem1, obj *elem2) {
      init(ot::list, elem1, elem2);
    }

    obj(ot type, obj *elem1, obj *elem2, obj *elem3) {
      init(type, elem1, new obj(type, elem2, elem3));
    }

    /*template <typename... elems>
    obj(ot type, obj *head, elems... tail) {
      init(type, head, new obj(type, tail...));
    }*/

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
    obj *head() const { return listsxp.carval; }
    obj *tail() const { return listsxp.cdrval; }
    obj *tag() const { return listsxp.tagval; }

    obj &set_type(ot value) { sxpinfo.type = value; return *this; }
    obj &set_head(obj *value) { listsxp.carval = value; return *this; }
    obj &set_tail(obj *value) { listsxp.cdrval = value; return *this; }
    obj &set_tag(obj *value) { listsxp.tagval = value; return *this; }

    obj &add_to_stretchy(obj *last) {
      obj *extra = new obj(last, null_const());
      head()->set_tail(extra);
      set_head(extra);
      return *this;
    }

    obj *last() {
      obj *p = this;
      while (tail() != null_const()) {
        p = tail();
      }
      return p;
    }

    obj *append(obj *val) {
      obj *t = tail();
      obj *extra = new obj(ot::list, val);
      t->set_tail(extra);
      return extra;
    }

    char *chr_data() { return (char*)this + sizeof(obj); }

    static obj *null_const() {
      static obj null_const_value;
      return &null_const_value;
    }

    static obj *make_string(const std::string &str) {
      obj *res = new (str.size() + 1) obj(ot::chr);
      memcpy(res->chr_data(), str.c_str(), str.size() + 1);
      return res;
    }

    static obj *make_symbol(const std::string &str) {
      obj *res = new (str.size() + 1) obj(ot::symbol);
      memcpy(res->chr_data(), str.c_str(), str.size() + 1);
      return res;
    }

    // make a list who's head points to the last element
    static obj *make_stretchy_list() {
      obj *res = new obj(null_const(), null_const());
      res->set_head(res);
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

  protected:
    void init(ot type, obj *head, obj *tail) {
      memset(this, 0, sizeof(*this));
      sxpinfo.type = type;
      //attrib = nullptr;
      //gengc_next_node = nullptr;
      //gengc_prev_node = nullptr;
      listsxp.carval = head;
      listsxp.cdrval = tail;
      //listsxp.tagval = nullptr;
    }

    // apologies for the readablility here, these names are from the
    // original R source code.
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
}

#endif
