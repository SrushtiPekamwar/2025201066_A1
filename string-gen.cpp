#define _FILE_OFFSET_BITS 64
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>

#define bufferSize 100000

char chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

void genRandomBuff(char buff[], int size) {
    int len = strlen(chars);
    for (int i = 0; i < size; i++) {
        buff[i] = chars[rand() % len];
    }
}

int main(int argc, char* argv[]) {
    char fname[256] = "input.txt";
    long long fsize = 1000000LL; // default

    if (argc >= 2) strcpy(fname, argv[1]);
    if (argc >= 3) fsize = strtoll(argv[2], NULL, 10);

    srand(time(NULL));

    int ofd = open(fname, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (ofd == -1) {
        perror("Can't create file");
        exit(1);
    }

    char* buff = (char*)malloc(bufferSize);
    if (!buff) {
        perror("malloc failed");
        close(ofd);
        exit(1);
    }

    struct timeval start, curr;
    gettimeofday(&start, NULL);

    long long written = 0;
    long long chunks = fsize / bufferSize;
    int rem = fsize % bufferSize;

    printf("Generating file %s with %lld chars...\n", fname, fsize);

    for (long long i = 0; i < chunks; i++) {
        genRandomBuff(buff, bufferSize);
        if (write(ofd, buff, bufferSize) != bufferSize) {
            perror("write failed");
            break;
        }
        written += bufferSize;

        if (i % 100 == 0) { // update progress more frequently
            gettimeofday(&curr, NULL);
            double elapsed = (curr.tv_sec - start.tv_sec) +
                             (curr.tv_usec - start.tv_usec) / 1000000.0;
            double percent = ((double)written / fsize) * 100;
            double speed = (written / 1024.0 / 1024.0) / elapsed;

            int bars = (int)(percent / 2);
            printf("\r[");
            for (int j = 0; j < 50; j++) {
                printf(j < bars ? "=" : " ");
            }
            printf("] %.1f%% %.1fMB/s", percent, speed);
            fflush(stdout);
        }
    }

    if (rem > 0) {
        genRandomBuff(buff, rem);
        write(ofd, buff, rem);
        written += rem;
    }

    gettimeofday(&curr, NULL);
    double total_time = (curr.tv_sec - start.tv_sec) +
                        (curr.tv_usec - start.tv_usec) / 1000000.0;

    printf("\nDone! Wrote %lld chars in %.2f seconds\n", written, total_time);

    free(buff);
    close(ofd);
    return 0;
}
