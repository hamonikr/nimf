#include "dasom-server.h"
#include <glib-unix.h>

int
main (int argc, char **argv)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  int status;
  DasomDaemon *daemon;

#if ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, DASOM_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  daemon = dasom_daemon_new ();
  g_unix_signal_add (SIGINT,  (GSourceFunc) dasom_daemon_stop, daemon);
  g_unix_signal_add (SIGTERM, (GSourceFunc) dasom_daemon_stop, daemon);

  status = dasom_daemon_start (daemon);
  g_object_unref (daemon);

  return status;
}
