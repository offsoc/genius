/* GENIUS Calculator
 * Copyright (C) 2003 George Lebl
 *
 * Author: George Lebl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the  Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 * USA.
 */
#include "config.h"

#include <gnome.h>
#include <string.h>
#include <libgnomecanvas/libgnomecanvas.h>
#include <math.h>

#include "calc.h"
#include "eval.h"
#include "util.h"
#include "dict.h"
#include "funclib.h"
#include "matrixw.h"
#include "compil.h"
#include "plugin.h"

#include "geloutput.h"

#include "mpwrap.h"

#include "graphing.h"

/* FIXME: need header for gnome-genius.c */
void genius_interrupt_calc (void);

extern GtkWidget *genius_window;
extern int interrupted;
extern GHashTable *uncompiled;
extern calcstate_t calcstate;

static GtkWidget *graph_window = NULL;
static GnomeCanvas *canvas = NULL;
static GnomeCanvasGroup *root = NULL;
static GnomeCanvasGroup *graph = NULL;

static double defx1 = -M_PI;
static double defx2 = M_PI;
static double defy1 = -1.1;
static double defy2 = 1.1;

#define MAXFUNC 10

/* Replotting info */
static GelEFunc *replot_func[MAXFUNC] = { NULL };
static double replotx1 = -M_PI;
static double replotx2 = M_PI;
static double reploty1 = -1.1;
static double reploty2 = 1.1;

static void replot_functions (GelCtx *ctx);

static int replot_in_progress = 0;

#define P2C_X(x) (((x)-x1)*xscale)
#define P2C_Y(y) ((y2-(y))*yscale)

#define WIDTH 640
#define HEIGHT 480

#define RESPONSE_STOP 1
#define RESPONSE_ZOOMOUT 2

static void
dialog_response (GtkWidget *w, int response, gpointer data)
{
	if (response == GTK_RESPONSE_CLOSE ||
	    response == GTK_RESPONSE_DELETE_EVENT) {
		if (replot_in_progress > 0)
			interrupted = TRUE;
		gtk_widget_destroy (graph_window);
	} else if (response == RESPONSE_STOP && replot_in_progress > 0) {
		interrupted = TRUE;
	} else if (response == RESPONSE_ZOOMOUT && replot_in_progress == 0) {
		double len = replotx2 - replotx1;
		GelCtx *ctx;
		replotx2 += len/2.0;
		replotx1 -= len/2.0;

		len = reploty2 - reploty1;
		reploty2 += len/2.0;
		reploty1 -= len/2.0;

		ctx = eval_get_context ();
		replot_functions (ctx);
		eval_free_context (ctx);
	}
}

static double button_press_x1 = 0.0;
static double button_press_y1 = 0.0;

static GnomeCanvasItem *zoom_rect = NULL;
static gboolean dragging = FALSE;

/* 10 pixels */
#define MINZOOM 10

static gboolean
mouse_event (GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
	double xscale, yscale;

	if (event->type == GDK_BUTTON_PRESS && event->button.button == 1) {
		dragging = TRUE;
		button_press_x1 = event->button.x;
		button_press_y1 = event->button.y;
	} else if (event->type == GDK_MOTION_NOTIFY && dragging) {
		double x1, x2, y1, y2;
		if (zoom_rect != NULL)
			gtk_object_destroy (GTK_OBJECT (zoom_rect));
		if (event->motion.x < button_press_x1) {
			x1 = event->motion.x;
			x2 = button_press_x1;
		} else {
			x1 = button_press_x1;
			x2 = event->motion.x;
		}

		if (event->motion.y < button_press_y1) {
			y1 = event->motion.y;
			y2 = button_press_y1;
		} else {
			y1 = button_press_y1;
			y2 = event->motion.y;
		}

		if (x2 - x1 < MINZOOM &&
		    y2 - y1 < MINZOOM)
			return FALSE;

		if (x2 - x1 < MINZOOM)
			x2 = x1 + MINZOOM;
		if (y2 - y1 < MINZOOM)
			y2 = y1 + MINZOOM;

		zoom_rect =
			gnome_canvas_item_new (root,
					       gnome_canvas_rect_get_type (),
					       "outline_color", "red",
					       "width_units", 1.0,
					       "x1", x1,
					       "x2", x2,
					       "y1", y1,
					       "y2", y2,
					       NULL);
		g_signal_connect (G_OBJECT (zoom_rect), "destroy",
				  G_CALLBACK (gtk_widget_destroyed), &zoom_rect);
	} else if (event->type == GDK_BUTTON_RELEASE && dragging) {
		double x1, x2, y1, y2;
		GelCtx *ctx;

		dragging = FALSE;

		if (zoom_rect != NULL)
			gtk_object_destroy (GTK_OBJECT (zoom_rect));

		if (event->button.x < button_press_x1) {
			x1 = event->button.x;
			x2 = button_press_x1;
		} else {
			x1 = button_press_x1;
			x2 = event->button.x;
		}

		if (event->button.y < button_press_y1) {
			y1 = event->button.y;
			y2 = button_press_y1;
		} else {
			y1 = button_press_y1;
			y2 = event->button.y;
		}

		if (x2 - x1 < MINZOOM &&
		    y2 - y1 < MINZOOM)
			return FALSE;

		if (x2 - x1 < MINZOOM)
			x2 = x1 + MINZOOM;
		if (y2 - y1 < MINZOOM)
			y2 = y1 + MINZOOM;

		/* get current scale */
		xscale = WIDTH/(replotx2-replotx1);
		yscale = HEIGHT/(reploty2-reploty1);

		/* rescale */
		replotx2 = x2 / xscale + replotx1;
		replotx1 = x1 / xscale + replotx1;
		/* note: y is flipped */
		reploty1 = reploty2 - y2 / yscale;
		reploty2 = reploty2 - y1 / yscale;

		ctx = eval_get_context ();
		replot_functions (ctx);
		eval_free_context (ctx);
	}

	return FALSE;
}

static void
canvas_realize (GtkWidget *win)
{
	GdkCursor *cursor = gdk_cursor_new (GDK_CROSSHAIR);
	gdk_window_set_cursor (win->window, cursor);
	gdk_cursor_unref (cursor);
}

static void
ensure_window (void)
{
	if (graph_window != NULL) {
		/* FIXME: present is evil in that it takes focus away */
		gtk_widget_show (graph_window);
		return;
	}

	graph_window = gtk_dialog_new_with_buttons
		(_("Genius Line Plot") /* title */,
		 GTK_WINDOW (genius_window) /* parent */,
		 0 /* flags */,
		 _("_Zoom out"),
		 RESPONSE_ZOOMOUT,
		 GTK_STOCK_STOP,
		 RESPONSE_STOP,
		 GTK_STOCK_CLOSE,
		 GTK_RESPONSE_CLOSE,
		 NULL);
	g_signal_connect (G_OBJECT (graph_window),
			  "destroy",
			  G_CALLBACK (gtk_widget_destroyed),
			  &graph_window);
	g_signal_connect (G_OBJECT (graph_window),
			  "response",
			  G_CALLBACK (dialog_response),
			  NULL);

	canvas = (GnomeCanvas *)gnome_canvas_new_aa ();
	g_signal_connect (G_OBJECT (canvas),
			  "realize",
			  G_CALLBACK (canvas_realize),
			  NULL);
	root = gnome_canvas_root (canvas);
	g_signal_connect (root, "event",
			  G_CALLBACK (mouse_event),
			  NULL);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (graph_window)->vbox),
			    GTK_WIDGET (canvas), TRUE, TRUE, 0);

	gtk_widget_set_size_request (GTK_WIDGET (canvas), WIDTH, HEIGHT);

	gnome_canvas_set_scroll_region (canvas,
					0 /*x1*/,
					0 /*y1*/,
					WIDTH /*x2*/,
					HEIGHT /*y2*/);

	gnome_canvas_item_new (root,
			       gnome_canvas_rect_get_type (),
			       "fill_color", "white",
			       "x1", 0.0,
			       "x2", (double)WIDTH,
			       "y1", 0.0,
			       "y2", (double)HEIGHT,
			       NULL);

	gtk_widget_show_all (graph_window);
}


static void
clear_graph (void)
{
	if (graph != NULL)
		gtk_object_destroy (GTK_OBJECT (graph));
	graph = (GnomeCanvasGroup *)
		gnome_canvas_item_new (root,
				       gnome_canvas_group_get_type (),
				       "x", 0.0,
				       "y", 0.0,
				       NULL);
	g_signal_connect (G_OBJECT (graph),
			  "destroy",
			  G_CALLBACK (gtk_widget_destroyed),
			  &graph);
}

static void
get_start_inc (double start, double end, double *first, double *inc, double *tol)
{
	int incs;
	double len = end-start;

	*inc = pow (10, floor (log10 (len)));
	incs = floor (len / *inc);

	while (incs < 3) {
		*inc /= 2.0;
		incs = floor (len / *inc);
	}

	while (incs > 6) {
		*inc *= 2.0;
		incs = floor (len / *inc);
	}

	*first = ceil (start / *inc) * *inc;

	if (start < 0.0 && end > 0.0) {
		*first = -1.0 * incs * *inc;
		while (*first < start)
			*first += *inc;
	}

	*tol = len / 1000.0;
}


static void
plot_axis (double xscale, double yscale, double x1, double x2, double y1, double y2)
{
	double inc, coord, tol, yleg, xleg;
	GnomeCanvasPoints *points;
	GtkAnchorType anchor, fanchor, canchor, lanchor;
	gboolean printed_zero = FALSE;

	if (x1 <= 0 && x2 >= 0) {
		points = gnome_canvas_points_new (2);
		points->coords[0] = P2C_X (0);
		points->coords[1] = P2C_Y (y1);
		points->coords[2] = P2C_X (0);
		points->coords[3] = P2C_Y (y2);

		gnome_canvas_item_new (graph,
				       gnome_canvas_line_get_type (),
				       "fill_color", "black",
				       "points", points,
				       NULL);

		gnome_canvas_points_unref (points);

		xleg = 0.0;
	} else {
		xleg = x1;
	}

	if (y1 <= 0 && y2 >= 0) {
		points = gnome_canvas_points_new (2);
		points->coords[0] = P2C_X (x1);
		points->coords[1] = P2C_Y (0);
		points->coords[2] = P2C_X (x2);
		points->coords[3] = P2C_Y (0);

		gnome_canvas_item_new (graph,
				       gnome_canvas_line_get_type (),
				       "fill_color", "black",
				       "points", points,
				       NULL);

		gnome_canvas_points_unref (points);

		yleg = 0.0;
	} else {
		yleg = y1;
	}

	/*
	 * X axis labels
	 */
	get_start_inc (x1, x2, &coord, &inc, &tol);

	if (yleg < y1 + (y2-y1) / 8.0) {
		fanchor = GTK_ANCHOR_SW;
		lanchor = GTK_ANCHOR_SE;
		canchor = GTK_ANCHOR_S;
	} else {
		fanchor = GTK_ANCHOR_NW;
		lanchor = GTK_ANCHOR_NE;
		canchor = GTK_ANCHOR_N;
	}

	if (coord - (inc/2.0) <= x1)
		anchor = fanchor;
	else
		anchor = canchor;
	do {
		char buf[20];
		if (fabs (coord) < tol) {
			strcpy (buf, "0");
			printed_zero = TRUE;
		} else {
			g_snprintf (buf, sizeof (buf), "%g", coord);
		}

		gnome_canvas_item_new (graph,
				       gnome_canvas_text_get_type (),
				       "text", buf,
				       "fill_color", "black",
				       "anchor", anchor,
				       "x", P2C_X (coord),
				       "y", P2C_Y (yleg),
				       NULL);

		points = gnome_canvas_points_new (2);
		points->coords[0] = P2C_X (coord);
		points->coords[1] = P2C_Y (yleg)-7;
		points->coords[2] = P2C_X (coord);
		points->coords[3] = P2C_Y (yleg)+7;

		gnome_canvas_item_new (graph,
				       gnome_canvas_line_get_type (),
				       "fill_color", "black",
				       "points", points,
				       NULL);

		gnome_canvas_points_unref (points);
		
		coord += inc;
		if (coord + (inc/2.0) >= x2)
			anchor = lanchor;
		else
			anchor = canchor;
	} while (coord <= x2);

	/*
	 * Y axis labels
	 */
	get_start_inc (y1, y2, &coord, &inc, &tol);

	if (xleg < x1 + (x2-x1) / 5.0) {
		fanchor = GTK_ANCHOR_SW;
		lanchor = GTK_ANCHOR_NW;
		canchor = GTK_ANCHOR_W;
	} else {
		fanchor = GTK_ANCHOR_SE;
		lanchor = GTK_ANCHOR_NE;
		canchor = GTK_ANCHOR_E;
	}

	if (coord - (inc/2.0) <= y1)
		anchor = fanchor;
	else
		anchor = canchor;
	do {
		char buf[20];
		if (fabs (coord) < tol) {
			if (printed_zero)
				buf[0] = '\0';
			else
				strcpy (buf, "0");
		} else {
			g_snprintf (buf, sizeof (buf), "%g", coord);
		}

		if (buf[0] != '\0') {
			gnome_canvas_item_new (graph,
					       gnome_canvas_text_get_type (),
					       "text", buf,
					       "fill_color", "black",
					       "anchor", anchor,
					       "x", P2C_X (xleg),
					       "y", P2C_Y (coord),
					       NULL);

			points = gnome_canvas_points_new (2);
			points->coords[0] = P2C_X (xleg)-7;
			points->coords[1] = P2C_Y (coord);
			points->coords[2] = P2C_X (xleg)+7;
			points->coords[3] = P2C_Y (coord);

			gnome_canvas_item_new (graph,
					       gnome_canvas_line_get_type (),
					       "fill_color", "black",
					       "points", points,
					       NULL);

			gnome_canvas_points_unref (points);
		}
		
		coord += inc;
		if (coord + (inc/2.0) >= y2)
			anchor = lanchor;
		else
			anchor = canchor;
	} while (coord <= y2);
}

static double
call_func (GelCtx *ctx, GelEFunc *func, GelETree *arg)
{
	GelETree *ret;
	double retd;
	GelETree *args[2];

	args[0] = arg;
	args[1] = NULL;

	ret = funccall (ctx, func, args, 1);

	/* FIXME: handle errors! */
	if (error_num != 0)
		error_num = 0;
	/* FIXME: handle errors! */
	if (ret == NULL || ret->type != VALUE_NODE) {
		gel_freetree (ret);
		return 0;
	}

	retd = mpw_get_double (ret->val.value);
	/* FIXME: handle errors! */
	if (error_num != 0)
		error_num = 0;
	
	gel_freetree (ret);
	return retd;
}

static void
plot_line (GnomeCanvasItem **line, GnomeCanvasItem **progress_line,
	   const char *color,
	   GnomeCanvasPoints *points,
	   GnomeCanvasPoints *progress_points,
	   int num)
{
	int old_points = points->num_points;
	points->num_points = num;

	if (*line != NULL)
		gtk_object_destroy (GTK_OBJECT (*line));
	if (*progress_line != NULL)
		gtk_object_destroy (GTK_OBJECT (*progress_line));
	*line = gnome_canvas_item_new (graph,
				       gnome_canvas_line_get_type (),
				       "fill_color", color,
				       "width_units", 1.5,
				       "points", points,
				       NULL);
	if (progress_points != NULL) {
		progress_points->coords[2] = points->coords[(num-1)*2];
		*progress_line = gnome_canvas_item_new
			(graph,
			 gnome_canvas_line_get_type (),
			 "fill_color", color,
			 "width_units", 10.0,
			 "points", progress_points,
			 NULL);
	}

	points->num_points = old_points;
}

static void
plot_func (GelCtx *ctx, GelEFunc *func, const char *color, double xscale, double yscale, double x1, double x2, double y1, double y2)
{
#define PERITER 2
	GnomeCanvasItem *line = NULL;
	GnomeCanvasItem *progress_line = NULL;
	GelETree *arg;
	mpw_t x;
	GnomeCanvasPoints *points = gnome_canvas_points_new (WIDTH/PERITER + 1);
	GnomeCanvasPoints *progress_points = gnome_canvas_points_new (2);
	int i;

	progress_points->coords[0] = 0.0;
	progress_points->coords[1] = (double)HEIGHT;
	progress_points->coords[2] = 0.0;
	progress_points->coords[3] = (double)HEIGHT;
	
	mpw_init (x);
	arg = gel_makenum_use (x);
	for (i = 0; i < WIDTH/PERITER + 1; i++) {
		double xd = x1+(i*PERITER)/xscale;
		double y;
		mpw_set_d (arg->val.value, xd);
		y = call_func (ctx, func, arg);
		points->coords[i*2] = P2C_X (xd);
		points->coords[i*2 + 1] = P2C_Y (y);
		/* hack for "infinity" */
		if (points->coords[i*2 + 1] >= HEIGHT*2)
			points->coords[i*2 + 1] = HEIGHT*2;
		/* hack for "infinity" */
		else if (points->coords[i*2 + 1] <= -HEIGHT)
			points->coords[i*2 + 1] = -HEIGHT;
		if (i%40 == 0 && i>0) {
			plot_line (&line, &progress_line, color,
				   points, progress_points, i+1);
		}
		if(evalnode_hook) {
			(*evalnode_hook)();
			if (interrupted) {
				gel_freetree (arg);
				gnome_canvas_points_unref (points);
				return;
			}
		}
	}
	gel_freetree (arg);

	plot_line (&line, &progress_line, color,
		   points, NULL, WIDTH/PERITER + 1);

	gnome_canvas_points_unref (points);
}

static void
label_func (GelCtx *ctx, int i, GelEFunc *func, const char *color)
{
	char *text;
	GnomeCanvasPoints *points = gnome_canvas_points_new (2);
	points->coords[0] = WIDTH-40;
	points->coords[1] = 20+15*i;
	points->coords[2] = WIDTH-20;
	points->coords[3] = 20+15*i;

	gnome_canvas_item_new (graph,
			       gnome_canvas_line_get_type (),
			       "fill_color", color,
			       "width_units", 1.5,
			       "points", points,
			       NULL);

	gnome_canvas_points_unref (points);

	text = NULL;
	if (func->id != NULL) {
		text = g_strdup_printf ("%s(x)", func->id->token);
	} else if (func->type == GEL_USER_FUNC) {
		int old_style, len;
		GelOutput *out = gel_output_new ();
		D_ENSURE_USER_BODY (func);
		gel_output_setup_string (out, 0, NULL);

		/* FIXME: the push/pop of style is UGLY */
		old_style = calcstate.output_style;
		calcstate.output_style = GEL_OUTPUT_NORMAL;
		print_etree (out, func->data.user, TRUE /* toplevel */);
		calcstate.output_style = old_style;

		text = gel_output_snarf_string (out);
		gel_output_unref (out);

		len = strlen (text);

		if (len > 2 &&
		    text[0] == '(' &&
		    text[len-1] == ')') {
			text[len-1] = '\0';
			strcpy (text, &text[1]);
			len-=2;
		}

		/* only print bodies of short functions */
		if (len > 64) {
			g_free (text);
			text = NULL;
		}
	}

	if (text == NULL) {
		text = g_strdup_printf (_("Function #%d"), i+1);
	}

	gnome_canvas_item_new (graph,
			       gnome_canvas_text_get_type (),
			       "x", (double)(WIDTH-45),
			       "y", (double)(20+15*i),
			       "font", "Monospace 10",
			       "fill_color", "black",
			       "anchor", GTK_ANCHOR_EAST,
			       "text", text,
			       NULL);

	g_free (text);
}

#define GET_DOUBLE(var,argnum) \
	{ \
	if (a[argnum]->type != VALUE_NODE) { \
		gel_errorout (_("%s: argument number %d not a number"), "LinePlot", argnum+1); \
		return NULL; \
	} \
	var = mpw_get_double (a[argnum]->val.value); \
	}

static gboolean
get_limits_from_matrix (GelETree *m, double *x1, double *x2, double *y1, double *y2)
{
	GelETree *t;

	if (m->type != MATRIX_NODE ||
	    gel_matrixw_elements (m->mat.matrix) != 4) {
		gel_errorout (_("Graph limits not given as a 4-vector"));
		return FALSE;
	}

	t = gel_matrixw_vindex (m->mat.matrix, 0);
	if (t->type != VALUE_NODE) {
		gel_errorout (_("Graph limits not given as numbers"));
		return FALSE;
	}
	*x1 = mpw_get_double (t->val.value);
	t = gel_matrixw_vindex (m->mat.matrix, 1);
	if (t->type != VALUE_NODE) {
		gel_errorout (_("Graph limits not given as numbers"));
		return FALSE;
	}
	*x2 = mpw_get_double (t->val.value);
	t = gel_matrixw_vindex (m->mat.matrix, 2);
	if (t->type != VALUE_NODE) {
		gel_errorout (_("Graph limits not given as numbers"));
		return FALSE;
	}
	*y1 = mpw_get_double (t->val.value);
	t = gel_matrixw_vindex (m->mat.matrix, 3);
	if (t->type != VALUE_NODE) {
		gel_errorout (_("Graph limits not given as numbers"));
		return FALSE;
	}
	*y2 = mpw_get_double (t->val.value);

	/* FIXME: what about errors */
	if (error_num != 0) {
		error_num = 0;
		return FALSE;
	}

	if (*x1 > *x2) {
		double s = *x1;
		*x1 = *x2;
		*x2 = s;
	}

	if (*y1 > *y2) {
		double s = *y1;
		*y1 = *y2;
		*y2 = s;
	}

	return TRUE;
}

static GelETree *
make_matrix_from_limits (void)
{
	GelETree *n;
	GelMatrixW *m;
	/*make us a new empty node*/
	GET_NEW_NODE (n);
	n->type = MATRIX_NODE;
	m = n->mat.matrix = gel_matrixw_new ();
	n->mat.quoted = FALSE;
	gel_matrixw_set_size (m, 4, 1);

	gel_matrixw_set_index (m, 0, 0) = gel_makenum_d (defx1);
	gel_matrixw_set_index (m, 1, 0) = gel_makenum_d (defx2);
	gel_matrixw_set_index (m, 2, 0) = gel_makenum_d (defy1);
	gel_matrixw_set_index (m, 3, 0) = gel_makenum_d (defy2);

	return n;
}

static void
replot_functions (GelCtx *ctx)
{
	char *colors[] = {
		"darkblue",
		"darkgreen",
		"darkred",
		"magenta",
		"black",
		"darkorange",
		"blue",
		"green",
		"red",
		"brown",
		NULL };
	double xscale, yscale;
	int i;

	ensure_window ();

	gtk_dialog_set_response_sensitive (GTK_DIALOG (graph_window),
					   RESPONSE_STOP, TRUE);
	gtk_dialog_set_response_sensitive (GTK_DIALOG (graph_window),
					   RESPONSE_ZOOMOUT, FALSE);

	replot_in_progress ++;

	clear_graph ();

	/* sanity */
	if (replotx2 == replotx1)
		replotx2 = replotx1 + 0.00000001;
	/* sanity */
	if (reploty2 == reploty1)
		reploty2 = reploty1 + 0.00000001;

	xscale = WIDTH/(replotx2-replotx1);
	yscale = HEIGHT/(reploty2-reploty1);

	plot_axis (xscale, yscale, replotx1, replotx2, reploty1, reploty2);

	for (i = 0; i < MAXFUNC && replot_func[i] != NULL; i++) {
		plot_func (ctx, replot_func[i], colors[i],
			   xscale, yscale, replotx1, replotx2, reploty1, reploty2);

		if (interrupted) break;

		label_func (ctx, i, replot_func[i], colors[i]);

		if (evalnode_hook)
			(*evalnode_hook)();

		if (interrupted) break;
	}

	if (graph_window != NULL) {
		gtk_dialog_set_response_sensitive (GTK_DIALOG (graph_window),
						   RESPONSE_STOP, FALSE);
		gtk_dialog_set_response_sensitive (GTK_DIALOG (graph_window),
						   RESPONSE_ZOOMOUT, TRUE);
	}

	replot_in_progress --;
}

static GelETree *
LinePlot_op (GelCtx *ctx, GelETree * * a, int *exception)
{
	double x1, x2, y1, y2;
	int funcs = 0;
	int i;

	for (i = 0; i < MAXFUNC && replot_func[i] != NULL; i++) {
		d_freefunc (replot_func[i]);
		replot_func[i] = NULL;
	}

	for (i = 0;
	     i < MAXFUNC && a[i] != NULL && a[i]->type == FUNCTION_NODE;
	     i++) {
		replot_func[funcs] = d_copyfunc (a[i]->func.func);
		replot_func[funcs]->context = -1;
		funcs++;
	}

	if (a[i] != NULL && a[i]->type == FUNCTION_NODE) {
		gel_errorout (_("%s: only up to 10 functions supported"), "LinePlot");
		return NULL;
	}

	if (funcs == 0) {
		gel_errorout (_("%s: argument not a function"), "LinePlot");
		return NULL;
	}

	/* Defaults */
	x1 = defx1;
	x2 = defx2;
	y1 = defy1;
	y2 = defy2;

	if (a[i] != NULL) {
		if (a[i]->type == MATRIX_NODE) {
			if ( ! get_limits_from_matrix (a[i], &x1, &x2, &y1, &y2))
				return NULL;
			i++;
		} else {
			GET_DOUBLE(x1,i);
			i++;
			if (a[i] != NULL) {
				GET_DOUBLE(x2,i);
				i++;
				if (a[i] != NULL) {
					GET_DOUBLE(y1,i);
					i++;
					if (a[i] != NULL) {
						GET_DOUBLE(y2,i);
						i++;
					}
				}
			}
			/* FIXME: what about errors */
			if (error_num != 0) {
				error_num = 0;
				return NULL;
			}
		}
	}

	if (x1 > x2) {
		double s = x1;
		x1 = x2;
		x2 = s;
	}

	if (y1 > y2) {
		double s = y1;
		y1 = y2;
		y2 = s;
	}

	if (x1 == x2) {
		gel_errorout (_("%s: invalid X range"), "LinePlot");
		return NULL;
	}

	if (y1 == y2) {
		gel_errorout (_("%s: invalid Y range"), "LinePlot");
		return NULL;
	}

	replotx1 = x1;
	replotx2 = x2;
	reploty1 = y1;
	reploty2 = y2;

	replot_functions (ctx);

	if (interrupted)
		return NULL;
	else
		return gel_makenum_null ();
}

static GelETree *
set_LinePlotWindow (GelETree * a)
{
	double x1, x2, y1, y2;
	if ( ! get_limits_from_matrix (a, &x1, &x2, &y1, &y2))
		return NULL;

	defx1 = x1;
	defx2 = x2;
	defy1 = y1;
	defy2 = y2;

	return make_matrix_from_limits ();
}

static GelETree *
get_LinePlotWindow (void)
{
	return make_matrix_from_limits ();
}

void
gel_add_graph_functions (void)
{
	GelEFunc *f;
	GelToken *id;

	new_category ("plotting", _("Plotting"));

	/* FIXME: add more help fields */
#define FUNC(name,args,argn,category,desc) \
	f = d_addfunc (d_makebifunc (d_intern ( #name ), name ## _op, args)); \
	d_add_named_args (f, argn); \
	add_category ( #name , category); \
	add_description ( #name , desc);
#define VFUNC(name,args,argn,category,desc) \
	f = d_addfunc (d_makebifunc (d_intern ( #name ), name ## _op, args)); \
	d_add_named_args (f, argn); \
	f->vararg = TRUE; \
	add_category ( #name , category); \
	add_description ( #name , desc);
#define ALIAS(name,args,aliasfor) \
	d_addfunc (d_makebifunc (d_intern ( #name ), aliasfor ## _op, args)); \
	add_alias ( #aliasfor , #name );
#define VALIAS(name,args,aliasfor) \
	f = d_addfunc (d_makebifunc (d_intern ( #name ), aliasfor ## _op, args)); \
	f->vararg = TRUE; \
	add_alias ( #aliasfor , #name );
#define PARAMETER(name,desc) \
	id = d_intern ( #name ); \
	id->parameter = 1; \
	id->built_in_parameter = 1; \
	id->data1 = set_ ## name; \
	id->data2 = get_ ## name; \
	add_category ( #name , "parameters"); \
	add_description ( #name , desc); \
	/* bogus value */ \
	d_addfunc_global (d_makevfunc (id, gel_makenum_null()));

	VFUNC (LinePlot, 1, "", "plotting", _("Plot a function with a line (very rudimentary).  First come the functions (up to 10) then optionally limits as x1,x2,y1,y2"));

	PARAMETER (LinePlotWindow, _("Plotting window (limits) as a 4-vector of the form [x1,x2,y1,y2]"));
}
