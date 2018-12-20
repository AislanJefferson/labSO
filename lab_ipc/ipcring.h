#ifndef IPCRING_H_INCLUDED
#define IPCRING_H_INCLUDED

void registraHandlers(bool debug);

bool join();

bool leave();

bool send(int valor);

int receive();

#endif
