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
#include "view/layout_proxy_setting.h"

struct _layout_proxy_setting_object {
	view_base_object *base;
	Elm_Object_Item *naviframe_item;
	Evas_Object *layout;
	Evas_Object *menu_list;
	Evas_Object *menu_list_circle;
	Elm_Object_Item *menu_item[PROXY_SETTING_ITEM_SIZE];
	Evas_Object *save_button;
	proxy_setting_menu_type selected_menu;

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
	} menu_cb[PROXY_SETTING_ITEM_SIZE];
	struct {
		Evas_Smart_Cb func;
		void *data;
	} tap_menu_cb[PROXY_SETTING_ITEM_SIZE];
	struct {
		Evas_Smart_Cb func;
		void *data;
	} tap_save_button_cb;
};

static inline gboolean _is_in_naviframe(layout_proxy_setting_object *self)
{
	return self->naviframe_item != NULL;
}

static Evas_Object *_create_layout(layout_proxy_setting_object *self, Evas_Object *parent)
{
	Evas_Object *layout = NULL;
	WIFI_RET_VAL_IF_FAIL(parent, NULL);

	layout = elm_layout_add(parent);
	WIFI_RET_VAL_IF_FAIL(layout, NULL);

	elm_layout_theme_set(layout, "layout", "bottom_button", "default");
	evas_object_show(layout);
	return layout;
}

static Evas_Object *_create_menu_list(layout_proxy_setting_object *self, Evas_Object *parent)
{
	Evas_Object *menu_list = NULL;

	menu_list = view_base_add_genlist_for_circle(self->base, parent, &(self->menu_list_circle));
	WIFI_RET_VAL_IF_FAIL(menu_list, NULL);
	evas_object_show(menu_list);

	return menu_list;
}

static Evas_Object *_create_save_button(layout_proxy_setting_object *self, Evas_Object *parent)
{
	Evas_Object *button = elm_button_add(parent);
	elm_object_style_set(button, "bottom");
	elm_object_text_set(button, STR_SAVE);
	evas_object_size_hint_weight_set(button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(button, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_propagate_events_set(button, EINA_FALSE);
	evas_object_smart_callback_add(button, "clicked",
				       self->tap_save_button_cb.func, self->tap_save_button_cb.data);
	return button;
}

static void _append_menu(layout_proxy_setting_object *self, proxy_setting_menu_type type)
{
	Elm_Genlist_Item_Class *itc = NULL;
	switch (type) {
	case PROXY_SETTING_ITEM_TITLE:
		itc = create_genlist_itc("title", self->menu_cb[type].text_get, NULL, NULL, NULL);
		break;

	case PROXY_SETTING_ITEM_ADDRESS:
	case PROXY_SETTING_ITEM_PORT:
		itc = create_genlist_itc("2text", self->menu_cb[type].text_get, NULL, NULL, NULL);
		break;

	default:
		return;
	}

	if (type == PROXY_SETTING_ITEM_TITLE) {
		self->menu_item[type] = elm_genlist_item_append(self->menu_list, itc,
								self->menu_cb[type].data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	} else {
		self->menu_item[type] = elm_genlist_item_append(self->menu_list, itc,
								self->menu_cb[type].data, NULL, ELM_GENLIST_ITEM_NONE,
								self->tap_menu_cb[type].func, self->tap_menu_cb[type].data);
	}
	elm_genlist_item_class_free(itc);
}

layout_proxy_setting_object *layout_proxy_setting_new(view_base_object *base)
{
	layout_proxy_setting_object *object = NULL;
	if (!base)
		return NULL;

	object = g_new0(layout_proxy_setting_object, 1);
	if (!object) {
		LOGE("layout_proxy_setting_new() failed.");
		return NULL;
	}
	object->base = base;
	return object;
}

void layout_proxy_setting_free(layout_proxy_setting_object *object)
{
	if (object)
		g_free(object);
}

gboolean layout_proxy_setting_create(layout_proxy_setting_object *self)
{
	__WIFI_FUNC_ENTER__;

	WIFI_RET_VAL_IF_FAIL(self, FALSE);
	WIFI_RET_VAL_IF_FAIL(self->base, FALSE);

	self->layout = _create_layout(self, view_base_get_naviframe(self->base));
	WIFI_RET_VAL_IF_FAIL(self->layout, FALSE);

	self->menu_list = _create_menu_list(self, self->layout);
	if (!self->menu_list) {
		WIFI_LOG_ERR("_create_menu_list() is failed.");
		layout_proxy_setting_destroy(self);
		return FALSE;
	}
	_append_menu(self, PROXY_SETTING_ITEM_TITLE);
	_append_menu(self, PROXY_SETTING_ITEM_ADDRESS);
	_append_menu(self, PROXY_SETTING_ITEM_PORT);
	elm_object_part_content_set(self->layout,
				    "elm.swallow.content", self->menu_list);

	self->save_button = _create_save_button(self, self->layout);
	if (!self->save_button) {
		WIFI_LOG_ERR("_create_save_button() is failed.");
		layout_proxy_setting_destroy(self);
		return FALSE;
	}
	elm_object_part_content_set(self->layout,
				    "elm.swallow.button", self->save_button);

	self->selected_menu = PROXY_SETTING_ITEM_NONE;

	return TRUE;
}

void layout_proxy_setting_destroy(layout_proxy_setting_object *self)
{
	WIFI_RET_IF_FAIL(self);
}

void layout_proxy_setting_show(layout_proxy_setting_object *self)
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

void layout_proxy_setting_pop(layout_proxy_setting_object *self)
{
	WIFI_RET_IF_FAIL(self);
	WIFI_RET_IF_FAIL(_is_in_naviframe(self));

	view_base_naviframe_item_pop(self->base);
}

void layout_proxy_setting_pop_to(layout_proxy_setting_object *self)
{
	WIFI_RET_IF_FAIL(self);

	view_base_naviframe_item_pop_to(self->base, self->naviframe_item);
}

void layout_proxy_setting_select_menu(layout_proxy_setting_object *self, proxy_setting_menu_type menu)
{
	WIFI_RET_IF_FAIL(self);

	self->selected_menu = menu;
}

proxy_setting_menu_type layout_proxy_setting_get_selected_menu(layout_proxy_setting_object *self)
{
	WIFI_RET_VAL_IF_FAIL(self, PROXY_SETTING_ITEM_NONE);

	return self->selected_menu;
}

void layout_proxy_setting_update_menu(layout_proxy_setting_object *self, proxy_setting_menu_type type)
{
	Elm_Object_Item *item = NULL;
	WIFI_RET_IF_FAIL(self);

	item = self->menu_item[type];
	WIFI_RET_IF_FAIL(item);

	elm_genlist_item_update(item);
}

proxy_setting_menu_type layout_proxy_setting_get_menu_type(layout_proxy_setting_object *self,
							   Elm_Object_Item *item)
{
	int index;
	WIFI_RET_VAL_IF_FAIL(self, PROXY_SETTING_ITEM_NONE);
	WIFI_RET_VAL_IF_FAIL(item, PROXY_SETTING_ITEM_NONE);

	for (index = 0; index < PROXY_SETTING_ITEM_SIZE; ++index) {
		if (self->menu_item[index] == item) {
			return index;
		}
	}
	return PROXY_SETTING_ITEM_NONE;
}

void layout_proxy_setting_activate_rotary_event(layout_proxy_setting_object *self)
{
	WIFI_RET_IF_FAIL(self);
	eext_rotary_object_event_activated_set(self->menu_list_circle, EINA_TRUE);
}

void layout_proxy_setting_deactivate_rotary_event(layout_proxy_setting_object *self)
{
	WIFI_RET_IF_FAIL(self);
	eext_rotary_object_event_activated_set(self->menu_list_circle, EINA_FALSE);
}

void layout_proxy_setting_set_del_cb(layout_proxy_setting_object *self,
				     Evas_Object_Event_Cb func, void *data)
{
	WIFI_RET_IF_FAIL(self);

	self->del_cb.func = func;
	self->del_cb.data = data;
}

const gchar *layout_proxy_setting_get_main_text(layout_proxy_setting_object *self,
						proxy_setting_menu_type type)
{
	WIFI_RET_VAL_IF_FAIL(self, NULL);
	WIFI_RET_VAL_IF_FAIL(type != PROXY_SETTING_ITEM_NONE, NULL);
	WIFI_RET_VAL_IF_FAIL(type != PROXY_SETTING_ITEM_TITLE, NULL);

	return elm_object_item_part_text_get(self->menu_item[type], "elm.text");
}

void layout_proxy_setting_set_menu_cb(layout_proxy_setting_object *self, proxy_setting_menu_type type,
				      Elm_Gen_Item_Text_Get_Cb text_get, Elm_Gen_Item_Content_Get_Cb content_get,
				      Elm_Gen_Item_State_Get_Cb state_get, Elm_Gen_Item_Del_Cb del, void *data)
{
	WIFI_RET_IF_FAIL(self);

	self->menu_cb[type].text_get = text_get;
	self->menu_cb[type].content_get = content_get;
	self->menu_cb[type].state_get = state_get;
	self->menu_cb[type].del = del;
	self->menu_cb[type].data = data;
}

void layout_proxy_setting_set_tap_menu_cb(layout_proxy_setting_object *self,
					  proxy_setting_menu_type type, Evas_Smart_Cb func, gpointer data)
{
	WIFI_RET_IF_FAIL(self);

	self->tap_menu_cb[type].func = func;
	self->tap_menu_cb[type].data = data;
}

void layout_proxy_setting_set_tap_save_button_cb(layout_proxy_setting_object *self,
						 Evas_Smart_Cb func, gpointer data)
{
	WIFI_RET_IF_FAIL(self);

	self->tap_save_button_cb.func = func;
	self->tap_save_button_cb.data = data;
}
