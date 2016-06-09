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
#include "view/popup_scanning.h"

struct _popup_scanning_object {
	view_base_object *base;
	Evas_Object *popup;
	Evas_Object *scanning;

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

static Evas_Object *_create_popup(popup_scanning_object *self, Evas_Object *parent)
{
	Evas_Object *popup;
	WIFI_RET_VAL_IF_FAIL(self != NULL, NULL);
	WIFI_RET_VAL_IF_FAIL(parent != NULL, NULL);

	popup = view_base_add_popup(self->base, parent);
	WIFI_RET_VAL_IF_FAIL(popup != NULL, NULL);

	elm_object_style_set(popup, "circle");
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK,
				       __popup_hide_cb, NULL);
	evas_object_smart_callback_add(popup, "dismissed",
				       self->destroy_cb.func, self->destroy_cb.data);

	return popup;
}

static Evas_Object *_create_scanning_label(Evas_Object *parent)
{
	Evas_Object *label = elm_label_add(parent);
	WIFI_RET_VAL_IF_FAIL(label, NULL);

	elm_object_style_set(label, "popup/default");
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
	elm_object_text_set(label, STR_SCANNING);

	return label;
}

static Evas_Object *_create_scanning_progressbar(Evas_Object *parent)
{
	Evas_Object *progressbar = NULL;

	progressbar = elm_progressbar_add(parent);
	if (!progressbar) {
		WIFI_LOG_ERR("progressbar create is failed.");
		return NULL;
	}
	elm_object_style_set(progressbar, "process/small");
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.5);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_progressbar_pulse(progressbar, EINA_TRUE);

	return progressbar;
}

static Evas_Object *_create_scanning(popup_scanning_object *self, Evas_Object *parent)
{
	Evas_Object *scanning;
	Evas_Object *scanning_label;
	Evas_Object *scanning_progressbar;

	scanning = create_layout_use_edj_file(parent, CUSTOM_GROUP_SCANNING);
	if (!scanning) {
		WIFI_LOG_ERR("create_layout_use_edj_file is failed.");
		return NULL;
	}

	scanning_label = _create_scanning_label(scanning);
	if (!scanning_label) {
		WIFI_LOG_ERR("_create_scanning_label is failed.");
		evas_object_del(scanning);
		return NULL;
	}
	elm_object_part_content_set(scanning, "elm.text.progressbar", scanning_label);

	scanning_progressbar = _create_scanning_progressbar(scanning);
	if (!scanning_progressbar) {
		WIFI_LOG_ERR("_create_scanning_progressbar is failed.");
		evas_object_del(scanning_label);
		evas_object_del(scanning);
		return NULL;
	}
	elm_object_part_content_set(scanning, "elm.swallow.content", scanning_progressbar);

	return scanning;
}

popup_scanning_object *popup_scanning_new(view_base_object *base)
{
	popup_scanning_object *object = NULL;
	if (!base)
		return NULL;
	object = g_new0(popup_scanning_object, 1);
	if (!object) {
		LOGE("popup_scanning_new() failed.");
		return NULL;
	}
	object->base = base;
	return object;
}

void popup_scanning_free(popup_scanning_object *object)
{
	if (object)
		g_free(object);
}

gboolean popup_scanning_create(popup_scanning_object *self)
{
	__WIFI_FUNC_ENTER__;

	WIFI_RET_VAL_IF_FAIL(self != NULL, FALSE);
	WIFI_RET_VAL_IF_FAIL(self->base != NULL, FALSE);

	self->popup = _create_popup(self, view_base_get_window(self->base));
	WIFI_RET_VAL_IF_FAIL(self->popup != NULL, FALSE);

	self->scanning = _create_scanning(self, self->popup);
	if (!self->scanning) {
		popup_scanning_destroy(self);
		return FALSE;
	}
	elm_object_content_set(self->popup, self->scanning);

	return TRUE;
}

void popup_scanning_destroy(popup_scanning_object *self)
{
	WIFI_RET_IF_FAIL(self != NULL);
	__WIFI_FUNC_ENTER__;

	if (self->popup) {
		evas_object_del(self->popup);
		self->popup = NULL;
		__WIFI_FUNC_EXIT__;
	}
}

void popup_scanning_show(popup_scanning_object *self)
{
	WIFI_RET_IF_FAIL(self != NULL);
	WIFI_RET_IF_FAIL(self->popup != NULL);

	evas_object_show(self->popup);
}

void popup_scanning_dismiss(popup_scanning_object *self)
{
	WIFI_RET_IF_FAIL(self != NULL);
	WIFI_RET_IF_FAIL(self->popup != NULL);

	elm_popup_dismiss(self->popup);

	/* TODO: It will be removed. */
	popup_scanning_destroy(self);
}

void popup_scanning_set_destroy_cb(popup_scanning_object *self,
				   Ea_Event_Cb func, void *data)
{
	WIFI_RET_IF_FAIL(self != NULL);

	self->destroy_cb.func = func;
	self->destroy_cb.data = data;
}
