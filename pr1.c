#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

void print_error(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void print_usage() {
    fprintf(stderr, "Usage: ./program poza.bmp\n");
    exit(EXIT_FAILURE);
}

void write_statistics(const char *filename, int height, int width, off_t size, uid_t user_id, time_t modification_time, int link_count, mode_t user_permissions, mode_t group_permissions, mode_t others_permissions) {
    int fd;
    // Crestem marimea buffer-ului
    char *statistics = (char *)malloc(1024);

    // Verificare malloc
    if (statistics == NULL) {
        print_error("Eroare la alocarea memoriei");
    }

    fd = open("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        print_error("Eroare la deschiderea statistica.txt");
    }

    // Formatam string
    sprintf(statistics, "nume fisier: %s\ninaltime: %d\nlungime: %d\ndimensiune: %ld\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %s contorul de legaturi: %d\ndrepturi de acces user: %03o\ndrepturi de acces grup: %03o\ndrepturi de acces altii: %03o\n",
            filename, height, width, size, user_id, ctime(&modification_time), link_count, user_permissions, group_permissions, others_permissions);

    // Scrie statisticile in fisier
    if (write(fd, statistics, strlen(statistics)) == -1) {
        print_error("Eroare in scrierea statistica.txt");
    }

    // Free malloc
    free(statistics);

    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        print_usage();
    }

    char *filename = argv[1];

    // Deschide fisierul de intrare
    int input_fd = open(filename, O_RDONLY);
    if (input_fd == -1) {
        print_error("Eroare la deschiderea fisierului");
    }

    // Citeste .BMP
    // Presupunem ca este pe 54 bytes
    char bmp_header[54];
    if (read(input_fd, bmp_header, sizeof(bmp_header)) == -1) {
        print_error("Eroare la citirea .BMP");
    }

    // Extragem informatiile necesare din fisierul BMP
    int width = *(int*)&bmp_header[18];
    int height = *(int*)&bmp_header[22];
    off_t size = lseek(input_fd, 0, SEEK_END);

    // Se extrag informatiile cu stat
    struct stat file_info;
    if (fstat(input_fd, &file_info) == -1) {
        print_error("Eroare la extragerea informatiilor");
    }

    uid_t user_id = file_info.st_uid;
    time_t modification_time = file_info.st_mtime;
    int link_count = file_info.st_nlink;

    // Extragem permisiunile pe biti
    mode_t user_permissions = file_info.st_mode & S_IRWXU;
    mode_t group_permissions = (file_info.st_mode & S_IRWXG) >> 3;  // Shift dr cu 3 biti
    mode_t others_permissions = (file_info.st_mode & S_IRWXO) >> 6;  // Shift dr cu 6 biti

    // Scriem statisticile in fisierul de iesire
    write_statistics(filename, height, width, size, user_id, modification_time, link_count, user_permissions, group_permissions, others_permissions);

    // Inchidem input
    close(input_fd);

    return 0;
}

