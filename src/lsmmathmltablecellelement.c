/* lsmmathmltablecellelement.c
 *
 * Copyright © 2007-2008  Emmanuel Pacaud
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 * Author:
 * 	Emmanuel Pacaud <emmanuel@gnome.org>
 */

#include <lsmmathmltablecellelement.h>

static GObjectClass *parent_class;

/* GdomNode implementation */

static const char *
lsm_mathml_table_cell_get_node_name (LsmDomNode *node)
{
	return "mtd";
}

/* LsmMathmlElement implementation */

static void
lsm_mathml_table_cell_element_update (LsmMathmlElement *self, LsmMathmlStyle *style)
{
	LsmMathmlTableCellElement *cell = LSM_MATHML_TABLE_CELL_ELEMENT (self);
	unsigned int span;

	span = 1;
	lsm_dom_unsigned_attribute_parse (&cell->row_span, &span);

	span = 1;
	lsm_dom_unsigned_attribute_parse (&cell->column_span, &span);
}

static const LsmMathmlBbox *
lsm_mathml_table_cell_element_measure (LsmMathmlElement *self, LsmMathmlView *view, const LsmMathmlBbox *bbox)
{
	LSM_MATHML_ELEMENT_CLASS (parent_class)->measure (self, view, bbox);

	return &self->bbox;
}

static void
lsm_mathml_table_cell_element_layout (LsmMathmlElement *self, LsmMathmlView *view,
				double x, double y, const LsmMathmlBbox *bbox)
{
	LSM_MATHML_ELEMENT_CLASS (parent_class)->layout (self, view, x, y, bbox);
}

/* LsmMathmlTableCellElement implementation */

void
lsm_mathml_table_cell_element_get_spans (const LsmMathmlTableCellElement *self,
				      unsigned int *row_span,
				      unsigned int *column_span)
{
	if (row_span != NULL)
		*row_span = 0;
	if (column_span != NULL)
		*column_span = 0;

	g_return_if_fail (LSM_IS_MATHML_TABLE_CELL_ELEMENT (self));

	if (row_span != NULL)
		*row_span = self->row_span.value;
	if (column_span != NULL)
		*column_span = self->column_span.value;
}

LsmDomNode *
lsm_mathml_table_cell_element_new (void)
{
	return g_object_new (LSM_TYPE_MATHML_TABLE_CELL_ELEMENT, NULL);
}

static void
lsm_mathml_table_cell_element_init (LsmMathmlTableCellElement *self)
{
}

/* LsmMathmlTableCellElement class */

static void
lsm_mathml_table_cell_element_class_init (LsmMathmlTableCellElementClass *table_cell_class)
{
	LsmDomNodeClass *d_node_class = LSM_DOM_NODE_CLASS (table_cell_class);
	LsmMathmlElementClass *m_element_class = LSM_MATHML_ELEMENT_CLASS (table_cell_class);

	parent_class = g_type_class_peek_parent (table_cell_class);

	d_node_class->get_node_name = lsm_mathml_table_cell_get_node_name;

	m_element_class->update = lsm_mathml_table_cell_element_update;
	m_element_class->measure = lsm_mathml_table_cell_element_measure;
	m_element_class->layout = lsm_mathml_table_cell_element_layout;

	m_element_class->attributes = lsm_dom_attribute_map_duplicate (m_element_class->attributes);

	lsm_dom_attribute_map_add_attribute (m_element_class->attributes, "rowspan",
					  offsetof (LsmMathmlTableCellElement, row_span));
	lsm_dom_attribute_map_add_attribute (m_element_class->attributes, "columnspan",
					  offsetof (LsmMathmlTableCellElement, column_span));
}

G_DEFINE_TYPE (LsmMathmlTableCellElement, lsm_mathml_table_cell_element, LSM_TYPE_MATHML_ELEMENT)