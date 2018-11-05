#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char* SERVER_PID_FPATH = "/var/run/serverid.run";

int main(int argc, char *argv[ ]) {
    if(argc != 2) {
	printf("Uso: cliente valor_a_enviar");
	exit(1);
    }
    int pid;

    FILE *fptr;
    if ((fptr = fopen(SERVER_PID_FPATH,"r")) == NULL){
        printf("Error! opening file");

        // Program exits if the file pointer returns NULL.
        exit(1);
    }
    fscanf(fptr,"%d", &pid);
    fclose(fptr); 

    int value_to_send = atoi(argv[1]);
    union sigval sv;
    // Adiciona o value_to_send como conteudo de msg do sinal
    sv.sival_int = value_to_send;
    printf("Enviando sinal SIGUSR1 com valor %d para %d\n",sv.sival_int,pid);
    // Envia sinal sv de cod SIGUSR1 para o PID pid
    sigqueue(pid, SIGUSR1, sv);
    exit(0);
}
