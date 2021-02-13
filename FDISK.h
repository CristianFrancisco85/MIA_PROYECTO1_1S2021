#ifndef FDISK_H
#define FDISK_H

#endif // FDISK_H

#include <list>
#include<iterator>
#include<iostream>
#include<stdio.h>
#include<string.h>
#include<MOUNT.h>
#include<structs.h>

using namespace  std;
extern list<MOUNT_> *mounted;

class FDISK_{
private:

    enum partitionTypes{P, E, L};
    enum operationTypes{create_, delete_, modify_};

    int size; //Tamaño de la particion
    string unit;// Tipo de Unidades
    char path[255];// Path del disco
    char fit[2];// Tipo de fit
    string deleteType; // Tipo de Eliminacion
    char *name; // Nombre de la particion
    int add; // Espacio a agregar o remover
    bool statusFlag;// Indica si ha errores con el comando.
    partitionTypes partitionType; // Tipo de Operacion
    operationTypes operationType; // Tipo de particion


public:
/**
 * Constructor
 */
FDISK_(){
    //Fit predeterminado Mejor Ajuste
    this->fit[0]='W';
    this->fit[1]='F';
    //Unidades predeterminadas MegaBytes
    this->unit='k';
    //Particion predeterminada
    this->partitionType=P;
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
void setType(char *type);

/**
 * Getter del tipo de fit
*/
char getFit();

/**
 * Multiplicador de las unidades
*/
char getUnit();

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

};

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
        strcpy(this->path, aux.c_str());
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
    this->operationType = delete_;
}

void FDISK_::setName(char *c){
    this->name = c;
}

void FDISK_::setAdd(char* add){
    this->add = atoi(add);
    this->operationType = modify_;
}

void FDISK_::setCorrect(bool boolean){
    this->statusFlag = boolean;
}

void FDISK_::setType(char *type){
    switch (tolower(type[0])) {
    case 'p':
        this->partitionType = P;
        break;
    case 'e':
        this->partitionType = E;
        break;
    case 'l':
        this->partitionType = L;
        break;
    }
}

char FDISK_::getFit(){
    return toupper(this->fit[0]);
}

char FDISK_::getUnit(){
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
    switch (partitionType) {
    case P:
        return('P');
        break;
    case E:
        return('E');
        break;
    case L:
        return('L');
        break;
    default:
        return('P');
    }
}

void FDISK_::createPartition(){
    switch (partitionType) {
    case P:
        createPrimaryPartition();
        break;
    case E:
        createExtendedPartition();
        break;
    case L:
        createLogicPartition();
        break;
    default:
        break;
    }
}

void FDISK_::createPrimaryPartition(){
    //Se lee MBR
    FILE *file;
    if(file = fopen(path,"rb+")){
        MBR master;
        fseek(file,0,SEEK_SET);
        fread(&master,sizeof(MBR),1,file);

        // Se verifica si hay una particion disponible y con espacio disponible
        int partitionIndex= -1;
        bool partition=false;;
        for(int i = 0; i < 4; i++){
            if(master.mbr_partitions[i].part_start == -1 || (master.mbr_partitions[i].part_status== '1' && master.mbr_partitions[i].part_size >= getSize())){
                partitionIndex= i;
                partition=true;
                break;
            }
        }
        if(partition){

            // Se calcula espacio disponible
            int bytes=0;
            for(int i= 0; i < 4; i++){
                if(master.mbr_partitions[i].part_status!='1'){
                    bytes += master.mbr_partitions[i].part_size;
                }
            }
            
            //Se verfica que las particion quepa
            bool sizeBool = (master.mbr_tamano - bytes) >= getSize();
            if(sizeBool){

                //Se verfica que no exista la particion
                if(!partitionExist(this->name)){

                    //Si el fit es el primer ajuste
                    if(master.disk_fit=='F'){
                        //Se configura el struct de la particion
                        master.mbr_partitions[partitionIndex].part_status = '0';
                        master.mbr_partitions[partitionIndex].part_type = 'P';
                        master.mbr_partitions[partitionIndex].part_size = getSize();
                        master.mbr_partitions[partitionIndex].part_fit = getFit();
                        strcpy(master.mbr_partitions[partitionIndex].part_name, this->name);
                        if(partitionIndex ==0){
                            master.mbr_partitions[partitionIndex].part_start = sizeof (MBR);
                        }
                        else{
                            master.mbr_partitions[partitionIndex].part_start = master.mbr_partitions[partitionIndex-1].part_start + master.mbr_partitions[partitionIndex-1].part_size;
                        }

                        // Se reescribe el MBR
                        fseek(file,0, SEEK_SET);
                        fwrite(&master,sizeof (MBR),1,file);

                        //Se escribe la particion
                        char myByte = '1';
                        fseek(file,master.mbr_partitions[partitionIndex].part_start,SEEK_SET);
                        for(int i = 0; i < getSize(); i++){
                            fwrite(&myByte,1,1,file);
                        }
                        cout<< "\u001B[32m" << "[OK] La particion " <<this->name<<" ha sido creada exitosamente"<< "\x1B[0m" << endl;
                    }
                    //Si es el mejor ajuste
                    else if(master.disk_fit == 'B'){
                        //Se calcula la mejor posicion
                        int bestPartitionIndex = partitionIndex;
                        for(int i = 0; i < 4; i++){
                            if(master.mbr_partitions[i].part_start == -1 || (master.mbr_partitions[i].part_status == '1' && master.mbr_partitions[i].part_size>=getSize())){
                                //Si se encontro una posicion candidata
                                if(i != partitionIndex){
                                    if(master.mbr_partitions[i].part_size < master.mbr_partitions[bestPartitionIndex].part_size ){
                                        bestPartitionIndex = i;
                                    }
                                }
                            }
                        }

                        //Se configura el struct de la particion
                        master.mbr_partitions[partitionIndex].part_status = '0';
                        master.mbr_partitions[partitionIndex].part_type = 'P';
                        master.mbr_partitions[partitionIndex].part_size = getSize();
                        master.mbr_partitions[partitionIndex].part_fit = getFit();
                        strcpy(master.mbr_partitions[partitionIndex].part_name, this->name);
                        if(partitionIndex ==0){
                            master.mbr_partitions[partitionIndex].part_start = sizeof (MBR);
                        }
                        else{
                            master.mbr_partitions[partitionIndex].part_start = master.mbr_partitions[partitionIndex-1].part_start + master.mbr_partitions[partitionIndex-1].part_size;
                        }

                        // Se reescribe el MBR
                        fseek(file,0, SEEK_SET);
                        fwrite(&master,sizeof (MBR),1,file);

                        //Se escribe la particion
                        char myByte = '1';
                        fseek(file,master.mbr_partitions[partitionIndex].part_start,SEEK_SET);
                        for(int i = 0; i < getSize(); i++){
                            fwrite(&myByte,1,1,file);
                        }
                        cout<< "\u001B[32m" << "[OK] La particion " <<this->name<<" ha sido creada exitosamente"<< "\x1B[0m" << endl;
                    }
                    //Si es el peor ajuste
                    else if(master.disk_fit == 'W')
                    {
                        int  worstPartitionIndex= partitionIndex;
                        for(int i = 0; i < 4; i++){
                            if(master.mbr_partitions[i].part_start == -1 || (master.mbr_partitions[i].part_status == '1' && master.mbr_partitions[i].part_size>=getSize())){
                                if(i != partitionIndex){
                                    if(master.mbr_partitions[worstPartitionIndex].part_size > master.mbr_partitions[i].part_size){
                                        worstPartitionIndex= i;
                                        break;
                                    }
                                }
                            }
                        }

                        //Se configura el struct de la particion
                        master.mbr_partitions[partitionIndex].part_status = '0';
                        master.mbr_partitions[partitionIndex].part_type = 'P';
                        master.mbr_partitions[partitionIndex].part_size = getSize();
                        master.mbr_partitions[partitionIndex].part_fit = getFit();
                        strcpy(master.mbr_partitions[partitionIndex].part_name, this->name);
                        if(partitionIndex ==0){
                            master.mbr_partitions[partitionIndex].part_start = sizeof (MBR);
                        }
                        else{
                            master.mbr_partitions[partitionIndex].part_start = master.mbr_partitions[partitionIndex-1].part_start + master.mbr_partitions[partitionIndex-1].part_size;
                        }

                        // Se reescribe el MBR
                        fseek(file,0, SEEK_SET);
                        fwrite(&master,sizeof (MBR),1,file);

                        //Se escribe la particion
                        char myByte = '1';
                        fseek(file,master.mbr_partitions[partitionIndex].part_start,SEEK_SET);
                        for(int i = 0; i < getSize(); i++){
                            fwrite(&myByte,1,1,file);
                        }
                        cout<< "\u001B[32m" << "[OK] La particion " <<this->name<<" ha sido creada exitosamente"<< "\x1B[0m" << endl;
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

    if(file = fopen(path,"rb+")){
        MBR master;
        fseek(file,0,SEEK_SET);
        fread(&master,sizeof(MBR),1,file);

        bool extendidaFlag = false;
        for(int i = 0;i < 4;i++){
            if(master.mbr_partitions[i].part_type == 'E'){
                extendidaFlag=true;
                break;
            }
        }
        //Si no hay una extendida
        if(!extendidaFlag){

            // Se verifica si hay una particion disponible y con espacio disponible
            int partitionIndex= -1;
            bool partition=false;;
            for(int i = 0; i < 4; i++){
                if(master.mbr_partitions[i].part_start == -1 || (master.mbr_partitions[i].part_status== '1' && master.mbr_partitions[i].part_size >= getSize())){
                    partitionIndex= i;
                    partition=true;
                    break;
                }
            }

            
            if(partition){
                // Se calcula espacio disponible
                int bytes=0;
                for(int i= 0; i < 4; i++){
                    if(master.mbr_partitions[i].part_status!='1'){
                        bytes += master.mbr_partitions[i].part_size;
                    }
                }

                bool sizeBool = (master.mbr_tamano - bytes) >= getSize();
                if(sizeBool){
                    //Se verfica que no exista la particion
                    if(!partitionExist(this->name)){
                        //Si el fit es el primer ajuste
                        if(master.disk_fit == 'F'){
                            //Se configura el struct de la particion
                            master.mbr_partitions[partitionIndex].part_status = '0';
                            master.mbr_partitions[partitionIndex].part_type = 'E';
                            master.mbr_partitions[partitionIndex].part_size = getSize();
                            master.mbr_partitions[partitionIndex].part_fit = getFit();
                            strcpy(master.mbr_partitions[partitionIndex].part_name, this->name);
                            if(partitionIndex ==0){
                                master.mbr_partitions[partitionIndex].part_start = sizeof (MBR);
                            }
                            else{
                                master.mbr_partitions[partitionIndex].part_start = master.mbr_partitions[partitionIndex-1].part_start + master.mbr_partitions[partitionIndex-1].part_size;
                            }

                            // Se reescribe el MBR
                            fseek(file,0, SEEK_SET);
                            fwrite(&master,sizeof (MBR),1,file);

                            //Se configura el EBR
                            fseek(file, master.mbr_partitions[partitionIndex].part_start, SEEK_SET);
                            EBR eBootRecord;

                            eBootRecord.part_status = '0';
                            eBootRecord.part_fit = getFit();
                            eBootRecord.part_start = master.mbr_partitions[partitionIndex].part_start;
                            eBootRecord.part_size = 0;
                            eBootRecord.part_next = -1;
                            strcpy(eBootRecord.part_name,this->name);
                            
                            //Se escribe el EBR
                            fwrite(&eBootRecord, sizeof (EBR),1,file);
                            char myByte = '1';//Para llenar byte a byte
                            for(int i = getSize() - (int)sizeof (EBR); i > 0 ; i--){
                                fwrite(&myByte,1,1,file);
                            }
                            cout<< "\u001B[32m" << "[OK] La particion " <<this->name<<" ha sido creada exitosamente"<< "\x1B[0m" << endl;
                        }
                        //Si es el mejor ajuste
                        else if(master.disk_fit == 'B'){

                            int bestPartitionIndex = partitionIndex;
                            for(int i = 0; i < 4; i++){
                                if(master.mbr_partitions[i].part_start == -1 || (master.mbr_partitions[i].part_status == '1' && master.mbr_partitions[i].part_size>=getSize())){
                                    if(i != partitionIndex){
                                        if(master.mbr_partitions[i].part_size < master.mbr_partitions[bestPartitionIndex].part_size ){
                                            bestPartitionIndex = i;
                                        }
                                    }
                                }
                            }

                            //Se configura el struct de la particion
                            master.mbr_partitions[partitionIndex].part_status = '0';
                            master.mbr_partitions[partitionIndex].part_type = 'E';
                            master.mbr_partitions[partitionIndex].part_size = getSize();
                            master.mbr_partitions[partitionIndex].part_fit = getFit();
                            strcpy(master.mbr_partitions[partitionIndex].part_name, this->name);
                            if(partitionIndex ==0){
                                master.mbr_partitions[partitionIndex].part_start = sizeof (MBR);
                            }
                            else{
                                master.mbr_partitions[partitionIndex].part_start = master.mbr_partitions[partitionIndex-1].part_start + master.mbr_partitions[partitionIndex-1].part_size;
                            }

                            // Se reescribe el MBR
                            fseek(file,0, SEEK_SET);
                            fwrite(&master,sizeof (MBR),1,file);

                            //Se configura el EBR
                            fseek(file, master.mbr_partitions[partitionIndex].part_start, SEEK_SET);
                            EBR eBootRecord;

                            eBootRecord.part_status = '0';
                            eBootRecord.part_fit = getFit();
                            eBootRecord.part_start = master.mbr_partitions[partitionIndex].part_start;
                            eBootRecord.part_size = 0;
                            eBootRecord.part_next = -1;
                            strcpy(eBootRecord.part_name,this->name);
                            
                            //Se escribe el EBR
                            fwrite(&eBootRecord, sizeof (EBR),1,file);
                            char myByte = '1';//Para llenar byte a byte
                            for(int i = getSize() - (int)sizeof (EBR); i > 0 ; i--){
                                fwrite(&myByte,1,1,file);
                            }
                            cout<< "\u001B[32m" << "[OK] La particion " <<this->name<<" ha sido creada exitosamente"<< "\x1B[0m" << endl;
                        }
                        else if(master.disk_fit =='W'){
                            int  worstPartitionIndex= partitionIndex;
                            for(int i = 0; i < 4; i++){
                                if(master.mbr_partitions[i].part_start == -1 || (master.mbr_partitions[i].part_status == '1' && master.mbr_partitions[i].part_size>=getSize())){
                                    if(i != partitionIndex){
                                        if(master.mbr_partitions[worstPartitionIndex].part_size < master.mbr_partitions[i].part_size){
                                            worstPartitionIndex= i;
                                            break;
                                        }
                                    }
                                }
                            }

                        //Se configura el struct de la particion
                            master.mbr_partitions[partitionIndex].part_status = '0';
                            master.mbr_partitions[partitionIndex].part_type = 'E';
                            master.mbr_partitions[partitionIndex].part_size = getSize();
                            master.mbr_partitions[partitionIndex].part_fit = getFit();
                            strcpy(master.mbr_partitions[partitionIndex].part_name, this->name);
                            if(partitionIndex ==0){
                                master.mbr_partitions[partitionIndex].part_start = sizeof (MBR);
                            }
                            else{
                                master.mbr_partitions[partitionIndex].part_start = master.mbr_partitions[partitionIndex-1].part_start + master.mbr_partitions[partitionIndex-1].part_size;
                            }

                            // Se reescribe el MBR
                            fseek(file,0, SEEK_SET);
                            fwrite(&master,sizeof (MBR),1,file);

                            //Se configura el EBR
                            fseek(file, master.mbr_partitions[partitionIndex].part_start, SEEK_SET);
                            EBR eBootRecord;

                            eBootRecord.part_status = '0';
                            eBootRecord.part_fit = getFit();
                            eBootRecord.part_start = master.mbr_partitions[partitionIndex].part_start;
                            eBootRecord.part_size = 0;
                            eBootRecord.part_next = -1;
                            strcpy(eBootRecord.part_name,this->name);
                            
                            //Se escribe el EBR
                            fwrite(&eBootRecord, sizeof (EBR),1,file);
                            char myByte = '1';//Para llenar byte a byte
                            for(int i = getSize() - (int)sizeof (EBR); i > 0 ; i--){
                                fwrite(&myByte,1,1,file);
                            }
                            cout<< "\u001B[32m" << "[OK] La particion " <<this->name<<" ha sido creada exitosamente"<< "\x1B[0m" << endl;
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
    
    if(file = fopen(path,"rb+")){
        MBR master;
        fseek(file,0,SEEK_SET);
        fread(&master,sizeof(MBR),1,file);
        int extendeIndex;
        bool extendidaFlag = false;
        for(int i = 0;i < 4;i++){
            if(master.mbr_partitions[i].part_type == 'E'){
                extendidaFlag=true;
                extendeIndex=i;
                break;
            }
        }
        if(!partitionExist(this->name)){
            if(extendidaFlag){

                //Se lee el ebr
                EBR ebr;
                int extendedPartitionInit = master.mbr_partitions[extendeIndex].part_start;
                fseek(file,extendedPartitionInit, SEEK_SET);
                fread(&ebr, sizeof (EBR),1,file);
                //Si la primera particion logica no esta vacia
                if(ebr.part_size != 0) {
                    //Se busca el ultimo EBR
                    while((ebr.part_next != -1) && (ftell(file) < (master.mbr_partitions[extendeIndex].part_size + master.mbr_partitions[extendeIndex].part_start))){
                        fseek(file,ebr.part_next,SEEK_SET);
                        fread(&ebr,sizeof(EBR),1,file);
                    }
                    
                    //Se calcula si hay suficiente espacio
                    bool sizeBool = ebr.part_size + ebr.part_start + getSize() <= (master.mbr_partitions[extendeIndex].part_size + master.mbr_partitions[extendeIndex].part_start);
                    if(sizeBool){
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
                        fseek(file,extendedPartitionInit,SEEK_SET);
                        fread(&ebr, sizeof (EBR),1, file);
                        cout<< "\u001B[32m" << "[OK] La particion " <<this->name<<" ha sido creada exitosamente"<< "\x1B[0m" << endl;
                    }
                    else{
                        cout<< "\u001B[31m" << "[BAD PARAM] Espacio insuficiente en la particion extendida "<< "\x1B[0m" << endl;
                    }
                }
                //Si es la primera particion logica
                else{
                    if(master.mbr_partitions[extendeIndex].part_size > getSize()){
                        //Se escribe particion logica y EBR
                        ebr.part_status = '0';
                        ebr.part_fit = getFit();
                        ebr.part_start = ftell(file);
                        ebr.part_size = getSize();
                        ebr.part_next = -1;

                        //Escribirla en memoria
                        fseek(file, extendedPartitionInit,SEEK_SET);
                        fwrite(&ebr,sizeof (EBR),1,file);
                        cout<< "\u001B[32m" << "[OK] La particion " <<this->name<<" ha sido creada exitosamente"<< "\x1B[0m" << endl;
                    }
                    else{
                        cout<< "\u001B[31m" << "[BAD PARAM] Espacio insuficiente en la particion extendida "<< "\x1B[0m" << endl;
                    }
                }
            }
            else{
                cout<< "\u001B[31m" << "[BAD PARAM] No existe particion extendida en el disco"<< "\x1B[0m" << endl;
            }
        }
        else{
            cout<< "\u001B[31m" << "[BAD PARAM] Ya existe una particion llamada"<<this->name<<" en el disco"<< "\x1B[0m" << endl;
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

    int ePartitionIndex = -1;
    for(int i = 0; i<4; i++){
        if(strcmp(name,master.mbr_partitions[i].part_name) == 0){
            return true;
        }
        if(master.mbr_partitions[i].part_type == 'E'){
            ePartitionIndex = i;
        }
    }
    if(ePartitionIndex != -1){
        //Se lee EBR
        fseek(file, master.mbr_partitions[ePartitionIndex].part_start, SEEK_SET);
        EBR extended;
        //Se lee toda la particion extendida y sus EBR
        while( ftell(file)<(master.mbr_partitions[ePartitionIndex].part_size + master.mbr_partitions[ePartitionIndex].part_start) && fread(&extended, sizeof (EBR),1,file) !=0){
            if(extended.part_next == -1){
                return  false;
            }
            if(extended.part_name == name){
                return  true;
            }
        }
    }
    return  false;
}

//860