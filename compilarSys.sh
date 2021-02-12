#!/bin/bash
echo "BISON..."
bison -o parser.cpp --defines=parser.h sintactico.y
echo "FLEX..."
flex -o scanner.cpp --header-file=scanner.h lexico.l
echo "SYSTEM..."
g++ -std=c++11 -I./  main.cpp parser.cpp scanner.cpp -o Ejecutable
echo "Generación terminada"


