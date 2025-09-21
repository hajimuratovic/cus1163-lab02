#include "proc_reader.h"

int list_process_directories(void) {
    DIR *dir = opendir("/proc");
    struct dirent *entry;
    int count = 0;

    if (dir == NULL) {
        printf("could not open /proc\n");
        return -1;
    }

    printf("Process directories in /proc:\n");
    printf("%-8s %-20s\n", "PID", "Type");
    printf("%-8s %-20s\n", "---", "----");

    while ((entry = readdir(dir)) != NULL) {
        if (is_number(entry->d_name)) {
            printf("%-8s %-20s\n", entry->d_name, "process");
            count++;
        }
    }

    closedir(dir);
    printf("Found %d process directories\n", count);
    printf("SUCCESS: Process directories listed!\n");
    return 0;
}

int read_process_info(const char* pid) {
    char filepath[256];

    printf("\n--- Process Information for PID %s ---\n", pid);
    snprintf(filepath, sizeof(filepath), "/proc/%s/status", pid);
    read_file_with_syscalls(filepath);

    printf("\n\n--- Command Line ---\n");
    snprintf(filepath, sizeof(filepath), "/proc/%s/cmdline", pid);

    {
        int fd = open(filepath, O_RDONLY);
        char buffer[1024];
        ssize_t bytes_read;
        int i;

        if (fd < 0) {
            printf("could not open %s\n", filepath);
            return -1;
        }

        bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        if (bytes_read < 0) {
            printf("could not read %s\n", filepath);
            close(fd);
            return -1;
        }
        buffer[bytes_read] = '\0';
        for (i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\0') buffer[i] = ' ';
        }
        if (bytes_read == 0) printf("(empty)\n");
        else printf("%s\n", buffer);

        close(fd);
    }

    printf("SUCCESS: Process information read!\n\n");
    return 0;
}

int show_system_info(void) {
    const int MAX_LINES = 10;
    int line_count;
    FILE *file;
    char line[256];

    printf("\n--- CPU Information (first %d lines) ---\n", MAX_LINES);
    file = fopen("/proc/cpuinfo", "r");
    if (!file) {
        printf("could not open /proc/cpuinfo\n");
        return -1;
    }
    line_count = 0;
    while (line_count < MAX_LINES && fgets(line, sizeof(line), file) != NULL) {
        printf("%s", line);
        line_count++;
    }
    fclose(file);

    printf("\n--- Memory Information (first %d lines) ---\n", MAX_LINES);
    file = fopen("/proc/meminfo", "r");
    if (!file) {
        printf("could not open /proc/meminfo\n");
        return -1;
    }
    line_count = 0;
    while (line_count < MAX_LINES && fgets(line, sizeof(line), file) != NULL) {
        printf("%s", line);
        line_count++;
    }
    fclose(file);

    printf("SUCCESS: System information displayed!\n");
    return 0;
}

void compare_file_methods(void) {
    const char* test_file = "/proc/version";

    printf("Comparing file reading methods for: %s\n\n", test_file);

    printf("=== Method 1: Using System Calls ===\n");
    read_file_with_syscalls(test_file);

    printf("\n=== Method 2: Using Library Functions ===\n");
    read_file_with_library(test_file);

    printf("\nNOTE: Run this program with strace to see the difference!\n");
    printf("Example: strace -e trace=openat,read,write,close ./lab2\n");
}

int read_file_with_syscalls(const char* filename) {
    int fd = open(filename, O_RDONLY);
    char buffer[1024];
    ssize_t bytes_read;

    if (fd < 0) {
        printf("could not open %s\n", filename);
        return -1;
    }

    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        printf("%s", buffer);
    }

    close(fd);

    if (bytes_read < 0) {
        printf("read error on %s\n", filename);
        return -1;
    }
    return 0;
}

int read_file_with_library(const char* filename) {
    FILE *file = fopen(filename, "r");
    char line[256];

    if (!file) {
        printf("could not open %s\n", filename);
        return -1;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s", line);
    }

    fclose(file);
    return 0;
}

int is_number(const char* str) {
    int i = 0;
    if (str == NULL) return 0;
    if (str[0] == '\0') return 0;
    while (str[i] != '\0') {
        if (!isdigit((unsigned char)str[i])) return 0;
        i++;
    }
    return 1;
}

