#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MINNUM 2
#define MAXNUM 35

void primesChild(int *pipefd)
{
    close(pipefd[1]);

    int int_len = sizeof(int);
    int recentNum;

    if (read(pipefd[0], &recentNum, int_len) == int_len)
    {
        printf("prime %d\n", recentNum); //读入第一个数值，输出

        int nextPipefd[2];
        if (pipe(nextPipefd) == -1)
        { //创建管道,绑定到nextPipefd上
            fprintf(2, "ERROR: Cannot creat a pipe\n");
            exit(-1);
        }

        int childId = fork();
        if (childId == -1)
        {
            fprintf(2, "ERROR: Cannot creat a child\nExit at pid: %d", getpid());
            exit(-1);
        }

        if (childId == 0)
        { //子进程迭代
            primesChild(nextPipefd);
            exit(0);
        }

        if (childId > 0)
        {             //父进程读取上一级的pipe内容，筛选并写入下一级pipe
            int temp; //存储读入的数值
            while (read(pipefd[0], &temp, int_len) == int_len)
            {
                if ((temp % recentNum) != 0)
                {
                    write(nextPipefd[1], &temp, int_len);
                } //筛选并写入下一级的pipe中
            }
            close(nextPipefd[0]); //关闭下一级的读端。
            close(nextPipefd[1]);
            wait(0);
        }
    }
}

//定义：管道句柄的1为写端,0为读端
int main(int argc, char *argv[])
{
    int pipefd[2];
    int childId;
    int int_len = sizeof(int);

    if (pipe(pipefd) == -1)
    { //创建管道,绑定到pipefd上
        fprintf(2, "ERROR: Cannot creat a pipe\n");
        exit(-1);
    }

    childId = fork();
    if (childId == -1)
    {
        fprintf(2, "ERROR: Cannot creat a child\nExit at pid: %d", getpid());
        exit(-1);
    } //子进程创建失败的情况

    if (childId == 0)
    {
        primesChild(pipefd);
        exit(0);
    } //子进程进入素数筛

    if (childId > 0)
    {
        int maxvar;
        if (argc == 2)
        { //如果随着primes命令有参数输入，尝试将参数作为筛查的上限
            maxvar = atoi(argv[1]);
        }
        else
        {
            maxvar = MAXNUM;
        }
        if (maxvar < 2)
        {
            printf("Command \"primes\" is to return primes from 2 to 35.");
            exit(0);
        }
        if (maxvar == 2)
        { //当查询上限数值为2会报错，避免一下。
            printf("primes 2\n");
            exit(0);
        }

        for (int i = MINNUM; i <= maxvar; i++)
        {
            int cur_write_bytes = 0;
            if ((cur_write_bytes = write(pipefd[1], &i, int_len)) != int_len)
            {
                fprintf(2, "ERROR: Cannot write integer %d to pipe, %d bytes\n", i, cur_write_bytes);
            }
        } //将定义域中的数值写入pipe

        close(pipefd[1]);
    }

    close(pipefd[1]);
    wait(0);

    exit(0);
}