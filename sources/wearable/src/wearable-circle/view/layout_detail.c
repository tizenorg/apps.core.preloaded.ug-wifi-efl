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
#include "view/layout_detail.h"

struct _layout_detail_object {
	view_base_object *base;
	Elm_Object_Item *naviframe_item;
	Evas_Object *layout;
	Evas_Object *menu_list;
	Evas_Object *menu_list_circle;
	Evas_Object *forget_button;

	struct {
		Evas_Object_Event_Cb func;
		void *data;
	} del_cb;
	struct {
		Elm_Gen_Item_Text_Get_Cb text_get;
		Elm_Gen_Item_Content_Get_Cb content_get;
		Elm_Gen_Item_State_Get_Cb state_get;
		Elm_Gen_Item_Del_Cb del;
		void *data;
	} menu_cb[DETAIL_MENU_SIZE];
	struct {
		Evas_Smart_Cb func;
		void *data;
	} tap_forget_button_cb;
};

static inline gboolean _is_in_naviframe(layout_detail_object *self)
{
	return self->naviframe_item != NULL;
}

static Evas_Object *_create_layout(Evas_Object *parent)
{
	Evas_Object *layout = NULL;
	WIFI_RET_VAL_IF_FAIL(parent, NULL);

	layout = elm_layout_add(parent);
	WIFI_RET_VAL_IF_FAIL(layout, NULL);

	elm_layout_theme_set(layout, "layout", "bottom_button", "default");
	evas_object_show(layout);
	return layout;
}

static Evas_Object *_create_menu_list(layout_detail_object *self, Evas_Object *parent)
{
	Evas_Object *menu_list = NULL;

	menu_list = view_base_add_genlist_for_circle(self->base, parent, &(self->menu_list_circle));
	WIFI_RET_VAL_IF_FAIL(menu_list, NULL);
	evas_object_show(menu_list);

	return menu_list;
}

static Evas_Object *_create_forget_button(layout_detail_object *self, Evas_Object *parent)
{
	Evas_Object *button = elm_button_add(parent);
	elm_object_style_set(button, "bottom");
	elm_object_text_set(button, STR_FORGET_UPPER);
	evas_object_size_hint_weight_set(button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(button, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_propagate_events_set(button, EINA_FALSE);

	evas_object_smart_callback_add(button, "clicked",
				       self->tap_forget_button_cb.func, self->tap_forget_button_cb.data);
	return button;
}

static void _append_menu(layout_detail_object *self, layout_detail_menu_type type)
{
	Elm_Genlist_Item_Class *itc = NULL;
	Elm_Object_Item *menu_item = NULL;
	switch (type) {
	case DETAIL_MENU_TITLE:
		itc = create_genlist_itc("title",
					 self->menu_cb[type].text_get, NULL, NULL, NULL);
		break;

	case DETAIL_MENU_STATUS:
	case DETAIL_MENU_STRENGTH:
	case DETAIL_MENU_SPEED:
	case DETAIL_MENU_IP:
		itc = create_genlist_itc("2text",
					 self->menu_cb[type].text_get, NULL, NULL, NULL);
		break;

	default:
		return;
	}

	menu_item = elm_genlist_item_append(self->menu_list, itc,
					    self->menu_cb[type].data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	if (!menu_item) {
		WIFI_LOG_ERR("elm_genlist_item_append() is failed.");
		elm_genlist_item_class_free(itc);
		return;
	}

	elm_genlist_item_select_mode_set(menu_item,
					 ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	elm_genlist_item_class_free(itc);
}

gboolean _layout_detail_create(layout_detail_object *self, gboolean is_show_forgetbutton)
{
	WIFI_RET_VAL_IF_FAIL(self != NULL, FALSE);
	WIFI_RET_VAL_IF_FAIL(self->base != NULL, FALSE);

	self->layout = _create_layout(view_base_get_naviframe(self->base));
	WIFI_RET_VAL_IF_FAIL(self->layout != NULL, FALSE);

	self->menu_list = _create_menu_list(self, self->layout);
	if (!self->menu_list) {
		WIFI_LOG_ERR("_create_menu_list() is failed.");
		layout_detail_destroy(self);
		return FALSE;
	}
	_append_menu(self, DETAIL_MENU_TITLE);
	_append_menu(self, DETAIL_MENU_STATUS);
	_append_menu(self, DETAIL_MENU_STRENGTH);
	_append_menu(self, DETAIL_MENU_SPEED);
	_append_menu(self, DETAIL_MENU_IP);
	elm_object_part_content_set(self->layout,
				    "elm.swallow.content", self->menu_list);

	if (is_show_forgetbutton) {
		self->forget_button = _create_forget_button(self, self->layout);
		if (!self->forget_button) {
			WIFI_LOG_ERR("_create_forget_button() is failed.");
			layout_detail_destroy(self);
			return FALSE;
		}
		elm_object_part_content_set(self->layout,
					    "elm.swallow.button", self->forget_button);
	}

	return TRUE;
}

layout_detail_object *layout_detail_new(view_base_object *base)
{
	layout_detail_object *object = NULL;
	if (!base)
		return NULL;
	object = g_new0(layout_detail_object, 1);
	if (!object) {
		LOGE("layout_detail_new() failed.");
		return NULL;
	}
	object->base = base;
	return object;
}

void layout_detail_free(layout_detail_object *object)
{
	if (object)
		g_free(object);
}

gboolean layout_detail_create(layout_detail_object *self)
{
	return _layout_detail_create(self, TRUE);
}

gboolean layout_detail_create_hidden_forgetbutton(layout_detail_object *self)
{
	return _layout_detail_create(self, FALSE);
}

void layout_detail_destroy(layout_detail_object *self)
{
	WIFI_RET_IF_FAIL(self);
}

void layout_detail_show(layout_detail_object *self)
{
	WIFI_RET_IF_FAIL(self);
	WIFI_RET_IF_FAIL(!_is_in_naviframe(self));

    __WIFI_FUNC_ENTER__;

	self->naviframe_item = view_base_naviframe_push(self->base,
							self->layout, self->del_cb.func, self->del_cb.data);
	if (!self->naviframe_item) {
		WIFI_LOG_ERR("layout push to naviframe failed.");
	}
}

void layout_detail_activate_rotary_event(layout_detail_object *self)
{
	WIFI_RET_IF_FAIL(self);
}

void layout_detail_deactivate_rotary_event(layout_detail_object *self)
{
	WIFI_RET_IF_FAIL(self);
}

void layout_detail_set_del_cb(layout_detail_object *self,
			      Evas_Object_Event_Cb func, void *data)
{
	WIFI_RET_IF_FAIL(self);

	self->del_cb.func = func;
	self->del_cb.data = data;
}

void layout_detail_set_menu_cb(layout_detail_object *self, layout_detail_menu_type type,
			       Elm_Gen_Item_Text_Get_Cb text_get, Elm_Gen_Item_Content_Get_Cb content_get,
			       Elm_Gen_Item_State_Get_Cb state_get, Elm_Gen_Item_Del_Cb del, gpointer data)
{
	WIFI_RET_IF_FAIL(self);

	self->menu_cb[type].text_get = text_get;
	self->menu_cb[type].content_get = content_get;
	self->menu_cb[type].state_get = state_get;
	self->menu_cb[type].del = del;
	self->menu_cb[type].data = data;
}

void layout_detail_set_tap_forget_button_cb(layout_detail_object *self,
					    Evas_Smart_Cb func, gpointer data)
{
	WIFI_RET_IF_FAIL(self);

	self->tap_forget_button_cb.func = func;
	self->tap_forget_button_cb.data = data;
}
