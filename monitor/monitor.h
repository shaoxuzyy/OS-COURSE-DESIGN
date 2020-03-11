#ifndef MONITOR_H
#define MONITOR_H

#include <gtk/gtk.h>
#include <dirent.h> //linux中，为获取某文件的目录使用的结构体
#include <stdio.h>      /* Standard I/O functions */
#include <stdlib.h>     /* Standard library */
#include <string.h>     /* String handling functions */
#include <unistd.h>     /* Unix system calls */
#include <errno.h>      /* Error difinations */
#include <fcntl.h>      /* Micros for I/O */
#include <sys/types.h>  /* System data type */
#include <sys/stat.h>   /* File system status */

#define BUF_LEN 1024
GtkWidget *clist;//
GtkWidget *entry;//用于输入
GtkWidget *popup_window; //用于新建窗口
GtkWidget *popuplabel;


gint process_num = 0; //目前进程的数量

char* now_pid = NULL;//记录目前查看的进程pid

#endif