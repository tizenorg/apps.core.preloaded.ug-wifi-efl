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
#include "view/toast_popup.h"

struct _toast_popup_object {
	view_base_object *base;
	Evas_Object *popup;
};

static void __on_popup_hide_finished(void *data, Evas_Object *obj, void *event_info)
{
	toast_popup_object *popup_obj = data;
	WIFI_RET_IF_FAIL(popup_obj != NULL);

	toast_popup_destroy(popup_obj);
	toast_popup_free(popup_obj);
}

static void __on_popup_hide(void *data, Evas_Object *obj, void *event_info)
{
	if (!obj) return;
	elm_popup_dismiss(obj);
}

static Evas_Object *_create_popup(toast_popup_object *self, Evas_Object *parent,
				  const gchar *msg)
{
	Evas_Object *popup;
#if 0
	Evas_Object *ao;
#endif
	WIFI_RET_VAL_IF_FAIL(parent != NULL, NULL);

	popup = elm_popup_add(parent);
	WIFI_RET_VAL_IF_FAIL(popup != NULL, NULL);

	elm_object_style_set(popup, "toast/circle");
	elm_popup_orient_set(popup, ELM_POPUP_ORIENT_BOTTOM);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_text_set(popup, "elm.text", msg);
	evas_object_smart_callback_add(popup, "dismissed", __on_popup_hide_finished, self);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, __on_popup_hide, self);
	evas_object_smart_callback_add(popup, "block,clicked", __on_popup_hide, self);

#if 0
	ao = elm_object_part_access_object_get(popup, "access.outline");
	elm_access_info_cb_set(ao, ELM_ACCESS_INFO, _on_popup_access_info, popup);
#endif

	if (elm_config_access_get()) {
		evas_object_smart_callback_add(popup, "access,read,stop", __on_popup_hide, self);
	} else {
		elm_popup_timeout_set(popup, 2.0);
	}
	evas_object_smart_callback_add(popup, "timeout", __on_popup_hide, self);
	return popup;
}

toast_popup_object *toast_popup_new(view_base_object *base)
{
	toast_popup_object *object = NULL;
	if (!base)
		return NULL;
	object = g_new0(toast_popup_object, 1);
	if (!object) {
		LOGE("toast_popup_new() failed.");
		return NULL;
	}
	object->base = base;
	return object;
}

void toast_popup_free(toast_popup_object *object)
{
	if (object)
		g_free(object);
}

gboolean toast_popup_create(toast_popup_object *self, const gchar *msg)
{
	Evas_Object *window = NULL;

	WIFI_RET_VAL_IF_FAIL(self, FALSE);
	WIFI_RET_VAL_IF_FAIL(self->base, FALSE);

	window = view_base_get_window(self->base);
	WIFI_RET_VAL_IF_FAIL(window, FALSE);

	self->popup = _create_popup(self, window, msg);
	if (!self->popup) {
		toast_popup_destroy(self);
		return FALSE;
	}

	return TRUE;
}

void toast_popup_destroy(toast_popup_object *self)
{
	WIFI_RET_IF_FAIL(self);

	if (self->popup) {
		evas_object_del(self->popup);
		self->popup = NULL;
	}
}

void toast_popup_show(toast_popup_object *self)
{
	WIFI_RET_IF_FAIL(self);
	WIFI_RET_IF_FAIL(self->popup);

	evas_object_show(self->popup);
}
