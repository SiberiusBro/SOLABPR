#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
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
        print_error("Eroare la alocarea memoriei in statistica");
    }

    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        print_error("Eroare la deschiderea statistica");
    }

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
        print_error("Eroare in scriere");
    }

    free(statistics);
    close(fd);
}

void convert_to_grayscale(const char *input_path, const char *output_path, int desired_width, int desired_height) {
    int input_fd = open(input_path, O_RDONLY);
    if (input_fd == -1) {
        print_error("Eroare la deschiderea fisierului BMP");
    }

    int output_fd = open(output_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (output_fd == -1) {
        print_error("Eroare la deschiderea fisierului de iesire");
    }

    char bmp_header[54];
    if (read(input_fd, bmp_header, sizeof(bmp_header)) == -1) {
        print_error("Eroare la citirea .BMP");
    }

    // Update width si height la valorile cerute
    *(int*)&bmp_header[18] = desired_width;
    *(int*)&bmp_header[22] = desired_height;

    if (write(output_fd, bmp_header, sizeof(bmp_header)) == -1) {
        print_error("Eroare la scrierea BMP");
    }

    for (int i = 0; i < desired_height; ++i) {
        for (int j = 0; j < desired_width; ++j) {
            unsigned char pixel[3];
            if (read(input_fd, pixel, sizeof(pixel)) == -1) {
                print_error("Eroare la citirea pixeli");
            }

            unsigned char grayscale_value = (unsigned char)(0.299 * pixel[2] + 0.587 * pixel[1] + 0.114 * pixel[0]);

            if (write(output_fd, &grayscale_value, sizeof(grayscale_value)) == -1) {
                print_error("Eroare la citirea pixelilor");
            }
        }
    }

    close(input_fd);
    close(output_fd);
}


void process_entry(const char *input_dir, const char *output_dir, const char *entry_name) {
    char input_path[1024];
    char output_path[1024];

    sprintf(input_path, "%s/%s", input_dir, entry_name);
    sprintf(output_path, "%s/%s_statistica.txt", output_dir, entry_name);

    struct stat entry_info;
    if (lstat(input_path, &entry_info) == -1) {
        print_error("Eroare la fetch informatii de intrare");
    }

    uid_t user_id = entry_info.st_uid;
    mode_t permissions = entry_info.st_mode;
    time_t modification_time = entry_info.st_mtime;
    int link_count = entry_info.st_nlink;

    int is_bmp = 0, width = 0, height = 0;

    if (S_ISREG(entry_info.st_mode)) {
        // Intrarea e un fisier normal
        if (strstr(entry_name, ".bmp") != NULL) {
            is_bmp = 1;

            // Fetch width si height din BMP
            int bmp_fd = open(input_path, O_RDONLY);
            if (bmp_fd == -1) {
                print_error("Eroare la deschiderea fisierului BMP");
            }

            char bmp_header[54];
            if (read(bmp_fd, bmp_header, sizeof(bmp_header)) == -1) {
                print_error("Eroare la citirea BMP");
            }

            width = *(int*)&bmp_header[18];
            height = *(int*)&bmp_header[22];

            close(bmp_fd);
        }
    }

    pid_t child_pid = fork();

    if (child_pid == -1) {
        print_error("Eroare la creerea procesului fiu");
    }

    if (child_pid == 0) {
        // Proces fiu
        write_statistics(output_path, S_ISDIR(entry_info.st_mode), is_bmp, entry_info.st_size, user_id, permissions, modification_time, link_count, width, height);

        if (is_bmp) {
            char grayscale_output_path[1024];
            sprintf(grayscale_output_path, "%s/%s_grayscale.bmp", output_dir, entry_name);
            convert_to_grayscale(input_path, grayscale_output_path, width, height);
        }

        exit(EXIT_SUCCESS);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <director_intrare> <director_iesire>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *input_dir = argv[1];
    char *output_dir = argv[2];

    DIR *dir = opendir(input_dir);
    if (dir == NULL) {
        print_error("Eroarea la deschiderea fisierului de intrare");
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            process_entry(input_dir, output_dir, entry->d_name);
        }
    }

    closedir(dir);

    int status;
    pid_t child_pid;
    int lines_written = 0;

    while ((child_pid = waitpid(-1, &status, 0)) != -1) {
        if (WIFEXITED(status)) {
            int child_exit_status = WEXITSTATUS(status);
            lines_written += child_exit_status;
            printf("S-a incheiat procesul cu pid-ul %d si codul %d. S-au scris %d linii in fisierul statistica.txt.\n", child_pid, child_exit_status, lines_written);
        } else {
            printf("Procesul cu pid-ul %d nu s-a incheiat normal.\n", child_pid);
        }
    }

    return 0;
}
