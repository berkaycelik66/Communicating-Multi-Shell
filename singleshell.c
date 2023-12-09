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
#define INBUF_SIZE 256

/* these should be the same as multishell.c */
#define MY_FILE_SIZE 1024
#define MY_SHARED_FILE_NAME "/sharedlogfile"

char *addr = NULL;
int fd = -1;

int initmem()
{
    fd = shm_open(MY_SHARED_FILE_NAME, O_RDWR, 0);
    if (fd < 0){
        perror("singleshell.c:fd:line31");
        exit(1);
    }
    addr = mmap(NULL, MY_FILE_SIZE,
                PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == NULL){
        perror("singleshell.c:mmap:");
        close(fd);
        exit(1);
    }
    
    return 0;

}

int main(int argc, char *argv[])
{
    initmem();

    char inbuf[INBUF_SIZE] = {'\0'};
    int nbyte; /* input byte count */
    
    pid_t pid = getpid();
    pid_t ppid = getppid();

    /* Get the current time(CHATGPT) */
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    /* Burada sprintf fonksiyonu ile yazılan kod addr'a kopyalanıyor.*/
    sprintf(addr, "Process id: %d\nParent process id: %d\nStart time: %d-%02d-%02d %02d:%02d:%02d\n",
            pid, ppid, tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    addr += strlen(addr); /*Daha sonra addr, karakter dizisinin sonunu işaret etmesi için arttırılır.*/

    while(1) {
        write(1, "$ ", 2);

        if ((nbyte = read(0, inbuf, 255)) <= 0){
            perror("input <=0 byte");
        } else{
            inbuf[nbyte - 1] = '\0';
        }
  
        if (strncmp(inbuf, "exit", 4) == 0){
            t = time(NULL);
            tm = localtime(&t);

            sprintf(addr,"\nProcess id: %d\n", getpid());
            addr += strlen(addr);

            sprintf(addr, "$ %s\n",inbuf);
            addr += strlen(addr);
            
            sprintf(addr, "Exit time: %d-%02d-%02d %02d:%02d:%02d\n\n",
                   tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
            addr += strlen(addr);
                         
            exit(0);
        }
        
        pid_t child_pid = fork();

        sprintf(addr,"\nProcess id: %d\n", getpid());
        addr += strlen(addr);

        sprintf(addr, "$ %s\n",inbuf);
        addr += strlen(addr);

        if (child_pid == 0){
            int r = execl(inbuf,inbuf, NULL );

            if(r == -1){
                char command[255] = {'/', 'b', 'i', 'n','/', '\0'};
                strncat(command,inbuf, 200);
                r = execl(command,inbuf, NULL );
                if (r == -1){
                    perror("execl");
                    exit(1);
                }    
            }
        }else if (child_pid > 0){
            wait(0);
        }else{
            perror("\nfork() hata");
        }
    }

    // Unmap the shared memory
    munmap(addr, 1024);

    // Close the shared memory file
    close(fd);

    return 0;
}
