/* vi:set sw=2 sts=2 ts=2 et ai tw=100: */
/*-
 * Copyright (c) 2008 Stephan Arts <stephan@xfce.org>
 * Copyright (c) 2008 Jannis Pohlmann <jannis@xfce.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

#include <glade/glade.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>
#include <xfconf/xfconf.h>
#include <libxfce4kbd-private/xfce-shortcut-dialog.h>
#include <libxfce4kbd-private/xfce-shortcuts-provider.h>

#include "xfwm4-dialog_glade.h"
#include "xfwm4-settings.h"



#define DEFAULT_THEME             "Default"

#define INDICATOR_SIZE            9

#define SHORTCUTS_NAME_COLUMN     0
#define SHORTCUTS_FEATURE_COLUMN  1
#define SHORTCUTS_SHORTCUT_COLUMN 2



#define XFWM_SETTINGS_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
                                                                     XFWM_TYPE_SETTINGS, \
                                                                     XfwmSettingsPrivate))



typedef struct _MenuTemplate     MenuTemplate;
typedef struct _ShortcutTemplate ShortcutTemplate;



/* Property identifiers */
enum
{
  PROP_0,
  PROP_GLADE_XML,
};



static void       xfwm_settings_class_init                           (XfwmSettingsClass    *klass);
static void       xfwm_settings_init                                 (XfwmSettings         *settings);
static void       xfwm_settings_constructed                          (GObject              *object);
static void       xfwm_settings_finalize                             (GObject              *object);
static void       xfwm_settings_get_property                         (GObject              *object,
                                                                      guint                 prop_id,
                                                                      GValue               *value,
                                                                      GParamSpec           *pspec);
static void       xfwm_settings_set_property                         (GObject              *object,
                                                                      guint                 prop_id,
                                                                      const GValue         *value,
                                                                      GParamSpec           *pspec);
static gint       xfwm_settings_theme_sort_func                      (GtkTreeModel         *model,
                                                                      GtkTreeIter          *iter1,
                                                                      GtkTreeIter          *iter2);
static void       xfwm_settings_load_themes                          (XfwmSettings         *settings);
static void       xfwm_settings_theme_selection_changed              (GtkTreeSelection     *selection,
                                                                      XfwmSettings         *settings);
static void       xfwm_settings_title_alignment_changed              (GtkComboBox          *combo,
                                                                      XfwmSettings         *settings);
static void       xfwm_settings_active_frame_drag_data               (GtkWidget            *widget,
                                                                      GdkDragContext       *drag_context,
                                                                      gint                  x,
                                                                      gint                  y,
                                                                      GtkSelectionData     *data,
                                                                      guint                 info,
                                                                      guint                 time,
                                                                      XfwmSettings         *settings);
static gboolean   xfwm_settings_active_frame_drag_motion             (GtkWidget            *widget,
                                                                      GdkDragContext       *drag_context,
                                                                      gint                  x,
                                                                      gint                  y,
                                                                      guint                 time,
                                                                      XfwmSettings         *settings);
static void       xfwm_settings_active_frame_drag_leave              (GtkWidget            *widget,
                                                                      GdkDragContext       *drag_context,
                                                                      guint                 time,
                                                                      XfwmSettings         *settings);
static void       xfwm_settings_hidden_frame_drag_data               (GtkWidget            *widget,
                                                                      GdkDragContext       *drag_context,
                                                                      gint                  x,
                                                                      gint                  y,
                                                                      GtkSelectionData     *data,
                                                                      guint                 info,
                                                                      guint                 time,
                                                                      XfwmSettings         *settings);
static void       xfwm_settings_delete_indicator                     (GtkWidget            *widget);
static void       xfwm_settings_create_indicator                     (GtkWidget            *widget,
                                                                      gint                  x,
                                                                      gint                  y,
                                                                      gint                  width,
                                                                      gint                  height);
static void       xfwm_settings_title_button_drag_data               (GtkWidget            *widget,
                                                                      GdkDragContext       *drag_context,
                                                                      GtkSelectionData     *data,
                                                                      guint                 info,
                                                                      guint                 time);
static void       xfwm_settings_title_button_drag_begin              (GtkWidget            *widget,
                                                                      GdkDragContext       *drag_context);
static void       xfwm_settings_title_button_drag_end                (GtkWidget            *widget,
                                                                      GdkDragContext       *drag_context);
static gboolean   xfwm_settings_signal_blocker                       (GtkWidget            *widget);
static GdkPixbuf *xfwm_settings_create_icon_from_widget              (GtkWidget            *widget);

static void       xfwm_settings_save_button_layout                   (XfwmSettings          *settings,
                                                                      GtkContainer          *container);
static void       xfwm_settings_double_click_action_changed          (GtkComboBox           *combo,
                                                                      XfwmSettings          *settings);
static void       xfwm_settings_title_button_alignment_changed       (GtkComboBox           *combo,
                                                                      GtkWidget             *button);

static void       xfwm_settings_button_layout_property_changed       (XfconfChannel         *channel,
                                                                      const gchar           *property,
                                                                      const GValue          *value,
                                                                      XfwmSettings          *settings);
static void       xfwm_settings_title_alignment_property_changed     (XfconfChannel         *channel,
                                                                      const gchar           *property,
                                                                      const GValue          *value,
                                                                      XfwmSettings          *settings);
static void       xfwm_settings_double_click_action_property_changed (XfconfChannel         *channel,
                                                                      const gchar           *property,
                                                                      const GValue          *value,
                                                                      XfwmSettings          *settings);
static void       xfwm_settings_click_to_focus_property_changed      (XfconfChannel         *channel,
                                                                      const gchar           *property,
                                                                      const GValue          *value,
                                                                      XfwmSettings          *settings);
static void       xfwm_settings_initialize_shortcuts                 (XfwmSettings          *settings);
static void       xfwm_settings_reload_shortcuts                     (XfwmSettings          *settings);
static void       xfwm_settings_shortcut_added                       (XfceShortcutsProvider *provider,
                                                                      const gchar           *shortcut,
                                                                      XfwmSettings          *settings);
static void       xfwm_settings_shortcut_removed                     (XfceShortcutsProvider *provider,
                                                                      const gchar           *shortcut,
                                                                      XfwmSettings          *settings);
static void       xfwm_settings_shortcut_clear_clicked               (GtkButton             *button,
                                                                      XfwmSettings          *settings);
static void       xfwm_settings_shortcut_reset_clicked               (GtkButton             *button,
                                                                      XfwmSettings          *settings);
static void       xfwm_settings_shortcut_row_activated               (GtkTreeView           *tree_view,
                                                                      GtkTreePath           *path,
                                                                      GtkTreeViewColumn     *column,
                                                                      XfwmSettings          *settings);



struct _XfwmSettingsPrivate
{
  GladeXML              *glade_xml;
  XfceShortcutsProvider *provider;
  XfconfChannel         *wm_channel;
};

struct _MenuTemplate
{
  const gchar *name;
  const gchar *value;
};

struct _ShortcutTemplate
{
  const gchar *name;
  const gchar *feature;
  const gchar *shortcut;
};

enum
{
  COL_THEME_NAME,
  COL_THEME_RC,
  N_COLUMNS
};



static GObjectClass      *xfwm_settings_parent_class = NULL;

static const MenuTemplate double_click_values[] = {
  { N_("Shade window"), "shade" },
  { N_("Hide window"), "hide" },
  { N_("Maximize window"), "maximize" },
  { N_("Fill window"), "fill" },
  { N_("Nothing"), "none" },
  { NULL, NULL }
};

static const MenuTemplate title_align_values[] = {
  { N_("Left"), "left" },
  { N_("Center"), "center" },
  { N_("Right"), "right" },
  { NULL, NULL },
};

static const ShortcutTemplate shortcut_values[] = {
  { N_("Window operations menu"), "popup_menu_key", NULL },
  { N_("Up"), "up_key", NULL },
  { N_("Down"), "down_key", NULL },
  { N_("Left"), "left_key", NULL },
  { N_("Right"), "right_key", NULL },
  { N_("Cancel"), "cancel_key", NULL },
  { N_("Cycle windows"), "cycle_windows_key", NULL },
  { N_("Cycle windows (Reverse)"), "cycle_reverse_windows_key", NULL },
  { N_("Close window"), "close_window_key", NULL },
  { N_("Maximize window horizontally"), "maximize_horiz_key", NULL },
  { N_("Maximize window vertically"), "maximize_vert_key", NULL },
  { N_("Maximize window"), "maximize_window_key", NULL },
  { N_("Hide window"), "hide_window_key", NULL },
  { N_("Move window"), "move_window_key", NULL },
  { N_("Resize window"), "resize_window_key", NULL },
  { N_("Shade window"), "shade_window_key", NULL },
  { N_("Stick window"), "stick_window_key", NULL },
  { N_("Raise window"), "raise_window_key", NULL },
  { N_("Lower window"), "lower_window_key", NULL },
  { N_("Fill window"), "fill_window_key", NULL },
  { N_("Fill window horizontally"), "fill_horiz_key", NULL },
  { N_("Fill window vertically"), "fill_vert_key", NULL },
  { N_("Toggle above"), "above_key", NULL },
  { N_("Toggle fullscreen"), "fullscreen_key", NULL },
  { N_("Move window to upper workspace"), "move_window_up_workspace_key", NULL },
  { N_("Move window to bottom workspace"), "move_window_down_workspace_key", NULL },
  { N_("Move window to left workspace"), "move_window_left_workspace_key", NULL },
  { N_("Move window to right workspace"), "move_window_right_workspace_key", NULL },
  { N_("Move window to previous workspace"), "move_window_prev_workspace_key", NULL },
  { N_("Move window to next workspace"), "move_window_next_workspace_key", NULL },
  { N_("Move window to workspace 1"), "move_window_workspace_1_key", NULL, },
  { N_("Move window to workspace 2"), "move_window_workspace_2_key", NULL, },
  { N_("Move window to workspace 3"), "move_window_workspace_3_key", NULL, },
  { N_("Move window to workspace 4"), "move_window_workspace_4_key", NULL, },
  { N_("Move window to workspace 5"), "move_window_workspace_5_key", NULL, },
  { N_("Move window to workspace 6"), "move_window_workspace_6_key", NULL, },
  { N_("Move window to workspace 7"), "move_window_workspace_7_key", NULL, },
  { N_("Move window to workspace 8"), "move_window_workspace_8_key", NULL, },
  { N_("Move window to workspace 9"), "move_window_workspace_9_key", NULL, },
  { N_("Move window to workspace 10"), "move_window_workspace_10_key", NULL, },
  { N_("Move window to workspace 11"), "move_window_workspace_11_key", NULL, },
  { N_("Move window to workspace 12"), "move_window_workspace_12_key", NULL, },
  { N_("Show desktop"), "show_desktop_key", NULL },
  { N_("Upper workspace"), "up_workspace_key", NULL },
  { N_("Bottom workspace"), "down_workspace_key", NULL },
  { N_("Left workspace"), "left_workspace_key", NULL },
  { N_("Right workspace"), "right_workspace_key", NULL },
  { N_("Previous workspace"), "prev_workspace_key", NULL },
  { N_("Next workspace"), "next_workspace_key", NULL },
  { N_("Workspace 1"), "workspace_1_key", NULL, },
  { N_("Workspace 2"), "workspace_2_key", NULL, },
  { N_("Workspace 3"), "workspace_3_key", NULL, },
  { N_("Workspace 4"), "workspace_4_key", NULL, },
  { N_("Workspace 5"), "workspace_5_key", NULL, },
  { N_("Workspace 6"), "workspace_6_key", NULL, },
  { N_("Workspace 7"), "workspace_7_key", NULL, },
  { N_("Workspace 8"), "workspace_8_key", NULL, },
  { N_("Workspace 9"), "workspace_9_key", NULL, },
  { N_("Workspace 10"), "workspace_10_key", NULL, },
  { N_("Workspace 11"), "workspace_11_key", NULL, },
  { N_("Workspace 12"), "workspace_12_key", NULL, },
  { N_("Add workspace"), "add_workspace_key", NULL },
  { N_("Add adjacent workspace"), "add_adjacent_workspace_key", NULL },
  { N_("Delete last workspace"), "del_workspace_key", NULL },
  { N_("Delete active workspace"), "del_active_workspace_key", NULL },
  { NULL, NULL, NULL }
};

static gboolean           opt_version = FALSE;
static GdkNativeWindow    opt_socket_id = 0;
static GOptionEntry       opt_entries[] = {
  { "socket-id", 's', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_INT, &opt_socket_id,
    N_("Settings manager socket"), N_("SOCKET ID") },
  { "version", 'V', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE, &opt_version,
    N_("Version information"), NULL },
  { NULL }
};


GType
xfwm_settings_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info =
      {
        sizeof (XfwmSettingsClass),
        NULL,
        NULL,
        (GClassInitFunc) xfwm_settings_class_init,
        NULL,
        NULL,
        sizeof (XfwmSettings),
        0,
        (GInstanceInitFunc) xfwm_settings_init,
        NULL,
      };

      type = g_type_register_static (G_TYPE_OBJECT, "XfwmSettings", &info, 0);
    }

  return type;
}



static void
xfwm_settings_class_init (XfwmSettingsClass *klass)
{
  GObjectClass *gobject_class;

  g_type_class_add_private (klass, sizeof (XfwmSettingsPrivate));

  /* Determine the parent type class */
  xfwm_settings_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
#if GLIB_CHECK_VERSION (2,14,0)
  gobject_class->constructed = xfwm_settings_constructed;
#endif
  gobject_class->finalize = xfwm_settings_finalize;
  gobject_class->get_property = xfwm_settings_get_property;
  gobject_class->set_property = xfwm_settings_set_property;

  g_object_class_install_property (gobject_class,
                                   PROP_GLADE_XML,
                                   g_param_spec_object ("glade-xml",
                                                        "glade-xml",
                                                        "glade-xml",
                                                        GLADE_TYPE_XML,
                                                        G_PARAM_CONSTRUCT_ONLY |
                                                        G_PARAM_WRITABLE));
}



static void
xfwm_settings_init (XfwmSettings *settings)
{
  settings->priv = XFWM_SETTINGS_GET_PRIVATE (settings);

  settings->priv->glade_xml = NULL;
  settings->priv->provider = xfce_shortcuts_provider_new ("xfwm4");
  settings->priv->wm_channel = xfconf_channel_new ("xfwm4");
}



static void
xfwm_settings_constructed (GObject *object)
{
  XfwmSettings       *settings = XFWM_SETTINGS (object);
  const MenuTemplate *template;
  GtkTreeSelection   *selection;
  GtkCellRenderer    *renderer;
  GtkTargetEntry      target_entry[2];
  GtkListStore       *list_store;
  GtkTreeIter         iter;
  GtkWidget          *theme_name_treeview;
  GtkWidget          *title_font_button;
  GtkWidget          *title_align_combo;
  GtkWidget          *active_frame;
  GtkWidget          *active_box;
  GtkWidget          *hidden_frame;
  GtkWidget          *hidden_box;
  GtkWidget          *shortcuts_treeview;
  GtkWidget          *shortcuts_clear_button;
  GtkWidget          *shortcuts_reset_button;
  GtkWidget          *focus_delay_scale;
  GtkWidget          *raise_on_click_check;
  GtkWidget          *raise_on_focus_check;
  GtkWidget          *click_to_focus_radio;
  GtkWidget          *focus_new_check;
  GtkWidget          *box_move_check;
  GtkWidget          *box_resize_check;
  GtkWidget          *wrap_workspaces_check;
  GtkWidget          *wrap_windows_check;
  GtkWidget          *snap_to_border_check;
  GtkWidget          *snap_to_window_check;
  GtkWidget          *double_click_action_combo;
  GtkWidget          *snap_width_scale;
  GtkWidget          *wrap_resistance_scale;
  GtkWidget          *button;
  GValue              value = { 0, };
  GList              *children;
  GList              *list_iter;
  const gchar        *name;

  /* Style tab widgets */
  theme_name_treeview = glade_xml_get_widget (settings->priv->glade_xml, "theme_name_treeview");
  title_font_button = glade_xml_get_widget (settings->priv->glade_xml, "title_font_button");
  title_align_combo = glade_xml_get_widget (settings->priv->glade_xml, "title_align_combo");
  active_frame = glade_xml_get_widget (settings->priv->glade_xml, "active-frame");
  active_box = glade_xml_get_widget (settings->priv->glade_xml, "active-box");
  hidden_frame = glade_xml_get_widget (settings->priv->glade_xml, "hidden-frame");
  hidden_box = glade_xml_get_widget (settings->priv->glade_xml, "hidden-box");

  /* Style tab: theme name */
  {
    list_store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);
    gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (list_store), COL_THEME_NAME,
                                     (GtkTreeIterCompareFunc) xfwm_settings_theme_sort_func,
                                     NULL, NULL);
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store), COL_THEME_NAME, GTK_SORT_ASCENDING);
    gtk_tree_view_set_model (GTK_TREE_VIEW (theme_name_treeview), GTK_TREE_MODEL (list_store));
    g_object_unref (G_OBJECT (list_store));

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (theme_name_treeview),
                                                 0, _("Theme"), renderer, "text", 0, NULL);

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (theme_name_treeview));
    g_signal_connect (selection, "changed", G_CALLBACK (xfwm_settings_theme_selection_changed),
                      settings);
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

    xfwm_settings_load_themes (settings);
  }

  /* Style tab: font */
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/title_font", G_TYPE_STRING,
                          title_font_button, "font-name");

  /* Style tab: title alignment */
  {
    gtk_cell_layout_clear (GTK_CELL_LAYOUT (title_align_combo));

    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (title_align_combo), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (title_align_combo), renderer, "text", 0);

    list_store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
    gtk_combo_box_set_model (GTK_COMBO_BOX (title_align_combo), GTK_TREE_MODEL (list_store));

    for (template = title_align_values; template->name != NULL; ++template)
      {
        gtk_list_store_append (list_store, &iter);
        gtk_list_store_set (list_store, &iter, 0, _(template->name), 1, template->value, -1);
      }
    g_object_unref (G_OBJECT (list_store));
    xfconf_channel_get_property (settings->priv->wm_channel, "/general/title_alignment", &value);
    xfwm_settings_title_alignment_property_changed (settings->priv->wm_channel,
                                                    "/general/title_alignment", &value, settings);
    g_value_unset (&value);

    g_signal_connect (title_align_combo, "changed",
                      G_CALLBACK (xfwm_settings_title_alignment_changed), settings);
    g_signal_connect (settings->priv->wm_channel, "property-changed::/general/title_alignment",
                      G_CALLBACK (xfwm_settings_title_alignment_property_changed), settings);
  }

  /* Style tab: button layout */
  {
    target_entry[0].target = "_xfwm4_button_layout";
    target_entry[0].flags = 0;
    target_entry[0].info = 2;

    target_entry[1].target = "_xfwm4_active_layout";
    target_entry[1].flags = 0;
    target_entry[1].info = 3;

    gtk_drag_dest_set (active_frame, GTK_DEST_DEFAULT_ALL, target_entry, 2, GDK_ACTION_MOVE);

    g_signal_connect (active_frame, "drag-data-received",
                      G_CALLBACK (xfwm_settings_active_frame_drag_data), settings);
    g_signal_connect (active_frame, "drag-motion",
                      G_CALLBACK (xfwm_settings_active_frame_drag_motion), settings);
    g_signal_connect (active_frame, "drag-leave",
                      G_CALLBACK (xfwm_settings_active_frame_drag_leave), settings);

    gtk_drag_dest_set (hidden_frame, GTK_DEST_DEFAULT_ALL, target_entry, 1, GDK_ACTION_MOVE);

    g_signal_connect (hidden_frame, "drag-data-received",
                      G_CALLBACK (xfwm_settings_hidden_frame_drag_data), settings);

    children = gtk_container_get_children (GTK_CONTAINER (active_box));
    for (list_iter = children; list_iter != NULL; list_iter = g_list_next (list_iter))
      {
        button = GTK_WIDGET (list_iter->data);
        name = gtk_widget_get_name (button);

        if (name[strlen (name) - 1] == '|')
          {
            g_signal_connect (title_align_combo, "changed",
                              G_CALLBACK (xfwm_settings_title_button_alignment_changed), button);
            xfwm_settings_title_button_alignment_changed (GTK_COMBO_BOX (title_align_combo),
                                                          button);
          }

        g_object_set_data (G_OBJECT (button), "key_char", (gpointer) &name[strlen (name) - 1]);
        gtk_drag_source_set (button, GDK_BUTTON1_MASK, &target_entry[1], 1, GDK_ACTION_MOVE);

        g_signal_connect (button, "drag_data_get",
                          G_CALLBACK (xfwm_settings_title_button_drag_data), NULL);
        g_signal_connect (button, "drag_begin", G_CALLBACK (xfwm_settings_title_button_drag_begin),
                          NULL);
        g_signal_connect (button, "drag_end", G_CALLBACK (xfwm_settings_title_button_drag_end),
                          NULL);
        g_signal_connect (button, "button_press_event",
                          G_CALLBACK (xfwm_settings_signal_blocker), NULL);
        g_signal_connect (button, "enter_notify_event",
                          G_CALLBACK (xfwm_settings_signal_blocker), NULL);
        g_signal_connect (button, "focus",  G_CALLBACK (xfwm_settings_signal_blocker), NULL);
      }
  }
  g_list_free (children);

  children = gtk_container_get_children (GTK_CONTAINER (hidden_box));
  for (list_iter = children; list_iter != NULL; list_iter = g_list_next (list_iter))
    {
      button = GTK_WIDGET (list_iter->data);
      name = gtk_widget_get_name (button);

      g_object_set_data (G_OBJECT (button), "key_char", (gpointer) &name[strlen (name) - 1]);
      gtk_drag_source_set (button, GDK_BUTTON1_MASK, &target_entry[0], 1, GDK_ACTION_MOVE);

        g_signal_connect (button, "drag_data_get",
                          G_CALLBACK (xfwm_settings_title_button_drag_data), NULL);
        g_signal_connect (button, "drag_begin", G_CALLBACK (xfwm_settings_title_button_drag_begin),
                          NULL);
        g_signal_connect (button, "drag_end", G_CALLBACK (xfwm_settings_title_button_drag_end),
                          NULL);
        g_signal_connect (button, "button_press_event",
                          G_CALLBACK (xfwm_settings_signal_blocker), NULL);
        g_signal_connect (button, "enter_notify_event",
                          G_CALLBACK (xfwm_settings_signal_blocker), NULL);
        g_signal_connect (button, "focus",  G_CALLBACK (xfwm_settings_signal_blocker), NULL);
    }
  g_list_free (children);

  xfconf_channel_get_property (settings->priv->wm_channel, "/general/button_layout", &value);
  xfwm_settings_button_layout_property_changed (settings->priv->wm_channel,
                                                "/general/button_layout", &value, settings);
  g_value_unset (&value);

  /* Keyboard tab widgets */
  shortcuts_treeview = glade_xml_get_widget (settings->priv->glade_xml, "shortcuts_treeview");
  shortcuts_clear_button = glade_xml_get_widget (settings->priv->glade_xml,
                                                 "shortcuts_clear_button");
  shortcuts_reset_button = glade_xml_get_widget (settings->priv->glade_xml,
                                                 "shortcuts_reset_button");

  /* Set reset button icon */
  gtk_button_set_image (GTK_BUTTON (shortcuts_reset_button),
                        gtk_image_new_from_stock (GTK_STOCK_REVERT_TO_SAVED,
                                                  GTK_ICON_SIZE_BUTTON));

  /* Keyboard tab: Shortcuts tree view */
  {
    gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (shortcuts_treeview)),
                                 GTK_SELECTION_MULTIPLE);

    list_store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    gtk_tree_view_set_model (GTK_TREE_VIEW (shortcuts_treeview), GTK_TREE_MODEL (list_store));
    g_object_unref (G_OBJECT (list_store));

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (shortcuts_treeview),
                                                 0, _("Action"), renderer,
                                                 "text", SHORTCUTS_NAME_COLUMN, NULL);

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (shortcuts_treeview),
                                                 1, _("Shortcut"), renderer,
                                                 "text", SHORTCUTS_SHORTCUT_COLUMN, NULL);

    g_signal_connect (shortcuts_treeview, "row-activated",
                      G_CALLBACK (xfwm_settings_shortcut_row_activated), settings);
  }

  /* Connect to shortcut buttons */
  g_signal_connect (shortcuts_clear_button, "clicked",
                    G_CALLBACK (xfwm_settings_shortcut_clear_clicked), settings);
  g_signal_connect (shortcuts_reset_button, "clicked",
                    G_CALLBACK (xfwm_settings_shortcut_reset_clicked), settings);

  /* Focus tab widgets */
  focus_delay_scale = glade_xml_get_widget (settings->priv->glade_xml, "focus_delay_scale");
  focus_new_check = glade_xml_get_widget (settings->priv->glade_xml, "focus_new_check");
  raise_on_focus_check = glade_xml_get_widget (settings->priv->glade_xml, "raise_on_focus_check");
  raise_on_click_check = glade_xml_get_widget (settings->priv->glade_xml, "raise_on_click_check");
  click_to_focus_radio = glade_xml_get_widget (settings->priv->glade_xml, "click_to_focus_radio");

  /* Focus tab */
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/focus_delay", G_TYPE_INT,
                          gtk_range_get_adjustment (GTK_RANGE (focus_delay_scale)), "value");
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/raise_on_click", G_TYPE_BOOLEAN,
                          raise_on_click_check, "active");
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/raise_on_focus", G_TYPE_BOOLEAN,
                          raise_on_focus_check, "active");
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/focus_new", G_TYPE_BOOLEAN,
                          focus_new_check, "active");
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/click_to_focus", G_TYPE_BOOLEAN,
                          click_to_focus_radio, "active");

  g_signal_connect (settings->priv->wm_channel, "property-changed::/general/click_to_focus",
                    G_CALLBACK (xfwm_settings_click_to_focus_property_changed), settings);

  xfconf_channel_get_property (settings->priv->wm_channel, "/general/click_to_focus", &value);
  xfwm_settings_click_to_focus_property_changed (settings->priv->wm_channel,
                                                 "/general/click_to_focus", &value, settings);
  g_value_unset (&value);

  /* Advanced tab widgets */
  box_move_check = glade_xml_get_widget (settings->priv->glade_xml, "box_move_check");
  box_resize_check = glade_xml_get_widget (settings->priv->glade_xml, "box_resize_check");
  wrap_workspaces_check = glade_xml_get_widget (settings->priv->glade_xml,
                                                "wrap_workspaces_check");
  wrap_windows_check = glade_xml_get_widget (settings->priv->glade_xml, "wrap_windows_check");
  snap_to_border_check = glade_xml_get_widget (settings->priv->glade_xml, "snap_to_border_check");
  snap_to_window_check = glade_xml_get_widget (settings->priv->glade_xml, "snap_to_window_check");
  double_click_action_combo = glade_xml_get_widget (settings->priv->glade_xml,
                                                    "double_click_action_combo");
  snap_width_scale = glade_xml_get_widget (settings->priv->glade_xml, "snap_width_scale");
  wrap_resistance_scale = glade_xml_get_widget (settings->priv->glade_xml,
                                                "wrap_resistance_scale");

  /* Advanced tab: double click action */
  {
    gtk_cell_layout_clear (GTK_CELL_LAYOUT (double_click_action_combo));

    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (double_click_action_combo), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (double_click_action_combo), renderer,
                                   "text", 0);

    list_store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
    gtk_combo_box_set_model (GTK_COMBO_BOX (double_click_action_combo),
                             GTK_TREE_MODEL (list_store));

    for (template = double_click_values; template->name != NULL; ++template)
      {
        gtk_list_store_append (list_store, &iter);
        gtk_list_store_set (list_store, &iter, 0, _(template->name), 1, template->value, -1);
      }
    g_object_unref (G_OBJECT (list_store));
    xfconf_channel_get_property (settings->priv->wm_channel, "/general/double_click_action",
                                 &value);
    xfwm_settings_double_click_action_property_changed (settings->priv->wm_channel,
                                                        "/general/double_click_action",
                                                        &value, settings);
    g_value_unset (&value);

    g_signal_connect (double_click_action_combo, "changed",
                      G_CALLBACK (xfwm_settings_double_click_action_changed),
                      settings);
    g_signal_connect (settings->priv->wm_channel, "property-changed::/general/double_click_action",
                      G_CALLBACK (xfwm_settings_double_click_action_property_changed), settings);
  }

  /* Advanced tab */
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/snap_width", G_TYPE_INT,
                          gtk_range_get_adjustment (GTK_RANGE (snap_width_scale)), "value");
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/wrap_resistance", G_TYPE_INT,
                          gtk_range_get_adjustment (GTK_RANGE (wrap_resistance_scale)), "value");
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/box_move", G_TYPE_BOOLEAN,
                          box_move_check, "active");
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/box_resize", G_TYPE_BOOLEAN,
                          box_resize_check, "active");
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/wrap_workspaces", G_TYPE_BOOLEAN,
                          wrap_workspaces_check, "active");
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/wrap_windows", G_TYPE_BOOLEAN,
                          wrap_windows_check, "active");
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/snap_to_border", G_TYPE_BOOLEAN,
                          snap_to_border_check, "active");
  xfconf_g_property_bind (settings->priv->wm_channel, "/general/snap_to_windows", G_TYPE_BOOLEAN,
                          snap_to_window_check, "active");

  /* Load shortcuts */
  xfwm_settings_initialize_shortcuts (settings);
  xfwm_settings_reload_shortcuts (settings);

  /* Connect to shortcuts provider */
  g_signal_connect (settings->priv->provider, "shortcut-added",
                    G_CALLBACK (xfwm_settings_shortcut_added), settings);
  g_signal_connect (settings->priv->provider, "shortcut-removed",
                    G_CALLBACK (xfwm_settings_shortcut_removed), settings);
}



static void
xfwm_settings_finalize (GObject *object)
{
  XfwmSettings *settings = XFWM_SETTINGS (object);

  g_object_unref (settings->priv->wm_channel);
  g_object_unref (settings->priv->provider);
  g_object_unref (settings->priv->glade_xml);

  (*G_OBJECT_CLASS (xfwm_settings_parent_class)->finalize) (object);
}



static void
xfwm_settings_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  XfwmSettings *settings = XFWM_SETTINGS (object);

  switch (prop_id)
    {
    case PROP_GLADE_XML:
      g_value_set_object (value, settings->priv->glade_xml);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
xfwm_settings_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  XfwmSettings *settings = XFWM_SETTINGS (object);

  switch (prop_id)
    {
    case PROP_GLADE_XML:
      if (GLADE_IS_XML (settings->priv->glade_xml))
        g_object_unref (settings->priv->glade_xml);
      settings->priv->glade_xml = g_value_get_object (value);
      g_object_notify (object, "glade-xml");
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



XfwmSettings *
xfwm_settings_new (void)
{
  XfwmSettings *settings = NULL;
  GladeXML     *glade_xml;

  glade_xml = glade_xml_new_from_buffer (xfwm4_dialog_glade, xfwm4_dialog_glade_length, NULL, NULL);

  if (G_LIKELY (glade_xml != NULL))
    settings = g_object_new (XFWM_TYPE_SETTINGS, "glade-xml", glade_xml, NULL);

#if !GLIB_CHECK_VERSION (2,14,0)
  xfwm_settings_constructed (G_OBJECT(settings));
#endif

  return settings;
}


static gint
xfwm_settings_theme_sort_func (GtkTreeModel *model,
                               GtkTreeIter  *iter1,
                               GtkTreeIter  *iter2)
{
  gchar *str1 = NULL;
  gchar *str2 = NULL;

  gtk_tree_model_get (model, iter1, 0, &str1, -1);
  gtk_tree_model_get (model, iter2, 0, &str2, -1);

  if (str1 == NULL) str1 = g_strdup ("");
  if (str2 == NULL) str2 = g_strdup ("");

  if (g_str_equal (str1, DEFAULT_THEME))
    return -1;

  if (g_str_equal (str2, DEFAULT_THEME))
    return 1;

  return g_utf8_collate (str1, str2);
}



static void
xfwm_settings_load_themes (XfwmSettings *settings)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  GtkWidget    *view;
  GHashTable   *themes;
  GDir         *dir;
  const gchar  *file;
  gchar       **theme_dirs;
  gchar        *filename;
  gchar        *active_theme_name;
  gint          i;

  themes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  view = glade_xml_get_widget (settings->priv->glade_xml, "theme_name_treeview");
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (view));

  xfce_resource_push_path (XFCE_RESOURCE_THEMES, DATADIR G_DIR_SEPARATOR_S "themes");
  theme_dirs = xfce_resource_dirs (XFCE_RESOURCE_THEMES);
  xfce_resource_pop_path (XFCE_RESOURCE_THEMES);

  active_theme_name = xfconf_channel_get_string (settings->priv->wm_channel, "/general/theme", DEFAULT_THEME);

  for (i = 0; theme_dirs[i] != NULL; ++i)
    {
      dir = g_dir_open (theme_dirs[i], 0, NULL);

      if (G_UNLIKELY (dir == NULL))
        continue;

      while ((file = g_dir_read_name (dir)) != NULL)
        {
          filename = g_build_filename (theme_dirs[i], file, "xfwm4", "themerc", NULL);

          /* check if the theme rc exists and there is not already a theme with the
           * same name in the database */
          if (g_file_test (filename, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR) &&
              g_hash_table_lookup (themes, file) == NULL)
            {
              g_hash_table_insert (themes, g_strdup (file), GINT_TO_POINTER (1));

              /* insert in the list store */
              gtk_list_store_append (GTK_LIST_STORE (model), &iter);
              gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                                  COL_THEME_NAME, file,
                                  COL_THEME_RC, filename, -1);

              if (G_UNLIKELY (g_str_equal (active_theme_name, file)))
                {
                  gtk_tree_selection_select_iter (gtk_tree_view_get_selection (GTK_TREE_VIEW (view)),
                                                  &iter);
                }
            }

          g_free (filename);
        }

      g_dir_close (dir);
    }

  g_free (active_theme_name);
  g_strfreev (theme_dirs);
  g_hash_table_destroy (themes);
}



static GtkWidget *
xfwm_settings_create_dialog (XfwmSettings *settings)
{
  g_return_val_if_fail (XFWM_IS_SETTINGS (settings), NULL);
  return glade_xml_get_widget (settings->priv->glade_xml, "main-dialog");
}



static GtkWidget *
xfwm_settings_create_plug (XfwmSettings   *settings,
                           GdkNativeWindow socket_id)
{
  GtkWidget *plug;
  GtkWidget *child;

  g_return_val_if_fail (XFWM_IS_SETTINGS (settings), NULL);

  plug = gtk_plug_new (socket_id);
  gtk_widget_show (plug);

  child = glade_xml_get_widget (settings->priv->glade_xml, "plug-child");
  gtk_widget_reparent (child, plug);
  gtk_widget_show (child);

  return plug;
}



int
main (int    argc,
      char **argv)
{
  XfwmSettings *settings;
  GtkWidget    *dialog;
  GtkWidget    *plug;
  GError       *error = NULL;

  xfce_textdomain (GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");

  if (G_UNLIKELY (!gtk_init_with_args (&argc, &argv, _("."), opt_entries, PACKAGE, &error)))
    {
      if (G_LIKELY (error != NULL))
        {
          g_print (_("%s: %s\nTry %s --help to see a full list of available command line options.\n"),
                   PACKAGE, error->message, PACKAGE_NAME);
          g_error_free (error);
        }

      return EXIT_FAILURE;
    }

  if (G_UNLIKELY (opt_version))
    {
      g_print ("%s\n", PACKAGE_STRING);
      return EXIT_SUCCESS;
    }

  if (G_UNLIKELY (!xfconf_init (&error)))
    {
      if (G_LIKELY (error != NULL))
        {
          g_error (_("Failed to initialize xfconf. Reason: %s"), error->message);
          g_error_free (error);
        }

      return EXIT_FAILURE;
    }

  settings = xfwm_settings_new ();

  if (G_UNLIKELY (settings == NULL))
    {
      g_error (_("Could not create the settings dialog."));
      xfconf_shutdown ();
      return EXIT_FAILURE;
    }

  if (G_UNLIKELY (opt_socket_id == 0))
    {
      dialog = xfwm_settings_create_dialog (settings);
      gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (dialog);
    }
  else
    {
      plug = xfwm_settings_create_plug (settings, opt_socket_id);
      g_signal_connect (plug, "delete-event", G_CALLBACK (gtk_main_quit), NULL);

      /* Stop startup notification */
      gdk_notify_startup_complete ();

      gtk_main ();
    }

  g_object_unref (settings);

  xfconf_shutdown ();

  return EXIT_SUCCESS;
}



static void
xfwm_settings_theme_selection_changed (GtkTreeSelection *selection,
                                       XfwmSettings     *settings)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  gchar        *theme, *filename;
  XfceRc       *rc;
  GtkWidget    *widget;
  gboolean      button_layout = FALSE;
  gboolean      title_alignment = FALSE;

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      gtk_tree_model_get (model, &iter,
                          COL_THEME_NAME, &theme,
                          COL_THEME_RC, &filename, -1);

      /* set the theme name */
      xfconf_channel_set_string (settings->priv->wm_channel, "/general/theme", theme);
      g_free (theme);

      /* check in the rc if the theme supports a custom button layout and/or
       * title alignement */
      rc = xfce_rc_simple_open (filename, TRUE);
      g_free (filename);

      if (G_LIKELY (rc != NULL))
        {
          button_layout = !xfce_rc_has_entry (rc, "button_layout");
          title_alignment = !xfce_rc_has_entry (rc, "title_alignment");
          xfce_rc_close (rc);
        }
    }

  widget = glade_xml_get_widget (settings->priv->glade_xml, "button_layout_box");
  gtk_widget_set_sensitive (widget, button_layout);

  widget = glade_xml_get_widget (settings->priv->glade_xml, "title_align_box");
  gtk_widget_set_sensitive (widget, title_alignment);
}



static void
xfwm_settings_title_alignment_changed (GtkComboBox  *combo,
                                       XfwmSettings *settings)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  gchar        *alignment;

  g_return_if_fail (XFWM_IS_SETTINGS (settings));

  model = gtk_combo_box_get_model (combo);

  gtk_combo_box_get_active_iter (combo, &iter);
  gtk_tree_model_get (model, &iter, 1, &alignment, -1);

  xfconf_channel_set_string (settings->priv->wm_channel, "/general/title_alignment", alignment);

  g_free (alignment);
}



static void
xfwm_settings_active_frame_drag_data (GtkWidget        *widget,
                                      GdkDragContext   *drag_context,
                                      gint              x,
                                      gint              y,
                                      GtkSelectionData *data,
                                      guint             info,
                                      guint             time,
                                      XfwmSettings     *settings)
{
  GtkWidget *source;
  GtkWidget *parent;
  GtkWidget *active_box;
  GList     *children;
  GList     *iter;
  gint       xoffset;
  gint       i;

  g_return_if_fail (XFWM_IS_SETTINGS (settings));

  source = glade_xml_get_widget (settings->priv->glade_xml,
#if GLIB_CHECK_VERSION (2,14,0)
                                 (const gchar *)gtk_selection_data_get_data (data));
#else
                                 (const gchar *) data->data);
#endif
  parent = gtk_widget_get_parent (source);

  active_box = glade_xml_get_widget (settings->priv->glade_xml, "active-box");

  g_object_ref (source);
  gtk_container_remove (GTK_CONTAINER (parent), source);
  gtk_box_pack_start (GTK_BOX (active_box), source, info == 3, info == 3, 0);
  g_object_unref (source);

  xoffset = widget->allocation.x;

  children = gtk_container_get_children (GTK_CONTAINER (active_box));

  for (i = 0, iter = children; iter != NULL; ++i, iter = g_list_next (iter))
    if (GTK_WIDGET_VISIBLE (iter->data))
      if (x < (GTK_WIDGET (iter->data)->allocation.width / 2 +
               GTK_WIDGET (iter->data)->allocation.x - xoffset))
        {
          break;
        }

  g_list_free (children);

  gtk_box_reorder_child (GTK_BOX (active_box), source, i);

  xfwm_settings_save_button_layout (settings, GTK_CONTAINER (active_box));
}



static gboolean
xfwm_settings_active_frame_drag_motion (GtkWidget      *widget,
                                        GdkDragContext *drag_context,
                                        gint            x,
                                        gint            y,
                                        guint           time,
                                        XfwmSettings   *settings)
{
  GtkWidget *active_box;
  GdkWindow *indicator;
  GList     *children;
  GList     *iter;
  gint       xoffset = widget->allocation.x;
  gint       height;
  gint       ix;
  gint       iy;

  g_return_val_if_fail (XFWM_IS_SETTINGS (settings), FALSE);

  active_box = glade_xml_get_widget (settings->priv->glade_xml, "active-box");
  children = gtk_container_get_children (GTK_CONTAINER (active_box));

  /* Set a value so that the compiler does not (rightfully) complain */
  ix = INDICATOR_SIZE;
  for (iter = children; iter != NULL; iter = g_list_next (iter))
    {
      if (GTK_WIDGET_VISIBLE (iter->data))
        {
          if (x < (GTK_WIDGET (iter->data)->allocation.width / 2 +
                   GTK_WIDGET (iter->data)->allocation.x - xoffset))
            {
              ix = GTK_WIDGET (iter->data)->allocation.x;
              break;
            }

          ix = GTK_WIDGET (iter->data)->allocation.x + GTK_WIDGET (iter->data)->allocation.width;
        }
    }

  g_list_free (children);

  ix -= INDICATOR_SIZE / 2 + 1;
  iy = active_box->allocation.y - INDICATOR_SIZE / 2 +
       gtk_container_get_border_width (GTK_CONTAINER (active_box));

  indicator = g_object_get_data (G_OBJECT (active_box), "indicator_window");

  if (G_UNLIKELY (indicator == NULL))
    {
      height = active_box->allocation.height + INDICATOR_SIZE -
               gtk_container_get_border_width (GTK_CONTAINER (active_box)) * 2;
      xfwm_settings_create_indicator (active_box, ix, iy, INDICATOR_SIZE, height);
    }
  else
    gdk_window_move (indicator, ix, iy);

  return FALSE;
}



static void
xfwm_settings_active_frame_drag_leave (GtkWidget      *widget,
                                       GdkDragContext *drag_context,
                                       guint           time,
                                       XfwmSettings   *settings)
{
  g_return_if_fail (XFWM_IS_SETTINGS (settings));

  xfwm_settings_delete_indicator (glade_xml_get_widget (settings->priv->glade_xml, "active-box"));
}



static void
xfwm_settings_hidden_frame_drag_data (GtkWidget        *widget,
                                      GdkDragContext   *drag_context,
                                      gint              x,
                                      gint              y,
                                      GtkSelectionData *data,
                                      guint             info,
                                      guint             time,
                                      XfwmSettings     *settings)
{
  GtkWidget *source;
  GtkWidget *parent;
  GtkWidget *hidden_box;
  GtkWidget *active_box;

  g_return_if_fail (XFWM_IS_SETTINGS (settings));

  source = glade_xml_get_widget (settings->priv->glade_xml,
#if GLIB_CHECK_VERSION (2,14,0)
                                 (const gchar *)gtk_selection_data_get_data (data));
#else
                                 (const gchar *) data->data);
#endif
  parent = gtk_widget_get_parent (source);

  hidden_box = glade_xml_get_widget (settings->priv->glade_xml, "hidden-box");
  active_box = glade_xml_get_widget (settings->priv->glade_xml, "active-box");

  if (G_UNLIKELY (parent == hidden_box))
    return;

  g_object_ref (source);
  gtk_container_remove (GTK_CONTAINER (parent), source);
  gtk_box_pack_start (GTK_BOX (hidden_box), source, FALSE, FALSE, 0);
  g_object_unref (source);

  xfwm_settings_save_button_layout (settings, GTK_CONTAINER (active_box));
}



static void
xfwm_settings_create_indicator (GtkWidget *widget,
                                gint       x,
                                gint       y,
                                gint       width,
                                gint       height)
{
  GdkWindowAttr attributes;
  GdkWindow    *indicator;
  GdkRegion    *shape;
  GdkPoint      points[9];
  gint          attr_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_COLORMAP | GDK_WA_VISUAL;

  g_return_if_fail (GTK_IS_WIDGET (widget));

  attributes.title = NULL;
  attributes.event_mask = 0;
  attributes.x = x;
  attributes.y = y;
  attributes.width = width;
  attributes.height = height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.cursor = NULL;
  attributes.wmclass_name = NULL;
  attributes.wmclass_class = NULL;
  attributes.override_redirect = FALSE;

  indicator = gdk_window_new (gtk_widget_get_parent_window (widget), &attributes, attr_mask);
  gdk_window_set_user_data (indicator, widget);
  g_object_set_data (G_OBJECT (widget), "indicator_window", indicator);

  points[0].x = 0;
  points[0].y = 0;
  points[1].x = width;
  points[1].y = 0;
  points[2].x = width / 2 + 1;
  points[2].y = width / 2;
  points[3].x = width / 2 + 1;
  points[3].y = height - 1 - width / 2;
  points[4].x = width;
  points[4].y = height;
  points[5].x = 0;
  points[5].y = height - 1;
  points[6].x = width / 2;
  points[6].y = height - 1 - width / 2;
  points[7].x = width / 2;
  points[7].y = width / 2;
  points[8].x = 0;
  points[8].y = 0;

  shape = gdk_region_polygon (points, 9, GDK_WINDING_RULE);
  gdk_window_shape_combine_region (indicator, shape, 0, 0);

  gdk_window_show (indicator);
  gdk_window_raise (indicator);
}



static void
xfwm_settings_delete_indicator (GtkWidget *widget)
{
  GdkWindow *indicator;

  g_return_if_fail (GTK_IS_WIDGET (widget));

  indicator = g_object_get_data (G_OBJECT (widget), "indicator_window");

  if (G_LIKELY (indicator != NULL))
    {
      gdk_window_destroy (indicator);
      g_object_set_data (G_OBJECT (widget), "indicator_window", NULL);
    }
}



static void
xfwm_settings_title_button_drag_data (GtkWidget        *widget,
                                      GdkDragContext   *drag_context,
                                      GtkSelectionData *data,
                                      guint             info,
                                      guint             time)
{
  gtk_widget_hide (widget);
  gtk_selection_data_set (data, gdk_atom_intern ("_xfwm4_button_layout", FALSE), 8,
                          (const guchar *)gtk_widget_get_name (widget),
                          strlen (gtk_widget_get_name (widget)));
}



static void
xfwm_settings_title_button_drag_begin (GtkWidget      *widget,
                                       GdkDragContext *drag_context)
{
  GdkPixbuf *pixbuf;

  g_return_if_fail (GTK_IS_WIDGET (widget));

  pixbuf = xfwm_settings_create_icon_from_widget (widget);
  gtk_drag_source_set_icon_pixbuf (widget, pixbuf);
  g_object_unref (pixbuf);

  gtk_widget_hide (widget);
}



static void
xfwm_settings_title_button_drag_end (GtkWidget      *widget,
                                     GdkDragContext *drag_context)
{
  gtk_widget_show (widget);
}



static gboolean
xfwm_settings_signal_blocker (GtkWidget *widget)
{
  return TRUE;
}



static GdkPixbuf *
xfwm_settings_create_icon_from_widget (GtkWidget *widget)
{
  GdkWindow *drawable;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

  drawable = GDK_DRAWABLE (gtk_widget_get_parent_window (widget));
  return gdk_pixbuf_get_from_drawable (NULL, drawable, NULL,
                                       widget->allocation.x, widget->allocation.y, 0, 0,
                                       widget->allocation.width, widget->allocation.height);
}



static void
xfwm_settings_button_layout_property_changed (XfconfChannel *channel,
                                              const gchar   *property,
                                              const GValue  *value,
                                              XfwmSettings  *settings)
{
  GtkWidget   *active_box;
  GtkWidget   *hidden_box;
  GtkWidget   *button;
  GList       *children;
  GList       *iter;
  const gchar *str_value;
  const gchar *key_char;

  g_return_if_fail (XFWM_IS_SETTINGS (settings));

  hidden_box = glade_xml_get_widget (settings->priv->glade_xml, "hidden-box");
  active_box = glade_xml_get_widget (settings->priv->glade_xml, "active-box");

  gtk_widget_set_app_paintable (active_box, FALSE);
  gtk_widget_set_app_paintable (hidden_box, FALSE);

  children = gtk_container_get_children (GTK_CONTAINER (active_box));

  /* Move all buttons to the hidden list, except for the title */
  for (iter = children; iter != NULL; iter = g_list_next (iter))
    {
      button = GTK_WIDGET (iter->data);
      key_char = (const gchar *) g_object_get_data (G_OBJECT (button), "key_char");

      if (G_LIKELY (key_char[0] != '|'))
        {
          g_object_ref (button);
          gtk_container_remove (GTK_CONTAINER (active_box), button);
          gtk_box_pack_start (GTK_BOX (hidden_box), button, FALSE, FALSE, 0);
          g_object_unref (button);
        }
    }

  g_list_free (children);

  children = g_list_concat (gtk_container_get_children (GTK_CONTAINER (active_box)),
                            gtk_container_get_children (GTK_CONTAINER (hidden_box)));

  /* Move buttons to the active list */
  for (str_value = g_value_get_string (value); *str_value != '\0'; ++str_value)
    for (iter = children; iter != NULL; iter = g_list_next (iter))
      {
        button = GTK_WIDGET (iter->data);
        key_char = (const gchar *) g_object_get_data (G_OBJECT (button), "key_char");

        if (g_str_has_prefix (str_value, key_char))
          {
            g_object_ref (button);
            gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (button)), button);
            gtk_box_pack_start (GTK_BOX (active_box), button,
                                key_char[0] == '|', key_char[0] == '|', 0);
            g_object_unref (button);
          }
      }

  g_list_free (children);

  gtk_widget_set_app_paintable (active_box, TRUE);
  gtk_widget_set_app_paintable (hidden_box, TRUE);
}



static void
xfwm_settings_title_alignment_property_changed (XfconfChannel *channel,
                                                const gchar   *property,
                                                const GValue  *value,
                                                XfwmSettings  *settings)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  GtkWidget    *combo;
  gchar        *alignment;

  g_return_if_fail (XFWM_IS_SETTINGS (settings));

  combo = glade_xml_get_widget (settings->priv->glade_xml, "title_align_combo");
  model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo));

  if (gtk_tree_model_get_iter_first (model, &iter))
    {
      do
        {
          gtk_tree_model_get (model, &iter, 1, &alignment, -1);

          if (G_UNLIKELY (g_str_equal (alignment, g_value_get_string (value))))
            {
              g_free (alignment);
              gtk_combo_box_set_active_iter (GTK_COMBO_BOX (combo), &iter);
              break;
            }

          g_free (alignment);
        }
      while (gtk_tree_model_iter_next (model, &iter));
    }
}



static void
xfwm_settings_save_button_layout (XfwmSettings *settings,
                                  GtkContainer *container)
{
  GList        *children;
  GList        *iter;
  const gchar **key_chars;
  gchar        *value;
  gint          i;

  g_return_if_fail (XFWM_IS_SETTINGS (settings));

  children = gtk_container_get_children (container);

  key_chars = g_new0 (const char *, g_list_length (children) + 1);

  for (i = 0, iter = children; iter != NULL; ++i, iter = g_list_next (iter))
    key_chars[i] = (const gchar *) g_object_get_data (G_OBJECT (iter->data), "key_char");

  value = g_strjoinv ("", (gchar **) key_chars);

  xfconf_channel_set_string (settings->priv->wm_channel, "/general/button_layout", value);

  g_list_free (children);
  g_free (key_chars);
  g_free (value);
}



static void
xfwm_settings_double_click_action_changed (GtkComboBox  *combo,
                                           XfwmSettings *settings)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  gchar        *value;

  g_return_if_fail (XFWM_IS_SETTINGS (settings));

  model = gtk_combo_box_get_model (combo);
  gtk_combo_box_get_active_iter (combo, &iter);
  gtk_tree_model_get (model, &iter, 1, &value, -1);

  xfconf_channel_set_string (settings->priv->wm_channel, "/general/double_click_action", value);

  g_free (value);
}



static void
xfwm_settings_title_button_alignment_changed (GtkComboBox *combo,
                                              GtkWidget   *button)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  gchar        *value;
  float         align = 0.5f;

  model = gtk_combo_box_get_model (combo);
  gtk_combo_box_get_active_iter (combo, &iter);
  gtk_tree_model_get (model, &iter, 1, &value, -1);

  if (g_str_equal (value, "left"))
    align = 0.0f;
  else if (g_str_equal (value, "right"))
    align = 1.0f;

  gtk_button_set_alignment (GTK_BUTTON (button), align, 0.5f);

  g_free (value);
}



static void
xfwm_settings_double_click_action_property_changed (XfconfChannel *channel,
                                                    const gchar   *property,
                                                    const GValue  *value,
                                                    XfwmSettings  *settings)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  GtkWidget    *combo;
  const gchar  *new_value;
  gchar        *current_value;

  g_return_if_fail (XFWM_IS_SETTINGS (settings));

  combo = glade_xml_get_widget (settings->priv->glade_xml, "double_click_action_combo");
  model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo));

  if (G_UNLIKELY (G_VALUE_TYPE (value) == G_TYPE_INVALID))
    new_value = "maximize";
  else
    new_value = g_value_get_string (value);

  if (G_LIKELY (gtk_tree_model_get_iter_first (model, &iter)))
    {
      do
        {
          gtk_tree_model_get (model, &iter, 1, &current_value, -1);

          if (G_UNLIKELY (g_str_equal (current_value, new_value)))
            {
              g_free (current_value);
              gtk_combo_box_set_active_iter (GTK_COMBO_BOX (combo), &iter);
              break;
            }

          g_free (current_value);
        }
      while (gtk_tree_model_iter_next (model, &iter));
    }
}



static void
xfwm_settings_click_to_focus_property_changed (XfconfChannel *channel,
                                               const gchar   *property,
                                               const GValue  *value,
                                               XfwmSettings  *settings)
{
  GtkWidget *click_to_focus_radio;
  GtkWidget *focus_follows_mouse_radio;
  GtkWidget *focus_delay_hbox;

  g_return_if_fail (XFWM_IS_SETTINGS (settings));
  g_return_if_fail (GLADE_IS_XML (settings->priv->glade_xml));

  click_to_focus_radio = glade_xml_get_widget (settings->priv->glade_xml,
                                               "click_to_focus_radio");
  focus_follows_mouse_radio = glade_xml_get_widget (settings->priv->glade_xml,
                                                    "focus_follows_mouse_radio");
  focus_delay_hbox = glade_xml_get_widget (settings->priv->glade_xml,
                                           "focus_delay_hbox");

  if (G_UNLIKELY (G_VALUE_TYPE (value) != G_TYPE_BOOLEAN))
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (click_to_focus_radio), TRUE);
  else
    {
      if (g_value_get_boolean (value))
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (click_to_focus_radio), TRUE);
      else
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (focus_follows_mouse_radio), TRUE);
    }
  gtk_widget_set_sensitive (GTK_WIDGET (focus_delay_hbox),
                            gtk_toggle_button_get_active (
                                  GTK_TOGGLE_BUTTON (focus_follows_mouse_radio)));
}




static void
xfwm_settings_initialize_shortcuts (XfwmSettings *settings)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  GtkWidget    *view;
  gint          i;

  g_return_if_fail (XFWM_IS_SETTINGS (settings));
  g_return_if_fail (GLADE_IS_XML (settings->priv->glade_xml));

  view = glade_xml_get_widget (settings->priv->glade_xml, "shortcuts_treeview");
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (view));

  gtk_list_store_clear (GTK_LIST_STORE (model));

  for (i = 0; shortcut_values[i].name != NULL; ++i)
    {
      gtk_list_store_append (GTK_LIST_STORE (model), &iter);
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                          SHORTCUTS_NAME_COLUMN, _(shortcut_values[i].name),
                          SHORTCUTS_FEATURE_COLUMN, shortcut_values[i].feature,
                          -1);
    }
}



static void
xfwm_settings_clear_shortcuts_view (XfwmSettings *settings)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  GtkWidget    *view;

  g_return_if_fail (XFWM_IS_SETTINGS (settings));
  g_return_if_fail (GLADE_IS_XML (settings->priv->glade_xml));

  view = glade_xml_get_widget (settings->priv->glade_xml, "shortcuts_treeview");
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (view));

  if (G_LIKELY (gtk_tree_model_get_iter_first (model, &iter)))
    {
      do
        {
          gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                              SHORTCUTS_SHORTCUT_COLUMN, NULL, -1);
        }
      while (gtk_tree_model_iter_next (model, &iter));
    }
}



static void
xfwm_settings_reload_shortcut (XfceShortcut *shortcut,
                               GtkTreeModel *model)
{
  GtkTreeIter iter;
  gchar      *feature;

  g_return_if_fail (GTK_IS_TREE_MODEL (model));
  g_return_if_fail (shortcut != NULL);

  if (G_LIKELY (gtk_tree_model_get_iter_first (model, &iter)))
    {
      do
        {
          gtk_tree_model_get (model, &iter, SHORTCUTS_FEATURE_COLUMN, &feature, -1);

          if (G_UNLIKELY (g_str_equal (feature, shortcut->command)))
            {
              gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                                  SHORTCUTS_SHORTCUT_COLUMN, shortcut->shortcut, -1);
            }

          g_free (feature);
        }
      while (gtk_tree_model_iter_next (model, &iter));
    }
}



static void
xfwm_settings_reload_shortcuts (XfwmSettings *settings)
{
  GtkTreeModel *model;
  GtkWidget    *view;
  GList        *shortcuts;

  g_return_if_fail (XFWM_IS_SETTINGS (settings));
  g_return_if_fail (GLADE_IS_XML (settings->priv->glade_xml));
  g_return_if_fail (XFCE_IS_SHORTCUTS_PROVIDER (settings->priv->provider));

  view = glade_xml_get_widget (settings->priv->glade_xml, "shortcuts_treeview");
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (view));

  xfwm_settings_clear_shortcuts_view (settings);

  shortcuts = xfce_shortcuts_provider_get_shortcuts (settings->priv->provider);
  g_list_foreach (shortcuts, (GFunc) xfwm_settings_reload_shortcut, model);
  xfce_shortcuts_free (shortcuts);
}



static void
xfwm_settings_shortcut_added (XfceShortcutsProvider *provider,
                              const gchar           *shortcut,
                              XfwmSettings          *settings)
{
  g_return_if_fail (XFWM_IS_SETTINGS (settings));
  xfwm_settings_reload_shortcuts (settings);
}



static void
xfwm_settings_shortcut_removed (XfceShortcutsProvider *provider,
                                const gchar           *shortcut,
                                XfwmSettings          *settings)
{
  g_return_if_fail (XFWM_IS_SETTINGS (settings));
  xfwm_settings_reload_shortcuts (settings);
}



static void
xfwm_settings_shortcut_clear_clicked (GtkButton    *button,
                                      XfwmSettings *settings)
{
  GtkTreeSelection *selection;
  GtkTreeModel     *model;
  GtkTreePath      *path;
  GtkTreeIter       tree_iter;
  GtkWidget        *view;
  GList            *rows;
  GList            *iter;
  GList            *row_references = NULL;
  gchar            *shortcut;

  g_return_if_fail (XFWM_IS_SETTINGS (settings));
  g_return_if_fail (GLADE_IS_XML (settings->priv->glade_xml));
  g_return_if_fail (XFCE_IS_SHORTCUTS_PROVIDER (settings->priv->provider));

  view = glade_xml_get_widget (settings->priv->glade_xml, "shortcuts_treeview");
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));
  rows = gtk_tree_selection_get_selected_rows (selection, &model);

  for (iter = g_list_first (rows); iter != NULL; iter = g_list_next (iter))
    {
      row_references = g_list_append (row_references,
                                      gtk_tree_row_reference_new (model, iter->data));
    }

  for (iter = g_list_first (row_references); iter != NULL; iter = g_list_next (iter))
    {
      path = gtk_tree_row_reference_get_path (iter->data);

      /* Convert tree path to tree iter */
      if (G_LIKELY (gtk_tree_model_get_iter (model, &tree_iter, path)))
        {
          /* Read shortcut */
          gtk_tree_model_get (model, &tree_iter, SHORTCUTS_SHORTCUT_COLUMN, &shortcut, -1);

          if (G_LIKELY (shortcut != NULL))
            {
              DBG ("clear shortcut %s", shortcut);

              /* Remove keyboard shortcut via xfconf */
              xfce_shortcuts_provider_reset_shortcut (settings->priv->provider, shortcut);

              /* Free shortcut string */
              g_free (shortcut);
            }
        }

      gtk_tree_path_free (path);
    }

  /* Free row reference list */
  g_list_foreach (row_references, (GFunc) gtk_tree_row_reference_free, NULL);
  g_list_free (row_references);

  /* Free row list */
  g_list_foreach (rows, (GFunc) gtk_tree_path_free, NULL);
  g_list_free (rows);
}



static void
xfwm_settings_shortcut_reset_clicked (GtkButton    *button,
                                      XfwmSettings *settings)
{
  gint response;

  g_return_if_fail (XFWM_IS_SETTINGS (settings));
  g_return_if_fail (XFCE_IS_SHORTCUTS_PROVIDER (settings->priv->provider));

  response = xfce_message_dialog (NULL, _("Reset to Defaults"), GTK_STOCK_DIALOG_QUESTION,
                                  _("Reset to Defaults"),
                                  _("This will reset all shortcuts to their default "
                                    "values. Do you really want to do this?"),
                                  GTK_STOCK_NO, GTK_RESPONSE_NO,
                                  GTK_STOCK_YES, GTK_RESPONSE_YES,
                                  NULL);

  if (G_LIKELY (response == GTK_RESPONSE_YES))
    xfce_shortcuts_provider_reset_to_defaults (settings->priv->provider);
}



static const gchar *
xfwm_settings_shortcut_feature_name (const gchar *feature)
{
  const gchar *result = feature;
  gint         i;

  for (i = 0; shortcut_values[i].name != NULL; ++i)
    if (G_UNLIKELY (g_str_equal (shortcut_values[i].feature, feature)))
      {
        result = shortcut_values[i].name;
        break;
      }

  return result;
}



static gboolean
xfwm_settings_validate_shortcut (XfceShortcutDialog  *dialog,
                                 const gchar         *shortcut,
                                 XfwmSettings        *settings)
{
  XfceShortcutsProvider *other_provider = NULL;
  XfceShortcut          *other_shortcut = NULL;
  GList                 *providers;
  GList                 *iter;
  gboolean               accepted = TRUE;
  gint                   response;

  g_return_val_if_fail (XFCE_IS_SHORTCUT_DIALOG (dialog), FALSE);
  g_return_val_if_fail (XFWM_IS_SETTINGS (settings), FALSE);
  g_return_val_if_fail (shortcut != NULL, FALSE);

  /* Ignore empty shortcuts */
  if (G_UNLIKELY (g_utf8_strlen (shortcut, -1) == 0))
    return FALSE;

  /* Ignore raw 'Return' and 'space' since that may have been used to activate the shortcut row */
  if (G_UNLIKELY (g_utf8_collate (shortcut, "Return") == 0 ||
                  g_utf8_collate (shortcut, "space") == 0))
    {
      return FALSE;
    }

  providers = xfce_shortcuts_provider_get_providers ();

  if (G_UNLIKELY (providers == NULL))
    return TRUE;

  for (iter = providers; iter != NULL && other_shortcut == NULL; iter = g_list_next (iter))
    {
      if (G_UNLIKELY (xfce_shortcuts_provider_has_shortcut (iter->data, shortcut)))
        {
          other_provider = g_object_ref (iter->data);
          other_shortcut = xfce_shortcuts_provider_get_shortcut (iter->data, shortcut);
        }
    }

  xfce_shortcuts_provider_free_providers (providers);

  if (G_UNLIKELY (other_shortcut != NULL))
    {
      response = xfce_shortcut_conflict_dialog (xfce_shortcuts_provider_get_name (settings->priv->provider),
                                                xfce_shortcuts_provider_get_name (other_provider),
                                                shortcut,
                                                xfce_shortcut_dialog_get_action_name (dialog),
                                                xfwm_settings_shortcut_feature_name (other_shortcut->command),
                                                FALSE);

      accepted = response == GTK_RESPONSE_ACCEPT;

      xfce_shortcut_free (other_shortcut);
      g_object_unref (other_provider);
    }

  return accepted;
}



static void
xfwm_settings_shortcut_row_activated (GtkTreeView       *tree_view,
                                      GtkTreePath       *path,
                                      GtkTreeViewColumn *column,
                                      XfwmSettings      *settings)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  GtkWidget    *dialog;
  const gchar  *new_shortcut;
  gchar        *shortcut;
  gchar        *feature;
  gchar        *name;
  gint          response;

  g_return_if_fail (XFWM_IS_SETTINGS (settings));
  g_return_if_fail (XFCE_IS_SHORTCUTS_PROVIDER (settings->priv->provider));

  model = gtk_tree_view_get_model (tree_view);

  if (G_LIKELY (gtk_tree_model_get_iter (model, &iter, path)))
    {
      /* Read shortcut from the activated row */
      gtk_tree_model_get (model, &iter,
                          SHORTCUTS_NAME_COLUMN, &name,
                          SHORTCUTS_FEATURE_COLUMN, &feature,
                          SHORTCUTS_SHORTCUT_COLUMN, &shortcut, -1);

      /* Request a new shortcut from the user */
      dialog = xfce_shortcut_dialog_new ("xfwm4", name, feature);
      g_signal_connect (dialog, "validate-shortcut",
                        G_CALLBACK (xfwm_settings_validate_shortcut), settings);
      response = xfce_shortcut_dialog_run (XFCE_SHORTCUT_DIALOG (dialog));

      if (G_LIKELY (response == GTK_RESPONSE_OK))
        {
          /* Remove old shortcut from the settings */
          if (G_LIKELY (shortcut != NULL))
            xfce_shortcuts_provider_reset_shortcut (settings->priv->provider, shortcut);

          /* Get new shortcut entered by the user */
          new_shortcut = xfce_shortcut_dialog_get_shortcut (XFCE_SHORTCUT_DIALOG (dialog));

          /* Save new shortcut */
          xfce_shortcuts_provider_set_shortcut (settings->priv->provider, new_shortcut, feature);
        }
      else if (G_UNLIKELY (response == GTK_RESPONSE_REJECT))
        {
          /* Remove old shortcut from the settings */
          if (G_LIKELY (shortcut != NULL))
            xfce_shortcuts_provider_reset_shortcut (settings->priv->provider, shortcut);
        }

      /* Destroy the shortcut dialog */
      gtk_widget_destroy (dialog);

      /* Free strings */
      g_free (feature);
      g_free (name);
      g_free (shortcut);
    }
}
