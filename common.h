#pragma once

#include <stdlib.h>

#include <arpa/inet.h>

#include <stdbool.h> 

#define END_CHAR '\x04' // Caractere especial '\end' - EOT (End of Transmission)

//COMANDOS
#define SELECT "SELECT"
#define SEND "SEND"
#define EXIT "EXIT"




void logexit(const char *msg);

int addrparse(const char *addrstr, const char *portstr, struct sockaddr_storage *storage);

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize);

int server_sockaddr_init(const char *proto, const char *portstr, struct sockaddr_storage *storage);

void changeLastCharacter(char *buffer, char oldCharacter, char newCharacter);

void removeNullChar(char *str);

bool containsWord(const char *str, const char *word);

void formatMessage(char *command, char *message, char *result);

void extractCommandAndMessage(const char *input, char *command, char *message);

void createFileWithMessage(const char *message, const char *filename);

void removeNullCharacters(char *string);

bool fileExists(const char *filename);

int isFileTypeValid(const char *filename);

void removeNewLine(char *str);

void getFileName(const char* str, char* filename);

void separarString(const char *string, char *fileName, char *conteudo);