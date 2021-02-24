#ifndef MKFS_H
#define MKFS_H

#endif // MKFS_H
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <MOUNT.h>
#include <list>
#include <iterator>

using namespace std;

extern list<MOUNT_> *mounted;

class MKFS_{
private:
    string id;//ID de la particion
    string type;//Tipo de sistema
    string typeFormat;//Tipo de formateo
    bool statusFlag; // Indica si hay errores en el comando
public:
    MKFS_(){
        this->id="";
        this->type="2fs";
        this->typeFormat="fast";
        this->statusFlag=false;
    };

    /**
     * Setter del ID
    */
    void setId(char *value);

    /**
     * Setter del Tipo de Sistema
    */
    void setType(char *value);

    /**
     * Setter del tipo de formateo
    */
    void setTypeFormat(char *value);
    
    /**
     * Inicia elm formateo de la particion
    */
    void initFormat();
    
    /**
     * Verifica que no hya errores en el comando
    */
    void setStatus();

    /**
     * Formatea como Ext2
    */
    void formatExt3(int partIndex,MOUNT_ *aux);

    /**
     * Formatea como Ext2
    */
    void formatExt2(int partIndex,MOUNT_ *aux);
};

void MKFS_::setId(char *value){
    this->id=value;
}

void MKFS_::setType(char *value){
    this->type=value;
}

void MKFS_::setTypeFormat(char *value){
    this->typeFormat=value;
}

void MKFS_::setStatus(){
    this->statusFlag=true;
    if(this->id == ""){
        this->statusFlag=false;
        cout<< "\u001B[31m" << "[BAD PARAM] ID invalido"<< "\x1B[0m" << endl;
    }
    if(this->type != "2fs" && this->type != "3fs"){
        this->statusFlag=false;
        cout<< "\u001B[31m" << "[BAD PARAM] Tipo de Formato de Sistema Invalido"<< "\x1B[0m" << endl;
    }
    if(this->typeFormat != "fast" && this->type != "full"){
        this->statusFlag=false;
        cout<< "\u001B[31m" << "[BAD PARAM] Tipo de Formateo Invalido"<< "\x1B[0m" << endl;
    }
}

void MKFS_::initFormat(){
    setStatus();
    if(this->statusFlag){
        list<MOUNT_>::iterator i;
        MOUNT_ *aux;
        bool boolFind = false;
        for(i=mounted->begin(); i != mounted->end();i++){
            if(i->getId() == this->id){
                aux = &*i;
                boolFind = true;
                break;
            }
        }
        if(boolFind){
            int  partIndex;
            partIndex = aux->findPartitionIndex();
            if(partIndex != -1){

                //Para particiones ext3
                if(this->type=="3fs"){
                    formatExt3(partIndex,aux);
                }
                //Para particiones ext2
                else if(this->type=="2fs"){
                    formatExt2(partIndex,aux);
                }
                else{
                    cout<< "\u001B[31m" << "[BAD PARAM] No se especifico el tipo de formateo"<< "\x1B[0m" << endl; 
                }
            }
            else{
                partIndex= aux->findLogicPartitionStart();
                if(partIndex != -1){
                    //Para particiones ext3
                    if(this->type=="3fs"){
                        formatExt3(partIndex,aux);
                    }
                    //Para particiones ext2
                    else if(this->type=="2fs"){
                        formatExt2(partIndex,aux);
                    }
                    else{
                        cout<< "\u001B[31m" << "[BAD PARAM] No se especifico el tipo de formateo"<< "\x1B[0m" << endl; 
                    }
                }
            }
        }
        else{
            cout<< "\u001B[31m" << "[BAD PARAM] La particion no esta montada"<< "\x1B[0m" << endl; 
        }
    }
}

void MKFS_::formatExt3(int partIndex,MOUNT_ *aux){
    int partStart;
    int partSize;
    FILE *file;

    if(partIndex>=0 && partIndex <=3){
        MBR master;
        file = fopen(aux->getPath().c_str(),"r+b");

        //Se lee MBR
        fseek(file,0, SEEK_SET);
        fread(&master,sizeof (MBR),1,file);
        fclose(file);

        partStart = master.mbr_partitions[partIndex].part_start;
        partSize = master.mbr_partitions[partIndex].part_size;
    }
    else{
        EBR extendedBoot;
        file = fopen(aux->getPath().c_str(),"r+b");
        //Se lee EBR
        fseek(file,partIndex, SEEK_SET);
        fread(&extendedBoot,sizeof (EBR),1,file);
        fclose(file);
        partStart = extendedBoot.part_start;
        partSize = extendedBoot.part_size;

    }

    //Formula
    double n = (partSize - sizeof(SuperBloque)) / (4 + sizeof(Journal) + sizeof (InodeTable) + 3*sizeof(BloqueArchivos));
    int numOfInodes = static_cast<int>(floor(n));
    int numOfBloques = 3*numOfInodes;

    int JournalSize = sizeof(Journal)*numOfInodes;

    //Se configura SuperBloque
    SuperBloque super;
    super.s_filesystem_type = 3;
    super.s_inodes_count = numOfInodes;
    super.s_blocks_count = numOfBloques;
    super.s_free_blocks_count = numOfBloques - 2;
    super.s_free_inodes_count = numOfInodes - 2;
    super.s_mtime = time(nullptr);
    super.s_umtime = 0;
    super.s_mnt_count = 0;
    super.s_magic = 0xEF53;
    super.s_inode_size = sizeof (InodeTable);
    super.s_block_size = sizeof(BloqueArchivos);
    super.s_first_ino = 2;
    super.s_first_blo = 2;
    super.s_bm_inode_start = partStart + sizeof(SuperBloque) + JournalSize;
    super.s_bm_block_start = partStart + sizeof(SuperBloque) + JournalSize  + numOfInodes;
    super.s_inode_start = partStart+sizeof(SuperBloque)+JournalSize+numOfInodes+numOfBloques;
    super.s_block_start = partStart+sizeof(SuperBloque)+JournalSize+numOfInodes+numOfBloques + static_cast<int>(sizeof(InodeTable))*numOfInodes;
    
    
    //Se configura Tabla de Inodos y Bloques Iniciales
    InodeTable iNode;
    BloqueCarpetas bloque;
    
    file = fopen(aux->getPath().data(),"r+b");

    //Se escribe el SuperBloque
    fseek(file,partStart,SEEK_SET);
    fwrite(&super,sizeof (SuperBloque),1,file);

    //Se escribe el Bitmap de inodos
    char myChar = '0';
    for(int i = 0; i < numOfInodes; i++){
        fseek(file,super.s_bm_inode_start + i,SEEK_SET);
        fwrite(&myChar,sizeof(char),1,file);
    }

    //Se marca bit para raiz y user.txt
    myChar = '1';
    fseek(file,super.s_bm_inode_start,SEEK_SET);
    fwrite(&myChar,sizeof(char),1,file);
    fwrite(&myChar,sizeof(char),1,file);

    //Se escribe Bitmap de Bloques
    myChar = '0';
    for(int i = 0; i < numOfBloques; i++){
        fseek(file,super.s_bm_block_start+ i,SEEK_SET);
        fwrite(&myChar,sizeof(char),1,file);
    }

    //Se marca bit para raiz y user.txt
    myChar = '1';
    fseek(file,super.s_bm_block_start,SEEK_SET);
    fwrite(&myChar,sizeof(char),1,file);
    myChar = '2';
    fwrite(&myChar,sizeof(char),1,file);

    //Configurando Inodo para Root
    iNode.i_uid = 1;
    iNode.i_gid = 1;
    iNode.i_size = 0;
    iNode.i_atime = time(nullptr);
    iNode.i_ctime = time(nullptr);
    iNode.i_mtime = time(nullptr);
    iNode.i_block[0] = 0;
    for(int i = 1 ; i < 15;i++){
        iNode.i_block[i] = -1;
    }
    iNode.i_type = '0';
    iNode.i_perm = 664;
    
    fseek(file,super.s_inode_start, SEEK_SET);
    fwrite(&iNode,sizeof (InodeTable),1,file);

    //Configurando Bloque para carpeta Root
    strcpy(bloque.b_content[0].b_name ,"-");
    bloque.b_content[0].b_inodo= 0;
    strcpy(bloque.b_content[1].b_name, "-");
    bloque.b_content[1].b_inodo = 0;
    strcpy(bloque.b_content[2].b_name, "users.txt");
    bloque.b_content[2].b_inodo = 1;
    strcpy(bloque.b_content[3].b_name, "-");
    bloque.b_content[3].b_inodo = 1;

    //Se escribe el bloque
    fseek(file,super.s_block_start,SEEK_SET);
    fwrite(&bloque, sizeof (BloqueArchivos),1,file);

    //Configurando Inodo para users.txt
    iNode.i_gid = 1;
    iNode.i_uid = 1;
    iNode.i_size = 27;              
    iNode.i_atime = time(nullptr);
    iNode.i_ctime = time(nullptr);
    iNode.i_mtime = time(nullptr);
    iNode.i_block[0] = 1;
    for(int i = 1 ; i < 15;i++){
        iNode.i_block[i] = -1;
    }
    iNode.i_type = '1';
    iNode.i_perm = 755;
    fseek(file,super.s_inode_start+static_cast<int>(sizeof(InodeTable)),SEEK_SET);
    fwrite(&iNode,sizeof (InodeTable),1,file);

    //Configurando Bloque para carpeta users.txt
    BloqueArchivos archivoBin;
    memset(archivoBin.b_content,0,sizeof (BloqueArchivos));
    strcpy(archivoBin.b_content,"1,G,root\n1,U,root,root,123\n");
    fseek(file,super.s_block_start + static_cast<int>(sizeof(BloqueCarpetas)), SEEK_SET);
    fwrite(&archivoBin,sizeof (BloqueArchivos),1,file);
    fclose(file);

    cout<< "\u001B[32m" << "[OK] Particion formateada como EXT3"<< "\x1B[0m" << endl; 
}

void MKFS_::formatExt2(int partIndex,MOUNT_ *aux){

    int partStart;
    int partSize;
    FILE *file;

    if(partIndex>=0 && partIndex <=3){
        MBR master;
        file = fopen(aux->getPath().c_str(),"r+b");

        //Se lee MBR
        fseek(file,0, SEEK_SET);
        fread(&master,sizeof (MBR),1,file);
        fclose(file);

        partStart = master.mbr_partitions[partIndex].part_start;
        partSize = master.mbr_partitions[partIndex].part_size;
    }
    else{
        EBR extendedBoot;
        file = fopen(aux->getPath().c_str(),"r+b");

        //Se lee EBR
        fseek(file,partIndex, SEEK_SET);
        fread(&extendedBoot,sizeof (EBR),1,file);
        fclose(file);

        partStart = extendedBoot.part_start;
        partSize = extendedBoot.part_size;

    }

    //Formula
    double n = (partSize - sizeof(SuperBloque)) / (4 + sizeof(Journal) + sizeof (InodeTable) + 3*sizeof(BloqueArchivos));
    int numOfInodes = static_cast<int>(floor(n));
    int numOfBloques = 3*numOfInodes;

    int JournalSize = sizeof(Journal)*numOfInodes;

    //Se configura SuperBloque
    SuperBloque super;
    super.s_filesystem_type = 2;
    super.s_inodes_count = numOfInodes;
    super.s_blocks_count = numOfBloques;
    super.s_free_blocks_count = numOfBloques - 2;
    super.s_free_inodes_count = numOfInodes - 2;
    super.s_mtime = time(nullptr);
    super.s_umtime = 0;
    super.s_mnt_count = 0;
    super.s_magic = 0xEF53;
    super.s_inode_size = sizeof (InodeTable);
    super.s_block_size = sizeof(BloqueArchivos);
    super.s_first_ino = 2;
    super.s_first_blo = 2;
    super.s_bm_inode_start = partStart + sizeof(SuperBloque);
    super.s_bm_block_start = partStart + sizeof(SuperBloque) + numOfInodes;
    super.s_inode_start = partStart+sizeof(SuperBloque)+numOfInodes+numOfBloques;
    super.s_block_start = partStart+sizeof(SuperBloque)+numOfInodes+numOfBloques + static_cast<int>(sizeof(InodeTable))*numOfInodes;
    
    
    //Se configura Tabla de Inodos y Bloques Iniciales
    InodeTable iNode;
    BloqueCarpetas bloque;
    
    file = fopen(aux->getPath().data(),"r+b");

    //Se escribe el SuperBloque
    fseek(file,partStart,SEEK_SET);
    fwrite(&super,sizeof (SuperBloque),1,file);

    //Se escribe el Bitmap de inodos
    char myChar = '0';
    for(int i = 0; i < numOfInodes; i++){
        fseek(file,super.s_bm_inode_start + i,SEEK_SET);
        fwrite(&myChar,sizeof(char),1,file);
    }

    //Se marca bit para raiz y user.txt
    myChar = '1';
    fseek(file,super.s_bm_inode_start,SEEK_SET);
    fwrite(&myChar,sizeof(char),1,file);
    fwrite(&myChar,sizeof(char),1,file);

    //Se escribe Bitmap de Bloques
    myChar = '0';
    for(int i = 0; i < numOfBloques; i++){
        fseek(file,super.s_bm_block_start+ i,SEEK_SET);
        fwrite(&myChar,sizeof(char),1,file);
    }

    //Se marca bit para raiz y user.txt
    myChar = '1';
    fseek(file,super.s_bm_block_start,SEEK_SET);
    fwrite(&myChar,sizeof(char),1,file);
    myChar = '2';
    fwrite(&myChar,sizeof(char),1,file);

    //Configurando Inodo para Root
    iNode.i_uid = 1;
    iNode.i_gid = 1;
    iNode.i_size = 0;
    iNode.i_atime = time(nullptr);
    iNode.i_ctime = time(nullptr);
    iNode.i_mtime = time(nullptr);
    iNode.i_block[0] = 0;
    for(int i = 1 ; i < 15;i++){
        iNode.i_block[i] = -1;
    }
    iNode.i_type = '0';
    iNode.i_perm = 664;
    
    fseek(file,super.s_inode_start, SEEK_SET);
    fwrite(&iNode,sizeof (InodeTable),1,file);

    //Configurando Bloque para carpeta Root
    strcpy(bloque.b_content[0].b_name ,"-");
    bloque.b_content[0].b_inodo= 0;
    strcpy(bloque.b_content[1].b_name, "--");
    bloque.b_content[1].b_inodo = 0;
    strcpy(bloque.b_content[2].b_name, "users.txt");
    bloque.b_content[2].b_inodo = 1;
    strcpy(bloque.b_content[3].b_name, "-");
    bloque.b_content[3].b_inodo = 1;

    //Se escribe el bloque
    fseek(file,super.s_block_start,SEEK_SET);
    fwrite(&bloque, sizeof (BloqueArchivos),1,file);

    //Configurando Inodo para users.txt
    iNode.i_gid = 1;
    iNode.i_uid = 1;
    iNode.i_size = 27;              
    iNode.i_atime = time(nullptr);
    iNode.i_ctime = time(nullptr);
    iNode.i_mtime = time(nullptr);
    iNode.i_block[0] = 1;
    for(int i = 1 ; i < 15;i++){
        iNode.i_block[i] = -1;
    }
    iNode.i_type = '1';
    iNode.i_perm = 755;
    fseek(file,super.s_inode_start+static_cast<int>(sizeof(InodeTable)),SEEK_SET);
    fwrite(&iNode,sizeof (InodeTable),1,file);

    //Configurando Bloque para carpeta users.txt
    BloqueArchivos archivoBin;
    memset(archivoBin.b_content,0,sizeof (BloqueArchivos));
    strcpy(archivoBin.b_content,"1,G,root\n1,U,root,root,123\n");
    fseek(file,super.s_block_start + static_cast<int>(sizeof(BloqueCarpetas)), SEEK_SET);
    fwrite(&archivoBin,sizeof (BloqueArchivos),1,file);
    fclose(file);

    cout<< "\u001B[32m" << "[OK] Particion formateada como EXT2"<< "\x1B[0m" << endl; 

}