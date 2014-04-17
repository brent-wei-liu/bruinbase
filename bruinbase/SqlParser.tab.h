/* A Bison parser, made by GNU Bison 1.875c.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     SELECT = 258,
     FROM = 259,
     WHERE = 260,
     LOAD = 261,
     WITH = 262,
     INDEX = 263,
     QUIT = 264,
     COUNT = 265,
     AND = 266,
     OR = 267,
     COMMA = 268,
     STAR = 269,
     LF = 270,
     INTEGER = 271,
     STRING = 272,
     ID = 273,
     EQUAL = 274,
     NEQUAL = 275,
     LESS = 276,
     LESSEQUAL = 277,
     GREATER = 278,
     GREATEREQUAL = 279
   };
#endif
#define SELECT 258
#define FROM 259
#define WHERE 260
#define LOAD 261
#define WITH 262
#define INDEX 263
#define QUIT 264
#define COUNT 265
#define AND 266
#define OR 267
#define COMMA 268
#define STAR 269
#define LF 270
#define INTEGER 271
#define STRING 272
#define ID 273
#define EQUAL 274
#define NEQUAL 275
#define LESS 276
#define LESSEQUAL 277
#define GREATER 278
#define GREATEREQUAL 279




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 32 "SqlParser.y"
typedef union YYSTYPE {
  int integer;
  char* string;
  SelCond* cond;
  std::vector<SelCond>* conds;
} YYSTYPE;
/* Line 1275 of yacc.c.  */
#line 92 "SqlParser.tab.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE sqllval;



