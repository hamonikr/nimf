#include "dasom-server.h"
#include <glib-unix.h>

int
main (int argc, char **argv)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  DasomServer *server;
  GMainLoop   *loop;
  GError      *error = NULL;

#if ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, DASOM_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  server = dasom_server_new ("unix:abstract=dasom", &error);

  if (server == NULL)
  {
    g_critical ("%s", error->message);
    g_clear_error (&error);

    return EXIT_FAILURE;
  }

  dasom_server_start (server);

  /* TODO: demonize */
  loop = g_main_loop_new (NULL, FALSE);

  g_unix_signal_add (SIGINT,  (GSourceFunc) g_main_loop_quit, loop);
  g_unix_signal_add (SIGTERM, (GSourceFunc) g_main_loop_quit, loop);

  g_main_loop_run (loop);

  g_main_loop_unref (loop);
  g_object_unref (server);

  return EXIT_SUCCESS;
}
