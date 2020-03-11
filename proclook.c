/* * MainWin.c * SystemMonitor * Created on: 2012-2-24 * Author: zhushengben */
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <cairo.h>
#include <time.h>

#define MAX 1024

#include "searchInfor.h"
#include "cpu.h"
#include "memory.h"
#include "module.h"
#include "process.h"
#include "menuBar.h"
#include "progressBar.h"
#include "showAbout.h"
#include "refresh.h"
#include "showImage.h"
int main(int argc, char **argv)
{
    GtkWidget *window;
    GtkWidget *main_vbox;
    GtkWidget *tooltip;

    gtk_init(&argc, &argv);
    /* 主窗口 */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    tooltip = gtk_tooltips_new();
    gtk_tooltips_set_tip(tooltip, window, "系统监视器\n此软件归华中科技大学所有\n未经授权不可擅自更改", NULL);
    gtk_window_set_title(GTK_WINDOW(window), "System Moniter");
    gtk_window_set_opacity(GTK_WINDOW(window), 0.95);
    // 设置透明度函数
    //update_widget_bg(window, BACK_IMAGE);
    /* 默认窗口大小 */
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    /* 窗口初始位置在屏幕最中央 */
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    /* 显示窗口 */ gtk_widget_show(window);
    /* 创建一个纵向盒 */
    main_vbox = gtk_vbox_new(FALSE, 10);
    /* 设定这个容器和周围的间距 */
    gtk_container_set_border_width(GTK_CONTAINER(main_vbox), 5);
    /* 将这个布局容器添加到整个视窗的容器中 */
    gtk_container_add(GTK_CONTAINER(window), main_vbox);
    /* 显示该盒 */ gtk_widget_show(main_vbox);
    createMenuBar(main_vbox); /* 建立多标签页notebook */
    GtkWidget *notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos((GtkNotebook *)notebook, GTK_POS_BOTTOM);
    gtk_notebook_set_show_border((GtkNotebook *)notebook, FALSE);  /* 在纵向盒顶部添加该笔记本 */
    gtk_box_pack_end(GTK_BOX(main_vbox), notebook, TRUE, TRUE, 0); /* 显示该笔记本 */
    gtk_widget_show(notebook);                                     /* 新建第一个标签页到notebook，用的是frame框架 */
    createCPUPage(notebook);                                       /* 新建第三个标签页到notebook */
    createModPage(notebook);                                       /* 新建第四个标签页到notebook */
    createProPage(notebook); /* 新建第五个标签页到notebook */      /* 原来是mem模块在死循环中 */
    createMemPage(notebook);                                       /* 新建第二个标签页到notebook */
    createAboutPage(notebook);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 0); /**************************************************/
    gtk_timeout_add(400, refresh, pdata);
    gtk_main();
    return 0;
}
/* * cpu.h * helloWorld * Created on: 2012-2-19 * Author: zhushengben */
#ifndef CPU_H_
#define CPU_H_            \
    GtkWidget *window;    \
    GtkWidget *main_vbox; \
    GtkWidget *clist; //进程时候需要的列表
GtkWidget *clist2;    //模块读取时需要的列表
GtkWidget *cpu_draw_area;
GdkPixmap *cpu_graph;
GtkWidget *pbar_cpu;
/* 全局变量 */
static gint cpuPoints[100];
static gfloat cpu_rate = 0.0;
static char PID[20];
static int selectedRow;
float zuser = 0, ztotal = 0;
typedef struct CPUinfo
{
    char modeName[50];  //line 5
    char cpuMHz[10];    //line 7
    char cacheSize[10]; //line 8
    char cpuCores[10];  //line 12
    char addrSize[50];  //line 30
} * cpuInfo;
static gboolean cpu_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    gdk_draw_drawable(widget->window, widget->style->fg_gc[GTK_WIDGET_STATE(widget)], cpu_graph, event->area.x, event->area.y, event->area.x, event->area.y, event->area.width, event->area.height);
    return FALSE;
}
static gboolean cpu_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
    if (cpu_graph)
    {
        g_object_unref(cpu_graph);
    }
    cpu_graph = gdk_pixmap_new(widget->window, widget->allocation.width, widget->allocation.height, -1);
    gdk_draw_rectangle(cpu_graph, widget->style->white_gc, TRUE, 0, 0, widget->allocation.width, widget->allocation.height);
    return TRUE;
} /* 取得cpu利用率 */
float getCpuUseRatio()
{
    FILE *fp;
    char buffer[1024];
    size_t buf;
    float useRatio; /* 分别为用户模式，低优先级用户模式，内核模式，空闲的处理器时间 */
    float user = 0, nice = 0, sys = 0, idle = 0, iowait = 0;
    fp = fopen("/proc/stat", "r"); /* 从fp中读取sizeof(buffer)块数据放入buffer，每次读一块 */
    buf = fread(buffer, 1, sizeof(buffer), fp);
    fclose(fp);
    if (buf == 0)
        return 0;
    buffer[buf] == '\0';
    sscanf(buffer, "cpu %f %f %f %f %f", &user, &nice, &sys, &idle, &iowait);
    if (idle <= 0)
        idle = 0;
    useRatio = 100 * (user - zuser) / (user + nice + sys + idle - ztotal);
    if (useRatio > 100)
        useRatio = 100;
    ztotal = user + nice + sys + idle;
    zuser = user;
    cpu_rate = useRatio;
    return useRatio;
} /* cpu标签页，显示cpu信息 */
void createCPUPage(GtkWidget *notebook)
{
    GtkWidget *label;
    GtkWidget *vbox;
    GtkWidget *frame1 = gtk_frame_new("");
    gtk_container_set_border_width(GTK_CONTAINER(frame1), 10);
    gtk_widget_set_size_request(frame1, 120, 500);
    gtk_widget_show(frame1);                                                            /* 将该标签页加入到notebook中 */
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), frame1, gtk_label_new("CPU信息")); /* 在该页建议个表格，分为上下2行，一列 */
    GtkWidget *table1 = gtk_table_new(10, 10, TRUE);                                    /* 将这个表添加到“cpu使用率“页 */
    gtk_container_add(GTK_CONTAINER(frame1), table1);                                   /* 新建一个frame框架用于显示图表 */
    GtkWidget *CPU_frame1 = gtk_frame_new("CPU使用历史曲线"); /* 放到表格第一行 */      /* 这个函数的参数不懂，不好调整 */
    gtk_table_attach_defaults(GTK_TABLE(table1), CPU_frame1, 0, 8, 0, 5);
    gtk_widget_show(CPU_frame1);                                    /* 现在,我们开始向屏幕绘图。 * 我们使用的构件是绘图区构件。 *一个绘图区构件本质上是一个 X 窗口,没有其它的东西。 *它是一个空白的画布,我们可以在其上绘制需要的东西。*/
    cpu_draw_area = gtk_drawing_area_new();                         /* 设置可以画图 */
    gtk_widget_set_app_paintable(cpu_draw_area, TRUE);              /* 画图区默认大小 */
    gtk_drawing_area_size(GTK_DRAWING_AREA(cpu_draw_area), 40, 80); /* 画布添加到cpu曲线框架中 */
    gtk_container_add(GTK_CONTAINER(CPU_frame1), cpu_draw_area);
    gtk_widget_show(cpu_draw_area); /* 关联回调函数 */ /* 关于各种事件不甚了解 */
    g_signal_connect(cpu_draw_area, "expose_event", G_CALLBACK(cpu_expose_event), NULL);
    g_signal_connect(cpu_draw_area, "configure_event", G_CALLBACK(cpu_configure_event), NULL);
    gtk_widget_show(cpu_draw_area);
    int i;
    for (i = 0; i < 100; i++)
    { /* 要取[a,b)之间的随机整数（包括a，但不包括b)，使用： * (rand() % (b - a)) + a */
        cpuPoints[i] = (rand() % 30 + 180);
    } // 此处出现问题 /* 不断刷新画图 */
    // gtk_timeout_add(100, (GtkFunction) drawGraph, NULL);
    /* 取得CPU信息 */
    char modeName[50], cpuMHz[20], cacheSize[20], cpuCores[20], addrSize[50];
    char cpuBuffer[1000];
    //getCPUInfo("/proc/cpuinfo", modeName, cacheSize); //cpuMHz,cacheSize,cpuCores,addrSize); /* 不能用\t !! */
    getInforNotd("/proc/cpuinfo", "model name", modeName);
    getInforNotd("/proc/cpuinfo", "cache size", cacheSize);
    getInforNotd("/proc/cpuinfo", "cpu MHz", cpuMHz);
    getInforNotd("/proc/cpuinfo", "cpu cores", cpuCores);
    getInforNotd("/proc/cpuinfo", "address sizes", addrSize); /* 建一个frame在表一下半部显示cpu信息 */
    GtkWidget *frame_down = gtk_frame_new("系统信息 ");
    gtk_table_attach_defaults(GTK_TABLE(table1), frame_down, 0, 10, 5, 10);
    gtk_widget_show(frame_down);                              /* 这里必须要show表，为什么？不知到该在什么时候show什么 */
    gtk_widget_show(table1);                                  /* 在该狂建建一个表格，将下半部分为1行，2列 */
    GtkWidget *table_down = gtk_table_new(1, 2, TRUE);        /* 将这个表添加到下面框架 */
    gtk_container_add(GTK_CONTAINER(frame_down), table_down); /* 建一个frame在表二左半部显示硬件信息 */
    GtkWidget *frame_left_down = gtk_frame_new("硬件");
    gtk_table_attach_defaults(GTK_TABLE(table_down), frame_left_down, 0, 1, 0, 1);
    gtk_widget_show(frame_left_down); /* 这里必须要show表，为什么？不知到该在什么时候show什么 */
    gtk_widget_show(table_down);      /* 容器 */
    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_container_add(GTK_CONTAINER(frame_left_down), vbox);
    gtk_widget_show(vbox); /* 建一个标签显示cpu型号 */
    strcpy(cpuBuffer, "CPU型号和主频:\n");
    strcat(cpuBuffer, modeName); /* 这里不该有？ * gtk_widget_show(label); * 原来是因为我们有清除cpuBuffer */ /* 建一个标签显示cache大小 */
    strcat(cpuBuffer, "\n\nCache 大小:");
    strcat(cpuBuffer, cacheSize); /* 建一个标签显示cpu主频 */
    strcat(cpuBuffer, "\n\ncpu 主频:");
    strcat(cpuBuffer, cpuMHz);
    strcat(cpuBuffer, " MHz"); /* 建一个标签显示cpu核数 */
    strcat(cpuBuffer, "\n\ncpu核数:");
    strcat(cpuBuffer, cpuCores);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5); /* 建一个标签显示addrSize */
    strcat(cpuBuffer, "\n\n寻址位数:");
    strcat(cpuBuffer, addrSize);
    label = gtk_label_new(cpuBuffer);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
    gtk_widget_show(label); /* 建一个frame在表二右半部显示系统版本信号 */
    GtkWidget *frame_right_down = gtk_frame_new("Ubuntu");
    GdkColor color;
    PangoFontDescription *font; /* 设置颜色 */
    gdk_color_parse("blue", &color);
    gtk_widget_modify_fg(frame_right_down, GTK_STATE_NORMAL, &color); /* 设置字体 */
    font = pango_font_description_from_string("San");
    pango_font_description_set_size(font, 30 * PANGO_SCALE);
    gtk_widget_modify_font(frame_right_down, font);
    gtk_table_attach_defaults(GTK_TABLE(table_down), frame_right_down, 1, 2, 0, 1);
    gtk_widget_show(frame_right_down); /* 这里必须要show表，为什么？不知到该在什么时候show什么 */
    gtk_widget_show(table_down);       /* 容器 */
    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_container_add(GTK_CONTAINER(frame_right_down), vbox);
    gtk_widget_show(vbox); /* 建一个标签显示cpu型号 */
    memset(cpuBuffer, 0, 1000);
    memset(addrSize, 0, 50);
    getSysVersion("/proc/version", "version", addrSize);
    strcat(cpuBuffer, "\n\n内核版本 : ");
    strcat(cpuBuffer, addrSize);
    strcat(cpuBuffer, "\n\n系统版本 10.10 (maverick)");
    strcat(cpuBuffer, "\n\nGNOME 2.32.0");
    GtkWidget *label1 = gtk_label_new(cpuBuffer);
    gtk_box_pack_start(GTK_BOX(vbox), label1, FALSE, FALSE, 5);
    gtk_widget_show(label1); /* 新建一个frame框架用于显示进度条*/
    GtkWidget *frame_cpu = gtk_frame_new("cpu使用率");
    gtk_table_attach_defaults(GTK_TABLE(table1), frame_cpu, 8, 10, 0, 5);
    gtk_widget_show(frame_cpu); /* 创建第一个进度条 * GTK_PROGRESS_BOTTOM_TO_TOP */
    pbar_cpu = gtk_progress_bar_new();
    gtk_container_add(GTK_CONTAINER(frame_cpu), pbar_cpu);
    gtk_progress_bar_set_orientation(pbar_cpu, GTK_PROGRESS_BOTTOM_TO_TOP);
    gtk_widget_show(pbar_cpu); /* 容器 放在右下角 */
    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_table_attach_defaults(GTK_TABLE(table1), vbox, 6, 10, 7, 10);
    gtk_widget_show(vbox);
    gtk_widget_show(table1); /* 创建一个图像 */
    GtkWidget *image = gtk_image_new_from_file("4.png");
    gtk_box_pack_start(GTK_BOX(vbox), image, FALSE, FALSE, 3);
    gtk_widget_show(image); /* 容器 放在右下角 */
    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_table_attach_defaults(GTK_TABLE(table1), vbox, 0, 5, 7, 10);
    gtk_widget_show(vbox);
    gtk_widget_show(table1); /* 创建一个图像 */
    image = gtk_image_new_from_file("1.png");
    gtk_box_pack_start(GTK_BOX(vbox), image, FALSE, FALSE, 3);
    gtk_widget_show(image);
}
#endif /* CPU_H_ */
/* * memory.h * systemMonitor * Created on: 2012-2-19 * Author: zhushengben */
#ifndef MEMORY_H_
#define MEMORY_H_                                                                                                                                               \
    typedef struct _ProgressData                                                                                                                                \
    {                                                                                                                                                           \
        GtkWidget *frame1;                                                                                                                                      \
        GtkWidget *pbar;                                                                                                                                        \
        GtkWidget *pbar1;                                                                                                                                       \
        GtkWidget *pbar2;                                                                                                                                       \
        GtkWidget *pbar3;                                                                                                                                       \
        gboolean activity_mode;                                                                                                                                 \
    } ProgressData;                                                                                                                                             \
    ProgressData *pdata_mem;                                                                                                                                    \
    GtkWidget *label_memSize;                                                                                                                                   \
    GtkWidget *label_freeSize;                                                                                                                                  \
    GtkWidget *label_cacheSize;                                                                                                                                 \
    GtkWidget *label_cacheUse;                                                                                                                                  \
    gdouble memUseRatio, swapUse;                                                                                                                               \
    char totalMem[20], freeMem[20], SwapTotal[20], SwapFree[20];                                                                                                \
    char Buffers[20], Cached[20], buffer[40]; /* 更新进度条,这样就能够看到进度条的移动 */                                                     \
    gint progress_timeout(gpointer data);     /* MEMERY标签页，显示内存信息 */                                                                        \
    void createMemPage(GtkWidget *notebook)                                                                                                                     \
    {                                                                                                                                                           \
        GtkWidget *align;                                                                                                                                       \
        GtkWidget *table;                                                                                                                                       \
        GtkWidget *vbox;                                                                                                                                        \
        GtkWidget *image;                                                                                                                                       \
        GdkColor color;                                                                                                                                         \
        PangoFontDescription *font;                                                                                                                             \
        memUseRatio = 0;                                                                                                                                        \
        memset(buffer, 0, 40); /* 为传递到回调函数中的数据分配内存 */                                                                           \
        pdata_mem = g_malloc(sizeof(ProgressData));                                                                                                             \
        pdata_mem->frame1 = gtk_frame_new("");                                                                                                                  \
        gtk_container_set_border_width(GTK_CONTAINER(pdata_mem->frame1), 0);                                                                                    \
        gtk_widget_set_size_request(pdata_mem->frame1, 120, 500);                                                                                               \
        gtk_widget_show(pdata_mem->frame1);                                                             /* 将该标签页加入到notebook中 */               \
        gtk_notebook_append_page(GTK_NOTEBOOK(notebook), pdata_mem->frame1, gtk_label_new("内存信息")); /* 建一个表格，将下半部分为10行，10列 */ \
        table = gtk_table_new(10, 10, TRUE);                                                            /* 将这个表添加到框架 */                       \
        gtk_container_add(GTK_CONTAINER(pdata_mem->frame1), table); /* 创建进度条 */                    /* 新建一个frame框架用于显示进度条*/       \
        GtkWidget *frame_cpu = gtk_frame_new("内存");                                                                                                           \
        gtk_table_attach_defaults(GTK_TABLE(table), frame_cpu, 0, 5, 0, 3);                                                                                     \
        gtk_widget_show(frame_cpu); /* 容器 */                                                                                                                \
        vbox = gtk_vbox_new(FALSE, 2);                                                                                                                          \
        gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);                                                                                                \
        gtk_container_add(GTK_CONTAINER(frame_cpu), vbox);                                                                                                      \
        gtk_widget_show(vbox);                                                                                                                                  \
        gtk_widget_show(table); /* 创建一个居中对齐的对象 */                                                                                         \
        align = gtk_alignment_new(0.5, 0.5, 0.8, 0);                                                                                                            \
        gtk_box_pack_start(GTK_BOX(vbox), align, FALSE, FALSE, 5);                                                                                              \
        gtk_widget_show(align); /* 创建第一个进度条 * GTK_PROGRESS_BOTTOM_TO_TOP */                                                                     \
        pdata_mem->pbar = gtk_progress_bar_new();                                                                                                               \
        gtk_container_add(GTK_CONTAINER(align), pdata_mem->pbar);                                                                                               \
        gtk_progress_bar_set_orientation(pdata_mem->pbar, GTK_PROGRESS_LEFT_TO_RIGHT);                                                                          \
        gtk_widget_show(pdata_mem->pbar); /* 创建一个居中对齐的对象 */                                                                               \
        align = gtk_alignment_new(0.5, 0.5, 0.8, 0);                                                                                                            \
        gtk_box_pack_start(GTK_BOX(vbox), align, FALSE, FALSE, 5);                                                                                              \
        gtk_widget_show(align); /* 创建第二个进度条 */                                                                                                  \
        pdata_mem->pbar1 = gtk_progress_bar_new();                                                                                                              \
        gtk_container_add(GTK_CONTAINER(align), pdata_mem->pbar1);
// toggle_show_text("内存使用率", pdata_mem);
toggle_activity_mode(FALSE, pdata_mem);
gtk_widget_show(pdata_mem->pbar1);
/* 新建一个frame框架用于显示进度条*/
frame_cpu = gtk_frame_new("交换区");
gtk_table_attach_defaults(GTK_TABLE(table), frame_cpu, 5, 10, 0, 3);
gtk_widget_show(frame_cpu);
vbox = gtk_vbox_new(FALSE, 2);
gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
gtk_container_add(GTK_CONTAINER(frame_cpu), vbox);
gtk_widget_show(vbox);
gtk_widget_show(table); /* 创建一个居中对齐的对象 */
align = gtk_alignment_new(0.5, 0.5, 0.8, 0);
gtk_box_pack_start(GTK_BOX(vbox), align, FALSE, FALSE, 5);
gtk_widget_show(align); /* 创建第三个进度条 */
pdata_mem->pbar2 = gtk_progress_bar_new();
gtk_container_add(GTK_CONTAINER(align), pdata_mem->pbar2);
toggle_show_text("内存交换区", pdata_mem);
toggle_activity_mode(FALSE, pdata_mem);
gtk_widget_show(pdata_mem->pbar2); /* 创建一个居中对齐的对象 */
align = gtk_alignment_new(0.5, 0.5, 0.8, 0);
gtk_box_pack_start(GTK_BOX(vbox), align, FALSE, FALSE, 5);
gtk_widget_show(align); /* 创建第四个进度条 */
pdata_mem->pbar3 = gtk_progress_bar_new();
gtk_container_add(GTK_CONTAINER(align), pdata_mem->pbar3);
toggle_show_text("交换区使用量", pdata_mem);
toggle_activity_mode(FALSE, pdata_mem);
gtk_widget_show(pdata_mem->pbar3); /* 加上这一行后，出现问题 * faile to NULL to ProgressBar * 关于timer的使用不明白 */
/* 不断刷新画图 */
//gtk_timeout_add(100, (GtkFunction) progress_timeout, pdata_mem);
progress_timeout(pdata_mem); /*************************************/ /* 容器 放在左下角 */
vbox = gtk_vbox_new(FALSE, 5);
gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
gtk_table_attach_defaults(GTK_TABLE(table), vbox, 0, 6, 4, 10);
gtk_widget_show(vbox);
gtk_widget_show(table);
/* 创建一个居中对齐的对象 */
align = gtk_alignment_new(0.5, 0.5, 0, 0);
gtk_box_pack_start(GTK_BOX(vbox), align, FALSE, FALSE, 5);
gtk_widget_show(align); /* 建一个标签显示内存大小 */
label_memSize = gtk_label_new(buffer);
gtk_container_add(GTK_CONTAINER(align), label_memSize);
font = pango_font_description_from_string("San");
pango_font_description_set_size(font, 15 * PANGO_SCALE);
gtk_widget_modify_font(label_memSize, font);             /* 设置控件可容字数从而设置其大小 */
gtk_label_set_width_chars(GTK_LABEL(label_memSize), 20); /* 设置控件对齐方式 * 第一个参数是lblTest,第二个参数是左右方向的对齐值，第三个参数是上下方向的对齐值; *对齐值的取值范围为0-1.取0时，为左对齐,取1时,为右对齐,取0.5时，为中间对齐. */
gtk_misc_set_alignment(GTK_MISC(label_memSize), 0, 0.5);
gtk_widget_show(label_memSize); /****************************************************/ /* 创建一个居中对齐的对象 */
align = gtk_alignment_new(0.5, 0.5, 0, 0);
gtk_box_pack_start(GTK_BOX(vbox), align, FALSE, FALSE, 5);
gtk_widget_show(align); /* 建一个标签显示空闲区大小 */
label_freeSize = gtk_label_new(buffer);
gtk_container_add(GTK_CONTAINER(align), label_freeSize); /* 设置颜色 */
gdk_color_parse("red", &color);
gtk_widget_modify_fg(label_freeSize, GTK_STATE_NORMAL, &color); /* 设置字体 */
font = pango_font_description_from_string("San");
pango_font_description_set_size(font, 15 * PANGO_SCALE);
gtk_widget_modify_font(label_freeSize, font);             /* 设置控件可容字数从而设置其大小 */
gtk_label_set_width_chars(GTK_LABEL(label_freeSize), 20); /* 设置控件对齐方式 * 第一个参数是lblTest,第二个参数是左右方向的对齐值，第三个参数是上下方向的对齐值; *对齐值的取值范围为0-1.取0时，为左对齐,取1时,为右对齐,取0.5时，为中间对齐. */
gtk_misc_set_alignment(GTK_MISC(label_freeSize), 0, 0.5);
gtk_widget_show(label_freeSize); /****************************************************/ /* 创建一个居中对齐的对象 */
align = gtk_alignment_new(0.5, 0.5, 0, 0);
gtk_box_pack_start(GTK_BOX(vbox), align, FALSE, FALSE, 5);
gtk_widget_show(align); /* 建一个标签显示cache大小 */
label_cacheSize = gtk_label_new(buffer);
gtk_container_add(GTK_CONTAINER(align), label_cacheSize); /* 设置颜色 */
gdk_color_parse("black", &color);
gtk_widget_modify_fg(label_cacheSize, GTK_STATE_NORMAL, &color); /* 设置字体 */
font = pango_font_description_from_string("San");
pango_font_description_set_size(font, 15 * PANGO_SCALE);
gtk_widget_modify_font(label_cacheSize, font);             /* 设置控件可容字数从而设置其大小 */
gtk_label_set_width_chars(GTK_LABEL(label_cacheSize), 20); /* 设置控件对齐方式 * 第一个参数是lblTest,第二个参数是左右方向的对齐值，第三个参数是上下方向的对齐值; *对齐值的取值范围为0-1.取0时，为左对齐,取1时,为右对齐,取0.5时，为中间对齐. */
gtk_misc_set_alignment(GTK_MISC(label_cacheSize), 0, 0.5);
gtk_widget_show(label_cacheSize); /****************************************************/ /* 创建一个居中对齐的对象 */
align = gtk_alignment_new(0.5, 0.5, 0, 0);
gtk_box_pack_start(GTK_BOX(vbox), align, FALSE, FALSE, 5);
gtk_widget_show(align); /* 建一个标签显示空闲区大小 */
label_cacheUse = gtk_label_new(buffer);
gtk_container_add(GTK_CONTAINER(align), label_cacheUse); /* 设置颜色 */
gdk_color_parse("black", &color);
gtk_widget_modify_fg(label_cacheUse, GTK_STATE_NORMAL, &color); /* 设置字体 */
font = pango_font_description_from_string("San");
pango_font_description_set_size(font, 15 * PANGO_SCALE);
gtk_widget_modify_font(label_cacheUse, font);             /* 设置控件可容字数从而设置其大小 */
gtk_label_set_width_chars(GTK_LABEL(label_cacheUse), 20); /* 设置控件对齐方式 * 第一个参数是lblTest,第二个参数是左右方向的对齐值，第三个参数是上下方向的对齐值; *对齐值的取值范围为0-1.取0时，为左对齐,取1时,为右对齐,取0.5时，为中间对齐. */
gtk_misc_set_alignment(GTK_MISC(label_cacheUse), 0, 0.5);
gtk_widget_show(label_cacheUse); /**********************************************/ /* 容器 放在中间角角 */
vbox = gtk_vbox_new(FALSE, 5);
gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
gtk_table_attach_defaults(GTK_TABLE(table), vbox, 0, 10, 3, 8);
gtk_widget_show(vbox);
gtk_widget_show(table); /* 新建一个frame框架用于显示进度条*/
frame_cpu = gtk_frame_new("");
gtk_table_attach_defaults(GTK_TABLE(table), frame_cpu, 0, 10, 3, 4);
gtk_widget_show(frame_cpu);   /* 创建一个进度条 */
createProgressBar(frame_cpu); /* 容器 放在右下角 */
vbox = gtk_vbox_new(FALSE, 5);
gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
gtk_table_attach_defaults(GTK_TABLE(table), vbox, 4, 10, 5, 10);
gtk_widget_show(vbox);
gtk_widget_show(table); /* 创建一个图像 */
image = gtk_image_new_from_file("penguin.gif");
gtk_box_pack_start(GTK_BOX(vbox), image, FALSE, FALSE, 3);
gtk_widget_show(image); /* 容器 放在右下角 */
vbox = gtk_vbox_new(FALSE, 5);
gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
gtk_table_attach_defaults(GTK_TABLE(table), vbox, 3, 7, 8, 10);
gtk_widget_show(vbox);
gtk_widget_show(table); /* 创建一个图像 */
image = gtk_image_new_from_file("5.png");
gtk_box_pack_start(GTK_BOX(vbox), image, FALSE, FALSE, 3);
gtk_widget_show(image); /* 容器 放在右下角 */
vbox = gtk_vbox_new(FALSE, 5);
gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
gtk_table_attach_defaults(GTK_TABLE(table), vbox, 0, 2, 4, 6);
gtk_widget_show(vbox);
gtk_widget_show(table); /* 创建一个图像 */
image = gtk_image_new_from_file("a.png");
gtk_box_pack_start(GTK_BOX(vbox), image, FALSE, FALSE, 3);
gtk_widget_show(image);
} /* 更新进度条,这样就能够看到进度条的移动 */
gint progress_timeout(gpointer data)
{
    if (pdata_mem->activity_mode)
        gtk_progress_bar_pulse(GTK_PROGRESS_BAR(pdata_mem->pbar));
    else
    {
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pdata_mem->pbar), memUseRatio);
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pdata_mem->pbar1), 1 - memUseRatio);
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pdata_mem->pbar2), swapUse);
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pdata_mem->pbar3), 1 - swapUse);
    }
    return TRUE;
} /* 设置进度条的滑槽上的文本显示 */
void toggle_show_text(const gchar *text, ProgressData *pdata_mem) { gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pdata_mem->pbar), "内存使用率"); } /* 设置进度条的活动模式 */
void toggle_activity_mode(gboolean activity_mode, ProgressData *pdata_mem) { pdata_mem->activity_mode = activity_mode; }                          /* 回调函数,切换进度条的移动方向 */
void toggle_orientation(GtkWidget *widget, ProgressData *pdata_mem)
{
    switch (gtk_progress_bar_get_orientation(GTK_PROGRESS_BAR(pdata_mem->pbar)))
    {
    case GTK_PROGRESS_LEFT_TO_RIGHT:
        gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pdata_mem->pbar), GTK_PROGRESS_RIGHT_TO_LEFT);
        break;
    case GTK_PROGRESS_RIGHT_TO_LEFT:
        gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(pdata_mem->pbar), GTK_PROGRESS_LEFT_TO_RIGHT);
        break;
    default:; // 什么也不做
    }
}
/* 清除分配的内存,删除定时器(timer) */
void destroy_progress(GtkWidget *widget, ProgressData *pdata_mem)
{
    pdata_mem->frame1 = NULL;
    pdata_mem->pbar = NULL;
    pdata_mem->pbar1 = NULL;
    pdata_mem->pbar2 = NULL;
    pdata_mem->pbar3 = NULL;
    g_free(pdata_mem);
    gtk_main_quit();
}
void set_label_mem_text(void)
{
    getInfor("/proc/meminfo", "MemTotal", totalMem);
    getInfor("/proc/meminfo", "MemFree", freeMem);
    getInfor("/proc/meminfo", "SwapTotal", SwapTotal);
    getInfor("/proc/meminfo", "SwapFree", SwapFree);
    float total_mem = (float)atoi(totalMem);
    float free_mem = (float)atoi(freeMem);
    memUseRatio = 1 - free_mem / total_mem;
    float total_swap = (float)atoi(SwapTotal);
    float free_swap = (float)atoi(SwapFree);
    swapUse = 1 - free_swap / total_swap;
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pdata_mem->pbar), memUseRatio);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pdata_mem->pbar1), 1 - memUseRatio);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pdata_mem->pbar2), swapUse);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pdata_mem->pbar3), 1 - swapUse);
    char p[20] = "内存使用率(", buf[10];
    gcvt(memUseRatio * 100, 4, buf);
    strcat(buf, "%)");
    strcat(p, buf);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pdata_mem->pbar), p);
    char p1[20] = "内存剩余量(", buf1[10];
    gcvt(100 - memUseRatio * 100, 4, buf1);
    strcat(buf1, "%)");
    strcat(p1, buf1);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pdata_mem->pbar1), p1);
    char p2[20] = "交换区使用率(", buf2[10];
    gcvt(swapUse * 100, 4, buf2);
    strcat(buf2, "%)");
    strcat(p2, buf2);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pdata_mem->pbar2), p2);
    char p3[20] = "交换区剩余量(", buf3[10];
    gcvt(100 - swapUse * 100, 4, buf3);
    strcat(buf3, "%)");
    strcat(p3, buf3);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pdata_mem->pbar3), p3);
    char buffer[100] = "";
    strcat(buffer, "\n\n内存大小 : ");
    strcat(buffer, totalMem);
    strcat(buffer, " MB");
    gtk_label_set_text((GtkLabel *)label_memSize, buffer);
    memset(buffer, 0, 40);
    strcat(buffer, "\n空闲区大小 : ");
    strcat(buffer, freeMem);
    strcat(buffer, " MB");
    gtk_label_set_text((GtkLabel *)label_freeSize, buffer);
    getInfor("/proc/meminfo", "Buffers", Buffers);
    memset(buffer, 0, 40);
    strcat(buffer, "\n缓存大小 : ");
    strcat(buffer, Buffers);
    strcat(buffer, " MB");
    gtk_label_set_text((GtkLabel *)label_cacheSize, buffer);
    getInfor("/proc/meminfo", "Cached", Cached);
    memset(buffer, 0, 40);
    strcat(buffer, "\nCached大小 : ");
    strcat(buffer, Cached);
    strcat(buffer, " MB");
    gtk_label_set_text((GtkLabel *)label_cacheUse, buffer);
}
#endif /* MEMORY_H_ */
/* * module.h * helloWorld * Created on: 2012-2-19 * Author: zhushengben */
#ifndef MODULE_H_
#define MODULE_H_ GtkWidget *clist_modul;
void getModInfo(char store[], int i, char modName[], char memUse[], char times[]);
void createModPage(GtkWidget *notebook)
{
    GtkWidget *frame_up, *frame_down, *table, *align, *button;
    GtkWidget *frame = gtk_frame_new("模块信息表");
    gtk_container_set_border_width(GTK_CONTAINER(frame), 10);
    gtk_widget_set_size_request(frame, 100, 355);
    gtk_widget_show(frame);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), frame, gtk_label_new("模块信息"));
    /* 建一个表格，分为上下两部分 */
    table = gtk_table_new(10, 4, TRUE);
    /* 将这个表添加到框架 */
    gtk_container_add(GTK_CONTAINER(frame), table);
    /* 建一个frame在表上边*/
    frame_down = gtk_frame_new("*0(^_^)0*");
    gtk_table_attach_defaults(GTK_TABLE(table), frame_down, 0, 4, 0, 10);
    gtk_widget_show(frame_down);
    gtk_widget_show(table);
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    gtk_container_add(GTK_CONTAINER(frame_down), scrolled_window);
    gtk_widget_show(scrolled_window);                      /* 老是不能显示，经测试直到这里都可以正确实现 */
    gchar *titles[3] = {"模块名", "占用内存", "加载次数"}; //由于含有特殊情况，所以只读取通//用格式的范例
    clist_modul = gtk_clist_new_with_titles(3, titles);
    gtk_clist_set_shadow_type(GTK_CLIST(clist_modul), GTK_SHADOW_IN);
    gtk_clist_set_column_width(GTK_CLIST(clist_modul), 0, 270);
    gtk_clist_set_column_width(GTK_CLIST(clist_modul), 1, 270);
    gtk_clist_set_column_width(GTK_CLIST(clist_modul), 2, 270);
    gtk_container_add(GTK_CONTAINER(scrolled_window), clist_modul);
    set_modul_info();
    gtk_widget_show(clist_modul);
}
void set_modul_info()
{
    /* 原来是这个函数中出现某种问题， * 删除这个函数后可以出现所有控件 * 于是我干脆把那个
    函数直接整合到这个函数中来 * 结果可以运行来！ * 我认为可能是将控件作为函数参数传递出的问题 */
    /* 读取当前进程信息，并显示到列表框中 */
    //add_modules_view(clist_modul);
    /* 原来用的是5000，出现问题，于是变小，可以 */
    char infoBuf[2000]; //暂存读取的modules文件内容
    int fd = open("/proc/modules", O_RDONLY);
    read(fd, infoBuf, sizeof(infoBuf));
    close(fd);
    unsigned int lines = 0;
    unsigned int i = 0;
    gtk_clist_clear(clist_modul); /* 先取得行数 */
    while (i != sizeof(infoBuf) / sizeof(char))
    {
        if (infoBuf[i] == '\n')
            lines++;
        i++;
    }
    i = 0;
    for (i = 1; i <= lines; i++)
    {
        char convert_mem[25];
        char modName[25]; //模块名
        char memUse[20];  //内存量
        char times[5];    //使用次数
        int mem_num;
        float real_mem;
        getModInfo(infoBuf, i, modName, memUse, times);
        mem_num = atoi(memUse);
        real_mem = (float)mem_num / (1024);
        gcvt(real_mem, 3, convert_mem);
        gchar *list[1][3] = {{modName, convert_mem, times}};
        gtk_clist_append((GtkCList *)clist_modul, list[0]);
        gtk_clist_thaw((GtkCList *)clist_modul); //更新list列表显示
    }
}
void getModInfo(char store[], int i, char modName[], char memUse[], char times[])
{
    int j = 0;
    int cflags = 0; //记录读取的回车键个数以便判断行数
    int k = 0;
    char name2[25];
    char mem2[20];
    char times2[5];
    while (cflags < i - 1)
    {
        if (store[j] == '\n')
            cflags++; //回车数加1
        j++;
    }
    while (store[j] != ' ')
    {
        //读取进程名
        name2[k++] = store[j];
        j++;
    }
    name2[k] = '\0';
    j++; //跳转到下一个不是空格的地方
    k = 0;
    while (store[j] != ' ')
    {
        mem2[k++] = store[j];
        j++;
    }
    mem2[k] = '\0'; //封口
    j++;
    times2[0] = store[j]; //读取模块的使用次数
    times2[1] = '\0';     //封口
    strcpy(modName, name2);
    strcpy(memUse, mem2);
    strcpy(times, times2);
}
void add_modul(void)
{
    system("insmod ./devDrv.ko");
    set_modul_info();
}
void remove_modul(void)
{
    system("rmmod devDrv.");
    set_modul_info();
}
#endif /* MODULE_H_ */
/* * process.h * helloWorld * Created on: 2012-2-19 * Author: zhushengben */
#ifndef PROCESS_H_
#define PROCESS_H_ GtkWidget *clist;
GtkWidget *label_proc;
GtkWidget *label_time;                           /* 刷新信息 */
void set_proc_info();                            /* 表示读取第i个空格之前的一个字符串(此即获得的数据)) */
void read_info(char store[], int i, char get[]); /* 用户选中某一行时的回调函数*/
void selection_made(GtkWidget *clist, gint row, gint column, GdkEventButton *event, gpointer data);
void createProPage(GtkWidget *notebook)
{
    char buffer[100];
    GtkWidget *table;
    GtkWidget *frame_up;
    GtkWidget *frame_down;
    GtkWidget *vbox;
    GtkWidget *font;
    GtkWidget *tooltip;
    GdkColor color;
    GtkWidget *frame = gtk_frame_new("");
    gtk_container_set_border_width(GTK_CONTAINER(frame), 10);
    gtk_widget_set_size_request(frame, 100, 355);
    gtk_widget_show(frame);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), frame, gtk_label_new("进程信息")); /* 建一个表格，分为上下两部分 */
    table = gtk_table_new(10, 4, TRUE);                                                 /* 将这个表添加到框架 */
    gtk_container_add(GTK_CONTAINER(frame), table);                                     /* 建一个frame在表左下边显示标签信息 */
    frame_up = gtk_frame_new("*0(^_^)0*");
    gtk_table_attach_defaults(GTK_TABLE(table), frame_up, 0, 2, 7, 10);
    gtk_widget_show(frame_up);
    gtk_widget_show(table); /* 建一个标签显示进程数等(用button代替) */
    label_proc = gtk_button_new_with_label("good");
    gtk_container_add(GTK_CONTAINER(frame_up), label_proc); /* 连接刷新 */
    g_signal_connect(G_OBJECT(label_proc), "clicked", G_CALLBACK(set_proc_info), NULL);
    tooltip = gtk_tooltips_new();
    gtk_tooltips_set_tip(tooltip, label_proc, "点击刷新", NULL);
    gtk_widget_show(label_proc); /* 建一个frame在表右下边显示标签信息 */
    frame_up = gtk_frame_new("(*^_^*)");
    gtk_table_attach_defaults(GTK_TABLE(table), frame_up, 2, 4, 7, 10);
    gtk_widget_show(frame_up);
    gtk_widget_show(table); /* 建一个标签显示系统时间(用button代替) */
    label_time = gtk_button_new_with_label("good");
    gtk_tooltips_set_tip(tooltip, label_time, "点击刷新", NULL);
    g_signal_connect(G_OBJECT(label_time), "clicked", G_CALLBACK(set_proc_info), NULL);
    gtk_container_add(GTK_CONTAINER(frame_up), label_time);
    gtk_widget_show(label_time); /* 建一个frame在表下边显示进程信息 */
    frame_down = gtk_frame_new("进程信息表");
    gtk_table_attach_defaults(GTK_TABLE(table), frame_down, 0, 4, 0, 7);
    gtk_widget_show(frame_down);
    gtk_widget_show(table); /* 创建一个滚动窗口构件，将GtkCList组装到里面。 * 这样使得内容超出列表时，可以用滚动条浏览*/
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    gtk_container_add(GTK_CONTAINER(frame_down), scrolled_window);
    gtk_widget_show(scrolled_window);
    gchar *titles[5] = {"进程名", "进程PID", "进程状态", "占用内存", "优先级"}; /* 创建GtkCList构件。本例中，我们使用了5列*/
    clist = gtk_clist_new_with_titles(5, titles);
    gtk_tooltips_set_tip(tooltip, clist, "点击杀死进程", NULL);                                 /* 当作出选择时，我们要知道选择了哪一个单元格。 * 使用 selection_row回调函数，代码在clist.h可以看见*/
    gtk_signal_connect(GTK_OBJECT(clist), "select_row", GTK_SIGNAL_FUNC(selection_made), NULL); /* 不一定要设置边框的阴影，但是效果挺不错*/
    gtk_clist_set_shadow_type(GTK_CLIST(clist), GTK_SHADOW_OUT);                                /* 设置每一列到宽度 */
    gtk_clist_set_column_width(GTK_CLIST(clist), 0, 200);
    gtk_clist_set_column_width(GTK_CLIST(clist), 1, 120);
    gtk_clist_set_column_width(GTK_CLIST(clist), 2, 110);
    gtk_clist_set_column_width(GTK_CLIST(clist), 3, 100);
    gtk_clist_set_column_width(GTK_CLIST(clist), 4, 100);
    gtk_container_add(GTK_CONTAINER(scrolled_window), clist);
    gtk_widget_show(clist);
    set_proc_info();
}
void set_proc_info()
{ /* 读取当前进程信息，并显示到列表框中 */
    DIR *dir;
    struct dirent *ptr;
    dir = opendir("/proc");
    char path[20];     //记录pid的数字文件夹
    int fd;            //文件描述，用于打开那些数字文件夹的stat文件
    char store[1000];  //将读取的stat文件内容暂时存放到store中
    char name[20];     //进程名
    char pid_num[20];  //pid号
    char stat[20];     //状态
    char memory[20];   //内存
    char priority[20]; //优先级
    gtk_clist_clear(GTK_CLIST(clist));
    while ((ptr = readdir(dir)) != NULL)
    {
        if ((ptr->d_name[0] >= '0') && (ptr->d_name[0] <= '9'))
        {
            sprintf(path, "/proc/%s/stat", ptr->d_name); //将此文件的全部路径写入name中
            //打开这个文件，并且从中读取到有用的信息放在相应的数组中
            fd = open(path, O_RDONLY);
            read(fd, store, sizeof(store));
            close(fd);
            read_info(store, 1, pid_num);                    //进去的是字符串和号，出来对应信息修改
            read_info(store, 2, name);                       //读取名称
            read_info(store, 3, stat);                       //进程状态，SDR已经转化为中文
            read_info(store, 18, priority);                  //优先级
            read_info(store, 23, memory);                    //注意是以B为单位，需要换算
            int mem_num = atoi(memory);                      //转换成整数形式B为单位
            float real_mem = (float)mem_num / (1024 * 1024); //得到MB为单位的信息
            char convert_mem[25];
            gcvt(real_mem, 5, convert_mem); //浮点数转换成字符串
            gchar *list[1][5] = {{name, pid_num, stat, convert_mem, priority}};
            /* 把这一行加入到clist中 */
            gtk_clist_append((GtkCList *)clist, list[0]);
            gtk_clist_thaw((GtkCList *)clist); //更新list列表显示
        }
    }
    closedir(dir);
} /* 用户选中某一行时的回调函数*/
void selection_made(GtkWidget *clist, gint row, gint column, GdkEventButton *event, gpointer data)
{
    /* 取得存储在被选中的行和列的单元格上的文本 * 当鼠标点击时，我们用t e x t参数接收一个指针*/
    gchar *Pid, *procName, buffer[40] = "Kill the process <";
    /* 该函数读取CLIST被选中到PID， * 它与gtk_clist_set_text函数功能相反。*/
    gtk_clist_get_text(GTK_CLIST(clist), row, 1, &Pid);
    gtk_clist_get_text(GTK_CLIST(clist), row, 0, &procName);
    selectedRow = row;
    strcat(buffer, procName);
    strcat(buffer, "> !?");
    GtkWidget *dialog; dialog = gtk_message_dialog_new(NULL, // GTK_DIALOG_DESTROY_WITH_PARENT, //跟随父窗口关闭 GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,// buffer); 
    gtk_window_set_title(GTK_WINDOW(dialog), "*0(^_^)0*"); 
    GtkResponseType result = gtk_dialog_run(GTK_DIALOG(dialog)); 
    gtk_widget_destroy(dialog); 
    if (result == GTK_RESPONSE_YES || result == GTK_RESPONSE_APPLY) {
        killProcess(Pid); } return;
} /* 根据一个pid杀死进程 */ /* 使用全局变量PID */
void killProcess(gchar *Pid)
{
    gchar killCmd[50] = "kill ";
    strcat(killCmd, Pid);
    system(killCmd);
    g_printf("%s", killCmd); /* 用gtk_clist_clear函数清除列表 * void gtk_clist_remove (GtkCList *clist,gint row); * 该函数用来删除某一索引行。 * 行数确定时，直接对应第二个参数即可。 * 对于行数不确定，而是由鼠标或键盘随机获得焦点行的动态行删除问题， * 在具体编程实现上，可以通过利用CLIST列表构件头文件 * gtkclist.h内GtkCList结构中的属性变量focus_row来实现，语句如下： * gtk_clist_remove (GTK_CLIST(clist),GTK_CLIST (clist)->focus_row);*/
    gtk_clist_remove(clist, selectedRow);
    GtkWidget *dialog; //声明一个对话框 /* 新建一个消息对话框 */ dialog = gtk_message_dialog_new(NULL, //没有父窗口 GTK_DIALOG_DESTROY_WITH_PARENT, //跟随父窗口关闭 GTK_MESSAGE_WARNING, //显示警告图标 GTK_BUTTONS_OK, //显示OK按钮 ("人生寂寞啊，被人杀死了！")); //提示信息的内容 gtk_window_set_title(GTK_WINDOW(dialog), ("进程已经被杀死……"));//对话框的标题 gtk_dialog_run(GTK_DIALOG(dialog));//运行对话框 gtk_widget_destroy(dialog);//删除对话框 } void set_label_proc_text() { char buffer[100]; char processes[10]; getInforAftSpace("/proc/stat", "processes", processes); strcpy(buffer, "当前进程数 : "); strcat(buffer, processes); char btime[10];//系统运行时间 char times[30]; int fd = open("/proc/uptime", O_RDONLY); read(fd, times, sizeof(times)); close(fd); read_info(times, 0, btime); float fInfor = (float) atof(btime); fInfor = fInfor / 60; //将单位化成min gcvt(fInfor, 5, btime); strcat(buffer, "\n系统运行时间 : "); strcat(buffer, btime); strcat(buffer, " min"); char aveLoad[20]; get_load_avg(aveLoad); strcat(buffer, "\n平均负载 : "); strcat(buffer, aveLoad); gtk_button_set_label((GtkButton*) label_proc, buffer); strcpy(buffer, "--------------点击刷新------------\n\n当前时间 ："); time_t rawtime; struct tm * timeinfo; time(&rawtime); timeinfo = localtime(&rawtime); strcat(buffer, asctime(timeinfo)); gtk_button_set_label((GtkButton*) label_time, buffer); } #endif /* PROCESS_H_ */
/* * showAbout.h * systemMonitor * Created on: 2012-2-19 * Author: zhushengben */
#ifndef SHOWABOUT_H_
#define SHOWABOUT_H_                                                                                                                                     \
    void showAbout(void)                                                                                                                                 \
    {                                                                                                                                                    \
        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file("3.png", NULL);                                                                                     \
        GtkWidget *dialog = gtk_about_dialog_new();                                                                                                      \
        gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(dialog), "Ubuntu 系统监视器");                                                                        \
        gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "beta0.1");                                                                               \
        gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "Copyright © 2012-2114 ZhuShengben");                                                   \
        gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "查看当前进程，监视系统状态 .");                                                         \
        gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), "http://user.qzone.qq.com/337075552");                                                    \
        gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), pixbuf);                                                                                     \
        g_object_unref(pixbuf), pixbuf = NULL;                                                                                                           \
        gtk_dialog_run(GTK_DIALOG(dialog));                                                                                                              \
        gtk_widget_destroy(dialog);                                                                                                                      \
    } /* 创建一个滚动文本区域,用于显示一个"信息" */                                                                                    \
    void createAboutPage(GtkWidget *notebook)                                                                                                            \
    {                                                                                                                                                    \
        GtkWidget *label;                                                                                                                                \
        GtkWidget *table;                                                                                                                                \
        GtkWidget *button;                                                                                                                               \
        GtkWidget *align;                                                                                                                                \
        GtkWidget *tooltip;                                                                                                                              \
        GtkWidget *vbox;                                                                                                                                 \
        GtkWidget *scrolled_window;                                                                                                                      \
        GtkWidget *view;                                                                                                                                 \
        GtkWidget *vpaned;                                                                                                                               \
        GtkTextBuffer *buffer;                                                                                                                           \
        GtkWidget *frame = gtk_frame_new("编辑邮件");                                                                                                    \
        gtk_container_set_border_width(GTK_CONTAINER(frame), 10);                                                                                        \
        gtk_widget_set_size_request(frame, 120, 500);                                                                                                    \
        gtk_widget_show(frame);                                                             /* 将该标签页加入到notebook中 */                    \
        gtk_notebook_append_page(GTK_NOTEBOOK(notebook), frame, gtk_label_new("错误报告")); /* 在顶级窗口上添加一个垂直分栏窗口构件 */ \
        vpaned = gtk_vpaned_new();                                                                                                                       \
        gtk_container_add(GTK_CONTAINER(frame), vpaned);                                                                                                 \
        gtk_widget_show(vpaned); /* 在分栏窗口的两部分各添加一些构件 */                                                                  \
        scrolled_window = gtk_scrolled_window_new(NULL, NULL);                                                                                           \
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);                                \
        gtk_paned_add1(GTK_PANED(vpaned), scrolled_window);                                                                                              \
        view = gtk_text_view_new();                                                                                                                      \
        gtk_container_add(GTK_CONTAINER(scrolled_window), view);                                                                                         \
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));                                                                                          \
        insert_text(buffer, TRUE);                                                                                                                       \
        gtk_widget_show_all(scrolled_window);                                                                                                            \
        gtk_widget_show(scrolled_window);                                                                                                                \
        scrolled_window = gtk_scrolled_window_new(NULL, NULL);                                                                                           \
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);                                \
        gtk_paned_add2(GTK_PANED(vpaned), scrolled_window);                                                                                              \
        view = gtk_text_view_new();                                                                                                                      \
        gtk_container_add(GTK_CONTAINER(scrolled_window), view);                                                                                         \
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));                                                                                          \
        insert_text(buffer, FALSE);                                                                                                                      \
        gtk_widget_show_all(scrolled_window);                                                                                                            \
        gtk_widget_show(scrolled_window); /* 建一个表格，将下半部分为10行，10列 */ // table = gtk_table_new(10, 10, TRUE); /* 将这个表添加到框架 */ // gtk_container_add(GTK_CONTAINER(frame), table); /* 创建一个居中对齐的对象 */ align = gtk_alignment_new(0.5, 0.5, 0.8, 0); gtk_container_add(GTK_CONTAINER(frame), align); gtk_widget_show(align); button = gtk_button_new_with_label("Submit"); gtk_container_add(GTK_CONTAINER(align), button); tooltip = gtk_tooltips_new(); gtk_tooltips_set_tip(tooltip, button, "点击发送", NULL); gtk_widget_show(button); } /* 向文本构件中添加一些文本*/ void insert_text(GtkTextBuffer *buffer, gboolean thanks) { GtkTextIter iter; gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0); if (TRUE == thanks) gtk_text_buffer_insert(buffer, &iter,// "From: @nasa.gov\n" "To: SillyBenzhu@gmail.com\n" "Subject: Bugs found !\n" "There are some bugs just as follows:\n" // , -1); else gtk_text_buffer_insert(buffer, &iter,// "介绍\n" "*\n" "系统监视器手册 V2.2\n" " 系统监视器程序使您能够显示系统的基本信息，并能够监视系统进程、系统资源使用情况和文件系统。您也可以用系统监视器来修改系统的行为。\n" "系统监视器包含四个选项卡部分\n" "系统\n" " 显示计算机硬件和软件的不同基本信息。\n" "发行版\n" "*\n" " 发行版版本\n" "*\n" " Linux 内核版本\n" "*\n" " GNOME 版本\n" " 硬件\n" "*\n" " 安装的内存\n" "*\n" " 处理器和速度\n" " 系统状态\n" "*\n" " 当前可用磁盘空间\n" "处理器\n" " 显示活动进程和进程之间的相互关系。为每个单独进程提供详细信息，并使您能够控制活动进程。\n" " 资源\n" " 显示以下系统资源的当前使用情况。\n" "*\n" " CPU(中央处理器)时间\n" "*\n" " 内存和交换空间\n" "*\n" " 网络使用情况\n" " 文件系统\n" "% CPU\n" " 选中此项以显示进程当前使用的 CPU 时间百分率。\n" "CPU 时间\n" " 选中此项以显示进程已作用的 CPU 时间。\n" "启动\n" " 选中此项以显示进程开始运行的时间。\n" "优先级\n" " 选中此项以显示进程的 nice 值。nice 值设置进程的优先级：nice 值越低，优先极越高。\n" "ID\n" " 选中此项以显示进程标识符，也即 pid。pid 是一个唯一标识进程的数字。您可以使用 pid 在命令行操纵进程。\n" "内存\n" " 选中此项以显示进程当前占用的系统内存大小。\n" // , -1); } /* 设置背景图片 */ void update_widget_bg(GtkWidget *widget, gchar *img_file) { GtkStyle *style; GdkPixbuf *pixbuf; GdkPixmap *pixmap; gint width, height; pixbuf = gdk_pixbuf_new_from_file(img_file, NULL); width = gdk_pixbuf_get_width(pixbuf); height = gdk_pixbuf_get_height(pixbuf); pixmap = gdk_pixmap_new(NULL, width, height, 24); gdk_pixbuf_render_pixmap_and_mask(pixbuf, &pixmap, NULL, 0); style = gtk_style_copy(GTK_WIDGET(widget)->style); if (style->bg_pixmap[GTK_STATE_NORMAL]) g_object_unref(style->bg_pixmap[GTK_STATE_NORMAL]); style->bg_pixmap[GTK_STATE_NORMAL] = g_object_ref(pixmap); style->bg_pixmap[GTK_STATE_ACTIVE] = g_object_ref(pixmap); style->bg_pixmap[GTK_STATE_PRELIGHT] = g_object_ref(pixmap); style->bg_pixmap[GTK_STATE_SELECTED] = g_object_ref(pixmap); style->bg_pixmap[GTK_STATE_INSENSITIVE] = g_object_ref(pixmap); gtk_widget_set_style(GTK_WIDGET(widget), style); g_object_unref(style); } #endif /* SHOWABOUT_H_ */
/* * showImage.h * SystemMonitor * Created on: 2012-2-24 * Author: zhushengben */
#ifndef SHOWIMAGE_H_
#define SHOWIMAGE_H_                                                                                  \
    void showImage(int argc, char *argv[])                                                            \
    {                                                                                                 \
        GtkWidget *window = NULL;                                                                     \
        GdkPixbuf *pixbuf = NULL;                                                                     \
        GdkBitmap *bitmap = NULL;                                                                     \
        GdkPixmap *pixmap = NULL;                                                                     \
        gtk_init(&argc, &argv);                                                                       \
        window = gtk_window_new(GTK_WINDOW_TOPLEVEL);                                                 \
        gtk_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC(gtk_main_quit), NULL); \
        gtk_signal_connect(GTK_OBJECT(window), "destroy", GTK_SIGNAL_FUNC(gtk_main_quit), NULL);      \
        gtk_window_set_decorated(GTK_WINDOW(window), FALSE); // 设置无边框 gtk_widget_set_app_paintable(window, TRUE); gtk_window_set_default_size(GTK_WINDOW(window), 800, 600); gtk_widget_realize(window); pixbuf = gdk_pixbuf_new_from_file("penguin.gif", NULL); // gdk函数读取png文件 gdk_pixbuf_render_pixmap_and_mask(pixbuf, &pixmap, &bitmap, 120); // alpha小于128认为透明 gtk_widget_shape_combine_mask(window, bitmap, 0, 0); // 设置透明蒙板 gdk_window_set_back_pixmap(window->window, pixmap, FALSE); // 设置窗口背景 g_object_unref(pixbuf); g_object_unref(bitmap); g_object_unref(pixmap); gtk_widget_show_all(window); gtk_main(); } #endif /* SHOWIMAGE_H_ */
/* * menuBar.h * systemMonitor * Created on: 2012-2-19 * Author: zhushengben */
#ifndef MENUBAR_H_
#define MENUBAR_H_                                                                                                                                                                                                                                                                                                                                                                                                                      \
#include < stdio.h > #include < gtk / gtk.h > static gint button_press(GtkWidget *, GdkEvent *);                                                                                                                                                                                                                                                                                                                                    \
    static void menuitem_response(gchar *);                                                                                                                                                                                                                                                                                                                                                                                             \
#define BACK_IMAGE("3.jpg") int createMenuBar(GtkWidget *vbox)                                                                                                                                                                                                                                                                                                                                                                      \
    {                                                                                                                                                                                                                                                                                                                                                                                                                                   \
        GtkWidget *menu;                                                                                                                                                                                                                                                                                                                                                                                                                \
        GtkWidget *menu_bar;                                                                                                                                                                                                                                                                                                                                                                                                            \
        GtkWidget *monitor_menu;                                                                                                                                                                                                                                                                                                                                                                                                        \
        GtkWidget *edit_menu;                                                                                                                                                                                                                                                                                                                                                                                                           \
        GtkWidget *view_menu;                                                                                                                                                                                                                                                                                                                                                                                                           \
        GtkWidget *help_menu;                                                                                                                                                                                                                                                                                                                                                                                                           \
        GtkWidget *menu_items;                                                                                                                                                                                                                                                                                                                                                                                                          \
        char buf[128];                                                                                                                                                                                                                                                                                                                                                                                                                  \
        int i; /* 初始化菜单构件,记住,永远也不要用 * gtk_show_widget() 来显示菜单构件。 * 这个是包含菜单项的菜单,当你在程序的"Root Menu"上点击时 * 它会弹出来 */                                                                                                                                                                                                                       \
        menu = gtk_menu_new();                                                                                                                                                                                                                                                                                                                                                                                                          \
        update_widget_bg(menu, "3.jpg");                                                              /* 创建一个新的菜单项,名称为... */                                                                                                                                                                                                                                                                                    \
        menu_items = gtk_menu_item_new_with_label("刷新模块");                                        /* ...并将它加到菜单。 */                                                                                                                                                                                                                                                                                                 \
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_items);                                      /* 当菜单项被选中时做点有趣的事 */                                                                                                                                                                                                                                                                                  \
        g_signal_connect_swapped(G_OBJECT(menu_items), "activate", G_CALLBACK(set_modul_info), NULL); /* 显示构件 */                                                                                                                                                                                                                                                                                                                \
        gtk_widget_show(menu_items);                                                                                                                                                                                                                                                                                                                                                                                                    \
        menu_items = gtk_menu_item_new_with_label("装载模块");                                   /* ...并将它加到菜单。 */                                                                                                                                                                                                                                                                                                      \
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_items);                                 /* 当菜单项被选中时做点有趣的事 */                                                                                                                                                                                                                                                                                       \
        g_signal_connect_swapped(G_OBJECT(menu_items), "activate", G_CALLBACK(add_modul), NULL); /* 显示构件 */                                                                                                                                                                                                                                                                                                                     \
        gtk_widget_show(menu_items);                                                                                                                                                                                                                                                                                                                                                                                                    \
        menu_items = gtk_menu_item_new_with_label("卸载模块");                                      /* ...并将它加到菜单。 */                                                                                                                                                                                                                                                                                                   \
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_items);                                    /* 当菜单项被选中时做点有趣的事 */                                                                                                                                                                                                                                                                                    \
        g_signal_connect_swapped(G_OBJECT(menu_items), "activate", G_CALLBACK(remove_modul), NULL); /* 显示构件 */                                                                                                                                                                                                                                                                                                                  \
        gtk_widget_show(menu_items);                                                                /* * 接着我们用一个小循环为"test-menu"产生三个菜单条目。 * 注意对 gtk_menu_append 的调用。这里我们将一序列的菜单项 * 加到我们的菜单上。通常,我们也捕获每个菜单项的"clicked" * 信号并为它设置一个回调,不过在这里这个被省略了以节省空间。 */ \
        for (i = 0; i < 3; i++)                                                                                                                                                                                                                                                                                                                                                                                                         \
        {                                                                                                             /* 将名称复制到 buf. */                                                                                                                                                                                                                                                                                     \
            sprintf(buf, "版权所有", i);                                                                              /* 创建一个新的菜单项,名称为... */                                                                                                                                                                                                                                                                    \
            menu_items = gtk_menu_item_new_with_label(buf);                                                           /* ...并将它加到菜单。 */                                                                                                                                                                                                                                                                                 \
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_items);                                                  /* 当菜单项被选中时做点有趣的事 */                                                                                                                                                                                                                                                                  \
            g_signal_connect_swapped(G_OBJECT(menu_items), "activate", G_CALLBACK(menuitem_response), g_strdup(buf)); /* 显示构件 */                                                                                                                                                                                                                                                                                                \
            gtk_widget_show(menu_items);                                                                                                                                                                                                                                                                                                                                                                                                \
        } /* 这个是根菜单,将成为显示在菜单栏上的标签。 * 这里不会附上一个信号处理函数,因为它只是在 * 被按下时弹出其余的菜单。 */                                                                                                                                                                                                                                                    \
        monitor_menu = gtk_menu_item_new_with_label("监视器(M)");                                                                                                                                                                                                                                                                                                                                                                       \
        edit_menu = gtk_menu_item_new_with_label("编辑(E)");                                                                                                                                                                                                                                                                                                                                                                            \
        view_menu = gtk_menu_item_new_with_label("查看(V)");                                                                                                                                                                                                                                                                                                                                                                            \
        help_menu = gtk_menu_item_new_with_label("帮助(H)");                                                                                                                                                                                                                                                                                                                                                                            \
        gtk_widget_show(monitor_menu);                                                                                                                                                                                                                                                                                                                                                                                                  \
        gtk_widget_show(edit_menu);                                                                                                                                                                                                                                                                                                                                                                                                     \
        gtk_widget_show(view_menu);                                                                                                                                                                                                                                                                                                                                                                                                     \
        gtk_widget_show(help_menu); /* 现在我们指定我们想要让新创建的"menu"成 * 为"root menu"的菜单 */                                                                                                                                                                                                                                                                                                              \
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(monitor_menu), menu);                                                                                                                                                                                                                                                                                                                                                                   \
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit_menu), menu);                                                                                                                                                                                                                                                                                                                                                                      \
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(view_menu), menu);                                                                                                                                                                                                                                                                                                                                                                      \
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_menu), menu); /* 创建一个菜单栏以包含菜单,并将它加到主窗口 * 一个纵向盒子里: */                                                                                                                                                                                                                                                                         \
        menu_bar = gtk_menu_bar_new();                                                                                                                                                                                                                                                                                                                                                                                                  \
        update_widget_bg(menu_bar, "3.jpg"); // update_widget_bg(menu_bar, BACK_IMAGE); gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 2); gtk_widget_show(menu_bar); /* 最后把菜单项添加到菜单栏上 -- 这就是我 * 咆哮了多次的“根”菜单项 =) */ gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), monitor_menu); gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), edit_menu); gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), view_menu); gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), help_menu); return 0; } int createButtonMenu(GtkWidget *vbox) { GtkWidget *menu; GtkWidget *menu_items; GtkWidget *button; /* 初始化菜单构件,记住,永远也不要用 * gtk_show_widget() 来显示菜单构件。 * 这个是包含菜单项的菜单,当你在程序的"Root Menu"上点击时 * 它会弹出来 */ menu = gtk_menu_new(); /* 创建一个新的菜单项,名称为... */ menu_items = gtk_menu_item_new_with_label("Sure to kill the process !!??"); /* ...并将它加到菜单。 */ gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_items); /* 当菜单项被选中时做点有趣的事 */ g_signal_connect_swapped(G_OBJECT(menu_items), "activate", G_CALLBACK( killProcess), NULL); /* 显示构件 */ gtk_widget_show(menu_items); /* 创建一个按钮,它带了一个弹出菜单 */ button = gtk_button_new_with_label( "* * * * * * * * * * * * * * * * * * * * * * *"); /* 连接弹出菜单 */ g_signal_connect_swapped(G_OBJECT(button), "event", G_CALLBACK(button_press), menu); gtk_box_pack_start(GTK_BOX(vbox), button, TRUE, TRUE, 2); gtk_widget_show(button); return 0; } /* 对鼠标按下作出回应,弹出 widget 变量传递进来的菜单。 * 注意"widget"参数是被传递进来的菜单,不是 * 被按下的按钮。 */ static gint button_press(GtkWidget *widget, GdkEvent *event) { if (event->type == GDK_BUTTON_PRESS) { GdkEventButton *bevent = (GdkEventButton *) event; gtk_menu_popup(GTK_MENU(widget), NULL, NULL, NULL, NULL, bevent->button, bevent->time); /* 告诉调用代码我们已经处理了这个事件;事件传播(buck)在 * 这里停止。 */ return TRUE; } /* 告诉调用代码我们没有处理这个事件;继续传播它。 */ return FALSE; } /* 当菜单项被选中时打印一个字符串 */ static void menuitem_response(gchar *string) { showAbout(); } #endif /* MENUBAR_H_ */
/* * progressBar.h * systemMonitor * Created on: 2012-2-19 * Author: zhushengben */
#ifndef PROGRESSBAR_H_
#define PROGRESSBAR_H_                                                                       \
    typedef struct ProgressData                                                              \
    {                                                                                        \
        GtkWidget *pbar;                                                                     \
        int timer;                                                                           \
        gboolean activity_mode;                                                              \
        gboolean showCpuProgress;                                                            \
    } _ProgressData;                                                                         \
    _ProgressData *pdata;                                                                    \
    int createProgressBar(GtkWidget *vbox, gboolean activity_mode, gboolean showCpuProgress) \
    {                                                                                        \
        GtkWidget *align;                                                                    \
        GtkWidget *separator;                                                                \
        GtkWidget *table;                                                                    \
        GtkWidget *button;                                                                   \
        GtkWidget *check; /* 为传递到回调函数中的数据分配内存 */             \
        pdata = g_malloc(sizeof(_ProgressData));                                             \
        pdata->showCpuProgress = showCpuProgress; /* 创建进度条 */                      \
        pdata->pbar = gtk_progress_bar_new();                                                \
        gtk_container_add(GTK_CONTAINER(vbox), pdata->pbar);                                 \
        gtk_progress_bar_set_orientation(pbar_cpu, GTK_PROGRESS_BOTTOM_TO_TOP);              \
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pdata->pbar), "running...");              \
        pdata->activity_mode = activity_mode;                                                \
        gtk_widget_show(pdata->pbar);                                                        \
        separator = gtk_hseparator_new();                                                    \
        gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 0);                       \
        gtk_widget_show(separator);                                                          \
        return 0;                                                                            \
    }                                                                                        \
#endif /* PROGRESSBAR_H_ */
/* * refresh.h * SystemMonitor * Created on: 2012-2-21 * Author: zhushengben */
#ifndef REFRESH_H_ #define REFRESH_H_
    /* 更新进度条,这样就能够看到进度条的移动 */
    gint refresh(gpointer data)
    { /* cpu */                                              /* 建一个矩形绘图区 */
        GdkGC *gc_chart = gdk_gc_new(cpu_draw_area->window); // update_widget_bg(gc_chart, BACK_IMAGE); /* 背景颜色 */ GdkColor color; color.red = 0x0000; color.green = 0x0000; color.blue = 0x0000; gdk_gc_set_rgb_fg_color(gc_chart, &color); int width, height, curPoint, step; cpu_rate = getCpuUseRatio() / 100; gdk_draw_rectangle(cpu_graph, gc_chart, TRUE, 0, 0, cpu_draw_area->allocation.width, cpu_draw_area->allocation.height); width = cpu_draw_area->allocation.width; height = cpu_draw_area->allocation.height; curPoint = (int) (cpu_rate * (double) height); cpuPoints[99] = height - curPoint; int i; for (i = 0; i < 99; i++) { /* 后一时刻的为前面取代 */ cpuPoints[i] = cpuPoints[i + 1]; } step = width / 99; GdkGC *gc = gdk_gc_new(GDK_DRAWABLE(cpu_graph)); gdk_color_parse("darkred", &color); if (cpu_rate > 0.1) gdk_color_parse("red", &color); gdk_gc_set_foreground(gc, &color); /* gdk_gc_set_line_attributes(GdkGC *gc,// line_width, GdkLineStyle line_style, GdkCapStyle cap_style, GdkJoinStyle join_style); */ gdk_gc_set_line_attributes(gc, 1, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_MITER); for (i = 99; i >= 1; i--) { gdk_draw_line(GDK_DRAWABLE(cpu_graph), gc, i * step, cpuPoints[i], /* 第一个点坐标 */ (i - 1) * step, cpuPoints[i - 1]); /* 第二个点坐标 */ } gtk_widget_queue_draw(cpu_draw_area); char buffer[50] = "cpu使用率:\n\n"; char cbuf[5]; gcvt(cpu_rate * 100, 5, cbuf); strcat(cbuf, "%"); strcat(buffer, cbuf); gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbar_cpu), buffer); gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbar_cpu), cpu_rate); _ProgressData *pdata = (_ProgressData *) data; gdouble new_val; if (pdata->activity_mode) gtk_progress_bar_pulse(GTK_PROGRESS_BAR(pdata->pbar)); else { if (!pdata->showCpuProgress) { /* 使用在调整对象中设置的取值范围计算进度条的值 */ new_val = gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR( pdata->pbar)) + 0.01; if (new_val > 1.0) new_val = 0.0; } else new_val = cpu_rate; /* 设置进度条的新值 */ gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pdata->pbar), new_val); } set_label_mem_text(); set_label_proc_text(label_proc); /* 这是一个 timeout 函数,返回 TRUE,这样它就能够继续被调用 */ return TRUE; } #endif /* REFRESH_H_ */
/* * searchInfor.h * helloWorld * Created on: 2012-2-19 * Author: zhushengben */
#ifndef SEARCHINFOR_H_
#define SEARCHINFOR_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#define MAX 1024
#define STANDARD 0
#define SHORT 1
#define LONG 2
        /* * 在指定的proc文件中查找所需要的内核信息,并将找到的字符串后面
的信息输出 * pah:指定的proc文件的路径 * name:所要查找的内核信息 * infor:装
载信息，已经转换为MB * 内存信息 * more /proc/meminfo 
* MemTotal: 2058448 kB * MemFree: 52496 kB * Buffers: 580832 kB * Cached: 877024 kB */
        void getInfor(char *path, char *name, char infor[])
        {
            int fd = open(path, O_RDONLY);
            char store[2000];
            int k = 0;
            char *p = NULL;
            read(fd, store, sizeof(store));
            close(fd);
            /* 取得所需要信息所在行首尾指针 */
            p = strstr(store, name);
            /* 跳过标题 */
            while ((*p < '0') || (*p > '9'))
            {
                p++;
            }
            /* 读取数值 */
            while ((*p >= '0') && (*p <= '9'))
            {
                infor[k] = *p;
                k++;
                p++;
            }
            infor[k] = '\0';
            float fInfor = (float)atoi(infor);
            fInfor = fInfor / 1024; //将单位化成MB
            gcvt(fInfor, 5, infor); //换算后的存放在total中
        }
        /* 直接取空格后数值,没有转换尾MB的 */
        void getInforAftSpace(char *path, char *name, char infor[])
        {
            int fd = open(path, O_RDONLY);
            char store[2000];
            int k = 0;
            char *p = NULL;
            read(fd, store, sizeof(store));
            close(fd);
            /* 取得所需要信息所在行首尾指针 */
            p = strstr(store, name);
            /* 跳过标题 */
            while ((*p < '0') || (*p > '9'))
            {
                p++;
            }
            /* 读取数值 */
            while ((*p >= '0') && (*p <= '9'))
            {
                infor[k] = *p;
                k++;
                p++;
            }
            infor[k] = '\0';
        }
        /* 获取cpu信息等 */
        /*model name : Pentium(R) Dual-Core CPU T4400 @ 2.20GHz *stepping : 10 *cpu MHz : 1200.000 *cache size : 1024 KB */
        void getInforNotd(char *path, char *name, char infor[])
        {
            int fd = open(path, O_RDONLY);
            char store[2000];
            int k = 0;
            char *p = NULL;
            read(fd, store, sizeof(store));
            close(fd);               /* 取得所需要信息所在行首尾指针 */
            p = strstr(store, name); /* 跳过标题 */
            do
            {
                p++;
            } while (*p != ':');
            p += 2; /* 读取数值 */
            while (*p != '\n')
            {
                infor[k] = *p;
                k++;
                p++;
            }
            infor[k] = '\0';
        }
        void getSysVersion(char *path, char *name, char infor[])
        {
            int fd = open(path, O_RDONLY);
            char store[2000];
            int k = 0;
            char *p = NULL;
            read(fd, store, sizeof(store));
            close(fd);               /* 取得所需要信息所在行首尾指针 */
            p = strstr(store, name); /* 读取数值 */
            while (*p != '\n' && *p != ')')
            {
                infor[k] = *p;
                k++;
                p++;
            }
            infor[k++] = ')';
            infor[k] = '\0';
        } /* 显示系统的平均负载的内容 */
        int get_load_avg(char aveLoad[])
        {
            int fd;
            int n, k = 0;
            int res = -1;
            char line_buf[50]; /* 打开存储系统平均负载的proc文件 */
            if ((fd = open("/proc/loadavg", O_RDONLY)) == -1)
            {
                perror("fail to loadavg");
                exit(-1);
            } /* 读取系统的平均负载的内容 */
            if ((n = read(fd, line_buf, MAX)) == -1)
            {
                perror("fail to read");
                goto err;
            }
            line_buf[n] = '/0';
            n = 0;
            while (line_buf[n] != '\n' && line_buf[n] != '\0')
            {
                aveLoad[k++] = line_buf[n++];
            }
            aveLoad[k] = '\0';
            res = 0;
        err:
            close(fd);
            return res;
        }
        void read_info(char store[], int i, char get[])
        {
            int j = 0; //j表示的就是这个字符串的下标
            int k = 0;
            int cflags = 0; //用来记录空格的个数
            char buf[20];   //用来保存需要的信息
            while (cflags < i - 1)
            { //特别注意此处是i-1，否则输出的顺序都是乱的，因为需要在i之前的空格，所以是i-1，这样执行完才是i-1的空格
                if (store[j++] == ' ')
                    cflags++;
            }
            while (store[j] != ' ')
            { //读取下一个空格之前的
                buf[k++] = store[j];
                j++;
            }
            buf[k] = '\0';
            if (2 == i)
            { //名称，去掉括号
                int i, j = 0;
                for (i = 1; ')' != buf[i]; i++)
                {
                    get[j++] = buf[i];
                }
                get[j] = '\0';
            }
            else if (i == 3)
            { //第三个是表示状态
                if (!strcmp(buf, "S"))
                    strcpy(get, "睡眠中");
                else if (!strcmp(buf, "R"))
                    strcpy(get, "运行中");
                else if (!strcmp(buf, "T"))
                    strcpy(get, "已停止");
                else if (!strcmp(buf, "D"))
                    strcpy(get, "不可中断");
                else if (!strcmp(buf, "Z"))
                    strcpy(get, "死锁中"); /* 这里出现一个问题，想不通啊！ * 为什么上面两行不是放在if外面？ * 原来是if中= 和 == 混淆 */
                else
                    strcpy(get, "奇怪！");
            }
            else
                strcpy(get, buf);
        }
#endif /* SEARCHINFOR_H_ */