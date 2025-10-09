#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <pid>\n", argv[0]);
        return 1;
    }

    pid_t pid = atoi(argv[1]);
    pid_t pgid = getpgid(pid);

    if (pgid == -1) {
        perror("getpgid");
        return 1;
    }

    printf("PID: %d\n", pid);
    printf("PGID: %d\n", pgid);
    printf("session id: %d\n", getsid(pid));
    return 0;
}

