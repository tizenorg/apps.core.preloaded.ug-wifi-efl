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
#include "view/layout_ap_info.h"

typedef enum {
	AP_INFO_VIEW_UNKNOWN,
	AP_INFO_VIEW_OPEN,
	AP_INFO_VIEW_WPS,
	AP_INFO_VIEW_EAP,
	AP_INFO_VIEW_SECURITY
} ap_info_view_state_e;

struct _layout_ap_info_object {
	view_base_object *base;
	Elm_Object_Item *naviframe_item;
	Evas_Object *layout;
	Evas_Object *menu_list;
	Evas_Object *menu_list_circle;
	Evas_Object *connect_button;
	Elm_Object_Item *menu_item[AP_INFO_MENU_SIZE];

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
	} menu_cb[AP_INFO_MENU_SIZE];
	struct {
		Evas_Smart_Cb func;
		void *data;
	} connect_button_tap_cb;
};

static inline gboolean _is_in_naviframe(layout_ap_info_object *self)
{
	return self->naviframe_item != NULL;
}

static char *__wps_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	return g_strdup(STR_WPS_METHOD);
}

static char *__password_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	return g_strdup(STR_PASSWORD);
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

static Evas_Object *_create_menu_list(layout_ap_info_object *self, Evas_Object *parent)
{
	Evas_Object *menu_list = view_base_add_genlist_for_circle(self->base, parent, &(self->menu_list_circle));
	if (!menu_list) {
		WIFI_LOG_ERR("menu_list create is failed.");
		return NULL;
	}
	evas_object_show(menu_list);
	return menu_list;
}

static Evas_Object *_create_connect_button(layout_ap_info_object *self, Evas_Object *parent)
{
	Evas_Object *connect_button = elm_button_add(parent);
	WIFI_RET_VAL_IF_FAIL(connect_button, NULL);

	elm_object_style_set(connect_button, "bottom");
	elm_object_text_set(connect_button, STR_CONNECT_UPPER);
	evas_object_size_hint_weight_set(connect_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(connect_button, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_propagate_events_set(connect_button, EINA_FALSE);

	evas_object_smart_callback_add(connect_button, "clicked",
				       self->connect_button_tap_cb.func, self->connect_button_tap_cb.data);

	return connect_button;
}

static void _append_menu(layout_ap_info_object *self, ap_info_menu_type type,
			 void *menu_data)
{
	Elm_Genlist_Item_Class *itc = NULL;
	switch (type) {
	case AP_INFO_MENU_TITLE:
		itc = create_genlist_itc("title",
					 self->menu_cb[type].text_get, NULL, NULL, NULL);
		break;

	case AP_INFO_MENU_WPS:
		itc = create_genlist_itc("1text",
					 __wps_text_get_cb, NULL, NULL, NULL);
		break;

	case AP_INFO_MENU_EAP:
		itc = create_genlist_itc("2text",
					 self->menu_cb[type].text_get, NULL, NULL, NULL);
		break;

	case AP_INFO_MENU_PASSWORD:
		itc = create_genlist_itc("1text",
					 __password_text_get_cb, NULL, NULL, NULL);
		break;

	case AP_INFO_MENU_STATIC:
	case AP_INFO_MENU_PROXY:
		itc = create_genlist_itc("1text.1icon.1",
					 self->menu_cb[type].text_get, self->menu_cb[type].content_get, NULL, NULL);
		break;

	case AP_INFO_MENU_EMPTY:
		/* Empty item */
		itc = create_genlist_itc("padding", NULL, NULL, NULL, NULL);
		break;

	default:
		return;
	}
	self->menu_item[type] = elm_genlist_item_append(self->menu_list, itc, menu_data,
							NULL, ELM_GENLIST_ITEM_NONE,
							self->menu_cb[type].tap, self->menu_cb[type].data);
	elm_genlist_item_class_free(itc);
}

layout_ap_info_object *layout_ap_info_new(view_base_object *base)
{
	layout_ap_info_object *object = NULL;
	if (!base)
		return NULL;
	object = g_new0(layout_ap_info_object, 1);
	if (!object) {
		LOGE("layout_ap_info_new() failed.");
		return NULL;
	}
	object->base = base;
	return object;
}

void layout_ap_info_free(layout_ap_info_object *object)
{
	if (object)
		g_free(object);
}

gboolean layout_ap_info_create(layout_ap_info_object *self)
{
	Evas_Object *naviframe = NULL;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_VAL_IF_FAIL(self, FALSE);
	WIFI_RET_VAL_IF_FAIL(self->base, FALSE);

	naviframe = view_base_get_naviframe(self->base);
	WIFI_RET_VAL_IF_FAIL(naviframe, FALSE);

	self->layout = _create_layout(naviframe);
	WIFI_RET_VAL_IF_FAIL(self->layout, FALSE);

	self->menu_list = _create_menu_list(self, self->layout);
	if (!self->menu_list) {
		layout_ap_info_destroy(self);
		return FALSE;
	}
	elm_object_part_content_set(self->layout,
				    "elm.swallow.content", self->menu_list);

	self->connect_button = _create_connect_button(self, self->layout);
	if (!self->connect_button) {
		layout_ap_info_destroy(self);
		return FALSE;
	}
	elm_object_part_content_set(self->layout,
				    "elm.swallow.button", self->connect_button);

	return TRUE;
}

void layout_ap_info_destroy(layout_ap_info_object *self)
{
	WIFI_RET_IF_FAIL(self);
}

void layout_ap_info_pop_to(layout_ap_info_object *self)
{
	WIFI_RET_IF_FAIL(self);

    WIFI_LOG_INFO("naviframe pop to - ap info");

	view_base_naviframe_item_pop_to(self->base, self->naviframe_item);
}

void layout_ap_info_open_show(layout_ap_info_object *self, void *menu_data)
{
	WIFI_RET_IF_FAIL(self);
	WIFI_RET_IF_FAIL(!_is_in_naviframe(self));

    __WIFI_FUNC_ENTER__;
	_append_menu(self, AP_INFO_MENU_TITLE, menu_data);
	_append_menu(self, AP_INFO_MENU_STATIC, menu_data);
	_append_menu(self, AP_INFO_MENU_PROXY, menu_data);
	_append_menu(self, AP_INFO_MENU_EMPTY, menu_data);
	self->naviframe_item = view_base_naviframe_push(self->base,
							self->layout, self->del_cb.func, self->del_cb.data);
	if (!self->naviframe_item) {
		WIFI_LOG_ERR("layout push to naviframe failed.");
	}
}

void layout_ap_info_wps_show(layout_ap_info_object *self, void *menu_data)
{
	WIFI_RET_IF_FAIL(self);
	WIFI_RET_IF_FAIL(!_is_in_naviframe(self));

    __WIFI_FUNC_ENTER__;
	_append_menu(self, AP_INFO_MENU_TITLE, menu_data);
	_append_menu(self, AP_INFO_MENU_PASSWORD, menu_data);
	_append_menu(self, AP_INFO_MENU_WPS, menu_data);
	_append_menu(self, AP_INFO_MENU_STATIC, menu_data);
	_append_menu(self, AP_INFO_MENU_PROXY, menu_data);
	_append_menu(self, AP_INFO_MENU_EMPTY, menu_data);
	self->naviframe_item = view_base_naviframe_push(self->base,
							self->layout, self->del_cb.func, self->del_cb.data);
	if (!self->naviframe_item) {
		WIFI_LOG_ERR("layout push to naviframe failed.");
	}
}

void layout_ap_info_eap_show(layout_ap_info_object *self, void *menu_data)
{
	WIFI_RET_IF_FAIL(self);
	WIFI_RET_IF_FAIL(!_is_in_naviframe(self));

    __WIFI_FUNC_ENTER__;

	_append_menu(self, AP_INFO_MENU_TITLE, menu_data);
	_append_menu(self, AP_INFO_MENU_EAP, menu_data);
	_append_menu(self, AP_INFO_MENU_STATIC, menu_data);
	_append_menu(self, AP_INFO_MENU_PROXY, menu_data);
	_append_menu(self, AP_INFO_MENU_EMPTY, menu_data);
	self->naviframe_item = view_base_naviframe_push(self->base,
							self->layout, self->del_cb.func, self->del_cb.data);
	if (!self->naviframe_item) {
		WIFI_LOG_ERR("layout push to naviframe failed.");
	}
}

void layout_ap_info_security_show(layout_ap_info_object *self, void *menu_data)
{
	WIFI_RET_IF_FAIL(self);
	WIFI_RET_IF_FAIL(!_is_in_naviframe(self));

    __WIFI_FUNC_ENTER__;

	_append_menu(self, AP_INFO_MENU_TITLE, menu_data);
	_append_menu(self, AP_INFO_MENU_PASSWORD, menu_data);
	_append_menu(self, AP_INFO_MENU_STATIC, menu_data);
	_append_menu(self, AP_INFO_MENU_PROXY, menu_data);
	_append_menu(self, AP_INFO_MENU_EMPTY, menu_data);
	self->naviframe_item = view_base_naviframe_push(self->base,
							self->layout, self->del_cb.func, self->del_cb.data);
	if (!self->naviframe_item) {
		WIFI_LOG_ERR("layout push to naviframe failed.");
	}
}

void layout_ap_info_menu_set_access_info(layout_ap_info_object *self, ap_info_menu_type menu_type,
					 Elm_Access_Info_Type access_type, const gchar *text)
{
	Evas_Object *access_obj;
	WIFI_RET_IF_FAIL(self != NULL);
	WIFI_RET_IF_FAIL(text != NULL);

	access_obj = elm_object_item_access_object_get(self->menu_item[menu_type]);
	WIFI_RET_IF_FAIL(access_obj != NULL);

	elm_access_info_set(access_obj, access_type, text);
}

void layout_ap_info_menu_update(layout_ap_info_object *self, ap_info_menu_type type)
{
	Elm_Object_Item *item = NULL;
	WIFI_RET_IF_FAIL(self);

	item = self->menu_item[type];
	WIFI_RET_IF_FAIL(item);

	elm_genlist_item_update(item);
}

void layout_ap_info_set_connect_button_enable(layout_ap_info_object *self,
					      Eina_Bool is_enable)
{
	WIFI_RET_IF_FAIL(self);

	elm_object_disabled_set(self->connect_button, !is_enable);
}

void layout_ap_info_activate_rotary_event(layout_ap_info_object *self)
{
	WIFI_RET_IF_FAIL(self);
	eext_rotary_object_event_activated_set(self->menu_list_circle, EINA_TRUE);
}

void layout_ap_info_deactivate_rotary_event(layout_ap_info_object *self)
{
	WIFI_RET_IF_FAIL(self);
	eext_rotary_object_event_activated_set(self->menu_list_circle, EINA_FALSE);
}

void layout_ap_info_set_del_cb(layout_ap_info_object *self,
			       Evas_Object_Event_Cb func, void *data)
{
	WIFI_RET_IF_FAIL(self);

	self->del_cb.func = func;
	self->del_cb.data = data;
}

void layout_ap_info_set_menu_cb(layout_ap_info_object *self, ap_info_menu_type type,
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

void layout_ap_info_set_tap_connect_button_cb(layout_ap_info_object *self,
					      Evas_Smart_Cb func, void *data)
{
	WIFI_RET_IF_FAIL(self);

	self->connect_button_tap_cb.func = func;
	self->connect_button_tap_cb.data = data;
}
