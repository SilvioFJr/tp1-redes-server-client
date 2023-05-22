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

//COMANDOS
#define SELECT "SELECT"
#define SEND "SEND"
#define EXIT "EXIT"

void usage(char **argv) {
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
    usage(argv);
  }

  struct sockaddr_storage storage; // Parseia qual o endereço do servidor, esse ponteiro vai armazenar
  if (addrparse(argv[1], argv[2], &storage) != 0) {
    usage(argv);
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

    //Remove o caracter '\n' e coloca o '\0' no fim da string
    changeLastCharacter(bufferToSlice, '\n', '\0');

    char *command = userBuf;
    char *filename;
    char message[BUFSIZ];

    command = strtok(bufferToSlice, " ");
    filename = strtok(NULL, " ");

    formatMessage(command, filename, message);

    stringToUpper(command);

    //SAIR
    if (containsWord(command, EXIT)) {
      // Envia pedido de desconexão para o servidor
      changeLastCharacter(bufferToSlice, '\0', END_CHAR);

      count = send(newSocket, message, strlen(message) + 1, 0);

      if (count != strlen(message) + 1) { // Erro no envio
        logexit("send");
      }

      close(newSocket);
      memset(message, 0, BUFSZ); // Inicia Buffer com 0
      break;
    }

    //SELECIONAR ARQUIVO
    if (containsWord(command, SELECT)) {

      if (isFileTypeValid(filename)) {
        FILE *file = fopen(filename, "r");
        if (file != NULL) {
          //Lê o arquivo, guarda os dados no buffer
          fread(filebuffer, sizeof(char), BUFSZ, file);
          fclose(file);

          // Avisa o servidor que um arquivo foi selecionado
          //Remove o '\n' e coloca o 'EOT'
          changeLastCharacter(filebuffer, '\0', END_CHAR);
          count = send(newSocket, message, strlen(message) + 1, 0);

          if (count != strlen(message) + 1) { // Erro no envio
            logexit("send");
          }
          printf("%s selected\n", filename);

        } else {
          if (filename == NULL) {
            printf("file name not provided\n");
          } else {
            printf("%s does not exist\n", filename);
          }
        }
      } else {
        printf("%s not valid!\n", filename);
      }

      memset(message, 0, BUFSZ);
    }



    //ENVIAR ARQUIVO
    if (containsWord(command, SEND)) {
      printf("Comando: %s\n", command);

      if (filebuffer != NULL) {

        formatMessage(command, filebuffer, message);
        changeLastCharacter(message, '\0', END_CHAR);

        count = send(newSocket, message, strlen(message) + 1, 0); // Envia dados para o servidor

        if (count != strlen(message) + 1) { // Erro no envio
          logexit("send");
        }
        printf("file %s received\n", filename);
      } else {
        printf("no file selected!\n");
      }


      memset(message, 0, BUFSZ);
    }

  }

  exit(EXIT_SUCCESS);

}