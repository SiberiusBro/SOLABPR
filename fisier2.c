#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
 
void print_error(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}
 
void scrie_statistici(const char *filename, int is_directory, int is_bmp, off_t size, uid_t user_id, mode_t permissions, time_t modification_time, int link_count, int width, int height) {
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
 
void convert_gri(const char *input_path, const char *output_path, int desired_width, int desired_height) {
    int input_fd = open(input_path, O_RDONLY);
    if (input_fd == -1) {
        print_error("Eroare la deschiderea fisierului BMP");
    }
 
    int output_fd = open(output_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (output_fd == -1) {
        print_error("Eroare la deschiderea fisierului de iesire");
    }
 
    char bmp_header[54];
 
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
 
void generate_content(const char *input_path, const char *output_path) {
    int input_fd = open(input_path, O_RDONLY);
    if (input_fd == -1) {
        print_error("Eroare la deschiderea fisierului obisnuit");
    }
 
    int output_fd = open(output_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (output_fd == -1) {
        print_error("Eroare la deschiderea fisierului de iesire");
    }
 
    char buffer[1024];
    ssize_t bytesRead;
 
    while ((bytesRead = read(input_fd, buffer, sizeof(buffer))) > 0) {
        if (write(output_fd, buffer, bytesRead) == -1) {
            print_error("Eroare in scriere");
        }
    }
 
    close(input_fd);
    close(output_fd);
}
 
void child_process(const char *input_dir, const char *output_dir, const char *entry_name, const char character) {
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
        } else {
            // Fișier obișnuit care nu este imagine BMP
            sprintf(output_path, "%s/%s_content.txt", output_dir, entry_name);
            generate_content(input_path, output_path);
        }
    }
 
    pid_t child_pid = fork();
 
    if (child_pid == -1) {
        print_error("Eroare la creerea procesului fiu");
    }
 
    if (child_pid == 0) {
        // Proces fiu
        scrie_statistici(output_path, S_ISDIR(entry_info.st_mode), is_bmp, entry_info.st_size, user_id, permissions, modification_time, link_count, width, height);
 
        if (is_bmp) {
            char grayscale_output_path[1024];
            sprintf(grayscale_output_path, "%s/%s_grayscale.bmp", output_dir, entry_name);
            convert_gri(input_path, grayscale_output_path, width, height);
        }
 
        exit(EXIT_SUCCESS);
    }
}

void process_file(const char *input_path, const char *output_dir, const char *entry_name, const char character) {
    char output_path[1024];

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

        // Intrarea e un fisier normal
        if (strstr(entry_name, ".bmp") != NULL) {
            is_bmp = 1;
        } else {
            // Fișier obișnuit care nu este imagine BMP
            sprintf(output_path, "%s/%s_content.txt", output_dir, entry_name);
            generate_content(input_path, output_path);
        }

    pid_t child_pid = fork();

    if (child_pid == -1) {
        print_error("Eroare la creerea procesului fiu");
    }

    if (child_pid != 0) {
    		// Proces parinte
        
    		if (is_bmp) {
        		pid_t bmp_child_pid = fork();
            if(bmp_child_pid == 0) {
            		// Nou proces fiu pentru bmp
                
                char grayscale_output_path[1024];
                sprintf(grayscale_output_path, "%s/%s_grayscale.bmp", output_dir, entry_name);
                convert_gri(input_path, grayscale_output_path, width, height);
                exit(EXIT_SUCCESS);
            }
        }
    }
    else {
    	// Proces fiu
    	scrie_statistici(output_path, S_ISDIR(entry_info.st_mode), is_bmp, entry_info.st_size, user_id, permissions, modification_time, link_count, width, height);
      if (is_bmp == 1){
        exit(EXIT_SUCCESS);
        return ;
      }
     // Open the file for reading
      size_t path_len = strlen(input_path) + strlen(entry_name) + 2; // +1 for the null terminator
      char *full_path = (char *)malloc(path_len);

			// Concatenate dir path and file
      snprintf(full_path, path_len, "%s/%s", input_path, entry_name);
      FILE *file = fopen(full_path, "r");

      if (file == NULL) {
          perror("Error opening file");
          free (full_path);
          return ;
      }
      char command[64];
snprintf(command, sizeof(command), "./script.sh %c", character);
    // Open the pipe
FILE *pipe = popen(command, "w");
      if (!pipe) {
          perror("popen");
          free (full_path);
          return ;
      }
      // Read the content of the file
      char buffer[1024];
      while (fgets(buffer, sizeof(buffer), file) != NULL) {
        fprintf(pipe, "%s", buffer);
      }
      // Close the file
      fclose(file);

      // Pass the file content to the Bash script
      if (pclose(pipe) == -1) {
          perror("pclose");
          free (full_path);
          return ;
      }
    
      exit(EXIT_SUCCESS);
    }
}
 
int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <director_intrare> <director_iesire> <c>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
 
    char *input_dir = argv[1];
    char *output_dir = argv[2];
    char character = argv[3][0];
 
    DIR *dir = opendir(input_dir);
    if (dir == NULL) {
        print_error("Eroare la deschiderea directorului de intrare");
    }
 
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
        process_file(input_dir, output_dir, entry->d_name, character);
    }
}
 
    closedir(dir);
 
    int status;
    pid_t child_pid;
    int lines_written = 0;
    int total_correct_sentences = 0;
 
    while ((child_pid = waitpid(-1, &status, 0)) != -1) {
        if (WIFEXITED(status)) {
            int child_exit_status = WEXITSTATUS(status);
            lines_written += child_exit_status;
            total_correct_sentences += lines_written;
 
            printf("S-a incheiat procesul cu pid-ul %d si codul %d. S-au scris %d linii in fisierul statistica.txt.\n", child_pid, child_exit_status, lines_written);
        } else {
            printf("Procesul cu pid-ul %d nu s-a incheiat normal.\n", child_pid);
        }
    }
 
    return 0;
}