/* -*- mode: C; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/*
 * Copyright (C) 2011  Tiger Soldier <tigersoldi@gmail.com>
 *
 * This file is part of OSD Lyrics.
 * 
 * OSD Lyrics is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OSD Lyrics is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OSD Lyrics.  If not, see <http://www.gnu.org/licenses/>. 
 */

#include <gio/gio.h>
#include "ol_player.h"
#include "ol_consts.h"
#include "ol_elapse_emulator.h"
#include "ol_debug.h"

#define assert_variant_type(value, type)                        \
  do {                                                          \
    if (!g_variant_is_of_type ((value), G_VARIANT_TYPE (type))) \
    {                                                           \
      ol_errorf ("Except variant type of %s, but %s got\n",     \
                 type, g_variant_get_type_string ((value)));    \
    }                                                           \
  } while (0);

#define OL_PLAYER_GET_PRIVATE(object)                                   \
  (G_TYPE_INSTANCE_GET_PRIVATE ((object), OL_TYPE_PLAYER, OlPlayerPrivate))

const int POSITION_ACCURACY_MS = 1000;

enum Mpris1Status {
  MPRIS1_STATUS_PLAYING = 0,
  MPRIS1_STATUS_PAUSED = 1,
  MPRIS1_STATUS_STOPPED = 2,
};

enum Mpris1Caps {
  MPRIS1_CAPS_NONE              = 0,
  MPRIS1_CAPS_NEXT              = 1 << 0,
  MPRIS1_CAPS_PREV              = 1 << 1,
  MPRIS1_CAPS_PAUSE             = 1 << 2,
  MPRIS1_CAPS_PLAY              = 1 << 3,
  MPRIS1_CAPS_SEEK              = 1 << 4,
  MPRIS1_CAPS_METADATA          = 1 << 5,
  MPRIS1_CAPS_TRACKLIST         = 1 << 6,
};

enum {
  PLAYER_LOST,
  PLAYER_CONNECTED,
  TRACK_CHANGED,
  STATUS_CHANGED,
  CAPS_CHANGED,
  LAST_SIGNAL,
};

typedef struct _OlPlayerPrivate OlPlayerPrivate;

struct _OlPlayerPrivate
{
  GDBusProxy *proxy;
  GDBusProxy *mpris1_proxy;
  gboolean connected;
  OlMetadata *metadata;
  enum OlPlayerStatus status;
  enum OlPlayerCaps caps;
  guint64 position;
  guint position_timer;
  gchar *player_name;
  gchar *player_icon;
  GCancellable *cancel_player_info;
  GCancellable *cancel_metadata;
  GCancellable *cancel_position;
  GCancellable *cancel_status;
  GCancellable *cancel_caps;
  OlElapseEmulator *elapse_emulator;
  gboolean initialized;
};

static guint signals[LAST_SIGNAL] = { 0 };

static void ol_player_class_init (OlPlayerClass *klass);
static void ol_player_init (OlPlayer *player);
static void ol_player_init_proxy (OlPlayer *player);
static void ol_player_init_mpris1_proxy (OlPlayer *player);
static void ol_player_finalize (GObject *object);
/* static void ol_player_set_property (GObject *object, */
/*                                     guint property_id, */
/*                                     const GValue *value, */
/*                                     GParamSpec *pspec); */
/* static void ol_player_get_property (GObject *object, */
/*                                     guint property_id, */
/*                                     GValue *value, */
/*                                     GParamSpec *pspec); */
static void ol_player_proxy_signal (GDBusProxy *proxy,
                                    gchar *sender_name,
                                    gchar *signal_name,
                                    GVariant *parameters,
                                    gpointer user_data);
static void ol_player_mpris1_proxy_signal (GDBusProxy *proxy,
                                           gchar *sender_name,
                                           gchar *signal_name,
                                           GVariant *parameters,
                                           gpointer user_data);

static void ol_player_fetch_player_info_async (OlPlayer *player);
static void ol_player_fetch_player_info_sync (OlPlayer *player);
static void ol_player_fetch_player_info_cb (GObject *source_object,
                                            GAsyncResult *res,
                                            gpointer user_data);
static void ol_player_set_player_info (OlPlayer *player,
                                       GVariant *value);

static void ol_player_fetch_position_async (OlPlayer *player);
static void ol_player_fetch_position_cb (GObject *source_object,
                                         GAsyncResult *res,
                                         gpointer user_data);
static void ol_player_set_position (OlPlayer *player,
                                    GVariant *value);

static void ol_player_fetch_metadata_async (OlPlayer *player);
static void ol_player_fetch_metadata_cb (GObject *source_object,
                                         GAsyncResult *res,
                                         gpointer user_data);
static void ol_player_set_metadata (OlPlayer *player,
                                    GVariant *value);

static void ol_player_fetch_status_async (OlPlayer *player);
static void ol_player_fetch_status_cb (GObject *source_object,
                                       GAsyncResult *res,
                                       gpointer user_data);
static void ol_player_set_status (OlPlayer *player,
                                  GVariant *value);

static void ol_player_fetch_caps_async (OlPlayer *player);
static void ol_player_fetch_caps_cb (GObject *source_object,
                                     GAsyncResult *res,
                                     gpointer user_data);
static void ol_player_set_caps (OlPlayer *player,
                                GVariant *value);

static void ol_player_start_position_timer (OlPlayer *player);
static void ol_player_stop_position_timer (OlPlayer *player);

static void ol_player_proxy_name_owner_changed (GObject *gobject,
                                                GParamSpec *pspec,
                                                gpointer user_data);
static void _cancel_call (GCancellable **cancellable);

G_DEFINE_TYPE (OlPlayer, ol_player, G_TYPE_OBJECT);

static void
ol_player_class_init (OlPlayerClass *klass)
{
  GObjectClass *gklass = G_OBJECT_CLASS (klass);

  /* gklass->set_property = ol_player_set_property; */
  /* gklass->get_property = ol_player_get_property; */
  gklass->finalize = ol_player_finalize;
  
  signals[PLAYER_LOST] =
    g_signal_new ("player-lost",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,            /* class_offset */
                  NULL, NULL,   /* accumulator, accu_data */
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  signals[PLAYER_CONNECTED] =
    g_signal_new ("player-connected",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,            /* class_offset */
                  NULL, NULL,   /* accumulator, accu_data */
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  signals[TRACK_CHANGED] =
    g_signal_new ("track-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,            /* class_offset */
                  NULL, NULL,   /* accumulator, accu_data */
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  signals[STATUS_CHANGED] =
    g_signal_new ("status-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,            /* class_offset */
                  NULL, NULL,   /* accumulator, accu_data */
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  signals[CAPS_CHANGED] =
    g_signal_new ("caps-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,            /* class_offset */
                  NULL, NULL,   /* accumulator, accu_data */
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_type_class_add_private (klass, sizeof (OlPlayerPrivate));
}

OlPlayer *
ol_player_new (void)
{
  return OL_PLAYER (g_object_new (OL_TYPE_PLAYER, NULL));
}

static void
ol_player_init (OlPlayer *player)
{
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  private->metadata = ol_metadata_new ();
  private->status = OL_PLAYER_UNKNOWN;
  private->caps = 0;
  private->position = 0;
  private->initialized = FALSE;
  /* TODO: Initialize player info */
  private->elapse_emulator = ol_elapse_emulator_new (0, POSITION_ACCURACY_MS);
  ol_player_init_proxy (player);
  ol_player_init_mpris1_proxy (player);
  ol_player_fetch_player_info_async (player);
}

static void
ol_player_init_proxy (OlPlayer *player)
{
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  GError *error = NULL;
  private->proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                                  G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                                                  NULL, /* GDBusInterfaceInfo */
                                                  OL_SERVICE_DAEMON,
                                                  OL_OBJECT_PLAYER,
                                                  OL_IFACE_PLAYER,
                                                  NULL, /* GCancellable */
                                                  &error);
  if (private->proxy == NULL && error != NULL) {
    ol_errorf ("Cannot connect to player object: %s", error->message);
    g_error_free (error);
    return;
  }
  g_signal_connect (private->proxy,
                    "g-signal",
                    G_CALLBACK (ol_player_proxy_signal),
                    player);
}

static void
ol_player_init_mpris1_proxy (OlPlayer *player)
{
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  GError *error = NULL;
  private->mpris1_proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                                         G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                                                         NULL, /* GDBusInterfaceInfo */
                                                         OL_SERVICE_DAEMON,
                                                         OL_OBJECT_MPRIS1_PLAYER,
                                                         OL_IFACE_MPRIS1,
                                                         NULL, /* GCancellable */
                                                         &error);
  if (private->mpris1_proxy == NULL && error != NULL) {
    ol_errorf ("Cannot connect to MPRIS1 player object: %s", error->message);
    g_error_free (error);
    return;
  }
  g_signal_connect (private->mpris1_proxy,
                    "g-signal",
                    G_CALLBACK (ol_player_mpris1_proxy_signal),
                    player);
  g_signal_connect (private->mpris1_proxy,
                    "notify::g-name-owner",
                    G_CALLBACK (ol_player_proxy_name_owner_changed),
                    player);
}

static void
_cancel_call (GCancellable **cancellable)
{
  if (cancellable && *cancellable)
  {
    g_cancellable_cancel (*cancellable);
    g_object_unref (*cancellable);
    *cancellable = NULL;
  }
}

static void
ol_player_finalize (GObject *object)
{
  OlPlayer *player = OL_PLAYER (object);
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (object);
  if (private->proxy != NULL)
  {
    g_object_unref (private->proxy);
    private->proxy = NULL;
  }
  if (private->mpris1_proxy != NULL)
  {
    g_object_unref (private->mpris1_proxy);
    private->mpris1_proxy = NULL;
  }
  if (private->metadata != NULL)
  {
    ol_metadata_free (private->metadata);
    private->metadata = NULL;
  }
  if (private->player_name != NULL)
  {
    g_free (private->player_name);
    private->player_name = NULL;
  }
  if (private->player_icon != NULL)
  {
    g_free (private->player_icon);
    private->player_icon = NULL;
  }
  _cancel_call (&private->cancel_player_info);
  _cancel_call (&private->cancel_metadata);
  _cancel_call (&private->cancel_position);
  _cancel_call (&private->cancel_caps);
  _cancel_call (&private->cancel_status);
  ol_elapse_emulator_free (private->elapse_emulator);
  private->elapse_emulator = NULL;
  ol_player_stop_position_timer (player);
  G_OBJECT_CLASS (ol_player_parent_class)->finalize (object);
}

static void
ol_player_proxy_signal (GDBusProxy *proxy,
                        gchar *sender_name,
                        gchar *signal_name,
                        GVariant *parameters,
                        gpointer user_data)
{
  ol_assert (OL_IS_PLAYER (user_data));
  OlPlayer *player = OL_PLAYER (user_data);
  if (g_strcmp0 (signal_name, "PlayerLost") == 0)
  {
    ol_player_set_player_info (player, NULL);
    g_signal_emit (player, signals[PLAYER_LOST], 0);
  }
  else if (g_strcmp0 (signal_name, "PlayerConnected") == 0)
  {
    assert_variant_type (parameters, "(a{sv})");
    GVariant *value;
    g_variant_get (parameters, "(@a{sv})", &value);
    ol_player_set_player_info (player, value);
    g_variant_unref (value);
    g_signal_emit (player, signals[PLAYER_CONNECTED], 0);
  }
}

static void
ol_player_mpris1_proxy_signal (GDBusProxy *proxy,
                               gchar *sender_name,
                               gchar *signal_name,
                               GVariant *parameters,
                               gpointer user_data)
{
  ol_assert (OL_IS_PLAYER (user_data));
  OlPlayer *player = OL_PLAYER (user_data);
  if (g_strcmp0 (signal_name, "TrackChange") == 0)
  {
    ol_player_set_metadata (player, parameters);
    g_signal_emit (player, signals[TRACK_CHANGED], 0);
  }
  else if (g_strcmp0 (signal_name, "StatusChange") == 0)
  {
    ol_player_set_status (player, parameters);
    g_signal_emit (player, signals[STATUS_CHANGED], 0);
  }
  else if (g_strcmp0 (signal_name, "CapsChange") == 0)
  {
    ol_player_set_caps (player, parameters);
    g_signal_emit (player, signals[CAPS_CHANGED], 0);
  }
}

static void
ol_player_proxy_name_owner_changed (GObject *gobject,
                                    GParamSpec *pspec,
                                    gpointer user_data)
{
  ol_assert (OL_IS_PLAYER (user_data));
  OlPlayer *player = OL_PLAYER (user_data);
  gchar *owner = NULL;
  if ((owner = g_dbus_proxy_get_name_owner (G_DBUS_PROXY (gobject))) == NULL)
  {
    /* Daemon disconnected, treat the player as lost */
    ol_debug ("Daemon lost");
    ol_player_set_player_info (player, NULL);
  }
  else
  {
    /* Daemon connected, get player info */
    ol_debugf ("Daemon connected: %s\n", owner);
    ol_player_fetch_player_info_async (player);
  }
}

static void
ol_player_fetch_player_info_sync (OlPlayer *player)
{
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  GError *error = NULL;
  GVariant *value = NULL;
  value = g_dbus_proxy_call_sync (private->proxy,
                                  "GetCurrentPlayer",
                                  NULL, /* parameters */
                                  G_DBUS_CALL_FLAGS_NO_AUTO_START,
                                  -1,   /* timeout_msec */
                                  NULL, /* cancellable */
                                  &error);
  if (value == NULL)
  {
    ol_debugf ("Cannot get player info: %s\n", error->message);
    g_error_free (error);
  }
  else
  {
    assert_variant_type (value, "(ba{sv})");
    gboolean connected;
    GVariant *info = NULL;
    g_variant_get (value, "(b@a{sv})", &connected, &info);
    if (connected)
      ol_player_set_player_info (player, info);
    else
      ol_player_set_player_info (player, NULL);
    g_variant_unref (info);
    g_variant_unref (value);
  }
}

static void
ol_player_fetch_player_info_async (OlPlayer *player)
{
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  if (private->cancel_player_info)
    return;
  private->cancel_player_info = g_cancellable_new ();
  g_dbus_proxy_call (private->proxy,
                     "GetCurrentPlayer",
                     NULL,      /* parameters */
                     G_DBUS_CALL_FLAGS_NO_AUTO_START,
                     -1,        /* timeout_msec */
                     private->cancel_player_info,
                     ol_player_fetch_player_info_cb,
                     player);
}

static void
ol_player_fetch_player_info_cb (GObject *source_object,
                                GAsyncResult *res,
                                gpointer user_data)
{
  ol_assert (OL_IS_PLAYER (user_data));
  OlPlayer *player = OL_PLAYER (user_data);
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  GError *error = NULL;
  GVariant *value =  g_dbus_proxy_call_finish (G_DBUS_PROXY (source_object),
                                               res,
                                               &error);
  if (value)
  {
    assert_variant_type (value, "(ba{sv})");
    gboolean connected;
    GVariant *info = NULL;
    g_variant_get (value, "(b@a{sv})", &connected, &info);
    if (connected)
      ol_player_set_player_info (player, info);
    else
      ol_player_set_player_info (player, NULL);
    g_variant_unref (info);
    g_variant_unref (value);
  }
  else
  {
    ol_errorf ("Cannot get player info: %s\n", error->message);
    ol_player_set_player_info (player, NULL);
    g_error_free (error);
  }
  if (private->cancel_player_info)
  {
    g_object_unref (private->cancel_player_info);
    
  }
}

static void
ol_player_set_player_info (OlPlayer *player,
                           GVariant *value)
{
  ol_assert (OL_IS_PLAYER (player));
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  private->initialized = TRUE;
  g_free (private->player_name);
  private->player_name = NULL;
  g_free (private->player_icon);
  private->player_icon = NULL;
  if (value == NULL)
  {
    /* Player lost, clear the properties */
    if (private->connected)
    {
      ol_player_set_metadata (player, NULL);
      ol_player_set_position (player, NULL);
      ol_player_set_status (player, NULL);
      ol_player_set_caps (player, NULL);
      private->connected = FALSE;
    }
  }
  else
  {
    assert_variant_type (value, "a{sv}");
    GVariantIter *iter = NULL;
    g_variant_get (value, "a{sv}", &iter);
    gchar *key;
    GVariant *dict_value;
    while (g_variant_iter_loop (iter, "{sv}", &key, &dict_value))
    {
      if (g_strcmp0 (key, "name") == 0)
      {
        const gchar *name = g_variant_get_string (dict_value, NULL);
        if (name)
        {
          private->player_name = g_strdup (name);
        }
        else
        {
          ol_errorf ("Cannot get the name of the player. Value type: %s",
                     g_variant_get_type_string (dict_value));
        }
      }
      else if (g_strcmp0 (key, "appname") == 0)
      {
        /* TODO: get appname */
      }
      else if (g_strcmp0 (key, "binname") == 0)
      {
        /* TODO: get binname */
      }
      else if (g_strcmp0 (key, "cmd") == 0)
      {
        /* TODO: get cmd */
      }
      else if (g_strcmp0 (key, "icon") == 0)
      {
        const gchar *icon = g_variant_get_string (dict_value, NULL);
        if (icon)
        {
          private->player_icon = g_strdup (icon);
        }
        else
        {
          ol_errorf ("Cannot get the icon of the player. Value type: %s",
                     g_variant_get_type_string (dict_value));
        }
      }
    }
    ol_debugf ("player name: %s, icon: %s\n",
               private->player_name,
               private->player_icon);
    g_variant_iter_free (iter);
    ol_player_fetch_metadata_async (player);
    ol_player_fetch_status_async (player);
    ol_player_fetch_caps_async (player);
    private->connected = TRUE;
  }
}

static void
ol_player_fetch_position_async (OlPlayer *player)
{
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  if (private->cancel_position)
  {
    return;
  }
  private->cancel_position = g_cancellable_new ();
  g_dbus_proxy_call (private->mpris1_proxy,
                     "PositionGet",
                     NULL,      /* parameters */
                     G_DBUS_CALL_FLAGS_NO_AUTO_START,
                     -1,        /* timeout_msec */
                     private->cancel_position,
                     ol_player_fetch_position_cb,
                     player);
}

static void
ol_player_fetch_position_cb (GObject *source_object,
                             GAsyncResult *res,
                             gpointer user_data)
{
  ol_assert (OL_IS_PLAYER (user_data));
  OlPlayer *player = OL_PLAYER (user_data);
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (user_data);
  GError *error = NULL;
  GVariant *value = g_dbus_proxy_call_finish (G_DBUS_PROXY (source_object),
                                              res,
                                              &error);
  if (value)
  {
    ol_player_set_position (player, value);
    g_variant_unref (value);
  }
  else
  {
    ol_player_set_position (player, NULL);
    ol_errorf ("Cannot get position of player: %s\n",
               error->message);
    g_error_free (error);
  }
  if (private->cancel_position)
  {
    g_object_unref (private->cancel_position);
    private->cancel_position = NULL;
  }
}

static void
ol_player_set_position (OlPlayer *player,
                        GVariant *value)
{
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  if (value == NULL)
  {
    private->position = 0;
  }
  else
  {
    assert_variant_type (value, "(i)");
    gint32 pos;
    g_variant_get (value, "(i)", &pos);
    private->position = pos;
  }
}

static void
ol_player_fetch_metadata_async (OlPlayer *player)
{
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  if (private->cancel_metadata)
    return;
  private->cancel_metadata = g_cancellable_new ();
  g_dbus_proxy_call (private->mpris1_proxy,
                     "GetMetadata",
                     NULL,      /* parameters */
                     G_DBUS_CALL_FLAGS_NO_AUTO_START,
                     -1,        /* timeout_msec */
                     private->cancel_metadata,
                     ol_player_fetch_metadata_cb,
                     player);
}

static void
ol_player_fetch_metadata_cb (GObject *source_object,
                             GAsyncResult *res,
                             gpointer user_data)
{
  ol_assert (OL_IS_PLAYER (user_data));
  OlPlayer *player = OL_PLAYER (user_data);
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (user_data);
  GError *error = NULL;
  GVariant *value = g_dbus_proxy_call_finish (G_DBUS_PROXY (source_object),
                                              res,
                                              &error);
  if (value)
  {
    ol_player_set_metadata (player, value);
    g_variant_unref (value);
  }
  else
  {
    ol_player_set_metadata (player, NULL);
    ol_errorf ("Cannot get metadata of player: %s\n",
               error->message);
    g_error_free (error);
  }
  if (private->cancel_metadata)
  {
    g_object_unref (private->cancel_metadata);
    private->cancel_metadata = NULL;
  }
  g_signal_emit (player, signals[TRACK_CHANGED], 0);
}

static void
ol_player_set_metadata (OlPlayer *player,
                        GVariant *value)
{
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  ol_metadata_clear (private->metadata);
  if (value != NULL)
  {
    ol_assert (g_variant_is_of_type (value, G_VARIANT_TYPE ("(a{sv})")));
    GVariantIter *iter;
    gchar *key;
    GVariant *dict_value;
    g_variant_get (value, "(a{sv})", &iter);
    while (g_variant_iter_loop (iter, "{sv}", &key, &dict_value))
    {
      if (g_strcmp0 (key, "title") == 0)
        ol_metadata_set_title (private->metadata,
                                 g_variant_get_string (dict_value, NULL));
      else if (g_strcmp0 (key, "artist") == 0)
        ol_metadata_set_artist (private->metadata,
                                  g_variant_get_string (dict_value, NULL));
      else if (g_strcmp0 (key, "album") == 0)
        ol_metadata_set_album (private->metadata,
                                 g_variant_get_string (dict_value, NULL));
      else if (g_strcmp0 (key, "location") == 0)
        ol_metadata_set_uri (private->metadata,
                               g_variant_get_string (dict_value, NULL));
      else if (g_strcmp0 (key, "tracknumber") == 0)
        ol_metadata_set_track_number_from_string (private->metadata,
                                                    g_variant_get_string (dict_value, NULL));
      else if (g_strcmp0 (key, "mtime") == 0)
        ol_metadata_set_duration (private->metadata,
                                  g_variant_get_uint32 (dict_value));
      /* else if (g_strcmp0 (key, "arturl") == 0) */
      /*   ol_metadata_set_album_art (private->metadata, */
      /*                                g_variant_get_string (dict_value, NULL)); */
      else if (g_strcmp0 (key, "arturl") == 0)
        ol_metadata_set_art (private->metadata,
                             g_variant_get_string (dict_value, NULL));
    }
    g_variant_iter_free (iter);
    ol_debugf ("Update metadata:\n"
               "  title: %s\n"
               "  artist: %s\n"
               "  album: %s\n"
               "  uri: %s\n"
               "  track_num: %d\n"
               "  duration: %"G_GUINT64_FORMAT"\n"
               "  album art: %s\n",
               ol_metadata_get_title (private->metadata),
               ol_metadata_get_artist (private->metadata),
               ol_metadata_get_album (private->metadata),
               ol_metadata_get_uri (private->metadata),
               ol_metadata_get_track_number (private->metadata),
               ol_metadata_get_duration (private->metadata),
               ol_metadata_get_art (private->metadata));
  }
  /* The position is likely to change when the track is changed, so we
     need to update to the new value. */
  ol_player_fetch_position_async (player);
}

static void
ol_player_fetch_status_async (OlPlayer *player)
{
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  if (private->cancel_status)
    return;
  private->cancel_status = g_cancellable_new ();
  g_dbus_proxy_call (private->mpris1_proxy,
                     "GetStatus",
                     NULL,      /* parameters */
                     G_DBUS_CALL_FLAGS_NO_AUTO_START,
                     -1,        /* timeout_msec */
                     private->cancel_status,
                     ol_player_fetch_status_cb,
                     player);
}

static void
ol_player_fetch_status_cb (GObject *source_object,
                           GAsyncResult *res,
                           gpointer user_data)
{
  ol_assert (OL_IS_PLAYER (user_data));
  OlPlayer *player = OL_PLAYER (user_data);
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (user_data);
  GError *error = NULL;
  GVariant *value = g_dbus_proxy_call_finish (G_DBUS_PROXY (source_object),
                                              res,
                                              &error);
  if (value)
  {
    ol_player_set_status (player, value);
    g_variant_unref (value);
  }
  else
  {
    ol_player_set_status (player, NULL);
    ol_errorf ("Cannot get status of player: %s\n",
               error->message);
    g_error_free (error);
  }
  if (private->cancel_status)
  {
    g_object_unref (private->cancel_status);
    private->cancel_status = NULL;
  }
  g_signal_emit (player, signals[STATUS_CHANGED], 0);
}

static void
ol_player_set_status (OlPlayer *player,
                      GVariant *value)
{
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  if (value != NULL)
  {
    if (!g_variant_is_of_type (value, G_VARIANT_TYPE ("((iiii))")))
    {
      ol_errorf ("Cannot get status of the player, expect value type of (iiii), "
                 "but %s got\n",
                 g_variant_get_type_string (value));
      private->status = OL_PLAYER_ERROR;
    }
    else
    {
      gint32 statusid = -1;
      g_variant_get (value, "((iiii))", &statusid, NULL, NULL, NULL);
      switch (statusid)
      {
      case MPRIS1_STATUS_PLAYING:
        private->status = OL_PLAYER_PLAYING;
        ol_player_start_position_timer (player);
        break;
      case MPRIS1_STATUS_PAUSED:
        private->status = OL_PLAYER_PAUSED;
        /* Even if the status is paused, users may change the position
           manually, so we need to keep updating the position. */
        ol_player_start_position_timer (player);
        break;
      case MPRIS1_STATUS_STOPPED:
        private->status = OL_PLAYER_STOPPED;
        ol_player_stop_position_timer (player);
        break;
      default:
        private->status = OL_PLAYER_ERROR;
      }
      ol_debugf ("Update player status: %d\n", private->status);
    }
  }
  else
  {
    private->status = OL_PLAYER_UNKNOWN;
    ol_player_stop_position_timer (player);
  }
}

static void
ol_player_fetch_caps_async (OlPlayer *player)
{
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  if (private->cancel_caps)
    return;
  private->cancel_caps = g_cancellable_new ();
  g_dbus_proxy_call (private->mpris1_proxy,
                     "GetCaps",
                     NULL,      /* parameters */
                     G_DBUS_CALL_FLAGS_NO_AUTO_START,
                     -1,        /* timeout_msec */
                     private->cancel_caps,
                     ol_player_fetch_caps_cb,
                     player);
}

static void
ol_player_fetch_caps_cb (GObject *source_object,
                         GAsyncResult *res,
                         gpointer user_data)
{
  ol_assert (OL_IS_PLAYER (user_data));
  OlPlayer *player = OL_PLAYER (user_data);
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (user_data);
  GError *error = NULL;
  GVariant *value = g_dbus_proxy_call_finish (G_DBUS_PROXY (source_object),
                                              res,
                                              &error);
  if (value)
  {
    ol_player_set_caps (player, value);
    g_variant_unref (value);
  }
  else
  {
    ol_player_set_caps (player, NULL);
    ol_errorf ("Cannot get capabilities of player: %s\n",
               error->message);
    g_error_free (error);
  }
  if (private->cancel_caps)
  {
    g_object_unref (private->cancel_caps);
    private->cancel_caps = NULL;
  }
  g_signal_emit (player, signals[CAPS_CHANGED], 0);
}

static void
ol_player_set_caps (OlPlayer *player,
                    GVariant *value)
{
  const static int CAPS_MAPS[][2] = {
    { MPRIS1_CAPS_NEXT, OL_PLAYER_NEXT },
    { MPRIS1_CAPS_PREV, OL_PLAYER_PREV },
    { MPRIS1_CAPS_PLAY, OL_PLAYER_PLAY },
    { MPRIS1_CAPS_PAUSE, OL_PLAYER_PAUSE },
    { MPRIS1_CAPS_SEEK, OL_PLAYER_SEEK },
  };
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  private->caps = 0;
  if (value)
  {
    assert_variant_type (value, "(i)");
    gint32 caps;
    g_variant_get (value, "(i)", &caps);
    int i;
    for (i = 0; i < G_N_ELEMENTS (CAPS_MAPS); i++)
    {
      if ((caps & CAPS_MAPS[i][0]) == CAPS_MAPS[i][0])
        private->caps |= CAPS_MAPS[i][1];
    }
    /* MPRIS1 didn't define stop capability, assuming it exists */
    private->caps |= OL_PLAYER_STOP;
    ol_debugf ("Update player caps: %d\n", private->caps);
  }
}

static gboolean
ol_player_position_timer_cb (OlPlayer *player)
{
  ol_player_fetch_position_async (player);
  return TRUE;
}

static void
ol_player_start_position_timer (OlPlayer *player)
{
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  if (private->position_timer)
    return;
  /* Fetch position once the timer starts */
  ol_player_fetch_position_async (player);
  private->position_timer = g_timeout_add (POSITION_ACCURACY_MS,
                                           (GSourceFunc)ol_player_position_timer_cb,
                                           player);
}

static void
ol_player_stop_position_timer (OlPlayer *player)
{
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  if (private->position_timer)
  {
    g_source_remove (private->position_timer);
    private->position_timer = 0;
  }
}

gboolean
ol_player_is_connected (OlPlayer *player)
{
  ol_assert_ret (OL_IS_PLAYER (player), FALSE);
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  if (!private->initialized)
  {
    ol_player_fetch_player_info_sync (player);
  }
  return private->connected;
}

const char*
ol_player_get_name (OlPlayer *player)
{
  ol_assert_ret (OL_IS_PLAYER (player), NULL);
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  if (!private->initialized)
  {
    ol_player_fetch_player_info_sync (player);
  }
  return private->player_name;
}

const char*
ol_player_get_icon_path (OlPlayer *player)
{
  ol_assert_ret (OL_IS_PLAYER (player), NULL);
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  if (!private->initialized)
  {
    ol_player_fetch_player_info_sync (player);
  }
  return private->player_icon;
}

gboolean
ol_player_get_metadata (OlPlayer *player, OlMetadata *metadata)
{
  ol_assert_ret (OL_IS_PLAYER (player), FALSE);
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  if (!private->connected)
    return FALSE;
  if (metadata != NULL)
    ol_metadata_copy (metadata, private->metadata);
  return TRUE;
}

gboolean
ol_player_get_position (OlPlayer *player, guint64 *pos_ms)
{
  ol_assert_ret (OL_IS_PLAYER (player), FALSE);
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  if (!private->connected)
    return FALSE;
  if (pos_ms)
  {
    if (private->status != OL_PLAYER_PLAYING)
      *pos_ms = ol_elapse_emulator_get_last_ms (private->elapse_emulator,
                                                private->position);
    else
      *pos_ms = ol_elapse_emulator_get_real_ms (private->elapse_emulator,
                                                private->position);
  }
  return TRUE;
}

enum OlPlayerStatus
ol_player_get_status (OlPlayer *player)
{
  ol_assert_ret (OL_IS_PLAYER (player), OL_PLAYER_ERROR);
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  return private->status;
}

int
ol_player_get_caps (OlPlayer *player)
{
  ol_assert_ret (OL_IS_PLAYER (player), -1);
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  return private->caps;
}

gboolean
ol_player_play (OlPlayer *player)
{
  ol_assert_ret (OL_IS_PLAYER (player), FALSE);
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  if (!private->connected || !(private->caps & OL_PLAYER_PLAY))
    return FALSE;
  g_dbus_proxy_call (private->mpris1_proxy,
                     "Play",
                     NULL,      /* parameters */
                     G_DBUS_CALL_FLAGS_NO_AUTO_START,
                     -1,        /* timeout_msec */
                     NULL,      /* cancellable */
                     NULL,      /* callback */
                     NULL);     /* user_data */
  return TRUE;
}

gboolean
ol_player_pause (OlPlayer *player)
{
  ol_assert_ret (OL_IS_PLAYER (player), FALSE);
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  if (!private->connected || !(private->caps & OL_PLAYER_PAUSE))
    return FALSE;
  g_dbus_proxy_call (private->mpris1_proxy,
                     "Pause",
                     NULL,      /* parameters */
                     G_DBUS_CALL_FLAGS_NO_AUTO_START,
                     -1,        /* timeout_msec */
                     NULL,      /* cancellable */
                     NULL,      /* callback */
                     NULL);     /* user_data */
  return TRUE;
}

gboolean
ol_player_stop (OlPlayer *player)
{
  ol_assert_ret (OL_IS_PLAYER (player), FALSE);
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  if (!private->connected || !(private->caps & OL_PLAYER_STOP))
    return FALSE;
  g_dbus_proxy_call (private->mpris1_proxy,
                     "Stop",
                     NULL,      /* parameters */
                     G_DBUS_CALL_FLAGS_NO_AUTO_START,
                     -1,        /* timeout_msec */
                     NULL,      /* cancellable */
                     NULL,      /* callback */
                     NULL);     /* user_data */
  return TRUE;
}

gboolean
ol_player_prev (OlPlayer *player)
{
  ol_assert_ret (OL_IS_PLAYER (player), FALSE);
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  if (!private->connected || !(private->caps & OL_PLAYER_PREV))
    return FALSE;
  g_dbus_proxy_call (private->mpris1_proxy,
                     "Prev",
                     NULL,      /* parameters */
                     G_DBUS_CALL_FLAGS_NO_AUTO_START,
                     -1,        /* timeout_msec */
                     NULL,      /* cancellable */
                     NULL,      /* callback */
                     NULL);     /* user_data */
  return TRUE;
}

gboolean
ol_player_next (OlPlayer *player)
{
  ol_assert_ret (OL_IS_PLAYER (player), FALSE);
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  if (!private->connected || !(private->caps & OL_PLAYER_NEXT))
    return FALSE;
  g_dbus_proxy_call (private->mpris1_proxy,
                     "Next",
                     NULL,      /* parameters */
                     G_DBUS_CALL_FLAGS_NO_AUTO_START,
                     -1,        /* timeout_msec */
                     NULL,      /* cancellable */
                     NULL,      /* callback */
                     NULL);     /* user_data */
  return TRUE;
}

gboolean
ol_player_seek (OlPlayer *player, guint64 pos_ms)
{
  ol_assert_ret (OL_IS_PLAYER (player), FALSE);
  OlPlayerPrivate *private = OL_PLAYER_GET_PRIVATE (player);
  if (!private->connected || !(private->caps & OL_PLAYER_SEEK))
    return FALSE;
  g_dbus_proxy_call (private->mpris1_proxy,
                     "PositionSet",
                     g_variant_new ("(i)", (gint32)pos_ms),
                     G_DBUS_CALL_FLAGS_NO_AUTO_START,
                     -1,        /* timeout_msec */
                     NULL,      /* cancellable */
                     NULL,      /* callback */
                     NULL);     /* user_data */
  private->position = pos_ms;
  ol_player_fetch_position_async (player);
  return TRUE;
}
