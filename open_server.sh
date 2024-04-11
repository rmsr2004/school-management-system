#!/bin/bash
clear
make -f makefiles/server_makefile

# Limpar arquivos de objeto
make -f makefiles/server_makefile clean

# Executar o programa
echo "Executando o servidor..."
./server/server 9000 9876 server/config.txt