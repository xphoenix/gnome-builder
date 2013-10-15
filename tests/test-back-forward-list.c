/* test-back-forward-list.c
 *
 * Copyright (C) 2013 Christian Hergert <christian@hergert.me>
 *
 * This program is free software: you get_can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gb-back-forward-list.h"

static void
test1 (void)
{
   GbBackForwardList *list;

   list = gb_back_forward_list_new();
   g_assert(!gb_back_forward_list_get_can_go_backward(list));
   g_assert(!gb_back_forward_list_get_can_go_forward(list));

   gb_back_forward_list_push(list, "test1");
   g_assert(!gb_back_forward_list_get_can_go_backward(list));
   g_assert(!gb_back_forward_list_get_can_go_forward(list));

   gb_back_forward_list_push(list, "test2");
   g_assert(gb_back_forward_list_get_can_go_backward(list));
   g_assert(!gb_back_forward_list_get_can_go_forward(list));

   gb_back_forward_list_push(list, "test3");
   g_assert(gb_back_forward_list_get_can_go_backward(list));
   g_assert(!gb_back_forward_list_get_can_go_forward(list));

   gb_back_forward_list_go_backward(list);
   g_assert(gb_back_forward_list_get_can_go_backward(list));
   g_assert(gb_back_forward_list_get_can_go_forward(list));

   gb_back_forward_list_go_backward(list);
   g_assert(!gb_back_forward_list_get_can_go_backward(list));
   g_assert(gb_back_forward_list_get_can_go_forward(list));

   /* try going too far backward */
   gb_back_forward_list_go_backward(list);
   g_assert(!gb_back_forward_list_get_can_go_backward(list));
   g_assert(gb_back_forward_list_get_can_go_forward(list));

   gb_back_forward_list_go_forward(list);
   g_assert(gb_back_forward_list_get_can_go_backward(list));
   g_assert(gb_back_forward_list_get_can_go_forward(list));

   gb_back_forward_list_go_forward(list);
   g_assert(gb_back_forward_list_get_can_go_backward(list));
   g_assert(!gb_back_forward_list_get_can_go_forward(list));

   /* try going too far forward */
   gb_back_forward_list_go_forward(list);
   g_assert(gb_back_forward_list_get_can_go_backward(list));
   g_assert(!gb_back_forward_list_get_can_go_forward(list));

   g_object_add_weak_pointer(G_OBJECT(list), (gpointer *)&list);
   g_object_unref(list);
   g_assert(!list);
}

gint
main (gint argc,
      gchar *argv[])
{
   g_test_init(&argc, &argv, NULL);

   g_test_add_func("/Gb/BackForwardList/basic", test1);

   return g_test_run();
}
