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
#include "view/layout_wps_progress.h"

struct _layout_wps_progress_object {
	view_base_object *base;
	Elm_Object_Item *naviframe_item;
	Evas_Object *popup;
	Evas_Object *progress_layout;
	Evas_Object *bottom_button_layout;
	Evas_Object *progress_popup_layout;
	Evas_Object *progressbar;
	Evas_Object *progress_label;
	Evas_Object *progress_label_circle;
	Evas_Object *progress_percent_label;
	Evas_Object *cancel_button;

	Ecore_Timer *progress_timer;

	gchar *progress_label_text;

	struct {
		Evas_Smart_Cb func;
		void *data;
	} show_finished_cb;
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

static void __response_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data) return;
	layout_wps_progress_object *self = data;
	elm_popup_dismiss(self->popup);

	/* TODO: It will be removed */
	layout_wps_progress_destroy(self);
}

static Eina_Bool __progress_timer_task_cb(void *data)
{
	double progressbar_value, progressbar_interval = 1.2;
	gchar percent_text[8];
	layout_wps_progress_object *wps_progress = data;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_VAL_IF_FAIL(wps_progress != NULL, ECORE_CALLBACK_CANCEL);
	WIFI_RET_VAL_IF_FAIL(wps_progress->progress_timer != NULL, ECORE_CALLBACK_CANCEL);

	progressbar_value = eext_circle_object_value_get(wps_progress->progressbar);
	if (progressbar_value < 100)
		progressbar_value += progressbar_interval;
	eext_circle_object_value_set(wps_progress->progressbar, progressbar_value);

	g_snprintf(percent_text, sizeof(percent_text), "%d%%", (gint)progressbar_value);
	elm_object_text_set(wps_progress->progress_percent_label, percent_text);

	WIFI_LOG_INFO("progress[%0.2f]", progressbar_value);
	if (progressbar_value < 100)
		return ECORE_CALLBACK_RENEW;

	layout_wps_progress_dismiss(wps_progress);
	return ECORE_CALLBACK_CANCEL;
}

static Evas_Object *_create_popup(layout_wps_progress_object *self, Evas_Object *parent)
{
	Evas_Object *popup = view_base_add_popup_for_circle(self->base, parent);
	WIFI_RET_VAL_IF_FAIL(popup != NULL, NULL);

	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK,
									__popup_hide_cb, NULL);
	evas_object_smart_callback_add(popup, "dismissed",
									self->destroy_cb.func, self->destroy_cb.data);
	return popup;
}

static Evas_Object *_create_progress_layout(layout_wps_progress_object *self, Evas_Object *parent)
{
	Evas_Object *progress_layout;
	WIFI_RET_VAL_IF_FAIL(self != NULL, NULL);
	WIFI_RET_VAL_IF_FAIL(parent != NULL, NULL);

	progress_layout = create_layout_use_edj_file(parent, CUSTOM_GROUP_WPS_PROGRESS);
	WIFI_RET_VAL_IF_FAIL(progress_layout != NULL, NULL);

	evas_object_size_hint_weight_set(progress_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(progress_layout);
	return progress_layout;
}

static Evas_Object *_create_bottom_button_layout(layout_wps_progress_object *self, Evas_Object *parent)
{
	Evas_Object *bottom_button_layout;
	WIFI_RET_VAL_IF_FAIL(self != NULL, NULL);
	WIFI_RET_VAL_IF_FAIL(parent != NULL, NULL);

	bottom_button_layout = elm_layout_add(parent);
	WIFI_RET_VAL_IF_FAIL(bottom_button_layout != NULL, NULL);

	elm_layout_theme_set(bottom_button_layout, "layout", "bottom_button", "default");
	evas_object_size_hint_weight_set(bottom_button_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(bottom_button_layout);
	return bottom_button_layout;
}

static inline Evas_Object *_create_progress_popup_layout(layout_wps_progress_object *self, Evas_Object *parent)
{
	Evas_Object *progress_popup_layout;
	WIFI_RET_VAL_IF_FAIL(self != NULL, NULL);
	WIFI_RET_VAL_IF_FAIL(parent != NULL, NULL);

	progress_popup_layout = create_layout_use_edj_file(parent, CUSTOM_GROUP_WPS_PROGRESS_POPUP);
	WIFI_RET_VAL_IF_FAIL(progress_popup_layout != NULL, NULL);

	evas_object_size_hint_weight_set(progress_popup_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(progress_popup_layout);
	return progress_popup_layout;
}

static Evas_Object *_create_progress_label(layout_wps_progress_object *self, Evas_Object *parent)
{
	Evas_Object *progress_label = NULL;
	Evas_Object *text_layout = NULL;
	WIFI_RET_VAL_IF_FAIL(parent, NULL);

	progress_label = view_base_add_scroller_for_circle(self->base, parent,
							   &(self->progress_label_circle));
	WIFI_RET_VAL_IF_FAIL(progress_label, NULL);
	WIFI_RET_VAL_IF_FAIL(self->progress_label_circle, NULL);

	elm_object_style_set(progress_label, "effect");
	evas_object_size_hint_weight_set(progress_label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_scroller_bounce_set(progress_label, EINA_TRUE, EINA_TRUE);
	evas_object_show(progress_label);

	text_layout = create_layout_use_edj_file(progress_label,
						 CUSTOM_GROUP_WPS_PROGRESS_TEXT_BLOCK);
	if (!text_layout) {
		WIFI_LOG_ERR("create_layout_use_edj_file() is failed.");
		return NULL;
	}
	elm_object_part_text_set(text_layout, "elm.text.content", self->progress_label_text);
	elm_object_content_set(progress_label, text_layout);

	return progress_label;
}

static Evas_Object *_create_progressbar(layout_wps_progress_object *self, Evas_Object *parent)
{
	Evas_Object *progressbar = NULL;
	WIFI_RET_VAL_IF_FAIL(parent, NULL);

	progressbar = eext_circle_object_progressbar_add(parent, NULL);
	WIFI_RET_VAL_IF_FAIL(progressbar, NULL);

	eext_circle_object_value_min_max_set(progressbar, 0.0, 100.0);
	evas_object_show(progressbar);
	return progressbar;
}

static Evas_Object *_create_progress_percent_label(layout_wps_progress_object *self, Evas_Object *parent)
{
	Evas_Object *percent_label;
	WIFI_RET_VAL_IF_FAIL(self != NULL, NULL);
	WIFI_RET_VAL_IF_FAIL(parent != NULL, NULL);

	percent_label = elm_label_add(parent);
	WIFI_RET_VAL_IF_FAIL(percent_label != NULL, NULL);

	elm_object_style_set(percent_label, "popup/default");
	elm_label_line_wrap_set(percent_label, ELM_WRAP_MIXED);
	elm_object_text_set(percent_label, "0%");
	evas_object_show(percent_label);

	return percent_label;
}

static Evas_Object *_create_cancel_button(layout_wps_progress_object *self, Evas_Object *parent)
{
	Evas_Object *cancel_button;
	WIFI_RET_VAL_IF_FAIL(self != NULL, NULL);
	WIFI_RET_VAL_IF_FAIL(parent != NULL, NULL);

	cancel_button = elm_button_add(parent);
	WIFI_RET_VAL_IF_FAIL(cancel_button != NULL, NULL);

	elm_object_style_set(cancel_button, "bottom");
	elm_object_text_set(cancel_button, STR_CANCEL);
	evas_object_size_hint_weight_set(cancel_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(cancel_button, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_propagate_events_set(cancel_button, EINA_FALSE);

	evas_object_smart_callback_add(cancel_button, "clicked",
				       __response_cb, self);

	return cancel_button;
}

layout_wps_progress_object *layout_wps_progress_new(view_base_object *base)
{
	layout_wps_progress_object *object = NULL;
	if (!base)
		return NULL;
	object = g_new0(layout_wps_progress_object, 1);
	if (!object) {
		LOGE("layout_wps_progress_new() failed.");
		return NULL;
	}
	object->base = base;
	return object;
}

void layout_wps_progress_free(layout_wps_progress_object *object)
{
	if (object)
		g_free(object);
}

gboolean layout_wps_progress_create(layout_wps_progress_object *self)
{
	__WIFI_FUNC_ENTER__;

	WIFI_RET_VAL_IF_FAIL(self != NULL, FALSE);
	WIFI_RET_VAL_IF_FAIL(self->base != NULL, FALSE);

	self->popup = _create_popup(self, view_base_get_naviframe(self->base));
	WIFI_RET_VAL_IF_FAIL(self->popup != NULL, FALSE);

	self->progress_layout = _create_progress_layout(self, self->popup);
	if (!self->progress_layout) {
		WIFI_LOG_ERR("_create_progress_layout() is failed.");
		layout_wps_progress_destroy(self);
		return FALSE;
	}
	elm_object_content_set(self->popup, self->progress_layout);

	self->progressbar = _create_progressbar(self, self->progress_layout);
	if (!self->progressbar) {
		WIFI_LOG_ERR("_create_progressbar() is failed.");
		layout_wps_progress_destroy(self);
		return FALSE;
	}
	elm_object_part_content_set(self->progress_layout,
				    "elm.swallow.progressbar", self->progressbar);

	self->bottom_button_layout = _create_bottom_button_layout(self, self->progress_layout);
	if (!self->bottom_button_layout) {
		WIFI_LOG_ERR("_create_bottom_button_layout() is failed.");
		layout_wps_progress_destroy(self);
		return FALSE;
	}
	elm_object_part_content_set(self->progress_layout,
				    "elm.swallow.content", self->bottom_button_layout);

	self->progress_popup_layout = _create_progress_popup_layout(self, self->bottom_button_layout);
	if (!self->progress_popup_layout) {
		WIFI_LOG_ERR("_create_progress_popup_layout() is failed.");
		layout_wps_progress_destroy(self);
		return FALSE;
	}
	elm_object_part_content_set(self->bottom_button_layout,
				    "elm.swallow.content", self->progress_popup_layout);

	self->progress_label = _create_progress_label(self, self->progress_popup_layout);
	if (!self->progress_label) {
		WIFI_LOG_ERR("_create_progress_label() is failed.");
		layout_wps_progress_destroy(self);
		return FALSE;
	}
	elm_object_part_content_set(self->progress_popup_layout,
				    "elm.swallow.label", self->progress_label);

	self->progress_percent_label =
		_create_progress_percent_label(self, self->progress_popup_layout);
	if (!self->progress_percent_label) {
		WIFI_LOG_ERR("_create_progress_percent_label() is failed.");
		layout_wps_progress_destroy(self);
		return FALSE;
	}
	elm_object_part_content_set(self->progress_popup_layout,
				    "elm.swallow.percent_label", self->progress_percent_label);

	self->cancel_button = _create_cancel_button(self, self->bottom_button_layout);
	if (!self->cancel_button) {
		WIFI_LOG_ERR("_create_cancel_button() is failed.");
		layout_wps_progress_destroy(self);
		return FALSE;
	}
	elm_object_part_content_set(self->bottom_button_layout,
				    "elm.swallow.button", self->cancel_button);

	self->progress_timer = ecore_timer_add(1.2, __progress_timer_task_cb, self);
	if (!self->progress_timer) {
		WIFI_LOG_ERR("ecore_timer_add() is failed.");
		layout_wps_progress_destroy(self);
		return FALSE;
	}

	return TRUE;
}

void layout_wps_progress_destroy(layout_wps_progress_object *self)
{
	WIFI_RET_IF_FAIL(self);

	if (self->progress_timer) {
		ecore_timer_del(self->progress_timer);
		self->progress_timer = NULL;
	}
	if (self->progress_label_text) {
		g_free(self->progress_label_text);
		self->progress_label_text = NULL;
	}

	if (self->popup) {
		evas_object_del(self->popup);
		self->popup = NULL;
	}
}

void layout_wps_progress_show(layout_wps_progress_object *self)
{
	WIFI_RET_IF_FAIL(self != NULL);
	WIFI_RET_IF_FAIL(self->popup != NULL);

	if (self->show_finished_cb.func) {
		evas_object_smart_callback_add(self->popup, "show,finished",
					       self->show_finished_cb.func, self->show_finished_cb.data);
	}
	evas_object_show(self->popup);
}

void layout_wps_progress_dismiss(layout_wps_progress_object *self)
{
	WIFI_RET_IF_FAIL(self != NULL);
	WIFI_RET_IF_FAIL(self->popup != NULL);

	elm_popup_dismiss(self->popup);

	/* TODO: It will be removed */
	layout_wps_progress_destroy(self);
}

void layout_wps_progress_activate_rotary_event(layout_wps_progress_object *self)
{
	WIFI_RET_IF_FAIL(self != NULL);
	WIFI_RET_IF_FAIL(self->progress_label_circle != NULL);
}

void layout_wps_progress_set_show_finished_cb(layout_wps_progress_object *self,
					      Evas_Smart_Cb func, void *data)
{
	WIFI_RET_IF_FAIL(self != NULL);

	self->show_finished_cb.func = func;
	self->show_finished_cb.data = data;
}

void layout_wps_progress_set_destroy_cb(layout_wps_progress_object *self,
					Ea_Event_Cb func, void *data)
{
	WIFI_RET_IF_FAIL(self != NULL);

	self->destroy_cb.func = func;
	self->destroy_cb.data = data;
}

void layout_wps_progress_set_label_text(layout_wps_progress_object *self,
					const gchar *label_text)
{
	WIFI_RET_IF_FAIL(self);
	WIFI_RET_IF_FAIL(label_text);

	if (self->progress_label_text) {
		g_free(self->progress_label_text);
	}
	self->progress_label_text = g_strdup(label_text);
}
