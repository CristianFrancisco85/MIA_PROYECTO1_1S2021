/*------------------------------Importacion de Codigo ----------------------------------*/
%{
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <string.h>
#include <scanner.h>
#include <typeinfo>
#include <MKDISK.h>
#include <RMDISK.h>
#include <FDISK.h>
#include <MOUNT.h>
#include <UNMOUNT.h>
#include <list>
#include <structs.h>


using namespace std;

extern int yylex(void);
extern char *yytext;
extern int SourceLine;
extern FILE *yyin;
list<MOUNT_>* mounted = new list<MOUNT_>();



MKDISK_* mkdisk_ = new MKDISK_();
RMDISK_* rmdisk_ = new RMDISK_();
FDISK_* fdisk_ = new FDISK_();
MOUNT_* mount_ = new MOUNT_();
UNMOUNT_* unmount_ = new UNMOUNT_();

void yyerror(const char *s);

string lineaGuiones="------------------------------------------------------------------------------------------------------------------------------------------------------";

%}
/*---------------Tokens de Flex--------------------*/

%start INICIO

%token<NUM> pause_
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
%token<STRING> id_
%token<STRING> fs
%token<STRING> cadena
%token<STRING> ruta
%token<STRING> guion
%token<STRING> bf
%token<STRING> ff
%token<STRING> wf
%token<STRING> delete__
%token<STRING> add__
%token<STRING> fast
%token<STRING> full
%token<STRING> p
%token<STRING> e
%token<STRING> l


/*----------------------Producciones-----------------------*/

%type<STRING> INICIO
%type<STRING> INSTRUCCIONES
%type<STRING> INSTRUCCION

%type<STRING> PAUSE

%type<STRING> MKDISK
%type<STRING> MKDISKPARAMS
%type<STRING> MKDISKPARAM

%type<STRING> RMDISK

%type<STRING> FDISK
%type<STRING> FDISKPARAMS
%type<STRING> FDISKPARAM

%type<STRING> MOUNT
%type<STRING> MOUNTPARAMS
%type<STRING> MOUNTPARAM

%type<STRING> UNMOUNT

/*-------------------------------- Opciones --------------------------------------*/

%error-verbose
%locations
%union{
char* STRING;
char* NUM;
}

/*------------------ Gramatica-------------------------*/

%%
INICIO: INSTRUCCIONES;

INSTRUCCIONES: 
    INSTRUCCION
    | INSTRUCCIONES INSTRUCCION
;

INSTRUCCION: 
    MKDISK
    |RMDISK
    |FDISK
    |MOUNT
    |UNMOUNT
    |PAUSE
    |error{}
;

MKDISK:
    mkdisk MKDISKPARAMS {mkdisk_->createDisk();cout << lineaGuiones <<endl;;mkdisk_ = new MKDISK_();}
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

RMDISK: 
    rmdisk guion path igual ruta {rmdisk_->setPath($5);rmdisk_->deleteDisk(); cout << lineaGuiones <<endl; rmdisk_ = new RMDISK_();}
    | rmdisk guion path igual cadena {rmdisk_->setPath($5);rmdisk_->deleteDisk();cout << lineaGuiones <<endl; rmdisk_ = new RMDISK_();}
;

FDISK: 
    fdisk FDISKPARAMS{fdisk_->init(); cout<<lineaGuiones <<endl; fdisk_ = new FDISK_();}
;

FDISKPARAMS: 
    FDISKPARAM
    | FDISKPARAMS FDISKPARAM
;

FDISKPARAM:   
    guion size igual numero {fdisk_->setSize($4);}
    | guion u igual k {fdisk_->setUnit($4);}
    | guion u igual m {fdisk_->setUnit($4);}
    | guion path igual ruta {fdisk_->setPath($4);}
    | guion path igual cadena {fdisk_->setPath($4);}
    | guion name igual id {fdisk_->setName($4);}
    | guion name igual cadena {fdisk_->setName($4);}
    | guion f igual bf {fdisk_->setFit($4);}
    | guion f igual ff {fdisk_->setFit($4);}
    | guion f igual wf {fdisk_->setFit($4);}
    | guion delete__ igual fast {fdisk_->setDelete($4);}
    | guion delete__ igual full {fdisk_->setDelete($4);}
    | guion add__ igual numero {fdisk_->setAdd($4);}
    | guion add__ igual numero_negativo {fdisk_->setAdd($4);}
    | guion type igual p {fdisk_->setType($4);}
    | guion type igual e {fdisk_->setType($4);}
    | guion type igual l {fdisk_->setType($4);}
;
 
MOUNT: 
    mount MOUNTPARAMS {mount_->beginToMount();cout<<lineaGuiones<<endl;mount_= new MOUNT_();}
;

MOUNTPARAMS: 
    MOUNTPARAM
    | MOUNTPARAMS MOUNTPARAM
;

MOUNTPARAM:   
    guion path igual ruta {mount_->setPath($4);}
    | guion path igual cadena {mount_->setPath($4);}
    | guion name igual id {mount_->setName($4);}
    | guion name igual cadena {mount_->setName($4);}
;

UNMOUNT: 
    unmount guion id_ igual id {unmount_->setId($5);unmount_->beginToUnmount();cout << lineaGuiones <<endl;unmount_= new UNMOUNT_();}
;

PAUSE:
    pause_{cout<<"Presiona Enter para continuar..."<<endl;cin.ignore();}
;

%%

void yyerror(const char *s){
    printf("Error sintactico en la linea %i: %s\n", SourceLine, s);
    cout << lineaGuiones <<endl;
}
