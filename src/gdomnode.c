/* gdomnode.c
 *
 * Copyright (C) 2007  Emmanuel Pacaud
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

#include <gdomnode.h>
#include <gdomdocument.h>

char*
gdom_node_get_node_name (GDomNode* self)
{
	GDomNodeClass *node_class = GDOM_NODE_GET_CLASS (self);

	g_return_val_if_fail (node_class != NULL, NULL);

	if (node_class->get_node_name)
		return node_class->get_node_name (self);

	return NULL;
}

char*
gdom_node_get_node_value (GDomNode* self)
{
	GDomNodeClass *node_class = GDOM_NODE_GET_CLASS (self);

	g_return_val_if_fail (node_class != NULL, NULL);

	if (node_class->get_node_value)
		return node_class->get_node_value (self);

	return NULL;
}

void
gdom_node_set_node_value (GDomNode* self, const char* new_value)
{
	GDomNodeClass *node_class = GDOM_NODE_GET_CLASS (self);

	g_return_if_fail (node_class != NULL);
	g_return_if_fail (new_value != NULL);

	if (node_class->set_node_value)
		node_class->set_node_value (self, new_value);
}

GDomNodeType gdom_node_get_node_type (GDomNode* self)
{
	GDomNodeClass *node_class = GDOM_NODE_GET_CLASS (self);

	g_return_val_if_fail (node_class != NULL, 0);

	if (node_class->get_node_type)
		return node_class->get_node_type (self);

	return 0;
}

GDomNode*
gdom_node_get_parent_node (GDomNode* self)
{
	g_return_val_if_fail (GDOM_IS_NODE (self), NULL);

	return self->parent_node;
}

/*GDomNodeList**/
/*gdom_node_get_child_nodes (GDomNode* self)*/
/*{*/
/*        return GDOM_NODE_GET_CLASS (self)->get_child_nodes (self);*/
/*}*/

GDomNode*
gdom_node_get_first_child (GDomNode* self)
{
	g_return_val_if_fail (GDOM_IS_NODE (self), NULL);

	return self->first_child;
}

GDomNode*
gdom_node_get_last_child (GDomNode* self)
{
	g_return_val_if_fail (GDOM_IS_NODE (self), NULL);

	return self->last_child;
}

GDomNode*
gdom_node_get_previous_sibling (GDomNode* self)
{
	g_return_val_if_fail (GDOM_IS_NODE (self), NULL);

	return self->previous_sibling;
}

GDomNode*
gdom_node_get_next_sibling (GDomNode* self)
{
	g_return_val_if_fail (GDOM_IS_NODE (self), NULL);

	return self->next_sibling;
}

/*GDomNamedNodeMap**/
/*gdom_node_get_attributes (GDomNode* self)*/
/*{*/
/*        return GDOM_NODE_GET_CLASS (self)->get_attributes (self);*/
/*}*/

GDomDocument*
gdom_node_get_owner_document (GDomNode* self)
{
	GDomNode *parent;

	g_return_val_if_fail (GDOM_IS_NODE (self), NULL);

	for (parent = self;
	     parent != NULL &&
	     !GDOM_IS_DOCUMENT (parent);
	     parent = parent->parent_node);

	return GDOM_DOCUMENT (parent);
}

GDomNode*
gdom_node_insert_before (GDomNode* self, GDomNode* new_child, GDomNode* ref_child)
{
	g_warning ("Not yet implemented");

	return NULL;
}

GDomNode*
gdom_node_replace_child (GDomNode* self, GDomNode* new_child, GDomNode* old_child)
{
	g_warning ("Not yet implemented");

	return NULL;
}

GDomNode*
gdom_node_remove_child (GDomNode* self, GDomNode* old_child)
{
	GDomNode *node;

	g_return_val_if_fail (GDOM_IS_NODE (self), NULL);
	g_return_val_if_fail (GDOM_IS_NODE (old_child), NULL);

	for (node = self->first_child;
	     node != NULL && node != old_child;
	     node = node->next_sibling);

	g_return_val_if_fail (node != NULL, NULL);

	if (self->first_child == old_child)
		self->first_child = old_child->next_sibling;
	if (self->last_child == old_child)
		self->last_child = old_child->previous_sibling;

	if (old_child->next_sibling != NULL)
		old_child->next_sibling->previous_sibling = old_child->previous_sibling;
	if (old_child->previous_sibling != NULL)
		old_child->previous_sibling->next_sibling = old_child->next_sibling;

	old_child->parent_node = NULL;
	old_child->next_sibling = NULL;
	old_child->previous_sibling = NULL;

	return old_child;
}

GDomNode*
gdom_node_append_child (GDomNode* self, GDomNode* new_child)
{
	g_return_val_if_fail (GDOM_IS_NODE (self), NULL);
	g_return_val_if_fail (GDOM_NODE_GET_CLASS (self)->can_append_child (self, new_child), NULL);
	g_return_val_if_fail (new_child->parent_node == NULL, NULL);

	if (self->first_child == NULL)
		self->first_child = new_child;
	if (self->last_child != NULL)
		self->last_child->next_sibling = new_child;

	new_child->parent_node = self;
	new_child->next_sibling = NULL;
	new_child->previous_sibling = self->last_child;
	self->last_child = new_child;

	return new_child;
}

static gboolean
gdom_node_can_append_child (GDomNode *self, GDomNode* new_child)
{
	return FALSE;
}

gboolean
gdom_node_has_child_nodes (GDomNode* self)
{
	return GDOM_NODE_GET_CLASS (self)->has_child_nodes (self);
}

void
gdom_node_dump (GDomNode *self)
{
	GDomNode *node;
	GDomNodeType type;
	char *text;

	g_return_if_fail (GDOM_IS_NODE (self));

	type = gdom_node_get_node_type (self);

	switch (type) {
		case GDOM_NODE_TYPE_ELEMENT_NODE:
			g_print ("<%s>", gdom_node_get_node_name (self));
			for (node = self->first_child;
			     node != NULL;
			     node = node->next_sibling)
				gdom_node_dump (node);
			g_print ("</%s>", gdom_node_get_node_name (self));
			break;
		case GDOM_NODE_TYPE_TEXT_NODE:
			text = gdom_node_get_node_value (self);
			g_print ("%s", text != NULL ? text : "null");
			break;
		case GDOM_NODE_TYPE_DOCUMENT_NODE:
			g_print ("Mathml Document\n");
			if (self->first_child != NULL) {
				gdom_node_dump (self->first_child);
				g_print ("\n");
			}
			break;
		default:
			g_print ("Not supported\n");
			break;
	}
}

static void
gdom_node_init (GDomNode *node)
{
}

static void
gdom_node_class_init (GDomNodeClass *klass)
{
	klass->can_append_child = gdom_node_can_append_child;
}

G_DEFINE_ABSTRACT_TYPE (GDomNode, gdom_node, G_TYPE_OBJECT)
