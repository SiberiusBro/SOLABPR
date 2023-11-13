#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/sysmacros.h>
 
void print_error(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}
 
void write_statistics(const char *filename, int is_directory) {
    int fd;
    char statistics[256];
 
    fd = open("statistica.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        print_error("Error opening statistica.txt");
    }
 
    if (is_directory) {
        // Get directory information
        struct stat dir_info;
        if (stat(filename, &dir_info) == -1) {
            print_error("Error getting directory information");
        }
 
        // Format the statistics string for a directory
        sprintf(statistics, "nume director: %s\nidentificatorul utilizatorului: %d\ndrepturi de acces user: RWX\ndrepturi de acces grup: R--\ndrepturi de acces altii: ---\n",
                filename, dir_info.st_uid);
    } else {
        // Get file information
        struct stat file_info;
        if (stat(filename, &file_info) == -1) {
            print_error("Error getting file information");
        }
 
        // Extract permission bits
        mode_t user_permissions = file_info.st_mode & S_IRWXU;
        mode_t group_permissions = (file_info.st_mode & S_IRWXG) >> 3;  // Shift right by 3 bits
        mode_t others_permissions = (file_info.st_mode & S_IRWXO) >> 6;  // Shift right by 6 bits
 
        if (S_ISREG(file_info.st_mode)) {
            // Regular file
            if (strstr(filename, ".bmp") != NULL) {
                // BMP file
                int width = 0;
                int height = 0;
 
                // Extract necessary information from BMP header
                if (S_ISLNK(file_info.st_mode)) {
                    // Symbolic link
                    struct stat target_info;
                    if (stat(filename, &target_info) == -1) {
                        print_error("Error getting target file information");
                    }
 
                    width = target_info.st_size;
                    sprintf(statistics, "nume legatura: %s\ndimensiune: %ld\ndimensiune fisier: %ld\ndrepturi de acces user: RWX\ndrepturi de acces grup: R--\ndrepturi de acces altii: ---\n",
                            filename, (long)file_info.st_size, (long)width);
                } else {
                    // Regular BMP file
                    FILE *bmp_file = fopen(filename, "rb");
                    if (bmp_file == NULL) {
                        print_error("Error opening BMP file");
                    }
 
                    fseek(bmp_file, 18, SEEK_SET);
                    fread(&width, sizeof(int), 1, bmp_file);
                    fread(&height, sizeof(int), 1, bmp_file);
 
                    fclose(bmp_file);
 
                    sprintf(statistics, "nume fisier: %s\ninaltime: %d\nlungime: %d\n",
                            filename, height, width);
                }
            } else {
                // Regular file without .bmp extension
                sprintf(statistics, "nume fisier: %s\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %scontorul de legaturi: %ld\ndrepturi de acces user: %03o\ndrepturi de acces grup: %03o\ndrepturi de acces altii: %03o\n",
                        filename, file_info.st_uid, ctime(&file_info.st_mtime), file_info.st_nlink, user_permissions, group_permissions, others_permissions);
            }
        }
    }
 
    // Write the statistics to the file
    if (write(fd, statistics, strlen(statistics)) == -1) {
        print_error("Error writing to statistica.txt");
    }
 
    close(fd);
}
 
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ./program <director_intrare>\n");
        exit(EXIT_FAILURE);
    }
 
    char *directory_path = argv[1];
 
    // Open the directory
    DIR *dir = opendir(directory_path);
    if (dir == NULL) {
        print_error("Error opening directory");
    }
 
    // Loop through directory entries
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char entry_path[257];
            snprintf(entry_path, sizeof(entry_path), "%s/%s", directory_path, entry->d_name);
 
            // Check if entry is a symbolic link
            struct stat entry_info;
            lstat(entry_path, &entry_info);
 
            int is_directory = S_ISDIR(entry_info.st_mode);
            int is_symbolic_link = S_ISLNK(entry_info.st_mode);
 
            // Write statistics to statistica.txt
            write_statistics(entry_path, is_directory);
 
            
