#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define FILENAMESZ 100
#define BUFSZ 1024

// Prints the usage of the program if the arguments are invalid
void usage(int argc, char **argv) {
	printf("Modo de Uso: %s <server IP> <server port>\n", argv[0]);
	printf("Exemplo: %s 127.0.0.1 51511\n", argv[0]);
	exit(EXIT_FAILURE);
}

// Extracts the filename and the message from the buffer
// Returns: [200] if the message is valid
// 			[400] if the message is invalid
// 			[404] if the file does not exist
int select_file(char *filename) {
	// Checks if file exists
	if (filename[0] == '\0')
		return 404;

	// Checks if file is valid
	int len = strlen(filename);
	if (len < 2)
		return 400; // Too short name

	char* extension = NULL;
	char* dotPosition = strrchr(filename, '.');

    if (dotPosition != NULL && len - (dotPosition - filename) >= 1) {
        extension = dotPosition;
    }

	if (extension == NULL)
		return 400;

	if (	   strcmp(extension, ".txt")  == 0
            || strcmp(extension, ".c") 	  == 0
			|| strcmp(extension, ".cpp")  == 0
            || strcmp(extension, ".py")   == 0
            || strcmp(extension, ".tex")  == 0
            || strcmp(extension, ".java") == 0
		) {
			if (access(filename, F_OK) != 0)
				return 404;
			return 200;
		}

	return 400;
}

// Wrap the message with the filename and the end tag
// Example: "filename.txtThis is the message\\end"
void wrap_message(char *filename, char* buf) {
	char aux[BUFSZ];
	memset(aux, 0, BUFSZ);
	strcat(aux, filename);
	strcat(aux, buf);
	strcat(aux, "\\end");

	strcpy(buf, aux);
}

// Sends the file to the server
// Returns: [200] if the file was sent successfully
// 			[404] if the file wasn't selected
int send_file(int s, char *filename) {
	if (filename[0] == '\0')
		return 404;

	// Setup
	char buf[BUFSZ];
	memset(buf, 0, BUFSZ);

	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
        return 404;
    }

	// Save file's content to buf
	fread(buf, 1, BUFSZ - 1, fp);
    fclose(fp);

	wrap_message(filename, buf);

	// Send buf to server
	size_t count = send(s, buf, strlen(buf)+1, 0);
	if (count != strlen(buf)+1) {
		logexit("send");
	}

	return 200;
}


int main(int argc, char **argv) {
	if (argc < 3) {
		usage(argc, argv);
	}

	struct sockaddr_storage storage;
	if (0 != addrparse(argv[1], argv[2], &storage)) {
		usage(argc, argv);
	}

	int s;
	s = socket(storage.ss_family, SOCK_STREAM, 0);
	if (s == -1) {
		logexit("socket");
	}
	struct sockaddr *addr = (struct sockaddr *)(&storage);
	if (0 != connect(s, addr, sizeof(storage))) {
		logexit("connect");
	}

	char buf[BUFSZ];
	char filename[FILENAMESZ];
	memset(filename, 0, FILENAMESZ);

	while(1) {
		// Input
		memset(buf, 0, BUFSZ);
		fgets(buf, BUFSZ-1, stdin);
		buf[strcspn(buf, "\n")] = '\0';

		// Input Action -- select file.
		if (strncmp(buf, "select file", 11) == 0) {
			char new_filename[FILENAMESZ];
			memset(new_filename, 0, FILENAMESZ);
			strcpy(new_filename, buf + 12);

			// Checks if file is valid
			int result = select_file(new_filename);

			switch (result)
			{
			case 404:
				printf("%s does not exist!\n", new_filename);
				break;
			case 400:
				printf("%s not valid!\n", new_filename);
				break;
			default:
				printf("%s selected!\n", new_filename);
				strcpy(filename, new_filename);
				break;
			}
			continue;
		}
		// Input Action -- send selected file.
		else if (strncmp(buf, "send file", 9) == 0) {
			int result = send_file(s, filename);
			if (result == 404) {
				printf("no file selected!\n");
				continue;
			}
		}
		// Input Action -- terminate session.
		else if (strncmp(buf, "exit", 4) == 0) {
			break;
		}

		memset(buf, 0, BUFSZ);
		recv(s, buf, BUFSZ - 1, 0);
		printf("%s", buf);

		if (strcmp(buf, "connection closed\n") == 0)
			break;
	}

	close(s);

	exit(EXIT_SUCCESS);
}