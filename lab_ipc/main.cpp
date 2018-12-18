#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <iostream>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "constantes.h"
#include "cores.h"
#include "mensagem.h"


// Variaveis especificas de cada processo
int state = DESCONECTADO;
bool enviando_mensagem = false;
int pidProximo = SEM_PROXIMO_PROCESSO;
mensagem msgRecebida = mensagem_padrao;
sem_t semaphore_msg;
int semaphore_status = sem_init(&semaphore_msg, 0, 0);
//


/**
 * Preenche o campo pidProximo como novoProximo.
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
 * Seta o PID new_pid para o RootPID.
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

/**
 * Recebe um id key e retorna um ponteiro pra um segmento
 * de memoria.
 * Se create=true, eh acionada a flag IPC_CREAT o qual cria
 * uma memoria compartilhada entre processos.
 **/
void *getMemoria(key_t key, bool create = false){
  int shmid;
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
   //Anexa o segmento a nossa area de dados
  if ((shm = shmat(shmid, NULL, 0)) == (int *) -1) {
      background(RED);
      perror("shmat");
      style(RESETALL); printf("\n");
      exit(1);
  }
  return shm;
}
/**
 * Preenche uma area da memoria compartilhada com o PID do
 * processo que tem posse do token.
 **/
void setTokenPID(int novoDono){
  // Semaforo para garantir apenas um acesso por vez
  // Declara uma chave para o semaforo
  key_t semKey = TOKEN_KEY;

  // Pede ao kernel para retornar o id do semaforo
  int semid = semget(semKey, 1, IPC_CREAT | 0666);

  if (semid == -1)
    perror("semget");
  else {
    if (semctl(semid, 0, SETVAL, 1) == -1)
      perror("smctl");
    else {
      // cria um struct do semaforo
      struct sembuf x = {0, -1, SEM_UNDO};

      // efetua operacao de lock no semaforo
      if (semop(semid, &x, 1) == -1)
        perror("semop");
      else {
        foreground(YELLOW);
        printf("Semaforo trancado para setToken\n");
        foreground(GREEN);
        printf("Escrevendo o novo dono do token na memoria de token...\n");
        int *memoriaToken = (int *) getMemoria(ENDERECO_TOKEN,true);
        *memoriaToken = novoDono;
        shmdt(memoriaToken);
        semctl(semid,0,SETVAL,1);
        foreground(YELLOW);
        printf("Semaforo destrancado para setToken");
        style(RESETALL); printf("\n");
      }
    }
  }
}

/**
 * Recupera o PID do dono do token na memoria compartilhada.
 **/
int getTokenPID(){
  // declara uma key pro semaforo
  key_t semKey = TOKEN_KEY;

  // Pede ao kernel para retornar o id do semaforo
  int semid = semget(semKey, 1, IPC_CREAT | 0666);

  if (semid == -1)
    perror("semget");
  else {
    struct sembuf x = {0, -1, SEM_UNDO};
    // efetua operacao de lock no semaforo
    if (semop(semid, &x, 1) == -1)
      perror("semop");
    else {
      printf("Semaforo trancado para getToken\n");
      int *memoriaToken = (int *) getMemoria(ENDERECO_TOKEN,true);
      int valorRetorno = *(memoriaToken);
      shmdt(memoriaToken);

      semctl(semid, 0, SETVAL, 1);
      printf("Semaforo destrancado para getToken\n");
      return valorRetorno;
    }
  }
}

/**
 * Retorna um true se este processo tiver o token.
 **/
bool temToken(){
  return getTokenPID() == getpid();
}

/**
 * Funcao que recebe o sinal SIGUSR1 (sinal de mensagem).
 **/
void handlerUSR1(int signo, siginfo_t *si, void *data) {
    (void) signo;
    (void) data;
    sigval v = si->si_value;
    if(enviando_mensagem){
      enviando_mensagem = false;
      background(GREEN);
      printf("Recebi msg que enviei e nao envio mais");
      style(RESETALL); printf("\n");
      setTokenPID(pidProximo);
      sendControlSignal(pidProximo, TOKEN_PASSADO);
    }else{
      int *memoriaMsg = (int *) getMemoria(v.sival_int);
      msgRecebida.dado = *(memoriaMsg);
      shmdt(memoriaMsg);
      msgRecebida.eh_valido = true;
      // Obtem valor atual do semaforo de msg
      int valor_semaforo;
      sem_getvalue(&semaphore_msg, &valor_semaforo);
      //
      if(valor_semaforo == 0){
        sem_post(&semaphore_msg);
      }

      sendMessageSignal(v);
    }
}

/**
 * Funcao que recebe o sinal SIGUSR2 (sinal de controle).
 **/
void handlerUSR2(int signo, siginfo_t *si, void *data) {
    (void) signo;
    (void) data;
    sigval v = si->si_value;
    // para cada sinal de controle diferente, existe uma funcionalidade
    switch (v.sival_int) {
        case TOKEN_PASSADO:
            break;
        case REQ_CONEXAO:
            if(pidProximo != SEM_PROXIMO_PROCESSO) {
              background(YELLOW);
              printf("Nao estou soh seu novo prox eh %d e estou setando seu pid %d como proximo", pidProximo,
                     si->si_pid);
              style(RESETALL); printf("\n");
            }
            sendControlSignal(si->si_pid, pidProximo);
            setProxPID(si->si_pid);
            break;
        case REQ_DESCONEXAO:
            if (state == DESCONECTANDO){
              if (getpid() == getRootPID()) setRootPID(pidProximo);
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
            // Seta o proximo PID para o remetente
            background(YELLOW);
            printf("Setei como prox o pid %d", si->si_pid);
            style(RESETALL); printf("\n");
            setProxPID(si->si_pid);
            break;
        case SEM_PROXIMO_PROCESSO:
            // Seta o proximo PID para o remetente
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

/**
 * Cadastra os tratadores de sinais.
 **/
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

/**
 * Faz com que o processo se junte ao anel.
 * Eh verificado se o anel existe ou nao.
 **/
bool join() {
  bool ret = false;
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
  ret = true;
  }else{
    background(RED);
    printf("Ja estou conectado!");
    style(RESETALL); printf("\n");
  }
  return ret;
}

/**
 * Efetua a saida do processo do anel.
 **/
bool leave() {
  bool ret = false;
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
    ret = true;
  }else{
    background(WHITE);
    foreground(BLACK);
    printf("%s","Ja desconectado" );
    style(RESETALL); printf("\n");
  }
  return ret;
}

/**
 * Envia uma mensagem (inteiro) em broadcast para os outros
 * processos do anel.
 * Retorna false se a mensagem nao for enviada
 **/
bool send(int valor) {
  bool ret = false;
  if(state == CONECTADO){
    while(!temToken()){
      background(RED);
      printf("Não tenho o token!");
      style(RESETALL);
      printf("\n");
      sleep(1);
    }
    time_t t;
    srand((unsigned) time(&t));
    key_t key = rand(); /* chave aleatoria para ser paassada para o shmget() */
    int *s;
    s = (int *) getMemoria(key,true);
    *s = valor;
    shmdt(s);
    union sigval sv;
    sv.sival_int = key;
    enviando_mensagem = true;
    sendMessageSignal(sv);
    ret = true;
  }
  return ret;
}

/**
 * Le o dado de msgRecebida.
 **/
int receive() {
  if(state == CONECTADO){
    bool token = temToken();
    if(!token) sem_wait (&semaphore_msg);
    if(msgRecebida.eh_valido){
      msgRecebida.eh_valido = false;
      background(BLUE);
      printf("Recebi msg %d", msgRecebida.dado);
      style(RESETALL); printf("\n");
      return msgRecebida.dado;
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
                join();
                break;
            case 'l':
                leave();
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
