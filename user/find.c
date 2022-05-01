#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

int find(char *path, char *targent)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;
    int findSuccess = 0;

    if ((fd = open(path, 0)) < 0)
    {
        fprintf(2, "find: cannot open %s\n", path);
        return -1;
    }

    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return -1;
    }

    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';

    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {
        if (de.inum == 0)
            continue;

        if (!strcmp(de.name, ".") || !strcmp(de.name, ".."))
        {
            continue;
        }
        memmove(p, de.name, DIRSIZ);
        *(p + DIRSIZ) = 0;

        if (stat(buf, &st) < 0)
        {
            printf("find: connot stat file %s\n", buf);
            continue;
        }
        switch (st.type)
        {
        case T_FILE:
            if (strcmp(de.name, targent) == 0)
            {
                printf("%s\n", buf);
                findSuccess = 1;
            }
            break;

        case T_DIR:
            findSuccess = find(buf, targent);
            break;
        }
    }
    close(fd);
    return findSuccess;
}

int main(int argc, char *argv[])
{
    int findSuccess = 0;

    if (argc < 2 || argc > 3) //参数数量过多或过少
    {
        printf("Usage: find the file...\n");
        printf("\tfind <filename>\n\t\t- find the targent in this directory\n");
        printf("\tfind <path> <filename>\n\t\t- find the targent in the path\n");
        exit(0);
    }
    if (argc == 2) //默认在当前路径下开始查找
        findSuccess = find(".", argv[1]);
    if (argc == 3)
        findSuccess = find(argv[1], argv[2]);

    if (findSuccess == 0) //查找失败
        printf("Cannot find the targent...\n");
    exit(0);
}
