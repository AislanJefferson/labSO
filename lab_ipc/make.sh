rm -rf /tmp/serverid.run
g++ -c ipcring.cpp -o ipcring.o
ar rvs ipcring.a ipcring.o
g++ main.cpp ipcring.a -o main -lpthread
