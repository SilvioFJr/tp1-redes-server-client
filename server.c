#include "common.h"

#include <pthread.h> // Criação das threads
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <ctype.h>

// Tamanho do buffer
#define BUFSZ 9000

// Tamanho da fila de conexões pendentes
#define maxQueueLimit 128


//COMANDOS
#define SELECT "SELECT"
#define SEND "SEND"
#define EXIT "EXIT"

//número máximo de clientes
#define MAX_CLIENTS 1

void usage(char **argv) {
  printf("usage: %s <v4|v6> <server port>\n", argv[0]);
  printf("example: %s v4 51511\n", argv[0]);
  exit(EXIT_FAILURE);
}
void stringToUpper(char *str) {
  int i = 0;
  while (str[i] != '\0') {
    str[i] = toupper(str[i]);
    i++;
  }
}

struct client_data {
  int clientSocket;
  struct sockaddr_storage storage;
};

int main(int argc, char **argv) {

  if (argc < 3) {
    usage(argv);
  }

  struct sockaddr_storage storage;

  if (server_sockaddr_init(argv[1], argv[2], &storage) != 0) {
    usage(argv);
  }

  int newSocket = socket(storage.ss_family, SOCK_STREAM, 0);
  if (newSocket == -1) {
    logexit("socket");
  }

  // Reutiliza o endereço da porta, mesmo se ele já estiver sendo utilizado...
  int enable = 1;
  if (setsockopt(newSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) != 0) {
    logexit("setsockopt");
  }

  struct sockaddr *addr = (struct sockaddr *)(&storage);

  if (bind(newSocket, addr, sizeof(storage)) != 0) {
    logexit("bind");
  }

  // if (listen(newSocket, maxQueueLimit) != 0) {
  // }

  // Inicia a escuta por conexões
  if (listen(newSocket, MAX_CLIENTS) == -1) {
    logexit("listen");
    exit(EXIT_FAILURE);
  }

  char addrstr[BUFSZ];
  addrtostr(addr, addrstr, BUFSZ);
  printf("bound to %s, waiting connections\n", addrstr);

  while (1) {
    struct sockaddr_storage clientStorage;
    struct sockaddr *caddr = (struct sockaddr *)(&clientStorage);
    socklen_t caddrlen = sizeof(clientStorage);

    int clientSocket = accept(newSocket, caddr, &caddrlen); // socket do cliente
    if (clientSocket == -1) {
      logexit("accept");
    }

    char caddrstr[BUFSZ];
    addrtostr(caddr, caddrstr, BUFSZ);
    printf("[log]: connection from %s\n", caddrstr);

    char buf[BUFSZ];
    char filename[BUFSIZ];
    memset(buf, 0, BUFSZ); // Inicia o buffer com 0 

    while (1) {
      char command[BUFSIZ];
      char message[BUFSIZ];

      int bytesReceived = recv(clientSocket, buf, BUFSZ - 1, 0);

      if (bytesReceived > 0) {

        //teste end char
        // if (containsWord(buf, END_CHAR)) {
        //   printf("sim END_CHAR\n");
        //   // close(clientSocket);
        //   // break;
        // } else if(containsWord(buf, "\0")){
        //   printf("sim '\0'\n");
        // }else{
        //   printf("não\n");
        // }

        //Trata a mensagem recebida
        removeNullChar(buf);
        extractCommandAndMessage(buf, command, message);


        stringToUpper(command);

        //Desconectar cliente
        if (containsWord(command, EXIT)) {
          printf("connection closed\n");
          close(clientSocket);
          break;
        }
      }

      //guardar nome do arquivo......
      if (containsWord(command, SELECT)) {
        if (filename != NULL) {
          strcpy(filename, message);
          removeNewLine(filename);
          memset(message, 0, BUFSZ);
        }
      }

      // RECEBER ARQUIVO

      // Verifica se o arquivo existe
      if (containsWord(command, SEND)) {
        char *fname = filename;
        char *MENSAGEM = message;

        if (fileExists(fname) == true) {
          createFileWithMessage(MENSAGEM, filename);
          printf("file %s overwritten\n", filename);
        } else {
          createFileWithMessage(MENSAGEM, filename);
          printf("file %s received\n", filename);
        }

        memset(message, 0, BUFSZ);
      }

      memset(buf, 0, BUFSZ);
    }
  }

  close(newSocket);
  exit(EXIT_SUCCESS);
}

