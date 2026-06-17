#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void create_file(char * file_name){
    int dst = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dst == -1) {
        perror("Can`t create file");
        return;
    }
    
    printf("File created: %s", file_name);
}


void remove_dir(char * package_name){
    DIR * dir = opendir(package_name);
    
    if (!dir){
	perror("Can`t open package");
	return;
    }
    
    struct dirent *entry;
    
    while ((entry = readdir(dir)) != NULL) {
	remove(entry->d_name);
        printf("Removed file: %s\n", entry->d_name);
	}
    
    rmdir(package_name);
}
void print_info_dir(char * package_name){
    DIR * dir = opendir(package_name);
    
    if (!dir){
	perror("Can`t open package");
	return;
    }
    
    struct dirent *entry;
    
    while ((entry = readdir(dir)) != NULL) {
        printf("Found file: %s\n", entry->d_name);
	}
}



void create_dir(char * package_name){
    printf("%s\n" ,package_name);
    if (mkdir(package_name, 0755) == -1) {
        perror("Can`t create package");
    }

    printf("Directory is successfully created");

}

void print_file(char * file_name){
    int src = open(file_name, O_RDONLY);
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
    
    char buffer;
    for(int i = 0; i < file_size; i++){
	if (read(src, &buffer, 1) != 1) {
            perror("Error while reading file.");
            close(src);
            return;
        }
        printf("%c", buffer);

    }
    
    close(src);
}

void remove_file(char * file_name){
    int src = open(file_name, O_RDONLY);
    if (src == -1) {
        perror("Can`t find file");
        return;
    }
    
    remove(file_name);
    printf("Removed file: %s\n", file_name);
}

void create_symlink(char * file_name){
    char symlink_name[256];
    strcpy(symlink_name, file_name);

    strcat(symlink_name, "_symlink");

    if (symlink(file_name, symlink_name) == -1) {
        perror("Failed to create symlink");
        return;
    }

    printf("Symbolic link is created for file: %s", file_name);
}

void infofile_symlink(char * sym_name){
    int fd = open(sym_name, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return;
    }
    
    char buffer[128];
    ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        printf("Read: %s\n", buffer);
    }

    close(fd);
}

void info_symlink(char * sym_name){
    char buffer[256];
    ssize_t len = readlink(sym_name, buffer, sizeof(buffer) - 1);
    if (len == -1) {
        perror("readlink");
        return;
        }
    
    buffer[len] = '\0';
    printf("Symlink content: %s\n", buffer);
}

void remove_symlink(char * sym_name){
    unlink(sym_name);
}


void create_hardlink(char * file_name){
    if(link(file_name, "hard_link") == 0)
	printf("Hard link created\n");
}


void remove_hardlink(char * file_name){
    struct stat target_stat;
    
    if (stat(file_name, &target_stat) == -1) {
        perror("stat");
        return;
    }
    
    ino_t target_inode = target_stat.st_ino; 
    
    DIR *dir = opendir(".");
    if (!dir) {
        perror("opendir");
        return;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        struct stat st;

        if (stat(entry->d_name, &st) == -1) {
            perror("stat");
            continue;
        }

        if (st.st_ino == target_inode && strcmp(entry->d_name, file_name) != 0) {
            if (unlink(entry->d_name) == -1) {
                perror("unlink");
            } else {
                printf("Deleted hard link: %s\n", entry->d_name);
            }
        }
    }

    closedir(dir);
}


void info_about_file(char * file_name){
    struct stat file_stat;
    
    
    if (stat(file_name, &file_stat) == -1) {
        perror("stat");
        return;
    }
    
    if(remove(file_name) == -1){
	perror("delete");
	return;
    }
    mode_t mode = file_stat.st_mode;
    
    
    char perms[10];
    
    perms[0] = (mode & S_IRUSR) ? 'r' : '-';  // Владелец: чтение
    perms[1] = (mode & S_IWUSR) ? 'w' : '-';  // Владелец: запись
    perms[2] = (mode & S_IXUSR) ? 'x' : '-';  // Владелец: выполнение

    perms[3] = (mode & S_IRGRP) ? 'r' : '-';  // Группа: чтение
    perms[4] = (mode & S_IWGRP) ? 'w' : '-';  // Группа: запись
    perms[5] = (mode & S_IXGRP) ? 'x' : '-';  // Группа: выполнение

    perms[6] = (mode & S_IROTH) ? 'r' : '-';  // Остальные: чтение
    perms[7] = (mode & S_IWOTH) ? 'w' : '-';  // Остальные: запись
    perms[8] = (mode & S_IXOTH) ? 'x' : '-';  // Остальные: выполнение

    perms[9] = '\0';

    printf("Permissions: %s\n", perms);
    
     printf("Hard links: %lu\n", (unsigned long)file_stat.st_nlink);


}


void change_rights(char * file_name){
    puts("Write new rights: ");
    char str[10];
    scanf("%9s", str);
    mode_t new_mode = strtol(str, NULL, 8);
    
    if (chmod(file_name, new_mode) == -1) {
        perror("chmod");
        return;
    }

    printf("Permissions of '%s' changed to %o.\n", file_name, new_mode);

}


int main(int argc, char * argv[]){

    if (argc < 2){
	perror("Not enough arguments");
	return 1;
	}
    
    if(strcmp(argv[0], "./mkdir_link") == 0){ //a
	create_dir(argv[1]);
    }
    else if(strcmp(argv[0], "./info_dir_link") == 0){ //b
	print_info_dir(argv[1]);
	}
    else if(strcmp(argv[0], "./rmdir_link") == 0){  //c
	remove_dir(argv[1]);
	}
    else if(strcmp(argv[0], "./create_file_link") == 0){ //d
	create_file(argv[1]);
	}
    else if(strcmp(argv[0], "./print_data_link") == 0){ //e
	print_file(argv[1]);
	}
    else if(strcmp(argv[0], "./rmfile_link") == 0){ //f
	remove_file(argv[1]);
	}
    else if(strcmp(argv[0], "./create_sym_link") == 0){ //g
	create_symlink(argv[1]);
	}
    else if(strcmp(argv[0], "./readfile_sym_link") == 0){ //i
	infofile_symlink(argv[1]);
	}
    else if(strcmp(argv[0], "./read_sym_link") == 0){ //h
	info_symlink(argv[1]);
	}
    else if(strcmp(argv[0], "./rm_sym_link") == 0){ //j
	remove_symlink(argv[1]);
	}
    else if(strcmp(argv[0], "./create_hard_link") == 0){ //k
	create_hardlink(argv[1]);
	}
    else if(strcmp(argv[0], "./rm_hard_link") == 0){ //l
	remove_hardlink(argv[1]);
	}
    else if(strcmp(argv[0], "./info_about_link") == 0){ //m
	info_about_file(argv[1]);
	}
    else if(strcmp(argv[0], "./change_rights_link") == 0){ //n
	change_rights(argv[1]);
	}
    return 0;
}