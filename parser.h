/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_PARSER_H_INCLUDED
# define YY_YY_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    pause_ = 258,
    numero = 259,
    numero_negativo = 260,
    igual = 261,
    mkdisk = 262,
    rmdisk = 263,
    fdisk = 264,
    mount = 265,
    unmount = 266,
    mkfs = 267,
    f = 268,
    u = 269,
    k = 270,
    m = 271,
    path = 272,
    size = 273,
    type = 274,
    del = 275,
    add = 276,
    name = 277,
    id = 278,
    id_ = 279,
    fs = 280,
    cadena = 281,
    ruta = 282,
    guion = 283,
    bf = 284,
    ff = 285,
    wf = 286,
    delete__ = 287,
    add__ = 288,
    fast = 289,
    full = 290,
    p = 291,
    e = 292,
    l = 293,
    login = 294,
    usr = 295,
    pwd = 296,
    logout = 297,
    mkgrp = 298,
    rmgrp = 299,
    grp = 300,
    mkusr = 301,
    rmusr = 302,
    rep = 303,
    mbr = 304,
    disk = 305,
    inode = 306,
    block = 307,
    bm_block = 308,
    bm_inode = 309,
    sb = 310,
    _2fs = 311,
    _3fs = 312
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 169 "sintactico.y" /* yacc.c:1909  */

char* STRING;
char* NUM;

#line 117 "parser.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE yylval;
extern YYLTYPE yylloc;
int yyparse (void);

#endif /* !YY_YY_PARSER_H_INCLUDED  */
