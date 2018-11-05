#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[ ]) {
    if(argc == 1) {
	printf("Uso: cliente valor_a_enviar PID_destino");
    }
    //converte pra int primeiro argumento
    int pid = atoi(argv[2]);
    // 
    int value_to_send = atoi(argv[1]);
    union sigval sv;
    // Adiciona o value_to_send como conteudo de msg do sinal
    sv.sival_int = value_to_send;
    printf("Enviando sinal SIGUSR1 com valor %d para %d\n",sv.sival_int,pid);
    // Envia sinal sv de cod SIGUSR1 para o PID pid
    sigqueue(pid, SIGUSR1, sv);
}
