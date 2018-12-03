#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <pthread.h>
#include <thread>
#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "constantes.h"
#include "cores.h"

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
        background(YELLOW);
        printf("Enviando sinal %d para %d", value, pidDestino);
        style(RESETALL); printf("\n");
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
        background(RED);
        printf("Error!");
        style(RESETALL); printf("\n");
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
      background(GREEN);
      printf("Recebi msg que enviei e nao envio mais...");
      style(RESETALL); printf("\n");
      sendControlSignal(pidProximo,TOKEN_PASSADO);
      temToken = false;
    }else{

      key_t key = v.sival_int; /* key to be passed to shmget() */
      int shmid; /* return value from shmget() */
      void *shm;
      int *s;
      if ((shmid = shmget(key, sizeof(int), 0666)) < 0) {
        background(RED);
        perror("shmget");
        style(RESETALL); printf("\n");
        exit(1);
    }

      /*
       * Now we attach the segment to our data space.
       */
      if ((shm = shmat(shmid, NULL, 0)) == (int *) -1) {
          background(RED);
          perror("shmat");
          style(RESETALL); printf("\n");
          exit(1);
      }
      /*
       * Now put some things into the memory for the
       * other process to read.
       */
      s = (int *) shm;
      background(BLUE);
      printf("Recebi msg %d e tou repassando", *s);
      style(RESETALL); printf("\n");
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
          // PRINTS DE INFORMACAO
            if(pidProximo != SEM_PROXIMO_PROCESSO) {
              background(YELLOW);
              printf("Nao estou soh seu novo prox eh %d e estou setando seu pid %d como proximo", pidProximo,
                     si->si_pid);
              style(RESETALL); printf("\n");
            }

           // FIM PRINTS
            sendControlSignal(si->si_pid, pidProximo);
            setProxPID(si->si_pid);
            break;
        case REQ_DESCONEXAO:
            if (state == DESCONECTANDO)
            {
              if (getpid() == getRootPID()) setRootPID(pidProximo);
                //
                background(YELLOW);
                printf("Desconectando e enviando para %d", pidProximo);
                style(RESETALL); printf("\n");
                sendControlSignal(pidProximo, si->si_pid);
                setProxPID(SEM_PROXIMO_PROCESSO);
                state = DESCONECTADO;
            }else{
              sendControlSignal(pidProximo, REQ_DESCONEXAO);
            }
            break;
        case NODE_CONNECT_REQ:
            // Seta o proximo para o remetente
            background(YELLOW);
            printf("Setei como prox o pid %d", si->si_pid);
            style(RESETALL); printf("\n");
            setProxPID(si->si_pid);
            break;
        case SEM_PROXIMO_PROCESSO:
            // Seta o proximo para o remetente
            background(YELLOW);
            printf("Setei como prox o pid %d", si->si_pid);
            style(RESETALL); printf("\n");
            setProxPID(si->si_pid);
            state = CONECTADO;
            break;

        default:
            if (state == CONECTANDO) {
                setProxPID(v.sival_int);
                background(YELLOW);
                printf("Setei como prox o pid  %d", v.sival_int);
                style(RESETALL); printf("\n");
                state = CONECTADO;
            } else {
                sendControlSignal(v.sival_int, NODE_CONNECT_REQ);
                background(YELLOW);
                printf("Estado diferente de conectando %d", state);
                style(RESETALL); printf("\n");
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
        background(RED);
        fprintf(stderr, "%s: %s", "sigaction", strerror(errno));
        style(RESETALL); printf("\n");
    }

    struct sigaction sa1;
    sigemptyset(&sa1.sa_mask);

    sa1.sa_flags = SA_SIGINFO;
    sa1.sa_sigaction = handlerUSR1;


    if (sigaction(SIGUSR1, &sa1, 0) == -1) {
        background(RED);
        fprintf(stderr, "%s: %s", "sigaction", strerror(errno));
        style(RESETALL); printf("\n");
    }


}

void connect() {
    if(state == DESCONECTADO){
      int rootPid = getRootPID();
      if (rootPid == PID_INEXISTENTE) {
          setRootPID(getpid());
          temToken = true;
          state = CONECTADO;
      } else {
          sendControlSignal(rootPid, REQ_CONEXAO);
          state = CONECTANDO;
      }
    }else{
      background(RED);
      printf("Ja estou conectado!");
      style(RESETALL); printf("\n");
    }
}

void disconnect() {
    if(state == CONECTADO){
      if(pidProximo == SEM_PROXIMO_PROCESSO){
        setRootPID(PID_INEXISTENTE);
        state = DESCONECTADO;
        background(WHITE);
        foreground(BLACK);
        printf("%s","Desconectado!" );
        style(RESETALL); printf("\n");
      }else{
        sendControlSignal(pidProximo, REQ_DESCONEXAO);
        state = DESCONECTANDO;
        background(WHITE);
        foreground(BLACK);
        printf("%s","Desconectando!" );
        style(RESETALL); printf("\n");
      }
    }else{
      background(WHITE);
      foreground(BLACK);
      printf("%s","Ja desconectado" );
      style(RESETALL); printf("\n");
    }
}

void send(int valor) {

  if(temToken){
    /* Intializes random number generator */
    time_t t;
    srand((unsigned) time(&t));
    key_t key = rand() % RAND_MAX; /* key to be passed to shmget() */
    int shmid; /* return value from shmget() */
    void *shm;
    int *s;
    if ((shmid = shmget(key, sizeof(valor), IPC_CREAT | 0666)) < 0) {
        background(RED);
        perror("shmget");
        style(RESETALL); printf("\n");
        exit(1);
    }
    /*
     * Now we attach the segment to our data space.
     */
    if ((shm = shmat(shmid, NULL, 0)) == (int *) -1) {
        background(RED);
        perror("shmat");
        style(RESETALL); printf("\n");
        exit(1);
    }
    /*
     * Now put some things into the memory for the
     * other process to read.
     */
    s = (int *) shm;
    *s = valor;
    union sigval sv;
    // Adiciona o value_to_send como conteudo de msg do sinal
    sv.sival_int = key;
    enviando_mensagem = true;
    sendMessageSignal(sv);
  }else{
    background(RED);
    printf("Não tenho o token!");
    style(RESETALL);
    printf("\n");
  }
}

void loop() {

}

int main(void) {
    registraHandlers();
    char input = 's';
    while(input != 'e') {
        background(WHITE);
        foreground(BLACK);
        printf("PID:%d proximo:%d e estado:%d", getpid(),pidProximo,state);
        style(RESETALL);
        printf("\n");
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
