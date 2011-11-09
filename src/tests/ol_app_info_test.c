#include <string.h>
#include <gio/gdesktopappinfo.h>
#include "ol_app_info.h"
#include "ol_test_util.h"

const char *DEFAULT_COMMAND = "mount /dev/mountdev /path/to/mount";
const char *DEFAULT_NAME = "Mount";
const char *DEFAULT_CMDNAME = "mount";
const char *DEFAULT_ICON_NAME = "back";
const char *COMMAND_WITH_ARG_SPACE = "ls 'a b'";
const char *CMDNAME_WITH_ARG_SPACE = "ls";
const char *COMMAND_NOT_EXIST = "a-not-exist-command like this";
const char *CMDNAME_NOT_EXIST = "a-not-exist-command";
const char *COMMAND_WITH_SPACE = "'l s' a b";
const char *CMDNAME_WITH_SPACE = "l s";
const char *COMMAND_PREFIX = "moun /dev/mountdev /path/to/mount";

const char *DESKTOP_CANDIDATES[][2] = {
  { "amarok", "kde4-amarok.desktop" },
  { "rhythmbox", "rhythmbox.desktop" },
  { "dolphin", "kde4-dolphin.desktop" },
  { "gedit", "gedit.desktop" },
  { "pidgin", "pidgin.desktop" },
  { "firefox", "firefox.desktop" },
  { "smplayer", "smplayer.desktop" },
  NULL,
};

int desktop_cmd = -1;

static void
init (void)
{
  int i;
  for (i = 0; DESKTOP_CANDIDATES[i] != NULL; i++)
  {
    gchar *path = NULL;
    if ((path = g_find_program_in_path (DESKTOP_CANDIDATES[i][0])) != NULL)
    {
      g_free (path);
      desktop_cmd = i;
      break;
    }
  }
}

static void
basic_test (void)
{
  GAppInfo *info = G_APP_INFO (ol_app_info_new (DEFAULT_COMMAND,
                                                DEFAULT_NAME,
                                                DEFAULT_ICON_NAME,
                                                OL_APP_INFO_FLAGS_NONE,
                                                NULL));
  ol_test_expect (strcmp (g_app_info_get_commandline (info), DEFAULT_COMMAND) == 0);
  ol_test_expect (strcmp (g_app_info_get_name (info), DEFAULT_NAME) == 0);
  ol_test_expect (strcmp (g_app_info_get_executable (info), DEFAULT_CMDNAME) == 0);
  ol_test_expect (g_app_info_should_show (info));
  g_object_unref (info);

  info = G_APP_INFO (ol_app_info_new (DEFAULT_COMMAND,
                                      NULL,
                                      NULL,
                                      OL_APP_INFO_FLAGS_NONE,
                                      NULL));
  ol_test_expect (strcmp (g_app_info_get_name (info), DEFAULT_CMDNAME) == 0);
  g_object_unref (info);

  info = G_APP_INFO (ol_app_info_new (COMMAND_NOT_EXIST,
                                      NULL,
                                      NULL,
                                      OL_APP_INFO_FLAGS_NONE,
                                      NULL));
  ol_test_expect (!g_app_info_should_show (info));
  g_object_unref (info);
}

static void
desktop_test (void)
{
  if (desktop_cmd < 0)
    return;
  const gchar *cmd = DESKTOP_CANDIDATES[desktop_cmd][0];
  const gchar *desktop_id = DESKTOP_CANDIDATES[desktop_cmd][1];
  GAppInfo *info = G_APP_INFO (ol_app_info_new (cmd, NULL, NULL,
                                                OL_APP_INFO_PREFER_DESKTOP_FILE,
                                                NULL));
  GAppInfo *desktop_info = G_APP_INFO (g_desktop_app_info_new (desktop_id));
  ol_test_expect (desktop_info != NULL);
  ol_test_expect (strcmp (g_app_info_get_name (info), g_app_info_get_name (desktop_info)) == 0);
  ol_test_expect (strcmp (g_app_info_get_commandline (info), g_app_info_get_commandline (desktop_info)) == 0);
  g_object_unref (desktop_info);
  g_object_unref (info);
}

static void
prefix_test (void)
{
  GAppInfo *info = G_APP_INFO (ol_app_info_new (COMMAND_PREFIX, NULL, NULL,
                                                OL_APP_INFO_WITH_PREFIX,
                                                NULL));
  ol_test_expect (g_app_info_should_show (info));
  ol_test_expect (strcmp (g_app_info_get_commandline (info), DEFAULT_COMMAND) == 0);
  g_object_unref (info);
  info = G_APP_INFO (ol_app_info_new (COMMAND_PREFIX, NULL, NULL,
                                      OL_APP_INFO_FLAGS_NONE,
                                      NULL));
  ol_test_expect (!g_app_info_should_show (info));
  g_object_unref (info);

  if (desktop_cmd >= 0)
  {
    const gchar *cmd = DESKTOP_CANDIDATES[desktop_cmd][0];
    const gchar *cmd_prefix = g_strndup (cmd, strlen (cmd) - 1);
    GAppInfo *prefix_info = G_APP_INFO (ol_app_info_new (cmd_prefix,
                                                         NULL,
                                                         NULL,
                                                         OL_APP_INFO_WITH_PREFIX |
                                                         OL_APP_INFO_USE_DESKTOP_NAME,
                                                         NULL));
    GAppInfo *info = G_APP_INFO (ol_app_info_new (cmd,
                                                  NULL,
                                                  NULL,
                                                  OL_APP_INFO_USE_DESKTOP_NAME,
                                                  NULL));
    ol_test_expect (strcmp (g_app_info_get_name (info), g_app_info_get_name (prefix_info)) == 0);
    g_object_unref (info);
    g_object_unref (prefix_info);
  }
}

static void
quote_test (void)
{
  GError *error = NULL;
  GAppInfo *info = G_APP_INFO (ol_app_info_new (COMMAND_WITH_ARG_SPACE,
                                                NULL,
                                                NULL,
                                                OL_APP_INFO_FLAGS_NONE,
                                                NULL));
  ol_test_expect_streq (g_app_info_get_commandline (info), COMMAND_WITH_ARG_SPACE);
  ol_test_expect_streq (g_app_info_get_executable (info), CMDNAME_WITH_ARG_SPACE);
  g_object_unref (info);
  
  info = G_APP_INFO (ol_app_info_new (COMMAND_WITH_SPACE,
                                      NULL,
                                      NULL,
                                      OL_APP_INFO_FLAGS_NONE,
                                      NULL));
  ol_test_expect_streq (g_app_info_get_executable (info), CMDNAME_WITH_SPACE);
  g_object_unref (info);
}

static void
second_exe_test (void)
{
  GAppInfo *info;
  gchar *cmd = g_strdup_printf ("sudo %s", DEFAULT_COMMAND);
  info = G_APP_INFO (ol_app_info_new (cmd, NULL, NULL,
                                      OL_APP_INFO_SECOND_IS_EXEC,
                                      NULL));
  ol_test_expect_streq (g_app_info_get_commandline (info), cmd);
  ol_test_expect_streq (g_app_info_get_executable (info), DEFAULT_CMDNAME);
  ol_test_expect (g_app_info_should_show (info));
  g_object_unref (info);

  gchar *prefix_cmd = g_strdup_printf ("sudo %s", COMMAND_PREFIX);
  info = G_APP_INFO (ol_app_info_new (cmd, NULL, NULL,
                                      OL_APP_INFO_SECOND_IS_EXEC |
                                      OL_APP_INFO_WITH_PREFIX,
                                      NULL));
  ol_test_expect_streq (g_app_info_get_commandline (info), cmd);
  ol_test_expect_streq (g_app_info_get_executable (info), DEFAULT_CMDNAME);
  ol_test_expect (g_app_info_should_show (info));
  g_object_unref (info);
  g_free (prefix_cmd);
  g_free (cmd);

  cmd = g_strdup_printf ("sudo %s", COMMAND_NOT_EXIST);
  info = G_APP_INFO (ol_app_info_new (cmd, NULL, NULL,
                                      OL_APP_INFO_SECOND_IS_EXEC,
                                      NULL));
  ol_test_expect_streq (g_app_info_get_commandline (info), cmd);
  ol_test_expect_streq (g_app_info_get_executable (info), CMDNAME_NOT_EXIST);
  ol_test_expect (!g_app_info_should_show (info));
  g_object_unref (info);
  g_free (cmd);
}

int
main (int argc, char **argv)
{
  g_type_init ();
  init ();
  basic_test ();
  desktop_test ();
  prefix_test ();
  quote_test ();
  second_exe_test ();
}
