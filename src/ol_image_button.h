#ifndef _OL_IMAGE_BUTTON_H_
#define _OL_IMAGE_BUTTON_H_

#include <gtk/gtk.h>

#define OL_IMAGE_BUTTON(obj)                  GTK_CHECK_CAST (obj, ol_image_button_get_type (), OlImageButton)
#define OL_IMAGE_BUTTON_CLASS(klass)          GTK_CHECK_CLASS_CAST (klass, ol_image_button_get_type (), OlImageButtonClass)
#define OL_IS_IMAGE_BUTTON(obj)               GTK_CHECK_TYPE (obj, ol_image_button_get_type ())
#define OL_IMAGE_BUTTON_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), ol_image_button_get_type (), OlImageButtonClass))

typedef struct _OlImageButton OlImageButton;
typedef struct _OlImageButtonClass OlImageButtonClass;

struct _OlImageButton
{
  GtkButton button;
};

struct _OlImageButtonClass
{
  GtkButtonClass button_class;
};

GtkType ol_image_button_get_type (void);

/** 
 * @brief Create a new image button
 * 
 * 
 * @return A new instance of image button
 */
GtkWidget *ol_image_button_new (void);

/** 
 * @brief Sets the image of the button
 *
 * The image should contains 4 frames: normal, hover, pressed, disabled, from
 * left to right. The width of each frame must be equal.
 * 
 * @param btn 
 * @param image The image of the button
 */
void ol_image_button_set_pixbuf (OlImageButton *btn, GdkPixbuf *image); 

#endif /* _OL_IMAGE_BUTTON_H_ */
