#ifndef FDISK_H
#define FDISK_H

#endif // FDISK_H

#include <list>
#include<iterator>
#include<iostream>
#include<stdio.h>
#include<string.h>
#include<structs.h>
#include<MOUNT.h>

using namespace  std;

class FDISK_{
private:

    int size; //Tamaño de la particion
    string unit;// Tipo de Unidades
    char path[255];// Path del disco
    char fit[2];// Tipo de fit
    string deleteType; // Tipo de Eliminacion
    char *name; // Nombre de la particion
    int add; // Espacio a agregar o remover
    bool statusFlag;// Indica si ha errores con el comando.
    char partitionType; // Tipo de Operacion
    string operationType; // Tipo de particion


public:

FDISK_(){
    //Fit predeterminado Peor Ajuste
    this->fit[0]='W';
    this->fit[1]='F';
    //Unidades predeterminadas kilobytes
    this->unit='k';
    //Particion predeterminada
    this->partitionType='P';
    //Operacion predetermianda
    this->operationType = "create";
};

/**
 * Setter del size
 * @param size: indica el tamaño de la partición
 */
void setSize(char* size);

/**
 * Setter de unit
 * @param unit: indica la unidad de medida del parametro size.
*/
void setUnit(char *value);

/**
 * Setter del path
 * @param path: Path del disco
*/
void setPath(char *value);

/**
 * Setter del fit
 * @param fit: Ajuste para las particiones del disco
 *
*/
void setFit(char *value);

/**
 * Setter de delete
 * @param delete: Tipo de borrado
*/
void setDelete(char* value);

/**
 * Setter del name.
 * @param name: Nombre de la partición.
*/
void setName(char *c);

/**
 * Setter del add
 * @param add: Cantidad de espacio a agregará o remover de la partición.
*/
void setAdd(char* add);

/**
 * Setter del statusFlag.
 * @param correct: Booleano que representa errores en el comando
*/
void setCorrect(bool boolean);

/**
 * Setter del tipo de particion
 * @param type: Tipo de particion que se creara
*/
void setPartitionType(char *type);

/**
 * Verifica si los parametros son validos
*/
void setStatus();

/**
 * Getter del tipo de fit
*/
char getFit();

/**
 * Multiplicador de las unidades
*/
int getUnit();

/**
 * Getter del size en Bytes
*/
int getSize();

/**
 * Indica el tipo de partición
*/
char getType();

/**
 * Crea una particion segun el tipo
 */
void createPartition();

/**
 * Agrega o quita espacion a una particion
*/
void addToPartition();

/**
 *Elimina una particion
*/
void deletePartition();

/**
 * Crea una partición primaria.
*/
void createPrimaryPartition();

/**
 * Crea una particion extendida
*/
void createExtendedPartition();

/**
 * Crea una particion lógica.
*/
void createLogicPartition();

/**
 * Verifica que la partición no exista
*/
bool partitionExist(char* name);

/**
 * Inicia la ejecucion del comando
*/
void init();

};

extern list<MOUNT_> *mounted;

void FDISK_::setSize(char* size){
    this->size = stoi(size);
}

void FDISK_::setUnit(char *value){
    this->unit=value;
}

void FDISK_::setPath(char *value){
    if(value[0]=='\"'){
        string aux = value;
        aux = aux.substr(1,aux.length()-2);
        strcpy(this->path,aux.c_str());
    }
    else{
        strcpy(this->path, value);
    }
}

void FDISK_::setFit(char *value){
    strcpy(this->fit, value);
    this->fit[0] = toupper(fit[0]);
    this->fit[1] = toupper(fit[1]);
}

void FDISK_::setDelete(char* value){
    this->deleteType = value;
    this->operationType = "delete";
}

void FDISK_::setName(char *c){
    this->name = c;
}

void FDISK_::setAdd(char* add){
    this->add = atoi(add);
    this->operationType = "add";
}

void FDISK_::setCorrect(bool boolean){
    this->statusFlag = boolean;
}

void FDISK_::setPartitionType(char *type){
    this->partitionType = toupper(type[0]);
}

void FDISK_::setStatus(){

    this->statusFlag=false;

    FILE *file = fopen(path, "rb+");
    if(file == NULL){
        cout<< "\u001B[31m" << "[BAD PARAM] No existe el disco"<< "\x1B[0m" << endl;
        return;
    }
    fclose(file);

    //Parametros obligatorios
    if(this->path[0] != '/'){
        cout<< "\u001B[31m" << "[BAD PARAM] Path no valido"<< "\x1B[0m" << endl;
        return;

    }
    if(this->name == ""){
        cout<< "\u001B[31m" << "[BAD PARAM] Name no valido"<< "\x1B[0m" << endl;
        return;
    }

    //Dependiendo del tipo de operacion
    if(strcmp(this->operationType.c_str(),"create") ==0){
        if(size<=0){
            cout<< "\u001B[31m" << "[BAD PARAM] Size no puede ser menor o igual a 0"<< "\x1B[0m" << endl;
            return;
        }
    }
    else if(strcmp(this->operationType.c_str(),"delete") ==0){
        
    }
    else if(strcmp(this->operationType.c_str(),"add") ==0){
        if(size==0){
            cout<< "\u001B[31m" << "[BAD PARAM] Size no puede ser 0"<< "\x1B[0m" << endl;
            return;
        }
    }

    this->statusFlag=true;

}

char FDISK_::getFit(){
    return toupper(this->fit[0]);
}

int FDISK_::getUnit(){
    if(this->unit=="m"){
        return 1024*1024;
    }
    else if(this->unit=="k"){
        return 1024;
    }
    return 1;
}

int FDISK_::getSize(){
    return this->size*this->getUnit();
}

char FDISK_::getType(){
    return this->partitionType;
}

void FDISK_::createPartition(){
    switch (partitionType) {
    case 'P':
        createPrimaryPartition();
        break;
    case 'E':
        createExtendedPartition();
        break;
    case 'L':
        createLogicPartition();
        break;
    default:
        break;
    }
}

void FDISK_::createPrimaryPartition(){
    //Se lee MBR
    FILE *file;
    file=fopen(path,"rb+");
    if(file != NULL){
        MBR master;
        fseek(file,0,SEEK_SET);
        fread(&master,sizeof(MBR),1,file);

        Partition *mbr_partitions[4];
        mbr_partitions[0]=&master.mbr_partition_1;
        mbr_partitions[1]=&master.mbr_partition_2;
        mbr_partitions[2]=&master.mbr_partition_3;
        mbr_partitions[3]=&master.mbr_partition_4;

        int partitionIndex= -1;

        // Se calcula espacio disponible
        int usedBytes=0;
        for(int i= 0; i < 4; i++){
            if(mbr_partitions[i]->part_status!='1'){
                usedBytes += mbr_partitions[i]->part_size;
            }
        }
        // Se verifica si hay una particion disponible y con espacio disponible
        for(int i = 0; i < 4; i++){
            if(mbr_partitions[i]->part_start == -1 || (mbr_partitions[i]->part_status== '1' && mbr_partitions[i]->part_size >= getSize())){
                partitionIndex= i;
                break;
            }
        }
        if(partitionIndex!=-1){
            
            //Se verfica que las particion quepa
            if((master.mbr_tamano - usedBytes) >= getSize()){

                //Se verfica que no exista la particion
                if(!partitionExist(this->name)){

                    //Si es el mejor ajuste
                    if(master.disk_fit == 'B'){
                        //Se calcula la mejor posicion
                        int bestPartitionIndex = partitionIndex;
                        for(int i = 0; i < 4; i++){
                            if(mbr_partitions[i]->part_start == -1 || (mbr_partitions[i]->part_status == '1' && mbr_partitions[i]->part_size>=getSize())){
                                //Si se encontro una posicion candidata diferente a la que se tiene marcada
                                if(i != partitionIndex){
                                    if(mbr_partitions[i]->part_size < mbr_partitions[bestPartitionIndex]->part_size ){
                                        bestPartitionIndex = i;
                                    }
                                }
                            }
                        }

                        //Se configura el struct de la particion
                        mbr_partitions[partitionIndex]->part_status = '0';
                        mbr_partitions[partitionIndex]->part_type = 'P';
                        mbr_partitions[partitionIndex]->part_size = getSize();
                        mbr_partitions[partitionIndex]->part_fit = getFit();
                        strcpy(mbr_partitions[partitionIndex]->part_name, this->name);
                        if(partitionIndex ==0){
                            mbr_partitions[partitionIndex]->part_start = sizeof (MBR);
                        }
                        else{
                            mbr_partitions[partitionIndex]->part_start = mbr_partitions[partitionIndex-1]->part_start + mbr_partitions[partitionIndex-1]->part_size;
                        }

                        // Se reescribe el MBR
                        fseek(file,0, SEEK_SET);
                        fwrite(&master,sizeof (MBR),1,file);

                        //Se escribe la particion
                        fseek(file,mbr_partitions[partitionIndex]->part_start,SEEK_SET);
                        char myChar = '\0';
                        for(int i = 0; i < getSize(); i++){
                            fwrite(&myChar,1,1,file);
                        }
                        cout<< "\u001B[32m" << "[OK] La particion primaria " <<this->name<<" ha sido creada exitosamente"<< "\x1B[0m" << endl;
                    }
                    //Si el fit es el primer ajuste
                    else if(master.disk_fit=='F'){
                        //Se configura el struct de la particion
                        mbr_partitions[partitionIndex]->part_status = '0';
                        mbr_partitions[partitionIndex]->part_type = 'P';
                        mbr_partitions[partitionIndex]->part_size = getSize();
                        mbr_partitions[partitionIndex]->part_fit = getFit();
                        strcpy(mbr_partitions[partitionIndex]->part_name, this->name);
                        if(partitionIndex ==0){
                            mbr_partitions[partitionIndex]->part_start = sizeof (MBR);
                        }
                        else{
                            mbr_partitions[partitionIndex]->part_start = mbr_partitions[partitionIndex-1]->part_start + mbr_partitions[partitionIndex-1]->part_size;
                        }

                        // Se reescribe el MBR
                        fseek(file,0, SEEK_SET);
                        fwrite(&master,sizeof (MBR),1,file);

                        //Se escribe la particion
                        fseek(file,mbr_partitions[partitionIndex]->part_start,SEEK_SET);
                        char myChar = '\0';
                        for(int i = 0; i < getSize(); i++){
                            fwrite(&myChar,1,1,file);
                        }
                        cout<< "\u001B[32m" << "[OK] La particion primaria " <<this->name<<" ha sido creada exitosamente"<< "\x1B[0m" << endl;
                    }
                    //Si es el peor ajuste
                    else if(master.disk_fit == 'W'){
                        //Se calcula la peor posicion
                        int worstPartitionIndex = partitionIndex;
                        for(int i = 0; i < 4; i++){
                            if(mbr_partitions[i]->part_start == -1 || (mbr_partitions[i]->part_status == '1' && mbr_partitions[i]->part_size>=getSize())){
                                //Si se encontro una posicion candidata diferente a la que se tiene marcada
                                if(i != partitionIndex){
                                    if(mbr_partitions[i]->part_size > mbr_partitions[worstPartitionIndex]->part_size ){
                                        worstPartitionIndex = i;
                                    }
                                }
                            }
                        }

                        //Se configura el struct de la particion
                        mbr_partitions[partitionIndex]->part_status = '0';
                        mbr_partitions[partitionIndex]->part_type = 'P';
                        mbr_partitions[partitionIndex]->part_size = getSize();
                        mbr_partitions[partitionIndex]->part_fit = getFit();
                        strcpy(mbr_partitions[partitionIndex]->part_name, this->name);
                        if(partitionIndex ==0){
                            mbr_partitions[partitionIndex]->part_start = sizeof (MBR);
                        }
                        else{
                            mbr_partitions[partitionIndex]->part_start = mbr_partitions[partitionIndex-1]->part_start + mbr_partitions[partitionIndex-1]->part_size;
                        }

                        // Se reescribe el MBR
                        fseek(file,0, SEEK_SET);
                        fwrite(&master,sizeof (MBR),1,file);

                        //Se escribe la particion
                        fseek(file,mbr_partitions[partitionIndex]->part_start,SEEK_SET);
                        char myChar = '\0';
                        for(int i = 0; i < getSize(); i++){
                            fwrite(&myChar,1,1,file);
                        }
                        cout<< "\u001B[32m" << "[OK] La particion primaria " <<this->name<<" ha sido creada exitosamente"<< "\x1B[0m" << endl;
                    }
                }
                else{
                    cout<< "\u001B[31m" << "[BAD PARAM] La particion " <<this->name<<" no se puede volver a crear"<< "\x1B[0m" << endl;
                }
            }
            else{
                cout<< "\u001B[31m" << "[BAD PARAM] Espacio insuficiente en el disco "<< "\x1B[0m" << endl;
            }

        }
        else{
            cout<< "\u001B[31m" << "[BAD PARAM] Se llego al limite de particiones "<< "\x1B[0m" << endl;
        }
    }
    else{
        cout<< "\u001B[31m" << "[BAD PARAM] No existe disco en el path "<< "\x1B[0m" << endl;
    }
    fclose(file);

}

void FDISK_::createExtendedPartition(){
    //Se lee MBR
    FILE *file;
    file=fopen(path,"rb+");
    if(file != NULL){
        MBR master;
        fseek(file,0,SEEK_SET);
        fread(&master,sizeof(MBR),1,file);

        Partition *mbr_partitions[4];
        mbr_partitions[0]=&master.mbr_partition_1;
        mbr_partitions[1]=&master.mbr_partition_2;
        mbr_partitions[2]=&master.mbr_partition_3;
        mbr_partitions[3]=&master.mbr_partition_4;

        bool extendidaFlag = false;
        for(int i = 0;i < 4;i++){
            if(mbr_partitions[i]->part_type == 'E'){
                extendidaFlag=true;
                break;
            }
        }

        //Si no hay una extendida
        if(!extendidaFlag){

            int partitionIndex= -1;

            // Se calcula espacio disponible
            int usedBytes=0;
            for(int i= 0; i < 4; i++){
                if(mbr_partitions[i]->part_status!='1'){
                    usedBytes += mbr_partitions[i]->part_size;
                }
            }
            // Se verifica si hay una particion disponible y con espacio disponible
            for(int i = 0; i < 4; i++){
                if(mbr_partitions[i]->part_start == -1 || (mbr_partitions[i]->part_status== '1' && mbr_partitions[i]->part_size >= getSize())){
                    partitionIndex= i;
                    break;
                }
            }

            
            if(partitionIndex!=-1){
                
                //Se verfica que las particion quepa
                if((master.mbr_tamano - usedBytes) >= getSize()){
                    //Se verfica que no exista la particion
                    if(!partitionExist(this->name)){
                        //Si es el mejor ajuste
                        if(master.disk_fit == 'B'){

                            //Se calcula la mejor posicion
                            int bestPartitionIndex = partitionIndex;
                            for(int i = 0; i < 4; i++){
                                if(mbr_partitions[i]->part_start == -1 || (mbr_partitions[i]->part_status == '1' && mbr_partitions[i]->part_size>=getSize())){
                                    //Si se encontro una posicion candidata diferente a la que se tiene marcada
                                    if(i != partitionIndex){
                                        if(mbr_partitions[i]->part_size < mbr_partitions[bestPartitionIndex]->part_size ){
                                            bestPartitionIndex = i;
                                        }
                                    }
                                }
                            }

                            //Se configura el struct de la particion
                            mbr_partitions[partitionIndex]->part_status = '0';
                            mbr_partitions[partitionIndex]->part_type = 'E';
                            mbr_partitions[partitionIndex]->part_size = getSize();
                            mbr_partitions[partitionIndex]->part_fit = getFit();
                            strcpy(mbr_partitions[partitionIndex]->part_name, this->name);
                            if(partitionIndex ==0){
                                mbr_partitions[partitionIndex]->part_start = sizeof (MBR);
                            }
                            else{
                                mbr_partitions[partitionIndex]->part_start = mbr_partitions[partitionIndex-1]->part_start + mbr_partitions[partitionIndex-1]->part_size;
                            }

                            // Se reescribe el MBR
                            fseek(file,0, SEEK_SET);
                            fwrite(&master,sizeof (MBR),1,file);

                            //Se configura el EBR
                            EBR ebr;
                            fseek(file, mbr_partitions[partitionIndex]->part_start, SEEK_SET);
                            
                            ebr.part_status = '0';
                            ebr.part_fit = getFit();
                            ebr.part_start = mbr_partitions[partitionIndex]->part_start;
                            ebr.part_size = 0;
                            ebr.part_next = -1;
                            strcpy(ebr.part_name,this->name);
                            
                            //Se escribe el EBR
                            fwrite(&ebr, sizeof (EBR),1,file);
                            char myChar = '\0';
                            for(int i = 0 ; i < getSize() - (int)sizeof (EBR) ; i++){
                                fwrite(&myChar,1,1,file);
                            }
                            cout<< "\u001B[32m" << "[OK] La particion extendida " <<this->name<<" ha sido creada exitosamente"<< "\x1B[0m" << endl;
                        }
                        //Si el fit es el primer ajuste
                        else if(master.disk_fit == 'F'){
                            //Se configura el struct de la particion
                            mbr_partitions[partitionIndex]->part_status = '0';
                            mbr_partitions[partitionIndex]->part_type = 'E';
                            mbr_partitions[partitionIndex]->part_size = getSize();
                            mbr_partitions[partitionIndex]->part_fit = getFit();
                            strcpy(mbr_partitions[partitionIndex]->part_name, this->name);
                            if(partitionIndex ==0){
                                mbr_partitions[partitionIndex]->part_start = sizeof (MBR);
                            }
                            else{
                                mbr_partitions[partitionIndex]->part_start = mbr_partitions[partitionIndex-1]->part_start + mbr_partitions[partitionIndex-1]->part_size;
                            }

                            // Se reescribe el MBR
                            fseek(file,0, SEEK_SET);
                            fwrite(&master,sizeof (MBR),1,file);

                            //Se configura el EBR
                            EBR ebr;
                            fseek(file, mbr_partitions[partitionIndex]->part_start, SEEK_SET);
                            
                            ebr.part_status = '0';
                            ebr.part_fit = getFit();
                            ebr.part_start = mbr_partitions[partitionIndex]->part_start;
                            ebr.part_size = 0;
                            ebr.part_next = -1;
                            strcpy(ebr.part_name,this->name);
                            
                            //Se escribe el EBR
                            fwrite(&ebr, sizeof (EBR),1,file);
                            char myChar = '\0';
                            for(int i = 0 ; i < getSize() - (int)sizeof (EBR) ; i++){
                                fwrite(&myChar,1,1,file);
                            }
                            cout<< "\u001B[32m" << "[OK] La particion extendida " <<this->name<<" ha sido creada exitosamente"<< "\x1B[0m" << endl;
                        }
                        //Si es el peor ajuste
                        else if(master.disk_fit =='W'){
                            //Se calcula la peor posicion
                            int wrostPartitionIndex = partitionIndex;
                            for(int i = 0; i < 4; i++){
                                if(mbr_partitions[i]->part_start == -1 || (mbr_partitions[i]->part_status == '1' && mbr_partitions[i]->part_size>=getSize())){
                                    //Si se encontro una posicion candidata diferente a la que se tiene marcada
                                    if(i != partitionIndex){
                                        if(mbr_partitions[i]->part_size > mbr_partitions[wrostPartitionIndex]->part_size ){
                                            wrostPartitionIndex = i;
                                        }
                                    }
                                }
                            }

                            //Se configura el struct de la particion
                            mbr_partitions[partitionIndex]->part_status = '0';
                            mbr_partitions[partitionIndex]->part_type = 'E';
                            mbr_partitions[partitionIndex]->part_size = getSize();
                            mbr_partitions[partitionIndex]->part_fit = getFit();
                            strcpy(mbr_partitions[partitionIndex]->part_name, this->name);
                            if(partitionIndex ==0){
                                mbr_partitions[partitionIndex]->part_start = sizeof (MBR);
                            }
                            else{
                                mbr_partitions[partitionIndex]->part_start = mbr_partitions[partitionIndex-1]->part_start + mbr_partitions[partitionIndex-1]->part_size;
                            }

                            // Se reescribe el MBR
                            fseek(file,0, SEEK_SET);
                            fwrite(&master,sizeof (MBR),1,file);

                            //Se configura el EBR
                            EBR ebr;
                            fseek(file, mbr_partitions[partitionIndex]->part_start, SEEK_SET);
                            
                            ebr.part_status = '0';
                            ebr.part_fit = getFit();
                            ebr.part_start = mbr_partitions[partitionIndex]->part_start;
                            ebr.part_size = 0;
                            ebr.part_next = -1;
                            strcpy(ebr.part_name,this->name);
                            
                            //Se escribe el EBR
                            fwrite(&ebr, sizeof (EBR),1,file);
                            char myChar = '\0';
                            for(int i = 0 ; i < getSize() - (int)sizeof (EBR) ; i++){
                                fwrite(&myChar,1,1,file);
                            }
                            cout<< "\u001B[32m" << "[OK] La particion extendida " <<this->name<<" ha sido creada exitosamente"<< "\x1B[0m" << endl;
                        }
                    }
                    else{
                        cout<< "\u001B[31m" << "[BAD PARAM] La particion " <<this->name<<" no se puede volver a crear"<< "\x1B[0m" << endl;
                    }
                }
                else{
                    cout<< "\u001B[31m" << "[BAD PARAM] Espacio insuficiente en el disco "<< "\x1B[0m" << endl;
                }
            }
            else{
                cout<< "\u001B[31m" << "[BAD PARAM] Se llego al limite de particiones "<< "\x1B[0m" << endl;
            }

        }
        else{
            cout<< "\u001B[31m" << "[BAD PARAM] ya existe una particion extendida "<< "\x1B[0m" << endl;
        }
    }
    else{
        cout<< "\u001B[31m" << "[BAD PARAM] No existe disco en el path "<< "\x1B[0m" << endl;
    }
    fclose(file);
}

void FDISK_::createLogicPartition(){
    FILE *file;
    file=fopen(path,"rb+");
    if(file != NULL){
        MBR master;
        fseek(file,0,SEEK_SET);
        fread(&master,sizeof(MBR),1,file);

        Partition *mbr_partitions[4];
        mbr_partitions[0]=&master.mbr_partition_1;
        mbr_partitions[1]=&master.mbr_partition_2;
        mbr_partitions[2]=&master.mbr_partition_3;
        mbr_partitions[3]=&master.mbr_partition_4;

        int extendedIndex=-1;
        for(int i = 0;i < 4;i++){
            if(mbr_partitions[i]->part_type == 'E'){
                extendedIndex=i;
                break;
            }
        }

        if(extendedIndex!=-1){
            if(!partitionExist(this->name)){
                //Se lee el ebr
                EBR ebr;
                int extendedPartitionInit = mbr_partitions[extendedIndex]->part_start;
                fseek(file,extendedPartitionInit, SEEK_SET);
                fread(&ebr, sizeof (EBR),1,file);

                //Si es la primera particion logica
                if(ebr.part_size == 0) {
                    if(mbr_partitions[extendedIndex]->part_size > getSize()){

                        //Se escribe particion logica y EBR
                        ebr.part_status = '0';
                        ebr.part_fit = getFit();
                        ebr.part_start = ftell(file);
                        ebr.part_size = getSize();
                        ebr.part_next = -1;
                        strcpy(ebr.part_name, this->name);

                        //Se escribe particion
                        fseek(file, extendedPartitionInit,SEEK_SET);
                        fwrite(&ebr,sizeof (EBR),1,file);
                        cout<< "\u001B[32m" << "[OK] La particion logica " <<this->name<<" ha sido creada exitosamente"<< "\x1B[0m" << endl;
                    }
                    else{
                        cout<< "\u001B[31m" << "[BAD PARAM] Espacio insuficiente en la particion extendida "<< "\x1B[0m" << endl;
                    }
                }
                
                //Si la primera particion logica no esta vacia
                else{
                    //Se busca el ultimo EBR
                    while(ebr.part_next != -1){
                        fseek(file,ebr.part_next,SEEK_SET);
                        fread(&ebr,sizeof(EBR),1,file);
                    }
                    
                    //Se verifica si hay suficiente espacio
                    if(ebr.part_size + ebr.part_start + getSize() <= (mbr_partitions[extendedIndex]->part_size + mbr_partitions[extendedIndex]->part_start)){

                        //Se configura el EBR
                        ebr.part_next = ebr.part_start + ebr.part_size;
                        //Se retrocede el EBR que se leyo
                        fseek(file,ftell(file)-sizeof (EBR),SEEK_SET);
                        //Se reescribe el EBR
                        fwrite(&ebr, sizeof(EBR),1 ,file);

                        //Se escribe particion logica y EBR
                        fseek(file,ebr.part_start + ebr.part_size, SEEK_SET);
                        ebr.part_status = '0';
                        ebr.part_fit = getFit();
                        ebr.part_start = ftell(file);
                        ebr.part_size = getSize();
                        ebr.part_next = -1;
                        strcpy(ebr.part_name, this->name);
                        fwrite(&ebr,sizeof (EBR),1, file);
                        cout<< "\u001B[32m" << "[OK] La particion logica " <<this->name<<" ha sido creada exitosamente"<< "\x1B[0m" << endl;
                    }
                    else{
                        cout<< "\u001B[31m" << "[BAD PARAM] Espacio insuficiente en la particion extendida "<< "\x1B[0m" << endl;
                    }
                }
            }
            else{
                cout<< "\u001B[31m" << "[BAD PARAM] Ya existe una particion llamada "<<this->name<<" en el disco"<< "\x1B[0m" << endl;
                
            }
        }
        else{
            cout<< "\u001B[31m" << "[BAD PARAM] No existe particion extendida en el disco"<< "\x1B[0m" << endl;
        }
    }
    else{
        cout<< "\u001B[31m" << "[BAD PARAM] No existe disco en el path "<< "\x1B[0m" << endl;
    }
    fclose(file);
}
    
bool FDISK_::partitionExist(char* name){

    FILE *file;
    file =fopen(path,"rb+");
    //Se lee MBR
    MBR master;
    fseek(file,0,SEEK_SET);
    fread(&master, sizeof (MBR), 1, file);

    Partition *mbr_partitions[4];
    mbr_partitions[0]=&master.mbr_partition_1;
    mbr_partitions[1]=&master.mbr_partition_2;
    mbr_partitions[2]=&master.mbr_partition_3;
    mbr_partitions[3]=&master.mbr_partition_4;

    int extendedIndex = -1;
    for(int i = 0; i<4; i++){
        if(strcmp(name,mbr_partitions[i]->part_name) == 0){
            return true;
        }
        if(mbr_partitions[i]->part_type == 'E'){
            extendedIndex = i;
        }
    }

    if(extendedIndex != -1){
        //Se lee EBR
        EBR ebr;
        fseek(file, mbr_partitions[extendedIndex]->part_start, SEEK_SET);
        //Se lee toda la particion extendida y sus EBR
        while(fread(&ebr, sizeof (EBR),1,file) !=0){

            if(strcmp(this->name,ebr.part_name)==0){
                return  true;
            }
            else if(ebr.part_next == -1){
                return  false;
            } 
            else{
                fseek(file,ebr.part_next,SEEK_SET);
            }
        }
    }
    return  false;
}

void FDISK_::addToPartition(){

    add = add * getUnit();

    FILE *file = fopen(path, "rb+");
    if(file!=NULL){

        MBR master;
        fseek(file,0,SEEK_SET);
        fread(&master,sizeof(MBR),1,file);

        Partition *mbr_partitions[4];
        mbr_partitions[0]=&master.mbr_partition_1;
        mbr_partitions[1]=&master.mbr_partition_2;
        mbr_partitions[2]=&master.mbr_partition_3;
        mbr_partitions[3]=&master.mbr_partition_4;

        int partIndex = -1;
        int extendedIndex = -1;
        bool flagExtendida = false;
        //Se busca el indice de la particion
        for(int i = 0; i < 4; i++){

            if(mbr_partitions[i]->part_type == 'E'){
                flagExtendida = true;
                extendedIndex = i;
            }
            if((strcmp(mbr_partitions[i]->part_name, name)) == 0){
                partIndex = i;
                
                break;
            }
            

        }
        if(partIndex != -1){

            if(!flagExtendida){

                //SE AGREGA ESPACIO
               if(add>0){
                    
                    if(partIndex==3){

                        int aux = mbr_partitions[partIndex]->part_start + mbr_partitions[partIndex]->part_size;
                        int total = master.mbr_tamano + (int)sizeof(MBR);
                        int fragmentacion = total - aux;
                        if(fragmentacion != 0){
                            
                            if(fragmentacion >= add){
                                mbr_partitions[partIndex]->part_size = mbr_partitions[partIndex]->part_size + add;
                                fseek(file,0,SEEK_SET);
                                fwrite(&master,sizeof(MBR),1,file);
                                cout<< "\u001B[32m" << "[OK] Espacio agregado exitosamente a la particion "<< "\x1B[0m" << endl;
                            }
                            else{
                                cout<< "\u001B[31m" << "[BAD PARAM] Espacio insuficiente para agregar espacio a la particion "<< "\x1B[0m" << endl;
                            }
                        }
                        else{
                            cout<< "\u001B[31m" << "[BAD PARAM] Espacio insuficiente para agregar espacio a la particion "<< "\x1B[0m" << endl;
                        }
                        
                    }
                    else{

                        int aux1 = mbr_partitions[partIndex]->part_start + mbr_partitions[partIndex]->part_size;
                        int aux2 = mbr_partitions[partIndex+1]->part_start;
                        int fragmentacion = aux2-aux1;
                        if(fragmentacion == 0){

                            if(mbr_partitions[partIndex + 1]->part_status == '1'){

                                if(mbr_partitions[partIndex + 1]->part_size >= add){
                                    
                                    mbr_partitions[partIndex + 1]->part_size = mbr_partitions[partIndex + 1]->part_size - add;
                                    mbr_partitions[partIndex + 1]->part_start = mbr_partitions[partIndex + 1]->part_start + add;
                                    mbr_partitions[partIndex]->part_size = mbr_partitions[partIndex]->part_size + add;
                                    
                                    fseek(file,0,SEEK_SET);
                                    fwrite(&master,sizeof(MBR),1,file);
                                    cout<< "\u001B[32m" << "[OK] Espacio agregado exitosamente a la particion "<< "\x1B[0m" << endl;
                                }
                                else{
                                    cout<< "\u001B[31m" << "[BAD PARAM] Espacio insuficiente para agregar espacio a la particion "<< "\x1B[0m" << endl;
                                }
                            }
                        }
                        else{
                            if(fragmentacion >= add){
                                mbr_partitions[partIndex]->part_size = mbr_partitions[partIndex]->part_size + add;
                                fseek(file,0,SEEK_SET);
                                fwrite(&master,sizeof(MBR),1,file);
                                cout<< "\u001B[32m" << "[OK] Espacio agregado exitosamente a la particion "<< "\x1B[0m" << endl;
                            }
                            else{
                                cout<< "\u001B[31m" << "[BAD PARAM] Espacio insuficiente para agregar espacio a la particion "<< "\x1B[0m" << endl;
                            }   
                        }
                        
                    }
                }
                //SE QUITA ESPACIO
                else{
                    add=add*-1;
                    if(add < mbr_partitions[partIndex]->part_size){
                        mbr_partitions[partIndex]->part_size = mbr_partitions[partIndex]->part_size - add;
                        fseek(file,0,SEEK_SET);
                        fwrite(&master,sizeof(MBR),1,file);
                        cout<< "\u001B[32m" << "[OK] Espacio reducido exitosamente a la particion "<< "\x1B[0m" << endl;
                    }
                    else{
                        cout<< "\u001B[31m" << "[BAD PARAM] No se puede reducir la particion mas alla de su tamaño actual "<< "\x1B[0m" << endl;
                    }
                }
            }
            else{
                //SE AGREGA ESPACIO
                if(add>0){
                    
                    if(partIndex==3){

                        int aux = mbr_partitions[partIndex]->part_start + mbr_partitions[partIndex]->part_size;
                        int total = master.mbr_tamano + (int)sizeof(MBR);
                        int fragmentacion = total - aux;
                        if(fragmentacion != 0){
                            
                            if(fragmentacion >= add){
                                mbr_partitions[partIndex]->part_size = mbr_partitions[partIndex]->part_size + add;
                                fseek(file,0,SEEK_SET);
                                fwrite(&master,sizeof(MBR),1,file);
                                cout<< "\u001B[32m" << "[OK] Espacio agregado exitosamente a la particion "<< "\x1B[0m" << endl;
                            }
                            else{
                                cout<< "\u001B[31m" << "[BAD PARAM] Espacio insuficiente para agregar espacio a la particion "<< "\x1B[0m" << endl;
                            }
                        }
                        else{
                            cout<< "\u001B[31m" << "[BAD PARAM] Espacio insuficiente para agregar espacio a la particion "<< "\x1B[0m" << endl;
                        }
                        
                    }
                    else{

                        int aux1 = mbr_partitions[partIndex]->part_start + mbr_partitions[partIndex]->part_size;
                        int aux2 = mbr_partitions[partIndex+1]->part_start;
                        int fragmentacion = aux2-aux1;
                        if(fragmentacion == 0){

                            if(mbr_partitions[partIndex + 1]->part_status == '1'){

                                if(mbr_partitions[partIndex + 1]->part_size >= add){
                                    
                                    mbr_partitions[partIndex + 1]->part_size = mbr_partitions[partIndex + 1]->part_size - add;
                                    mbr_partitions[partIndex + 1]->part_start = mbr_partitions[partIndex + 1]->part_start + add;
                                    mbr_partitions[partIndex]->part_size = mbr_partitions[partIndex]->part_size + add;
                                    
                                    fseek(file,0,SEEK_SET);
                                    fwrite(&master,sizeof(MBR),1,file);
                                    cout<< "\u001B[32m" << "[OK] Espacio agregado exitosamente a la particion "<< "\x1B[0m" << endl;
                                }
                                else{
                                    cout<< "\u001B[31m" << "[BAD PARAM] Espacio insuficiente para agregar espacio a la particion "<< "\x1B[0m" << endl;
                                }
                            }
                        }
                        else{
                            if(fragmentacion >= add){
                                mbr_partitions[partIndex]->part_size = mbr_partitions[partIndex]->part_size + add;
                                fseek(file,0,SEEK_SET);
                                fwrite(&master,sizeof(MBR),1,file);
                                cout<< "\u001B[32m" << "[OK] Espacio agregado exitosamente a la particion "<< "\x1B[0m" << endl;
                            }
                            else{
                                cout<< "\u001B[31m" << "[BAD PARAM] Espacio insuficiente para agregar espacio a la particion "<< "\x1B[0m" << endl;
                            }   
                        }
                        
                    }
                }
                //SE QUITA ESPACIO
                else{
                    add = add * -1;
                    if(add < mbr_partitions[partIndex]->part_size){
                        EBR ebr;
                        fseek(file, mbr_partitions[partIndex]->part_start,SEEK_SET);
                        fread(&ebr,sizeof(EBR),1,file);
                        while(ebr.part_next != -1){
                            fseek(file,ebr.part_next,SEEK_SET);
                            fread(&ebr,sizeof(EBR),1,file);
                        }
                        int aux = mbr_partitions[partIndex]->part_start + mbr_partitions[partIndex]->part_size - add;
                        if(aux > ebr.part_start+ebr.part_size){
                            mbr_partitions[partIndex]->part_size = mbr_partitions[partIndex]->part_size - add;
                            fseek(file,0,SEEK_SET);
                            fwrite(&master,sizeof(MBR),1,file);
                            cout<< "\u001B[32m" << "[OK] Espacio reducido exitosamente a la particion "<< "\x1B[0m" << endl;
                        }
                        else{
                            cout<< "\u001B[31m" << "[BAD PARAM] No se puede reducir la particion ya que hay particiones logicas usando el espacio"<< "\x1B[0m" << endl;
                        }
                        
                    }
                    else{
                        cout<< "\u001B[31m" << "[BAD PARAM] No se puede reducir la particion mas alla de su tamaño actual "<< "\x1B[0m" << endl;
                    }
                }
            }
        }
        else{
            if(extendedIndex!=-1){      
                int logicIndex = -1;
                //Se lee Extended Boot Record
                EBR ebr;
                fseek(file, mbr_partitions[extendedIndex]->part_start,SEEK_SET);
                //se busca la particion logica
                while(logicIndex==-1){
                    fread(&ebr,sizeof(EBR),1,file);
                    if(strcmp(this->name,ebr.part_name) == 0){
                        logicIndex= ftell(file) - sizeof(EBR);
                        break;
                    }
                    else if(ebr.part_next == -1){
                        break;
                    }
                    else {
                        fseek(file,ebr.part_next,SEEK_SET);
                    }
                }
                
                if(logicIndex != -1){


                    if(add>0){
                        if(ebr.part_next != -1 ){

                            if(ebr.part_start+ebr.part_size+add < ebr.part_next){
                                ebr.part_size = ebr.part_size + add;
                                fseek(file,logicIndex,SEEK_SET);
                                fwrite(&ebr,sizeof(EBR),1,file);
                                cout<< "\u001B[32m" << "[OK] Espacio agregado exitosamente a la particion "<< "\x1B[0m" << endl;
                            }
                            else{
                                cout<< "\u001B[31m" << "[BAD PARAM] Espacio insuficiente para agregar espacio a la particion "<< "\x1B[0m" << endl;
                            }   

                        }
                        else{
                            if(ebr.part_start+ebr.part_size+add < mbr_partitions[extendedIndex]->part_start+mbr_partitions[extendedIndex]->part_size){
                                ebr.part_size = ebr.part_size + add;
                                fseek(file,logicIndex,SEEK_SET);
                                fwrite(&ebr,sizeof(EBR),1,file);
                                cout<< "\u001B[32m" << "[OK] Espacio agregado exitosamente a la particion "<< "\x1B[0m" << endl;
                            }
                            else{
                                cout<< "\u001B[31m" << "[BAD PARAM] Espacio insuficiente para agregar espacio a la particion "<< "\x1B[0m" << endl;
                            }
                        }
                    }
                    else{
                        add=add*-1;
                        if(add < ebr.part_size){
                            ebr.part_size = ebr.part_size - add;
                            fseek(file,logicIndex,SEEK_SET);
                            fwrite(&ebr,sizeof(EBR),1,file);
                            cout<< "\u001B[32m" << "[OK] Espacio reducido exitosamente a la particion "<< "\x1B[0m" << endl;
                        }
                        else{
                            cout<< "\u001B[31m" << "[BAD PARAM] No se puede reducir la particion mas alla de su tamaño actual "<< "\x1B[0m" << endl;
                        }
                    }
                }
                else{
                    cout<< "\u001B[31m" << "[BAD PARAM] No se encontro una particion con el nombre indicado"<< "\x1B[0m" << endl;
                }
            }
            else{
                cout<< "\u001B[31m" << "[BAD PARAM] No se encontro una particion con el nombre indicado"<< "\x1B[0m" << endl;   
            }
        }
        fclose(file);
    }
    else{
        cout<< "\u001B[31m" << "[BAD PARAM] No se encontro el disco con esa direccion"<< "\x1B[0m" << endl;
    }
}

void FDISK_::deletePartition(){

    FILE *file;
    file=fopen(path,"rb+");
    if(file != NULL){

        //Se verifica que no este montada
        bool mount = false;
        list<MOUNT_>::iterator i;
        for(i = mounted->begin();i != mounted->end();i++){           
            if(i->getName() == this->name && i->getPath() == this->path){
                mount  = true;
            }
        }
        if(!mount){

            //Se lee el MBR
            MBR master;
            fseek(file,0,SEEK_SET);
            fread(&master,sizeof (MBR),1,file);

            Partition *mbr_partitions[4];
            mbr_partitions[0]=&master.mbr_partition_1;
            mbr_partitions[1]=&master.mbr_partition_2;
            mbr_partitions[2]=&master.mbr_partition_3;
            mbr_partitions[3]=&master.mbr_partition_4;

            int index = -1;
            long logicIndex = -1; //Apunta al EBR de la particion que se va eliminar
            long auxEbrIndex=-1; //Apunta al anterior EBR del EBR de la particion que se va eliminar
            bool isExtended = false;
            char myChar='\0';

            //Se busca indice de la particion
            for(int i = 0; i < 4; i++){
                if(strcmp(this->name,mbr_partitions[i]->part_name) == 0){
                    index = i;
                    if(mbr_partitions[i]->part_type == 'E'){
                        isExtended = true;
                    } 
                    break;
                }
                else if(mbr_partitions[i]->part_type == 'E'){

                    //Se lee Extended Boot Record
                    EBR ebr;
                    fseek(file, mbr_partitions[i]->part_start,SEEK_SET);
                    
                    //Se busca la particion logica
                    while(fread(&ebr,sizeof(EBR),1,file)!=0){

                        if(strcmp(this->name,ebr.part_name) == 0){
                            logicIndex = ftell(file) - sizeof(EBR);
                            break;
                        }
                        //Si ya no hay una EBR que le siga
                        if(ebr.part_next == -1){
                            break;
                        }
                        //Se pasa al siguiente EBR
                        else {
                            auxEbrIndex = ftell(file) - sizeof(EBR);
                            fseek(file,ebr.part_next,SEEK_SET);
                        }
                    }
                } 
            }

            //Si se va eliminar una particion extendida o logica
            if(index != -1){

                if(!isExtended){
                    if(this->deleteType=="full"){
                        //Se sobre escribe el disco
                        fseek(file,mbr_partitions[index]->part_start,SEEK_SET);
                        fwrite(&myChar,1,mbr_partitions[index]->part_size,file);
                        //Se configura y se vuelve a escribir el MBR
                        Partition tempPartition;
                        mbr_partitions[index] = &tempPartition;
                        mbr_partitions[index]->part_status = '1';
                        fseek(file,0,SEEK_SET);
                        fwrite(&master,sizeof(MBR),1,file);
                        cout<< "\u001B[32m" << "[OK] Particion primaria eliminada exitosamente"<< "\x1B[0m" << endl;                    
                    }
                    else if(this->deleteType=="fast"){
                        //Se configura y se vuelve a escribir el MBR
                        Partition tempPartition;
                        mbr_partitions[index] = &tempPartition;
                        mbr_partitions[index]->part_status = '1';
                        fseek(file,0,SEEK_SET);
                        fwrite(&master,sizeof(MBR),1,file);
                        cout<< "\u001B[32m" << "[OK] Particion primaria eliminada exitosamente"<< "\x1B[0m" << endl;   
                    }     
                    else{
                        cout<< "\u001B[32m" << "[OK] No se indico el tipo de eliminacion"<< "\x1B[0m" << endl; 
                    }       
                }
                else{
                    if(this->deleteType=="full"){
                        //Se sobre escribe el disco
                        fseek(file,mbr_partitions[index]->part_start,SEEK_SET);
                        fwrite(&myChar,1,mbr_partitions[index]->part_size,file);
                        //Se configura y se vuelve a escribir el MBR
                        Partition tempPartition;
                        mbr_partitions[index] = &tempPartition;
                        mbr_partitions[index]->part_status = '1';
                        fseek(file,0,SEEK_SET);
                        fwrite(&master,sizeof(MBR),1,file);
                        cout<< "\u001B[32m" << "[OK] Particion extendida eliminada exitosamente"<< "\x1B[0m" << endl;                    
                    }
                    else if(this->deleteType=="fast"){
                        //Se configura y se vuelve a escribir el MBR
                        Partition tempPartition;
                        mbr_partitions[index] = &tempPartition;
                        mbr_partitions[index]->part_status = '1';
                        fseek(file,0,SEEK_SET);
                        fwrite(&master,sizeof(MBR),1,file);
                        cout<< "\u001B[32m" << "[OK] Particion extendida eliminada exitosamente"<< "\x1B[0m" << endl;   
                    }
                    else{
                        cout<< "\u001B[32m" << "[OK] No se indico el tipo de eliminacion"<< "\x1B[0m" << endl; 
                    }
                }
            }
            else if(logicIndex != -1){
                
                EBR ebr;

                //Se lee EBR
                fseek(file,logicIndex,SEEK_SET);
                fread(&ebr,sizeof(EBR),1,file);
                int nextPart = ebr.part_next; //Guarda el indice de la proxima particion logica

                //Se lee EBR anterior y se actualiza
                fseek(file,auxEbrIndex,SEEK_SET);
                fread(&ebr,sizeof(EBR),1,file);
                ebr.part_next = nextPart;
                fseek(file,auxEbrIndex,SEEK_SET);
                fwrite(&ebr,sizeof(EBR),1,file);

                //Se vuelve a leer EBR
                fseek(file,logicIndex,SEEK_SET);
                fread(&ebr,sizeof(EBR),1,file);


                if(this->deleteType=="full"){
                    //Se sobre escribe el disco
                    fseek(file,logicIndex,SEEK_SET);
                    fwrite(&myChar,1,sizeof(EBR)+ebr.part_size,file);
                    cout<< "\u001B[32m" << "[OK] Particion logica eliminada exitosamente"<< "\x1B[0m" << endl;                    
                }
                else if(this->deleteType=="fast"){
                    cout<< "\u001B[32m" << "[OK] Particion logica eliminada exitosamente"<< "\x1B[0m" << endl;   
                }     
                else{
                    cout<< "\u001B[32m" << "[OK] No se indico el tipo de eliminacion"<< "\x1B[0m" << endl; 
                }  
                

            }
            else{
                cout<< "\u001B[31m" << "[BAD PARAM] La particion no existe "<< "\x1B[0m" << endl;
            }

        }
        else{
            cout<< "\u001B[31m" << "[BAD PARAM] Se tiene que desmontar la particion antes de eliminarla "<< "\x1B[0m" << endl;
        }
        fclose(file);
    }
    else{
        cout<< "\u001B[31m" << "[BAD PARAM] No existe disco en el path "<< "\x1B[0m" << endl;
    }

}

void FDISK_::init(){

    setStatus();

    if(statusFlag){

        if(strcmp(operationType.c_str(),"delete")==0){
            deletePartition();
        }
        else if(strcmp(operationType.c_str(),"add")==0){
            addToPartition();
        }
        else if(strcmp(operationType.c_str(),"create")==0){
            createPartition();
        }

    }
    

}