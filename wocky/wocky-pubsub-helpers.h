/*
 * wocky-pubsub-internal.h — header for PubSub helper functions
 * Copyright © 2009–2010 Collabora Ltd.
 * Copyright © 2010 Nokia Corporation
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

#ifndef WOCKY_PUBSUB_HELPERS_H
#define WOCKY_PUBSUB_HELPERS_H

#include <gio/gio.h>

#include "wocky-xmpp-stanza.h"

WockyXmppStanza *wocky_pubsub_make_publish_stanza (
    const gchar *service,
    const gchar *node,
    WockyXmppNode **publish_out,
    WockyXmppNode **item_out);

gboolean wocky_pubsub_distill_iq_reply (GObject *source,
    GAsyncResult *res,
    const gchar *pubsub_ns,
    const gchar *child_name,
    WockyXmppNode **child_out,
    GError **error);

#endif /* WOCKY_PUBSUB_HELPERS_H */
