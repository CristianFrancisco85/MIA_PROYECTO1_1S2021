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
    FILE *f;
    if((f = fopen(diskPath.c_str(),"rb+"))){

        string dotCode = "digraph{\n";
        dotCode+= "MBR_TABLE[\n";
        dotCode+="shape=none;label=<\n";
        dotCode+="<TABLE width='600' height= '1000' cellspacing='-1' cellborder='1'>\n";
        dotCode+= "<tr><td width='300'>Nombre</td><td width='300'>Valor</td></tr>";
        
        //Se lee el MBR
        MBR master;
        fseek(f,0,SEEK_SET);
        fread(&master,sizeof(MBR),1,f);

        //Se escribe Tamaño
        dotCode += "<tr>";
        dotCode += "<td>mbr_tamaño</td>";
        dotCode += "<td>"+to_string(master.mbr_tamano)+" bytes</td>";
        dotCode += "</tr>\n";

        //Se escribe Fecha de Creacion
        struct tm *tm;
        char fecha[100];
        tm = localtime(&master.mbr_fecha_creacion);
        strftime(fecha,100,"%d/%m/%y %H:%S", tm);
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
            if(master.mbr_partitions[i].part_status!='1' && master.mbr_partitions[i].part_start!=-1){
                if(master.mbr_partitions[i].part_type == 'E'){
                    extIndex = i;
                }

                //Se escribe Status
                dotCode += "<tr>";
                dotCode += "<td>part_status_"+to_string(i+1)+"</td><td>";
                dotCode += master.mbr_partitions[i].part_status;
                dotCode += "</td></tr>\n";

                //Se escribe Type
                dotCode += "<tr>";
                dotCode += "<td>part_type_"+to_string(i+1)+"</td><td>";
                dotCode += master.mbr_partitions[i].part_type;
                dotCode += "</td></tr>\n";

                //Se escribe Fit
                dotCode += "<tr>";
                dotCode += "<td>part_fit_"+to_string(i+1)+"</td><td>";
                dotCode += master.mbr_partitions[i].part_fit;
                dotCode += "</td></tr>\n";

                //Se escribe Start
                dotCode += "<tr>";
                dotCode += "<td>part_start_"+to_string(i+1)+"</td>";
                dotCode += "<td>"+to_string(master.mbr_partitions[i].part_start)+"</td>";
                dotCode += "</tr>\n";

                //Se escribe Size
                dotCode += "<tr>";
                dotCode += "<td>part_size_"+to_string(i+1)+"</td>";
                dotCode += "<td>"+to_string(master.mbr_partitions[i].part_size)+" bytes</td>";
                dotCode += "</tr>\n";

                //Se escribe Name
                dotCode += "<tr>";
                dotCode += "<td>part_name_"+to_string(i+1)+"</td>";
                dotCode += "<td>";
                dotCode += master.mbr_partitions[i].part_name;
                dotCode += "</td> </tr>\n";

            }
        }
        
        dotCode += "</TABLE>\n>];\n";

        //Si hay EBR para graficar
        if(extIndex != -1){
            int ebrIndex = 1;
            EBR ebr;
            fseek(f,master.mbr_partitions[extIndex].part_start,SEEK_SET);
            //Se leen todos los EBR y se grafica
            while(fread(&ebr,sizeof(EBR),1,f)!=0 && (ftell(f) < master.mbr_partitions[extIndex].part_start + master.mbr_partitions[extIndex].part_size)) {

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
                    fseek(f,ebr.part_next,SEEK_SET);
                }
            }
        }
        dotCode+="}\n";
        fclose(f);

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
            pathSinName = pathSinExt.substr(0, lastPoint);
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

        //Se elimina archivo si existiera
        comando = "sudo rm '";
        comando += auxPath2 + "\'";
        system(comando.c_str());
        this_thread::sleep_for(chrono::milliseconds(1000));

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
        this_thread::sleep_for(chrono::milliseconds(1500));
        system(comando.c_str());
        this_thread::sleep_for(chrono::milliseconds(1500));
        cout<< "\u001B[32m" << "[OK] Reporte MBR creado exitosamente"<< "\x1B[0m" << endl;
    }
    else{
        cout<< "\u001B[31m" << "[BAD PARAM] El disco no existe"<< "\x1B[0m" << endl;
    }

}









