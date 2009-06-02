#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <glib.h>

#include <wocky/wocky-xmpp-scheduler.h>
#include "wocky-test-stream.h"

typedef struct {
  GMainLoop *loop;
  GQueue *expected_stanzas;
  WockyXmppConnection *in;
  WockyXmppConnection *out;
  WockyXmppScheduler *sched_in;
  WockyXmppScheduler *sched_out;
  WockyTestStream *stream;
  guint outstanding;
} test_data_t;

static gboolean
timeout_cb (gpointer data)
{
  g_test_message ("Timeout reached :(");
  g_assert_not_reached ();

  return FALSE;
}

static test_data_t *
setup_test (void)
{
  test_data_t *data;

  data = g_new0 (test_data_t, 1);
  data->loop = g_main_loop_new (NULL, FALSE);

  data->stream = g_object_new (WOCKY_TYPE_TEST_STREAM, NULL);
  data->in = wocky_xmpp_connection_new (data->stream->stream0);
  data->out = wocky_xmpp_connection_new (data->stream->stream1);

  data->sched_in = wocky_xmpp_scheduler_new (data->in);
  data->sched_out = wocky_xmpp_scheduler_new (data->out);

  data->expected_stanzas = g_queue_new ();

  g_timeout_add (1000, timeout_cb, NULL);

  return data;
}

static void
teardown_test (test_data_t *data)
{
  g_main_loop_unref (data->loop);
  g_object_unref (data->stream);
  g_object_unref (data->in);
  g_object_unref (data->out);
  g_object_unref (data->sched_in);
  g_object_unref (data->sched_out);

  /* All the stanzas should have been received */
  g_assert (g_queue_get_length (data->expected_stanzas) == 0);
  g_queue_free (data->expected_stanzas);

  g_free (data);
}

static void
test_wait_pending (test_data_t *test)
{
  while (test->outstanding > 0)
    g_main_loop_run (test->loop);
}

static void
test_instantiation (void)
{
  WockyXmppScheduler *scheduler;
  WockyXmppConnection *connection;
  WockyTestStream *stream;;

  stream = g_object_new (WOCKY_TYPE_TEST_STREAM, NULL);
  connection = wocky_xmpp_connection_new (stream->stream0);
  scheduler = wocky_xmpp_scheduler_new (connection);

  g_assert (scheduler != NULL);

  g_object_unref (scheduler);
  g_object_unref (connection);
  g_object_unref (stream);
}

static void
send_received_open_cb (GObject *source, GAsyncResult *res, gpointer user_data)
{
  WockyXmppConnection *conn = WOCKY_XMPP_CONNECTION (source);
  test_data_t *d = (test_data_t *) user_data;

  g_assert (wocky_xmpp_connection_recv_open_finish (conn, res,
      NULL, NULL, NULL, NULL, NULL));

  d->outstanding--;
  g_main_loop_quit (d->loop);
}

static void
send_open_cb (GObject *source, GAsyncResult *res, gpointer user_data)
{
  test_data_t *data = (test_data_t *) user_data;

  g_assert (wocky_xmpp_connection_send_open_finish (
      WOCKY_XMPP_CONNECTION (source), res, NULL));

  wocky_xmpp_connection_recv_open_async (data->out,
      NULL, send_received_open_cb, user_data);
}

/* Open XMPP connections on both sides */
static void
open_connection (test_data_t *test)
{
  wocky_xmpp_connection_send_open_async (test->in,
      NULL, NULL, NULL, NULL,
      NULL, send_open_cb, test);
  test->outstanding++;

  test_wait_pending (test);
}

static void
wait_close_cb (GObject *source, GAsyncResult *res,
  gpointer user_data)
{
  test_data_t *data = (test_data_t *) user_data;
  WockyXmppStanza *s;
  GError *error = NULL;

  s = wocky_xmpp_connection_recv_stanza_finish (WOCKY_XMPP_CONNECTION (source),
      res, &error);

  g_assert (s == NULL);
  /* connection has been disconnected */
  g_assert (g_error_matches (error, WOCKY_XMPP_CONNECTION_ERROR,
        WOCKY_XMPP_CONNECTION_ERROR_CLOSED));
  g_error_free (error);

  data->outstanding--;
  g_main_loop_quit (data->loop);
}

static void
close_sent_cb (GObject *source, GAsyncResult *res,
  gpointer user_data)
{
  test_data_t *data = (test_data_t *) user_data;

  g_assert (wocky_xmpp_connection_send_close_finish (
    WOCKY_XMPP_CONNECTION (source), res, NULL));

  data->outstanding--;
  g_main_loop_quit (data->loop);
}

/* Close XMPP connections on both sides */
static void
close_connection (test_data_t *test)
{
  wocky_xmpp_connection_recv_stanza_async (test->out, NULL, wait_close_cb,
      test);
  test->outstanding++;

  /* Close the connection */
  wocky_xmpp_connection_send_close_async (
    WOCKY_XMPP_CONNECTION (test->in),
    NULL, close_sent_cb, test);
  test->outstanding++;

  test_wait_pending (test);
}

/* send testing */

static void
send_stanza_received_cb (GObject *source, GAsyncResult *res,
  gpointer user_data)
{
  WockyXmppConnection *connection = WOCKY_XMPP_CONNECTION (source);
  WockyXmppStanza *s;
  test_data_t *data = (test_data_t *) user_data;
  GError *error = NULL;
  WockyXmppStanza *expected;

  s = wocky_xmpp_connection_recv_stanza_finish (connection, res, &error);
  g_assert (s != NULL);

  expected = g_queue_pop_head (data->expected_stanzas);
  g_assert (expected != NULL);

  g_assert (wocky_xmpp_node_equal (s->node, expected->node));

  if (g_queue_get_length (data->expected_stanzas) > 0)
    {
      /* We need to receive more stanzas */
      wocky_xmpp_connection_recv_stanza_async (
          WOCKY_XMPP_CONNECTION (source), NULL, send_stanza_received_cb,
          user_data);

      data->outstanding++;
    }

  g_object_unref (s);
  g_object_unref (expected);

  data->outstanding--;
  g_main_loop_quit (data->loop);
}

static void
send_stanza_cb (GObject *source, GAsyncResult *res, gpointer user_data)
{
  test_data_t *data = (test_data_t *) user_data;
  g_assert (wocky_xmpp_scheduler_send_full_finish (
      WOCKY_XMPP_SCHEDULER (source), res, NULL));

  data->outstanding--;
  g_main_loop_quit (data->loop);
}

static void
send_stanza_cancelled_cb (GObject *source, GAsyncResult *res,
  gpointer user_data)
{
  test_data_t *data = (test_data_t *) user_data;
  GError *error = NULL;

  g_assert (!wocky_xmpp_scheduler_send_full_finish (
      WOCKY_XMPP_SCHEDULER (source), res, &error));
  g_assert (error->domain == G_IO_ERROR);
  g_assert (error->code == G_IO_ERROR_CANCELLED);
  g_error_free (error);

  data->outstanding--;
  g_main_loop_quit (data->loop);
}

static void
test_send (void)
{
  test_data_t *test = setup_test ();
  WockyXmppStanza *s;
  GCancellable *cancellable;

  open_connection (test);

  /* Send a stanza */
  s = wocky_xmpp_stanza_build (WOCKY_STANZA_TYPE_MESSAGE,
    WOCKY_STANZA_SUB_TYPE_CHAT, "juliet@example.com", "romeo@example.net",
    WOCKY_STANZA_END);

  wocky_xmpp_scheduler_send_full (test->sched_in, s, NULL, send_stanza_cb,
      test);
  g_queue_push_tail (test->expected_stanzas, s);
  test->outstanding++;

  /* Send a stanza */
  s = wocky_xmpp_stanza_build (WOCKY_STANZA_TYPE_MESSAGE,
    WOCKY_STANZA_SUB_TYPE_CHAT, "juliet@example.com", "tybalt@example.net",
    WOCKY_STANZA_END);

  wocky_xmpp_scheduler_send_full (test->sched_in, s, NULL, send_stanza_cb,
      test);
  g_queue_push_tail (test->expected_stanzas, s);
  test->outstanding++;

  /* Send two stanzas and cancel them immediately */
  cancellable = g_cancellable_new ();

  s = wocky_xmpp_stanza_build (WOCKY_STANZA_TYPE_MESSAGE,
    WOCKY_STANZA_SUB_TYPE_CHAT, "juliet@example.com", "peter@example.net",
    WOCKY_STANZA_END);

  wocky_xmpp_scheduler_send_full (test->sched_in, s, cancellable,
      send_stanza_cancelled_cb, test);
  g_object_unref (s);
  test->outstanding++;

  s = wocky_xmpp_stanza_build (WOCKY_STANZA_TYPE_MESSAGE,
    WOCKY_STANZA_SUB_TYPE_CHAT, "juliet@example.com", "samson@example.net",
    WOCKY_STANZA_END);

  wocky_xmpp_scheduler_send_full (test->sched_in, s, cancellable,
      send_stanza_cancelled_cb, test);
  g_object_unref (s);
  test->outstanding++;

  /* the stanza are not added to expected_stanzas as it was cancelled */
  g_cancellable_cancel (cancellable);

  /* ... and a second (using the simple send method) */
  s = wocky_xmpp_stanza_build (WOCKY_STANZA_TYPE_MESSAGE,
    WOCKY_STANZA_SUB_TYPE_CHAT, "juliet@example.com", "nurse@example.net",
    WOCKY_STANZA_END);

  wocky_xmpp_scheduler_send (test->sched_in, s);
  g_queue_push_tail (test->expected_stanzas, s);
  test->outstanding++;

  wocky_xmpp_connection_recv_stanza_async (test->out, NULL,
      send_stanza_received_cb, test);

  g_object_unref (cancellable);

  test_wait_pending (test);

  close_connection (test);
  teardown_test (test);
}

int
main (int argc, char **argv)
{
  g_thread_init (NULL);

  g_test_init (&argc, &argv, NULL);
  g_type_init ();

  g_test_add_func ("/xmpp-scheduler/initiation", test_instantiation);
  g_test_add_func ("/xmpp-scheduler/send", test_send);
  return g_test_run ();
}
