#!/bin/bash
if [[ $# == 2 ]]; then
  rm -rf /tmp/serverid.run
  g++ -c ipcring.cpp -o ipcring.o
  ar rvs ipcring.a ipcring.o  &> /dev/null
  g++ $1 ipcring.a -o $2 -lpthread
  rm -rf ipcring.a ipcring.o
  echo "Programa $2 criado com sucesso!"
else
  echo -e "Uso:\n\tsh build.sh <codigofonte.cpp> <destino>"
fi
