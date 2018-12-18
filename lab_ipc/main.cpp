#include "ipcring.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cores.h"
#include <iostream>
#include <sys/types.h>
#include <unistd.h>


int main(int argc, char* argv[]) {
    printf("-----------------------\n");
    printf("        Legenda\n");
    printf("join: j \n");
    printf("leave: l \n");
    printf("send: s \n");
    printf("receive: r \n");
    printf("-----------------------\n\n");
    bool debug = false;
    if (argc == 2 && *argv[1] == 'd'){
      debug = true;
    }
    registraHandlers(debug);
    char input = 's';
    
    while(input != 'e') {
        printf("%d $ ", getpid());
        input = getchar();
        switch (input) {
            case 'j':
                join();
                break;
            case 'l':
                leave();
                break;
            case 'r':
                printf("%d\n",receive());
                break;
            case 's':
                int msg;
                scanf("%d",&msg);
                send(msg);

                break;
        }
    }
    return 0;
}