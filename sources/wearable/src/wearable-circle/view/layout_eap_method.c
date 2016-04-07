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
#include "view/layout_eap_method.h"

struct _layout_eap_method_object {
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
	} menu_cb[EAP_METHOD_MENU_SIZE];
};

static inline gboolean _is_in_naviframe(layout_eap_method_object *self)
{
	return self->naviframe_item != NULL;
}

static char *__title_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	return g_strdup(STR_EAP_METHOD_TITLE);
}

static char *__menu_aka_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	return g_strdup("AKA");
}

static char *__menu_sim_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	return g_strdup("SIM");
}

static Evas_Object *_create_initialized_menu_list(layout_eap_method_object *self,
						  Evas_Object *parent)
{
	Evas_Object *menu_list = view_base_add_genlist_for_circle(self->base, parent, &(self->menu_list_circle));
	Elm_Genlist_Item_Class *menu_title_itc = NULL;
	Elm_Genlist_Item_Class *menu_aka_itc = NULL;
	Elm_Genlist_Item_Class *menu_sim_itc = NULL;

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
	menu_aka_itc = create_genlist_itc("1text",
					  __menu_aka_text_get_cb, NULL, NULL, NULL);
	if (!menu_aka_itc) {
		WIFI_LOG_ERR("menu aka itc create failed.");
		elm_genlist_item_class_free(menu_title_itc);
		evas_object_del(menu_list);
		return NULL;
	}
	menu_sim_itc = create_genlist_itc("1text",
					  __menu_sim_text_get_cb, NULL, NULL, NULL);
	if (!menu_sim_itc) {
		WIFI_LOG_ERR("menu sim itc create failed.");
		elm_genlist_item_class_free(menu_title_itc);
		elm_genlist_item_class_free(menu_aka_itc);
		evas_object_del(menu_list);
		return NULL;
	}
	elm_genlist_item_append(menu_list, menu_title_itc, NULL,
				NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_append(menu_list, menu_aka_itc, NULL,
				NULL, ELM_GENLIST_ITEM_NONE,
				self->menu_cb[EAP_METHOD_MENU_AKA].tap,
				self->menu_cb[EAP_METHOD_MENU_AKA].data);
	elm_genlist_item_append(menu_list, menu_sim_itc, NULL,
				NULL, ELM_GENLIST_ITEM_NONE,
				self->menu_cb[EAP_METHOD_MENU_SIM].tap,
				self->menu_cb[EAP_METHOD_MENU_SIM].data);
	elm_genlist_item_class_free(menu_title_itc);
	elm_genlist_item_class_free(menu_aka_itc);
	elm_genlist_item_class_free(menu_sim_itc);
	return menu_list;
}

layout_eap_method_object *layout_eap_method_new(view_base_object *base)
{
	layout_eap_method_object *object = NULL;
	if (!base)
		return NULL;
	object = g_new0(layout_eap_method_object, 1);
	if (!object) {
		LOGE("layout_eap_method_new() failed.");
		return NULL;
	}
	object->base = base;
	return object;
}

void layout_eap_method_free(layout_eap_method_object *object)
{
	if (object)
		g_free(object);
}


gboolean layout_eap_method_create(layout_eap_method_object *self)
{
	Evas_Object *naviframe = NULL;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_VAL_IF_FAIL(self, FALSE);
	WIFI_RET_VAL_IF_FAIL(self->base, FALSE);

	naviframe = view_base_get_naviframe(self->base);
	WIFI_RET_VAL_IF_FAIL(naviframe, FALSE);

	self->menu_list = _create_initialized_menu_list(self, naviframe);
	WIFI_RET_VAL_IF_FAIL(self->menu_list, FALSE);

	return TRUE;
}

void layout_eap_method_destroy(layout_eap_method_object *self)
{
	WIFI_RET_IF_FAIL(self);
}

void layout_eap_method_show(layout_eap_method_object *self)
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

void layout_eap_method_pop(layout_eap_method_object *self)
{
	WIFI_RET_IF_FAIL(self);
	WIFI_RET_IF_FAIL(_is_in_naviframe(self));

	view_base_naviframe_item_pop(self->base);
}

void layout_eap_method_activate_rotary_event(layout_eap_method_object *self)
{
	WIFI_RET_IF_FAIL(self);
}

void layout_eap_method_deactivate_rotary_event(layout_eap_method_object *self)
{
	WIFI_RET_IF_FAIL(self);
}

void layout_eap_method_set_del_cb(layout_eap_method_object *self,
				  Evas_Object_Event_Cb func, void *data)
{
	WIFI_RET_IF_FAIL(self);

	self->del_cb.func = func;
	self->del_cb.data = data;
}

void layout_eap_method_set_menu_cb(layout_eap_method_object *self, eap_method_menu_type type,
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
