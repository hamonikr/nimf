#include <gtk/gtk.h>

static void on_entry_changed(GtkEditable *editable, gpointer user_data) {
  GtkLabel *status = GTK_LABEL(user_data);
  const char *text = gtk_editable_get_text(editable);
  char *msg = g_strdup_printf("Entry text: %s", text);
  gtk_label_set_text(status, msg);
  g_free(msg);
}

static void on_entry_insert_text(GtkEditable *editable, const char *text, int length, int *position, gpointer user_data) {
  (void)position; (void)user_data;
  char *utf8 = g_strndup(text, length);
  g_print("[insert-text] '%s' (len=%d)\n", utf8, length);
  g_free(utf8);
}

static gboolean on_key_pressed(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data) {
  (void)controller; (void)user_data;
  g_print("[key-pressed] keyval=%u keycode=%u state=0x%x\n", keyval, keycode, state);
  return FALSE; // don't stop
}

static gboolean on_key_released(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data) {
  (void)controller; (void)user_data;
  g_print("[key-released] keyval=%u keycode=%u state=0x%x\n", keyval, keycode, state);
  return FALSE;
}

static void activate(GtkApplication *app, gpointer user_data) {
  GtkWidget *window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "GTK4 IM Test (nimf)");
  gtk_window_set_default_size(GTK_WINDOW(window), 500, 300);

  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
  gtk_window_set_child(GTK_WINDOW(window), box);

  GtkWidget *label_info = gtk_label_new("아래 Entry 또는 TextView에 한글 조합 입력 후 Enter를 눌러보세요.\nGTK_IM_MODULE=nimf 로 실행하십시오.");
  gtk_box_append(GTK_BOX(box), label_info);

  GtkWidget *entry = gtk_entry_new();
  gtk_widget_set_hexpand(entry, TRUE);
  gtk_box_append(GTK_BOX(box), entry);

  GtkWidget *status = gtk_label_new("Entry text: ");
  gtk_box_append(GTK_BOX(box), status);

  GtkWidget *scroller = gtk_scrolled_window_new();
  gtk_widget_set_vexpand(scroller, TRUE);
  gtk_box_append(GTK_BOX(box), scroller);

  GtkWidget *textview = gtk_text_view_new();
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroller), textview);

  // Signals for entry
  g_signal_connect(entry, "changed", G_CALLBACK(on_entry_changed), status);
  g_signal_connect(entry, "insert-text", G_CALLBACK(on_entry_insert_text), NULL);

  GtkEventController *keyc1 = gtk_event_controller_key_new();
  g_signal_connect(keyc1, "key-pressed", G_CALLBACK(on_key_pressed), NULL);
  g_signal_connect(keyc1, "key-released", G_CALLBACK(on_key_released), NULL);
  gtk_widget_add_controller(entry, keyc1);

  // Signals for textview (to observe key flow as well)
  GtkEventController *keyc2 = gtk_event_controller_key_new();
  g_signal_connect(keyc2, "key-pressed", G_CALLBACK(on_key_pressed), NULL);
  g_signal_connect(keyc2, "key-released", G_CALLBACK(on_key_released), NULL);
  gtk_widget_add_controller(textview, keyc2);

  gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
  GtkApplication *app = gtk_application_new("org.nimf.gtk4imtest", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}
