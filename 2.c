#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_PROCESSES 8  // Максимальное количество процессов

void copy_file(const char *src, const char *dst) {
    int in_fd = open(src, O_RDONLY);
    if (in_fd == -1) {
        perror("Failed to open source file");
        exit(1);
    }

    int out_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (out_fd == -1) {
        perror("Failed to open destination file");
        close(in_fd);
        exit(1);
    }

    char buffer[4096];
    ssize_t bytes_read, total_bytes = 0;
    
    while ((bytes_read = read(in_fd, buffer, sizeof(buffer))) > 0) {
        write(out_fd, buffer, bytes_read);
        total_bytes += bytes_read;
    }

    struct stat st;
    if (stat(src, &st) == 0) {
        chmod(dst, st.st_mode);
    }

    printf("PID %d copied %s -> %s (%ld bytes)\n", getpid(), src, dst, total_bytes);

    close(in_fd);
    close(out_fd);
    exit(0);
}

void sync_directories(const char *dir1, const char *dir2, int max_processes) {
    DIR *d1 = opendir(dir1);
    if (!d1) {
        perror("Failed to open source directory");
        exit(1);
    }

    struct dirent *entry;
    int process_count = 0;

    while ((entry = readdir(d1)) != NULL) {
        if (entry->d_type == DT_REG) {  // Файл (не каталог)
            char src_path[512], dst_path[512];
            snprintf(src_path, sizeof(src_path), "%s/%s", dir1, entry->d_name);
            snprintf(dst_path, sizeof(dst_path), "%s/%s", dir2, entry->d_name);

            struct stat st;
            if (stat(dst_path, &st) == -1) {  // Файл отсутствует в Dir2
                pid_t pid = fork();
                if (pid == 0) {
                    copy_file(src_path, dst_path);
                } else if (pid > 0) {
                    process_count++;
                    if (process_count >= max_processes) {
                        wait(NULL);
                        process_count--;
                    }
                } else {
                    perror("Fork failed");
                }
            }
        }
    }

    // Дождаться завершения всех процессов
    while (wait(NULL) > 0);

    closedir(d1);
}

int main() {
    char dir1[256], dir2[256];
    int max_processes;

    printf("Enter source directory: ");
    scanf("%255s", dir1);
    printf("Enter destination directory: ");
    scanf("%255s", dir2);
    printf("Enter max concurrent processes: ");
    scanf("%d", &max_processes);

    if (max_processes <= 0 || max_processes > MAX_PROCESSES) {
        printf("Invalid number of processes. Using default: %d\n", MAX_PROCESSES);
        max_processes = MAX_PROCESSES;
    }

    sync_directories(dir1, dir2, max_processes);

    return 0;
}
