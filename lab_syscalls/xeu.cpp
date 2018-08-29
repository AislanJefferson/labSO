#include "xeu_utils/StreamParser.h"

#include <iostream>
#include <vector>
#include <cstdio>
#include <sstream>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

using namespace xeu_utils;
using namespace std;

// This function is just to help you learn the useful methods from Command
void io_explanation(const Command& command) {
  // Let's use this input as example: (ps aux >out >o2 <in 2>er >ou)
  // we would print "$       io(): [0] >out [1] >o2 [2] <in [3] 2>er [4] >ou"
  cout << "$       io():";
  for (size_t i = 0; i < command.io().size(); i++) {
    IOFile io = command.io()[i];
    cout << " [" << i << "] " << io.repr();
    // Other methods - if we had 2>file, then:
    // io.fd() == 2
    // io.is_input() == false ('>' is output)
    // io.is_output() == true
    // io.path() == "file"
  }
}

// This function is just to help you learn the useful methods from Command
void command_explanation(const Command& command) {

  /* Methods that return strings (useful for debugging & maybe other stuff) */

  // This prints the command in a format that can be run by our xeu. If you
  // run the printed command, you will get the exact same Command
  {
    // Note: command.repr(false) doesn't show io redirection (i.e. <in >out)
    cout << "$     repr(): " << command.repr() << endl;
    cout << "$    repr(0): " << command.repr(false) << endl;
    // cout << "$   (string): " << string(command) << endl; // does the same
    // cout << "$ operator<<: " << command << endl; // does the same
  }

  // This is just args()[0] (e.g. in (ps aux), name() is "ps")
  {
    cout << "$     name(): " << command.name() << endl;
  }

  // Notice that args[0] is the command/filename
  {
    cout << "$     args():";
    for (int i = 0; i < command.args().size(); i++) {
      cout << " [" << i << "] " << command.args()[i];
    }
    cout << endl;
  }

  /* Methods that return C-string (useful in exec* syscalls) */

  // this is just the argv()[0] (same as name(), but in C-string)
  {
    printf("$ filename(): %s\n", command.filename());
  }

  // This is similar to args, but in the format required by exec* syscalls
  // After the last arg, there is always a NULL pointer (as required by exec*)
  {
    printf("$     argv():");
    for (int i = 0; command.argv()[i]; i++) {
      printf(" [%d] %s", i, command.argv()[i]);
    }
    printf("\n");
  }

  io_explanation(command);
}

// This function is just to help you learn the useful methods from Command
void commands_explanation(const vector<Command>& commands) {
  // Shows a representation (repr) of the command you input
  // cout << "$ Command::repr(0): " << Command::repr(commands, false) << endl;
  cout << "$ Command::repr(): " << Command::repr(commands) << endl << endl;

  // Shows details of each command
  for (int i = 0; i < commands.size(); i++) {
    cout << "# Command " << i << endl;
    command_explanation(commands[i]);
    cout << endl;
  }
}

int main() {
    // Waits for the user to input a command and parses it. Commands separated
    // by pipe, "|", generate multiple commands. For example, try to input
    //   ps aux | grep xeu
    // commands.size() would be 2: (ps aux) and (grep xeu)
    // If the user just presses ENTER without any command, commands.size() is 0
    //STDOUT_FILENO <- fd de saida padrao
    //STDIN_FILENO <- fd de entrada padrao
    string entrada = "";
    while (entrada != "exit") {
        cout << "% ";
        const vector<Command> commands = StreamParser().parse().commands();
        int pipefd[2];
        pipe(pipefd);

        for (int i = 0; i < commands.size(); i++) {
            Command comando = commands[i];
            if(comando.name() == "exit"){
              entrada = comando.name();
              break;
            }
            switch (int pidFilho = fork()) {
                case -1:
                    printf("Erro ao criar filho");
                    break;
                case 0:
                    //Se houver mais de um comando numa linha
                    // faca a saida padrao ser o descritor de escrita do pipe
                    // e se for o segundo comando faca a entrada padrao ser
                    // o descritor de leitura do pipe
                    if (commands.size() > 1) {
                        if (i == 0) {
                            dup2(pipefd[1], STDOUT_FILENO);
                        } else if (i > 0) {
                            dup2(pipefd[0], STDIN_FILENO);
                        }
                    }
                    close(pipefd[0]);
                    close(pipefd[1]);
                    execvp(comando.filename(), comando.argv());
                    break;
                //default:
                  // Parte executada pelo pai
                  // Comentado pois o while não encerra o processo pai
                  //int status;
                  //wait(&status);
            }
        }
        //Pai fecha pipe
        close(pipefd[0]);
        close(pipefd[1]);
        wait(NULL);
    }
    return 0;
}