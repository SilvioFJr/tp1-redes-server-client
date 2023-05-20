#include <inttypes.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <arpa/inet.h>

#define END_CHAR '\x04' // Caractere especial '\end' - EOT (End of Transmission)


// #define PORT 5151

void logexit(const char *msg) { // Apresenta o erro e sai do programa
    perror(msg);
    exit(EXIT_FAILURE);
}

int addrparse(const char *addrstr, const char *portstr, struct sockaddr_storage *storage) { // Função que passa o ponteiro pra struck
    if (addrstr == NULL || portstr == NULL) { // Se receber errado
        return -1;
    }


    uint16_t port = (uint16_t)atoi(portstr);
    // int port = PORT;
    if (port == 0) {
        return -1;
    }
    port = htons(port); // host to network short

    struct in_addr inaddr4; // 32-bit IPv4
    if (inet_pton(AF_INET, addrstr, &inaddr4)) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr = inaddr4;
        return 0;
    }

    struct in6_addr inaddr6; // 128-bit IPv6
    if (inet_pton(AF_INET6, addrstr, &inaddr6)) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = port;
        memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6));
        return 0;
    }

    return -1;
}

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize) {
    int version;
    char addrstr[INET6_ADDRSTRLEN + 1] = "";
    uint16_t port;

    if (addr->sa_family == AF_INET) {
        version = 4;
        struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;
        if (!inet_ntop(AF_INET, &(addr4->sin_addr), addrstr,
            INET6_ADDRSTRLEN + 1)) {
            logexit("ntop");
        }
        port = ntohs(addr4->sin_port); // network to host short
    } else if (addr->sa_family == AF_INET6) {
        version = 6;
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;
        if (!inet_ntop(AF_INET6, &(addr6->sin6_addr), addrstr,
            INET6_ADDRSTRLEN + 1)) {
            logexit("ntop");
        }
        port = ntohs(addr6->sin6_port); // network to host short
    } else {
        logexit("unknown protocol family.");
    }
    if (str) {
        snprintf(str, strsize, "IPv%d %s %hu", version, addrstr, port);
    }
}

int server_sockaddr_init(const char *proto, const char *portstr, struct sockaddr_storage *storage) {
    uint16_t port = (uint16_t)atoi(portstr); // unsigned short
    if (port == 0) {
        return -1;
    }
    port = htons(port); // host to network short

    // inicializa com 0
    memset(storage, 0, sizeof(*storage));

    if (strcmp(proto, "v4") == 0) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr.s_addr = INADDR_ANY;
        return 0;
    } else if (strcmp(proto, "v6") == 0) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = port;
        addr6->sin6_addr = in6addr_any;
        return 0;
    } else {
        return -1;
    }
}




void changeLastCharacter(char buffer[], char oldCharacter, char newCharacter){
    size_t length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == oldCharacter) {
      buffer[length - 1] = newCharacter;
    }
}


void removeNullChar(char *str) {
    char newStr[strlen(str)];
    int i, j = 0;

    for (i = 0; i < strlen(str); i++) {
        if (str[i] != END_CHAR) {
            newStr[j++] = str[i];
        }
    }

    newStr[j] = END_CHAR;

    strcpy(str, newStr);
}

bool containsWord(const char *str, const char *word) {
    const char *ptr = strstr(str, word);
    return (ptr != NULL);
}

void formatMessage(char *command, char *message, char *result) {
    sprintf(result, "[%s]**[%s]", command, message);
}


// void extractCommandAndMessage(const char *input, char *command, char *message, size_t commandSize, size_t messageSize) {
//     const char *delimiter = "**";
//     const char *start = strchr(input, '[');
//     const char *end = strstr(input, delimiter);

//     if (start && end && start < end) {
//         start++; // Pula o caractere '['
//         size_t commandLength = end - start - 1;
//         if (commandLength < commandSize) {
//             strncpy(command, start, commandLength);
//             command[commandLength] = '\0';
//         }

//         const char *messageStart = end + strlen(delimiter);
//         const char *messageEnd = strchr(messageStart, ']');
//         if (messageEnd && messageEnd < input + strlen(input)) {
//             size_t messageLength = messageEnd - messageStart;
//             if (messageLength < messageSize) {
//                 strncpy(message, messageStart, messageLength);
//                 message[messageLength] = '\0';
//             }
//         }
//     } else {
//         command[0] = '\0';
//         message[0] = '\0';
//     }
// }


// void extractCommandAndMessage(const char *input, char *command, char *message, size_t commandSize, size_t messageSize) {
//     const char *commandStart = strchr(input, '[');
//     const char *commandEnd = strstr(input, "]*");
//     const char *messageStart = strstr(input, "*[");
//     const char *messageEnd = strrchr(input, ']');

//     if (commandStart && commandEnd && messageStart && messageEnd && commandStart < commandEnd && messageStart < messageEnd) {
//         commandStart++; // Pula o caractere '['
//         size_t commandLength = commandEnd - commandStart;
//         if (commandLength < commandSize) {
//             strncpy(command, commandStart, commandLength);
//             command[commandLength] = '\0';
//         }

//         messageStart += 2; // Pula o delimitador "**["
//         size_t messageLength = messageEnd - messageStart;
//         if (messageLength < messageSize) {
//             strncpy(message, messageStart, messageLength);
//             message[messageLength] = '\0';
//         }
//     } else {
//         command[0] = '\0';
//         message[0] = '\0';
//     }
// }


void extractCommandAndMessage(const char *input, char *command, char *message) {
    const char *commandStart = strchr(input, '[');
    const char *commandEnd = strstr(input, "]*");
    const char *messageStart = strstr(input, "*[");
    const char *messageEnd = strrchr(input, ']');

    if (commandStart && commandEnd && messageStart && messageEnd && commandStart < commandEnd && messageStart < messageEnd) {
        commandStart++; // Pula o caractere '['
        size_t commandLength = commandEnd - commandStart;
        strncpy(command, commandStart, commandLength);
        command[commandLength] = '\0';

        messageStart += 2; // Pula o delimitador "**["
        size_t messageLength = messageEnd - messageStart;
        strncpy(message, messageStart, messageLength);
        message[messageLength] = '\0';
    } else {
        command[0] = '\0';
        message[0] = '\0';
    }
}




void createFileWithMessage(const char *message, const char *filename) {
    // Abre o arquivo no modo de escrita
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return;
    }

    
    // Escreve a mensagem no arquivo
    fprintf(file, "%s", message);

    // Fecha o arquivo
    fclose(file);

    printf("Arquivo criado com sucesso: %s\n", filename);
}

void removeNullCharacters(char *string) {
    int i, j;
    int len = strlen(string);

    for (i = 0, j = 0; i < len; i++) {
        if (string[i] != '\0') {
            string[j] = string[i];
            j++;
        }
    }

    string[j] = '\0';
}

bool fileExists(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file != NULL) {
        fclose(file);
        return true;
    } else {
        return false;
    }
}





