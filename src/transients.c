/*      $Id$

        This program is free software; you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 2, or (at your option)
        any later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program; if not, write to the Free Software
        Foundation, Inc., Inc., 51 Franklin Street, Fifth Floor, Boston,
        MA 02110-1301, USA.


        xfwm4    - (c) 2002-2011 Olivier Fourdan

 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#include <libxfce4util/libxfce4util.h>

#include "screen.h"
#include "client.h"
#include "stacking.h"
#include "transients.h"

Client *
clientGetTransient (Client * c)
{
    g_return_val_if_fail (c != NULL, NULL);

    TRACE ("entering clientGetTransient");

    if ((c->transient_for) && (c->transient_for != c->screen_info->xroot))
    {
        return myScreenGetClientFromWindow (c->screen_info, c->transient_for, SEARCH_WINDOW);
    }
    return NULL;
}

gboolean
clientIsTransient (Client * c)
{
    g_return_val_if_fail (c != NULL, FALSE);

    TRACE ("entering clientIsTransient");

    return (((c->transient_for != c->screen_info->xroot) && (c->transient_for != None) && (c->transient_for != c->window)) ||
            ((c->transient_for == c->screen_info->xroot) && (c->group_leader != None) && (c->group_leader != c->window)));
}

gboolean
clientIsModal (Client * c)
{
    g_return_val_if_fail (c != NULL, FALSE);

    TRACE ("entering clientIsModal");
    /*
       If the WM_TRANSIENT_FOR hint is set to another toplevel window, the dialog is modal for that window;
       if WM_TRANSIENT_FOR is not set or set to the root window the dialog is modal for its window group.
     */
    return (FLAG_TEST (c->flags, CLIENT_FLAG_STATE_MODAL) && (c->type & WINDOW_REGULAR_FOCUSABLE) &&
            (((c->transient_for != c->screen_info->xroot) && (c->transient_for != None) && (c->transient_for != c->window)) ||
             ((c->group_leader != None) && (c->group_leader != c->window))));
}

gboolean
clientIsTransientOrModal (Client * c)
{
    g_return_val_if_fail (c != NULL, FALSE);

    TRACE ("entering clientIsTransientOrModal");

    return (clientIsTransient(c) || clientIsModal(c));
}

gboolean
clientSameGroup (Client * c1, Client * c2)
{
    g_return_val_if_fail (c1 != NULL, FALSE);
    g_return_val_if_fail (c2 != NULL, FALSE);

    TRACE ("entering clientSameGroup");

    return ((c1 != c2) &&
            (((c1->group_leader != None) &&
              (c1->group_leader == c2->group_leader)) ||
             (c1->group_leader == c2->window) ||
             (c2->group_leader == c1->window)));
}

gboolean
clientSameLeader (Client * c1, Client * c2)
{
    g_return_val_if_fail (c1 != NULL, FALSE);
    g_return_val_if_fail (c2 != NULL, FALSE);

    TRACE ("entering clientSameLeader");

    return ((c1 != c2) &&
            (((c1->client_leader != None) &&
              (c1->client_leader == c2->client_leader)) ||
             (c1->client_leader == c2->window) ||
             (c2->client_leader == c1->window)));
}

gboolean
clientIsTransientFor (Client * c1, Client * c2)
{
    g_return_val_if_fail (c1 != NULL, FALSE);
    g_return_val_if_fail (c2 != NULL, FALSE);

    TRACE ("entering clientIsTransientFor");

    if (c1->transient_for)
    {
        if (c1->transient_for != c1->screen_info->xroot)
        {
            return (c1->transient_for == c2->window);
        }
        /*
         * Transients for group shouldn't apply to other transients, or
         * it breaks stacking for some apps, noticeably mozilla "save as"
         * dialog...
         */
        else if (c2->transient_for == None)
        {
            return (clientSameGroup (c1, c2));
        }
    }
    return FALSE;
}

gboolean
clientIsModalFor (Client * c1, Client * c2)
{
    g_return_val_if_fail (c1 != NULL, FALSE);
    g_return_val_if_fail (c2 != NULL, FALSE);

    TRACE ("entering clientIsModalFor");

    if (FLAG_TEST (c1->flags, CLIENT_FLAG_STATE_MODAL) && (c1->type & WINDOW_REGULAR_FOCUSABLE) && (c1->serial >= c2->serial))
    {
        /*
         * We used to consider all windows of the same group here
         * (ie. "clientSameGroup (c1, c2)") but that seems too
         * wide so we restric modality to transient relationship
         * for now.
         */
        return (clientIsTransientFor (c1, c2));
    }
    return FALSE;
}

gboolean
clientIsTransientOrModalFor (Client * c1, Client * c2)
{
    g_return_val_if_fail (c1 != NULL, FALSE);
    g_return_val_if_fail (c2 != NULL, FALSE);

    TRACE ("entering clientIsTransientOrModalFor");

    return (clientIsTransientFor(c1, c2) || clientIsModalFor(c1, c2));
}

gboolean
clientIsTransientForGroup (Client * c)
{
    g_return_val_if_fail (c != NULL, FALSE);

    TRACE ("entering clientIsTransientForGroup");

    return ((c->transient_for == c->screen_info->xroot) && (c->group_leader != None));
}

gboolean
clientIsModalForGroup (Client * c)
{
    g_return_val_if_fail (c != NULL, FALSE);

    TRACE ("entering clientIsModalForGroup");

    return (FLAG_TEST (c->flags, CLIENT_FLAG_STATE_MODAL) && (c->type & WINDOW_REGULAR_FOCUSABLE) &&
            !clientIsTransient(c) && (c->group_leader != None));
}

gboolean
clientIsTransientOrModalForGroup (Client * c)
{
    g_return_val_if_fail (c != NULL, FALSE);

    TRACE ("entering clientIsTransientOrModalForGroup");

    return (clientIsTransientForGroup(c) || clientIsModalForGroup(c));
}

gboolean
clientIsValidTransientOrModal (Client * c)
{
    g_return_val_if_fail (c != NULL, FALSE);

    TRACE ("entering clientIsValidTransientOrModal");
    if (clientIsTransientOrModalForGroup (c))
    {
        ScreenInfo *screen_info = c->screen_info;
        GList *list;

        /* Look for a valid transient or modal for the same group */
        for (list = screen_info->windows_stack; list; list = g_list_next (list))
        {
            Client *c2 = (Client *) list->data;
            if (c2 != c)
            {
                if (clientIsTransientOrModalFor (c, c2))
                {
                    /* We found one, look no further */
                    return TRUE;
                }
            }
        }
    }
    else if (clientIsTransientOrModal (c))
    {
        return (clientGetTransient (c) != NULL);
    }

    return (FALSE);
}

gboolean
clientTransientOrModalHasAncestor (Client * c, guint ws)
{
    Client *c2;
    GList *list;
    ScreenInfo *screen_info;

    g_return_val_if_fail (c != NULL, FALSE);

    TRACE ("entering clientTransientOrModalHasAncestor");

    if (!clientIsTransientOrModal (c))
    {
        return FALSE;
    }

    screen_info = c->screen_info;
    for (list = screen_info->windows_stack; list; list = g_list_next (list))
    {
        c2 = (Client *) list->data;
        if ((c2 != c)
            && !clientIsTransientOrModal (c2)
            && clientIsTransientOrModalFor (c, c2)
            && FLAG_TEST (c2->xfwm_flags, XFWM_FLAG_VISIBLE)
            && (c2->win_workspace == ws)
            && (((ws == screen_info->current_ws) && FLAG_TEST (c2->xfwm_flags, XFWM_FLAG_VISIBLE))
                || !FLAG_TEST (c2->flags, CLIENT_FLAG_ICONIFIED)))
        {
            return TRUE;
        }
    }
    return FALSE;

}

Client *
clientGetModalFor (Client * c)
{
    ScreenInfo *screen_info;
    Client *c2;
    GList *list;

    g_return_val_if_fail (c != NULL, NULL);
    TRACE ("entering clientGetModalFor");

    screen_info = c->screen_info;
    for (list = g_list_last(screen_info->windows_stack); list; list = g_list_previous (list))
    {
        c2 = (Client *) list->data;
        if (c2)
        {
            if ((c2 != c) && clientIsModalFor (c2, c))
            {
                return c2;
            }
        }
    }
    return NULL;
}

Client *
clientGetTransientFor (Client * c)
{
    ScreenInfo *screen_info;
    Client *latest_transient;
    Client *c2;
    GList *list;

    g_return_val_if_fail (c != NULL, NULL);
    TRACE ("entering clientGetTransientFor");

    latest_transient = c;
    screen_info = c->screen_info;
    for (list = g_list_last(screen_info->windows_stack); list; list = g_list_previous (list))
    {
        if (!clientIsTransient (latest_transient))
        {
            break;
        }
        c2 = (Client *) list->data;
        if (c2)
        {
            if (clientIsTransientFor (latest_transient, c2))
            {
                latest_transient = c2;
            }
        }
    }

    return latest_transient;
}

/* Build a GList of clients that have a transient relationship */
GList *
clientListTransient (Client * c)
{
    ScreenInfo *screen_info;
    Client *c2, *c3;
    GList *transients;
    GList *list1, *list2;

    g_return_val_if_fail (c != NULL, NULL);

    transients = NULL;
    screen_info = c->screen_info;
    transients = g_list_append (transients, c);
    for (list1 = screen_info->windows_stack; list1; list1 = g_list_next (list1))
    {
        c2 = (Client *) list1->data;
        if (c2 != c)
        {
            if (clientIsTransientFor (c2, c))
            {
                transients = g_list_append (transients, c2);
            }
            else
            {
                for (list2 = transients; list2;
                    list2 = g_list_next (list2))
                {
                    c3 = (Client *) list2->data;
                    if ((c3 != c2) && clientIsTransientFor (c2, c3))
                    {
                        transients = g_list_append (transients, c2);
                        break;
                    }
                }
            }
        }
    }
    return transients;
}

/* Build a GList of clients that have a transient or modal relationship */
GList *
clientListTransientOrModal (Client * c)
{
    ScreenInfo *screen_info;
    Client *c2, *c3;
    GList *transients;
    GList *list1, *list2;

    g_return_val_if_fail (c != NULL, NULL);

    screen_info = c->screen_info;
    transients = NULL;
    transients = g_list_append (transients, c);
    for (list1 = screen_info->windows_stack; list1; list1 = g_list_next (list1))
    {
        c2 = (Client *) list1->data;
        if (c2 != c)
        {
            if (clientIsTransientOrModalFor (c2, c))
            {
                transients = g_list_append (transients, c2);
            }
            else
            {
                for (list2 = transients; list2;
                    list2 = g_list_next (list2))
                {
                    c3 = (Client *) list2->data;
                    if ((c3 != c2) && clientIsTransientOrModalFor (c2, c3))
                    {
                        transients = g_list_append (transients, c2);
                        break;
                    }
                }
            }
        }
    }
    return transients;
}

/* Check if a window is not already listed in transients of a client.
   That's to avoid potential self transient relationship...
 */
gboolean
clientCheckTransientWindow (Client *c, Window w)
{
    GList *transients;
    GList *list;
    Client *c2;

    g_return_val_if_fail (c != NULL, FALSE);

    transients = clientListTransient (c);
    for (list = transients; list; list = g_list_next (list))
    {
        c2 = (Client *) list->data;
        if (c2->window == w)
        {
            g_list_free (transients);
            return FALSE;
        }
    }
    g_list_free (transients);
    return TRUE;
}

gboolean
clientSameApplication (Client *c1, Client *c2)
{
    g_return_val_if_fail (c1 != NULL, FALSE);
    g_return_val_if_fail (c2 != NULL, FALSE);

    return (clientIsTransientOrModalFor (c1, c2) ||
            clientIsTransientOrModalFor (c2, c1) ||
            clientSameGroup (c1, c2) ||
            clientSameLeader (c1, c2) ||
            (c1->pid != 0 && c1->pid == c2->pid &&
             c1->hostname && c2->hostname &&
             !g_strcasecmp (c1->hostname, c2->hostname)));
}
