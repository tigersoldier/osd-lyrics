#ifndef _OL_OSD_TOOLBAR_H_
#define _OL_OSD_TOOLBAR_H_

#include <gtk/gtk.h>
#include "ol_player.h"

#define OL_OSD_TOOLBAR(obj)                  GTK_CHECK_CAST (obj, ol_osd_toolbar_get_type (), OlOsdToolbar)
#define OL_OSD_TOOLBAR_CLASS(klass)          GTK_CHECK_CLASS_CAST (klass, ol_osd_toolbar_get_type (), OlOsdToolbarClass)
#define OL_IS_OSD_TOOLBAR(obj)               GTK_CHECK_TYPE (obj, ol_osd_toolbar_get_type ())
#define OL_OSD_TOOLBAR_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), ol_osd_toolbar_get_type (), OlOsdToolbarClass))

typedef struct _OlOsdToolbar OlOsdToolbar;
typedef struct _OlOsdToolbarClass OlOsdToolbarClass;

struct _OlOsdToolbar
{
  GtkAlignment alignment;
  GtkHBox *center_box;
  GtkButton *play_button;
  GtkButton *pause_button;
  GtkButton *prev_button;
  GtkButton *next_button;
  GtkButton *stop_button;
};

struct _OlOsdToolbarClass
{
  GtkAlignmentClass parent_class;
};

GtkType ol_osd_toolbar_get_type (void);

GtkWidget *ol_osd_toolbar_new (void);
void ol_osd_toolbar_set_player (OlOsdToolbar *toolbar, OlPlayerController *player);
void ol_osd_toolbar_set_status (OlOsdToolbar *toolbar, enum OlPlayerStatus status);

#endif /* _OL_OSD_TOOLBAR_H_ */
