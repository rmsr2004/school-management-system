#!/bin/bash
clear
make -f makefiles/server_makefile all

# Limpar arquivos de objeto
make -f makefiles/server_makefile clean

# Executar o programa
echo "Executando o servidor..."
./class_server 9000 9876 server/config.txt