/*
 * Copyright (c) 2014 - 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * PROPRIETARY/CONFIDENTIAL
 *
 * This software is the confidential and proprietary information of
 * SAMSUNG ELECTRONICS (Confidential Information).
 * You shall not disclose such Confidential Information and shall
 * use it only in accordance with the terms of the license agreement
 * you entered into with SAMSUNG ELECTRONICS.  SAMSUNG make no
 * representations or warranties about the suitability
 * of the software, either express or implied, including but not
 * limited to the implied warranties of merchantability, fitness for a particular purpose, or non-
 * infringement. SAMSUNG shall not be liable for any damages suffered by licensee as
 * a result of using, modifying or distributing this software or its derivatives.
 */


#include <app.h>
#include <dlog.h>
#include <Elementary.h>
#include <Evas.h>
#include <efl_assist.h>
#include <efl_extension.h>
#include <glib.h>

#include "util.h"
#include "view/util/efl_helper.h"
#include "view/base.h"
#include "view/layout_scan.h"

typedef enum {
	SCAN_VIEW_UNKNOWN,
	SCAN_VIEW_NO_AP,
	SCAN_VIEW_AP_LIST
} scan_view_state;

struct _layout_scan_object {
	view_base_object *base;
	scan_view_state state;
	Elm_Object_Item *naviframe_item;
	Evas_Object *layout;
	Evas_Object *scan_button;

	Evas_Object *no_ap;

	Evas_Object *ap_list;
	Evas_Object *ap_list_circle;
	GList *ap_data_list;
	GDestroyNotify ap_data_del_cb;
	struct {
		Evas_Object_Event_Cb func;
		void *data;
	} del_cb;
	struct {
		Elm_Gen_Item_Text_Get_Cb text_get;
		Elm_Gen_Item_Content_Get_Cb content_get;
		Elm_Gen_Item_State_Get_Cb state_get;
		Elm_Gen_Item_Del_Cb del;
		Evas_Smart_Cb tap;
		void *data;
	} menu_cb[SCAN_MENU_SIZE];
	struct {
		Evas_Smart_Cb func;
		void *data;
	} scan_button_tap_cb;
};

static inline gboolean _is_in_naviframe(layout_scan_object *self)
{
	return self->naviframe_item != NULL;
}

static char *__title_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	return g_strdup(STR_WIFI_NETWORKS_HEADER);
}

static Evas_Object *_create_layout(Evas_Object *parent)
{
	Evas_Object *layout = NULL;
	WIFI_RET_VAL_IF_FAIL(parent, NULL);

	layout = elm_layout_add(parent);
	WIFI_RET_VAL_IF_FAIL(layout, NULL);

	elm_layout_theme_set(layout, "layout", "bottom_button", "default");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_show(layout);
	return layout;
}

static Evas_Object *_create_no_ap(layout_scan_object *self, Evas_Object *parent)
{
	Evas_Object *no_ap = elm_layout_add(parent);
	if (!no_ap) {
		WIFI_LOG_ERR("no_ap create is failed.");
		return NULL;
	}
	elm_layout_theme_set(no_ap, "layout", "popup", "content/circle");
	evas_object_size_hint_weight_set(no_ap, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(no_ap, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_domain_translatable_part_text_set(no_ap, "elm.text.title", NULL,
						     "WDS_WIFI_HEADER_WI_FI_NETWORKS_ABB");
	elm_object_domain_translatable_part_text_set(no_ap, "elm.text", NULL,
						     "WDS_WIFI_NPBODY_NO_WI_FI_ACCESS_POINT_FOUND");

	return no_ap;
}

static Evas_Object *_create_ap_list(layout_scan_object *self, Evas_Object *parent)
{
	Evas_Object *ap_list = view_base_add_genlist_for_circle(self->base, parent, &(self->ap_list_circle));
	if (!ap_list) {
		WIFI_LOG_ERR("scan_list create is failed.");
		return NULL;
	}
	evas_object_show(ap_list);
	return ap_list;
}

static Evas_Object *_create_scan_button(layout_scan_object *self, Evas_Object *parent)
{
	Evas_Object *scan_button = elm_button_add(parent);
	WIFI_RET_VAL_IF_FAIL(scan_button, NULL);

	elm_object_style_set(scan_button, "bottom");
	elm_object_text_set(scan_button, STR_SCAN);
	evas_object_size_hint_weight_set(scan_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(scan_button, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_propagate_events_set(scan_button, EINA_FALSE);

	evas_object_smart_callback_add(scan_button, "clicked",
				       self->scan_button_tap_cb.func, self->scan_button_tap_cb.data);

	return scan_button;
}

static void _layout_content_change(layout_scan_object *self, scan_view_state state)
{
	Evas_Object *layout_content;
	Evas_Object *old_layout_content;

	WIFI_RET_IF_FAIL(self != NULL);
	WIFI_RET_IF_FAIL(self->state != state);

	switch (state) {
	case SCAN_VIEW_NO_AP:
		layout_content = self->no_ap;
		break;

	case SCAN_VIEW_AP_LIST:
		layout_content = self->ap_list;
		break;

	default:
		return;
	}

	old_layout_content =
		elm_object_part_content_unset(self->layout, "elm.swallow.content");
	evas_object_hide(old_layout_content);
	elm_object_part_content_set(self->layout,
				    "elm.swallow.content", layout_content);

	self->state = state;
}

static Elm_Object_Item *_get_item_from_ap_list_by_data(layout_scan_object *self, gpointer data, GCompareFunc cmp)
{
	Elm_Object_Item *ap_item;
	WIFI_RET_VAL_IF_FAIL(self != NULL, NULL);
	WIFI_RET_VAL_IF_FAIL(self->ap_list != NULL, NULL);

	ap_item = elm_genlist_nth_item_get(self->ap_list, 1);
	while (ap_item) {
		gpointer item_data = elm_object_item_data_get(ap_item);
		if (cmp(item_data, data) == 0) {
			return ap_item;
		}
		ap_item = elm_genlist_item_next_get(ap_item);
	}
	return ap_item;
}

static gboolean _is_ap_list_exists(layout_scan_object *self)
{
	return self != NULL &&
	       self->ap_list != NULL &&
	       elm_genlist_items_count(self->ap_list) > 0;
}

static gboolean _ap_list_append_title(layout_scan_object *self)
{
	Elm_Genlist_Item_Class *title_itc = create_genlist_itc("type1",
							       __title_text_get_cb, NULL,
							       NULL, NULL);
	WIFI_RET_VAL_IF_FAIL(title_itc != NULL, FALSE);

	elm_genlist_clear(self->ap_list);
	elm_genlist_item_append(self->ap_list, title_itc, NULL, NULL,
				ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_class_free(title_itc);
	return TRUE;
}

static inline Elm_Genlist_Item_Class *_create_wifi_ap_itc(layout_scan_object *self)
{
	return create_genlist_itc("type1",
				  self->menu_cb[SCAN_MENU_WIFI_AP_ITEM].text_get,
				  self->menu_cb[SCAN_MENU_WIFI_AP_ITEM].content_get,
				  self->menu_cb[SCAN_MENU_WIFI_AP_ITEM].state_get,
				  self->menu_cb[SCAN_MENU_WIFI_AP_ITEM].del);
}

static void _ap_list_append_ap_item(layout_scan_object *self,
				    Elm_Genlist_Item_Class *itc, gpointer data)
{
	Elm_Object_Item *item = elm_genlist_item_append(self->ap_list, itc,
							data, NULL, ELM_GENLIST_ITEM_NONE,
							self->menu_cb[SCAN_MENU_WIFI_AP_ITEM].tap,
							self->menu_cb[SCAN_MENU_WIFI_AP_ITEM].data);
	Evas_Object *obj = elm_object_item_widget_get(item);
	if (obj) {
		evas_object_data_set(obj, LAYOUT_SCAN_DATA_KEY_WIFI_AP_ITEM_SELECT,
				     self->menu_cb[SCAN_MENU_WIFI_AP_ITEM].data);
	}
}

static gboolean _ap_list_append_ap_items(layout_scan_object *self)
{
	GList *l = self->ap_data_list;
	Elm_Genlist_Item_Class *wifi_ap_itc = _create_wifi_ap_itc(self);

	__WIFI_FUNC_ENTER__;
	WIFI_RET_VAL_IF_FAIL(wifi_ap_itc != NULL, FALSE);

	while (l) {
		_ap_list_append_ap_item(self, wifi_ap_itc, l->data);
		l = l->next;
	}
	elm_genlist_item_class_free(wifi_ap_itc);
	return TRUE;
}

static gboolean _ap_list_item_data_change(layout_scan_object *self,
					  guint index, gpointer data)
{
	Elm_Object_Item *ap_item = elm_genlist_nth_item_get(self->ap_list, index);
	WIFI_RET_VAL_IF_FAIL(ap_item != NULL, FALSE);

	elm_object_item_data_set(ap_item, data);
	elm_genlist_item_update(ap_item);
	return TRUE;
}

static gboolean _ap_list_update_ap_items(layout_scan_object *self)
{
	GList *l = self->ap_data_list;
	guint index, old_items_count = elm_genlist_items_count(self->ap_list) - 1;
	guint new_items_count = g_list_length(self->ap_data_list);
	Elm_Genlist_Item_Class *wifi_ap_itc = _create_wifi_ap_itc(self);

	__WIFI_FUNC_ENTER__;
	WIFI_RET_VAL_IF_FAIL(wifi_ap_itc != NULL, FALSE);

	if (new_items_count < old_items_count) {
		guint rm_index;
		for (rm_index = old_items_count; rm_index > new_items_count; rm_index--) {
			elm_object_item_del(elm_genlist_nth_item_get(self->ap_list, rm_index));
		}
	}
	for (index = 1; l != NULL; index++, l = l->next) {
		if (index <= old_items_count) {
			if (!_ap_list_item_data_change(self, index, l->data)) {
				WIFI_LOG_ERR("ap list item change failed.");
			}
		} else {
			_ap_list_append_ap_item(self, wifi_ap_itc, l->data);
		}
	}
	elm_genlist_item_class_free(wifi_ap_itc);

	WIFI_LOG_INFO("ap list update items count %d.", elm_genlist_items_count(self->ap_list) - 1);
	return TRUE;
}

layout_scan_object *layout_scan_new(view_base_object *base)
{
	layout_scan_object *object = NULL;
	if (!base)
		return NULL;
	object = g_new0(layout_scan_object, 1);
	if (!object) {
		LOGE("layout_scan_new() failed.");
		return NULL;
	}
	object->base = base;
	return object;
}

void layout_scan_free(layout_scan_object *object)
{
	if (object)
		g_free(object);
}

gboolean layout_scan_create(layout_scan_object *self)
{
	Evas_Object *naviframe = NULL;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_VAL_IF_FAIL(self, FALSE);
	WIFI_RET_VAL_IF_FAIL(self->base, FALSE);

	naviframe = view_base_get_naviframe(self->base);
	WIFI_RET_VAL_IF_FAIL(naviframe, FALSE);

	self->layout = _create_layout(naviframe);
	WIFI_RET_VAL_IF_FAIL(self->layout, FALSE);

	self->scan_button = _create_scan_button(self, self->layout);
	if (!self->scan_button) {
		layout_scan_destroy(self);
		return FALSE;
	}
	elm_object_part_content_set(self->layout,
				    "elm.swallow.button", self->scan_button);
	self->no_ap = _create_no_ap(self, self->layout);
	if (!self->no_ap) {
		layout_scan_destroy(self);
		return FALSE;
	}
	self->ap_list = _create_ap_list(self, self->layout);
	if (!self->ap_list) {
		layout_scan_destroy(self);
		return FALSE;
	}

	layout_scan_ap_list_update(self);

	return TRUE;
}

void layout_scan_destroy(layout_scan_object *self)
{
	WIFI_RET_IF_FAIL(self);
	if (self->ap_data_list) {
		layout_scan_ap_list_clear_data(self);
		self->ap_data_list = NULL;
	}
}

void layout_scan_pop_to(layout_scan_object *self)
{
	WIFI_RET_IF_FAIL(self);

	view_base_naviframe_item_pop_to(self->base, self->naviframe_item);
}

gboolean layout_scan_is_top(layout_scan_object *self)
{
	Evas_Object *naviframe;
	WIFI_RET_VAL_IF_FAIL(self != NULL, FALSE);

	naviframe = view_base_get_naviframe(self->base);
	WIFI_RET_VAL_IF_FAIL(naviframe != NULL, FALSE);

	return elm_naviframe_top_item_get(naviframe) == self->naviframe_item;
}

void layout_scan_no_ap_show(layout_scan_object *self)
{
	WIFI_RET_IF_FAIL(self);

    __WIFI_FUNC_ENTER__;

	_layout_content_change(self, SCAN_VIEW_NO_AP);

	WIFI_RET_IF_FAIL(!_is_in_naviframe(self));
	self->naviframe_item = view_base_naviframe_push(self->base,
							self->layout, self->del_cb.func, self->del_cb.data);
	if (!self->naviframe_item) {
		WIFI_LOG_ERR("layout push to naviframe failed.");
	}
}

void layout_scan_ap_list_show(layout_scan_object *self)
{
	WIFI_RET_IF_FAIL(self);

	layout_scan_ap_list_update(self);

	_layout_content_change(self, SCAN_VIEW_AP_LIST);

    __WIFI_FUNC_ENTER__;

	WIFI_RET_IF_FAIL(!_is_in_naviframe(self));
	self->naviframe_item = view_base_naviframe_push(self->base,
							self->layout, self->del_cb.func, self->del_cb.data);
	if (!self->naviframe_item) {
		WIFI_LOG_ERR("layout push to naviframe failed.");
	}
}

void layout_scan_ap_list_item_move_to_top(layout_scan_object *self,
					  Elm_Object_Item *item)
{
	Elm_Genlist_Item_Class *wifi_ap_itc;
	Elm_Object_Item *first_item;
	Elm_Object_Item *new_item;
	Evas_Object *widget_obj;
	gpointer data;
	WIFI_RET_IF_FAIL(self != NULL);
	WIFI_RET_IF_FAIL(item != NULL);

	first_item = elm_genlist_nth_item_get(self->ap_list, 1);
	if (first_item == item) {
		WIFI_LOG_INFO("Top item skip.");
		return;
	}

	wifi_ap_itc = _create_wifi_ap_itc(self);
	WIFI_RET_IF_FAIL(wifi_ap_itc != NULL);

	data = elm_object_item_data_get(item);
	elm_object_item_del(item);

	new_item = elm_genlist_item_insert_after(self->ap_list,
						 wifi_ap_itc, data, NULL,
						 elm_genlist_first_item_get(self->ap_list),
						 ELM_GENLIST_ITEM_NONE,
						 self->menu_cb[SCAN_MENU_WIFI_AP_ITEM].tap,
						 self->menu_cb[SCAN_MENU_WIFI_AP_ITEM].data);
	widget_obj = elm_object_item_widget_get(new_item);
	if (widget_obj) {
		evas_object_data_set(widget_obj,
				     LAYOUT_SCAN_DATA_KEY_WIFI_AP_ITEM_SELECT,
				     self->menu_cb[SCAN_MENU_WIFI_AP_ITEM].data);
	}

	elm_genlist_item_class_free(wifi_ap_itc);
}

void layout_scan_ap_list_top_item_show(layout_scan_object *self)
{
	Elm_Object_Item *first_item;
	WIFI_RET_IF_FAIL(self != NULL);
	WIFI_RET_IF_FAIL(self->ap_list != NULL);

	first_item = elm_genlist_nth_item_get(self->ap_list, 1);
	WIFI_RET_IF_FAIL(first_item != NULL);

	elm_genlist_item_show(first_item, ELM_GENLIST_ITEM_SCROLLTO_MIDDLE);
}

void layout_scan_ap_list_update(layout_scan_object *self)
{
	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(self != NULL);

	if (_is_ap_list_exists(self)) {
		WIFI_RET_IF_FAIL(_ap_list_update_ap_items(self));
	} else {
		WIFI_RET_IF_FAIL(_ap_list_append_title(self));
		WIFI_RET_IF_FAIL(_ap_list_append_ap_items(self));
	}
}

void layout_scan_ap_list_update_item_by_data(layout_scan_object *self, gpointer data, GCompareFunc cmp)
{
	Elm_Object_Item *ap_item;
	WIFI_RET_IF_FAIL(self);

	ap_item = _get_item_from_ap_list_by_data(self, data, cmp);
	WIFI_RET_IF_FAIL(ap_item);

	elm_genlist_item_update(ap_item);
}

Elm_Object_Item *layout_scan_ap_list_find_item_by_data(layout_scan_object *self, gpointer data, GCompareFunc cmp)
{
	WIFI_RET_VAL_IF_FAIL(self != NULL, NULL);

	return _get_item_from_ap_list_by_data(self, data, cmp);
}

gboolean layout_scan_ap_list_is_realized_item(layout_scan_object *self,
					      Elm_Object_Item *item)
{
	Eina_List *realized_list, *l = NULL;
	Elm_Object_Item *it = NULL;
	gboolean is_found = FALSE;
	WIFI_RET_VAL_IF_FAIL(self != NULL, FALSE);
	WIFI_RET_VAL_IF_FAIL(item != NULL, FALSE);

	realized_list = elm_genlist_realized_items_get(self->ap_list);
	EINA_LIST_FOREACH(realized_list, l, it)
	{
		if (it == item) {
			is_found = TRUE;
			break;
		}
	}
	eina_list_free(realized_list);
	return is_found;
}

void layout_scan_ap_list_set_data_list(layout_scan_object *self, GList *list)
{
	WIFI_RET_IF_FAIL(self);

	self->ap_data_list = list;
}

void layout_scan_ap_list_sort_data_list(layout_scan_object *self, GCompareFunc cmp)
{
	WIFI_RET_IF_FAIL(self != NULL);
	WIFI_RET_IF_FAIL(self->ap_data_list != NULL);
	WIFI_RET_IF_FAIL(cmp != NULL);

	self->ap_data_list = g_list_sort(self->ap_data_list, cmp);
}

void layout_scan_ap_list_append_data(layout_scan_object *self, gpointer data)
{
	WIFI_RET_IF_FAIL(self);
	WIFI_RET_IF_FAIL(data);

	self->ap_data_list = g_list_append(self->ap_data_list, data);
}

void layout_scan_ap_list_clear_data(layout_scan_object *self)
{
	WIFI_RET_IF_FAIL(self != NULL);
	WIFI_RET_IF_FAIL(self->ap_data_list != NULL);

	if (self->ap_data_del_cb) {
		g_list_free_full(self->ap_data_list, self->ap_data_del_cb);
	} else {
		g_list_free_full(self->ap_data_list, g_free);
	}
	self->ap_data_list = NULL;
}

void layout_scan_ap_list_activate_rotary_event(layout_scan_object *self)
{
	WIFI_RET_IF_FAIL(self);
}

void layout_scan_ap_list_deactivate_rotary_event(layout_scan_object *self)
{
	WIFI_RET_IF_FAIL(self);
}

void layout_scan_set_del_cb(layout_scan_object *self,
			    Evas_Object_Event_Cb func, void *data)
{
	WIFI_RET_IF_FAIL(self);

	self->del_cb.func = func;
	self->del_cb.data = data;
}

void layout_scan_set_ap_data_del_cb(layout_scan_object *self, GDestroyNotify func)
{
	WIFI_RET_IF_FAIL(self);

	self->ap_data_del_cb = func;
}

void layout_scan_set_menu_cb(layout_scan_object *self, scan_menu_type type,
			     Elm_Gen_Item_Text_Get_Cb text_get, Elm_Gen_Item_Content_Get_Cb content_get,
			     Elm_Gen_Item_State_Get_Cb state_get, Elm_Gen_Item_Del_Cb del,
			     Evas_Smart_Cb tap, void *data)
{
	WIFI_RET_IF_FAIL(self);
	WIFI_RET_IF_FAIL(type > SCAN_MENU_TITLE);
	WIFI_RET_IF_FAIL(type < SCAN_MENU_SIZE);

	self->menu_cb[type].text_get = text_get;
	self->menu_cb[type].content_get = content_get;
	self->menu_cb[type].state_get = state_get;
	self->menu_cb[type].del = del;
	self->menu_cb[type].tap = tap;
	self->menu_cb[type].data = data;
}

void layout_scan_set_scan_button_tap_cb(layout_scan_object *self,
					Evas_Smart_Cb func, void *data)
{
	WIFI_RET_IF_FAIL(self);

	self->scan_button_tap_cb.func = func;
	self->scan_button_tap_cb.data = data;
}
