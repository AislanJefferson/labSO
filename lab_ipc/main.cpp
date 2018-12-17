#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "constantes.h"
#include "cores.h"
#include "mensagem.h"


// Dados especifico de cada processo
int state = DESCONECTADO;
bool enviando_mensagem = false;
int pidProximo = SEM_PROXIMO_PROCESSO;
mensagem msgRecebida = mensagem_padrao;
//


/**
 * Preenche o campo pidProximo como novoProximo
**/
void setProxPID(int novoProximo){
  pidProximo = (novoProximo != getpid()) ? novoProximo : SEM_PROXIMO_PROCESSO;
}

/**
 * Funcao que trata o envio de sinais de controle.
**/
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

/**
 * Funcao que trata o envio de sinais de mensagem (inteiros).
 **/
void sendMessageSignal(sigval msg) {
    if (pidProximo != SEM_PROXIMO_PROCESSO) {
        // Adiciona o value_to_send como conteudo de msg do sinal
        // Envia sinal sv de cod SIGUSR1 para o PID pid
        sigqueue(pidProximo, SIGUSR1, msg);
    }
}


/**
 * Retorna o PID do primeiro processo a compor o anel.
 **/
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

/**
 * Seta o PID new_pid para o RootPID
 * Este metodo eh chamado quando um anel eh criado ou quando
 * o processo root precisa setar o seu proximo como novo root.
 **/
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

void *getMemoria(key_t key, bool create = false){
  int shmid; /* return value from shmget() */
  void *shm;
  if(create){
    shmid = shmget(key, sizeof(int), IPC_CREAT | 0666);
  }else{
    shmid = shmget(key, sizeof(int), 0666);
  }
  if (shmid < 0) {
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
  return shm;
}

void setTokenPID(int novoDono){
  //
  // SEMAFORO PARA GARANTIR APENAS UM ACESSO POR VEZ
  // declaring key for semaphore
  key_t semKey = TOKEN_KEY;

  // requesting kernel to return semaphore memory id
  int semid =
      semget(semKey, 1, IPC_CREAT | 0666); // semkey, no.of sem, flg|permission

  if (semid == -1)
    perror("semget");
  else {
    // ACQUIRING SEMAPHORE:semval = 1 means semaphore is available
    // semid, semaphore number, setvalue = 1
    if (semctl(semid, 0, SETVAL, 1) == -1)
      perror("smctl");
    else {
      // creating semaphore structure
      struct sembuf x = {0, -1, SEM_UNDO}; // semaphore number, decrement operation,
                                    // IPC_NOWAIT:process should release sem
                                    // explicitly

      // perform locking operation on
      if (semop(semid, &x, 1) == -1)
        perror("semop");
      else {
        printf("semaphore locked \n");
        int *memoriaToken = (int *) getMemoria(ENDERECO_TOKEN,true);
        *memoriaToken = novoDono;
        shmdt(memoriaToken);
        semctl(semid,0,SETVAL,1);
        printf("semaphore unlocked \n");
      }
    }
  }
}

int getTokenPID(){
  // declara key pro semaforo
  key_t semKey = TOKEN_KEY;

  // requesting kernel to return semaphore memory id
  int semid =
      semget(semKey, 1, IPC_CREAT | 0666); // semkey, no.of sem, flg|permission

  if (semid == -1)
    perror("semget");
  else {
    struct sembuf x = {0, -1, 0}; // semaphore number, decrement operation, IPC_NOWAIT
    // perform locking operation on
    if (semop(semid, &x, 1) == -1)
      perror("semop");
    else {
      printf("semaphore locked \n");
      int *memoriaToken = (int *) getMemoria(ENDERECO_TOKEN,true);
      int valorRetorno = *(memoriaToken);
      shmdt(memoriaToken);
      // release explicitly
      semctl(semid, 0, SETVAL, 1);
      printf("semaphore unlocked \n");
      return valorRetorno;
    }
  }
}

bool temToken(){
  return getTokenPID() == getpid();
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

      setTokenPID(pidProximo);

    }else{
      int *memoriaMsg = (int *) getMemoria(v.sival_int);
      msgRecebida.dado = *(memoriaMsg);
      shmdt(memoriaMsg);
      msgRecebida.eh_valido = 1;
      sendMessageSignal(v);
    }
}

void handlerUSR2(int signo, siginfo_t *si, void *data) {
    (void) signo;
    (void) data;
    sigval v = si->si_value;
    switch (v.sival_int) {
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
          setTokenPID(getpid());
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

  if(state == CONECTADO && temToken()){
    /* Intializes random number generator */
    time_t t;
    srand((unsigned) time(&t));
    key_t key = rand(); /* key to be passed to shmget() */
    int *s;
    s = (int *) getMemoria(key,true);
    *s = valor;
    shmdt(s);
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

void receive() {
  if(state == CONECTADO){
    int token = temToken();
    while(!token && !msgRecebida.eh_valido) sleep(60);
    if(msgRecebida.eh_valido){
      msgRecebida.eh_valido = 0;
      background(BLUE);
      printf("Recebi msg %d e tou repassando", msgRecebida.dado);
      style(RESETALL); printf("\n");
    }
  }
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
            case 'r':
                receive();
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
