/*--------- Definiciones de FLEX----------*/
%option noyywrap
%option case-insensitive
%option yylineno
%option nounput
/*------------Definiciones de C ------------*/
%{
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "parser.h"

int SourceLine = 1;
%}

/*-----------Expresiones regulares ---------*/
comentarios [#]([^\n])*
numero [0-9]+
numero_negativo [-][0-9]+
string (\"(\\.|[^\\"])*\")
ruta "/"("/"|"-"|"_"|"."|[a-zA-Z0-9])+
id ([a-zA-Z]|[0-9]|"_")*

%% // <<<<<<<<<<<<<<<<<<< [Reglas lexicas] >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><
('\s'|'\r'|'\t')+ {};
"\n" {SourceLine++;}
{comentarios} {};
{numero} {yylval.NUM = strdup(yytext);return (numero);};
{numero_negativo}     {yylval.NUM = strdup(yytext);return (numero_negativo);};
"=" {yylval.STRING = strdup(yytext);return (igual);};
"mkdisk" {yylval.STRING = strdup(yytext);return (mkdisk);};
"rmdisk" {yylval.STRING = strdup(yytext);return (rmdisk);};
"fdisk" {yylval.STRING = strdup(yytext);return (fdisk);};
"mount" {yylval.STRING = strdup(yytext);return (mount);};
"unmount" {yylval.STRING = strdup(yytext);return (unmount);};
"mkfs" {yylval.STRING = strdup(yytext);return (mkfs);};
"f" {yylval.STRING = strdup(yytext);return (f);};
"u" {yylval.STRING = strdup(yytext);return (u);};
"k" {yylval.STRING = strdup(yytext);return (k);};
"m" {yylval.STRING = strdup(yytext);return (m);};
"path" {yylval.STRING = strdup(yytext);return (path);};
"size" {yylval.STRING = strdup(yytext);return (size);};
"type" {yylval.STRING = strdup(yytext);return (type);};
"delete" {yylval.STRING = strdup(yytext);return (delete__);};
"fast" {yylval.STRING = strdup(yytext);return (fast);};
"full" {yylval.STRING = strdup(yytext);return (full);};
"pause" {yylval.STRING = strdup(yytext);return (pause_);};
"p" {yylval.STRING = strdup(yytext);return (p);};
"e" {yylval.STRING = strdup(yytext);return (e);};
"l" {yylval.STRING = strdup(yytext);return (l);};
"add" {yylval.STRING = strdup(yytext);return (add__);};
"name" {yylval.STRING = strdup(yytext);return (name);};
"id" {yylval.STRING = strdup(yytext);return (id_);};
"fs" {yylval.STRING = strdup(yytext);return (fs);};
"bf" {yylval.STRING = strdup(yytext);return (bf);};
"ff" {yylval.STRING = strdup(yytext);return (ff);};
"wf" {yylval.STRING = strdup(yytext);return (wf);};
"rep" {yylval.STRING = strdup(yytext);return (rep);};
"mbr" {yylval.STRING = strdup(yytext);return (mbr);};
"disk" {yylval.STRING = strdup(yytext);return (disk);};
"inode" {yylval.STRING = strdup(yytext);return (inode);};
"block" {yylval.STRING = strdup(yytext);return (block);};
"bm_inode" {yylval.STRING = strdup(yytext);return (bm_inode);};
"bm_block" {yylval.STRING = strdup(yytext);return (bm_block);};
"sb" {yylval.STRING = strdup(yytext);return (sb);};

"2fs" {yylval.STRING = strdup(yytext);return (_2fs);};
"3fs" {yylval.STRING = strdup(yytext);return (_3fs);};
"login" {yylval.STRING = strdup(yytext);return (login);};
"usuario" {yylval.STRING = strdup(yytext);return (usuario);};
"password" {yylval.STRING = strdup(yytext);return (password);};
"logout" {yylval.STRING = strdup(yytext); return (logout);}

{string} {yylval.STRING = strdup(yytext);return (cadena);};
{ruta} {yylval.STRING = strdup(yytext);return(ruta);};
{id} {yylval.STRING = strdup(yytext);return(id);};
[-] {yylval.STRING = strdup(yytext);return(guion);};
. {}
%%//<<<<<<<<<<<<<<<<<<<<<<<<<<<< [Reglas léxicas] >>>>>>>>>>>>>>>>>>>>>>>>>>>>>
