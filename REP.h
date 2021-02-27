#pragma once
#ifndef REP_H
#define REP_H

#endif // REP_H

#include "MOUNT.h"
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <structs.h>
#include "parser.h"
#include <libgen.h>
#include <chrono>
#include <thread>
#include <list>
#include <iterator>
#include <structs.h>


using namespace std;

class REP_{
private:
    char *name;// Nombre del reporte
    string path;//Ruta con nombre del reporte a generar
    char *id;//ID de la particion
    bool statusFlag;//Indica si hay errores en el comando
    char *ruta; // Ruta del disco

public:
    /**
     * Constructor
    */
    REP_(){
        this->name=0;
        this->path="";
        this->id=0;
        this->statusFlag=false;
    }
    
    /**
     * Setter del name
     * @param name: Nombre del reporte a generar
    */
    void setName(char* name);

    /**
     * Setter del ID
     * @param id: Id de la particion montada
    */
    void setId(char* id);

    /**
     * Setter del path
     * @param path: Ruta con nombre del reporte a generar
    */
    void setPath(char* path);

    /*
     * Getter del ID
    */
    char* getId();
 
    /**
     * Getter del name
    */
    char* getName();

    /**
     * Getter de la ruta del disco
    */
    char* getRuta();

    /**
     *Configura ruta del disco
    */
    void configDiskPath();

    /**
     * Inicia la ejecucion del comando
    */
    void initRep();

    /**
     *Convierte a minusculas un string 
    */
    string toLowerString(string input);

    /**
     * Obtiene el directorio del archivo
    */
    string dirNam(string);

    /**
     * Generar reporte del MBR
    */
    void reportMBR();

    /**
     * Generar reporte del Disco
    */
    void reportDisk();

    /**
     * Reporte de Inodos
    */
    void reportInode();

    /**
     * Reporte de Bloques
    */
    void reportBloque();

    /**
     * Reporte de BitMap de Inodos
    */
    void reportBitMapInodos();

    /**
     * Reporte de BitMap de Bloques
    */
    void reportBitMapBloques();

    /**
     * Reporte del SuperBloque
    */
    void reportSuperBloque();


    /**
     * Verifica si no hay errorres en el comando
    */
    void setStatus();

    /**
     * Indica si la particion esta montada
    */
    bool isMounted();

};

extern list<MOUNT_> *mounted;

void REP_::setName(char* name){
    this->name = name;
}

void REP_::setPath(char *value){
    path = value;
    if(path[0] == '\"'){
        path = path.substr(1, path.size()-2);
    }  
}

void REP_::setId(char *value){
    if(value[0] == '\"'){
        string aux = id;
        aux = aux.substr(1,aux.size()-2);
        strcpy(this->id, aux.c_str());
    }
    else{
        this->id = value;
    }
}

char* REP_::getId(){
    return this->id;
}

char* REP_::getName(){
    return  this->name;
}

char* REP_::getRuta(){
    return  this->ruta;
}

void REP_::setStatus(){

    this->statusFlag=false;

    if(this->id ==0 ){
        cout<< "\u001B[31m" << "[BAD PARAM] ID no valido" << "\x1B[0m" << endl;
        return;
    }
    if(this->path == ""){
        cout<< "\u001B[31m" << "[BAD PARAM] Path no valido" << "\x1B[0m" << endl;
        return;
    }
    if(this->name ==0 ){
        cout<< "\u001B[31m" << "[BAD PARAM] Name no valido" << "\x1B[0m" << endl;
        return;
    }

    this->statusFlag=true;
}

bool REP_::isMounted(){
    list<MOUNT_>::iterator i;
    for(i = mounted->begin();i != mounted->end();i++){
        if(i->getId() == this->id){
            return  true;
        }
    }
    return  false;
}

void REP_::initRep(){
    setStatus();
    if(this->statusFlag){
        if(strcmp(this->getName(),"mbr")==0){
            reportMBR();
        }
        else if(strcmp(this->getName(),"disk")==0){
            reportDisk();
        }
        else if(strcmp(this->getName(),"inode")==0){
            reportInode();
        }
        else if(strcmp(this->getName(),"block")==0){
            reportBloque();
        }
        else if(strcmp(this->getName(),"bm_inode")==0){
            reportBitMapInodos();
        }
        else if(strcmp(this->getName(),"bm_block")==0){
            reportBitMapBloques();
        }
        else if(strcmp(this->getName(),"sb")==0){
            reportSuperBloque();
        }
    }
}

void REP_::configDiskPath(){
    //Se busca particion y su ruta
    list<MOUNT_>::iterator i;
    for(i=mounted->begin(); i != mounted->end(); i++){
        if(i->getId() == this->id){
            char aux[255];
            strcpy(aux,i->getPath().c_str());
            this->ruta =aux;
            break;
        }
    }
}

//Metodos de reportes

void REP_::reportMBR(){
    this->configDiskPath();
    string diskPath = this->getRuta();
    FILE *file;
    file=fopen(diskPath.c_str(),"rb+");
    if(file != NULL){

        string dotCode = "digraph{\n";
        dotCode+= "MBR_TABLE[\n";
        dotCode+= "shape=none;label=<\n";
        dotCode+= "<TABLE width='600' height= '1000' cellspacing='-1' cellborder='1'>\n";
        dotCode+= "<tr>\n";
        dotCode+= "<td width='300'>Nombre</td><td width='300'>Valor</td></tr>";
        
        //Se lee el MBR
        MBR master;
        fseek(file,0,SEEK_SET);
        fread(&master,sizeof(MBR),1,file);
        Partition *mbr_partitions[4];
        mbr_partitions[0]=&master.mbr_partition_1;
        mbr_partitions[1]=&master.mbr_partition_2;
        mbr_partitions[2]=&master.mbr_partition_3;
        mbr_partitions[3]=&master.mbr_partition_4;

        //Se escribe Tamaño
        dotCode += "<tr>";
        dotCode += "<td>mbr_tamaño</td>";
        dotCode += "<td>"+to_string(master.mbr_tamano)+" bytes</td>";
        dotCode += "</tr>\n";

        //Se escribe Fecha de Creacion
        struct tm *timeStruct;
        char fecha[50];

        timeStruct = localtime(&master.mbr_fecha_creacion);
        strftime(fecha,100,"%d/%m/%y %H:%M", timeStruct);
        dotCode += "<tr>";
        dotCode += "<td>mbr_fecha_creacion</td>";
        dotCode += "<td>";
        dotCode += fecha;
        dotCode += "</td></tr>\n";

        //Se escribe Signature
        dotCode += "<tr>";
        dotCode += "<td>mbr_disk_signature</td>";
        dotCode += "<td>"+to_string(master.mbr_disk_signature)+"</td>";
        dotCode += "</tr>\n";

        //Se escribe Fit
        dotCode += "<tr>";
        dotCode += "<td>Disk_fit</td>";
        dotCode += "<td>"+to_string(master.disk_fit)+"</td>";
        dotCode += "</tr>\n";


        int extIndex = -1;
        //Se recorren particiones y se escriben sus datos
        for (int i = 0; i < 4; i++){
            if(mbr_partitions[i]->part_status!='1' && mbr_partitions[i]->part_start!=-1){
                if(mbr_partitions[i]->part_type == 'E'){
                    extIndex = i;
                }

                //Se escribe Status
                dotCode += "<tr>";
                dotCode += "<td>part_status_"+to_string(i+1)+"</td><td>";
                dotCode += mbr_partitions[i]->part_status;
                dotCode += "</td></tr>\n";

                //Se escribe Type
                dotCode += "<tr>";
                dotCode += "<td>part_type_"+to_string(i+1)+"</td><td>";
                dotCode += mbr_partitions[i]->part_type;
                dotCode += "</td></tr>\n";

                //Se escribe Fit
                dotCode += "<tr>";
                dotCode += "<td>part_fit_"+to_string(i+1)+"</td><td>";
                dotCode += mbr_partitions[i]->part_fit;
                dotCode += "</td></tr>\n";

                //Se escribe Start
                dotCode += "<tr>";
                dotCode += "<td>part_start_"+to_string(i+1)+"</td>";
                dotCode += "<td>"+to_string(mbr_partitions[i]->part_start)+"</td>";
                dotCode += "</tr>\n";

                //Se escribe Size
                dotCode += "<tr>";
                dotCode += "<td>part_size_"+to_string(i+1)+"</td>";
                dotCode += "<td>"+to_string(mbr_partitions[i]->part_size)+" bytes</td>";
                dotCode += "</tr>\n";

                //Se escribe Name
                dotCode += "<tr>";
                dotCode += "<td>part_name_"+to_string(i+1)+"</td>";
                dotCode += "<td>";
                dotCode += mbr_partitions[i]->part_name;
                dotCode += "</td> </tr>\n";

            }
        }
        
        dotCode += "</TABLE>\n>];\n";

        //Si hay EBR para graficar
        if(extIndex != -1){
            int ebrIndex = 1;
            EBR ebr;
            fseek(file,mbr_partitions[extIndex]->part_start,SEEK_SET);
            //Se leen todos los EBR y se grafica
            while(fread(&ebr,sizeof(EBR),1,file)!=0) {

                if(ebr.part_status != '1'){
                    dotCode+= "subgraph cluster_"+to_string(ebrIndex)+"{\n";
                    dotCode+= "label=\"EBR_"+to_string(ebrIndex)+"\"\n";
                    dotCode+= "table_"+to_string(ebrIndex)+"[shape=box, label=<\n ";
                    
                    dotCode+= "<TABLE width='400' height='200' cellspacing='-1' cellborder='1' >\n";
                    dotCode+= "<tr><td width='200'>Nombre</td><td width='200'>Valor</td></tr>\n";

                    //Se escribe Status
                    dotCode += "<tr>";
                    dotCode += "<td>part_status_1</td><td>";
                    dotCode += +ebr.part_status;
                    dotCode += "</td></tr>\n";

                    //Se escribe Fit
                    dotCode += "<tr>";
                    dotCode += "<td>part_fit_1</td><td>";
                    dotCode += ebr.part_fit;
                    dotCode += "</td></tr>\n";

                    //Se escribe Start
                    dotCode += "<tr>";
                    dotCode += "<td>part_start_1</td>";
                    dotCode += "<td>"+to_string(ebr.part_start)+"</td>";
                    dotCode += "</tr>\n";

                    //Se escribe Size
                    dotCode += "<tr>";
                    dotCode += "<td>part_size_1</td>";
                    dotCode += "<td>"+to_string(ebr.part_size)+" bytes</td>";
                    dotCode += "</tr>\n";

                    //Se escribe Next
                    dotCode += "<tr>";
                    dotCode += "<td>part_next_1</td>";
                    dotCode += "<td>"+to_string(ebr.part_next)+"</td>";
                    dotCode += "</tr>\n";

                    //Se escribe Name
                    dotCode += "<tr>";
                    dotCode += "<td>part_name_1</td>";
                    dotCode += "<td>";
                    dotCode += ebr.part_name;
                    dotCode += "</td>";
                    dotCode += "</tr>\n";
                    dotCode += "</TABLE>\n>];\n}";

                    ebrIndex++;
                }
                if(ebr.part_next == -1){
                    break;
                }
                else{
                    fseek(file,ebr.part_next,SEEK_SET);
                }
            }
        }
        dotCode+="}\n";
        fclose(file);

        // Obtener la ruta.
        string pathSinExt = this->path;
        string extension;
        const size_t lastPoint = pathSinExt.find_last_of(".");
        if (string::npos != lastPoint){
            extension = pathSinExt.substr(lastPoint, pathSinExt.length());
            pathSinExt = pathSinExt.substr(0, lastPoint);
        }
        string auxPath2 = pathSinExt;
        auxPath2 += ".txt";


        string pathSinName = this->path;
        const size_t lastSlash = pathSinName.find_last_of("/");
        if (string::npos != lastSlash){
            pathSinName = pathSinExt.substr(0, lastSlash);
        }

        string comando;

        //Se crea carpeta si se necesita
        comando= "sudo mkdir -p \'";
        comando+= pathSinName;
        comando+= '\'';
        system(comando.c_str());

        //Se conceden permisos
        comando = "sudo chmod -R 777 \'";
        comando+= pathSinName;
        comando += '\'';
        system(comando.c_str());

        // Se crea el .dot
        FILE *dotFile = fopen(auxPath2.c_str(),"w+");
        freopen(NULL,"w+",dotFile);
        char charArr[dotCode.length()];
        strcpy(charArr,dotCode.c_str());
        fprintf(dotFile,"%s\n",charArr);
        fclose(dotFile);
        comando = "dot \'";
        comando += auxPath2;
        comando +="\' -o \'" + pathSinExt + extension + " \' -T" + extension.substr(1,extension.length()-1);
        this_thread::sleep_for(chrono::milliseconds(1000));
        system(comando.c_str());
        this_thread::sleep_for(chrono::milliseconds(1000));
        //Se elimina archivo de residuo
        comando = "sudo rm '";
        comando += auxPath2 + "\'";
        system(comando.c_str());
        this_thread::sleep_for(chrono::milliseconds(1000));
        cout<< "\u001B[32m" << "[OK] Reporte MBR creado exitosamente"<< "\x1B[0m" << endl;
    }
    else{
        cout<< "\u001B[31m" << "[BAD PARAM] El disco no existe"<< "\x1B[0m" << endl;
    }

}

void REP_::reportDisk(){
    this->configDiskPath();
    string diskPath = this->getRuta();
    FILE *file;
    if((file = fopen(diskPath.c_str(),"r"))){
        string dotCode ="digraph G{\n";
        dotCode+= "DiskTable[\n";
        dotCode+= "shape=box;label=<\n";
        dotCode+= "<TABLE border='0' cellborder='2' width='500' height='200'>\n";
        dotCode+= "<tr>\n";
        dotCode+= "<td height='180' width='100'> MBR </td>\n";

        //Se lee MBR
        MBR master;
        fseek(file,0,SEEK_SET);
        fread(&master,sizeof(MBR),1,file);
        Partition *mbr_partitions[4];
        mbr_partitions[0]=&master.mbr_partition_1;
        mbr_partitions[1]=&master.mbr_partition_2;
        mbr_partitions[2]=&master.mbr_partition_3;
        mbr_partitions[3]=&master.mbr_partition_4;

        double usedSpaceTotal = 0;
        for(int i = 0; i < 4; i++){
            if(mbr_partitions[i]->part_start > -1){

                double usedSpace = (mbr_partitions[i]->part_size*100)/master.mbr_tamano;
                usedSpaceTotal += usedSpace;
                
                if(mbr_partitions[i]->part_status != '1'){
                    if(mbr_partitions[i]->part_type == 'P'){
                        dotCode+=  "<td height='180' width='200'> Primaria<br/>"+to_string(usedSpace)+"% del disco</td>\n";
                        //Se verifica si no hay espacio libre
                        int partEnd = mbr_partitions[i]->part_start + mbr_partitions[i]->part_size;
                        if(i!=3){
                            int exact = mbr_partitions[i+1]->part_start;
                            if(mbr_partitions[i+1]->part_start != -1){
                                if((exact-partEnd)!=0){
                                    double aux =  ((exact-partEnd)*100)/master.mbr_tamano;
                                    aux = (aux*500)/100;
                                    dotCode+= "<td height='120' width='"+to_string(aux)+"'>LIBRE<br/>"+"% del disco</td>\n";
                                }
                            }
                        }
                        else{
                            int mbrSize = master.mbr_tamano + (int)sizeof(MBR);
                            if((master.mbr_tamano + (int)sizeof(MBR)-partEnd)!=0){
                                double freeSpace = (mbrSize - partEnd) + sizeof(MBR);
                                freeSpace = (freeSpace*100)/master.mbr_tamano;
                                dotCode+=  "<td height='120' width='"+to_string(freeSpace*4)+"'>Libre<br/>"+to_string(freeSpace)+"% del disco</td>\n";
                            }
                        }
                    }
                    else{    
                        dotCode+= "<td height='200' width='100'>\n";
                        dotCode+= "<TABLE border='0'  height='180' width='100' cellborder='1'>\n";
                        dotCode+= "<tr>  <td height='60' colspan='15'>Extendida</td>  </tr>\n<tr>\n";

                        //Se lee el EBR
                        EBR ebr;
                        fseek(file, mbr_partitions[i]->part_start,SEEK_SET);
                        fread(&ebr,sizeof(EBR),1,file);

                        if(ebr.part_size != 0){
                            fseek(file, mbr_partitions[i]->part_start,SEEK_SET);
                            while(fread(&ebr,sizeof (EBR),1,file)!=0){

                                double porcentaje = (ebr.part_size*100)/master.mbr_tamano;
                                if(porcentaje!=0){
                                    if(ebr.part_status != '1'){
                                        dotCode+=  "<td height='120' width='75'> EBR </td>\n";
                                        dotCode+=  "<td height='120' width='150'>Logica<br/>"+to_string(porcentaje)+"% del disco</td>\n";
                                    }
                                    else{
                                        dotCode+=  "<td height='120' width='150'>Libre<br/>"+to_string(porcentaje)+"% del disco</td>\n";
                                    }
                                    if(ebr.part_next==-1){
                                        porcentaje = (mbr_partitions[i]->part_start + mbr_partitions[i]->part_size) - (ebr.part_start + ebr.part_size);
                                        porcentaje = (porcentaje*100)/master.mbr_tamano;
                                        if(porcentaje!=0){
                                            dotCode+=  "<td height='120' width='150'>Libre<br/> "+to_string(porcentaje)+"% del disco</td>\n";
                                        }
                                        break;
                                    }
                                    else{
                                        fseek(file,ebr.part_next,SEEK_SET);
                                    }
                                }
                            }
                        }
                        else{
                            dotCode+= "<td height='120'>"+to_string(usedSpace)+"% del disco </td>";
                        }
                        dotCode+= "</tr>\n</TABLE>\n</td>\n";

                        if(i!=3){
                            int aux1 = mbr_partitions[i]->part_start + mbr_partitions[i]->part_size;
                            int aux2 = mbr_partitions[i+1]->part_start;
                            if(aux2 != -1){
                                if((aux2-aux1)!=0){
                                    double porcentaje = ((aux2-aux1)*100)/master.mbr_tamano;
                                    dotCode+= "<td height='120' width='200'>Libre<br/>"+to_string(porcentaje)+"% del disco</td>\n";
                                }
                            }
                        }
                        else{
                            int aux1 = mbr_partitions[i]->part_start + mbr_partitions[i]->part_size;
                            int mbrSize = master.mbr_tamano + (int)sizeof(MBR);
                            if((mbrSize-aux1)!=0){
                                double porcentaje = (mbrSize - aux1) + sizeof(MBR);
                                porcentaje = (porcentaje*100)/master.mbr_tamano;
                                dotCode+= "<td height='120' width='100'>Libre<br/>"+to_string(porcentaje)+"% del disco</td>\n";
                            }
                        }
                    }
                }
                else{
                    dotCode+= "<td height='180'width='200'>Libre <br/>"+to_string(usedSpace)+"% del disco</td>\n";
                }
            }
        }
        dotCode+= "</tr>\n</TABLE>\n>];\n}";
        fclose(file);

        // Obtener la ruta.
        string pathSinExt = this->path;
        string extension;
        const size_t lastPoint = pathSinExt.find_last_of(".");
        if (string::npos != lastPoint){
            extension = pathSinExt.substr(lastPoint, pathSinExt.length());
            pathSinExt = pathSinExt.substr(0, lastPoint);
        }
        string auxPath2 = pathSinExt;
        auxPath2 += ".txt";

        string pathSinName = this->path;
        const size_t lastSlash = pathSinName.find_last_of("/");
        if (string::npos != lastSlash){
            pathSinName = pathSinExt.substr(0, lastSlash);
        }

        string comando;

        //Se crea carpeta si se necesita
        comando= "sudo mkdir -p \'";
        comando+= pathSinName;
        comando+= '\'';
        system(comando.c_str());

        //Se conceden permisos
        comando = "sudo chmod -R 777 \'";
        comando+= pathSinName;
        comando += '\'';
        system(comando.c_str());

        // Se crea el .dot
        FILE *dotFile = fopen(auxPath2.c_str(),"w+");
        freopen(NULL,"w+",dotFile);
        char charArr[dotCode.length()];
        strcpy(charArr,dotCode.c_str());
        fprintf(dotFile,"%s\n",charArr);
        fclose(dotFile);
        comando = "dot \'";
        comando += auxPath2;
        comando +="\' -o \'" + pathSinExt + extension + " \' -T" + extension.substr(1,extension.length()-1);
        this_thread::sleep_for(chrono::milliseconds(1000));
        system(comando.c_str());
        this_thread::sleep_for(chrono::milliseconds(1000));
        //Se elimina archivo de residuo
        comando = "sudo rm '";
        comando += auxPath2 + "\'";
        system(comando.c_str());
        this_thread::sleep_for(chrono::milliseconds(1000));
        cout<< "\u001B[32m" << "[OK] Reporte Disk creado exitosamente"<< "\x1B[0m" << endl;
    }
    else{
        cout<< "\u001B[31m" << "[BAD PARAM] El disco no existe"<< "\x1B[0m" << endl;
    }
}

void REP_::reportInode(){
    
    //Se busca en las particiones montadas
    int partIndex = -1;
    MOUNT_ *disk;
    list<MOUNT_>::iterator i;
    for(i=mounted->begin(); i != mounted->end(); i++){
        if(i->getId() == this->id){
            disk = &*i;
            break;
        }
    }

    partIndex = disk->findPartitionIndex();

    if(partIndex != -1){

        //Se lee el MBR y Super Bloque
        MBR master;
        SuperBloque super;
        FILE *file = fopen(disk->getPath().c_str(),"r+b");
        fread(&master,sizeof(MBR),1,file);
        Partition *mbr_partitions[4];
        mbr_partitions[0]=&master.mbr_partition_1;
        mbr_partitions[1]=&master.mbr_partition_2;
        mbr_partitions[2]=&master.mbr_partition_3;
        mbr_partitions[3]=&master.mbr_partition_4;

        fseek(file,mbr_partitions[partIndex]->part_start,SEEK_SET);
        fread(&super,sizeof(SuperBloque),1,file);

        InodeTable auxInodo;
        int bitMapStart = super.s_bm_inode_start;

        string dotCode = "digraph G{\n\n";

        for(int i=0;bitMapStart < super.s_bm_block_start;i++){
            fseek(file,super.s_bm_inode_start + i,SEEK_SET);
            char myBit;
            myBit = fgetc(file);
            //Si el inodo se esta utilizando
            if(myBit == '1'){
                fseek(file,super.s_inode_start + sizeof(InodeTable)*i,SEEK_SET);
                fread(&auxInodo,sizeof(InodeTable),1,file);

                dotCode += " nodo_"+to_string(i)+"[ shape=none,label=<\n";
                dotCode += "<table border='0' cellborder='1' cellspacing='0'>";
                dotCode += "<tr> <td colspan='2'> <b>Inodo "+to_string(i)+"</b> </td></tr>\n";
                dotCode += "<tr> <td> i_uid </td> <td> "+to_string(auxInodo.i_gid)+"</td>  </tr>\n";
                dotCode += "<tr> <td> i_gid </td> <td> "+to_string(auxInodo.i_gid)+"</td>  </tr>\n";
                dotCode += "<tr> <td> i_size </td> <td> "+to_string(auxInodo.i_size)+"</td> </tr>\n";

                struct tm *timeStruct;
                char date[50];

                timeStruct=localtime(&auxInodo.i_atime);
                strftime(date,100,"%d/%m/%y %H:%M",timeStruct);
                dotCode += "<tr><td> i_atime </td><td>";
                dotCode += date;
                dotCode += "</td> </tr>\n";

                timeStruct=localtime(&auxInodo.i_ctime);
                strftime(date,100,"%d/%m/%y %H:%M",timeStruct);
                dotCode += " <tr><td> i_ctime </td><td>";
                dotCode += date;
                dotCode += "</td></tr>\n";

                timeStruct=localtime(&auxInodo.i_mtime);
                strftime(date,100,"%d/%m/%y %H:%M",timeStruct);
                dotCode += "<tr><td> i_mtime </td><td>";
                dotCode += "</td></tr>\n";

                for(int j = 0; j < 15; j++){
                    dotCode += "<tr><td> i_block_"+to_string(j)+"</td><td>"+to_string(auxInodo.i_block[j])+" </td> </tr>\n";
                }

                dotCode += "<tr><td> i_type </td><td> ";
                dotCode += auxInodo.i_type;
                dotCode += "</td></tr>\n";
                dotCode += "<tr><td> i_perm </td><td>"+to_string(auxInodo.i_perm)+"</td></tr>\n";
                dotCode += "</table>>]\n";
            }
            bitMapStart++;
        }
        dotCode +="\n}";
        fclose(file);

        //Obtener la ruta.
        string pathSinExt = this->path;
        string extension;
        const size_t lastPoint = pathSinExt.find_last_of(".");
        if (string::npos != lastPoint){
            extension = pathSinExt.substr(lastPoint, pathSinExt.length());
            pathSinExt = pathSinExt.substr(0, lastPoint);
        }
        string auxPath2 = pathSinExt;
        auxPath2 += ".txt";

        string pathSinName = this->path;
        const size_t lastSlash = pathSinName.find_last_of("/");
        if (string::npos != lastSlash){
            pathSinName = pathSinExt.substr(0, lastSlash);
        }

        string comando;

        //Se crea carpeta si se necesita
        comando= "sudo mkdir -p \'";
        comando+= pathSinName;
        comando+= '\'';
        system(comando.c_str());

        //Se conceden permisos
        comando = "sudo chmod -R 777 \'";
        comando+= pathSinName;
        comando += '\'';
        system(comando.c_str());

        // Se crea el .dot
        FILE *dotFile = fopen(auxPath2.c_str(),"w+");
        freopen(NULL,"w+",dotFile);
        char charArr[dotCode.length()];
        strcpy(charArr,dotCode.c_str());
        fprintf(dotFile,"%s\n",charArr);
        fclose(dotFile);
        comando = "dot \'";
        comando += auxPath2;
        comando +="\' -o \'" + pathSinExt + extension + " \' -T" + extension.substr(1,extension.length()-1);
        this_thread::sleep_for(chrono::milliseconds(1000));
        system(comando.c_str());
        this_thread::sleep_for(chrono::milliseconds(1000));
        //Se elimina archivo de residuo
        comando = "sudo rm '";
        comando += auxPath2 + "\'";
        system(comando.c_str());
        this_thread::sleep_for(chrono::milliseconds(1000));
        cout<< "\u001B[32m" << "[OK] Reporte de Tabla de Inodos creado exitosamente"<< "\x1B[0m" << endl;
    }
    else{

        partIndex = disk->findLogicPartitionStart();
        if(partIndex != -1){
            EBR extendedBoot;
            SuperBloque super;
            FILE *file = fopen(disk->getPath().c_str(),"r+b");
            fseek(file,partIndex,SEEK_SET);
            fread(&extendedBoot,sizeof(EBR),1,file);
            fread(&super,sizeof(SuperBloque),1,file);
             InodeTable auxInodo;
            int bitMapStart = super.s_bm_inode_start;

            string dotCode = "digraph G{\n\n";

            for(int i=0;bitMapStart < super.s_bm_block_start;i++){

                fseek(file,super.s_bm_inode_start + i,SEEK_SET);
                char myBit;
                myBit = fgetc(file);
                //Si el inodo se esta utilizando
                if(myBit == '1'){
                fseek(file,super.s_inode_start + sizeof(InodeTable)*i,SEEK_SET);
                fread(&auxInodo,sizeof(InodeTable),1,file);

                dotCode += " nodo_"+to_string(i)+"[ shape=none,label=<\n";
                dotCode += "<table border='0' cellborder='1' cellspacing='0'>";
                dotCode += "<tr> <td colspan='2'> <b>Inodo "+to_string(i)+"</b> </td></tr>\n";
                dotCode += "<tr> <td> i_uid </td> <td> "+to_string(auxInodo.i_gid)+"</td>  </tr>\n";
                dotCode += "<tr> <td> i_gid </td> <td> "+to_string(auxInodo.i_gid)+"</td>  </tr>\n";
                dotCode += "<tr> <td> i_size </td> <td> "+to_string(auxInodo.i_size)+"</td> </tr>\n";

                struct tm *timeStruct;
                char date[50];

                timeStruct=localtime(&auxInodo.i_atime);
                strftime(date,100,"%d/%m/%y %H:%M",timeStruct);
                dotCode += "<tr><td> i_atime </td><td>";
                dotCode += date;
                dotCode += "</td> </tr>\n";

                timeStruct=localtime(&auxInodo.i_ctime);
                strftime(date,100,"%d/%m/%y %H:%M",timeStruct);
                dotCode += " <tr><td> i_ctime </td><td>";
                dotCode += date;
                dotCode += "</td></tr>\n";

                timeStruct=localtime(&auxInodo.i_mtime);
                strftime(date,100,"%d/%m/%y %H:%M",timeStruct);
                dotCode += "<tr><td> i_mtime </td><td>";
                dotCode += "</td></tr>\n";

                for(int j = 0; j < 15; j++){
                    dotCode += "<tr><td> i_block_"+to_string(j)+"</td><td>"+to_string(auxInodo.i_block[j])+" </td> </tr>\n";
                }

                dotCode += "<tr><td> i_type </td><td> ";
                dotCode += auxInodo.i_type;
                dotCode += "</td></tr>\n";
                dotCode += "<tr><td> i_perm </td><td>"+to_string(auxInodo.i_perm)+"</td></tr>\n";
                dotCode += "</table>>]\n";
            }
            bitMapStart++;
        }
        dotCode +="\n}";
        fclose(file);

            //Obtener la ruta.
            string pathSinExt = this->path;
            string extension;
            const size_t lastPoint = pathSinExt.find_last_of(".");
            if (string::npos != lastPoint){
                extension = pathSinExt.substr(lastPoint, pathSinExt.length());
                pathSinExt = pathSinExt.substr(0, lastPoint);
            }
            string auxPath2 = pathSinExt;
            auxPath2 += ".txt";

            string pathSinName = this->path;
            const size_t lastSlash = pathSinName.find_last_of("/");
            if (string::npos != lastSlash){
                pathSinName = pathSinExt.substr(0, lastSlash);
            }

            string comando;

            //Se crea carpeta si se necesita
            comando= "sudo mkdir -p \'";
            comando+= pathSinName;
            comando+= '\'';
            system(comando.c_str());

            //Se conceden permisos
            comando = "sudo chmod -R 777 \'";
            comando+= pathSinName;
            comando += '\'';
            system(comando.c_str());

            // Se crea el .dot
            FILE *dotFile = fopen(auxPath2.c_str(),"w+");
            freopen(NULL,"w+",dotFile);
            char charArr[dotCode.length()];
            strcpy(charArr,dotCode.c_str());
            fprintf(dotFile,"%s\n",charArr);
            fclose(dotFile);
            comando = "dot \'";
            comando += auxPath2;
            comando +="\' -o \'" + pathSinExt + extension + " \' -T" + extension.substr(1,extension.length()-1);
            this_thread::sleep_for(chrono::milliseconds(1000));
            system(comando.c_str());
            this_thread::sleep_for(chrono::milliseconds(1000));
            //Se elimina archivo de residuo
            comando = "sudo rm '";
            comando += auxPath2 + "\'";
            system(comando.c_str());
            this_thread::sleep_for(chrono::milliseconds(1000));
            cout<< "\u001B[32m" << "[OK] Reporte de Tabla de Inodos creado exitosamente"<< "\x1B[0m" << endl;

        }
    }
}

void REP_::reportBloque(){

    //Se busca en las particiones montadas
    int partIndex = -1;
    MOUNT_ *disk;
    list<MOUNT_>::iterator i;
    for(i=mounted->begin(); i != mounted->end(); i++){
        if(i->getId() == this->id){
            disk = &*i;
            break;
        }
    }

    partIndex = disk->findPartitionIndex();

    if(partIndex != -1){
        MBR master;
        SuperBloque super;
        FILE *file = fopen(disk->getPath().data(),"r+b");
        fread(&master,sizeof(MBR),1,file);
        Partition *mbr_partitions[4];
        mbr_partitions[0]=&master.mbr_partition_1;
        mbr_partitions[1]=&master.mbr_partition_2;
        mbr_partitions[2]=&master.mbr_partition_3;
        mbr_partitions[3]=&master.mbr_partition_4;

        fseek(file,mbr_partitions[partIndex]->part_start,SEEK_SET);
        fread(&super,sizeof(SuperBloque),1,file);

        int aux = super.s_bm_block_start;
        char myChar;

        string dotCode = "digraph G{\n";

        for(int i = 0;aux < super.s_inode_start;i++){
            fseek(file,super.s_bm_block_start + i,SEEK_SET);
            myChar = fgetc(file);
            if(myChar == '1'){
                BloqueCarpetas carpetas;
                fseek(file,super.s_block_start + sizeof(BloqueCarpetas)*i,SEEK_SET);
                fread(&carpetas,sizeof(BloqueCarpetas),1,file);
                dotCode += "nodo_"+to_string(i) + "[shape=none,label=< \n";
                dotCode += "<table border='0' cellborder='1' cellspacing='0'>\n";
                dotCode += "<tr><td colspan='2'><b> Bloque Carpeta "+to_string(i)+"</b></td></tr>\n";
                dotCode += "<tr><td> b_name </td><td> b_inodo </td></tr>\n";
                for(int j = 0;j < 4;j++){
                    dotCode += "<tr><td>";
                    dotCode += carpetas.b_content[j].b_name;
                    dotCode += "</td><td>"+to_string(carpetas.b_content[j].b_inodo)+"</td></tr>\n";
                }
                dotCode += "</table>>]\n";
            }
            else if(myChar == '2'){
                BloqueArchivos archivos;
                fseek(file,super.s_block_start + sizeof(BloqueArchivos)*i,SEEK_SET);
                fread(&archivos,sizeof(BloqueArchivos),1,file);
                dotCode += "nodo_"+to_string(i) + "[shape=none,label=< \n";
                dotCode += "<table border='0' cellborder='1' cellspacing='0'>\n";
                dotCode += "<tr><td colspan='2'><b> Bloque Archivo "+to_string(i)+"</b> </td></tr>\n";
                dotCode += "<tr><td colspan='2'>";
                dotCode += archivos.b_content;
                dotCode += "</td></tr>\n";
                dotCode += "</table>>]\n";
            }
            else if(myChar == '3'){
                BloqueApuntadores apuntadores;
                fseek(file,super.s_block_start + sizeof(BloqueApuntadores)*i,SEEK_SET);
                fread(&apuntadores,sizeof(BloqueApuntadores),1,file);
                fseek(file,super.s_block_start + sizeof(BloqueApuntadores)*i,SEEK_SET);
                fread(&apuntadores,sizeof(BloqueApuntadores),1,file);
                dotCode += "bloque_"+to_string(i)+" [shape=plaintext,label=< \n";
                dotCode += "<table border='0'>\n";
                dotCode += "<tr><td><b>Bloque de Apuntadores "+to_string(i)+"</b></td></tr>\n";
                for(int j = 0; j < 16; j++){
                    dotCode += "<tr><td>"+to_string(apuntadores.b_pointers[j])+"</td></tr>\n";
                }
                dotCode += "</table>>]\n";
            }
            aux++;
        }
        dotCode +="}";
        fclose(file);

        //Obtener la ruta.
        string pathSinExt = this->path;
        string extension;
        const size_t lastPoint = pathSinExt.find_last_of(".");
        if (string::npos != lastPoint){
            extension = pathSinExt.substr(lastPoint, pathSinExt.length());
            pathSinExt = pathSinExt.substr(0, lastPoint);
        }
        string auxPath2 = pathSinExt;
        auxPath2 += ".txt";

        string pathSinName = this->path;
        const size_t lastSlash = pathSinName.find_last_of("/");
        if (string::npos != lastSlash){
            pathSinName = pathSinExt.substr(0, lastSlash);
        }

        string comando;

        //Se crea carpeta si se necesita
        comando= "sudo mkdir -p \'";
        comando+= pathSinName;
        comando+= '\'';
        system(comando.c_str());

        //Se conceden permisos
        comando = "sudo chmod -R 777 \'";
        comando+= pathSinName;
        comando += '\'';
        system(comando.c_str());

        // Se crea el .dot
        FILE *dotFile = fopen(auxPath2.c_str(),"w+");
        freopen(NULL,"w+",dotFile);
        char charArr[dotCode.length()];
        strcpy(charArr,dotCode.c_str());
        fprintf(dotFile,"%s\n",charArr);
        fclose(dotFile);
        comando = "dot \'";
        comando += auxPath2;
        comando +="\' -o \'" + pathSinExt + extension + " \' -T" + extension.substr(1,extension.length()-1);
        this_thread::sleep_for(chrono::milliseconds(1000));
        system(comando.c_str());
        this_thread::sleep_for(chrono::milliseconds(1000));
        //Se elimina archivo de residuo
        comando = "sudo rm '";
        comando += auxPath2 + "\'";
        //system(comando.c_str());
        this_thread::sleep_for(chrono::milliseconds(1000));
        cout<< "\u001B[32m" << "[OK] Reporte de Tabla de Bloques creado exitosamente"<< "\x1B[0m" << endl;



    }
    else{
        partIndex = disk->findLogicPartitionStart();
        if(partIndex != -1){
            EBR extendedBoot;
            SuperBloque super;
            FILE *file = fopen(disk->getPath().data(),"r+b");
            fseek(file,partIndex,SEEK_SET);
            fread(&extendedBoot,sizeof(EBR),1,file);
            fread(&super,sizeof(SuperBloque),1,file);

        int aux = super.s_bm_block_start;
        char myChar;

        string dotCode = "digraph G{\n";

        for(int i = 0;aux < super.s_inode_start;i++){
            fseek(file,super.s_bm_block_start + i,SEEK_SET);
            myChar = fgetc(file);
            if(myChar == '1'){
                BloqueCarpetas carpetas;
                fseek(file,super.s_block_start + sizeof(BloqueCarpetas)*i,SEEK_SET);
                fread(&carpetas,sizeof(BloqueCarpetas),1,file);
                dotCode += "nodo_"+to_string(i) + "[shape=none,label=< \n";
                dotCode += "<table border='0' cellborder='1' cellspacing='0'>\n";
                dotCode += "<tr><td colspan='2'><b> Bloque Carpeta "+to_string(i)+"</b></td></tr>\n";
                dotCode += "<tr><td> b_name </td><td> b_inodo </td></tr>\n";
                for(int j = 0;j < 4;j++){
                    dotCode += "<tr><td>";
                    dotCode += carpetas.b_content[j].b_name;
                    dotCode += "</td><td>"+to_string(carpetas.b_content[j].b_inodo)+"</td></tr>\n";
                }
                dotCode += "</table>>]\n";
            }
            else if(myChar == '2'){
                BloqueArchivos archivos;
                fseek(file,super.s_block_start + sizeof(BloqueArchivos)*i,SEEK_SET);
                fread(&archivos,sizeof(BloqueArchivos),1,file);
                dotCode += "nodo_"+to_string(i) + "[shape=none,label=< \n";
                dotCode += "<table border='0' cellborder='1' cellspacing='0'>\n";
                dotCode += "<tr><td colspan='2'><b> Bloque Archivo "+to_string(i)+"</b> </td></tr>\n";
                dotCode += "<tr><td colspan='2'>";
                dotCode += archivos.b_content;
                dotCode += "</td></tr>\n";
                dotCode += "</table>>]\n";
            }
            else if(myChar == '3'){
                BloqueApuntadores apuntadores;
                fseek(file,super.s_block_start + sizeof(BloqueApuntadores)*i,SEEK_SET);
                fread(&apuntadores,sizeof(BloqueApuntadores),1,file);
                fseek(file,super.s_block_start + sizeof(BloqueApuntadores)*i,SEEK_SET);
                fread(&apuntadores,sizeof(BloqueApuntadores),1,file);
                dotCode += "bloque_"+to_string(i)+" [shape=plaintext,label=< \n";
                dotCode += "<table border='0'>\n";
                dotCode += "<tr><td><b>Bloque de Apuntadores "+to_string(i)+"</b></td></tr>\n";
                for(int j = 0; j < 16; j++){
                    dotCode += "<tr><td>"+to_string(apuntadores.b_pointers[j])+"</td></tr>\n";
                }
                dotCode += "</table>>]\n";
            }
            aux++;
        }
        dotCode +="}";
        fclose(file);

        //Obtener la ruta.
        string pathSinExt = this->path;
        string extension;
        const size_t lastPoint = pathSinExt.find_last_of(".");
        if (string::npos != lastPoint){
            extension = pathSinExt.substr(lastPoint, pathSinExt.length());
            pathSinExt = pathSinExt.substr(0, lastPoint);
        }
        string auxPath2 = pathSinExt;
        auxPath2 += ".txt";

        string pathSinName = this->path;
        const size_t lastSlash = pathSinName.find_last_of("/");
        if (string::npos != lastSlash){
            pathSinName = pathSinExt.substr(0, lastSlash);
        }

        string comando;

        //Se crea carpeta si se necesita
        comando= "sudo mkdir -p \'";
        comando+= pathSinName;
        comando+= '\'';
        system(comando.c_str());

        //Se conceden permisos
        comando = "sudo chmod -R 777 \'";
        comando+= pathSinName;
        comando += '\'';
        system(comando.c_str());

        // Se crea el .dot
        FILE *dotFile = fopen(auxPath2.c_str(),"w+");
        freopen(NULL,"w+",dotFile);
        char charArr[dotCode.length()];
        strcpy(charArr,dotCode.c_str());
        fprintf(dotFile,"%s\n",charArr);
        fclose(dotFile);
        comando = "dot \'";
        comando += auxPath2;
        comando +="\' -o \'" + pathSinExt + extension + " \' -T" + extension.substr(1,extension.length()-1);
        this_thread::sleep_for(chrono::milliseconds(1000));
        system(comando.c_str());
        this_thread::sleep_for(chrono::milliseconds(1000));
        //Se elimina archivo de residuo
        comando = "sudo rm '";
        comando += auxPath2 + "\'";
        system(comando.c_str());
        this_thread::sleep_for(chrono::milliseconds(1000));
        cout<< "\u001B[32m" << "[OK] Reporte de Tabla de Bloques creado exitosamente"<< "\x1B[0m" << endl;

        }
    }
}

void REP_::reportBitMapInodos(){

    //Se busca en las particiones montadas
    int partIndex = -1;
    MOUNT_ *disk;
    list<MOUNT_>::iterator i;
    for(i=mounted->begin(); i != mounted->end(); i++){
        if(i->getId() == this->id){
            disk = &*i;
            break;
        }
    }

    partIndex = disk->findPartitionIndex();

    if(partIndex != -1){
        MBR master;
        SuperBloque super;
        FILE *file = fopen(disk->getPath().data(),"r+b");
        fread(&master,sizeof(MBR),1,file);
        Partition *mbr_partitions[4];
        mbr_partitions[0]=&master.mbr_partition_1;
        mbr_partitions[1]=&master.mbr_partition_2;
        mbr_partitions[2]=&master.mbr_partition_3;
        mbr_partitions[3]=&master.mbr_partition_4;

        fseek(file,mbr_partitions[partIndex]->part_start,SEEK_SET);
        fread(&super,sizeof(SuperBloque),1,file);

        char myByte;
        string fileData;
        int aux = 0;
        
        for (int i = 0; i < super.s_inodes_count; i++) {
            fseek(file,super.s_bm_inode_start + i,SEEK_SET);
            myByte = fgetc(file);
            if(myByte == '0'){
                fileData +="0 ";
            }
            else{
                fileData +="1 ";
            }
            if(aux == 19){
                aux = 0;
                fileData += "\n";
            }
            else{
                aux++;
            }
        }
        fclose(file);
        file = fopen(this->path.data(),"w");
        fprintf(file,"%s\n", fileData.data());
        fclose(file);
        
        cout<< "\u001B[32m" << "[OK] Reporte de Bitmap de Inodos creado exitosamente"<< "\x1B[0m" << endl;
    }
    else{
        partIndex = disk->findLogicPartitionStart();
        EBR extendedBoot;
        SuperBloque super;
        FILE *file = fopen(disk->getPath().data(),"rb+");
        fseek(file,partIndex,SEEK_SET);
        fread(&extendedBoot,sizeof(EBR),1,file);
        fread(&super,sizeof(SuperBloque),1,file);

        char myByte;
        int aux = 0;
        string fileData;
        for (int i = 0; i < super.s_inodes_count; i++) {
            fseek(file,super.s_bm_inode_start + i,SEEK_SET);
            myByte = fgetc(file);
            if(myByte == '0'){
                fileData +="0 ";
            }
            else{
                fileData +="1 ";
            }
            if(aux == 19){
                aux = 0;
                fileData += "\n";
            }
            else{
                aux++;
            }
        }
        fclose(file);
        file = fopen(this->path.data(),"w");
        fprintf(file,"%s\n", fileData.data());
        fclose(file);
        cout<< "\u001B[32m" << "[OK] Reporte de Bitmap de Inodos creado exitosamente"<< "\x1B[0m" << endl;
    }

}

void REP_::reportBitMapBloques(){

    //Se busca en las particiones montadas
    int partIndex = -1;
    MOUNT_ *disk;
    list<MOUNT_>::iterator i;
    for(i=mounted->begin(); i != mounted->end(); i++){
        if(i->getId() == this->id){
            disk = &*i;
            break;
        }
    }

    partIndex = disk->findPartitionIndex();

    if(partIndex != -1){
        MBR master;
        SuperBloque super;
        FILE *file = fopen(disk->getPath().data(),"r+b");
        fread(&master,sizeof(MBR),1,file);
        Partition *mbr_partitions[4];
        mbr_partitions[0]=&master.mbr_partition_1;
        mbr_partitions[1]=&master.mbr_partition_2;
        mbr_partitions[2]=&master.mbr_partition_3;
        mbr_partitions[3]=&master.mbr_partition_4;
        fseek(file,mbr_partitions[partIndex]->part_start,SEEK_SET);
        fread(&super,sizeof(SuperBloque),1,file);

        char myByte;
        string fileData;
        int aux = 0;

        for (int i = 0; i < super.s_blocks_count; i++) {
            fseek(file,super.s_bm_block_start + i,SEEK_SET);
            myByte = fgetc(file);
            if(myByte == '0'){
                fileData +="0 ";
            }
            else{
                fileData +="1 ";
            }
            if(aux == 19){
                aux = 0;
                fileData += "\n";
            }
            else{
                aux++;
            }
        }
        fclose(file);
        file = fopen(this->path.data(),"w");
        fprintf(file,"%s\n", fileData.data());
        fclose(file);
        
        cout<< "\u001B[32m" << "[OK] Reporte de Bitmap de Bloques creado exitosamente"<< "\x1B[0m" << endl;
    }
    else{
        partIndex = disk->findLogicPartitionStart();
        EBR extendedBoot;
        SuperBloque super;
        FILE *file = fopen(disk->getPath().data(),"rb+");
        fseek(file,partIndex,SEEK_SET);
        fread(&extendedBoot,sizeof(EBR),1,file);
        fread(&super,sizeof(SuperBloque),1,file);

        char myByte;
        int aux = 0;
        string fileData;
        for (int i = 0; i < super.s_blocks_count; i++) {
            fseek(file,super.s_bm_block_start + i,SEEK_SET);
            myByte = fgetc(file);
            if(myByte == '0'){
                fileData +="0 ";
            }
            else{
                fileData +="1 ";
            }
            if(aux == 19){
                aux = 0;
                fileData += "\n";
            }
            else{
                aux++;
            }
        }
        fclose(file);
        file = fopen(this->path.data(),"w");
        fprintf(file,"%s\n", fileData.data());
        fclose(file);
        cout<< "\u001B[32m" << "[OK] Reporte de Bitmap de Bloques creado exitosamente"<< "\x1B[0m" << endl;
    }
}

void REP_::reportSuperBloque(){
    //Se busca en las particiones montadas
    int partIndex = -1;
    MOUNT_ *disk;
    list<MOUNT_>::iterator i;
    for(i=mounted->begin(); i != mounted->end(); i++){
        if(i->getId() == this->id){
            disk = &*i;
            break;
        }
    }

    partIndex = disk->findPartitionIndex();

    if(partIndex != -1){
        MBR master;
        SuperBloque super;
        FILE *file = fopen(disk->getPath().data(),"r+b");
        fread(&master,sizeof(MBR),1,file);
        Partition *mbr_partitions[4];
        mbr_partitions[0]=&master.mbr_partition_1;
        mbr_partitions[1]=&master.mbr_partition_2;
        mbr_partitions[2]=&master.mbr_partition_3;
        mbr_partitions[3]=&master.mbr_partition_4;

        fseek(file,mbr_partitions[partIndex]->part_start,SEEK_SET);
        fread(&super,sizeof (super),1,file);

        string dotCode = "digraph G{\n";
        dotCode += "superBloqueTable[shape=none,label=<";
        dotCode += "<table border='0' cellborder='1' cellspacing='0'>\n";
        dotCode += "<tr><td colspan='2'><b> Super Bloque </b></td></tr>\n";
        dotCode += "<tr><td> s_inodes_count </td><td>"+to_string(super.s_inodes_count)+" </td> </tr>\n";
        dotCode += "<tr><td> s_blocks_count </td><td>"+to_string(super.s_blocks_count)+"</td></tr>\n";
        dotCode += "<tr><td> s_free_block_count </td><td>"+to_string(super.s_free_blocks_count)+"</td> </tr>\n";
        dotCode += "<tr><td> s_free_inodes_count </td><td>"+to_string(super.s_free_inodes_count)+"</td></tr>\n";

        struct tm *timeStruct;
        char fecha[50];

        timeStruct=localtime(&super.s_mtime);
        strftime(fecha,100,"%d/%m/%y %H:%M",timeStruct);
        dotCode += "<tr><td> s_mtime </td><td> ";
        dotCode += fecha;
        dotCode += "</td></tr>\n";
        
        timeStruct=localtime(&super.s_umtime);
        strftime(fecha,100,"%d/%m/%y %H:%M",timeStruct);
        dotCode += "<tr><td> s_umtime </td><td>";
        dotCode += fecha;
        dotCode += " </td></tr>\n";

        dotCode += "<tr><td> s_mnt_count </td><td> "+to_string(super.s_mnt_count)+" </td> </tr>\n";
        dotCode += "<tr><td> s_magic </td> <td> "+to_string(super.s_magic)+" </td> </tr>\n";
        dotCode += "<tr><td> s_inode_size </td> <td>"+to_string(super.s_inode_size)+"</td></tr>\n";
        dotCode += "<tr><td> s_block_size </td><td>"+to_string(super.s_block_size)+"</td></tr>\n";
        dotCode += "<tr><td> s_first_ino </td><td>"+to_string( super.s_first_ino)+" </td> </tr>\n";
        dotCode += "<tr><td> s_first_blo </td><td>"+to_string(super.s_first_blo)+"</td></tr>\n";
        dotCode += "<tr><td> s_bm_inode_start </td><td>"+to_string(super.s_bm_inode_start)+"</td></tr>\n";
        dotCode += "<tr><td> s_bm_block_start </td><td>"+to_string(super.s_bm_block_start)+"</td></tr>\n";
        dotCode += "<tr><td> s_inode_start </td><td>"+to_string(super.s_inode_start)+"</td></tr>\n";
        dotCode += "<tr><td> s_block_start </td><td>"+to_string(super.s_block_start)+"</td></tr>\n";
        dotCode += "</table>>]\n";
        dotCode +="\n}";

        fclose(file);

        //Obtener la ruta.
        string pathSinExt = this->path;
        string extension;
        const size_t lastPoint = pathSinExt.find_last_of(".");
        if (string::npos != lastPoint){
            extension = pathSinExt.substr(lastPoint, pathSinExt.length());
            pathSinExt = pathSinExt.substr(0, lastPoint);
        }
        string auxPath2 = pathSinExt;
        auxPath2 += ".txt";

        string pathSinName = this->path;
        const size_t lastSlash = pathSinName.find_last_of("/");
        if (string::npos != lastSlash){
            pathSinName = pathSinExt.substr(0, lastSlash);
        }

        string comando;

        //Se crea carpeta si se necesita
        comando= "sudo mkdir -p \'";
        comando+= pathSinName;
        comando+= '\'';
        system(comando.c_str());

        //Se conceden permisos
        comando = "sudo chmod -R 777 \'";
        comando+= pathSinName;
        comando += '\'';
        system(comando.c_str());

        // Se crea el .dot
        FILE *dotFile = fopen(auxPath2.c_str(),"w+");
        freopen(NULL,"w+",dotFile);
        char charArr[dotCode.length()];
        strcpy(charArr,dotCode.c_str());
        fprintf(dotFile,"%s\n",charArr);
        fclose(dotFile);
        comando = "dot \'";
        comando += auxPath2;
        comando +="\' -o \'" + pathSinExt + extension + " \' -T" + extension.substr(1,extension.length()-1);
        this_thread::sleep_for(chrono::milliseconds(1000));
        system(comando.c_str());
        this_thread::sleep_for(chrono::milliseconds(1000));
        //Se elimina archivo de residuo
        comando = "sudo rm '";
        comando += auxPath2 + "\'";
        system(comando.c_str());
        this_thread::sleep_for(chrono::milliseconds(1000));
        cout<< "\u001B[32m" << "[OK] Reporte de Super Bloque creado exitosamente"<< "\x1B[0m" << endl;

    }
    else{
        partIndex = disk->findLogicPartitionStart();
        if(partIndex != -1){
            EBR extendedBoot;
            SuperBloque super;
            FILE *file = fopen(disk->getPath().data(),"r+b");
            fseek(file,partIndex,SEEK_SET);
            fread(&extendedBoot,sizeof(EBR),1,file);

            fseek(file,extendedBoot.part_start,SEEK_SET);
            fread(&super,sizeof (super),1,file);

            string dotCode = "digraph G{\n";
            dotCode += "superBloqueTable[shape=none,label=<";
            dotCode += "<table border='0' cellborder='1' cellspacing='0'>\n";
            dotCode += "<tr><td colspan='2'><b> Super Bloque </b></td></tr>\n";
            dotCode += "<tr><td> s_inodes_count </td><td>"+to_string(super.s_inodes_count)+" </td> </tr>\n";
            dotCode += "<tr><td> s_blocks_count </td><td>"+to_string(super.s_blocks_count)+"</td></tr>\n";
            dotCode += "<tr><td> s_free_block_count </td><td>"+to_string(super.s_free_blocks_count)+"</td> </tr>\n";
            dotCode += "<tr><td> s_free_inodes_count </td><td>"+to_string(super.s_free_inodes_count)+"</td></tr>\n";

            struct tm *timeStruct;
            char fecha[50];

            timeStruct=localtime(&super.s_mtime);
            strftime(fecha,100,"%d/%m/%y %H:%M",timeStruct);
            dotCode += "<tr><td> s_mtime </td><td> ";
            dotCode += fecha;
            dotCode += "</td></tr>\n";
            
            timeStruct=localtime(&super.s_umtime);
            strftime(fecha,100,"%d/%m/%y %H:%M",timeStruct);
            dotCode += "<tr><td> s_umtime </td><td>";
            dotCode += fecha;
            dotCode += " </td></tr>\n";

            dotCode += "<tr><td> s_mnt_count </td><td> "+to_string(super.s_mnt_count)+" </td> </tr>\n";
            dotCode += "<tr><td> s_magic </td> <td> "+to_string(super.s_magic)+" </td> </tr>\n";
            dotCode += "<tr><td> s_inode_size </td> <td>"+to_string(super.s_inode_size)+"</td></tr>\n";
            dotCode += "<tr><td> s_block_size </td><td>"+to_string(super.s_block_size)+"</td></tr>\n";
            dotCode += "<tr><td> s_first_ino </td><td>"+to_string( super.s_first_ino)+" </td> </tr>\n";
            dotCode += "<tr><td> s_first_blo </td><td>"+to_string(super.s_first_blo)+"</td></tr>\n";
            dotCode += "<tr><td> s_bm_inode_start </td><td>"+to_string(super.s_bm_inode_start)+"</td></tr>\n";
            dotCode += "<tr><td> s_bm_block_start </td><td>"+to_string(super.s_bm_block_start)+"</td></tr>\n";
            dotCode += "<tr><td> s_inode_start </td><td>"+to_string(super.s_inode_start)+"</td></tr>\n";
            dotCode += "<tr><td> s_block_start </td><td>"+to_string(super.s_block_start)+"</td></tr>\n";
            dotCode += "</table>>]\n";
            dotCode +="\n}";

            fclose(file);

            //Obtener la ruta.
            string pathSinExt = this->path;
            string extension;
            const size_t lastPoint = pathSinExt.find_last_of(".");
            if (string::npos != lastPoint){
                extension = pathSinExt.substr(lastPoint, pathSinExt.length());
                pathSinExt = pathSinExt.substr(0, lastPoint);
            }
            string auxPath2 = pathSinExt;
            auxPath2 += ".txt";

            string pathSinName = this->path;
            const size_t lastSlash = pathSinName.find_last_of("/");
            if (string::npos != lastSlash){
                pathSinName = pathSinExt.substr(0, lastSlash);
            }

            string comando;

            //Se crea carpeta si se necesita
            comando= "sudo mkdir -p \'";
            comando+= pathSinName;
            comando+= '\'';
            system(comando.c_str());

            //Se conceden permisos
            comando = "sudo chmod -R 777 \'";
            comando+= pathSinName;
            comando += '\'';
            system(comando.c_str());

            // Se crea el .dot
            FILE *dotFile = fopen(auxPath2.c_str(),"w+");
            freopen(NULL,"w+",dotFile);
            char charArr[dotCode.length()];
            strcpy(charArr,dotCode.c_str());
            fprintf(dotFile,"%s\n",charArr);
            fclose(dotFile);
            comando = "dot \'";
            comando += auxPath2;
            comando +="\' -o \'" + pathSinExt + extension + " \' -T" + extension.substr(1,extension.length()-1);
            this_thread::sleep_for(chrono::milliseconds(1000));
            system(comando.c_str());
            this_thread::sleep_for(chrono::milliseconds(1000));
            //Se elimina archivo de residuo
            comando = "sudo rm '";
            comando += auxPath2 + "\'";
            system(comando.c_str());
            this_thread::sleep_for(chrono::milliseconds(1000));
            cout<< "\u001B[32m" << "[OK] Reporte de Super Bloque creado exitosamente"<< "\x1B[0m" << endl;
        }
    }
}

