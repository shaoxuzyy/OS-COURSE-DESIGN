#include <gtk/gtk.h>
#include <stdio.h>
#include <unistd.h>

#define REFRESH_TIME 1000

gboolean timedisplay(gpointer label)
{
    time_t times;
    struct tm *timebuf;
    time(&times);
    timebuf = localtime(&times);

    gchar *text_day = g_strdup_printf("<span font_desc='48'>%04d-%02d-%02d</span>",
                                      1900 + timebuf->tm_year, 1 + timebuf->tm_mon, timebuf->tm_mday);
    gchar *text_time = g_strdup_printf("<span font_desc='32'>%02d:%02d:%02d</span>",
                                       timebuf->tm_hour, timebuf->tm_min, timebuf->tm_sec);
    gchar *text_data = g_strdup_printf("\n%s\n\n%s\n", text_day, text_time);

    gtk_label_set_markup(GTK_LABEL(label),text_data);
    return TRUE;
}

gboolean countup(gpointer label)
{
    static int num = 0;

    gchar *num_old = g_strdup_printf("<span font_desc='32'>%d</span>",num);
    num++;
    gchar *num_new = g_strdup_printf("<span font_desc='32'>%d</span>",num);
    if(num == 20) num = 0;

    gchar *printdata = g_strdup_printf("\n%s\n\n%s\n",num_old,num_new);
    gtk_label_set_markup(GTK_LABEL(label),printdata);
    return TRUE;
}

gboolean countdown(gpointer label)
{
    static int num = 20;

    gchar *num_old = g_strdup_printf("<span font_desc='32'>%d</span>",num);
    num--;
    gchar *num_new = g_strdup_printf("<span font_desc='32'>%d</span>",num);
    if(num == 0) num = 20;

    gchar *printdata = g_strdup_printf("\n%s\n\n%s\n",num_old,num_new);
    gtk_label_set_markup(GTK_LABEL(label),printdata);
    return TRUE;
}

void on_timewindow_destroy(){
	gtk_main_quit();
}

void on_countwindow_destroy(){
    gtk_main_quit();
}

void on_numwindow_destroy(){
    gtk_main_quit();
}

int main()
{
    int pid1, pid2;
    if ((pid1 = fork()) == 0)
    {
        GtkWidget *window;
        GtkWidget *label;

        gtk_init(NULL,NULL);
        GtkBuilder *timebuilder = gtk_builder_new();
        if(!gtk_builder_add_from_file(timebuilder,"./timewindow.glade",NULL)){
			printf("cannot load time glade!\n");
		}
        window = GTK_WIDGET(gtk_builder_get_object(timebuilder,"timewindow"));
		label = GTK_WIDGET(gtk_builder_get_object(timebuilder,"time"));

        gtk_builder_connect_signals(timebuilder,NULL);

        g_timeout_add(REFRESH_TIME,timedisplay,label);
        gtk_widget_show_all(window);
        gtk_main();
        printf("window pid1 closed!\n");
        exit(0);
    }
    else if((pid2 = fork()) == 0){
        GtkWidget *window;
        GtkWidget *label;

        gtk_init(NULL,NULL);
        GtkBuilder *countbuilder = gtk_builder_new();
        if(!gtk_builder_add_from_file(countbuilder,"./countwindow.glade",NULL)){
			printf("cannot load count glade!\n");
		}
        window = GTK_WIDGET(gtk_builder_get_object(countbuilder,"countwindow"));
		label = GTK_WIDGET(gtk_builder_get_object(countbuilder,"num"));
        gtk_builder_connect_signals(countbuilder,NULL);
        g_timeout_add(REFRESH_TIME,countup,label);
        gtk_widget_show_all(window);
        gtk_main();
        printf("window pid2 closed!\n");
        exit(0);
    }
    else{
        GtkWidget *window;
        GtkWidget *label;

        gtk_init(NULL,NULL);
        GtkBuilder *countdownbuilder = gtk_builder_new();
        if(!gtk_builder_add_from_file(countdownbuilder,"./countdownwindow.glade",NULL)){
			printf("cannot load count glade!\n");
		}
        window = GTK_WIDGET(gtk_builder_get_object(countdownbuilder,"numwindow"));
		label = GTK_WIDGET(gtk_builder_get_object(countdownbuilder,"num"));
        gtk_builder_connect_signals(countdownbuilder,NULL);
        g_timeout_add(REFRESH_TIME,countdown,label);
        gtk_widget_show_all(window);
        gtk_main();
        wait(0);
        wait(0);
        return 0;
    }
}