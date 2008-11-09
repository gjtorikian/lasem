/* gmathmlscriptelement.c
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

#include <gmathmlscriptelement.h>
#include <gmathmlview.h>
#include <math.h>

/* GdomNode implementation */

static const char *
gmathml_script_element_get_node_name (GDomNode *node)
{
	GMathmlScriptElement *script = GMATHML_SCRIPT_ELEMENT (node);

	switch (script->type) {
		case GMATHML_SCRIPT_ELEMENT_TYPE_SUB:
			return "msub";
		case GMATHML_SCRIPT_ELEMENT_TYPE_SUP:
			return "msup";
		case GMATHML_SCRIPT_ELEMENT_TYPE_SUB_SUP:
		default:
			return "msubsup";
	}
}

static gboolean
gmathml_script_element_can_append_child (GDomNode *self, GDomNode *child)
{
	GMathmlScriptElement *script = GMATHML_SCRIPT_ELEMENT (self);

	if (!GMATHML_ELEMENT (child))
		return FALSE;

	if (self->first_child == NULL ||
	    self->first_child->next_sibling == NULL)
		return TRUE;

	if (script->type != GMATHML_SCRIPT_ELEMENT_TYPE_SUB_SUP)
		return FALSE;

	return (self->first_child->next_sibling->next_sibling == NULL);
}

static void
gmathml_script_element_update_child_pointers (GMathmlScriptElement *self)
{
	GDomNode *node;

	node = GDOM_NODE (self)->first_child;
	if (node == NULL) {
		self->base = NULL;
		self->subscript = NULL;
		self->superscript = NULL;
		return;
	} else
		self->base = GMATHML_ELEMENT (node);

	node = node->next_sibling;
	if (node != NULL)
		switch (self->type) {
			case GMATHML_SCRIPT_ELEMENT_TYPE_SUP:
				self->subscript = NULL;
				self->superscript = GMATHML_ELEMENT (node);
				break;
			case GMATHML_SCRIPT_ELEMENT_TYPE_SUB:
				self->subscript = GMATHML_ELEMENT (node);
				self->superscript = NULL;
				break;
			default:
				self->subscript = GMATHML_ELEMENT (node);
				node = node->next_sibling;
				if (node != NULL)
					self->superscript = GMATHML_ELEMENT (node);
		}
	else {
		self->subscript = NULL;
		self->superscript = NULL;
	}
}

static void
gmathml_script_element_post_new_child (GDomNode *self, GDomNode *child)
{
	GMathmlScriptElement *script = GMATHML_SCRIPT_ELEMENT (self);

	gmathml_script_element_update_child_pointers (script);
}

/* GMathmlElement implementation */

static void
gmathml_script_element_update (GMathmlElement *self, GMathmlStyle *style)
{
	GMathmlScriptElement *script = GMATHML_SCRIPT_ELEMENT (self);

	gmathml_attribute_length_parse (&script->superscript_shift, &style->superscript_shift, style->math_size_value);
	gmathml_attribute_length_parse (&script->subscript_shift, &style->subscript_shift, style->math_size_value);

	script->display = style->display;
}

static void
gmathml_script_element_update_child (GMathmlElement *self, GMathmlStyle *style)
{
	GMathmlScriptElement *script = GMATHML_SCRIPT_ELEMENT (self);

	if (script->base != NULL)
		gmathml_element_update (GMATHML_ELEMENT (script->base), style);

	gmathml_style_change_script_level (style, +1);
	style->display = GMATHML_DISPLAY_INLINE;

	if (script->subscript != NULL)
		gmathml_element_update (GMATHML_ELEMENT (script->subscript), style);
	if (script->superscript != NULL)
		gmathml_element_update (GMATHML_ELEMENT (script->superscript), style);
}

static const GMathmlBbox *
gmathml_script_element_measure (GMathmlElement *element, GMathmlView *view,
				const GMathmlBbox *stretch_bbox)
{
	GMathmlScriptElement *script = GMATHML_SCRIPT_ELEMENT (element);
	GDomNode *node;
	GMathmlBbox const *base_bbox = NULL;
	GMathmlBbox const *subscript_bbox = NULL;
	GMathmlBbox const *superscript_bbox = NULL;
	GMathmlBbox children_bbox = gmathml_bbox_null;
	double axis_offset, ascent, descent;
	double v_space, h_space;

	element->bbox = gmathml_bbox_null;

	node = GDOM_NODE (element)->first_child;
	if (node == NULL)
		return &element->bbox;

	axis_offset = gmathml_view_measure_axis_offset (view, element->style.math_size);
	h_space = element->style.math_size * GMATHML_SPACE_EM_VERY_THIN;
	v_space = element->style.math_size * GMATHML_SPACE_EM_THIN;
	gmathml_view_get_font_metrics (view, &script->base->style, &ascent, &descent);

	if (script->display == GMATHML_DISPLAY_INLINE)
		descent /= 2.0;

	base_bbox = gmathml_element_measure (GMATHML_ELEMENT (node), view, stretch_bbox);
	element->bbox = *base_bbox;

	element->bbox.width += gmathml_view_measure_length (view, h_space);

	node = node->next_sibling;

	if (node != NULL) {
		GMathmlBbox const *bbox;

		bbox = gmathml_element_measure (GMATHML_ELEMENT (node), view, NULL);

		switch (script->type) {
			case GMATHML_SCRIPT_ELEMENT_TYPE_SUP:
				superscript_bbox = bbox;
				break;
			case GMATHML_SCRIPT_ELEMENT_TYPE_SUB:
				subscript_bbox = bbox;
				break;
			case GMATHML_SCRIPT_ELEMENT_TYPE_SUB_SUP:
				subscript_bbox = bbox;

				node = node->next_sibling;

				if (node != NULL)
					superscript_bbox = gmathml_element_measure (GMATHML_ELEMENT (node),
										    view, NULL);
		}
	}

	if (superscript_bbox != NULL) {
		if (script->display == GMATHML_DISPLAY_INLINE)
			script->superscript_offset = axis_offset;
		else {
			double superscript_descent;

			gmathml_view_get_font_metrics (view, &script->superscript->style,
						       NULL, &superscript_descent);
			script->superscript_offset = axis_offset + superscript_descent;
		}

		if (base_bbox->height > ascent)
			script->superscript_offset += base_bbox->height - ascent;
		if (script->superscript_offset - superscript_bbox->depth < axis_offset)
			script->superscript_offset = axis_offset + superscript_bbox->depth;
	} else
		script->superscript_offset = 0.0;

	if (subscript_bbox != NULL) {
		script->subscript_offset = descent;

		if (base_bbox->depth > descent)
			script->subscript_offset += base_bbox->depth - descent;
		if (subscript_bbox->height - script->subscript_offset > axis_offset)
			script->subscript_offset = subscript_bbox->height - axis_offset;
	} else
		script->subscript_offset = 0.0;

	if (superscript_bbox != NULL && subscript_bbox != NULL) {
		double delta = (script->superscript_offset + script->subscript_offset) -
			(superscript_bbox->depth + subscript_bbox->height);
		if (delta < v_space) {
			script->superscript_offset += fabs (delta - v_space) * 0.5;
			script->subscript_offset   += fabs (delta - v_space) * 0.5;
		}
	}

	script->superscript_offset = gmathml_view_measure_length (view, MAX (script->superscript_offset,
									    script->superscript_shift.value));
	script->subscript_offset = gmathml_view_measure_length (view, MAX (script->subscript_offset,
									  script->subscript_shift.value));

	if (subscript_bbox != NULL)
		gmathml_bbox_merge_vertically (&children_bbox, subscript_bbox, -script->subscript_offset);
	if (superscript_bbox != NULL)
		gmathml_bbox_merge_vertically (&children_bbox, superscript_bbox, script->superscript_offset);

	gmathml_bbox_add_horizontally (&element->bbox, &children_bbox);

	return &element->bbox;
}

static void
gmathml_script_element_layout (GMathmlElement *self, GMathmlView *view,
			       double x, double y, const GMathmlBbox *bbox)
{
	GMathmlScriptElement *script = GMATHML_SCRIPT_ELEMENT (self);
	const GMathmlBbox *base_bbox;
	double h_space;

	if (script->base == NULL)
		return;

	base_bbox = gmathml_element_get_bbox (script->base);

	gmathml_element_layout (script->base, view, x, y, base_bbox);

	h_space = self->style.math_size * GMATHML_SPACE_EM_VERY_THIN;
	x += gmathml_view_measure_length (view, h_space);

	if (script->subscript)
		gmathml_element_layout (script->subscript, view,
					x + base_bbox->width,
					y + script->subscript_offset,
					gmathml_element_get_bbox (script->subscript));
	if (script->superscript)
		gmathml_element_layout (script->superscript, view,
					x + base_bbox->width,
					y - script->superscript_offset,
					gmathml_element_get_bbox (script->superscript));
}

static const GMathmlOperatorElement *
gmathml_script_element_get_embellished_core (const GMathmlElement *self)
{
	if (GDOM_NODE (self)->first_child != NULL)
		return gmathml_element_get_embellished_core (GMATHML_ELEMENT (GDOM_NODE (self)->first_child));

	return NULL;
}

/* GMathmlScriptElement implementation */

GDomNode *
gmathml_sub_element_new (void)
{
	GDomNode *node;

	node = g_object_new (GMATHML_TYPE_SCRIPT_ELEMENT, NULL);
	g_return_val_if_fail (node != NULL, NULL);

	GMATHML_SCRIPT_ELEMENT (node)->type = GMATHML_SCRIPT_ELEMENT_TYPE_SUB;

	return node;
}

GDomNode *
gmathml_sup_element_new (void)
{
	GDomNode *node;

	node = g_object_new (GMATHML_TYPE_SCRIPT_ELEMENT, NULL);
	g_return_val_if_fail (node != NULL, NULL);

	GMATHML_SCRIPT_ELEMENT (node)->type = GMATHML_SCRIPT_ELEMENT_TYPE_SUP;

	return node;
}

GDomNode *
gmathml_sub_sup_element_new (void)
{
	GDomNode *node;

	node = g_object_new (GMATHML_TYPE_SCRIPT_ELEMENT, NULL);
	g_return_val_if_fail (node != NULL, NULL);

	GMATHML_SCRIPT_ELEMENT (node)->type = GMATHML_SCRIPT_ELEMENT_TYPE_SUB_SUP;

	return node;
}

static void
gmathml_script_element_init (GMathmlScriptElement *self)
{
}

/* GMathmlScriptElement class */

static void
gmathml_script_element_class_init (GMathmlScriptElementClass *script_class)
{
	GDomNodeClass *d_node_class = GDOM_NODE_CLASS (script_class);
	GMathmlElementClass *m_element_class = GMATHML_ELEMENT_CLASS (script_class);

	d_node_class->get_node_name = gmathml_script_element_get_node_name;
	d_node_class->can_append_child = gmathml_script_element_can_append_child;
	d_node_class->post_new_child = gmathml_script_element_post_new_child;

	m_element_class->update = gmathml_script_element_update;
	m_element_class->update_child = gmathml_script_element_update_child;
	m_element_class->measure = gmathml_script_element_measure;
	m_element_class->layout = gmathml_script_element_layout;
	m_element_class->get_embellished_core = gmathml_script_element_get_embellished_core;
	m_element_class->is_inferred_row = NULL;
}

G_DEFINE_TYPE (GMathmlScriptElement, gmathml_script_element, GMATHML_TYPE_ELEMENT)
