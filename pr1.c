#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/stat.h>

#pragma pack(push, 1)
typedef struct {
    uint16_t type;
    uint16_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint16_t offset;
} BMPHeader;
#pragma pack(pop)

void get_permissions(mode_t mode, char *perm) {
    perm[0] = (mode & S_IRUSR) ? 'R' : '-';
    perm[1] = (mode & S_IWUSR) ? 'W' : '-';
    perm[2] = (mode & S_IXUSR) ? 'X' : '-';
    perm[3] = (mode & S_IRUSR) ? 'R' : '-';
    perm[4] = (mode & S_IWUSR) ? 'W' : '-';
    perm[5] = (mode & S_IXUSR) ? 'X' : '-';
    perm[6] = (mode & S_IRUSR) ? 'R' : '-';
    perm[7] = (mode & S_IWUSR) ? 'W' : '-';
    perm[8] = (mode & S_IXUSR) ? 'X' : '-';
    perm[9] = '\0';
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <fisier_intrare>\n", argv[0]);
        return 1;
    }

    char *input_file = argv[1];
    int input_fd = open(input_file, O_RDONLY);

    if(input_fd == -1) {
        perror("Eroare la deschiderea fisierului intrare");
        return 1;
    }

    BMPHeader bmp_header;
    if (read(input_fd, &bmp_header, sizeof(BMPHeader)) != sizeof(BMPHeader)){
        perror("Error citire BMP Header");
        close(input_fd);
        return 1;
    }

    if (bmp_header.type != 0x4D42) {
        fprintf(stderr, "Nu e fisier BPM\n");
        close(input_fd);
        return 1;
    }

    off_t file_size = bmp_header.size;

    struct stat file_info;
    if(fstat(input_fd, &file_info) == )

    printf("Nume fisier: %s\nDimensiune: %lld octeti\n", input_file, (long long) input_fd);
    close(*input_file);

    return 0;

}