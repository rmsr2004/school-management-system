#!/bin/bash

clear
# Compilar o servidor
make -f makefiles/client_makefile all

# Limpar arquivos de objeto
make -f makefiles/client_makefile clean

# Executar o programa
echo "Executando o cliente TCP..."
./client/client 127.0.0.1 9000
