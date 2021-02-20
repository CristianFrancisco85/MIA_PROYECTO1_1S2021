#pragma once
#ifndef STRUCTS_H
#define STRUCTS_H

#endif // STRUCTS_H

#include <time.h>
#include <string>

/**
 * Particiones Primarias.
 * Part Status 0 existe, 1 no existe y 2 si esta montada
*/
struct Partition{
public:
    char part_status = '0';
    char part_type = '0';
    char part_fit = '0';
    int part_start = -1;
    int part_size = 0;
    char part_name[16] = "";
};

/**
 * Master Boot Record
*/
struct MBR{
public:
    int mbr_tamano;
    time_t mbr_fecha_creacion;
    int mbr_disk_signature;
    char disk_fit;
    Partition mbr_partitions[4];
};

/**
 * Extended Boot Record
*/
struct EBR{
    char part_status='0';
    char part_fit='0';
    int part_start=-1;
    int part_size=0;
    int part_next=-1;
    char part_name[16];
};

/**
 * SuperBloque
*/
struct SuperBloque{
    int s_filesystem_type; //Guarda el numero que identifica al sistema de archivos utilizados
    int s_inodes_count; //Guarda el numero total de inodos
    int s_blocks_count; //Guarda el numero total de bloques
    int s_free_blocks_count; //Contiene el numero de bloques libres
    int s_free_inodes_count; //Contiene el numero inodos libres
    time_t s_mtime; //Ultima fecha en el que el sistema fue montado
    time_t s_umtime; //Ultima fecha en que el sistema fue desmontado
    int s_mnt_count; //Indica cuantas veces se ha montado el sistema
    int s_magic; //Valor que identifica al sistema de archivos 0xEF53
    int s_inode_size; //Tamaño del inodo
    int s_block_size; //Tamaño del bloque
    int s_first_ino; //Primer inodo libre
    int s_first_blo; //Primero bloque libre
    int s_bm_inode_start; //Guardara el inicio del bitmap de inodos
    int s_bm_block_start; //Guardara el inicio del bitmap de bloques
    int s_inode_start; //Guardara el inicio de la tabla de inodos
    int s_block_start; //Guardara el inicio de la tabla de bloques
};

/**
 * Tabla de Inodos
*/
struct InodeTable{
    int i_uid; //UID del usuario propiertario del archivo/carpeta
    int i_gid; //GID del grupo al que pertenece el archivo/carpeta
    int i_size; //Tamaño del archivo en bytes
    time_t i_atime; //Ultima fecha en que se leyo el inodo sin modificarlo
    time_t i_ctime; //Fecha en que se creo el el inodo
    time_t i_mtime; //Ultima fecha en la que se modifco
    int i_block[15]; //Primeros 12 bloques directos. 13 Bloque Simple Indirecto. 14 Bloque Doble Indirecto. 15  Bloque Triple Indirecto.
    char i_type; //Indica si es archivo o carpeta 1=Archivo 0 =Carpeta
    int i_perm; //Guardara los permisos del archivo o carpeta
    
};

/* Struct para guardar un registro de las operaciones que se hacen el sistema
 * de archivos ejemplo: creacion de carpetas o archivos */
struct Journal{
    char journal_operation_type[10];
    int journal_type;//Archivo o Carpeta
    char journal_name[100];
    char journal_content[100];
    time_t journal_date;
    int journal_owner;
    int journal_permissions;
};

struct Content{
    char b_name[12];//Nombre carpeta o archivo
    int b_inodo;//Apuntador hacia un inodo asociado al archivo o carpeta
};

struct BloqueCarpetas{
    Content b_content[4];//Array con el contenido de la carpeta
};

struct BloqueArchivos{
    char b_content[64]; //Array con el contenido del archivo
};

struct BloqueApuntadores{
    int b_pointers [16];//Array con los apuntadores hacia bloques
};

