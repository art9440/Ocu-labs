#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


void reverse_string(char *str) {
    int length = strlen(str);
    for (int i = 0; i < length / 2; i++) {
        char temp = str[i];
        str[i] = str[length - i - 1];
        str[length - i - 1] = temp;
    }
}


void copy_file_reverse(const char *src_path, const char *dst_path) {
    int src = open(src_path, O_RDONLY);
    if (src == -1) {
        perror("Can`t open file");
        return;
    }

    struct stat st;
    if (fstat(src, &st) == -1) {
        perror("Can`t get info about file");
        close(src);
        return;
    }

    int file_size = st.st_size;

    int dst = open(dst_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dst == -1) {
        perror("Can`t create distanation file");
        close(src);
        return;
    }

    char buffer;
    for (int i = file_size - 1; i >= 0; i--) {
        if (lseek(src, i, SEEK_SET) == -1) {
            perror("Error while doing lseek");
            close(src);
            close(dst);
            return;
        }
        if (read(src, &buffer, 1) != 1) {
            perror("Error while reading file");
            close(src);
            close(dst);
            return;
        }
        if (write(dst, &buffer, 1) != 1) {
            perror("Error while writing to file");
            close(src);
            close(dst);
            return;
        }
    }

    close(src);
    close(dst);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Can`t find path.\n");
        return 1;
    }

    char *src_dir = argv[1];
    printf("src_dir %s\n", src_dir);
    
    struct stat statbuf;
    if (stat(src_dir, &statbuf) == -1 || !S_ISDIR(statbuf.st_mode)) {
        printf("This path is not a dir.\n");
        return 1;
    }
    
    char *last_slash = strrchr(src_dir, '/');
    if (last_slash == NULL) {
        last_slash = src_dir;
    } else {
        last_slash++;
    }
    
    char new_dir[256];
    strncpy(new_dir, src_dir, last_slash - src_dir);
    new_dir[last_slash - src_dir] = '\0';
    
    char last_name[256];
    strcpy(last_name, last_slash); 
    reverse_string(last_name);  // Invert name
    printf("after reverse %s\n", src_dir);

    char full_new_dir[512];
    snprintf(full_new_dir, sizeof(full_new_dir), "%s%s", new_dir, last_name); //create new path

    printf("Try to create dir: %s\n", full_new_dir);
    printf("src_dir %s\n", src_dir);
    if (mkdir(full_new_dir, 0755) == -1) {
        perror("Can`t create dir");
        return 1;
    }

    DIR *dir = opendir(src_dir);
    if (!dir) {
        perror("Can`t open dir");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
	printf("Found file: %s\n", entry->d_name);
	puts("*\n");
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char src_path[512];
        snprintf(src_path, sizeof(src_path), "%s/%s", src_dir, entry->d_name);

        struct stat file_stat;
        if (stat(src_path, &file_stat) == -1 || !S_ISREG(file_stat.st_mode)) {
    	    printf("Not regular: %s\n", entry->d_name);
            continue;  // Пропускаем файлы других типов
        }

        char reversed_name[256];
        strcpy(reversed_name, entry->d_name);
        reverse_string(reversed_name);
        printf("original name: %s\n", entry->d_name);
        printf("reversed name: %s\n", reversed_name);

        char dst_path[512];
        snprintf(dst_path, sizeof(dst_path), "%s/%s", full_new_dir, reversed_name);

        printf("Copy file %s в %s\n", src_path, dst_path);

        copy_file_reverse(src_path, dst_path);
    }

    closedir(dir);

    printf("Package and files are successfully copied.\n");

    return 0;
}
