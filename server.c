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

  // Inicia a escuta por conexões
  if (listen(newSocket, MAX_CLIENTS) == -1) {
    logexit("listen");
    exit(EXIT_FAILURE);
  }

  char addrstr[BUFSZ];
  addrtostr(addr, addrstr, BUFSZ);
  printf("bound to %s, waiting connections\n", addrstr);

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
  char fileName[BUFSIZ];
  char conteudo[BUFSIZ];
  while (1) {

    memset(buf, 0, BUFSZ); // Inicia o buffer com 0 
    memset(fileName, 0, BUFSZ); // Inicia o buffer com 0 
    memset(conteudo, 0, BUFSZ); // Inicia o buffer com 0 

    recv(clientSocket, buf, BUFSZ - 1, 0);


    handleMessage(buf, fileName, conteudo);

    if (conteudo[0] != '\0') {
      // RECEBER ARQUIVO
        // Verifica se o arquivo existe
      if (fileExists(fileName) == true) {
        createFileWithMessage(conteudo, fileName);
        printf("file %s overwritten\n", fileName);
      } else {
        createFileWithMessage(conteudo, fileName);
        printf("file %s received\n", fileName);
      }
    } else {//Desconecta o cliente em casode exit ou comandos errados
      printf("connection closed\n");
      close(clientSocket);
      break;
    }
  }

  close(newSocket);
  exit(EXIT_SUCCESS);
}

