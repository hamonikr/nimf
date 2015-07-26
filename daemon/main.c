#include "dasom-server.h"
#include <glib-unix.h>

int
main (int argc, char **argv)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  int status;
  DasomServer *server;

#if ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, DASOM_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  server = dasom_server_new ();
  g_unix_signal_add (SIGINT,  (GSourceFunc) dasom_server_stop, server);
  g_unix_signal_add (SIGTERM, (GSourceFunc) dasom_server_stop, server);

  status = dasom_server_start (server);
  g_object_unref (server);

  return status;
}
