#!/bin/bash

clear
# Compilar o servidor
make -f makefiles/client_makefile all

# Limpar arquivos de objeto
make -f makefiles/client_makefile clean

# Executar o programa
./class_client 127.0.0.1 9000

rm -f class_client