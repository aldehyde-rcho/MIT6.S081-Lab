#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    int pipefd[2];
    char buf[128];
    int pid;

    if (pipe(pipefd) == -1)
    { //创建管道
        fprintf(2, "ERROR: Cannot creat a pipe\n");
        exit(-1);
    }

    int chpid = fork(); //创建子进程
    if (chpid == -1)
    {
        fprintf(2, "ERROR: Cannot fork a child\n");
        exit(-1);
    }

    if (chpid == 0)
    {                              //子进程
        read(pipefd[0], buf, 128); //读取管道

        pid = getpid();
        printf("%d: received %s\n", pid, buf); //输出

        write(pipefd[1], "pong", strlen("pong")); //写入管道

        close(pipefd[1]);
        close(pipefd[0]);
        exit(0);
    }
    else
    {                                             //父进程
        write(pipefd[1], "ping", strlen("ping")); //写入管道
        close(pipefd[1]);

        wait(0); //等待子进程

        read(pipefd[0], buf, 128); //读取管道

        pid = getpid();
        printf("%d: received %s\n", pid, buf); //输出

        close(pipefd[0]);
        exit(0);
    }

    exit(0);
}