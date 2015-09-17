
#ifndef PARSER_HPP
#define PARSER_HPP

#include "lexer.hpp"
#include "objects.hpp"

namespace little_r {
  class parser : public lexer {
  public:
    parser(std::wistream &istr) : lexer(istr) {
      next();
      prog();
    }
  private:
    void prog() {
      for(;;) {
        if (tok() == tt::error) break;
        if (tok() == tt::end_of_input) break;
        expr(0);
      }
    }

    int get_precedence(tt sym) {
      switch (sym) {
        // 100 %left		'?'
        case tt::question: return 100;
        // 110 %left		LOW WHILE FOR REPEAT
        case tt::while_: return 110;
        case tt::for_: return 110;
        case tt::repeat: return 110;
        // 120 %right		IF
        case tt::if_: return 120;
        // 130 %left		ELSE
        case tt::else_: return 130;
        // 140 %right		LEFT_ASSIGN
        case tt::left_assign: return 140;
        // 150 %right		EQ_ASSIGN
        case tt::eq_assign: return 150;
        // 160 %left		RIGHT_ASSIGN
        case tt::right_assign: return 160;
        // 170 %left		'~' TILDE
        case tt::tilde: return 170;
        // 180 %left		OR OR2
        case tt::or: return 180;
        case tt::or2: return 180;
        // 190 %left		AND AND2
        case tt::and: return 190;
        case tt::and2: return 190;
        // 200 %left		UNOT NOT
        case tt::not: return 200;
        // 210 %nonassoc   	GT GE LT LE EQ NE
        case tt::lt: case tt::le: case tt::eq:
        case tt::ne: case tt::ge: case tt::gt: return 210;
        // 220 %left		'+' '-'
        case tt::plus: case tt::minus: return 220;
        // 230 %left		'*' '/'
        case tt::star: case tt::divide: return 230;
        // 240 %left		SPECIAL
        case tt::special: return 240;
        // 250 %left		':'
        case tt::colon: return 250;
        // 260 %left		UMINUS UPLUS
        //case tt::uminus: case tt::uplus: return 260;
        // 270 %right		'^'
        case tt::caret: return 270;
        // 280 %left		'$' '@'
        // 290 %left		NS_GET NS_GET_INT
        default: throw std::runtime_error("get_precedence");
      }
      return 0;
    }

    obj *expr(int precedence=0, bool allow_assign=true, bool allow_ltgt=true) {
      obj *result = obj::null_const();

      // skip newlines
      while (tok() == tt::newline) {
        next();
      }

      switch (tok()) {
        //expr	: 	NUM_CONST			{ $$ = $1;	setId( $$, @$); }
        // |	STR_CONST			{ $$ = $1;	setId( $$, @$); }
        // |	NULL_CONST			{ $$ = $1;	setId( $$, @$); }          
        // |	SYMBOL				{ $$ = $1;	setId( $$, @$); }
        // |	SYMBOL NS_GET SYMBOL		{ $$ = xxbinary($2,$1,$3);      setId( $$, @$); modif_token( &@1, SYMBOL_PACKAGE ) ; }
        // |	SYMBOL NS_GET STR_CONST		{ $$ = xxbinary($2,$1,$3);      setId( $$, @$); modif_token( &@1, SYMBOL_PACKAGE ) ; }
        // |	STR_CONST NS_GET SYMBOL		{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	STR_CONST NS_GET STR_CONST	{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	SYMBOL NS_GET_INT SYMBOL	{ $$ = xxbinary($2,$1,$3);      setId( $$, @$); modif_token( &@1, SYMBOL_PACKAGE ) ;}
        // |	SYMBOL NS_GET_INT STR_CONST	{ $$ = xxbinary($2,$1,$3);      setId( $$, @$); modif_token( &@1, SYMBOL_PACKAGE ) ;}
        // |	STR_CONST NS_GET_INT SYMBOL	{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        // |	STR_CONST NS_GET_INT STR_CONST	{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
        case tt::num_const:
        case tt::str_const:
        case tt::null_const:
        case tt::symbol: {
          result = value();
          next();
          if (tok() == tt::ns_get || tok() == tt::ns_get_int) {
            next();
            if (tok() == tt::symbol || tok() == tt::str_const) {
              next();
            }
          }
          break;
        }

        // |	'{' exprlist '}'		{ $$ = xxexprlist($1,&@1,$2); setId( $$, @$); }
        case tt::lbrace: {
          next();
          result = new obj(ot::
          expr();
          expect(tt::rbrace);
          break;
        }

        // |	'(' expr_or_assign ')'		{ $$ = xxparen($1,$2);	setId( $$, @$); }
        case tt::lparen: {
          next();
          result = expr();
          expect(tt::rparen);
          break;
        }

        // |	'-' expr %prec UMINUS		{ $$ = xxunary($1,$2);	setId( $$, @$); }
        case tt::minus: {
          next();
          result = expr(0);
          //result = new expr(tt::minus, expr(0));
          break;
        }

        // |	'+' expr %prec UMINUS		{ $$ = xxunary($1,$2);	setId( $$, @$); }
        case tt::plus: {
          next();
          //result = new expr(tt::minus, expr(0));
          result = expr(0);
          break;
        }

        // |	'!' expr %prec UNOT		{ $$ = xxunary($1,$2);	setId( $$, @$); }
        case tt::not: {
          next();
          //result = new expr(tt::minus, expr(0));
          result = expr(0);
          break;
        }

        // |	'~' expr %prec TILDE		{ $$ = xxunary($1,$2);	setId( $$, @$); }
        case tt::tilde: {
          next();
          //result = new expr(tt::minus, expr(0));
          result = expr(0);
          break;
        }

        // |	'?' expr			{ $$ = xxunary($1,$2);	setId( $$, @$); }
        case tt::question: {
          next();
          //result = new expr(tt::minus, expr(0));
          result = expr(0);
          break;
        }

        // |	FUNCTION '(' formlist ')' cr expr_or_assign %prec LOW
        case tt::function: {
          next();
          expect(tt::lparen);
          while (tok() != tt::rparen) {
            next();
          }
          expect(tt::rparen);
          while (tok() == tt::newline) {
            next();
          }
          obj *body = expr(0);
          break;
        }

        // |	IF ifcond expr_or_assign 	{ $$ = xxif($1,$2,$3);	setId( $$, @$); }
        // |	IF ifcond expr_or_assign ELSE expr_or_assign	{ $$ = xxifelse($1,$2,$3,$5);	setId( $$, @$); }
        case tt::if_: {
          next();
          obj *cond = expr(0);
          obj *stmt = expr(0);
          if (tok() == tt::else_) {
            next();
            obj *stmt2 = expr(0);
          }
          break;
        }
        // |	FOR forcond expr_or_assign %prec FOR 	{ $$ = xxfor($1,$2,$3);	setId( $$, @$); }
        case tt::for_: {
          next();
          obj *cond = expr(0);
          obj *stmt = expr(0);
          break;
        }
        // |	WHILE cond expr_or_assign	{ $$ = xxwhile($1,$2,$3);	setId( $$, @$); }
        case tt::while_: {
          next();
          obj *cond = expr(0);
          obj *stmt = expr(0);
          break;
        }
        // |	REPEAT expr_or_assign			{ $$ = xxrepeat($1,$2);	setId( $$, @$); }
        case tt::repeat: {
          next();
          obj *stmt = expr(0);
          break;
        }
        // |	NEXT				{ $$ = xxnxtbrk($1);	setId( $$, @$); }
        case tt::next: {
          next();
          break;
        }
        // |	BREAK				{ $$ = xxnxtbrk($1);	setId( $$, @$); }
        case tt::break_: {
          next();
          break;
        }
        // ;

        default: {
          error("expected expression");
          return result;
        }
      }

        // |	expr '(' sublist ')'		{ $$ = xxfuncall($1,$3);  setId( $$, @$); modif_token( &@1, SYMBOL_FUNCTION_CALL ) ; }
      if (tok() == tt::lparen) {
        next();
        obj *actuals = sublist();
        expect(tt::rparen);
        return 
      }


      bool done = false;
      while (!done) {
        int prec = 0;
        int right = 0;

        if (tok() == tt::newline) {
          next();
          return result;
        }

        switch (tok()) {
          // |	expr LBB sublist ']' ']'	{ $$ = xxsubscript($1,$2,$3);	setId( $$, @$); }
          // |	expr '[' sublist ']'		{ $$ = xxsubscript($1,$2,$3);	setId( $$, @$); }
          case tt::lbb: {
            next();
            sublist();
            expect(tt::rbracket);
            expect(tt::rbracket);
            break;
          }
          case tt::lbracket: {
            next();
            sublist();
            expect(tt::rbracket);
            break;
          }

          // |	expr '$' SYMBOL			{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
          // |	expr '$' STR_CONST		{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
          // |	expr '@' SYMBOL			{ $$ = xxbinary($2,$1,$3);      setId( $$, @$); modif_token( &@3, SLOT ) ; }
          // |	expr '@' STR_CONST		{ $$ = xxbinary($2,$1,$3);	setId( $$, @$); }
          case tt::dollar:
          case tt::at: {
            next();
            expect(tt::symbol, tt::str_const);
          }


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
          case tt::colon:
          case tt::plus:
          case tt::minus:
          case tt::star:
          case tt::divide:
          case tt::caret:
          case tt::special:
          case tt::modulus:
          case tt::tilde:
          case tt::question:
          case tt::lt: case tt::le: case tt::eq:
          case tt::ne: case tt::ge: case tt::gt:
          case tt::and:
          case tt::or:
          case tt::and2:
          case tt::or2:
          case tt::left_assign:
          case tt::right_assign:
          binary:
          {
            int tok_prec = get_precedence(tok());
            if (tok_prec < precedence) {
              done = true;
              break;
            }
            next();
            obj *rhs = expr(prec);
            result = new obj(ot::lang, result, rhs);
            break;
          }
          default: {
            error("expected operator");
          }
        }
      }
      return result;
    }

    // eg. 1, 3, var = 4
    // sub	:					{ $$ = xxsub0();	 }
    //	|	expr				{ $$ = xxsub1($1, &@1);  }
    //	|	SYMBOL EQ_ASSIGN 		{ $$ = xxsymsub0($1, &@1); 	modif_token( &@2, EQ_SUB ) ; modif_token( &@1, SYMBOL_SUB ) ; }
    //	|	SYMBOL EQ_ASSIGN expr		{ $$ = xxsymsub1($1,$3, &@1); 	modif_token( &@2, EQ_SUB ) ; modif_token( &@1, SYMBOL_SUB ) ; }
    //	|	STR_CONST EQ_ASSIGN 		{ $$ = xxsymsub0($1, &@1); 	modif_token( &@2, EQ_SUB ) ; }
    //	|	STR_CONST EQ_ASSIGN expr	{ $$ = xxsymsub1($1,$3, &@1); 	modif_token( &@2, EQ_SUB ) ; }
    //	|	NULL_CONST EQ_ASSIGN 		{ $$ = xxnullsub0(&@1); 	modif_token( &@2, EQ_SUB ) ; }
    //	|	NULL_CONST EQ_ASSIGN expr	{ $$ = xxnullsub1($3, &@1); 	modif_token( &@2, EQ_SUB ) ; }
    //	;
    obj *sublist() {
      obj *result = obj::null_const();
      obj *prev = result;
      for (;;) {
        obj *lhs = expr(0);
        if (tok() == tt::eq_assign) {
          next();
          if (!lhs->isNull() && !lhs->isSymbol() && !lhs->isString()) {
            error("expected symbol, string or null before '='");
            return obj::null_const();
          }
          if (tok() != tt::rparen && tok() != tt::comma) {
            obj *rhs = expr(0);
            lhs = new obj(ot::lang, lhs, rhs);
          }
          if (result == obj::null_const()) {
            prev = result = new obj();
          }
          prev = prev->append(expr);
          if (tok() != tt::comma) {
            break;
          }
          next();
        }
      }
      return result;
    }

    void expect(tt token) {
      if (tok() != token) {
        throw std::runtime_error(std::string("expected") + tok_to_str[(unsigned)token]);
      }
      next();
    }

    void error(const char *str) {
      std::cout << "error " << str << "\n";
    }
  };
}

#endif
