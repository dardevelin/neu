%{
  
  /*================================= Neu =================================
   
   Copyright (c) 2013-2014, Andrometa LLC
   All rights reserved.
   
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
   
   1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
   
   2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
   
   3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
   
   =======================================================================*/
  
  #include <string>
  #include <iostream>
  
  #include "NPLParser_.h"
  #include "parse.h"
  
  using namespace std;
  using namespace neu;
  
%}

%name-prefix="npl_"
%pure-parser
%parse-param {NPLParser_* PS}
%parse-param {void* scanner}
%lex-param {yyscan_t* scanner}

%token<v> IDENTIFIER STRING_LITERAL EQ NE GE LE INC ADD_BY SUB_BY MUL_BY DIV_BY MOD_BY AND OR KW_TRUE KW_FALSE KW_VOID KW_FOR KW_IF KW_ELSE KW_WHILE KW_RETURN KW_BREAK KW_CONTINUE KW_CLASS DEFINE DOUBLE INTEGER TYPE

%type<v> stmt expr expr_num func_def func_def_vec block stmts if_stmt expr_vec class_vec

%left ','
%right '=' ADD_BY SUB_BY MUL_BY DIV_BY MOD_BY
%right OR
%right AND
%right EQ NE
%right '<' '>' GE LE
%left '+' '-'
%left '*' '/' '%'
%left '^'
%left '!' INC DEC '@'

%%

input: /* empty */
| input KW_CLASS IDENTIFIER '{' class_vec '}' {
  PS->addClass($3, $5);
}
| input DEFINE IDENTIFIER expr {
  PS->define($3, $4);
}
;

class_vec: KW_VOID func_def block {
  $$ = PS->newClass();
  PS->addMethod($$, PS->func("TypedFunc") << PS->sym("Void") << move($2) << move($3));
}
| TYPE IDENTIFIER ';' {
  nvar t = move($1);
  t << PS->sym($2);
  $$ = PS->newClass();
  PS->addAttribute($$, t);
}
| class_vec KW_VOID func_def block {
  $$ = move($1);
  PS->addMethod($$, PS->func("TypedFunc") << PS->sym("Void") << move($3) << move($4));
}
| class_vec TYPE IDENTIFIER ';' {
  nvar t = move($2);
  t << PS->sym($3);
  $$ = move($1);
  PS->addAttribute($$, t);
}

expr_num: DOUBLE {
  $$ = PS->var($1);
}
| INTEGER {
  $$ = PS->var($1);
}
;

expr: expr_num {
  $$ = move($1);
}
| IDENTIFIER {
  $$ = PS->sym($1);
}
| '(' expr ')' {
  $$ = move($2);
}
| '@' IDENTIFIER {
  $$ = PS->func("O2") << PS->sym($2);
}
| KW_TRUE {
  $$ = PS->var(true);
}
| KW_FALSE {
  $$ = PS->var(false);
}
| STRING_LITERAL {
  $$ = move($1);
}
| '-' expr {
  if($2.isNumeric()){
    $$ = PS->var(-$2);
  }
  else{
    $$ = PS->func("Neg") << move($2);
  }
}
| '!' expr {
  $$ = PS->func("Not") << move($2);
}
| expr '+' expr {
  $$ = PS->func("Add") << move($1) << move($3);
}
| expr '-' expr {
  $$ = PS->func("Sub") << move($1) << move($3);
}
| expr '*' expr {
  $$ = PS->func("Mul") << move($1) << move($3);
}
| expr '/' expr {
  $$ = PS->func("Div") << move($1) << move($3);
}
| expr '^' expr {
  $$ = PS->func("Pow") << move($1) << move($3);
}
| expr '%' expr {
  $$ = PS->func("Mod") << move($1) << move($3);
}
| expr INC {
  $$ = PS->func("PostInc") << move($1);
}
| INC expr {
  $$ = PS->func("Inc") << move($2);
}
| expr DEC {
  $$ = PS->func("PostDec") << move($1);
}
| DEC expr {
  $$ = PS->func("Dec") << move($2);
}
| expr ADD_BY expr {
  $$ = PS->func("AddBy") << move($1) << move($3);
}
| expr SUB_BY expr {
  $$ = PS->func("SubBy") << move($1) << move($3);
}
| expr MUL_BY expr {
  $$ = PS->func("MulBy") << move($1) << move($3);
}
| expr DIV_BY expr {
  $$ = PS->func("DivBy") << move($1) << move($3);
}
| expr MOD_BY expr {
  $$ = PS->func("ModBy") << move($1) << move($3);
}
| expr '=' expr {
  $$ = PS->func("Set") << move($1) << move($3);
}
| expr '<' expr {
  $$ = PS->func("LT") << move($1) << move($3);
}
| expr LE expr {
  $$ = PS->func("LE") << move($1) << move($3);
}
| expr '>' expr {
  $$ = PS->func("GT") << move($1) << move($3);
}
| expr GE expr {
  $$ = PS->func("GE") << move($1) << move($3);
}
| expr EQ expr {
  $$ = PS->func("EQ") << move($1) << move($3);
}
| expr NE expr {
  $$ = PS->func("NE") << move($1) << move($3);
}
| expr AND expr {
  $$ = PS->func("And") << move($1) << move($3);
}
| expr OR expr {
  $$ = PS->func("Or") << move($1) << move($3);
}
| IDENTIFIER '(' expr_vec ')' {
  nvar c = PS->func($1);
  c.append($3);
  $$ = PS->func("Call") << move(c);
}
;

func_def: IDENTIFIER '(' func_def_vec ')' {
  $$ = PS->func($1);
  $$.append($3);
}
;

func_def_vec: /* empty */ {
  $$ = undef;
}
| func_def_vec ',' TYPE IDENTIFIER {
  $$ = move($1);
  $$ << move($3 << move($4));
}
| TYPE IDENTIFIER {
  $$ = move($1);
  $$ << move($2);
}
;

expr_vec: /* empty */ {
  $$ = undef;
}
| expr_vec ',' expr {
  $$ = move($1);
  $$ << move($3);
}
| expr {
  $$ = move($1);
}
;

stmt: expr ';' {
  $$ = move($1);
}
| ';' {
  $$ = none;
}
| KW_RETURN ';' {
  $$ = PS->func("Ret");
}
| KW_RETURN expr ';' {
  $$ = PS->func("Ret") << move($2);
}
| KW_BREAK ';' {
  $$ = PS->func("Break");
}
| KW_CONTINUE ';' {
  $$ = PS->func("Continue");
}
| if_stmt {
  $$ = move($1);
}
| KW_WHILE '(' expr ')' block {
  $$ = PS->func("While") << move($3) << move($5);
}
| KW_FOR '(' stmt stmt expr ')' block {
  $$ = PS->func("For") << move($3) << move($4) << move($5) << move($7);
}
;

if_stmt: KW_IF '(' expr ')' block {
  $$ = PS->func("If") << move($3) << move($5);
}
| KW_IF '(' expr ')' block KW_ELSE block {
  $$ = PS->func("If") << move($3) << move($5) << move($7);
}
| KW_IF '(' expr ')' block KW_ELSE if_stmt {
  $$ = PS->func("If") << move($3) << move($5) << move($7);
}
;

block: '{' stmts '}' {
  $$ = $2;
}
| '{' '}' {
  $$ = PS->func("Block");
}

stmts: stmts stmt {
  $$ = move($1);
  $$ << move($2);
}
| stmt {
  $$ = PS->func("Block") << move($1);
}
;

%%

int npl_error(NPLParser_* parser, const void*, const char *s){
  parser->error(s);
  return 1;
}
