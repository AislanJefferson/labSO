#include "ipcring.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cores.h"
#include <iostream>


int main(int argc, char* argv[]) {
    bool debug = false;
    if (argc == 2 && *argv[1] == 'd'){
      debug = true;
    }
    registraHandlers(debug);
    char input = 's';
    
    while(input != 'e') {
        background(WHITE);
        foreground(BLACK);
        printf("$ ");
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