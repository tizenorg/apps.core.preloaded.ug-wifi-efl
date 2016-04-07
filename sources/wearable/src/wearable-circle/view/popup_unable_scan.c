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
#include "view/popup_unable_scan.h"

struct _popup_unable_scan_object {
	view_base_object *base;
	Evas_Object *popup;
	Evas_Object *content_layout;
	Evas_Object *ok_button;

	struct {
		Ea_Event_Cb func;
		void *data;
	} destroy_cb;
};

static void __popup_hide_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!obj) return;
	elm_popup_dismiss(obj);
}

static Evas_Object *_create_popup(popup_unable_scan_object *self, Evas_Object *parent)
{
	Evas_Object *popup = view_base_add_popup_for_circle(self->base, parent);
	WIFI_RET_VAL_IF_FAIL(popup, NULL);

	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK,
				       __popup_hide_cb, NULL);
	evas_object_smart_callback_add(popup, "dismissed",
				       self->destroy_cb.func, self->destroy_cb.data);
	return popup;
}

static Evas_Object *_create_content_layout(popup_unable_scan_object *self, Evas_Object *parent)
{
	Evas_Object *layout = elm_layout_add(parent);
	WIFI_RET_VAL_IF_FAIL(layout, NULL);

	elm_layout_theme_set(layout, "layout", "popup", "content/circle/buttons1");
	elm_layout_edje_object_can_access_set(layout, EINA_TRUE);
	elm_object_part_text_set(layout, "elm.text",
				 wifi_text("WDS_WIMAX_TPOP_GEAR_WILL_SCAN_FOR_WI_FI_NETWORKS_WHEN_DISCONNECTED_FROM_YOUR_MOBILE_DEVICE"));
	return layout;
}

static Evas_Object *_create_ok_button(popup_unable_scan_object *self, Evas_Object *parent)
{
	Evas_Object *ok_button = elm_button_add(parent);
	WIFI_RET_VAL_IF_FAIL(ok_button != NULL, NULL);

	elm_object_style_set(ok_button, "popup/circle");
	elm_object_text_set(ok_button, STR_OK);
	evas_object_size_hint_weight_set(ok_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ok_button, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_smart_callback_add(ok_button, "clicked",
				       self->destroy_cb.func, self->destroy_cb.data);
	return ok_button;
}

popup_unable_scan_object *popup_unable_scan_new(view_base_object *base)
{
	popup_unable_scan_object *object = NULL;
	if (!base)
		return NULL;
	object = g_new0(popup_unable_scan_object, 1);
	if (!object) {
		LOGE("popup_unable_scan_new() failed.");
		return NULL;
	}
	object->base = base;
	return object;
}

void popup_unable_scan_free(popup_unable_scan_object *object)
{
	if (object)
		g_free(object);
}

gboolean popup_unable_scan_create(popup_unable_scan_object *self)
{
	Evas_Object *window = NULL;

	WIFI_RET_VAL_IF_FAIL(self, FALSE);
	WIFI_RET_VAL_IF_FAIL(self->base, FALSE);

	window = view_base_get_window(self->base);
	WIFI_RET_VAL_IF_FAIL(window, FALSE);

	self->popup = _create_popup(self, window);
	if (!self->popup) {
		popup_unable_scan_destroy(self);
		return FALSE;
	}
	self->content_layout = _create_content_layout(self, self->popup);
	if (!self->content_layout) {
		popup_unable_scan_destroy(self);
		return FALSE;
	}
	elm_object_content_set(self->popup, self->content_layout);

	self->ok_button = _create_ok_button(self, self->popup);
	if (!self->ok_button) {
		popup_unable_scan_destroy(self);
		return FALSE;
	}
	elm_object_part_content_set(self->popup, "button1", self->ok_button);

	return TRUE;
}

void popup_unable_scan_destroy(popup_unable_scan_object *self)
{
	WIFI_RET_IF_FAIL(self);

	if (self->ok_button) {
		evas_object_del(self->ok_button);
		self->ok_button = NULL;
	}
	if (self->content_layout) {
		evas_object_del(self->content_layout);
		self->content_layout = NULL;
	}
	if (self->popup) {
		evas_object_del(self->popup);
		self->popup = NULL;
	}
}

void popup_unable_scan_show(popup_unable_scan_object *self)
{
	WIFI_RET_IF_FAIL(self);
	WIFI_RET_IF_FAIL(self->popup);

	evas_object_show(self->popup);
}

void popup_unable_scan_set_destroy_cb(popup_unable_scan_object *self,
				      Ea_Event_Cb func, gpointer data)
{
	WIFI_RET_IF_FAIL(self);

	self->destroy_cb.func = func;
	self->destroy_cb.data = data;
}
