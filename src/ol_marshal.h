
#ifndef __ol_marshal_MARSHAL_H__
#define __ol_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:STRING,STRING (marshal:1) */
extern void ol_marshal_VOID__STRING_STRING (GClosure     *closure,
                                            GValue       *return_value,
                                            guint         n_param_values,
                                            const GValue *param_values,
                                            gpointer      invocation_hint,
                                            gpointer      marshal_data);

/* VOID:UINT,DOUBLE (marshal:2) */
extern void ol_marshal_VOID__UINT_DOUBLE (GClosure     *closure,
                                          GValue       *return_value,
                                          guint         n_param_values,
                                          const GValue *param_values,
                                          gpointer      invocation_hint,
                                          gpointer      marshal_data);

/* VOID:ENUM,POINTER (marshal:3) */
extern void ol_marshal_VOID__ENUM_POINTER (GClosure     *closure,
                                           GValue       *return_value,
                                           guint         n_param_values,
                                           const GValue *param_values,
                                           gpointer      invocation_hint,
                                           gpointer      marshal_data);

/* VOID:ENUM,STRING,UINT (marshal:4) */
extern void ol_marshal_VOID__ENUM_STRING_UINT (GClosure     *closure,
                                               GValue       *return_value,
                                               guint         n_param_values,
                                               const GValue *param_values,
                                               gpointer      invocation_hint,
                                               gpointer      marshal_data);

G_END_DECLS

#endif /* __ol_marshal_MARSHAL_H__ */

