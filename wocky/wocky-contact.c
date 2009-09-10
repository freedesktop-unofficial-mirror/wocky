/*
 * wocky-contact.c - Source for WockyContact
 * Copyright (C) 2009 Collabora Ltd.
 * @author Guillaume Desmottes <guillaume.desmottes@collabora.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * SECTION: wocky-contact
 * @title: WockyContact
 * @short_description:
 * @include: wocky/wocky-contact.h
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <gio/gio.h>

#include "wocky-contact.h"
#include "wocky-signals-marshal.h"
#include "wocky-utils.h"

#define DEBUG_FLAG DEBUG_ROSTER
#include "wocky-debug.h"

G_DEFINE_TYPE (WockyContact, wocky_contact, G_TYPE_OBJECT)

/* signal enum */
enum
{
  LAST_SIGNAL,
};

/*
static guint signals[LAST_SIGNAL] = {0};
*/

/* private structure */
typedef struct _WockyContactPrivate WockyContactPrivate;

struct _WockyContactPrivate
{
  gboolean dispose_has_run;
};

#define WOCKY_CONTACT_GET_PRIVATE(o)  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((o), WOCKY_TYPE_CONTACT, \
    WockyContactPrivate))

static void
wocky_contact_init (WockyContact *obj)
{
  /*
  WockyContact *self = WOCKY_CONTACT (obj);
  WockyContactPrivate *priv = WOCKY_CONTACT_GET_PRIVATE (self);
  */
}

static void
wocky_contact_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  /*
  WockyContactPrivate *priv =
      WOCKY_CONTACT_GET_PRIVATE (object);
  */

  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
wocky_contact_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  /*
  WockyContactPrivate *priv =
      WOCKY_CONTACT_GET_PRIVATE (object);
  */

  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
wocky_contact_constructed (GObject *object)
{
}

static void
wocky_contact_dispose (GObject *object)
{
  WockyContact *self = WOCKY_CONTACT (object);
  WockyContactPrivate *priv = WOCKY_CONTACT_GET_PRIVATE (self);

  if (priv->dispose_has_run)
    return;

  priv->dispose_has_run = TRUE;

  if (G_OBJECT_CLASS (wocky_contact_parent_class)->dispose)
    G_OBJECT_CLASS (wocky_contact_parent_class)->dispose (object);
}

static void
wocky_contact_finalize (GObject *object)
{
  /*
  WockyContact *self = WOCKY_CONTACT (object);
  WockyContactPrivate *priv = WOCKY_CONTACT_GET_PRIVATE (self);
  */

  G_OBJECT_CLASS (wocky_contact_parent_class)->finalize (object);
}

static void
wocky_contact_class_init (WockyContactClass *wocky_contact_class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (wocky_contact_class);

  g_type_class_add_private (wocky_contact_class,
      sizeof (WockyContactPrivate));

  object_class->constructed = wocky_contact_constructed;
  object_class->set_property = wocky_contact_set_property;
  object_class->get_property = wocky_contact_get_property;
  object_class->dispose = wocky_contact_dispose;
  object_class->finalize = wocky_contact_finalize;
}