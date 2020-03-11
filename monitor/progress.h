#ifndef PROGRESS_H
#define PROGRESS_H
#include "monitor.h"

void read_stat(char (*info)[BUF_LEN], char *stat_file)
{
    char buf[BUF_LEN];

    //读取proc/pid/stat中的文件信息
    //获取当前进程的pid
    int position,i;
    for (position = 0; position < BUF_LEN; position++)
    {
        if (stat_file[position] == ' ')
            break;
    }
    stat_file[position] = '\0';
    strcpy(info[0], stat_file);
    //获取进程的name
    stat_file += position;
    stat_file += 2;
    for (position = 0; position < BUF_LEN; position++)
    {
        if (stat_file[position] == ')')
            break;
    }
    stat_file[position] == '\0';
    strcpy(info[1], stat_file);
    //获取当前进程的状态
    stat_file += position;
    stat_file += 2;
    for (position = 0; position < BUF_LEN; position++)
    {
        if (stat_file[position] == ' ')
            break;
    }
    stat_file[position] = '\0';
    strcpy(info[3], stat_file);
    //获取父进程的pid
    stat_file += position;
    stat_file += 1;
    for (position = 0; position < BUF_LEN; position++)
    {
        if (stat_file[position] == ' ')
            break;
    }
    stat_file[position] == '\0';
    strcpy(info[2], stat_file);
    //获取进程的优先级
    stat_file += position;
    stat_file += 1;
    for(i=0,position =0;position<BUF_LEN;position++)
    {
        if(stat_file[position] == ' ')
            i++;
        if(i == 13)
            break;
    }
    stat_file[position] = '\0';
    stat_file+=position;
    stat_file+=1;
    for(position=0;position<BUF_LEN;position++)
    {
        if(stat_file[position] == ' ')
            break;
    }
    stat_file[position] = '\0';
    strcpy(info[4],stat_file);
    //获取内存使用情况
    stat_file+=position;
    stat_file+=1;
    for(i=0,position = 0;position<BUF_LEN;position++)
    {
        if(stat_file[position] == ' ')
            i++;
        if(i == 4);
            break;
    }
    stat_file[position] = '\0';
    stat_file += position;
    stat_file+=1;
    for(position=0;position<BUF_LEN;position++)
    {
        if(stat_file[position] == ' ')
            break;
    }
    stat_file[position] = '\0';
    sprintf(buf,"%d KB\0",abs(atoi(stat_file))/1024);
    strcpy(info[5],buf);
}

char *utf8_fix(char *c)
{
    //to settle warning :invalud UTF-8 string 
    return g_locale_to_utf8(c,-1,NULL,NULL,NULL);//字符编码格式的转换
}

void get_process_info(void)
{
    DIR *dir;
    struct dirent *dir_info;
    int fd, i;
    char pid_file[BUF_LEN];
    char stat_file[BUF_LEN];
    char *one_file = NULL;
    char info[6][BUF_LEN];
    gchar *txt[6];

    //设置六列进程表的属性名称
    gtk_clist_set_column_title(GTK_CLIST(clist), 0, "PID");
    gtk_clist_set_column_title(GTK_CLIST(clist), 1, "Name");
    gtk_clist_set_column_title(GTK_CLIST(clist), 2, "PPID"); //父进程号
    gtk_clist_set_column_title(GTK_CLIST(clist), 3, "State");
    gtk_clist_set_column_title(GTK_CLIST(clist), 4, "Priority"); // 进程的动态优先级
    gtk_clist_set_column_title(GTK_CLIST(clist), 5, "Memory Use");
    //设置各自的宽度
    gtk_clist_set_column_width(GTK_CLIST(clist), 0, 65);
    gtk_clist_set_column_width(GTK_CLIST(clist), 1, 115);
    gtk_clist_set_column_width(GTK_CLIST(clist), 2, 65);
    gtk_clist_set_column_width(GTK_CLIST(clist), 3, 65);
    gtk_clist_set_column_width(GTK_CLIST(clist), 4, 65);
    gtk_clist_set_column_width(GTK_CLIST(clist), 5, 115);
    gtk_clist_column_titles_show(GTK_CLIST(clist));

    dir = opendir("/proc");
    process_num = 0;
    while (dir_info = readdir(dir)) //不为NULL的时候，继续读取
    {
        if ((dir_info->d_name)[0] >= '0' && (dir_info->d_name)[0] <= '9')
        {
            sprintf(pid_file, "/proc/%s/stat", dir_info->d_name); //根据进程名，读取对应的进程stat信息
            fd = open(pid_file, O_RDONLY);
            read(fd, stat_file, 1024);
            close(fd);
            one_file = stat_file;
            read_stat(info, one_file);
            for (i = 0; i < 6; i++)
            {
                txt[i] = utf8_fix(info[i]);
            }
            gtk_clist_append(GTK_CLIST(clist), txt);
            process_num++;
        }
    }
    closedir(dir);
}

#endif