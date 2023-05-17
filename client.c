#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <ctype.h>

#define BUFSZ 1024

void usage(int argc, char **argv) {
  printf("usage: %s <server IP> <server port>\n", argv[0]);
  printf("example: %s 127.0.0.1 51511\n", argv[0]);
  exit(EXIT_FAILURE);
}

void stringToUpper(char *str) {
  int i = 0;
  while (str[i] != '\0') {
    str[i] = toupper(str[i]);
    i++;
  }
}


int main(int argc, char **argv) {
  if (argc < 3) { // Verifica ip correto
    usage(argc, argv);
  }

  struct sockaddr_storage storage; // Parseia qual o endereço do servidor, esse ponteiro vai armazenar
  if (addrparse(argv[1], argv[2], &storage) != 0) {
    usage(argc, argv);
  }

  int newSocket = socket(storage.ss_family, SOCK_STREAM, 0); // Cria Socket, ss_family, é a família do IP
  if (newSocket == -1) { // Verifica se deu erro de Socket
    logexit("socket");
  }
  struct sockaddr *addr = (struct sockaddr *)(&storage); // Conecta
  if (connect(newSocket, addr, sizeof(storage)) != 0) { // Verifica se deu erro de conexão
    logexit("connect");
  }


  char addrstr[BUFSZ];
  addrtostr(addr, addrstr, BUFSZ); // Converte pra printar o endereço

  printf("connected to %s\n", addrstr);

  char userBuf[BUFSIZ];
  char bufferToSlice[BUFSIZ];
  char filebuffer[BUFSIZ];
  char buf[BUFSZ];       // Buffer para armazenar os dados
  memset(buf, 0, BUFSZ); // Inicia Buffer com 0
  size_t count;

  while (1) {
    printf(">> ");
    fgets(userBuf, BUFSZ - 1, stdin);


    // Removendo o caractere de nova linha do final da string e colocando o \end
    strcpy(bufferToSlice, userBuf);
    size_t length = strlen(bufferToSlice);
    if (length > 0 && bufferToSlice[length - 1] == '\n') {
      // userBuf[length - 1] = END_CHAR;
      bufferToSlice[length - 1] = '\0';
    }

    char *command;
    char *filename;
    command = strtok(bufferToSlice, " ");
    filename = strtok(NULL, " ");

    stringToUpper(command);


    //SELECIONAR ARQUIVO
    if (command != NULL && (strcmp(command, "SELECT") == 0)) {
      // Lê o arquivo e envia ao servidor

      printf("Comando: %s\n", command);
      printf("Nome do arquivo: %s\n", filename);

      FILE *file = fopen(filename, "r");
      if (file == NULL) {
        printf("%s does not exist", filename);
      } else {
        //Lê o arquivo e guarda no buf para depois enviar ao servidor
        int n;

        while ((n = fread(filebuffer, sizeof(char), BUFSZ, file)) > 0) {
          if (write(newSocket, filebuffer, n) < 0) {
            logexit("Error writing to socket");
          }
        }
        printf("userBuf:  %s ", userBuf);

        count = send(newSocket, userBuf, strlen(userBuf) + 1, 0); // Avisa o servidor que um arquivo foi selecionado

        printf("%s selected", filename);
      }

    }



    //ENVIAR ARQUIVO
    if (command != NULL && (strcmp(command, "SEND") == 0)) {
      printf("Comando: %s\n", command);
      printf("Nome do arquivo: %s\n", filename);

      count = send(newSocket, filebuffer, strlen(filebuffer) + 1, 0); // Envia dados para o servidor, o terceiro paramero é o tamanho enviado, o /0 é o +1

      if (count != strlen(filebuffer) + 1) { // Erro no envio
        logexit("send");
      }

    }



    // memset(buf, 0, BUFSZ);
    // unsigned total = 0; // Total de bits recebido até o momento
    // while (1) {
    //   count = recv(newSocket, buf + total, BUFSZ - total, 0); // Recebendo dado do servidor
    //   if (count == 0) {
    //     // Connection terminated.
    //     break;
    //   }
    //   total += count; // Fica recebendo o dado até o dado ser igual a 0
    // }

    //SAIR
    if (command != NULL && (strcmp(command, "EXIT") == 0)) {

      close(newSocket);

    }



    // printf("received %u bytes\n", total); // Printa a quantidade de bits e a mensagem
    puts(buf);

  }
  exit(EXIT_SUCCESS);
}