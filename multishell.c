#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

/* these should be the same as multishell.c */
#define MY_FILE_SIZE 1024
#define MY_SHARED_FILE_NAME "/sharedlogfile"

#define MAX_SHELL 10
#define DEFAULT_NSHELL 2
char *addr = NULL; /*mmap addres*/
int fd = -1;       /*fd for shared file object*/

int initmem()
{
    fd = shm_open(MY_SHARED_FILE_NAME,
                O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (fd < 0){
        perror("multishell.c:open file:");
        exit(1);
    }

    if (ftruncate(fd, 1024) == -1){
        perror("ftruncate");
        exit(1);
    }

    addr = mmap(NULL, MY_FILE_SIZE,
                PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == NULL){
        perror("mmap:");
        exit(1);
    }
    return 0;
}

int main(int argc, char **argv)
{   

    initmem();

    char addr_log[MY_FILE_SIZE]; /*Copy Adder'dakileri dosyaya yazmak için*/
    char *cpy_addr = addr_log; /*Birden fazla singleshell oluşturuluyor ve herhangi biri kapatılınca adder sıfırlanıyor. 
                                Bu sebeple açılan bütün singleshell'lerin addr bilgisini cpy_addr ile tek bir yerde toplamak için oluşturduk. */

    pid_t child_pid;
    int shell_count; /*Açılacak pencere sayısını atamak için oluşturduk.*/

    if(argc < 2){
        shell_count = DEFAULT_NSHELL;
    } else {
        shell_count = atoi(argv[1]); /* argüman olarak verilen sayıyı integer'a çevirir (CHATGPT)*/    
    } 
    
    if(shell_count < DEFAULT_NSHELL){ /* 2'den küçük ise default olarak 2 açılmalı.*/
        shell_count = DEFAULT_NSHELL;
    } else if (shell_count > MAX_SHELL){ /*10'dan büyük ise en yüksek değer olan 10 açılmalı*/
        shell_count = MAX_SHELL;
    }

    for(int i=0; i < shell_count; i++){
        child_pid = fork();
        if (child_pid == 0){
            int r = execl("/usr/bin/xterm","xterm", "-e", "./singleshell", NULL );
            if (r == -1){
                perror("execl");
                exit(0);
            }
        }else if (child_pid > 0){
            
        }else{
            perror("\nfork() hata");
        }
    }

    for(int j=0; j < shell_count ;j++){ 
        wait(0); /*En son singleshell kapatılana kadar multishell'i bekletir.*/
        sprintf(cpy_addr, "%s\n", addr);
        cpy_addr += strlen(cpy_addr);
    }

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char filename[100];
    sprintf(filename, "shelllog-%d-%02d-%02d %02d:%02d:%02d.txt",
        tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
            

    int fd_log = open(filename, O_WRONLY | O_APPEND | O_CREAT, S_IRWXU); /*shelllog için yeni bir file descriptor oluşturuldu.*/
    if (fd_log == -1){
        perror("cannot open file: ");
        return -1;
    }

    int ret = write(fd_log, addr_log, strlen(addr_log));
    if (ret == -1){
        perror("cannot write!");
        close(fd_log);
        return -1;
    }

    close(fd_log);

    /*unlink mmap*/
    munmap(addr, 1024);
    /* close the shared memory file*/
    close(fd);

    return 0;
}
