
#include "lexer.hpp"
#include "objects.hpp"

namespace little_r {
  class parser : public lexer {
  public:
    parser(std::wistream &istr) : lexer(istr) {
      prog();
    }
  private:
    void prog() {
      for(;;) {
        if (tok() == tt::error) break;
        if (tok() == tt::end_of_input) break;
        expr_or_assign(0);
      }
    }

    /*int get_precidence(tt token) {
      switch (token) {
        case tt::question: return 10; //%left		'?'
        case tt::while_: case tt::for_: case tt::repeat: return 20; //%left		LOW WHILE FOR REPEAT
        case tt::if_: return 30; //%right		IF
        case tt::else_: return 40; //%left		ELSE
        case tt::left_assign: return 50; //%right		LEFT_ASSIGN
        case tt::eq_assign: return 60; //%right		EQ_ASSIGN
        case tt::right_assign: return 70; //%left		RIGHT_ASSIGN
        case tt::tilde: return 80; //%left		'~' TILDE
        case tt::or: case tt::or2: return 90; //%left		OR OR2
        case tt::and: case tt::and2: return 100; //%left		AND AND2
        case tt::not: return 110; //%left		UNOT NOT
        case tt::gt: case tt::ge: case tt::lt: case tt::le: case tt::eq: case tt::ne: {
          return 120; //%nonassoc   	GT GE LT LE EQ NE
        }
        case tt::plus: tt::minus: return 130; //%left		'+' '-'
        case tt::star: tt::divide: : return 140; //%left		'*' '/'
        case tt::special: return 150; //%left		SPECIAL
        case tt::colon: return 160; //%left		':'
        //case tt::minus: return 170; //%left		UMINUS UPLUS
        case tt::caret: return 180; //%right		'^'
        case tt::dollar: case tt::at: return 190; //%left		'$' '@'
        case tt::ns_get: case tt::ns_get_int: return 200; //%left		NS_GET NS_GET_INT
        case tt::lbb: return 210; //%nonassoc	'(' '[' LBB
      }
    }*/

    obj *expr_or_assign(int precidence) {
      obj *result = nullptr;
      switch (tok()) {
        //expr	: 	NUM_CONST			{ $$ = $1;	setId( $$, @$); }
        // |	STR_CONST			{ $$ = $1;	setId( $$, @$); }
        // |	NULL_CONST			{ $$ = $1;	setId( $$, @$); }          
        // |	SYMBOL				{ $$ = $1;	setId( $$, @$); }
        case tt::num_const:
        case tt::str_const:
        case tt::null_const:
        case tt::symbol:

        // |	'{' exprlist '}'		{ $$ = xxexprlist($1,&@1,$2); setId( $$, @$); }
        case tt::lbrace: {
          next();
          result = expr_or_assign(0);
          expect(tt::rbrace);
          break;
        }

        // |	'(' expr_or_assign ')'		{ $$ = xxparen($1,$2);	setId( $$, @$); }
        case tt::lparen: {
          next();
          result = expr_or_assign(0);
          expect(tt::rparen);
          break;
        }

        // |	'-' expr %prec UMINUS		{ $$ = xxunary($1,$2);	setId( $$, @$); }
        case tt::minus: {
          next();
          result = expr_or_assign(0);
          //result = new expr(tt::minus, expr_or_assign(0));
          break;
        }

        // |	'+' expr %prec UMINUS		{ $$ = xxunary($1,$2);	setId( $$, @$); }
        case tt::plus: {
          next();
          //result = new expr(tt::minus, expr_or_assign(0));
          result = expr_or_assign(0);
          break;
        }

        // |	'!' expr %prec UNOT		{ $$ = xxunary($1,$2);	setId( $$, @$); }
        case tt::not: {
          next();
          //result = new expr(tt::minus, expr_or_assign(0));
          result = expr_or_assign(0);
          break;
        }

        // |	'~' expr %prec TILDE		{ $$ = xxunary($1,$2);	setId( $$, @$); }
        case tt::tilde: {
          next();
          //result = new expr(tt::minus, expr_or_assign(0));
          result = expr_or_assign(0);
          break;
        }

        // |	'?' expr			{ $$ = xxunary($1,$2);	setId( $$, @$); }
        case tt::question: {
          next();
          //result = new expr(tt::minus, expr_or_assign(0));
          result = expr_or_assign(0);
          break;
        }
      }

      switch (tok()) {
        // |	expr ':'  expr			{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	expr '+'  expr			{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	expr '-' expr			{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	expr '*' expr			{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	expr '/' expr			{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	expr '^' expr 			{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	expr SPECIAL expr		{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	expr '%' expr			{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	expr '~' expr			{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	expr '?' expr			{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	expr LT expr			{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	expr LE expr			{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	expr EQ expr			{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	expr NE expr			{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	expr GE expr			{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	expr GT expr			{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	expr AND expr			{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	expr OR expr			{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	expr AND2 expr			{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	expr OR2 expr			{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }

        // |	expr LEFT_ASSIGN expr 		{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	expr RIGHT_ASSIGN expr 		{ $$ = xxbinary($2,$3,$1);	setId( $$, @$); }
        // |	FUNCTION '(' formlist ')' cr expr_or_assign %prec LOW
	        //		    { $$ = xxdefun($1,$3,$6,&@$); 	setId( $$, @$); }
        // |	expr '(' sublist ')'		{ $$ = xxfuncall($1,$3);  setId( $$, @$); modif_token( &@1, SYMBOL_FUNCTION_CALL ) ; }
        // |	IF ifcond expr_or_assign 	{ $$ = xxif($1,$2,$3);	setId( $$, @$); }
        // |	IF ifcond expr_or_assign ELSE expr_or_assign	{ $$ = xxifelse($1,$2,$3,$5);	setId( $$, @$); }
        // |	FOR forcond expr_or_assign %prec FOR 	{ $$ = xxfor($1,$2,$3);	setId( $$, @$); }
        // |	WHILE cond expr_or_assign	{ $$ = xxwhile($1,$2,$3);	setId( $$, @$); }
        // |	REPEAT expr_or_assign			{ $$ = xxrepeat($1,$2);	setId( $$, @$); }
        // |	expr LBB sublist ']' ']'	{ $$ = xxsubscript($1,$2,$3);	setId( $$, @$); }
        // |	expr '[' sublist ']'		{ $$ = xxsubscript($1,$2,$3);	setId( $$, @$); }
        // |	SYMBOL NS_GET SYMBOL		{ $$ = xxbinary($2,$1,$3);      setId( $$, @$); modif_token( &@1, SYMBOL_PACKAGE ) ; }
        // |	SYMBOL NS_GET STR_CONST		{ $$ = xxbinary($2,$1,$3);      setId( $$, @$); modif_token( &@1, SYMBOL_PACKAGE ) ; }
        // |	STR_CONST NS_GET SYMBOL		{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	STR_CONST NS_GET STR_CONST	{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	SYMBOL NS_GET_INT SYMBOL	{ $$ = xxbinary($2,$1,$3);      setId( $$, @$); modif_token( &@1, SYMBOL_PACKAGE ) ;}
        // |	SYMBOL NS_GET_INT STR_CONST	{ $$ = xxbinary($2,$1,$3);      setId( $$, @$); modif_token( &@1, SYMBOL_PACKAGE ) ;}
        // |	STR_CONST NS_GET_INT SYMBOL	{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	STR_CONST NS_GET_INT STR_CONST	{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	expr '$' SYMBOL			{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	expr '$' STR_CONST		{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	expr '@' SYMBOL			{ $$ = xxbinary($2,$1,$3);      setId( $$, @$); modif_token( &@3, SLOT ) ; }
        // |	expr '@' STR_CONST		{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	NEXT				{ $$ = xxnxtbrk($1);	setId( $$, @$); }
        // |	BREAK				{ $$ = xxnxtbrk($1);	setId( $$, @$); }
        // ;
      }

    }

    void expect(tt token) {
      if (tok() != token) {
        throw std::runtime_error(std::string("expected") + tok_to_str[(unsigned)token]);
      }
      next();
    }
  };
}
