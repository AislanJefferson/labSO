//
// Created by Aislan on 19/11/2018.
//

#ifndef UNTITLED_CONSTANTES_H
#define UNTITLED_CONSTANTES_H
#define    TOKEN_PASSADO -1  /* Notificacao de pertencimento do token */
#define    REQ_CONEXAO -2  /* interrupt */
#define    REQ_DESCONEXAO    -3  /* quit */
#define    SEM_PROXIMO_PROCESSO -10  /* illegal instruction (not reset when caught) */
#define    NODE_CONNECT_REQ    -5  /* USADO PARA REQUISITAR QUE O NO CONECTE-SE AO REQUISITANTE*/
#define    PID_INEXISTENTE    -6

// Estados
#define    CONECTANDO    -19 /* continue a stopped process */
#define    CONECTADO    -20 /* to parent on child stop or exit */
#define    DESCONECTANDO -21 /* System V name for SIGCHLD */
#define    DESCONECTADO    -22 /* to readers pgrp upon background tty read */

#define ENDERECO_TOKEN -500

#endif //UNTITLED_CONSTANTES_H
