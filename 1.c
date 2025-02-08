#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>

void print_time_and_pid(const char *process_name) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    struct tm *tm_info = localtime(&tv.tv_sec);
    
    printf("[%s] PID: %d, PPID: %d, Time: %02d:%02d:%02d:%03ld\n",
           process_name, getpid(), getppid(),
           tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, tv.tv_usec / 1000);
}

int main() {
    pid_t pid1, pid2;

    pid1 = fork();
    if (pid1 == 0) {
        print_time_and_pid("Child 1");
        exit(0);
    }

    pid2 = fork();
    if (pid2 == 0) {
        print_time_and_pid("Child 2");
        exit(0);
    }

    // Ожидание завершения дочерних процессов
    wait(NULL);
    wait(NULL);

    print_time_and_pid("Parent");

    // Выполнение команды `ps -x`
    system("ps -x");

    return 0;
}
