#!/bin/bash

clear
# Compilar o servidor
make -f makefiles/admin_makefile all

# Limpar arquivos de objeto
make -f makefiles/admin_makefile clean

# Executar o programa
echo "Executando o admin UDP..."
./class_admin 127.0.0.1 9876