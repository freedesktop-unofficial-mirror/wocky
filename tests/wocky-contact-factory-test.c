#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <glib.h>

#include <wocky/wocky-contact-factory.h>
#include <wocky/wocky-utils.h>

#include "wocky-test-helper.h"

static void
test_instantiation (void)
{
  WockyContactFactory *factory;

  factory = wocky_contact_factory_new ();

  g_object_unref (factory);
}

static void
test_ensure_bare_contact(void)
{
  WockyContactFactory *factory;
  WockyBareContact *juliet, *a, *b;

  factory = wocky_contact_factory_new ();
  juliet = wocky_bare_contact_new ("juliet@example.org");

  a = wocky_contact_factory_ensure_bare_contact (factory, "juliet@example.org");
  g_assert (wocky_bare_contact_equal (a, juliet));
  b = wocky_contact_factory_ensure_bare_contact (factory, "juliet@example.org");
  g_assert (a == b);

  g_object_unref (factory);
  g_object_unref (juliet);
  g_object_unref (a);
  g_object_unref (b);
}

static void
test_lookup_bare_contact (void)
{
  WockyContactFactory *factory;
  WockyBareContact *contact;

  factory = wocky_contact_factory_new ();

  g_assert (wocky_contact_factory_lookup_bare_contact (factory,
        "juliet@example.org") == NULL);

  contact = wocky_contact_factory_ensure_bare_contact (factory,
      "juliet@example.org");

  g_assert (wocky_contact_factory_lookup_bare_contact (factory,
        "juliet@example.org") != NULL);

  g_object_unref (contact);

  /* contact has been disposed and so is not in the factory anymore */
  g_assert (wocky_contact_factory_lookup_bare_contact (factory,
        "juliet@example.org") == NULL);

  g_object_unref (factory);
}

static void
test_ensure_resource_contact(void)
{
  WockyContactFactory *factory;
  WockyBareContact *juliet;
  WockyResourceContact *juliet_balcony, *juliet_pub, *a, *b;

  factory = wocky_contact_factory_new ();
  juliet = wocky_bare_contact_new ("juliet@example.org");
  juliet_balcony = wocky_resource_contact_new (juliet, "Balcony");
  juliet_pub = wocky_resource_contact_new (juliet, "Pub");

  /* Bare contact isn't in the factory yet */
  a = wocky_contact_factory_ensure_resource_contact (factory,
      "juliet@example.org/Balcony");
  g_assert (wocky_resource_contact_equal (a, juliet_balcony));
  g_assert (wocky_contact_factory_lookup_bare_contact (factory,
        "juliet@example.org") != NULL);

  /* Bare contact is already in the factory */
  b = wocky_contact_factory_ensure_resource_contact (factory,
      "juliet@example.org/Pub");
  g_assert (wocky_resource_contact_equal (b, juliet_pub));

  g_object_unref (factory);
  g_object_unref (juliet);
  g_object_unref (juliet_balcony);
  g_object_unref (juliet_pub);
  g_object_unref (a);
  g_object_unref (b);
}

int
main (int argc, char **argv)
{
  int result;

  test_init (argc, argv);

  g_test_add_func ("/contact-factory/instantiation", test_instantiation);
  g_test_add_func ("/contact-factory/ensure-bare-contact",
      test_ensure_bare_contact);
  g_test_add_func ("/contact-factory/lookup-bare-contact",
      test_lookup_bare_contact);
  g_test_add_func ("/contact-factory/ensure-resource-contact",
      test_ensure_resource_contact);

  result = g_test_run ();
  test_deinit ();
  return result;
}
