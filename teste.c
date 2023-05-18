// Program that opens testando.txt, saves it's content into a char buf[1024] and the prints it
#include <stdio.h>
#include <string.h>
#include <unistd.h>


int main() {
    char buf[1024];
    memset(buf, 0, 1024);

    FILE *fp = fopen("testando.txt", "r");
    if (fp == NULL) {
        return 404;
    }

    fread(buf, 1, 1024 - 1, fp);
    fclose(fp);

    printf("%s\n", buf);
}

// To run it:
// gcc teste.c -o teste