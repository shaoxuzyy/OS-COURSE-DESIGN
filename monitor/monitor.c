#include <stdio.h>
#include "monitor.h"
#include "progress.h"


void select_row_callback(GtkWidget *clist, gint row, gint column, GdkEventButton *event, gpointer data)
{
    gtk_clist_get_text(GTK_CLIST(clist), row, 0, &now_pid); //读取clist中的数据
    gtk_entry_set_text(GTK_ENTRY(entry), (gchar *)now_pid); //与上面相反
    return;
}

void search(GtkButton *button, gpointer data)
{
    const gchar *entry_txt;
    gchar *txt;
    gint ret, i = 0;
    entry_txt = gtk_entry_get_text(GTK_ENTRY(entry)); //获取输入的要搜索的进程PID
    while ((ret = gtk_clist_get_text(GTK_CLIST(clist), i, 0, &txt)) != 0)
    {
        //遍历寻找该进程的位置
        if (!strcmp(entry_txt, txt))
            break;
        i++;
    }
    gtk_clist_select_row(GTK_CLIST(clist), i, 0);
    scroll_to_line(data, process_num, i); //跳转到对应的行
    return;
}

void killproc(void)
{
    int ret;
    if (now_pid != NULL)
    {
        ret = kill(atoi(now_pid), SIGKILL);
        if (ret == -EPERM)
        { //判断是否是用户权限的问题
            popup_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
            popuplabel = gtk_label_new("You Need ROOT Authority!\n");
            set_label_fontsize(popuplabel, "14"); //设置字体大小
            gtk_widget_set_size_request(popup_window, 300, 180);
            gtk_container_add(GTK_CONTAINER(popup_window), popuplabel);
            gtk_window_set_title(GTK_WINDOW(popup_window), "ERROR");
            gtk_widget_show_all(popup_window);
        }
    }
    return;
}

void refresh(void)
{
    gtk_clist_freeze(GTK_CLIST(clist));
    gtk_clist_clear(GTK_CLIST(clist));
    get_process_info();
    gtk_clist_thaw(GTK_CLIST(clist));
    gtk_clist_select_row(GTK_CLIST(clist), 0, 0);
    return;
}

int main(int argc, char **argv)
{
    gtk_init(&argc, &argv);

    GtkWidget *top_window; //窗口主题
    GtkWidget *notebook;
    GtkWidget *hbox;
    GtkWidget *vbox;

    GtkWidget *label;
    GtkWidget *label1;
    GtkWidget *label2;
    GtkWidget *label3;

    GtkWidget *scrolled_window;
    GtkWidget *frame;

    GtkWidget *cpu_use;
    GtkWidget *mem_use;
    GtkWidget *swap_use;

    GtkWidget *button1;
    GtkWidget *button2;
    GtkWidget *button3;

    GtkWidget *fixed;
    GtkWidget *image;

    char title_buf[BUF_LEN]; //页的名字
    char buffer[BUF_LEN];
    char cpu_name[BUF_LEN];
    char cpu_addr_digit[BUF_LEN];
    char cpu_cache_size[BUF_LEN];
    char cpu_core[BUF_LEN];
    //设置主窗口
    top_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(top_window), "System Monitor");
    gtk_widget_set_size_request(top_window, 600, 700); //设置监控器窗口大小
    g_signal_connect(G_OBJECT(top_window), "delete_event", G_CALLBACK(gtk_main_quit), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(top_window), 10); //设置窗口边框宽度
    //设置相应的页
    notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(top_window), notebook);
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP); //设置页标签的位置

    //设置第一页
    sprintf(title_buf, "Process");
    vbox = gtk_vbox_new(FALSE, 0); //FALSE:子构件是否相同大小； 子构建之间的距离
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scrolled_window, 500, 550);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC); //设置滚动条出现的方式

    clist = gtk_clist_new(6); //创建分栏列表构件
    get_process_info();
    gtk_signal_connect(GTK_OBJECT(clist), "select_row", GTK_SIGNAL_FUNC(select_row_callback), NULL);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window), clist);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 5);

    hbox = gtk_hbox_new(FALSE, 10);
    entry = gtk_entry_new(); //获取输入
    gtk_entry_set_max_length(GTK_ENTRY(entry), 0);
    button1 = gtk_button_new_with_label("Search");
    button2 = gtk_button_new_with_label("Kill");
    button3 = gtk_button_new_with_label("Refresh");
    //设置相关回调函数
    g_signal_connect(G_OBJECT(button1), "clicked", G_CALLBACK(search), scrolled_window);
    g_signal_connect(G_OBJECT(button2), "clicked", G_CALLBACK(killproc), NULL);
    g_signal_connect(G_OBJECT(button3), "clicked", G_CALLBACK(refresh), NULL);

    gtk_widget_set_size_request(entry, 200, 30);
    gtk_widget_set_size_request(button1, 120, 30);
    gtk_widget_set_size_request(button2, 80, 30);
    gtk_widget_set_size_request(button3, 80, 30);
    gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), button1, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), button2, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), button3, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);


    /* gtk show*/
    gtk_widget_show_all(top_window);
    gtk_main();
    return 0;
}
