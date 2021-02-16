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


using namespace std;

class REP_{
private:
    char *id; //ID de la particion
    char *ruta;/// Destino del reporte.
    string path;/// Ruta auxiliar del disco que se leera.
    string cont;
    char *name;/// Nombre que se le dará al reporte.
    bool correct;/// Semántica de la acción.
    bool isMounted;/// Indica si la partición que se desea está montada.
public:
    /**
     * Constructor
    */
    REP_():id(0),ruta(0),path(""),name(0),correct(false),isMounted(false){}
    /**
     * Configura el id de la partición montada a escribir.
     * @param id: identificador de la partición montada en el sistema.
    */
    void setId(char* id);

    /**
     * Configura la ruta en donde se guardará el reporte.
     * @param path: ruta que se pretende guardar el reporte.
    */
    void setPath(char* path);

    /**
     * Configura la ruta en la que se guardará
     * ya que se necesitan variantes para guardar el dot
     * y para la imagen.
    */
    void confRuta();

    /**
     * Guarda el valor de la ruta
     * @param ruta: dirección en la que se almacenará el reporte.
    */
    void setRuta(char* ruta)
    {
        this->ruta = ruta;
    }

    void setCont(char* cont)
    {
        this->cont = cont;
    }

    /**
     * Guarda el nombre de la partición montada que se reportará.
     * @param name: indica el nombre de la partición en el sistema de particiones montadas.
    */
    void setName(char* name)
    {
        this->name = name;
    }

    /*
     * Obtiene el id de la partición.
    */
    char* getId()
    {
        return this->id;
    }

    /*
     * Obtiene la ruta que se eligió como destino.
    */
    char* getRuta()
    {
        return  this->ruta;
    }

    /**
     * Obtiene el nombre que se le dará al rerporte.
    */
    char* getName()
    {
        return  this->name;
    }

    /**
     * Método para verificar semántica de la operación
    */
    void semantic();

    /**
     * Método para ejecutar
    */
    void run();

    /**
     * Método para convertir un string a minúsculas.
    */
    string toLowerString(string input);

    /**
     * Método que obtiene el directorio de un archivo.
    */
    string dirNam(string);

    /**
     * Método para generar el reporte de MBR
    */
    void reportMBR();

    /**
     * Método para generar el reporte del disco
    */
    void reportDISK();

};
