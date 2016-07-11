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
#include "view/layout_main.h"

struct _layout_main_object {
	view_base_object *base;
	Elm_Object_Item *naviframe_item;
	Evas_Object *menu_list;
	Evas_Object *menu_list_circle;
	Elm_Object_Item *menu_item[MAIN_MENU_SIZE];

	struct {
		Evas_Object_Event_Cb func;
		void *data;
	} del_cb;
	struct {
		Elm_Gen_Item_Text_Get_Cb text_get;
		Elm_Gen_Item_Content_Get_Cb content_get;
		Evas_Smart_Cb tap;
		void *data;
	} menu_cb[MAIN_MENU_SIZE];
};

static inline gboolean _is_in_naviframe(layout_main_object *self)
{
	return self->naviframe_item != NULL;
}

static char *__title_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	return g_strdup(STR_WIFI);
}

static gboolean _create_menu_itc_array(layout_main_object *obj,
				       Elm_Genlist_Item_Class *menu_item_array[])
{
	menu_item_array[MAIN_MENU_POWER] = create_genlist_itc("1text.1icon.1",
							      obj->menu_cb[MAIN_MENU_POWER].text_get,
							      obj->menu_cb[MAIN_MENU_POWER].content_get,
							      NULL, NULL);
	WIFI_RET_VAL_IF_FAIL(menu_item_array[MAIN_MENU_POWER], FALSE);

	menu_item_array[MAIN_MENU_SCAN] = create_genlist_itc("2text",
							     obj->menu_cb[MAIN_MENU_SCAN].text_get,
							     obj->menu_cb[MAIN_MENU_SCAN].content_get,
							     NULL, NULL);
	if (!menu_item_array[MAIN_MENU_SCAN]) {
		elm_genlist_item_class_free(menu_item_array[MAIN_MENU_POWER]);
	}

	menu_item_array[MAIN_MENU_EMPTY] = create_genlist_itc("padding", NULL, NULL, NULL, NULL);
	if (!menu_item_array[MAIN_MENU_EMPTY]) {
		elm_genlist_item_class_free(menu_item_array[MAIN_MENU_EMPTY]);
	}

	return TRUE;
}

static Evas_Object *_create_initialized_menu_list(layout_main_object *self, Evas_Object *parent)
{
	Evas_Object *menu_list = NULL;
	Elm_Genlist_Item_Class *menu_title_itc = NULL;
	Elm_Genlist_Item_Class *menu_itc_array[MAIN_MENU_SIZE];
	int menu_index;

	menu_list = view_base_add_genlist_for_circle(self->base, parent, &(self->menu_list_circle));
	if (!menu_list) {
		WIFI_LOG_ERR("_create_menu_list() is failed.");
		return NULL;
	}
	evas_object_show(menu_list);

	menu_title_itc = create_genlist_itc("title",
					    __title_text_get_cb, NULL, NULL, NULL);
	if (!menu_title_itc) {
		WIFI_LOG_ERR("menu_title_itc create failed.");
		evas_object_del(menu_list);
		return NULL;
	}
	if (!_create_menu_itc_array(self, menu_itc_array)) {
		WIFI_LOG_ERR("_create_menu_item_array() is failed.");
		evas_object_del(menu_list);
		elm_genlist_item_class_free(menu_title_itc);
		return NULL;
	}

	elm_genlist_item_append(menu_list, menu_title_itc,
				NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_class_free(menu_title_itc);
	for (menu_index = 0; menu_index < MAIN_MENU_SIZE; ++menu_index) {
		self->menu_item[menu_index] = elm_genlist_item_append(menu_list,
								      menu_itc_array[menu_index], self->menu_cb[menu_index].data,
								      NULL, ELM_GENLIST_ITEM_NONE,
								      self->menu_cb[menu_index].tap, self->menu_cb[menu_index].data);
		elm_genlist_item_class_free(menu_itc_array[menu_index]);
	}
	return menu_list;
}

layout_main_object *layout_main_new(view_base_object *base)
{
	layout_main_object *object = NULL;
	if (!base)
		return NULL;
	object = g_new0(layout_main_object, 1);
	if (!object) {
		LOGE("layout_main_new() failed.");
		return NULL;
	}
	object->base = base;
	return object;
}

void layout_main_free(layout_main_object *object)
{
	if (object)
		g_free(object);
}

gboolean layout_main_create(layout_main_object *self)
{
	Evas_Object *naviframe = NULL;
	WIFI_RET_VAL_IF_FAIL(self, FALSE);
	WIFI_RET_VAL_IF_FAIL(self->base, FALSE);

	naviframe = view_base_get_naviframe(self->base);
	WIFI_RET_VAL_IF_FAIL(naviframe, FALSE);

	self->menu_list = _create_initialized_menu_list(self, naviframe);
	WIFI_RET_VAL_IF_FAIL(self->menu_list, FALSE);

	return TRUE;
}

void layout_main_destroy(layout_main_object *self)
{
	WIFI_RET_IF_FAIL(self);
}

void layout_main_pop_to(layout_main_object *self)
{
	WIFI_RET_IF_FAIL(self);

    WIFI_LOG_INFO("base naviframe item pop");
	view_base_naviframe_item_pop_to(self->base, self->naviframe_item);
}

void layout_main_show(layout_main_object *self)
{
	WIFI_RET_IF_FAIL(self);
	WIFI_RET_IF_FAIL(!_is_in_naviframe(self));

    __WIFI_FUNC_ENTER__;

	self->naviframe_item = view_base_naviframe_push(self->base,
							self->menu_list, self->del_cb.func, self->del_cb.data);
	if (!self->naviframe_item) {
		WIFI_LOG_ERR("layout push to naviframe failed.");
	}
}

void layout_main_menu_set_access_info(layout_main_object *self, main_menu_type menu_type,
				      Elm_Access_Info_Type access_type, const gchar *text)
{
	Evas_Object *access_obj;
	WIFI_RET_IF_FAIL(self != NULL);
	WIFI_RET_IF_FAIL(text != NULL);

	access_obj = elm_object_item_access_object_get(self->menu_item[menu_type]);
	WIFI_RET_IF_FAIL(access_obj != NULL);

	elm_access_info_set(access_obj, access_type, text);
}

void layout_main_menu_set_enable(layout_main_object *self, main_menu_type type,
				 Eina_Bool is_enable)
{
	WIFI_RET_IF_FAIL(self);

	elm_object_item_disabled_set(self->menu_item[type], !is_enable);
}

void layout_main_menu_update(layout_main_object *self, main_menu_type type)
{
	Elm_Object_Item *item = NULL;
	WIFI_RET_IF_FAIL(self);

	item = self->menu_item[type];
	WIFI_RET_IF_FAIL(item);

	elm_genlist_item_update(item);
}

void layout_main_menu_show(layout_main_object *self, main_menu_type type)
{
	Elm_Object_Item *item = NULL;
	WIFI_RET_IF_FAIL(self);

	item = self->menu_item[type];
	WIFI_RET_IF_FAIL(item);

	elm_genlist_item_show(item, ELM_GENLIST_ITEM_SCROLLTO_MIDDLE);
}

void layout_main_activate_rotary_event(layout_main_object *self)
{
	WIFI_RET_IF_FAIL(self);
	eext_rotary_object_event_activated_set(self->menu_list_circle, EINA_TRUE);
}

void layout_main_deactivate_rotary_event(layout_main_object *self)
{
	WIFI_RET_IF_FAIL(self);
	eext_rotary_object_event_activated_set(self->menu_list_circle, EINA_FALSE);
}

void layout_main_set_del_cb(layout_main_object *self,
			    Evas_Object_Event_Cb func, void *data)
{
	WIFI_RET_IF_FAIL(self);

	self->del_cb.func = func;
	self->del_cb.data = data;
}

void layout_main_set_menu_cb(layout_main_object *self, main_menu_type type,
			     Elm_Gen_Item_Text_Get_Cb text_get, Elm_Gen_Item_Content_Get_Cb content_get,
			     Evas_Smart_Cb tap, void *data)
{
	WIFI_RET_IF_FAIL(self);

	self->menu_cb[type].text_get = text_get;
	self->menu_cb[type].content_get = content_get;
	self->menu_cb[type].tap = tap;
	self->menu_cb[type].data = data;
}
