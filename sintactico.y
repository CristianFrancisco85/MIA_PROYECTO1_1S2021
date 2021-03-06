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
#include <REP.h>
#include <MKFS.h>
#include <LOGIN.h>
#include <MKGRP.h>
#include <RMGRP.h>
#include <MKUSR.h>
#include <RMUSR.h>
#include <MKFILE.h>
#include <MKDIR.h>
#include <LOSS.h>
#include <RECOVERY.h>
#include <CAT.h>
#include <list>
#include <structs.h>


using namespace std;

extern int yylex(void);
extern char *yytext;
extern int SourceLine;
extern FILE *yyin;
void yyerror(const char *s);

list<MOUNT_>* mounted = new list<MOUNT_>();

MKDISK_* mkdisk_ = new MKDISK_();
RMDISK_* rmdisk_ = new RMDISK_();
FDISK_* fdisk_ = new FDISK_();
MOUNT_* mount_ = new MOUNT_();
UNMOUNT_* unmount_ = new UNMOUNT_();
REP_* rep_ = new REP_();
MKFS_* mkfs_ = new MKFS_();
LOGIN_* login_ = new LOGIN_();
MKGRP_* mkgrp_ = new MKGRP_();
RMGRP_* rmgrp_ = new RMGRP_();
MKUSR_* mkusr_ = new MKUSR_();
RMUSR_* rmusr_ = new RMUSR_();
MKFILE_* mkfile_ = new MKFILE_();
MKDIR_* mkdir_= new MKDIR_();
LOSS_* loss_= new LOSS_();
RECOVERY_* recovery_= new RECOVERY_();
CAT_* cat_= new CAT_();

bool loged = false;
Sesion sesion;

string lineaGuiones="------------------------------------------------------------------------------------------------------------------------------------------------";

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
%token<STRING> login
%token<STRING> usr
%token<STRING> pwd
%token<STRING> logout
%token<STRING> mkgrp
%token<STRING> rmgrp
%token<STRING> grp
%token<STRING> mkusr
%token<STRING> rmusr
%token<STRING> mkfile
%token<STRING> r
%token<STRING> cont
%token<STRING> mkdir
%token<STRING> loss
%token<STRING> recovery
%token<STRING> ruta_
%token<STRING> filen
%token<STRING> cat

%token<STRING> rep
%token<STRING> mbr
%token<STRING> disk
%token<STRING> inode
%token<STRING> block
%token<STRING> bm_block
%token<STRING> bm_inode
%token<STRING> sb
%token<STRING> journaling
%token<STRING> tree
%token<STRING> file_

%token<STRING> _2fs
%token<STRING> _3fs

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

%type<STRING> REP
%type<STRING> REPPARAMS
%type<STRING> REPPARAM

%type<STRING> MKFS
%type<STRING> MKFSPARAMS
%type<STRING> MKFSPARAM

%type<STRING> LOGIN
%type<STRING> LOGINPARAMS
%type<STRING> LOGINPARAM

%type<STRING> LOGOUT

%type<STRING> MKGRP

%type<STRING> RMGRP

%type<STRING> MKUSR
%type<STRING> MKUSRPARAMS
%type<STRING> MKUSRPARAM

%type<STRING> RMUSR

%type<STRING> MKFILE
%type<STRING> MKFILEPARAMS
%type<STRING> MKFILEPARAM

%type<STRING> MKDIR
%type<STRING> MKDIRPARAMS
%type<STRING> MKDIRPARAM

%type<STRING> LOSS

%type<STRING> RECOVERY

%type<STRING> CAT
%type<STRING> CATPARAMS
%type<STRING> CATPARAM

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
    |REP
    |MKFS
    |LOGIN
    |LOGOUT
    |MKGRP
    |RMGRP
    |MKUSR
    |RMUSR
    |MKFILE
    |MKDIR
    |LOSS
    |RECOVERY
    |CAT
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
    | guion type igual p {fdisk_->setPartitionType($4);}
    | guion type igual e {fdisk_->setPartitionType($4);}
    | guion type igual l {fdisk_->setPartitionType($4);}
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

REP: 
    rep REPPARAMS{rep_->initRep();cout<<lineaGuiones<<endl;free(rep_);rep_ = new REP_();}
;

REPPARAMS: 
    REPPARAM
    | REPPARAMS REPPARAM
;

REPPARAM: 
    guion path igual ruta {rep_->setPath($4);}
    | guion path igual cadena {rep_->setPath($4);}
    | guion id_ igual id {rep_->setId($4);}
    | guion id_ igual cadena {rep_->setId($4);}
    | guion name igual mbr {rep_->setName($4);}
    | guion name igual disk {rep_->setName($4);}
    | guion name igual inode {rep_->setName($4);}
    | guion name igual block {rep_->setName($4);}
    | guion name igual bm_inode {rep_->setName($4);}
    | guion name igual bm_block {rep_->setName($4);}
    | guion name igual sb {rep_->setName($4);}
    | guion name igual journaling {rep_->setName($4);}
    | guion name igual tree {rep_->setName($4);}
    | guion name igual file_ {rep_->setName($4);}
    | guion ruta_ igual ruta {rep_->setRuta($4);}
    | guion ruta_ igual cadena {rep_->setRuta($4);}
;

MKFS: 
    mkfs MKFSPARAMS {mkfs_->initFormat();cout<<lineaGuiones<<endl; mkfs_ = new MKFS_();}
;

MKFSPARAMS: 
    MKFSPARAM
    | MKFSPARAMS MKFSPARAM
;

MKFSPARAM: 
    guion id_ igual id {mkfs_->setId($4);}
    | guion id_ igual cadena {mkfs_->setId($4);}
    | guion type igual full {mkfs_->setTypeFormat($4);}
    | guion type igual fast {mkfs_->setTypeFormat($4);}
    | guion fs igual _2fs {mkfs_->setType($4);}
    | guion fs igual _3fs {mkfs_->setType($4);}
;

LOGIN: 
    login LOGINPARAMS {login_->initLog();cout<<lineaGuiones<<endl; login_ = new LOGIN_();}
;

LOGINPARAMS:  
    LOGINPARAM LOGINPARAMS
    | LOGINPARAM
;

LOGINPARAM: 
    guion usr igual id {login_->setUser($4);}
    | guion usr igual cadena {login_->setUser($4);}
    | guion pwd igual id {login_->setPassword($4);}
    | guion pwd igual numero {login_->setPassword($4);}
    | guion pwd igual cadena {login_->setPassword($4);}
    | guion id_ igual id {login_->setId($4);}
    | guion id_ igual cadena {login_->setId($4);}
;

LOGOUT: 
    logout {login_->logout();cout<<lineaGuiones<<endl;}
;

MKGRP: 
    mkgrp guion name igual id {mkgrp_->setName($5);mkgrp_->init();cout<<lineaGuiones<<endl;mkgrp_ = new MKGRP_();}
    | mkgrp guion name igual cadena {mkgrp_->setName($5);mkgrp_->init();cout<<lineaGuiones<<endl;mkgrp_ = new MKGRP_();}
;

RMGRP: 
    rmgrp guion name igual id {rmgrp_->setName($5);rmgrp_->init();cout<<lineaGuiones<<endl;rmgrp_ = new RMGRP_();}
    | rmgrp guion name igual cadena {rmgrp_->setName($5);rmgrp_->init();cout<<lineaGuiones<<endl;rmgrp_ = new RMGRP_();}
;

MKUSR: 
    mkusr MKUSRPARAMS {mkusr_->init();cout<<lineaGuiones<<endl; mkusr_ = new MKUSR_();}
;

MKUSRPARAMS: 
    MKUSRPARAM MKUSRPARAMS
    | MKUSRPARAM
;

MKUSRPARAM: 
    guion usr igual id  {mkusr_->setUser($4);}
    | guion usr igual cadena {mkusr_->setUser($4);}
    | guion pwd igual id {mkusr_->setPassword($4);}
    | guion pwd igual cadena {mkusr_->setPassword($4);}
    | guion pwd igual numero {mkusr_->setPassword($4);}
    | guion grp igual id {mkusr_->setGroup($4);}
    | guion grp igual cadena {mkusr_->setGroup($4);}
;

RMUSR: 
    rmusr guion usr igual id {rmusr_->setUser($5);rmusr_->init();cout<<lineaGuiones<<endl;rmusr_ = new RMUSR_();}
    | rmusr guion usr igual cadena {rmusr_->setUser($5);rmusr_->init();cout<<lineaGuiones<<endl;rmusr_ = new RMUSR_();}
;

MKFILE:
    mkfile MKFILEPARAMS {mkfile_->init();cout<<lineaGuiones<<endl; mkfile_ = new MKFILE_();}
;

MKFILEPARAMS:
    MKFILEPARAM MKFILEPARAMS
    | MKFILEPARAM
;

MKFILEPARAM:
    guion r {mkfile_->setRParam();}
    | guion path igual ruta {mkfile_->setPath($4);}
    | guion path igual cadena {mkfile_->setPath($4);}
    | guion size igual numero {mkfile_->setSize($4);}
    | guion size igual numero_negativo {mkfile_->setSize($4);}
    | guion cont igual ruta {mkfile_->setCont($4);}
    | guion cont igual cadena {mkfile_->setCont($4);}

;

MKDIR:
    mkdir MKDIRPARAMS {mkdir_->init();cout<<lineaGuiones<<endl; mkdir_ = new MKDIR_();}
;

MKDIRPARAMS:
    MKDIRPARAM MKDIRPARAMS
    | MKDIRPARAM
;

MKDIRPARAM:
    guion p {mkdir_->setPParam();}
    | guion path igual ruta {mkdir_->setPath($4);}
    | guion path igual cadena {mkdir_->setPath($4);}
;

LOSS:
    loss guion id_ igual id {loss_->setId($5);loss_->init();cout << lineaGuiones <<endl;loss_= new LOSS_();}
;

RECOVERY:
    recovery guion id_ igual id {recovery_->setId($5);recovery_->init();cout << lineaGuiones <<endl;recovery_= new RECOVERY_();}
;

CAT:
    cat CATPARAMS {cat_->init();cout<<lineaGuiones<<endl; cat_ = new CAT_();}
;

CATPARAMS:
    CATPARAM CATPARAMS
    | CATPARAM
;

CATPARAM:
    guion filen igual ruta {cat_->addPath($4);}
    | guion filen igual cadena {cat_->addPath($4);}
;

%%

void yyerror(const char *s){
    printf("Error sintactico en la linea %i: %s\n", SourceLine, s);
    cout << lineaGuiones <<endl;
}
