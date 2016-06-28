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
#include "view/layout_wps_method.h"

struct _layout_wps_method_object {
	view_base_object *base;
	Elm_Object_Item *naviframe_item;
	Evas_Object *menu_list;
	Evas_Object *menu_list_circle;

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
	} menu_cb[WPS_METHOD_MENU_SIZE];
};

static inline gboolean _is_in_naviframe(layout_wps_method_object *self)
{
	return self->naviframe_item != NULL;
}

static char *__title_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	return g_strdup(STR_WPS_TITLE);
}

static char *__menu_wps_button_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	return g_strdup(STR_WPS_BTN);
}

static char *__menu_wps_pin_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	return g_strdup(STR_WPS_PIN);
}

static Evas_Object *_create_initialized_menu_list(layout_wps_method_object *self,
						  Evas_Object *parent, void *menu_data)
{
	Evas_Object *menu_list = view_base_add_genlist_for_circle(self->base, parent, &(self->menu_list_circle));
	Elm_Genlist_Item_Class *menu_title_itc = NULL;
	Elm_Genlist_Item_Class *menu_wps_button_itc = NULL;
	Elm_Genlist_Item_Class *menu_wps_pin_itc = NULL;
	Elm_Genlist_Item_Class *menu_empty_itc = NULL;

	if (!menu_list) {
		WIFI_LOG_ERR("menu_list create is failed.");
		return NULL;
	}
	evas_object_show(menu_list);

	menu_title_itc = create_genlist_itc("title",
					    __title_text_get_cb, NULL, NULL, NULL);
	if (!menu_title_itc) {
		WIFI_LOG_ERR("menu title itc create failed.");
		evas_object_del(menu_list);
		return NULL;
	}
	menu_wps_button_itc = create_genlist_itc("1text",
						 __menu_wps_button_text_get_cb, NULL, NULL, NULL);
	if (!menu_wps_button_itc) {
		WIFI_LOG_ERR("menu wps button itc create failed.");
		elm_genlist_item_class_free(menu_title_itc);
		evas_object_del(menu_list);
		return NULL;
	}
	menu_wps_pin_itc = create_genlist_itc("1text",
					      __menu_wps_pin_text_get_cb, NULL, NULL, NULL);
	if (!menu_wps_pin_itc) {
		WIFI_LOG_ERR("menu wps pin itc create failed.");
		elm_genlist_item_class_free(menu_title_itc);
		elm_genlist_item_class_free(menu_wps_button_itc);
		evas_object_del(menu_list);
		return NULL;
	}
	menu_empty_itc = create_genlist_itc("1text", NULL, NULL, NULL, NULL);
	if (!menu_empty_itc) {
		WIFI_LOG_ERR("menu wps pin itc create failed.");
		elm_genlist_item_class_free(menu_title_itc);
		elm_genlist_item_class_free(menu_wps_button_itc);
		elm_genlist_item_class_free(menu_wps_pin_itc);
		evas_object_del(menu_list);
		return NULL;
	}
	elm_genlist_item_append(menu_list, menu_title_itc, NULL,
				NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_append(menu_list, menu_wps_button_itc, menu_data,
				NULL, ELM_GENLIST_ITEM_NONE,
				self->menu_cb[WPS_METHOD_MENU_WPS_BUTTON].tap,
				self->menu_cb[WPS_METHOD_MENU_WPS_BUTTON].data);
	elm_genlist_item_append(menu_list, menu_wps_pin_itc, menu_data,
				NULL, ELM_GENLIST_ITEM_NONE,
				self->menu_cb[WPS_METHOD_MENU_WPS_PIN].tap,

				self->menu_cb[WPS_METHOD_MENU_WPS_PIN].data);
	elm_genlist_item_append(menu_list, menu_empty_itc, menu_data,
				NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_class_free(menu_title_itc);
	elm_genlist_item_class_free(menu_wps_button_itc);
	elm_genlist_item_class_free(menu_wps_pin_itc);
	elm_genlist_item_class_free(menu_empty_itc);
	return menu_list;
}

layout_wps_method_object *layout_wps_method_new(view_base_object *base)
{
	layout_wps_method_object *object = NULL;
	if (!base)
		return NULL;
	object = g_new0(layout_wps_method_object, 1);
	if (!object) {
		LOGE("layout_wps_method_new() failed.");
		return NULL;
	}
	object->base = base;
	return object;
}

void layout_wps_method_free(layout_wps_method_object *object)
{
	if (object)
		g_free(object);
}

gboolean layout_wps_method_create(layout_wps_method_object *self, void *menu_data)
{
	Evas_Object *naviframe = NULL;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_VAL_IF_FAIL(self, FALSE);
	WIFI_RET_VAL_IF_FAIL(self->base, FALSE);

	naviframe = view_base_get_naviframe(self->base);
	WIFI_RET_VAL_IF_FAIL(naviframe, FALSE);

	self->menu_list = _create_initialized_menu_list(self, naviframe, menu_data);
	WIFI_RET_VAL_IF_FAIL(self->menu_list, FALSE);

	return TRUE;
}

void layout_wps_method_destroy(layout_wps_method_object *self)
{
	WIFI_RET_IF_FAIL(self);
}

void layout_wps_method_show(layout_wps_method_object *self)
{
	WIFI_RET_IF_FAIL(self);
	WIFI_RET_IF_FAIL(!_is_in_naviframe(self));

	WIFI_RET_IF_FAIL(!_is_in_naviframe(self));
    __WIFI_FUNC_ENTER__;
	self->naviframe_item = view_base_naviframe_push(self->base,
							self->menu_list, self->del_cb.func, self->del_cb.data);
	if (!self->naviframe_item) {
		WIFI_LOG_ERR("layout push to naviframe failed.");
	}
}

void layout_wps_method_pop(layout_wps_method_object *self)
{
	WIFI_RET_IF_FAIL(self);
	WIFI_RET_IF_FAIL(_is_in_naviframe(self));

	view_base_naviframe_item_pop(self->base);
}

void layout_wps_method_activate_rotary_event(layout_wps_method_object *self)
{
	WIFI_RET_IF_FAIL(self);
}

void layout_wps_method_deactivate_rotary_event(layout_wps_method_object *self)
{
	WIFI_RET_IF_FAIL(self);
}

void layout_wps_method_set_del_cb(layout_wps_method_object *self,
				  Evas_Object_Event_Cb func, void *data)
{
	WIFI_RET_IF_FAIL(self);

	self->del_cb.func = func;
	self->del_cb.data = data;
}

void layout_wps_method_set_menu_cb(layout_wps_method_object *self, wps_method_menu_type type,
				   Elm_Gen_Item_Text_Get_Cb text_get, Elm_Gen_Item_Content_Get_Cb content_get,
				   Elm_Gen_Item_State_Get_Cb state_get, Elm_Gen_Item_Del_Cb del,
				   Evas_Smart_Cb tap, void *data)
{
	WIFI_RET_IF_FAIL(self);

	self->menu_cb[type].text_get = text_get;
	self->menu_cb[type].content_get = content_get;
	self->menu_cb[type].state_get = state_get;
	self->menu_cb[type].del = del;
	self->menu_cb[type].tap = tap;
	self->menu_cb[type].data = data;
}
