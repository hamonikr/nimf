#include <gtk/gtk.h>

int main (int argc, char *argv[])
{
     GtkWidget *window, *textview;

     gtk_init (&argc, &argv);

     window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
     gtk_window_set_title (GTK_WINDOW (window), "Text View");
     gtk_container_set_border_width (GTK_CONTAINER (window), 10);
     gtk_widget_set_size_request (window, 640, 480);
     g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

     textview = gtk_text_view_new ();
     gtk_container_add (GTK_CONTAINER (window), textview);
     gtk_widget_show_all (window);

     gtk_main();
     return 0;
}
