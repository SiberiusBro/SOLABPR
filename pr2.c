#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <time.h>

void print_error(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void write_statistics(const char *filename, int is_directory, int is_bmp, off_t size, uid_t user_id, mode_t permissions, time_t modification_time, int link_count, int width, int height) {
    int fd;
    char *statistics = (char *)malloc(1024);

    if (statistics == NULL) {
        print_error("Eroare la alocarea memoriei");
    }

    fd = open("statistica.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        print_error("Eroare la deschiderea statistica.txt");
    }
    //Verificam toate cazurile
    if (is_directory) {
        sprintf(statistics, "nume director: %s\nidentificatorul utilizatorului: %d\ndrepturi de acces user: RWX\ndrepturi de acces grup: R--\ndrepturi de acces altii: ---\n", filename, user_id);
    } else if (is_bmp) {
        sprintf(statistics, "nume fisier: %s\ninaltime: %d\nlungime: %d\ndimensiune: %ld\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %scontorul de legaturi: %d\ndrepturi de acces user: %03o\ndrepturi de acces grup: %03o\ndrepturi de acces altii: %03o\n",
                filename, height, width, size, user_id, ctime(&modification_time), link_count, permissions, (permissions >> 3) & 7, (permissions >> 6) & 7);
    } else {
        sprintf(statistics, "nume fisier: %s\ndimensiune: %ld\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %scontorul de legaturi: %d\ndrepturi de acces user: %03o\ndrepturi de acces grup: %03o\ndrepturi de acces altii: %03o\n",
                filename, size, user_id, ctime(&modification_time), link_count, permissions, (permissions >> 3) & 7, (permissions >> 6) & 7);
    }

    if (write(fd, statistics, strlen(statistics)) == -1) {
        print_error("Eroare la scrierea in statistica.txt");
    }

    free(statistics);
    close(fd);
}

void process_entry(const char *dir_path, const char *entry_name) {
    char entry_path[1024];
    sprintf(entry_path, "%s/%s", dir_path, entry_name);

    struct stat entry_info;
    if (lstat(entry_path, &entry_info) == -1) {
        print_error("Eroare la fis de intrare");
    }

    uid_t user_id = entry_info.st_uid;
    mode_t permissions = entry_info.st_mode;
    time_t modification_time = entry_info.st_mtime;
    int link_count = entry_info.st_nlink;

    if (S_ISDIR(entry_info.st_mode)) {
        // Intrarea este director
        write_statistics(entry_name, 1, 0, entry_info.st_size, user_id, permissions, modification_time, link_count, 0, 0);

        // Procesam directorul recursiv
        DIR *dir = opendir(entry_path);
        if (dir == NULL) {
            print_error("Eoare la deschiderea directorului");
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                process_entry(entry_path, entry->d_name);
            }
        }

        closedir(dir);
    } else if (S_ISREG(entry_info.st_mode)) {
        // Intrarea este un fisier normal
        int is_bmp = 0;
        int width = 0, height = 0;

        // Verificam daca intrarea are .bmp
        if (strstr(entry_name, ".bmp") != NULL) {
            is_bmp = 1;

            // Extragem width si height
            int bmp_fd = open(entry_path, O_RDONLY);
            if (bmp_fd == -1) {
                print_error("Eroare la deschiderea .BMP");
            }

            char bmp_header[54];
            if (read(bmp_fd, bmp_header, sizeof(bmp_header)) == -1) {
                print_error("Eroare la citirea .BMP");
            }

            width = *(int*)&bmp_header[18];
            height = *(int*)&bmp_header[22];

            close(bmp_fd);
        }

        write_statistics(entry_name, 0, is_bmp, entry_info.st_size, user_id, permissions, modification_time, link_count, width, height);
    } else if (S_ISLNK(entry_info.st_mode)) {
        // Intrarea este legatura simbolica
        char target_path[1024];
        ssize_t target_size = readlink(entry_path, target_path, sizeof(target_path) - 1);
        if (target_size == -1) {
            print_error("Eroare la citirea legaturii simbolice");
        }
        target_path[target_size] = '\0';

        struct stat target_info;
        if (lstat(target_path, &target_info) == -1) {
            print_error("Eroare la fetch");
        }

        write_statistics(entry_name, 0, 0, entry_info.st_size, user_id, permissions, modification_time, link_count, target_info.st_size, 0);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <director_intrare>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *dir_path = argv[1];

    // Procesam intrarile din director
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        print_error("Eroare la deschiderea directorului");
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            process_entry(dir_path, entry->d_name);
        }
    }

    closedir(dir);

    return 0;
}
