
namespace little_r {
  enum class ot {
    nils = 0, // nil  = null
    syms = 1, // symbols
    lists = 2, // lists of dotted pairs
    clos = 3, // closures
    envs = 4, // environments
    proms = 5, // promises: [un]evaluated closure arguments
    langs = 6, // language constructs (special lists)
    specials = 7, // special forms
    builtins = 8, // builtin non-special forms
    chars = 9, // "scalar" string type (internal only)
    lgls = 10, // logical vectors
    ints = 13, // integer vectors
    reals = 14, // real variables
    cplxs = 15, // complex variables
    strs = 16, // string vectors
    dots = 17, // dot-dot-dot object
    anys = 18, // make "any" args work
    vecs = 19, // generic vectors
    exprs = 20, // expressions vectors
    bcodes = 21, // byte code
    extptrs = 22, // external pointer
    weakrefs = 23, // weak reference
    raws = 24, // raw bytes
    s4s = 25, // s4 non-vector

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

  // maintaining binary compatibility with old R libraries may be a challenge.
  // especially with the toxic LGPL license.
  class obj {
  public:
    obj() {
      sxpinfo.type = ot::nils;
      attrib = nullptr;
      gengc_next_node = nullptr;
      gengc_prev_node = nullptr;
      listsxp.carval = nullptr;
      listsxp.cdrval = nullptr;
      listsxp.tagval = nullptr;
    }

    ot type() const { return sxpinfo.type; }
    obj *head() const { return listsxp.carval; }
    obj *tail() const { return listsxp.cdrval; }
    obj *tag() const { return listsxp.tagval; }
  protected:
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
