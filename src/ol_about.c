#include <gtk/gtk.h>
#include <glade/glade.h>
#include "config.h"
#include "ol_intl.h"
#include "ol_about.h"
#include "ol_glade.h"

static gboolean ol_about_destroy (GtkWidget *widget);

static gboolean
ol_about_destroy (GtkWidget *widget)
{
  gtk_widget_hide (widget);
  puts ("asdf");
  return FALSE;
}

void
ol_about_show ()
{
  /* static GtkWidget *window = NULL; */
  /* if (window == NULL) */
  /* { */
  /*   window = ol_glade_get_widget ("aboutdialog"); */
  /*   g_signal_connect (window, "close", G_CALLBACK (ol_about_destroy), NULL); */
  /* } */
  /* g_return_if_fail (window != NULL); */
  /* gtk_dialog_run (GTK_DIALOG (window)); */
  const char *authors[] = {"Tiger Soldier <tigersoldi@gmail.com>",
                           "SarlmolApple <sarlmolapple@gmail.com>",
                           "SimplyZhao <simplyzhao@gmail.com>", NULL};
  gtk_show_about_dialog (NULL,
                         "program-name", PACKAGE,
                         "version", PACKAGE_VERSION,
                         "license", "GPLv3",
                         "comments", _("An OSD lyrics show supports multiple players"),
                         "copyright", _("Copyright 2009 The OSD Lyrics project."),
                         /* "authors", "Tiger Soldier <tigersoldi@gmail.com>\n" */
                         /*              "SarlmolApple <sarlmolapple@gmail.com>\n" */
                         /*              "SimplyZhao <simplyzhao@gmail.com>", */
                         "authors", authors,
                         "translator-credits", "",
                         "website", "http://code.google.com/p/osd-lyrics/",
                         NULL);
  /* static GtkWidget *dialog = NULL; */
  /* if (dialog == NULL) */
  /* { */
  /*   dialog = gtk_about_dialog_new (); */
  /*   gtk_about_dialog_set_name (dialog, PACKAGE); */
  /* } */
  /* gtk_widget_show (dialog); */
}
