#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSZ 1024

void usage(int argc, char **argv) {
	printf("usage: %s <server IP> <server port>\n", argv[0]);
	printf("example: %s 127.0.0.1 51511\n", argv[0]);
	exit(EXIT_FAILURE);
}

int select_file(char *filename) {
	// Checks if file exists
	if (filename == NULL)
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

int send_file(int s, char *filename) {
	if (filename == NULL)
		return 404;

	// Setup
	char buf[BUFSZ];
	memset(buf, 0, BUFSZ);
	FILE *fp = fopen(filename, "r");

	// Save file's content to buf
	while (fgets(buf, BUFSZ-1, fp) != NULL) {
		buf[strcspn(buf, "\n")] = '\0';
	}

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

	char addrstr[BUFSZ];
	addrtostr(addr, addrstr, BUFSZ);

	printf("connected to %s\n", addrstr);



	char buf[BUFSZ];
	memset(buf, 0, BUFSZ);
	char *filename = NULL;
	unsigned total;
	size_t count;
	while(1) {
		
		// Input
		memset(buf, 0, BUFSZ);
		fgets(buf, BUFSZ-1, stdin);
		buf[strcspn(buf, "\n")] = '\0';

		// Input Action -- select file.
		if (strncmp(buf, "select file", 11) == 0) {
			filename = buf + 12;
			int result = select_file(filename);
			switch (result)
			{
			case 404:
				printf("%s does not exist\n", filename);
				filename = NULL;
				break;
			case 400:
				printf("%s not valid\n", filename);
				filename = NULL;
				break;
			default:
				printf("%s selected\n", filename);
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
		total = 0;
		while (1) {
			count = recv(s, buf + total, BUFSZ - total, 0);
			if (count == 0) {
				// Connection terminated.
				break;
			}
			total += count;
		}
	}
	close(s);

	exit(EXIT_SUCCESS);
}