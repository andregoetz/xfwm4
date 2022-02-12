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


        oroborus - (c) 2001 Ken Lynch
        xfwm4    - (c) 2002-2011 Olivier Fourdan

 */

#ifndef INC_KEYBOARD_H
#define INC_KEYBOARD_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "device.h"

#include <X11/keysym.h>

typedef struct _MyKey MyKey;
struct _MyKey
{
    KeyCode keycode;
    /*
     * Modifier, key pressed along with myKey
     * e.g. ctrl + a where ctrl is the modifier of myKey a:
     * no modifier    = 0
     * shift          = 1
     * ctrl           = 4
     * alt            = 8
     * super          = 64
     * with combinations the value gets added:
     * ctrl + shift   = 4 + 1 = 5
     */
    guint modifier;
    gchar *internal_name;
};

extern unsigned int AltMask;
extern unsigned int MetaMask;
extern unsigned int NumLockMask;
extern unsigned int ScrollLockMask;
extern unsigned int SuperMask;
extern unsigned int HyperMask;

gboolean                 getModifierMap                         (const char *,
                                                                 guint *);
void                     parseKeyString                         (Display *,
                                                                 MyKey *,
                                                                 const char *);
gboolean                 grabKey                                (XfwmDevices *,
                                                                 Display *,
                                                                 MyKey *,
                                                                 Window);
void                     ungrabKeys                             (XfwmDevices *,
                                                                 Display *,
                                                                 Window);
gboolean                 grabButton                             (XfwmDevices *,
                                                                 Display *,
                                                                 guint,
                                                                 guint,
                                                                 Window);
void                     ungrabButton                           (XfwmDevices *,
                                                                 Display *,
                                                                 guint,
                                                                 guint,
                                                                 Window);
void                     initModifiers                          (Display *);

#endif /* INC_KEYBOARD_H */
