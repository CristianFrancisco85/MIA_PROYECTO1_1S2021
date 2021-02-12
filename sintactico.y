/*------------------------------Código utilizado----------------------------------*/
%{
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <string.h>
#include <scanner.h>
#include <typeinfo>
#include <MKDISK.h>
#include <structs.h>



using namespace std;

extern int yylex(void);
extern char *yytext;
extern int SourceLine;
extern FILE *yyin;


MKDISK_* mkdisk_ = new MKDISK_();
void yyerror(const char *s);

string lineaGuiones="------------------------------------------------------------------------------------------------------------------------------------------------------";

%}
/*---------------Declaración de tokens utilizados en el léxico--------------------*/

%start INICIO

%token<NUM> numero
%token<NUM> numero_negativo
%token<STRING> igual
%token<STRING> mkdisk
%token<STRING> rmdisk
%token<STRING> fdisk
%token<STRING> mount
%token<STRING> unmount
%token<STRING> mkfs
%token<STRING> f
%token<STRING> u
%token<STRING> k
%token<STRING> m
%token<STRING> path
%token<STRING> size
%token<STRING> type
%token<STRING> del
%token<STRING> add
%token<STRING> name
%token<STRING> id
%token<STRING> fs
%token<STRING> cadena
%token<STRING> ruta
%token<STRING> guion
%token<STRING> bf
%token<STRING> ff
%token<STRING> wf


/*----------------------Declaración de producciones------------------------*/

%type<STRING> INICIO
%type<STRING> INSTRUCCIONES
%type<STRING> INSTRUCCION
%type<STRING> MKDISK
%type<STRING> MKDISKPARAMS
%type<STRING> MKDISKPARAM

/*-------------------------------- Opciones --------------------------------------*/

%error-verbose
%locations
%union{
char* STRING;
char* NUM;
}

/*------------------ Declaración de la gramática -------------------------*/

%%
INICIO: INSTRUCCIONES;

INSTRUCCIONES: 
    INSTRUCCION
    | INSTRUCCIONES INSTRUCCION
;

INSTRUCCION: 
    MKDISK
    | error{}
;

MKDISK:
    mkdisk MKDISKPARAMS {mkdisk_->createDisk();std::cout << lineaGuiones << std::endl;;mkdisk_ = new MKDISK_();}
;

MKDISKPARAMS: 
    MKDISKPARAMS MKDISKPARAM
    | MKDISKPARAM
;

MKDISKPARAM: 
    guion size igual numero {mkdisk_->setSize($4);}
    | guion f igual bf {mkdisk_->setFit($4);}
    | guion f igual ff {mkdisk_->setFit($4);}
    | guion f igual wf {mkdisk_->setFit($4);}
    | guion u igual k {mkdisk_->setUnit($4);}
    | guion u igual m {mkdisk_->setUnit($4);}
    | guion path igual ruta {mkdisk_->setPath($4);}
    | guion path igual cadena {mkdisk_->setPath($4);}
;


%%

void yyerror(const char *s)
{
    printf("Error sintactico en la linea %i: %s\n", SourceLine, s);
    std::cout << lineaGuiones << std::endl;
}
