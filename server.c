#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#define FILENAMESZ 100
#define BUFSZ 500

void usage(int argc, char **argv) {
    printf("Forma de uso: %s <v4|v6> <server port>\n", argv[0]);
    printf("Exemplo: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

int extract_data(char *buf, char *filename, char *message) {
    // Find the position of the filename
    char *filename_pos = strrchr(buf, '.');
    if (filename_pos == NULL) {
        return 400;
    }

    if (filename_pos[1] == 't') { // txt or tex
        filename_pos += 4;
    } else if (filename_pos[1] == 'j') { // java
        filename_pos += 5;
    } else if (filename_pos[1] == 'p') { // py
        filename_pos += 3;
    } else if (filename_pos[1] == 'c' && filename_pos[2] == 'p') { // c or cpp
        filename_pos += 4;
    } else {
        filename_pos += 2;
    }

    // Find the position of the message start
    char *message_start = strstr(buf, filename_pos);
    if (message_start == NULL) {
        return 400;
    }

    // Find the position of the message end
    char *message_end = strstr(message_start, "\\end");
    if (message_end == NULL) {
        return 400;
    }

    // Extract the filename
    int filename_length = filename_pos - buf;
    strncpy(filename, buf, filename_length);
    filename[filename_length] = '\0';

    // Extract the message
    int message_length = message_end - message_start;
    strncpy(message, message_start, message_length);
    message[message_length] = '\0';

    return 200;
}

int write_file(char *filename, char *message) {
    // 200 OK - Overwrite
    // 201 CREATED - New file
    int return_code;

    if (access(filename, F_OK) == 0)
		return_code = 200;
	else
        return_code = 201;

    // Create or overwrite file
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        return_code = 500;
    }

    fprintf(fp, "%s", message);
    fclose(fp);

    return return_code;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
        usage(argc, argv);
    }

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logexit("socket");
    }

    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage))) {
        logexit("bind");
    }

    if (0 != listen(s, 10)) {
        logexit("listen");
    }

    while (1) {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1) {
            logexit("accept");
        }

        while (1)
        {
            char buf[BUFSZ];
            memset(buf, 0, BUFSZ);
            recv(csock, buf, BUFSZ - 1, 0);

            // Dealing with exit
            if (strcmp(buf, "exit\\end") == 0) {
                memset(buf, 0, BUFSZ);
                strcpy(buf, "connection closed\n");
                size_t count = send(csock, buf, strlen(buf) + 1, 0);
                if (count != strlen(buf) + 1) {
                    logexit("send");
                }
                break;
            }

            // Extracting filename and message
            char filename[FILENAMESZ];
            memset(filename, 0, BUFSZ);
            char message[BUFSZ];
            memset(message, 0, BUFSZ);
            
            int response_code = extract_data(buf, filename, message);

            // Preparing response to client and creating file
            memset(buf, 0, BUFSZ);
            if (response_code != 200) {
                sprintf(buf, "error receiving file %s\n", filename);
            } 

            else {
                // Writing file
                response_code = write_file(filename, message);

                // Response to Client
                memset(buf, 0, BUFSZ);
                if      (response_code == 200) {
                    sprintf(buf, "file %s overwritten\n", filename);
                } 
                else if (response_code == 201) {
                    sprintf(buf, "file %s received\n", filename);
                }
            }

            // Send response
            size_t count = send(csock, buf, strlen(buf) + 1, 0);
            if (count != strlen(buf) + 1) {
                logexit("send");
            }
        }
        
        close(csock);
    }

    exit(EXIT_SUCCESS);
}