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
    string type;//Tipo de formateo
    bool statusFlag; // Indica si hay errores en el comando
public:
    MKFS_(){
        this->id="";
        this->type="";
        this->statusFlag=false;
    };
    void setId(char *value);
    void setType(char *value);
    void initFormat();
    void setStatus();
};

void MKFS_::setId(char *value){
    this->id=value;
}

void MKFS_::setType(char *value){
    this->type=value;
}

void MKFS_::setStatus(){
    this->statusFlag=true;
    if(this->id == ""){
        this->statusFlag=false;
        cout<< "\u001B[31m" << "[BAD PARAM] ID invalido"<< "\x1B[0m" << endl;
    }
    if(this->type == ""){
        this->statusFlag=false;
        cout<< "\u001B[31m" << "[BAD PARAM] Tipo de Formato Invalido"<< "\x1B[0m" << endl;
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
            int  partIndex = aux->getPartitionIndex();
            if(partIndex != -1){
                MBR master;
                FILE *file = fopen(aux->getPath().c_str(),"r+b");

                //Se lee MBR
                fseek(file,0, SEEK_SET);
                fread(&master,sizeof (MBR),1,file);
                fclose(file);

                int partStart = master.mbr_partitions[partIndex].part_start;
                int partSize = master.mbr_partitions[partIndex].part_size;

                //Formula
                double n = (partSize - sizeof(SuperBloque)) / (4 + sizeof(Journal) + sizeof (InodeTable) + 3*sizeof(BloqueArchivos));
                int numOfInodes = static_cast<int>(floor(n));
                int numOfBloques = 3*numOfInodes;

                //PENDIENTE DE NOMBRES
                int numOfJInodes = sizeof(Journal)*numOfInodes;
                int ssize = sizeof(SuperBloque)*numOfInodes;

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
                super.s_inode_start = partStart+ssize+numOfJInodes+numOfInodes+numOfBloques;
                super.s_block_start = partStart+ssize+numOfJInodes+numOfInodes+numOfBloques + static_cast<int>(sizeof(InodeTable))*numOfInodes;
                super.s_bm_inode_start = partStart + ssize + numOfJInodes;
                super.s_bm_block_start = partStart + ssize + numOfJInodes  + numOfInodes;
                
                // REVISAR DESDE AQUI
                
                //Se configura Tabla de Inodos y Bloques Iniciales
                InodeTable iNodo;
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

                // Carpeta raiz
                myChar = '1';
                fseek(file,super.s_bm_inode_start,SEEK_SET);
                fwrite(&myChar,sizeof(char),1,file);
                // Users.txt
                fwrite(&myChar,sizeof(char),1,file);

                //Se llena Bitmap de Bloques
                myChar = '0';
                for(int i = 0; i < numOfBloques; i++){
                    fseek(file,super.s_bm_block_start+ i,SEEK_SET);
                    fwrite(&myChar,sizeof(char),1,file);
                }

                //Carpeta Raiz
                myChar = '1';
                fseek(file,super.s_bm_block_start,SEEK_SET);
                fwrite(&myChar,sizeof(char),1,file);
                //Users.txt
                myChar = '2';
                fwrite(&myChar,sizeof(char),1,file);

                //Configurando Inodo para Root
                iNodo.i_uid = 1;
                iNodo.i_gid = 1;
                iNodo.i_size = 0;
                iNodo.i_atime = time(nullptr);
                iNodo.i_ctime = time(nullptr);
                iNodo.i_mtime = time(nullptr);
                iNodo.i_block[0] = 0;
                iNodo.i_type = '0';
                iNodo.i_perm = 664;

                for(int i = 1 ; i < 15;i++){
                    iNodo.i_block[i] = -1;
                }
                fseek(file,super.s_inode_start, SEEK_SET);
                fwrite(&iNodo,sizeof (InodeTable),1,file);

                //Configurando Bloque para Root
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

                // Seteando inodo para el archivo users.txt
                iNodo.i_gid = 1;
                iNodo.i_uid = 1;
                iNodo.i_perm = 755;
                iNodo.i_size = 27;
                iNodo.i_type = '1';
                iNodo.i_atime = time(nullptr);
                iNodo.i_block[0] = 1;
                iNodo.i_ctime = time(nullptr);
                iNodo.i_mtime = time(nullptr);
                for(int i = 1 ; i < 15;i++){
                    iNodo.i_block[i] = -1;
                }
                fseek(file,super.s_inode_start+static_cast<int>(sizeof(InodeTable)),SEEK_SET);
                fwrite(&iNodo,sizeof (InodeTable),1,file);

                // Seteando bloque para el archivo users.txt
                BloqueArchivos arch;
                memset(arch.b_content,0,sizeof (BloqueArchivos));
                strcpy(arch.b_content,"1,G,root\n1,U,root,root,123\n");
                fseek(file,super.s_block_start + static_cast<int>(sizeof(BloqueCarpetas)), SEEK_SET);
                fwrite(&arch,sizeof (BloqueArchivos),1,file);
                fclose(file);

                cout << "Partición formateada con EXT3 con éxito." << endl;
            }
        }
        else
        {
            cout << "La partición que deseas formatear no está montada." << endl;
        }
    }
}

