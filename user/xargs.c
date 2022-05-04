#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"
#define MAXBUF 1024

int readline(int new_argc, char *new_argv[MAXARG]) //函数readline从标准输入流中读取*一行*字符串，并将字符串转化为不同的参数，写到new_argv新参数表的末尾，并返回新的参数表计数new_argc。如果标准输入流中未能读取字符串，则返回0。
{
    char buf[MAXBUF];
    int i = 0;
    while (read(0, buf + i, 1)) //从标准输入流（stdin，句柄为0）读入一个字，存储的指针为buf+i，i每个循环递增
    {
        if (i == MAXBUF - 1)
        {                                      //读入的字符过长，超过缓冲区大小
            fprintf(2, "argument too long\n"); //这里fprintf用到的句柄2为标准错误流（stderr）的句柄
            exit(1);
        }
        if (buf[i] == '\n') //读到换行符，停止读入
        {
            break;
        }
        i++;
    }
    buf[i] = 0; //在读入的字符数组最后写一个'\0'空字符，表示字符串结束

    if (i == 0)
        return 0; //若stdin中没有字符待读取，返回0

    //将缓冲区的字符串写入到new_argv中，并作出如下处理：
    //  将字符串中的空格转化为空字符，使空格前后的字符串被视为不同的参数

    int count = 0;
    while (count < i)
    {
        new_argv[new_argc++] = buf + count; //将buf[count]的指针赋值给argv指针数组的下一位指针。new_argc是对new_argv中指针的计数，写入新指针之后递增
        while (buf[count] != ' ' && count < i)
        {
            count++;
        }
        while (buf[count] == ' ' && count < i)
        {
            buf[count] = '\0';
            count++;
        }
        //如上两个while，逐位检查读入的字符是否为空格，并将所有空格转化为空字符，结束上一个参数的字符串，以隔开不同的参数。
        //以上两个循环结束以后，buf+count指向下一个参数字符串的第一位，在下一次嵌套的外循环开始时写入到argv中。
    }
    return new_argc; //返回修改过new_argv之后的new_argc值
}

int main(int argc, char *argv[])
{
    if (argc <= 1)
    {
        fprintf(2, "Usage: xargs command (arg ...)\n");
        exit(1);
    }

    char *command = malloc(strlen(argv[1]) + 1);
    char *new_argv[MAXARG];
    strcpy(command, argv[1]);
    for (int i = 1; i < argc; ++i)
    {
        new_argv[i - 1] = malloc(strlen(argv[i]) + 1);
        strcpy(new_argv[i - 1], argv[i]);
    }

    int new_argc = argc - 1;
    // argv的第一个指针指向command，直接从shell获取的话，在这个程序中，*argv[0]=="xarg"，*argv[1]=="<command>"，*argv[argc]=""，*new_argv[0]=="<command>"，因此在逻辑上，new_argc==argc-1，并且对i<argc，*new_argv[i-1]==*argv[i]

    while ((new_argc = readline(new_argc, new_argv)) != 0)
    {
        new_argv[new_argc] = 0;
        if (fork() == 0)
        {
            exec(command, new_argv);
            fprintf(2, "exec error !\n");
            exit(1);
        }
        wait(0);
        new_argc = argc - 1; //将new_argc重新置为argc-1
    }

    exit(0);
}
