/* gtkplot3d - 3d scientific plots widget for gtk+
 * Copyright 1999-2001  Adrian E. Feiguin <feiguin@ifir.edu.ar>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION: gtkplot3d
 * @short_description: 3d scientific plots widget
 *
 * FIXME:: Need long description.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
#include "gtkplot.h"
#include "gtkplotdata.h"
#include "gtkplotsurface.h"
#include "gtkplot3d.h"
#include "gtkpsfont.h"

#define DEFAULT_WIDTH 420
#define DEFAULT_HEIGHT 340
#define DEFAULT_FONT_HEIGHT 10 
/* This should be same as in gtkplot.c */
#define LABEL_MAX_LENGTH 100

#ifndef PI
#define PI 3.141592653589793238462643383279502884197
#endif
#ifndef SQRT2
#define SQRT2 1.41421356237309504880168872420969807856967 
#endif

#define P_(string) string

enum
{
  PROP_0,
  PROP_CENTER,
  PROP_ORIGIN,
  PROP_A1,
  PROP_A2,
  PROP_A3,
  PROP_XY_VISIBLE,
  PROP_YZ_VISIBLE,
  PROP_ZX_VISIBLE,
  PROP_COLOR_XY,
  PROP_COLOR_YZ,
  PROP_COLOR_ZX,
  PROP_FRAME,
  PROP_CORNER,
  PROP_CORNER_VISIBLE,
  PROP_ZMIN,
  PROP_ZMAX,
  PROP_ZSCALE,
  PROP_TITLES_OFFSET,
  PROP_XFACTOR,
  PROP_YFACTOR,
  PROP_ZFACTOR,
  PROP_E1,
  PROP_E2,
  PROP_E3,
  PROP_XY_LABEL_MASK,
  PROP_XY_MAJOR_MASK,
  PROP_XY_MINOR_MASK,
  PROP_XY_TITLE_VISIBLE,
  PROP_XZ_LABEL_MASK,
  PROP_XZ_MAJOR_MASK,
  PROP_XZ_MINOR_MASK,
  PROP_XZ_TITLE_VISIBLE,
  PROP_YX_LABEL_MASK,
  PROP_YX_MAJOR_MASK,
  PROP_YX_MINOR_MASK,
  PROP_YX_TITLE_VISIBLE,
  PROP_YZ_LABEL_MASK,
  PROP_YZ_MAJOR_MASK,
  PROP_YZ_MINOR_MASK,
  PROP_YZ_TITLE_VISIBLE,
  PROP_ZX_LABEL_MASK,
  PROP_ZX_MAJOR_MASK,
  PROP_ZX_MINOR_MASK,
  PROP_ZX_TITLE_VISIBLE,
  PROP_ZY_LABEL_MASK,
  PROP_ZY_MAJOR_MASK,
  PROP_ZY_MINOR_MASK,
  PROP_ZY_TITLE_VISIBLE,
};

static void gtk_plot3d_class_init 		(GtkPlot3DClass *klass);
static void gtk_plot3d_init 			(GtkPlot3D *plot);
static void gtk_plot3d_destroy 			(GtkWidget *object);
static void gtk_plot3d_set_property             (GObject *object,
			                         guint prop_id,
			                         const GValue *value,
			                         GParamSpec *pspec);
static void gtk_plot3d_get_property             (GObject *object,
			                         guint prop_id,
			                         GValue *value,
			                         GParamSpec *pspec);
static void gtk_plot3d_real_paint 		(GtkWidget *widget);
static void gtk_plot3d_draw_plane		(GtkPlot3D *plot, 
						 GtkPlotVector v1, 
						 GtkPlotVector v2, 
						 GtkPlotVector v3, 
						 GtkPlotVector v4, 
						 GdkRGBA background);
static void gtk_plot3d_draw_grids               (GtkPlot3D *plot, 
                                                 GtkPlotAxis *axis,
                                                 GtkPlotVector origin);
static void gtk_plot3d_draw_axis		(GtkPlot3D *plot, 
					 	 GtkPlotAxis *axis,
                                                 GtkPlotVector tick,
                                                 GtkPlotVector delta);
static void gtk_plot3d_draw_labels		(GtkPlot3D *plot, 
						 GtkPlotAxis *axis, 
                                                 GtkPlotVector delta);
static void gtk_plot3d_real_get_pixel		(GtkWidget *widget, 
                          			 gdouble x, 
						 gdouble y, 
						 gdouble z,
                          			 gdouble *px, 
						 gdouble *py, 
						 gdouble *pz);
gint roundint			(gdouble x);

static GtkPlotClass *parent_class = NULL;

GType
gtk_plot3d_get_type (void)
{
  static GType plot_type = 0;

  if (!plot_type)
    {
      plot_type = g_type_register_static_simple (
		gtk_plot_get_type(),
		"GtkPlot3D",
		sizeof (GtkPlot3DClass),
		(GClassInitFunc) gtk_plot3d_class_init,
		sizeof (GtkPlot3D),
		(GInstanceInitFunc) gtk_plot3d_init,
		0);
    }
  return plot_type;
}

static void
gtk_plot3d_class_init (GtkPlot3DClass *klass)
{
  GtkWidgetClass *object_class;
  GtkPlotClass *plot_class;
  GtkPlot3DClass *plot3d_class;
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  parent_class = g_type_class_ref (gtk_plot_get_type ());

  object_class = (GtkWidgetClass *) klass;
  plot_class = (GtkPlotClass *) klass;
  plot3d_class = (GtkPlot3DClass *) klass;

  object_class->destroy = gtk_plot3d_destroy;
  gobject_class->set_property = gtk_plot3d_set_property;
  gobject_class->get_property = gtk_plot3d_get_property;

  plot_class->plot_paint = gtk_plot3d_real_paint;
  plot3d_class->get_pixel = gtk_plot3d_real_get_pixel;


  /**
   * GtkPlot3D:center_vector:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_CENTER,
  g_param_spec_pointer ("center_vector",
                       P_("Center"),
                       P_("Position of the center point"),
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:origin_vector:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_ORIGIN,
  g_param_spec_pointer ("origin_vector",
                       P_("Origin"),
                       P_("Position of the origin"),
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:a1:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_A1,
  g_param_spec_double ("a1",
                       P_("Angle 1"),
                       P_("Angle 1"),
                           -G_MAXDOUBLE, G_MAXDOUBLE,0.0,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:a2:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_A2,
  g_param_spec_double ("a2",
                       P_("Angle 2"),
                       P_("Angle 2"),
                           -G_MAXDOUBLE, G_MAXDOUBLE,0.0,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:a3:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_A3,
  g_param_spec_double ("a3",
                       P_("Angle 3"),
                       P_("Angle 3"),
                           -G_MAXDOUBLE, G_MAXDOUBLE,0.0,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:xy_visible:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_XY_VISIBLE,
  g_param_spec_boolean ("xy_visible",
                       P_("XY Visible"),
                       P_("XY Plane visible"),
                           FALSE,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:yz_visible:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_YZ_VISIBLE,
  g_param_spec_boolean ("yz_visible",
                       P_("YZ Visible"),
                       P_("YZ Plane visible"),
                           FALSE,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:zx_visible:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_ZX_VISIBLE,
  g_param_spec_boolean ("zx_visible",
                       P_("ZX Visible"),
                       P_("ZX Plane visible"),
                           FALSE,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:color_xy:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_COLOR_XY,
  g_param_spec_pointer ("color_xy",
                       P_("XY Color"),
                       P_("Color of XY Plane"),
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:color_yz:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_COLOR_YZ,
  g_param_spec_pointer ("color_yz",
                       P_("YZ Color"),
                       P_("Color of YZ Plane"),
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:color_zx:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_COLOR_ZX,
  g_param_spec_pointer ("color_zx",
                       P_("ZX Color"),
                       P_("Color of ZX Plane"),
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:frame_line:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_FRAME,
  g_param_spec_pointer ("frame_line",
                       P_("Frame Line"),
                       P_("Frame Line"),
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:corner_line:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_CORNER,
  g_param_spec_pointer ("corner_line",
                       P_("Corner Line"),
                       P_("Corner Line"),
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:corner_visible:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_CORNER_VISIBLE,
  g_param_spec_boolean ("corner_visible",
                       P_("Corner Visible"),
                       P_("Draw the entire cube"),
                           FALSE,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:zmin:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_ZMIN,
  g_param_spec_double ("zmin",
                       P_("Z Min"),
                       P_("Min value of the Z axis"),
                           -G_MAXDOUBLE, G_MAXDOUBLE,0.0,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:zmax:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_ZMAX,
  g_param_spec_double ("zmax",
                       P_("Z Max"),
                       P_("Max value of the Z axis"),
                           -G_MAXDOUBLE, G_MAXDOUBLE,0.0,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:zscale:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_ZSCALE,
  g_param_spec_int ("zscale",
                       P_("Z Scale"),
                       P_("Scale used for the Z axis"),
                           0,G_MAXINT,0,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:titles_offset:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_TITLES_OFFSET,
  g_param_spec_int ("titles_offset",
                       P_("Titles Offset"),
                       P_("Titles distance from the axes"),
                           -G_MAXINT,G_MAXINT,0,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:xfactor:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_XFACTOR,
  g_param_spec_double ("xfactor",
                       P_("X Factor"),
                       P_("Relative size of the x axis"),
                           0.0, G_MAXDOUBLE,0.0,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:yfactor:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_YFACTOR,
  g_param_spec_double ("yfactor",
                       P_("Y Factor"),
                       P_("Relative size of the y axis"),
                           0.0, G_MAXDOUBLE,0.0,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:zfactor:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_ZFACTOR,
  g_param_spec_double ("zfactor",
                       P_("Z Factor"),
                       P_("Relative size of the z axis"),
                           0.0, G_MAXDOUBLE,0.0,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:e1_vector:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_E1,
  g_param_spec_pointer ("e1_vector",
                       P_("E1"),
                       P_("Vector e1"),
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:e2_vector:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_E2,
  g_param_spec_pointer ("e2_vector",
                       P_("E2"),
                       P_("Vector e2"),
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:e3_vector:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_E3,
  g_param_spec_pointer ("e3_vector",
                       P_("E3"),
                       P_("Vector e3"),
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:xy_label_mask:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_XY_LABEL_MASK,
  g_param_spec_int ("xy_label_mask",
                    P_("XY label mask"),
                    P_("XY label mask"),
                    0,3,0,
                    G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:xy_major_mask:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_XY_MAJOR_MASK,
  g_param_spec_int ("xy_major_mask",
                    P_("XY major mask"),
                    P_("XY major mask"),
                    0,3,0,
                    G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:xy_minor_mask:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_XY_MINOR_MASK,
  g_param_spec_int ("xy_minor_mask",
                    P_("XY minor mask"),
                    P_("XY minor mask"),
                    0,3,0,
                    G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:xy_title_visible:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_XY_TITLE_VISIBLE,
  g_param_spec_boolean ("xy_title_visible",
                       P_("XY Title Visible"),
                       P_("XY title Visible"),
                       FALSE,
                       G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:xz_label_mask:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_XZ_LABEL_MASK,
  g_param_spec_int ("xz_label_mask",
                    P_("XZ label mask"),
                    P_("XZ label mask"),
                    0,3,0,
                    G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:xz_major_mask:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_XZ_MAJOR_MASK,
  g_param_spec_int ("xz_major_mask",
                    P_("XZ major mask"),
                    P_("XZ major mask"),
                    0,3,0,
                    G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:xz_minor_mask:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_XZ_MINOR_MASK,
  g_param_spec_int ("xz_minor_mask",
                    P_("XZ minor mask"),
                    P_("XZ minor mask"),
                    0,3,0,
                    G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:xz_title_visible:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_XZ_TITLE_VISIBLE,
  g_param_spec_boolean ("xz_title_visible",
                       P_("XZ Title Visible"),
                       P_("XZ title Visible"),
                       FALSE,
                       G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:yz_label_mask:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_YZ_LABEL_MASK,
  g_param_spec_int ("yz_label_mask",
                    P_("YZ label mask"),
                    P_("YZ label mask"),
                    0,3,0,
                    G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:yz_major_mask:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_YZ_MAJOR_MASK,
  g_param_spec_int ("yz_major_mask",
                    P_("YZ major mask"),
                    P_("YZ major mask"),
                    0,3,0,
                    G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:yz_minor_mask:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_YZ_MINOR_MASK,
  g_param_spec_int ("yz_minor_mask",
                    P_("YZ minor mask"),
                    P_("YZ minor mask"),
                    0,3,0,
                    G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:yz_title_visible:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_YZ_TITLE_VISIBLE,
  g_param_spec_boolean ("yz_title_visible",
                       P_("YZ Title Visible"),
                       P_("YZ title Visible"),
                       FALSE,
                       G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:yx_label_mask:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_YX_LABEL_MASK,
  g_param_spec_int ("yx_label_mask",
                    P_("YX label mask"),
                    P_("YX label mask"),
                    0,3,0,
                    G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:yx_major_mask:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_YX_MAJOR_MASK,
  g_param_spec_int ("yx_major_mask",
                    P_("YX major mask"),
                    P_("YX major mask"),
                    0,3,0,
                    G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:yx_minor_mask:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_YX_MINOR_MASK,
  g_param_spec_int ("yx_minor_mask",
                    P_("YX minor mask"),
                    P_("YX minor mask"),
                    0,3,0,
                    G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:yx_title_visible:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_YX_TITLE_VISIBLE,
  g_param_spec_boolean ("yx_title_visible",
                       P_("YX Title Visible"),
                       P_("YX title Visible"),
                       FALSE,
                       G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:zx_label_mask:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_ZX_LABEL_MASK,
  g_param_spec_int ("zx_label_mask",
                    P_("ZX label mask"),
                    P_("ZX label mask"),
                    0,3,0,
                    G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:zx_major_mask:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_ZX_MAJOR_MASK,
  g_param_spec_int ("zx_major_mask",
                    P_("ZX major mask"),
                    P_("ZX major mask"),
                    0,3,0,
                    G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:zx_minor_mask:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_ZX_MINOR_MASK,
  g_param_spec_int ("zx_minor_mask",
                    P_("ZX minor mask"),
                    P_("ZX minor mask"),
                    0,3,0,
                    G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:zx_title_visible:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_ZX_TITLE_VISIBLE,
  g_param_spec_boolean ("zx_title_visible",
                       P_("ZX Title Visible"),
                       P_("ZX title Visible"),
                       FALSE,
                       G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:zy_label_mask:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_ZY_LABEL_MASK,
  g_param_spec_int ("zy_label_mask",
                    P_("ZY label mask"),
                    P_("ZY label mask"),
                    0,3,0,
                    G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:zy_major_mask:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_ZY_MAJOR_MASK,
  g_param_spec_int ("zy_major_mask",
                    P_("ZY major mask"),
                    P_("ZY major mask"),
                    0,3,0,
                    G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:zy_minor_mask:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_ZY_MINOR_MASK,
  g_param_spec_int ("zy_minor_mask",
                    P_("ZY minor mask"),
                    P_("ZY minor mask"),
                    0,3,0,
                    G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlot3D:zy_title_visible:
   *
   *
   **/
  g_object_class_install_property(gobject_class,
                           PROP_ZY_TITLE_VISIBLE,
  g_param_spec_boolean ("zy_title_visible",
                       P_("ZY Title Visible"),
                       P_("ZY title Visible"),
                       FALSE,
                       G_PARAM_READABLE|G_PARAM_WRITABLE));
}


static void
gtk_plot3d_init (GtkPlot3D *plot)
{
  GdkRGBA color, black, white;
  gint i;

  gtk_widget_set_has_window(GTK_WIDGET(plot), FALSE);

  for(i = 0; i < 360; i++){
    plot->ncos[i] = cos(i*PI/180.);
    plot->nsin[i] = sin(i*PI/180.);
  }

  gdk_rgba_parse(&black, "black");
  gdk_rgba_parse(&white, "white");

  GTK_PLOT(plot)->legends_x = .8;

  plot->ax = GTK_PLOT(plot)->bottom;
  plot->ay = GTK_PLOT(plot)->left;
  plot->az = GTK_PLOT(plot)->top;

  plot->center.x = 0.5;
  plot->center.y = 0.5;
  plot->center.z = 0.5;

  plot->xfactor = 1.0;
  plot->yfactor = 1.0;
  plot->zfactor = 1.0;

  gtk_plot3d_reset_angles(plot);
  gtk_plot3d_rotate_x(plot, 60.);
  gtk_plot3d_rotate_z(plot, 30.);

  GTK_PLOT(plot)->xmin = 0.;
  GTK_PLOT(plot)->xmax = 1.000000;
  GTK_PLOT(plot)->ymin = 0.;
  GTK_PLOT(plot)->ymax = 1.000000;
  plot->zmin = 0.;
  plot->zmax = 1.000000;

  plot->xy_visible = TRUE;
  plot->yz_visible = TRUE;
  plot->zx_visible = TRUE;

  plot->ax->show_major_grid = FALSE;
  plot->ax->show_minor_grid = FALSE;
  plot->ay->show_major_grid = FALSE;
  plot->ay->show_minor_grid = FALSE;
  plot->az->show_major_grid = FALSE;
  plot->az->show_minor_grid = FALSE;
 
  plot->ax->show_major_grid = TRUE;
  plot->ax->show_minor_grid = TRUE;
  plot->ay->show_major_grid = TRUE;
  plot->ay->show_minor_grid = TRUE;
  plot->az->show_major_grid = TRUE;
  plot->az->show_minor_grid = TRUE;

  plot->ax->ticks.nmajorticks = 0;
  plot->ax->ticks.nminorticks = 0;
  plot->ax->ticks.values = NULL;
  plot->ax->ticks.nticks = 0;
  plot->ax->ticks.set_limits = FALSE;
  plot->ax->ticks.begin = 0;
  plot->ax->ticks.end = 0;
  plot->ax->ticks.step = .100000000;
  plot->ax->ticks.nminor = 1;

  plot->ay->ticks.nmajorticks = 0;
  plot->ay->ticks.nminorticks = 0;
  plot->ay->ticks.values = NULL;
  plot->ay->ticks.nticks = 0;
  plot->ay->ticks.set_limits = FALSE;
  plot->ay->ticks.begin = 0;
  plot->ay->ticks.end = 0;
  plot->ay->ticks.step = .100000000;
  plot->ay->ticks.nminor = 1;

  plot->az->ticks.nmajorticks = 0;
  plot->az->ticks.nminorticks = 0;
  plot->az->ticks.values = NULL;
  plot->az->ticks.nticks = 0;
  plot->az->ticks.set_limits = FALSE;
  plot->az->ticks.begin = 0;
  plot->az->ticks.end = 0;
  plot->az->ticks.step = .100000000;
  plot->az->ticks.nminor = 1;

  plot->ax->ticks.min = 0.0;
  plot->ax->ticks.max = 1.0;
  plot->ax->labels_offset = 25;
  plot->ax->major_mask = GTK_PLOT_TICKS_OUT;
  plot->ax->minor_mask = GTK_PLOT_TICKS_OUT;
  plot->ax->ticks_length = 8;
  plot->ax->ticks_width = 1;
  plot->ax->orientation = GTK_PLOT_AXIS_X;
  plot->ax->ticks.scale = GTK_PLOT_SCALE_LINEAR;
  plot->ax->is_visible = TRUE;
  plot->ax->custom_labels = FALSE;
  plot->ay->ticks.min = 0.0;
  plot->ay->ticks.max = 1.0;
  plot->ay->major_mask = GTK_PLOT_TICKS_OUT;
  plot->ay->minor_mask = GTK_PLOT_TICKS_OUT;
  plot->ay->ticks_length = 8;
  plot->ay->ticks_width = 1;
  plot->ay->labels_offset = 25;
  plot->ay->orientation = GTK_PLOT_AXIS_Y;
  plot->ay->ticks.scale = GTK_PLOT_SCALE_LINEAR;
  plot->ay->is_visible = TRUE;
  plot->ay->custom_labels = FALSE;
  plot->az->ticks.min = 0.0;
  plot->az->ticks.max = 1.0;
  plot->az->major_mask = GTK_PLOT_TICKS_OUT;
  plot->az->minor_mask = GTK_PLOT_TICKS_OUT;
  plot->az->ticks_length = 8;
  plot->az->ticks_width = 1;
  plot->az->labels_offset = 25;
  plot->az->orientation = GTK_PLOT_AXIS_Z;
  plot->az->ticks.scale = GTK_PLOT_SCALE_LINEAR;
  plot->az->is_visible = TRUE;
  plot->az->custom_labels = FALSE;

  plot->az->line.line_style = GTK_PLOT_LINE_SOLID;
  plot->az->line.line_width = 2;
  plot->az->line.color = black;
  plot->az->labels_attr.text = NULL;
  plot->az->labels_attr.height = DEFAULT_FONT_HEIGHT;
  plot->az->labels_attr.fg = black;
  plot->az->labels_attr.bg = white;
  plot->az->labels_attr.transparent = TRUE;
  plot->az->labels_attr.justification = GTK_JUSTIFY_CENTER;
  plot->az->labels_attr.angle = 0;
  plot->az->label_mask = GTK_PLOT_LABEL_OUT;
  plot->az->label_style = GTK_PLOT_LABEL_FLOAT;
  plot->az->label_precision = 1;
  plot->az->title.angle = 90;
  plot->az->title.justification = GTK_JUSTIFY_CENTER;
  plot->az->title.height = DEFAULT_FONT_HEIGHT;
  plot->az->title.fg = black;
  plot->az->title.bg = white;
  plot->az->title.transparent = TRUE;
  plot->az->title_visible = TRUE;

  plot->ax->line.line_style = GTK_PLOT_LINE_SOLID;
  plot->ax->line.line_width = 2;
  plot->ax->line.color = black;
  plot->ax->labels_attr.text = NULL;
  plot->ax->labels_attr.height = DEFAULT_FONT_HEIGHT;
  plot->ax->labels_attr.fg = black;
  plot->ax->labels_attr.bg = white;
  plot->ax->labels_attr.transparent = TRUE;
  plot->ax->labels_attr.justification = GTK_JUSTIFY_CENTER;
  plot->ax->labels_attr.angle = 0;
  plot->ax->label_mask = GTK_PLOT_LABEL_OUT;
  plot->ax->label_style = GTK_PLOT_LABEL_FLOAT;
  plot->ax->label_precision = 1;
  plot->ax->title.angle = 0;
  plot->ax->title.justification = GTK_JUSTIFY_CENTER;
  plot->ax->title.height = DEFAULT_FONT_HEIGHT;
  plot->ax->title.fg = black;
  plot->ax->title.bg = white;
  plot->ax->title.transparent = TRUE;
  plot->ax->title_visible = TRUE;

  plot->ay->line.line_style = GTK_PLOT_LINE_SOLID;
  plot->ay->line.line_width = 2;
  plot->ay->line.color = black;
  plot->ay->labels_attr.text = NULL;
  plot->ay->labels_attr.height = DEFAULT_FONT_HEIGHT;
  plot->ay->labels_attr.fg = black;
  plot->ay->labels_attr.bg = white;
  plot->ay->labels_attr.transparent = TRUE;
  plot->ay->labels_attr.angle = 0;
  plot->ay->label_mask = GTK_PLOT_LABEL_OUT;
  plot->ay->label_style = GTK_PLOT_LABEL_FLOAT;
  plot->ay->label_precision = 1;
  plot->ay->labels_attr.justification = GTK_JUSTIFY_CENTER;
  plot->ay->title.angle = 0;
  plot->ay->title.justification = GTK_JUSTIFY_CENTER;
  plot->ay->title.height = DEFAULT_FONT_HEIGHT;
  plot->ay->title.fg = black;
  plot->ay->title.bg = white;
  plot->ay->title.transparent = TRUE;
  plot->ay->title_visible = TRUE;

  gtk_plot_axis_set_title(GTK_PLOT(plot)->bottom, "X Title");
  gtk_plot_axis_set_title(GTK_PLOT(plot)->left, "Y Title");
  gtk_plot_axis_set_title(GTK_PLOT(plot)->top, "Z Title");

  GTK_PLOT(plot)->xscale = GTK_PLOT_SCALE_LINEAR;
  GTK_PLOT(plot)->yscale = GTK_PLOT_SCALE_LINEAR;
  plot->zscale = GTK_PLOT_SCALE_LINEAR;

  plot->xy = GTK_PLOT_AXIS(gtk_plot_axis_new(GTK_PLOT_AXIS_X));  
  g_object_ref (plot->xy);
  g_object_ref_sink (plot->xy);
  g_object_unref (plot->xy);
  plot->xz = GTK_PLOT_AXIS(gtk_plot_axis_new(GTK_PLOT_AXIS_X));  
  g_object_ref (plot->xz);
  g_object_ref_sink (plot->xz);
  g_object_unref (plot->xz);
  plot->yx = GTK_PLOT_AXIS(gtk_plot_axis_new(GTK_PLOT_AXIS_Y));  
  g_object_ref (plot->yx);
  g_object_ref_sink (plot->yx);
  g_object_unref (plot->yx);
  plot->yz = GTK_PLOT_AXIS(gtk_plot_axis_new(GTK_PLOT_AXIS_Y));  
  g_object_ref (plot->yz);
  g_object_ref_sink (plot->yz);
  g_object_unref (plot->yz);
  plot->zx = GTK_PLOT_AXIS(gtk_plot_axis_new(GTK_PLOT_AXIS_Z));  
  g_object_ref (plot->zx);
  g_object_ref_sink (plot->zx);
  g_object_unref (plot->zx);
  plot->zy = GTK_PLOT_AXIS(gtk_plot_axis_new(GTK_PLOT_AXIS_Z));  
  g_object_ref (plot->zy);
  g_object_ref_sink (plot->zy);
  g_object_unref (plot->zy);

  plot->xy->major_mask = plot->ax->major_mask;
  plot->xy->minor_mask = plot->ax->minor_mask;
  plot->xy->label_mask = plot->ax->label_mask;
  plot->xz->major_mask = plot->ax->major_mask;
  plot->xz->minor_mask = plot->ax->minor_mask;
  plot->xz->label_mask = plot->ax->label_mask;
  plot->yx->major_mask = plot->ay->major_mask;
  plot->yx->minor_mask = plot->ay->minor_mask;
  plot->yx->label_mask = plot->ay->label_mask;
  plot->yz->major_mask = plot->ay->major_mask;
  plot->yz->minor_mask = plot->ay->minor_mask;
  plot->yz->label_mask = plot->ay->label_mask;
  plot->zx->major_mask = plot->az->major_mask;
  plot->zx->minor_mask = plot->az->minor_mask;
  plot->zx->label_mask = plot->az->label_mask;
  plot->zy->major_mask = plot->az->major_mask;
  plot->zy->minor_mask = plot->az->minor_mask;
  plot->zy->label_mask = plot->az->label_mask;

  plot->xy->title_visible = plot->ax->title_visible;
  plot->xz->title_visible = plot->ax->title_visible;
  plot->yx->title_visible = plot->ay->title_visible;
  plot->yz->title_visible = plot->ay->title_visible;
  plot->zx->title_visible = plot->az->title_visible;
  plot->zy->title_visible = plot->az->title_visible;

  plot->frame.color = black;
  plot->frame.line_width = 1;
  plot->frame.line_style = GTK_PLOT_LINE_SOLID;

  plot->corner_visible = FALSE;
  plot->corner.line_style = GTK_PLOT_LINE_SOLID;
  plot->corner.line_width = 0;
  plot->corner.color = black;

  plot->ax->direction = plot->e1;
  plot->ay->direction = plot->e2;
  plot->az->direction = plot->e3;

  gdk_rgba_parse(&color, "gray95");
  plot->color_xy = color;

  gdk_rgba_parse(&color, "gray80");
  plot->color_yz = color;

  gdk_rgba_parse(&color, "gray65");
  plot->color_zx = color;

  plot->titles_offset = 60;
  GTK_PLOT(plot)->legends_attr.transparent = FALSE;

  gtk_plot_axis_ticks_recalc(plot->ax);
  gtk_plot_axis_ticks_recalc(plot->ay);
  gtk_plot_axis_ticks_recalc(plot->az);

  GTK_PLOT(plot)->clip_data = TRUE;

  gtk_psfont_init();
}

static void
gtk_plot3d_destroy (GtkWidget *object)
{
  GtkPlot3D *plot = GTK_PLOT3D(object);
  gtk_psfont_unref();

  if (plot->xy)
    g_object_unref(plot->xy);
  if (plot->xz)
    g_object_unref(plot->xz);
  if (plot->yx)
    g_object_unref(plot->yx);
  if (plot->yz)
    g_object_unref(plot->yz);
  if (plot->zx)
    g_object_unref(plot->zx);
  if (plot->zy)
    g_object_unref(plot->zy);

  if ( GTK_WIDGET_CLASS (parent_class)->destroy )
    (* GTK_WIDGET_CLASS (parent_class)->destroy) (object);
}

static void
gtk_plot3d_set_property (GObject      *object,
			 guint prop_id,
			 const GValue *value,
			 GParamSpec *pspec)
{
  GtkPlot3D *plot;

  plot = GTK_PLOT3D (object);

  switch(prop_id){
    case PROP_CENTER:
      plot->center = *((GtkPlotVector *)g_value_get_pointer(value));
      break;
    case PROP_ORIGIN:
      plot->origin = *((GtkPlotVector *)g_value_get_pointer(value));
      break;
/*
    case PROP_A1:
      gtk_plot3d_rotate_x(plot, -plot->a1+g_value_get_double(value));
      break;
    case PROP_A2:
      gtk_plot3d_rotate_y(plot, -plot->a2+g_value_get_double(value));
      break;
    case PROP_A3:
      gtk_plot3d_rotate_z(plot, -plot->a3+g_value_get_double(value));
      break;
*/
    case PROP_XY_VISIBLE:
      plot->xy_visible = g_value_get_boolean(value);
      break;
    case PROP_YZ_VISIBLE:
      plot->yz_visible = g_value_get_boolean(value);
      break;
    case PROP_ZX_VISIBLE:
      plot->zx_visible = g_value_get_boolean(value);
      break;
    case PROP_COLOR_XY:
      plot->color_xy = *((GdkRGBA *)g_value_get_pointer(value));
      break;
    case PROP_COLOR_YZ:
      plot->color_yz = *((GdkRGBA *)g_value_get_pointer(value));
      break;
    case PROP_COLOR_ZX:
      plot->color_zx = *((GdkRGBA *)g_value_get_pointer(value));
      break;
    case PROP_FRAME:
      plot->frame = *((GtkPlotLine *)g_value_get_pointer(value));
      break;
    case PROP_CORNER:
      plot->corner = *((GtkPlotLine *)g_value_get_pointer(value));
      break;
    case PROP_CORNER_VISIBLE:
      plot->corner_visible = g_value_get_boolean(value);
      break;
    case PROP_ZMIN:
      plot->zmin = g_value_get_double(value);
      break;
    case PROP_ZMAX:
      plot->zmax = g_value_get_double(value);
      break;
    case PROP_ZSCALE:
      plot->zscale = g_value_get_int(value);
      break;
    case PROP_TITLES_OFFSET:
      plot->titles_offset = g_value_get_int(value);
      break;
    case PROP_XFACTOR:
      gtk_plot3d_set_xfactor(plot, g_value_get_double(value));
      break;
    case PROP_YFACTOR:
      gtk_plot3d_set_yfactor(plot, g_value_get_double(value));
      break;
    case PROP_ZFACTOR:
      gtk_plot3d_set_zfactor(plot, g_value_get_double(value));
      break;
    case PROP_E1:
      plot->e1 = *((GtkPlotVector *)g_value_get_pointer(value));
      break;
    case PROP_E2:
      plot->e2 = *((GtkPlotVector *)g_value_get_pointer(value));
      break;
    case PROP_E3:
      plot->e3 = *((GtkPlotVector *)g_value_get_pointer(value));
      break;
    case PROP_XY_MAJOR_MASK:
      plot->xy->major_mask = g_value_get_int(value);
      break;
    case PROP_XY_MINOR_MASK:
      plot->xy->minor_mask = g_value_get_int(value);
      break;
    case PROP_XY_LABEL_MASK:
      plot->xy->label_mask = g_value_get_int(value);
      break;
    case PROP_XY_TITLE_VISIBLE:
      plot->xy->title_visible = g_value_get_boolean(value);
      break;
    case PROP_XZ_MAJOR_MASK:
      plot->xz->major_mask = g_value_get_int(value);
      break;
    case PROP_XZ_MINOR_MASK:
      plot->xz->minor_mask = g_value_get_int(value);
      break;
    case PROP_XZ_LABEL_MASK:
      plot->xz->label_mask = g_value_get_int(value);
      break;
    case PROP_XZ_TITLE_VISIBLE:
      plot->xz->title_visible = g_value_get_boolean(value);
      break;
    case PROP_YX_MAJOR_MASK:
      plot->yx->major_mask = g_value_get_int(value);
      break;
    case PROP_YX_MINOR_MASK:
      plot->yx->minor_mask = g_value_get_int(value);
      break;
    case PROP_YX_LABEL_MASK:
      plot->yx->label_mask = g_value_get_int(value);
      break;
    case PROP_YX_TITLE_VISIBLE:
      plot->yx->title_visible = g_value_get_boolean(value);
      break;
    case PROP_YZ_MAJOR_MASK:
      plot->yz->major_mask = g_value_get_int(value);
      break;
    case PROP_YZ_MINOR_MASK:
      plot->yz->minor_mask = g_value_get_int(value);
      break;
    case PROP_YZ_LABEL_MASK:
      plot->yz->label_mask = g_value_get_int(value);
      break;
    case PROP_YZ_TITLE_VISIBLE:
      plot->yz->title_visible = g_value_get_boolean(value);
      break;
    case PROP_ZX_MAJOR_MASK:
      plot->zx->major_mask = g_value_get_int(value);
      break;
    case PROP_ZX_MINOR_MASK:
      plot->zx->minor_mask = g_value_get_int(value);
      break;
    case PROP_ZX_LABEL_MASK:
      plot->zx->label_mask = g_value_get_int(value);
      break;
    case PROP_ZX_TITLE_VISIBLE:
      plot->zx->title_visible = g_value_get_boolean(value);
      break;
    case PROP_ZY_MAJOR_MASK:
      plot->zy->major_mask = g_value_get_int(value);
      break;
    case PROP_ZY_MINOR_MASK:
      plot->zy->minor_mask = g_value_get_int(value);
      break;
    case PROP_ZY_LABEL_MASK:
      plot->zy->label_mask = g_value_get_int(value);
      break;
    case PROP_ZY_TITLE_VISIBLE:
      plot->zy->title_visible = g_value_get_boolean(value);
      break;
    default:
      break;
  }
}

static void
gtk_plot3d_get_property (GObject      *object,
                         guint            prop_id,
                         GValue          *value,
                         GParamSpec      *pspec)

{
  GtkPlot3D *plot;

  plot = GTK_PLOT3D (object);

  switch(prop_id){
    case PROP_CENTER:
      g_value_set_pointer(value, &plot->center);
      break;
    case PROP_ORIGIN:
      g_value_set_pointer(value, &plot->origin);
      break;
    case PROP_A1:
      g_value_set_double(value, plot->a1);
      break;
    case PROP_A2:
      g_value_set_double(value, plot->a2);
      break;
    case PROP_A3:
      g_value_set_double(value, plot->a3);
      break;
    case PROP_XY_VISIBLE:
      g_value_set_boolean(value, plot->xy_visible);
      break;
    case PROP_YZ_VISIBLE:
      g_value_set_boolean(value, plot->yz_visible);
      break;
    case PROP_ZX_VISIBLE:
      g_value_set_boolean(value, plot->zx_visible);
      break;
    case PROP_COLOR_XY:
      g_value_set_pointer(value, &plot->color_xy);
      break;
    case PROP_COLOR_YZ:
      g_value_set_pointer(value, &plot->color_yz);
      break;
    case PROP_COLOR_ZX:
      g_value_set_pointer(value, &plot->color_zx);
      break;
    case PROP_FRAME:
      g_value_set_pointer(value, &plot->frame);
      break;
    case PROP_CORNER:
      g_value_set_pointer(value, &plot->corner);
      break;
    case PROP_CORNER_VISIBLE:
      g_value_set_boolean(value, plot->corner_visible);
      break;
    case PROP_ZMIN:
      g_value_set_double(value, plot->zmin);
      break;
    case PROP_ZMAX:
      g_value_set_double(value, plot->zmax);
      break;
    case PROP_ZSCALE:
      g_value_set_int(value, plot->zscale);
      break;
    case PROP_TITLES_OFFSET:
      g_value_set_int(value, plot->titles_offset);
      break;
    case PROP_XFACTOR:
      g_value_set_double(value, plot->xfactor);
      break;
    case PROP_YFACTOR:
      g_value_set_double(value, plot->yfactor);
      break;
    case PROP_ZFACTOR:
      g_value_set_double(value, plot->zfactor);
      break;
    case PROP_E1:
      g_value_set_pointer(value, &plot->e1);
      break;
    case PROP_E2:
      g_value_set_pointer(value, &plot->e2);
      break;
    case PROP_E3:
      g_value_set_pointer(value, &plot->e3);
      break;
    case PROP_XY_LABEL_MASK:
      g_value_set_int(value, plot->xy->label_mask);
      break;
    case PROP_XY_MAJOR_MASK:
      g_value_set_int(value, plot->xy->major_mask);
      break;
    case PROP_XY_MINOR_MASK:
      g_value_set_int(value, plot->xy->minor_mask);
      break;
    case PROP_XY_TITLE_VISIBLE:
      g_value_set_boolean(value, plot->xy->title_visible);
      break;
    case PROP_XZ_LABEL_MASK:
      g_value_set_int(value, plot->xz->label_mask);
      break;
    case PROP_XZ_MAJOR_MASK:
      g_value_set_int(value, plot->xz->major_mask);
      break;
    case PROP_XZ_MINOR_MASK:
      g_value_set_int(value, plot->xz->minor_mask);
      break;
    case PROP_XZ_TITLE_VISIBLE:
      g_value_set_boolean(value, plot->xz->title_visible);
      break;
    case PROP_YZ_LABEL_MASK:
      g_value_set_int(value, plot->yz->label_mask);
      break;
    case PROP_YZ_MAJOR_MASK:
      g_value_set_int(value, plot->yz->major_mask);
      break;
    case PROP_YZ_MINOR_MASK:
      g_value_set_int(value, plot->yz->minor_mask);
      break;
    case PROP_YZ_TITLE_VISIBLE:
      g_value_set_boolean(value, plot->yz->title_visible);
      break;
    case PROP_YX_LABEL_MASK:
      g_value_set_int(value, plot->yx->label_mask);
      break;
    case PROP_YX_MAJOR_MASK:
      g_value_set_int(value, plot->yx->major_mask);
      break;
    case PROP_YX_MINOR_MASK:
      g_value_set_int(value, plot->yx->minor_mask);
      break;
    case PROP_YX_TITLE_VISIBLE:
      g_value_set_boolean(value, plot->yx->title_visible);
      break;
    case PROP_ZX_LABEL_MASK:
      g_value_set_int(value, plot->zx->label_mask);
      break;
    case PROP_ZX_MAJOR_MASK:
      g_value_set_int(value, plot->zx->major_mask);
      break;
    case PROP_ZX_MINOR_MASK:
      g_value_set_int(value, plot->zx->minor_mask);
      break;
    case PROP_ZX_TITLE_VISIBLE:
      g_value_set_boolean(value, plot->zx->title_visible);
      break;
    case PROP_ZY_LABEL_MASK:
      g_value_set_int(value, plot->zy->label_mask);
      break;
    case PROP_ZY_MAJOR_MASK:
      g_value_set_int(value, plot->zy->major_mask);
      break;
    case PROP_ZY_MINOR_MASK:
      g_value_set_int(value, plot->zy->minor_mask);
      break;
    case PROP_ZY_TITLE_VISIBLE:
      g_value_set_boolean(value, plot->zy->title_visible);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
gtk_plot3d_real_paint (GtkWidget *widget)
{
  GtkPlot3D *plot;
  GtkPlotText *child_text;
  GtkPlotPC *pc;
  GList *datasets;
  GList *text;
  gint width, height;
  gint xoffset, yoffset;
  gint origin;
  gdouble pz;
  GtkPlotVector e[8], o, v[8];
  GtkPlotVector vx, vy, vz;
  gint i;

  if(!gtk_widget_get_visible(widget)) return;

  plot = GTK_PLOT3D(widget);

  xoffset = GTK_PLOT(plot)->internal_allocation.x;
  yoffset = GTK_PLOT(plot)->internal_allocation.y;
  width = GTK_PLOT(plot)->internal_allocation.width;
  height = GTK_PLOT(plot)->internal_allocation.height;

  /* pixmap = GTK_PLOT(plot)->drawable; */
  pc = GTK_PLOT(plot)->pc;

  gtk_plot_pc_gsave(pc);
  gtk_plot_pc_set_color(pc, &GTK_PLOT(plot)->background);

  if(!gtk_plot_is_transparent(GTK_PLOT(plot)))
    gtk_plot_pc_draw_rectangle (pc, TRUE,
  		        xoffset, yoffset,
		        width , height);

  /* draw frame to guide the eyes*/
/*
  gdk_draw_rectangle (pixmap, gtk_widget_get_style(widget)->black_gc, FALSE,
		      xoffset, yoffset,
		      width , height);
*/

  /* find origin */

  /* 8 vertices of the cube */

  e[0].x = 0;
  e[0].y = 0;
  e[0].z = 0;
  e[1].x = 1;
  e[1].y = 0;
  e[1].z = 0;
  e[2].x = 1;
  e[2].y = 1;
  e[2].z = 0;
  e[3].x = 0;
  e[3].y = 1;
  e[3].z = 0;
  e[4].x = 0;
  e[4].y = 0;
  e[4].z = 1;
  e[5].x = 1;
  e[5].y = 0;
  e[5].z = 1;
  e[6].x = 1;
  e[6].y = 1;
  e[6].z = 1;
  e[7].x = 0;
  e[7].y = 1;
  e[7].z = 1;

  for(i = 0; i < 8; i++){
    v[i].x = (1.0 - e[i].x) * plot->ax->ticks.min + e[i].x * plot->ax->ticks.max;
    v[i].y = (1.0 - e[i].y) * plot->ay->ticks.min + e[i].y * plot->ay->ticks.max;
    v[i].z = (1.0 - e[i].z) * plot->az->ticks.min + e[i].z * plot->az->ticks.max;
  }

  /* Origin for drawing the planes */

  origin = 0;
  o.x = 0.0;
  o.y = 0.0;
  o.z = 0.0;

  for(i = 1; i < 8; i++){
     pz = e[i].x * plot->e1.z + e[i].y * plot->e2.z + e[i].z * plot->e3.z ;

     if(pz > o.z){
       o.z = pz;
       origin = i;
     }
  }
 
  plot->origin = v[origin];

  plot->ax->direction.x = 1.0;
  plot->ax->direction.y = 0.0;
  plot->ax->direction.z = 0.0;
  plot->ay->direction.x = 0.0;
  plot->ay->direction.y = 1.0;
  plot->ay->direction.z = 0.0;
  plot->az->direction.x = 0.0;
  plot->az->direction.y = 0.0;
  plot->az->direction.z = 1.0;

  plot->ax->origin.x = 0.0;
  plot->ax->origin.y = v[origin].y;
  plot->ax->origin.z = v[origin].z;

  plot->ay->origin.y = 0.0;
  plot->ay->origin.x = v[origin].x;
  plot->ay->origin.z = v[origin].z;

  plot->az->origin.z = 0.0;
  plot->az->origin.x = v[origin].x;
  plot->az->origin.y = v[origin].y;

  /* Tick directions */

  vx.x = plot->e1.x * (1. - 2. * e[origin].x);
  vx.y = plot->e1.y * (1. - 2. * e[origin].x);
  vx.z = plot->e1.z * (1. - 2. * e[origin].x);
  vy.x = plot->e2.x * (1. - 2. * e[origin].y);
  vy.y = plot->e2.y * (1. - 2. * e[origin].y);
  vy.z = plot->e2.z * (1. - 2. * e[origin].y);
  vz.x = plot->e3.x * (1. - 2. * e[origin].z);
  vz.y = plot->e3.y * (1. - 2. * e[origin].z);
  vz.z = plot->e3.z * (1. - 2. * e[origin].z);

  /* draw planes, ticks & grid lines */

  gtk_plot_axis_ticks_recalc(plot->ax);
  gtk_plot_axis_ticks_recalc(plot->ay);
  gtk_plot_axis_ticks_recalc(plot->az);

  if(plot->xy_visible)
    {
      /* PLANES */

      if(origin == 0 || origin == 1 || origin == 2 || origin == 3)
        gtk_plot3d_draw_plane(plot, v[0], v[1], v[2], v[3], plot->color_xy);
      if(origin == 4 || origin == 5 || origin == 6 || origin == 7)
        gtk_plot3d_draw_plane(plot, v[4], v[5], v[6], v[7], plot->color_xy);

      /* X AXIS */

      plot->ax->major_mask = plot->xy->major_mask;
      plot->ax->minor_mask = plot->xy->minor_mask;
      plot->ax->label_mask = plot->xy->label_mask;
      plot->ax->title_visible = plot->xy->title_visible;

      o.x = 0.0;
      o.y = (1.0 - e[origin].y) * plot->ay->ticks.max + e[origin].y * plot->ay->ticks.min - plot->ax->origin.y;
      o.z = 0.0;

      gtk_plot3d_draw_grids(plot, plot->ax, o);
      gtk_plot3d_draw_axis(plot, plot->ax, vy, o);
      gtk_plot3d_draw_labels(plot, plot->ax, o); 

      /* Y AXIS */

      plot->ay->major_mask = plot->yx->major_mask;
      plot->ay->minor_mask = plot->yx->minor_mask;
      plot->ay->label_mask = plot->yx->label_mask;
      plot->ay->title_visible = plot->yx->title_visible;

      o.x = (1.0 - e[origin].x) * plot->ax->ticks.max + e[origin].x * plot->ax->ticks.min - plot->ay->origin.x;
      o.y = 0.0;
      o.z = 0.0;

      gtk_plot3d_draw_grids(plot, plot->ay, o);
      gtk_plot3d_draw_axis(plot, plot->ay, vx, o);
      gtk_plot3d_draw_labels(plot, plot->ay, o); 
    }

  if(plot->yz_visible)
    {
      /* PLANES */

      if(origin == 0 || origin == 3 || origin == 7 || origin == 4)
        gtk_plot3d_draw_plane(plot, v[0], v[3], v[7], v[4], plot->color_yz);
      if(origin == 1 || origin == 2 || origin == 6 || origin == 5)
        gtk_plot3d_draw_plane(plot, v[1], v[2], v[6], v[5], plot->color_yz);

      /* Y AXIS */

      plot->ay->major_mask = plot->yz->major_mask;
      plot->ay->minor_mask = plot->yz->minor_mask;
      plot->ay->label_mask = plot->yz->label_mask;
      plot->ay->title_visible = plot->yz->title_visible;

      o.x = 0.0;
      o.y = 0.0;
      o.z = (1.0 - e[origin].z) * plot->az->ticks.max + e[origin].z * plot->az->ticks.min - plot->ay->origin.z;

      gtk_plot3d_draw_grids(plot, plot->ay, o);
      gtk_plot3d_draw_axis(plot, plot->ay, vz, o);
      gtk_plot3d_draw_labels(plot, plot->ay, o); 

      /* Z AXIS */

      plot->az->major_mask = plot->zy->major_mask;
      plot->az->minor_mask = plot->zy->minor_mask;
      plot->az->label_mask = plot->zy->label_mask;
      plot->az->title_visible = plot->zy->title_visible;

      o.x = 0.0;
      o.y = (1.0 - e[origin].y) * plot->ay->ticks.max + e[origin].y * plot->ay->ticks.min - plot->az->origin.y;
      o.z = 0.0;

      gtk_plot3d_draw_grids(plot, plot->az, o);
      gtk_plot3d_draw_axis(plot, plot->az, vy, o);
      gtk_plot3d_draw_labels(plot, plot->az, o); 
    }

  if(plot->zx_visible)
    {
      /* PLANES */

      if(origin == 0 || origin == 4 || origin == 5 || origin == 1)
        gtk_plot3d_draw_plane(plot, v[0], v[4], v[5], v[1], plot->color_zx);
      if(origin == 3 || origin == 7 || origin == 6 || origin == 2)
        gtk_plot3d_draw_plane(plot, v[3], v[7], v[6], v[2], plot->color_zx);

      /* Z AXIS */

      plot->az->major_mask = plot->zx->major_mask;
      plot->az->minor_mask = plot->zx->minor_mask;
      plot->az->label_mask = plot->zx->label_mask;
      plot->az->title_visible = plot->zx->title_visible;

      o.x = (1.0 - e[origin].x) * plot->ax->ticks.max + e[origin].x * plot->ax->ticks.min - plot->az->origin.x;
      o.y = 0.0;
      o.z = 0.0;

      gtk_plot3d_draw_grids(plot, plot->az, o);
      gtk_plot3d_draw_axis(plot, plot->az, vx, o);
      gtk_plot3d_draw_labels(plot, plot->az, o); 

      /* X AXIS */

      plot->ax->major_mask = plot->xz->major_mask;
      plot->ax->minor_mask = plot->xz->minor_mask;
      plot->ax->label_mask = plot->xz->label_mask;
      plot->ax->title_visible = plot->xz->title_visible;

      o.x = 0.0;
      o.y = 0.0;
      o.z = (1.0 - e[origin].z) * plot->az->ticks.max + e[origin].z * plot->az->ticks.min - plot->ax->origin.z;

      gtk_plot3d_draw_grids(plot, plot->ax, o);
      gtk_plot3d_draw_axis(plot, plot->ax, vz, o);
      gtk_plot3d_draw_labels(plot, plot->ax, o); 
    }

  datasets = GTK_PLOT(plot)->data_sets;
  while(datasets)
   {
     GtkPlotData *dataset;

     dataset = GTK_PLOT_DATA(datasets->data);

     gtk_plot_data_paint(dataset);

     datasets = datasets->next;
   }

  if(plot->corner_visible){
    gdouble px0, py0, pz0;
    gdouble px, py, pz;
    gint corner;
    gint end[3];
    
    corner = origin + 2;
    if(corner > 3) corner -= 4;

    end[0] = corner;

    corner += 4;
    if(corner > 7) corner -= 8;

    end[1] = corner + 1;
    if(end[1] == 8 || end[1] == 4) end[1] -= 4;

    end[2] = corner - 1;
    if(end[2] == -1 || end[2] == 3) end[2] += 4;

    gtk_plot3d_get_pixel(plot, v[corner].x, v[corner].y, v[corner].z, 
                         &px0, &py0, &pz0);

    for (i = 0; i < 3; i++){
       gtk_plot3d_get_pixel(plot, v[end[i]].x, v[end[i]].y, v[end[i]].z, 
                            &px, &py, &pz);

       gtk_plot_draw_line(GTK_PLOT(plot), plot->corner, px0, py0, px, py);
    }

  }

  text = GTK_PLOT(plot)->text;
  while(text)
   {
     child_text = (GtkPlotText *) text->data;  
     gtk_plot_draw_text(GTK_PLOT(plot), *child_text);
     text = text->next;
   }

  GTK_PLOT_CLASS(GTK_WIDGET_GET_CLASS(GTK_WIDGET(plot)))->draw_legends(GTK_WIDGET(plot));

  gtk_plot_pc_grestore(pc);
}

/**
 * gtk_plot3d_new:
 * @drawable: a Gdk drawable.
 *
 *
 *
 * Return value:
 */
GtkWidget*
gtk_plot3d_new (cairo_surface_t *drawable)
{
  GtkWidget *plot;

  plot = gtk_widget_new (gtk_plot3d_get_type (), NULL);

  gtk_plot3d_construct(GTK_PLOT3D(plot), drawable);

  return plot;
}

/**
 * gtk_plot3d_construct:
 * @plot: a #GtkPlot3D widget.
 * @drawable: a Gdk drawable.
 *
 *
 */
void
gtk_plot3d_construct(GtkPlot3D *plot, cairo_surface_t *drawable)
{
  GTK_PLOT(plot)->drawable = drawable;
}

/**
 * gtk_plot3d_new_with_size:
 * @drawable: a Gdk drawable.
 * @width:
 * @height:
 *
 *
 *
 * Return value:
 */
GtkWidget*
gtk_plot3d_new_with_size (cairo_surface_t *drawable, gdouble width, gdouble height)
{
  GtkWidget *plot; 

  plot = gtk_widget_new (gtk_plot3d_get_type (), NULL);

  gtk_plot3d_construct_with_size(GTK_PLOT3D(plot), drawable, width, height);

  return(plot);
}

/**
 * gtk_plot3d_construct_with_size:
 * @plot: a #GtkPlot3D widget.
 * @drawable: a Gdk drawable.
 * @width:
 * @height:
 *
 *
 */
void
gtk_plot3d_construct_with_size(GtkPlot3D *plot, cairo_surface_t *drawable,
			       gdouble width, gdouble height)
{
  gtk_plot3d_construct(plot, drawable);

  gtk_plot_resize (GTK_PLOT(plot), width, height);
}

/**
 * gtk_plot3d_draw_plane:
 * @plot: a #GtkPlot3D widget.
 * @v1:
 * @v2:
 * @v3:
 * @v4:
 *
 *
 */
static void
gtk_plot3d_draw_plane(GtkPlot3D *plot, 
                      GtkPlotVector v1, 
                      GtkPlotVector v2, 
                      GtkPlotVector v3, 
                      GtkPlotVector v4, 
                      GdkRGBA background)
{
  GtkWidget *widget;
  GtkPlotPC *pc;
  GtkPlotVector v[4];
  GtkPlotPoint p[4];
  gdouble px, py, pz;
  gint i;

  widget = GTK_WIDGET(plot);
  if(!gtk_widget_get_visible(widget)) return;

  pc = GTK_PLOT(plot)->pc;

  gtk_plot_pc_set_color(pc, &background);

  v[0] = v1;
  v[1] = v2;
  v[2] = v3;
  v[3] = v4;
  for(i = 0; i < 4; i++){
    gtk_plot3d_get_pixel(plot, v[i].x, v[i].y, v[i].z, &px, &py, &pz);
    p[i].x = px;
    p[i].y = py;
  }

  gtk_plot_pc_draw_polygon(pc, TRUE, p, 4);
  gtk_plot_pc_set_color(pc, &plot->frame.color);

  gtk_plot_pc_set_lineattr(pc, 
                           plot->frame.line_width, 
                           plot->frame.line_style == GTK_PLOT_LINE_SOLID ? 0 : 1,
                           0, 0);

  if(plot->frame.line_style != GTK_PLOT_LINE_NONE)
    gtk_plot_pc_draw_polygon(pc, FALSE, p, 4);

}

static void
gtk_plot3d_draw_grids(GtkPlot3D *plot, GtkPlotAxis *axis, GtkPlotVector delta)
{
  gdouble xx;
  GtkPlotLine major_grid, minor_grid;
  gdouble x1, x2, y1, y2;
  gdouble oz;
  gint ntick;

  /* no values! */
  if (axis->ticks.values == NULL) return;

  major_grid = GTK_PLOT(plot)->left->major_grid;
  minor_grid = GTK_PLOT(plot)->left->minor_grid;
  switch(axis->orientation){
    case GTK_PLOT_AXIS_X:
      major_grid = GTK_PLOT(plot)->left->major_grid;
      minor_grid = GTK_PLOT(plot)->left->minor_grid;
      break;
    case GTK_PLOT_AXIS_Y:
      major_grid = GTK_PLOT(plot)->bottom->major_grid;
      minor_grid = GTK_PLOT(plot)->bottom->minor_grid;
      break;
    case GTK_PLOT_AXIS_Z:
      major_grid = GTK_PLOT(plot)->top->major_grid;
      minor_grid = GTK_PLOT(plot)->top->minor_grid;
      break;
  }

  if(axis->show_minor_grid)
    {
        for(ntick = 0; ntick < axis->ticks.nticks; ntick++){
          if(!axis->ticks.values[ntick].minor) continue;
          if(axis->ticks.values[ntick].value >= axis->ticks.min){
               xx = axis->ticks.values[ntick].value;
               gtk_plot3d_get_pixel(plot,
                             axis->origin.x + axis->direction.x * xx,
                             axis->origin.y + axis->direction.y * xx,
                             axis->origin.z + axis->direction.z * xx,
                             &x1, &y1, &oz);
               gtk_plot3d_get_pixel(plot,
                             axis->origin.x + axis->direction.x * xx + delta.x,
                             axis->origin.y + axis->direction.y * xx + delta.y,
                             axis->origin.z + axis->direction.z * xx + delta.z,
                             &x2, &y2, &oz);
               gtk_plot_draw_line(GTK_PLOT(plot),  
                                  minor_grid,
                                  x1, y1, x2, y2);
          }
        }
    }

  if(axis->show_major_grid)
    {
        for(ntick = 0; ntick < axis->ticks.nticks; ntick++){
          if(axis->ticks.values[ntick].minor) continue;
          if(axis->ticks.values[ntick].value > axis->ticks.min &&
             axis->ticks.values[ntick].value < axis->ticks.max){
               xx = axis->ticks.values[ntick].value;
               gtk_plot3d_get_pixel(plot,
                             axis->origin.x + axis->direction.x * xx,
                             axis->origin.y + axis->direction.y * xx,
                             axis->origin.z + axis->direction.z * xx,
                             &x1, &y1, &oz);
               gtk_plot3d_get_pixel(plot,
                             axis->origin.x + axis->direction.x * xx + delta.x,
                             axis->origin.y + axis->direction.y * xx + delta.y,
                             axis->origin.z + axis->direction.z * xx + delta.z,
                             &x2, &y2, &oz);
               gtk_plot_draw_line(GTK_PLOT(plot),  
                                  major_grid,
                                  x1, y1, x2, y2);
          }
        }
    }

}

static void
gtk_plot3d_draw_axis(GtkPlot3D *plot, 
                     GtkPlotAxis *axis, 
                     GtkPlotVector tick,
                     GtkPlotVector delta)
{
  GtkPlotPC *pc;
  gdouble xx;
  gint line_width;
  gint ntick;
  gdouble m;
  gdouble oz;
  gdouble x1, x2, y1, y2;
  gint ticks_length;

  if (axis->ticks.values == NULL)
    return;

  pc = GTK_PLOT(plot)->pc;

  m = GTK_PLOT(plot)->magnification;

  line_width = plot->frame.line_width;
  gtk_plot_pc_set_color(pc, &plot->frame.color);
  gtk_plot_pc_set_lineattr(pc, line_width, 0, 3, 0);

/*
  gtk_plot_pc_draw_line(pc, x1, y1, x2, y2);
*/

  gtk_plot_pc_set_lineattr(pc, axis->ticks_width, 0, 1, 0);

  for(ntick = 0; ntick < axis->ticks.nticks; ntick++){
    xx = axis->ticks.values[ntick].value;
    if(!axis->ticks.values[ntick].minor){
         ticks_length = axis->ticks_length;
         gtk_plot3d_get_pixel(plot, 
                        axis->origin.x + axis->direction.x * xx + delta.x,
                        axis->origin.y + axis->direction.y * xx + delta.y,
                        axis->origin.z + axis->direction.z * xx + delta.z,
                        &x1, &y1, &oz);
         if(xx >= axis->ticks.min){
           x2 = x1 + m * ticks_length * tick.x;
           y2 = y1 + m * ticks_length * tick.y;
           if(axis->major_mask == GTK_PLOT_TICKS_OUT)
              gtk_plot_pc_draw_line(pc, x1, y1, x2, y2);
           x2 = x1 - m * axis->ticks_length * tick.x;
           y2 = y1 - m * axis->ticks_length * tick.y;
           if(axis->major_mask == GTK_PLOT_TICKS_IN)
              gtk_plot_pc_draw_line(pc, x1, y1, x2, y2);
         }
    } else {
         ticks_length = axis->ticks_length / 2.;
         gtk_plot3d_get_pixel(plot, 
                        axis->origin.x + axis->direction.x * xx + delta.x,
                        axis->origin.y + axis->direction.y * xx + delta.y,
                        axis->origin.z + axis->direction.z * xx + delta.z,
                        &x1, &y1, &oz);
         if(xx >= axis->ticks.min){
           x2 = x1 + m * ticks_length * tick.x;
           y2 = y1 + m * ticks_length * tick.y;
           if(axis->minor_mask == GTK_PLOT_TICKS_OUT)
              gtk_plot_pc_draw_line(pc, x1, y1, x2, y2);
             
           x2 = x1 - m * axis->ticks_length * tick.x;
           y2 = y1 - m * axis->ticks_length * tick.y;
           if(axis->minor_mask == GTK_PLOT_TICKS_IN) 
              gtk_plot_pc_draw_line(pc, x1, y1, x2, y2);
         }
    }
  }     

}


static void
gtk_plot3d_draw_labels(GtkPlot3D *plot, 
                       GtkPlotAxis *axis,
                       GtkPlotVector delta)
{
  GtkWidget *widget;
  GtkPlotPC *pc;
  GtkPlotText title, tick;
  gchar label[LABEL_MAX_LENGTH];
  gdouble tick_value;
  gdouble xx;
  gint text_height, text_width, ascent, descent;
  gint ntick;
  gdouble m;
  gdouble ox, oy, oz;
  gdouble sx, sy, sz;
  gdouble y = 0;
  GtkPlotVector ticks_direction, center;
  gboolean veto = FALSE;
  GtkAllocation allocation;

  widget = GTK_WIDGET(plot); 
  pc = GTK_PLOT(plot)->pc;

  m = GTK_PLOT(plot)->magnification;

  gtk_plot_pc_set_color (pc, &axis->labels_attr.fg);

  gtk_plot_text_get_size("0", 0, axis->labels_attr.font, roundint(axis->labels_attr.height * m), &text_width, &text_height, &ascent, &descent); 

  switch(axis->labels_attr.angle){
    case 0:
           y += text_height / 2.;
           break;
    case 90:
           break;
    case 180:
           y -= text_height / 2.;
           break;
    case 270:
           break;
  }

  tick = axis->labels_attr;

  center.x = (plot->ax->ticks.max + plot->ax->ticks.min) / 2.0;
  center.y = (plot->ay->ticks.max + plot->ay->ticks.min) / 2.0;
  center.z = (plot->az->ticks.max + plot->az->ticks.min) / 2.0;

  gtk_plot3d_get_pixel(plot, 
		       axis->origin.x + delta.x + center.x * axis->direction.x,
		       axis->origin.y + delta.y + center.y * axis->direction.y,
		       axis->origin.z + delta.z + center.z * axis->direction.z,
		       &ox, &oy, &oz);
  gtk_plot3d_get_pixel(plot, 
		       center.x,
		       center.y,
		       center.z,
		       &sx, &sy, &sz);

  ticks_direction.x = (ox-sx)/sqrt((ox-sx)*(ox-sx) + (oy-sy)*(oy-sy));
  ticks_direction.y = (oy-sy)/sqrt((ox-sx)*(ox-sx) + (oy-sy)*(oy-sy));

  for(ntick = 0;
      axis->ticks.values != NULL &&
      ntick < axis->ticks.nticks; ntick++){
    if(axis->ticks.values[ntick].minor) continue;
    xx = axis->ticks.values[ntick].value;
    gtk_plot3d_get_pixel(plot, 
                         axis->origin.x + axis->direction.x * xx + delta.x,
                         axis->origin.y + axis->direction.y * xx + delta.y,
                         axis->origin.z + axis->direction.z * xx + delta.z,
                         &ox, &oy, &oz);
   
    tick.x = ox + m * axis->labels_offset * ticks_direction.x;
    tick.y = oy + m * axis->labels_offset * ticks_direction.y;

    tick_value = axis->ticks.values[ntick].value;

    if(tick_value >= axis->ticks.min-1.e-9){
      if(!axis->custom_labels){
        gtk_plot_axis_parse_label(axis, tick_value, axis->label_precision, axis->label_style, label);
      }
      else
      {
        g_signal_emit_by_name(axis, "tick_label",
                                &tick_value, label, &veto);
        if(!veto)
          gtk_plot_axis_parse_label(axis, tick_value, axis->label_precision, axis->label_style, label);
      }
      tick.text = label;

      gtk_widget_get_allocation(widget, &allocation);
      if(axis->label_mask == GTK_PLOT_LABEL_OUT){
         tick.x = (gdouble)tick.x / (gdouble)allocation.width;
         tick.y = (gdouble)tick.y / (gdouble)allocation.height;
         gtk_plot_draw_text(GTK_PLOT(plot), tick);
      }
    }
  }

  if(axis->title_visible && axis->title.text)
       {
         title = axis->title;

         gtk_plot3d_get_pixel(plot,
                       axis->origin.x + center.x * axis->direction.x + delta.x, 
                       axis->origin.y + center.y * axis->direction.y + delta.y, 
                       axis->origin.z + center.z * axis->direction.z + delta.z, 
                       &ox, &oy, &oz);
    
         title.x = ox + m * plot->titles_offset * ticks_direction.x; 
         title.y = oy + m * plot->titles_offset * ticks_direction.y; 

         title.x = title.x / (gdouble)allocation.width;
         title.y = title.y / (gdouble)allocation.height;

         gtk_plot_draw_text(GTK_PLOT(plot), title); 
       }
}

/*
gdouble 
get_clean_tick_size(gdouble delta) 
{
  gint magnitude;
  delta/= 5.0; 
  if (delta<0.0) delta*= -1.0;
  magnitude= (gint)(floor(log10(delta)));
  return ceil(delta/pow(10,magnitude))*pow(10,magnitude);
}
*/


/******************************************
 ******************************************/

/**
 * gtk_plot3d_autoscale:
 * @plot: a #GtkPlot3D widget.
 *
 *
 */
void
gtk_plot3d_autoscale(GtkPlot3D *plot)
{
  GList *datasets;
  gdouble xmin, xmax, ymin, ymax, zmin, zmax;
  gint px, py, pz;
  gint first=1;

  if(!GTK_PLOT(plot)->data_sets) return;

  xmin = GTK_PLOT(plot)->bottom->ticks.max;
  xmax = GTK_PLOT(plot)->bottom->ticks.min;
  ymin = GTK_PLOT(plot)->left->ticks.max;
  ymax = GTK_PLOT(plot)->left->ticks.min;
  zmin = GTK_PLOT(plot)->top->ticks.max;
  zmax = GTK_PLOT(plot)->top->ticks.min;

  datasets = GTK_PLOT(plot)->data_sets;

  while(datasets){
    GtkPlotData *data;
    gint i;

    data = GTK_PLOT_DATA(datasets->data);

    if(!data->is_function){
      if(GTK_IS_PLOT_SURFACE(data))
        if(GTK_PLOT_SURFACE(data)->use_amplitud)
          gtk_plot_data_gradient_autoscale_a(data);
        else
          gtk_plot_data_gradient_autoscale_z(data);
      else
        gtk_plot_data_gradient_autoscale_a(data);

      for (i = 0; i < data->num_points; i++){
        gdouble x, y, z, a, dx, dy, dz, da;
        gchar *label;
        gboolean error;

        gtk_plot_data_get_point(data, i, 
                       &x, &y, &z, &a, &dx, &dy, &dz, &da, &label, &error);
	if (first) { 
	  first = 0; xmin = xmax = x; ymin = ymax = y; zmin = zmax = z; 
	} else {
          xmin = MIN(xmin, x); 
	  xmax = MAX(xmax, x); 
	  ymin = MIN(ymin, y); 
	  ymax = MAX(ymax, y); 
	  zmin = MIN(zmin, z); 
	  zmax = MAX(zmax, z); 
	}
      }
    }

    datasets = datasets->next;
  }

  gtk_plot_axis_ticks_autoscale(plot->ax, xmin, xmax, &px);
  gtk_plot_axis_ticks_autoscale(plot->ay, ymin, ymax, &py);
  gtk_plot_axis_ticks_autoscale(plot->az, zmin, zmax, &pz);

  plot->ax->label_precision = px;
  plot->ay->label_precision = py;
  plot->az->label_precision = pz;

  GTK_PLOT(plot)->xmin = plot->ax->ticks.min; 
  GTK_PLOT(plot)->xmax = plot->ax->ticks.max; 
  GTK_PLOT(plot)->ymin = plot->ay->ticks.min; 
  GTK_PLOT(plot)->ymax = plot->ay->ticks.max; 
  plot->zmin = plot->az->ticks.min; 
  plot->zmax = plot->az->ticks.max; 

  g_signal_emit_by_name(plot, "update", TRUE);
  g_signal_emit_by_name(plot, "changed");
}


/**
 * gtk_plot3d_rotate:
 * @plot: a #GtkPlot3D widget.
 * @angle_x:
 * @angle_y:
 * @angle_z:
 *
 *
 */
void
gtk_plot3d_rotate(GtkPlot3D *plot, gdouble angle_x, gdouble angle_y, gdouble angle_z)
{
  gtk_plot3d_rotate_vector(plot, &plot->e1, angle_x, angle_y, angle_z);
  gtk_plot3d_rotate_vector(plot, &plot->e2, angle_x, angle_y, angle_z);
  gtk_plot3d_rotate_vector(plot, &plot->e3, angle_x, angle_y, angle_z);

  g_signal_emit_by_name(plot, "update", FALSE);
  g_signal_emit_by_name(plot, "changed");
}

/**
 * gtk_plot3d_rotate_vector:
 * @plot: a #GtkPlot3D widget.
 * @a1:
 * @a2:
 * @a3:
 *
 *
 */
void
gtk_plot3d_rotate_vector(GtkPlot3D *plot, 
                          GtkPlotVector *vector,
                          gdouble a1, gdouble a2, gdouble a3)
{
  GtkPlotVector v;
  gdouble cos1, sin1;
  gdouble cos2, sin2;
  gdouble cos3, sin3;

  if(a1 < 0) a1 = 360 + a1;
  if(a2 < 0) a2 = 360 + a2;
  if(a3 < 0) a3 = 360 + a3;

  cos1 = plot->ncos[(int)a1%360];
  sin1 = plot->nsin[(int)a1%360];
  cos2 = plot->ncos[(int)a2%360];
  sin2 = plot->nsin[(int)a2%360];
  cos3 = plot->ncos[(int)a3%360];
  sin3 = plot->nsin[(int)a3%360];

  v.y = vector->y*cos1 - vector->z*sin1;
  v.z = vector->z*cos1 + vector->y*sin1;

  vector->y = v.y;
  vector->z = v.z;

  v.z = vector->z*cos2 - vector->x*sin2;
  v.x = vector->x*cos2 + vector->z*sin2;

  vector->x = v.x;
  vector->z = v.z;

  v.x = vector->x*cos3 - vector->y*sin3;
  v.y = vector->y*cos3 + vector->x*sin3;

  vector->x = v.x;
  vector->y = v.y;
}

/**
 * gtk_plot3d_get_pixel:
 * @plot: a #GtkPlot3D widget.
 * @x:
 * @y:
 * @z:
 * @px:
 * @py:
 * @pz:
 *
 *
 */
void
gtk_plot3d_get_pixel(GtkPlot3D *plot, 
                     gdouble x, gdouble y, gdouble z,
                     gdouble *px, gdouble *py, gdouble *pz)
{
  GTK_PLOT3D_CLASS(GTK_WIDGET_GET_CLASS(GTK_WIDGET(plot)))->get_pixel(GTK_WIDGET(plot), x, y, z, px, py, pz);
}
 
static void
gtk_plot3d_real_get_pixel(GtkWidget *widget,
                          gdouble x, gdouble y, gdouble z,
                          gdouble *px, gdouble *py, gdouble *pz)
{
  GtkPlot3D *plot;
  GtkPlotVector e1, e2, e3, center;
  gint xp, yp, width, height, size;
  gdouble rx, ry, rz;
  gdouble cx, cy, cz;

  plot = GTK_PLOT3D(widget);
  xp = GTK_PLOT(plot)->internal_allocation.x;
  yp = GTK_PLOT(plot)->internal_allocation.y;
  width = GTK_PLOT(plot)->internal_allocation.width;
  height =GTK_PLOT(plot)->internal_allocation.height;

  size = MIN(width, height) / SQRT2;
  e1 = plot->e1;
  e2 = plot->e2;
  e3 = plot->e3;

  ry = gtk_plot_axis_ticks_transform(plot->ay, y);
  rx = gtk_plot_axis_ticks_transform(plot->ax, x);
  rz = gtk_plot_axis_ticks_transform(plot->az, z);

  center = plot->center;

  cx = -(center.x * e1.x + center.y * e2.x + center.z * e3.x);
  cy = -(center.x * e1.y + center.y * e2.y + center.z * e3.y);
  cz = -(center.x * e1.z + center.y * e2.z + center.z * e3.z);

  *px = xp + width / 2.;
  *py = yp + height / 2.;
  *pz = 0.0;

  *px += (cx + rx * e1.x + ry * e2.x + rz * e3.x) * size;
  *py += (cy + rx * e1.y + ry * e2.y + rz * e3.y) * size;
  *pz += (cz + rx * e1.z + ry * e2.z + rz * e3.z) * size;
}

/***********************************************************/


/***********************************************************
 * gtk_plot3d_set_xrange
 * gtk_plot3d_set_yrange
 * gtk_plot3d_set_zrange
 * gtk_plot3d_set_xfactor
 * gtk_plot3d_set_yfactor
 * gtk_plot3d_set_zfactor
 * gtk_plot3d_get_xfactor
 * gtk_plot3d_get_yfactor
 * gtk_plot3d_get_zfactor
 ***********************************************************/

/**
 * gtk_plot3d_set_xrange:
 * @plot: a #GtkPlot3D widget.
 * @min:
 * @max:
 *
 *
 */
void
gtk_plot3d_set_xrange(GtkPlot3D *plot, gdouble min, gdouble max)
{
  if(min > max) return;

  GTK_PLOT(plot)->xmin = min;
  GTK_PLOT(plot)->xmax = max;
  plot->ax->ticks.min = min;
  plot->ax->ticks.max = max;
  gtk_plot_axis_ticks_recalc(plot->ax);

  g_signal_emit_by_name(plot, "update", TRUE);
  g_signal_emit_by_name(plot, "changed");
}

/**
 * gtk_plot3d_set_yrange:
 * @plot: a #GtkPlot3D widget.
 * @min:
 * @max:
 *
 *
 */

void
gtk_plot3d_set_yrange(GtkPlot3D *plot, gdouble min, gdouble max)
{
  if(min > max) return;

  GTK_PLOT(plot)->ymin = min;
  GTK_PLOT(plot)->ymax = max;
  plot->ay->ticks.min = min;
  plot->ay->ticks.max = max;
  gtk_plot_axis_ticks_recalc(plot->ay);

  g_signal_emit_by_name(plot, "update", TRUE);
  g_signal_emit_by_name(plot, "changed");
}
/**
 * gtk_plot3d_set_zrange:
 * @plot: a #GtkPlot3D widget.
 * @min:
 * @max:
 *
 *
 */
void
gtk_plot3d_set_zrange(GtkPlot3D *plot, gdouble min, gdouble max)
{
  if(min > max) return;

  plot->zmin = min;
  plot->zmax = max;
  plot->az->ticks.min = min;
  plot->az->ticks.max = max;
  gtk_plot_axis_ticks_recalc(plot->az);

  g_signal_emit_by_name(plot, "update", TRUE);
  g_signal_emit_by_name(plot, "changed");
}

/**
 * gtk_plot3d_set_xfactor:
 * @plot: a #GtkPlot3D widget.
 * @xfactor:
 *
 *
 */
void
gtk_plot3d_set_xfactor(GtkPlot3D *plot, gdouble xfactor)
{
  if(xfactor <= 0.0) return;

  plot->e1.x /= plot->xfactor;
  plot->e1.y /= plot->xfactor;
  plot->e1.z /= plot->xfactor;

  plot->xfactor = xfactor;
  
  plot->e1.x *= plot->xfactor;
  plot->e1.y *= plot->xfactor;
  plot->e1.z *= plot->xfactor;

  plot->ax->direction = plot->e1;

  g_signal_emit_by_name(plot, "update", FALSE);
  g_signal_emit_by_name(plot, "changed");
}

/**
 * gtk_plot3d_set_yfactor:
 * @plot: a #GtkPlot3D widget.
 * @yfactor:
 *
 *
 */
void
gtk_plot3d_set_yfactor(GtkPlot3D *plot, gdouble yfactor)
{
  if(yfactor <= 0.0) return;

  plot->e2.x /= plot->yfactor;
  plot->e2.y /= plot->yfactor;
  plot->e2.z /= plot->yfactor;

  plot->yfactor = yfactor;
  
  plot->e2.x *= plot->yfactor;
  plot->e2.y *= plot->yfactor;
  plot->e2.z *= plot->yfactor;

  plot->ay->direction = plot->e1;

  g_signal_emit_by_name(plot, "update", FALSE);
  g_signal_emit_by_name(plot, "changed");
}

/**
 * gtk_plot3d_set_zfactor:
 * @plot: a #GtkPlot3D widget.
 * @zfactor:
 *
 *
 */
void
gtk_plot3d_set_zfactor(GtkPlot3D *plot, gdouble zfactor)
{
  if(zfactor <= 0.0) return;

  plot->e3.x /= plot->zfactor;
  plot->e3.y /= plot->zfactor;
  plot->e3.z /= plot->zfactor;

  plot->zfactor = zfactor;
  
  plot->e3.x *= plot->zfactor;
  plot->e3.y *= plot->zfactor;
  plot->e3.z *= plot->zfactor;

  plot->az->direction = plot->e1;

  g_signal_emit_by_name(plot, "update", FALSE);
  g_signal_emit_by_name(plot, "changed");
}

/**
 * gtk_plot3d_get_xfactor:
 * @plot: a #GtkPlot3D widget.
 *
 *
 *
 * Return value:
 */
gdouble
gtk_plot3d_get_xfactor(GtkPlot3D *plot)
{
  return (plot->xfactor);
}

/**
 * gtk_plot3d_get_yfactor:
 * @plot: a #GtkPlot3D widget.
 *
 *
 *
 * Return value:
 */
gdouble
gtk_plot3d_get_yfactor(GtkPlot3D *plot)
{
  return (plot->yfactor);
}

/**
 * gtk_plot3d_get_zfactor:
 * @plot: a #GtkPlot3D widget.
 *
 *
 *
 * Return value:
 */
gdouble
gtk_plot3d_get_zfactor(GtkPlot3D *plot)
{
  return (plot->zfactor);
}


/***********************************************************
 * gtk_plot3d_plane_set_color
 * gtk_plot3d_plane_set_visible
 * gtk_plot3d_plane_visible
 ***********************************************************/

/**
 * gtk_plot3d_plane_set_color:
 * @plane:
 * @color:
 *
 *
 */
void            
gtk_plot3d_plane_set_color      (GtkPlot3D *plot,
                                 GtkPlotPlane plane,
                                 const GdkRGBA *color)
{

  switch(plane){
    case GTK_PLOT_PLANE_XY:
      plot->color_xy = *color;
      break;
    case GTK_PLOT_PLANE_YZ:
      plot->color_yz = *color;
      break;
    case GTK_PLOT_PLANE_ZX:
      plot->color_zx = *color;
      break;
    default:
      break;
  }    

}

/**
 * gtk_plot3d_plane_set_visible:
 * @plane:
 * @visible:
 *
 *
 */
void            
gtk_plot3d_plane_set_visible    (GtkPlot3D *plot,
                                 GtkPlotPlane plane,
                                 gboolean visible)
{

  switch(plane){
    case GTK_PLOT_PLANE_XY:
      plot->xy_visible = visible;
      break;
    case GTK_PLOT_PLANE_YZ:
      plot->yz_visible = visible;
      break;
    case GTK_PLOT_PLANE_ZX:
      plot->zx_visible = visible;
      break;
    default:
      break;
  }    

}

/**
 * gtk_plot3d_plane_visible:
 * @plot: a #GtkPlot3D widget.
 * @plane:
 *
 *
 *
 * Return value:
 */
gboolean        
gtk_plot3d_plane_visible    (GtkPlot3D *plot,
                                 GtkPlotPlane plane)
{
  gboolean visible = FALSE;

  switch(plane){
    case GTK_PLOT_PLANE_XY:
      visible = plot->xy_visible;
      break;
    case GTK_PLOT_PLANE_YZ:
      visible = plot->yz_visible;
      break;
    case GTK_PLOT_PLANE_ZX:
      visible = plot->zx_visible;
      break;
    default:
      break;
  }    

  return visible;
}


/***********************************************************
 * gtk_plot3d_corner_set_visible     
 * gtk_plot3d_corner_visible     
 * gtk_plot3d_corner_set_attributes     
 * gtk_plot3d_corner_get_attributes     
 * gtk_plot3d_frame_set_attributes     
 * gtk_plot3d_frame_get_attributes     
 * gtk_plot3d_show_labels     
 * gtk_plot3d_show_title     
 * gtk_plot3d_hide_title     
 * gtk_plot3d_show_major_ticks 
 * gtk_plot3d_show_minor_ticks 
 * gtk_plot3d_set_ticks 
 * gtk_plot3d_set_major_ticks 
 * gtk_plot3d_set_minor_ticks 
 * gtk_plot3d_set_ticks_length
 * gtk_plot3d_set_ticks_width
 * gtk_plot3d_show_ticks
 * gtk_plot3d_set_titles_offset
 * gtk_plot3d_get_titles_offset
 ***********************************************************/


/**
 * gtk_plot3d_corner_set_visible:
 * @plot: a #GtkPlot3D widget.
 * @visible:
 *
 *
 */
void
gtk_plot3d_corner_set_visible(GtkPlot3D *plot, gboolean visible)
{
  plot->corner_visible = visible;
}

/**
 * gtk_plot3d_corner_visible:
 * @plot: a #GtkPlot3D widget.
 *
 *
 *
 * Return value:
 */
gboolean
gtk_plot3d_corner_visible(GtkPlot3D *plot)
{
  return (plot->corner_visible);
}

/**
 * gtk_plot3d_corner_set_attributes:
 * @plot: a #GtkPlot3D widget.
 * @style:
 * @width:
 * @color:
 *
 *
 */
void            
gtk_plot3d_corner_set_attributes   (GtkPlot3D *plot,
                                    GtkPlotLineStyle style,
                                    gfloat width,
                                    const GdkRGBA *color)
{
  plot->corner.line_style = style;
  plot->corner.line_width = width;
  if(color) plot->corner.color = *color;
}

/**
 * gtk_plot3d_corner_get_attribute:
 * @plot: a #GtkPlot3D widget.
 * @style:
 * @width:
 * @color:
 *
 *
 */
void            
gtk_plot3d_corner_get_attributes   (GtkPlot3D *plot,
                                    GtkPlotLineStyle *style,
                                    gfloat *width,
                                    GdkRGBA *color)
{
  *style = plot->corner.line_style;
  *width = plot->corner.line_width;
  *color = plot->corner.color;
}

/**
 * gtk_plot3d_frame_set_attributes:
 * @plot: a #GtkPlot3D widget.
 * @style:
 * @width:
 * @color:
 *
 *
 */
void            
gtk_plot3d_frame_set_attributes   (GtkPlot3D *plot,
                                   GtkPlotLineStyle style,
                                   gfloat width,
                                   const GdkRGBA *color)
{
  plot->frame.line_style = style;
  plot->frame.line_width = width;
  if(color) plot->frame.color = *color;
}

/**
 * gtk_plot3d_frame_get_attributes:
 * @plot: a #GtkPlot3D widget.
 * @style:
 * @width:
 * @color:
 *
 *
 */
void            
gtk_plot3d_frame_get_attributes   (GtkPlot3D *plot,
                                   GtkPlotLineStyle *style,
                                   gfloat *width,
                                   GdkRGBA *color)
{
  *style = plot->frame.line_style;
  *width = plot->frame.line_width;
  *color = plot->frame.color;
}

/**
 * gtk_plot3d_get_axis:
 * @plot: a #GtkPlot3D widget.
 * @orientation: of axis to be returned
 *
 * Return value: (transfer none) the #GtkPlotAxis with given 
 * orientation 
 */
GtkPlotAxis *
gtk_plot3d_get_axis(GtkPlot3D *plot, GtkPlotOrientation orientation)
{
  GtkPlotAxis *axis = NULL;

  switch(orientation){
    case GTK_PLOT_AXIS_X:
      axis = plot->ax;
      break;
    case GTK_PLOT_AXIS_Y:
      axis = plot->ay;
      break;
    case GTK_PLOT_AXIS_Z:
      axis = plot->az;
      break;
    default:
      axis = NULL;
      break;
  }

  return axis;
} 

/**
 * gtk_plot3d_get_side:
 * @plot: a #GtkPlot3D widget.
 * @side: #GtkPlotSide of the wanted axis
 *
 * Return value: (transfer none): the #GtkPlotAxis on the given 
 * side 
 */
GtkPlotAxis *
gtk_plot3d_get_side(GtkPlot3D *plot, GtkPlotSide side)
{
  GtkPlotAxis *axis = NULL;

  switch(side){
    case GTK_PLOT_SIDE_XY:
      axis = plot->xy;
      break;
    case GTK_PLOT_SIDE_XZ:
      axis = plot->xz;
      break;
    case GTK_PLOT_SIDE_YX:
      axis = plot->yx;
      break;
    case GTK_PLOT_SIDE_YZ:
      axis = plot->yz;
      break;
    case GTK_PLOT_SIDE_ZX:
      axis = plot->zx;
      break;
    case GTK_PLOT_SIDE_ZY:
      axis = plot->zy;
      break;
    default:
      axis = NULL;
  }

  return axis;
}

/**
 * gtk_plot3d_show_labels:
 * @plot: a #GtkPlot3D widget.
 * @side:
 * @label_mask:
 *
 *
 */
void            
gtk_plot3d_show_labels     (GtkPlot3D *plot,
			    GtkPlotSide side,
                            gint label_mask)
{
  GtkPlotAxis *axis;

  axis = gtk_plot3d_get_side(plot, side);

  axis->label_mask = label_mask;
}

/**
 * gtk_plot3d_show_title:
 * @plot: a #GtkPlot3D widget.
 * @side:
 *
 *
 */
void            
gtk_plot3d_show_title     (GtkPlot3D *plot,
			   GtkPlotSide side)
{
  GtkPlotAxis *axis;

  axis = gtk_plot3d_get_side(plot, side);

  axis->title_visible = TRUE;
}

/**
 * gtk_plot3d_hide_title:
 * @plot: a #GtkPlot3D widget.
 * @side:
 *
 *
 */
void            
gtk_plot3d_hide_title     (GtkPlot3D *plot,
			   GtkPlotSide side)
{
  GtkPlotAxis *axis;

  axis = gtk_plot3d_get_side(plot, side);

  axis->title_visible = FALSE;
}


/**
 * gtk_plot3d_show_major_ticks:
 * @plot: a #GtkPlot3D widget.
 * @side:
 * @ticks_mask:
 *
 *
 */
void            
gtk_plot3d_show_major_ticks (GtkPlot3D *plot,
			     GtkPlotSide side,
                             gint ticks_mask)
{
  GtkPlotAxis *axis;

  axis = gtk_plot3d_get_side(plot, side);

  axis->major_mask = ticks_mask;
}

/**
 * gtk_plot3d_show_minor_ticks:
 * @plot: a #GtkPlot3D widget.
 * @side:
 * @ticks_mask:
 *
 *
 */
void            
gtk_plot3d_show_minor_ticks(GtkPlot3D *plot,
			    GtkPlotSide side,
                            gint ticks_mask)
{
  GtkPlotAxis *axis;

  axis = gtk_plot3d_get_side(plot, side);

  axis->minor_mask = ticks_mask;
}

/**
 * gtk_plot3d_set_ticks:
 * @plot: a #GtkPlot3D widget.
 * @direction:
 * @major_step:
 * @nminor:
 *
 *
 */
void
gtk_plot3d_set_ticks        (GtkPlot3D *plot,
			     GtkPlotOrientation direction,
                             gdouble major_step,
                             gint nminor)
{
  GtkPlotAxis *axis;

  axis = gtk_plot3d_get_axis(plot, direction);

  axis->ticks.step = major_step;

  axis->ticks.nminor = nminor;
}

/**
 * gtk_plot3d_set_major_ticks:
 * @plot: a #GtkPlot3D widget.
 * @direction:
 * @major_step:
 *
 *
 */
void
gtk_plot3d_set_major_ticks  (GtkPlot3D *plot,
                             GtkPlotOrientation direction,
                             gdouble major_step)
{
  GtkPlotAxis *axis;

  axis = gtk_plot3d_get_axis(plot, direction);

  axis->ticks.step = major_step;
}

/**
 * gtk_plot3d_set_minor_ticks:
 * @plot: a #GtkPlot3D widget.
 * @direction:
 * @nminor:
 *
 *
 */
void
gtk_plot3d_set_minor_ticks  (GtkPlot3D *plot,
                             GtkPlotOrientation direction,
                             gint nminor)
{
  GtkPlotAxis *axis;

  axis = gtk_plot3d_get_axis(plot, direction);

  axis->ticks.nminor = nminor;
}

/**
 * gtk_plot3d_set_ticks_length:
 * @plot: a #GtkPlot3D widget.
 * @direction:
 * @length:
 *
 *
 */
void            
gtk_plot3d_set_ticks_length  (GtkPlot3D *plot,
                              GtkPlotOrientation direction,
                              gint length)
{
  GtkPlotAxis *axis;

  axis = gtk_plot3d_get_axis(plot, direction);

  axis->ticks_length = length;
}

/**
 * gtk_plot3d_set_ticks_width:
 * @plot: a #GtkPlot3D widget.
 * @direction:
 * @width:
 *
 *
 */
void            
gtk_plot3d_set_ticks_width   (GtkPlot3D *plot,
                              GtkPlotOrientation direction,
                              gfloat width)
{
  GtkPlotAxis *axis;

  axis = gtk_plot3d_get_axis(plot, direction);

  axis->ticks_width = width;
}

/**
 * gtk_plot3d_show_ticks:
 * @plot: a #GtkPlot3D widget.
 * @side:
 * @major_mask:
 * @minor_mask:
 *
 *
 */
void            
gtk_plot3d_show_ticks        (GtkPlot3D *plot,
                              GtkPlotSide side,
                              gint major_mask,
                              gint minor_mask)
{
  GtkPlotAxis *axis;

  axis = gtk_plot3d_get_side(plot, side);

  axis->major_mask = major_mask;
  axis->minor_mask = minor_mask;
}

/**
 * gtk_plot3d_set_titles_offset:
 * @plot: a #GtkPlot3D widget.
 * @offset
 *
 *
 */
void
gtk_plot3d_set_titles_offset    (GtkPlot3D *plot,
                                 gint offset)
{
  plot->titles_offset = offset;
}

/**
 * gtk_plot3d_get_titles_offset:
 * @plot: a #GtkPlot3D widget.
 *
 *
 *
 * Return value:
 */
gint            
gtk_plot3d_get_titles_offset    (GtkPlot3D *plot)
{
  return (plot->titles_offset);
}

/***********************************************************
 * gtk_plot3d_major_grids_set_visible     
 * gtk_plot3d_major_grids_visible     
 * gtk_plot3d_minor_grids_set_visible     
 * gtk_plot3d_minor_grids_visible     
 * gtk_plot3d_major_grids_set_attributes     
 * gtk_plot3d_minor_grids_set_attributes     
 * gtk_plot3d_major_grids_get_attributes     
 * gtk_plot3d_minor_grids_get_attributes     
 ***********************************************************/

/**
 * gtk_plot3d_major_grids_set_visible:
 * @plot: a #GtkPlot3D widget.
 * @x:
 * @y:
 * @z:
 *
 *
 */
void            
gtk_plot3d_major_grids_set_visible    (GtkPlot3D *plot,
                                       gboolean x,
                                       gboolean y,
                                       gboolean z)
{
  plot->ax->show_major_grid = x;
  plot->ay->show_major_grid = y;
  plot->az->show_major_grid = z;
}

/**
 * gtk_plot3d_minor_grids_set_visible:
 * @plot: a #GtkPlot3D widget.
 * @x:
 * @y:
 * @z:
 *
 *
 */
void            
gtk_plot3d_minor_grids_set_visible    (GtkPlot3D *plot,
                                       gboolean x,
                                       gboolean y,
                                       gboolean z)
{
  plot->ax->show_minor_grid = x;
  plot->ay->show_minor_grid = y;
  plot->az->show_minor_grid = z;
}

/**
 * gtk_plot3d_major_grids_visible:
 * @plot: a #GtkPlot3D widget.
 * @x:
 * @y:
 * @z:
 *
 *
 */
void            
gtk_plot3d_major_grids_visible    (GtkPlot3D *plot,
                                       gboolean *x,
                                       gboolean *y,
                                       gboolean *z)
{
  *x = plot->ax->show_major_grid;
  *y = plot->ay->show_major_grid;
  *z = plot->az->show_major_grid;
}

/**
 * gtk_plot3d_minor_grids_visible:
 * @plot: a #GtkPlot3D widget.
 * @x:
 * @y:
 * @z:
 *
 *
 */
void            
gtk_plot3d_minor_grids_visible    (GtkPlot3D *plot,
                                       gboolean *x,
                                       gboolean *y,
                                       gboolean *z)
{
  *x = plot->ax->show_minor_grid;
  *y = plot->ay->show_minor_grid;
  *z = plot->az->show_minor_grid;
}

/**
 * gtk_plot3d_major_zgrid_set_attributes:
 * @plot: a #GtkPlot3D widget.
 * @style:
 * @width:
 * @color:
 *
 *
 */
void            
gtk_plot3d_major_zgrid_set_attributes    (GtkPlot3D *plot,
                                         GtkPlotLineStyle style,
                                         gfloat width,
                                         const GdkRGBA *color)
{
  plot->az->major_grid.line_style = style;
  plot->az->major_grid.line_width = width;
  if(color) plot->az->major_grid.color = *color;
}

/**
 * gtk_plot3d_minor_zgrid_set_attributes:
 * @plot: a #GtkPlot3D widget.
 * @style:
 * @width:
 * @color:
 *
 *
 */
void            
gtk_plot3d_minor_zgrid_set_attributes    (GtkPlot3D *plot,
                                         GtkPlotLineStyle style,
                                         gfloat width,
                                         const GdkRGBA *color)
{
  plot->az->minor_grid.line_style = style;
  plot->az->minor_grid.line_width = width;
  if(color) plot->az->minor_grid.color = *color;
}

/**
 * gtk_plot3d_major_zgrid_get_attributes:
 * @plot: a #GtkPlot3D widget.
 * @style:
 * @width:
 * @color:
 *
 *
 */
void            
gtk_plot3d_major_zgrid_get_attributes    (GtkPlot3D *plot,
                                         GtkPlotLineStyle *style,
                                         gfloat *width,
                                         GdkRGBA *color)
{
  *style = plot->az->major_grid.line_style;
  *width = plot->az->major_grid.line_width;
  *color = plot->az->major_grid.color;
}

/**
 * gtk_plot3d_minor_zgrid_get_attributes: 
 * @plot: a #GtkPlot3D widget.
 * @style:
 * @width:
 * @color:
 *
 *
 */
void            
gtk_plot3d_minor_zgrid_get_attributes    (GtkPlot3D *plot,
                                         GtkPlotLineStyle *style,
                                         gfloat *width,
                                         GdkRGBA *color)
{
  *style = plot->az->minor_grid.line_style;
  *width = plot->az->minor_grid.line_width;
  *color = plot->az->minor_grid.color;
}

/******************************************* 
 * gtk_plot3d_reset_angles                     
 * gtk_plot3d_rotate_x                     
 * gtk_plot3d_rotate_y                     
 * gtk_plot3d_rotate_z                     
 *******************************************/

/**
 * gtk_plot3d_reset_angles:
 * @plot: a #GtkPlot3D widget.
 *
 *
 */
void
gtk_plot3d_reset_angles(GtkPlot3D *plot)
{
  plot->a1 = 0;
  plot->a2 = 0;
  plot->a3 = 0;
  plot->e1.x = plot->xfactor;
  plot->e1.y = 0.;
  plot->e1.z = 0.;
  plot->e2.x = 0.;
  plot->e2.y = -plot->yfactor;
  plot->e2.z = 0.;
  plot->e3.x = 0.;
  plot->e3.y = 0.;
  plot->e3.z = -plot->zfactor;

  g_signal_emit_by_name(plot, "update", FALSE);
  g_signal_emit_by_name(plot, "changed");
}

/**
 * gtk_plot3d_rotate_x:
 * @plot: a #GtkPlot3D widget.
 * @angle:
 *
 *
 */
void
gtk_plot3d_rotate_x(GtkPlot3D *plot, gdouble angle)
{
  GtkPlotVector vector, aux, e1, e2, e3;
  gdouble c, s;

  int iangle = (int)angle;
  iangle = iangle%360;

  plot->a1 += iangle;

  iangle = 360 - iangle;
  iangle = iangle%360;
  c = plot->ncos[iangle];
  s = plot->nsin[iangle];

  e1 = plot->e1;
  e2 = plot->e2;
  e3 = plot->e3;

  vector.x = 0.0;
  vector.y = 1.0;
  vector.z = 0.0;
  aux = vector;
  aux.y = vector.y*c - vector.z*s;
  aux.z = vector.z*c + vector.y*s;

  plot->e2.x = aux.x * e1.x / plot->xfactor + aux.y * e2.x / plot->yfactor + aux.z * e3.x / plot->zfactor ;
  plot->e2.y = aux.x * e1.y / plot->xfactor + aux.y * e2.y / plot->yfactor + aux.z * e3.y / plot->zfactor ;
  plot->e2.z = aux.x * e1.z / plot->xfactor + aux.y * e2.z / plot->yfactor + aux.z * e3.z / plot->zfactor ;

  vector.x = 0.0;
  vector.y = 0.0;
  vector.z = 1.0;
  aux = vector;
  aux.y = vector.y*c - vector.z*s;
  aux.z = vector.z*c + vector.y*s;

  plot->e3.x = aux.x * e1.x / plot->xfactor + aux.y * e2.x / plot->yfactor + aux.z * e3.x / plot->zfactor ;
  plot->e3.y = aux.x * e1.y / plot->xfactor + aux.y * e2.y / plot->yfactor + aux.z * e3.y / plot->zfactor ;
  plot->e3.z = aux.x * e1.z / plot->xfactor + aux.y * e2.z / plot->yfactor + aux.z * e3.z / plot->zfactor ;

  plot->e1.x *= plot->xfactor;
  plot->e1.y *= plot->xfactor;
  plot->e1.z *= plot->xfactor;
  plot->e2.x *= plot->yfactor;
  plot->e2.y *= plot->yfactor;
  plot->e2.z *= plot->yfactor;
  plot->e3.x *= plot->zfactor;
  plot->e3.y *= plot->zfactor;
  plot->e3.z *= plot->zfactor;

  g_signal_emit_by_name(plot, "update", FALSE);
  g_signal_emit_by_name(plot, "changed");
}

/**
 * gtk_plot3d_rotate_y:
 * @plot: a #GtkPlot3D widget.
 * @angle:
 *
 *
 */
void
gtk_plot3d_rotate_y(GtkPlot3D *plot, gdouble angle)
{
  GtkPlotVector vector, aux, e1, e2, e3;
  gdouble  c, s;

  int iangle = (int)angle;
  iangle = iangle%360;

  plot->a2 += iangle;

  iangle = 360 - iangle;
  iangle = iangle%360;
  c = plot->ncos[iangle];
  s = plot->nsin[iangle];

  e1 = plot->e1;
  e2 = plot->e2;
  e3 = plot->e3;

  vector.x = 1.0;
  vector.y = 0.0;
  vector.z = 0.0;
  aux = vector;
  aux.z = vector.z*c - vector.x*s;
  aux.x = vector.x*c + vector.z*s;

  plot->e1.x = aux.x * e1.x / plot->xfactor + aux.y * e2.x / plot->yfactor + aux.z * e3.x / plot->zfactor ;
  plot->e1.y = aux.x * e1.y / plot->xfactor + aux.y * e2.y / plot->yfactor + aux.z * e3.y / plot->zfactor ;
  plot->e1.z = aux.x * e1.z / plot->xfactor + aux.y * e2.z / plot->yfactor + aux.z * e3.z / plot->zfactor ;

  vector.x = 0.0;
  vector.y = 0.0;
  vector.z = 1.0;
  aux = vector;
  aux.z = vector.z*c - vector.x*s;
  aux.x = vector.x*c + vector.z*s;

  plot->e3.x = aux.x * e1.x / plot->xfactor + aux.y * e2.x / plot->yfactor + aux.z * e3.x / plot->zfactor ;
  plot->e3.y = aux.x * e1.y / plot->xfactor + aux.y * e2.y / plot->yfactor + aux.z * e3.y / plot->zfactor ;
  plot->e3.z = aux.x * e1.z / plot->xfactor + aux.y * e2.z / plot->yfactor + aux.z * e3.z / plot->zfactor ;

  plot->e1.x *= plot->xfactor;
  plot->e1.y *= plot->xfactor;
  plot->e1.z *= plot->xfactor;
  plot->e2.x *= plot->yfactor;
  plot->e2.y *= plot->yfactor;
  plot->e2.z *= plot->yfactor;
  plot->e3.x *= plot->zfactor;
  plot->e3.y *= plot->zfactor;
  plot->e3.z *= plot->zfactor;

  g_signal_emit_by_name(plot, "update", FALSE);
  g_signal_emit_by_name(plot, "changed");
}

/**
 * gtk_plot3d_rotate_z:
 * @plot: a #GtkPlot3D widget.
 * @angle:
 *
 *
 */
void
gtk_plot3d_rotate_z(GtkPlot3D *plot, gdouble angle)
{
  GtkPlotVector vector, aux, e1, e2, e3;
  gdouble c, s;
  int iangle = (int)angle;
  iangle = iangle%360;

  plot->a3 += iangle;

  iangle = 360 - iangle;
  iangle = iangle%360;
  c = plot->ncos[iangle];
  s = plot->nsin[iangle];

  e1 = plot->e1;
  e2 = plot->e2;
  e3 = plot->e3;

  vector.x = 1.0;
  vector.y = 0.0;
  vector.z = 0.0;
  aux = vector;
  aux.x = vector.x*c - vector.y*s;
  aux.y = vector.y*c + vector.x*s;

  plot->e1.x = aux.x * e1.x / plot->xfactor + aux.y * e2.x / plot->yfactor + aux.z * e3.x / plot->zfactor ;
  plot->e1.y = aux.x * e1.y / plot->xfactor + aux.y * e2.y / plot->yfactor + aux.z * e3.y / plot->zfactor ;
  plot->e1.z = aux.x * e1.z / plot->xfactor + aux.y * e2.z / plot->yfactor + aux.z * e3.z / plot->zfactor ;

  vector.x = 0.0;
  vector.y = 1.0;
  vector.z = 0.0;
  aux = vector;
  aux.x = vector.x*c - vector.y*s;
  aux.y = vector.y*c + vector.x*s;

  plot->e2.x = aux.x * e1.x / plot->xfactor + aux.y * e2.x / plot->yfactor + aux.z * e3.x / plot->zfactor ;
  plot->e2.y = aux.x * e1.y / plot->xfactor + aux.y * e2.y / plot->yfactor + aux.z * e3.y / plot->zfactor ;
  plot->e2.z = aux.x * e1.z / plot->xfactor + aux.y * e2.z / plot->yfactor + aux.z * e3.z / plot->zfactor ;

  plot->e1.x *= plot->xfactor;
  plot->e1.y *= plot->xfactor;
  plot->e1.z *= plot->xfactor;
  plot->e2.x *= plot->yfactor;
  plot->e2.y *= plot->yfactor;
  plot->e2.z *= plot->yfactor;
  plot->e3.x *= plot->zfactor;
  plot->e3.y *= plot->zfactor;
  plot->e3.z *= plot->zfactor;

  g_signal_emit_by_name(plot, "update", FALSE);
  g_signal_emit_by_name(plot, "changed");
}

/**
 * gtk_plot3d_set_scale:
 * @plot: a #GtkPlot3D widget.
 * @axis:
 * @scale:
 *
 *
 */
void
gtk_plot3d_set_scale       (GtkPlot3D *plot,
                            GtkPlotOrientation axis,
                            GtkPlotScale scale)
{
  GtkPlotAxis *ax;
  ax = gtk_plot3d_get_axis(plot, axis);
  ax->ticks.scale = scale;
}

/**
 * gtk_plot3d_get_scale:
 * @plot: a #GtkPlot3D widget.
 * @axis:
 *
 *
 *
 * Return value:
 */

GtkPlotScale
gtk_plot3d_get_scale       (GtkPlot3D *plot,
                            GtkPlotOrientation axis)
{
  GtkPlotAxis *ax;
  ax = gtk_plot3d_get_axis(plot, axis);
  return(ax->ticks.scale);
}
