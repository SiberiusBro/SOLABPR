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
    char statistics[256];
 
    fd = open("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        print_error("Error opening statistica.txt");
    }
 
    // Format the statistics string
    sprintf(statistics, "nume fisier: %s\ninaltime: %d\nlungime: %d\ndimensiune: %ld\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %scontorul de legaturi: %d\ndrepturi de acces user: %03o\ndrepturi de acces grup: %03o\ndrepturi de acces altii: %03o\n",
            filename, height, width, size, user_id, ctime(&modification_time), link_count, user_permissions, group_permissions, others_permissions);
 
    // Write the statistics to the file
    if (write(fd, statistics, strlen(statistics)) == -1) {
        print_error("Error writing to statistica.txt");
    }
 
    close(fd);
}
 
int main(int argc, char *argv[]) {
    if (argc != 2) {
        print_usage();
    }
 
    char *filename = argv[1];
 
    // Open the input file
    int input_fd = open(filename, O_RDONLY);
    if (input_fd == -1) {
        print_error("Error opening input file");
    }
 
    // Read BMP header
    // Assuming BMP header structure is a fixed size of 54 bytes
    char bmp_header[54];
    if (read(input_fd, bmp_header, sizeof(bmp_header)) == -1) {
        print_error("Error reading BMP header");
    }
 
    // Extract necessary information from BMP header
    int width = *(int*)&bmp_header[18];
    int height = *(int*)&bmp_header[22];
    off_t size = lseek(input_fd, 0, SEEK_END);
 
    // Get file information using stat
    struct stat file_info;
    if (fstat(input_fd, &file_info) == -1) {
        print_error("Error getting file information");
    }
 
    uid_t user_id = file_info.st_uid;
    time_t modification_time = file_info.st_mtime;
    int link_count = file_info.st_nlink;
 
    // Extract permission bits
    mode_t user_permissions = file_info.st_mode & S_IRWXU;
    mode_t group_permissions = (file_info.st_mode & S_IRWXG) >> 3;  // Shift right by 3 bits
    mode_t others_permissions = (file_info.st_mode & S_IRWXO) >> 6;  // Shift right by 6 bits
 
    // Write statistics to the output file
    write_statistics(filename, height, width, size, user_id, modification_time, link_count, user_permissions, group_permissions, others_permissions);
 
    // Close the input file
    close(input_fd);
 
    return 0;
}
