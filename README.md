# A Communicating MultiShell
## Prerequisites
- C system calls
- Exec family functions
- Fork
- File I/O

## Learning objectives
Practicing fork/exec relationships. Understanding the multi-process environment and how they can communicate through a shared object, understanding the details of a shell program. Understanding the need for inter process communication mechanisms which will be taught in the coming weeks.
##Intro
In this project, you are required to implement a multi-shell program that opens multiple shells which share a file object with each other. For this, you are given singleshell.c ve multishell.c files. Below is the details and todos for these files:
##Details
### singleshell.c
It is a shell application that we have done partially in the class.
Here, it will write all the stdout messages we see in the shell also to a shared file object. The memory shared in the file is created as follows:


/*contents of myinit() function*/
fd = shm_open(MY_SHARED_FILE_NAME, O_RDWR, 0);
/*Map shared memory*/
addr = mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

Here shm_open creates a shared file object (the usage is exactly the same as open), mmap maps this object to memory.We will cover the details of this in the coming weeks.
**You can think of it similar to malloc with more options. Therefore, you can think of the pointer addr as an array. You can treat the variable as an array variable:
addr[0], addr[1] 

To copy a string to this address, you can use the following functions:
strncpy(&addr[...], …, …), 
memcpy(&addr[...],...., …) **




Of course, before copying a string, you have to find the end of addr then add the string to the end. Make sure not to overflow the memory. As an eaple
strncpy(&addr[strlen(addr)], data, max_length);

- You can also use fd directly with write and read system calls.
In singleshell.c you need to complete the followings:
1. **do not use system() anywhere in your program!!!!** You should implement it by using exec and fork.
2. Program should copy/write all the stdout and stderr messages to addr as well(put pid, to the beginning, and  \n to the end of each message).
- The messages should be seen in both shell and in the file(you can think of it like a log file)
- When writing to the file, it should add its own process id before every message (we will use userids by changing this step in future projects).
3. The initial message is written when the program starts to run: it should write its own process id and parent process id and start date, month, day, and time to the file.
4. exit when shell is given exit command, it should terminate and write its end date, month, day, time before the termination process.
5. ls  etc: when given system programs,  it should run them as in a normal shell. 
- The shell should wait for the program to finish, then continue its work.
6. It should be able to run them when the program name is given in the current directory or in another directory
- for example,./directory1/directory2/programname
- Also, the given arguments should be guided appropriately.
7. When exiting, it should close all file descriptors and so on. 
8- It should also check for errors in all the system calls you use.

### multishell
It creates a given number of singleshell program instances. For example
multishell 4

1. 4 singleshell process is created with xterm to show them in separate windows (xterm singleshell):
![image](https://github.com/berkaycelik66/Communicating-Multi-Shell/assets/90213645/9f1dbe17-de01-45f9-83e7-3aba16ed8d25)

2- When all the latest shell applications are finished, the data is saved from the shared memory (addr) to a newly opened file with the name shelllog-[datetime].txt 

3- The program ends by creating a file, saving it here and closing all fds.
