/* GENIUS Calculator
 * Copyright (C) 1997-2002 George Lebl
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

#ifndef _MATRIXW_H_
#define _MATRIXW_H_

#include <glib.h>
#include "structs.h"
#include "matrix.h"

/*the GelMatrixW typedef is in structs.h*/
struct _GelMatrixW {
	GelMatrix *m;
	int *regx;
	int *regy;
	int regw;
	int regh;
	guint16 tr:1; /*transposed*/
	guint16 cached_value_only:1;
	guint16 value_only:1;
	guint16 cached_value_only_real:1;
	guint16 value_only_real:1;
	guint16 cached_value_only_rational:1;
	guint16 value_only_rational:1;
	guint16 cached_value_only_integer:1;
	guint16 value_only_integer:1;
	guint16 cached_value_or_bool_only:1;
	guint16 value_or_bool_only:1;
};

/*new matrix*/
GelMatrixW *gel_matrixw_new(void);
GelMatrixW *gel_matrixw_new_with_matrix(GelMatrix *mat);

/*set size of a matrix*/
void gel_matrixw_set_size(GelMatrixW *m, int width, int height);
/*set the size of the matrix to be at least this*/
void gel_matrixw_set_at_least_size(GelMatrixW *m, int width, int height);

/*set element*/
void gel_matrixw_set_element(GelMatrixW *m, int x, int y, gpointer data);
void gel_matrixw_set_velement(GelMatrixW *m, int i, gpointer data);

/*copy a matrix*/
GelMatrixW * gel_matrixw_copy(GelMatrixW *source);

/* get rowsof and columsof matrices */
GelMatrixW * gel_matrixw_rowsof (GelMatrixW *source);
GelMatrixW * gel_matrixw_columnsof (GelMatrixW *source);
GelMatrixW * gel_matrixw_diagonalof (GelMatrixW *source);

GelMatrixW * gel_matrixw_get_region (GelMatrixW *m, int *regx, int *regy, int w, int h);
GelMatrixW * gel_matrixw_get_vregion (GelMatrixW *m, int *reg, int len);
void gel_matrixw_set_region(GelMatrixW *m, GelMatrixW *src,
			    int *destx, int *desty,
			    int w, int h);
void gel_matrixw_set_region_etree(GelMatrixW *m, GelETree *src,
				  int *destx, int *desty,
				  int w, int h);

void gel_matrixw_set_vregion (GelMatrixW *m, GelMatrixW *src,
			      int *desti, int len);
void gel_matrixw_set_vregion_etree(GelMatrixW *m, GelETree *src,
				  int *desti, int len);

/*transpose a matrix*/
GelMatrixW * gel_matrixw_transpose(GelMatrixW *m);

/*free a matrix*/
void gel_matrixw_free(GelMatrixW *m);

/*make private copy of the GelMatrix*/
void gel_matrixw_make_private(GelMatrixW *m);

extern GelETree *the_zero;

/*sort of unsafe, and only for setting, since we can
  get a NULL from this*/
#define gel_matrixw_set_index(a,i,j) \
	(gel_matrix_index((a)->m, \
			  (a)->regx ? (a)->regx[(a)->tr?(j):(i)] : (a)->tr?(j):(i), \
			  (a)->regy ? (a)->regy[(a)->tr?(i):(j)] : (a)->tr?(i):(j)))

/*get the value at, make sure it's in the range*/
G_INLINE_FUNC GelETree *gel_matrixw_index(GelMatrixW *m, int x, int y);
#ifdef G_CAN_INLINE
G_INLINE_FUNC GelETree *
gel_matrixw_index(GelMatrixW *m, int x, int y) {
	GelETree *t = gel_matrixw_set_index (m, x, y);
	return t?t:the_zero;
}
#endif

#define gel_matrixw_vindex(m,i) gel_matrixw_index((m),i%gel_matrixw_width(m),i/gel_matrixw_width(m))
#define gel_matrixw_set_vindex(m,i) gel_matrixw_set_index((m),i%gel_matrixw_width(m),i/gel_matrixw_width(m))

#define gel_matrixw_width(a) ((a)->tr?(a)->regh:(a)->regw)
#define gel_matrixw_height(a) ((a)->tr?(a)->regw:(a)->regh)
#define gel_matrixw_elements(a) ((a)->regw*(a)->regh)

#endif
