#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void usage(int argc, char **argv) {
  printf("usage: %s <server IP> <server port>\n", argv[0]);
  printf("example: %s 127.0.0.1 51511\n", argv[0]);
  exit(EXIT_FAILURE);
}

#define BUFSZ 1024

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

  char buf[BUFSZ];       // Buffer para armazenar os dados
  memset(buf, 0, BUFSZ); // Inicia Buffer com 0
  printf("mensagem> ");
  fgets(buf, BUFSZ - 1, stdin);

  // ---------------------------------------------------------------------------------------------------------------------------
  if (sizeof(buf) <= 127 && sizeof(buf) >= 127) {
    printf("Sua mensagem possui caracteres invalidos, redigite por favor");
    printf("mensagem> ");
    fgets(buf, BUFSZ - 1, stdin);
  }
  // ---------------------------------------------------------------------------------------------------------------------------

  size_t count = send(newSocket, buf, strlen(buf) + 1, 0); // Envia dados para o servidor, o terceiro paramero é o tamanho enviado, o /0 é o +1
  if (count != strlen(buf) + 1) { // Erro no envio
    logexit("send");
  }

  memset(buf, 0, BUFSZ);
  unsigned total = 0; // Total de bits recebido até o momento
  while (1) {
    count = recv(newSocket, buf + total, BUFSZ - total, 0); // Recebendo dado do servidor
    if (count == 0) {
      // Connection terminated.
      break;
    }
    total += count; // Fica recebendo o dado até o dado ser igual a 0
  }
  close(newSocket);

  printf("received %u bytes\n", total); // Printa a quantidade de bits e a mensagem
  puts(buf);

  exit(EXIT_SUCCESS);
}

// if((int)char >= int ACSII && (int)char <= int ACSII)