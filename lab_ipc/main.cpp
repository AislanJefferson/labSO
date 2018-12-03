#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <pthread.h>
#include <thread>
#include "constantes.h"
#include <iostream>


#define EXIT_VALUE 50
const char *SERVER_PID_FPATH = "/tmp/serverid.run";

int state = DESCONECTADO;
bool temToken = false;
bool enviando_mensagem = false;
int pidProximo = SEM_PROXIMO_PROCESSO;

void setProxPID(int novoProximo){
  pidProximo = (novoProximo != getpid()) ? novoProximo : SEM_PROXIMO_PROCESSO;
}

void sendControlSignal(int pidDestino, int value) {
    if (pidDestino > 0) {
        union sigval sv;
        // Adiciona o value_to_send como conteudo de msg do sinal
        sv.sival_int = value;
        printf("Valor de sinal %d para enviar para %d\n", value,pidDestino);
        // Envia sinal sv de cod SIGUSR1 para o PID pid
        sigqueue(pidDestino, SIGUSR2, sv);
    }
}

void sendMessageSignal(sigval msg) {
    if (pidProximo != SEM_PROXIMO_PROCESSO) {
        // Adiciona o value_to_send como conteudo de msg do sinal
        // Envia sinal sv de cod SIGUSR1 para o PID pid
        sigqueue(pidProximo, SIGUSR1, msg);
    }
}

int getRootPID() {
    int pid;
    FILE *fptr;
    if ((fptr = fopen(SERVER_PID_FPATH, "r")) == NULL) {
        pid = PID_INEXISTENTE;
    } else {
        fscanf(fptr, "%d", &pid);
        fclose(fptr);
    }
    return pid;
}

void setRootPID(int new_pid) {
    FILE *fptr;
    fptr = fopen(SERVER_PID_FPATH, "w+");

    if (fptr == NULL) {
        printf("Error!");
    } else {

        fprintf(fptr, "%d", new_pid);
        fclose(fptr);
    }
}


void handlerUSR1(int signo, siginfo_t *si, void *data) {
    (void) signo;
    (void) data;
    sigval v = si->si_value;
    if(enviando_mensagem){
      enviando_mensagem = false;
      printf("Recebi msg que enviei e nao envio mais...\n");
      sendControlSignal(pidProximo,TOKEN_PASSADO);
      temToken = false;
    }else{
      printf("Recebi msg %d e tou repassando\n", v.sival_int);
      sendMessageSignal(v);
    }
}

void handlerUSR2(int signo, siginfo_t *si, void *data) {
    (void) signo;
    (void) data;
    sigval v = si->si_value;
    switch (v.sival_int) {
        case TOKEN_PASSADO:
            temToken = true;
            break;
        case REQ_CONEXAO:
            printf("Nao estou soh seu novo prox eh %d e estou setando seu pid %d como proximo\n", pidProximo,
                   si->si_pid);
            sendControlSignal(si->si_pid, pidProximo);
            setProxPID(si->si_pid);
            break;
        case REQ_DESCONEXAO:
            if (state == DESCONECTANDO)
            {
              if (getpid() == getRootPID()) setRootPID(pidProximo);
                //
                printf("Desconectando e enviando para %d\n", pidProximo);
                sendControlSignal(pidProximo, si->si_pid);
                setProxPID(SEM_PROXIMO_PROCESSO);
                state = DESCONECTADO;
            }else{
              sendControlSignal(pidProximo, REQ_DESCONEXAO);
            }
            break;
        case NODE_CONNECT_REQ:
            // Seta o proximo para o remetente
            printf("Setei como prox o pid %d\n", si->si_pid);
            setProxPID(si->si_pid);
            break;
        case SEM_PROXIMO_PROCESSO:
            // Seta o proximo para o remetente
            printf("Setei como prox o pid %d\n", si->si_pid);
            setProxPID(si->si_pid);
            state = CONECTADO;
            break;

        default:
            if (state == CONECTANDO) {
                setProxPID(v.sival_int);
                printf("Setei como prox o pid  2 %d\n", v.sival_int);
                state = CONECTADO;
            } else {
                sendControlSignal(v.sival_int, NODE_CONNECT_REQ);
                printf("Estado diferente de conectando %d\n", state);
            }
            break;
    }
}

void registraHandlers() {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);

    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handlerUSR2;


    if (sigaction(SIGUSR2, &sa, 0) == -1) {
        fprintf(stderr, "%s: %s\n", "sigaction", strerror(errno));
    }

    struct sigaction sa1;
    sigemptyset(&sa1.sa_mask);

    sa1.sa_flags = SA_SIGINFO;
    sa1.sa_sigaction = handlerUSR1;


    if (sigaction(SIGUSR1, &sa1, 0) == -1) {
        fprintf(stderr, "%s: %s\n", "sigaction", strerror(errno));
    }


}

void connect() {

    int rootPid = getRootPID();
    if (rootPid == PID_INEXISTENTE) {
        setRootPID(getpid());
        temToken = true;
        state = CONECTADO;
    } else {
        sendControlSignal(rootPid, REQ_CONEXAO);
        state = CONECTANDO;
    }

}

void disconnect() {
    if(state == CONECTADO){
      if(pidProximo == SEM_PROXIMO_PROCESSO){
        setRootPID(PID_INEXISTENTE);
        state = DESCONECTADO;
        printf("%s\n","Desconectado!" );
      }else{
        sendControlSignal(pidProximo, REQ_DESCONEXAO);
        state = DESCONECTANDO;printf("%s\n","Desconectando!" );
      }
    }else{
      printf("%s\n","Ja desconectado" );
    }
}

void send(int valor) {
  if(temToken){
    union sigval sv;
    // Adiciona o value_to_send como conteudo de msg do sinal
    sv.sival_int = valor;
    enviando_mensagem = true;
    sendMessageSignal(sv);
  }
}

void loop() {

}

int main(void) {
    registraHandlers();

    printf("%d\n", getpid());
    for (;;) {
        char input;
        printf("PID:%d proximo:%d e estado:%d\nComando: ", getpid(),pidProximo,state);
        input = getchar();
        switch (input) {
            case 'c':
                connect();
                break;
            case 'd':
                disconnect();
                break;
            case 'm':
                int msg;
                scanf("%d",&msg);
                send(msg);
                break;
        }

    }
    return 0;
}
