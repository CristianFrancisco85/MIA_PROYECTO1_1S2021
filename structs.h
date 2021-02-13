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