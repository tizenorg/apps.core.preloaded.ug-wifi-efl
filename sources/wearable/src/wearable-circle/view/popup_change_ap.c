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

#include "util.h"
#include "view/util/efl_helper.h"
#include "view/base.h"
#include "view/popup_change_ap.h"

struct _popup_change_ap_object {
	view_base_object *base;
	Evas_Object *popup;
	Evas_Object *cancel_button;
	Evas_Object *ok_button;
	Evas_Object *content_layout;
	Evas_Object *forget_button;
	gchar *rssi_text;
	popup_change_ap_dismiss_reason dismiss_reason;

	struct {
		Ea_Event_Cb func;
		void *data;
	} destroy_cb;
	struct {
		Evas_Smart_Cb func;
		void *data;
	} tap_ok_button_cb;
	struct {
		Evas_Smart_Cb func;
		void *data;
	} tap_forget_button_cb;
};

static void __popup_hide_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!obj) return;
	elm_popup_dismiss(obj);
}

static Evas_Object *_create_popup(popup_change_ap_object *self, Evas_Object *parent)
{
	Evas_Object *popup = view_base_add_popup_for_circle(self->base, parent);
	WIFI_RET_VAL_IF_FAIL(popup, NULL);

	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK,
				       __popup_hide_cb, NULL);
	evas_object_smart_callback_add(popup, "dismissed",
				       self->destroy_cb.func, self->destroy_cb.data);
	return popup;
}

static Evas_Object *_create_butotn_icon_use_image_file(Evas_Object *parent,
						       const gchar *image_name)
{
	Evas_Object *icon = create_icon_use_image_file(parent, image_name, NULL);
	WIFI_RET_VAL_IF_FAIL(icon, NULL);

	evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	return icon;
}

static inline Evas_Object *_create_cancel_button_icon(popup_change_ap_object *self, Evas_Object *parent)
{
	return _create_butotn_icon_use_image_file(parent, "tw_ic_popup_btn_delete.png");
}

static Evas_Object *_create_cancel_button(popup_change_ap_object *self, Evas_Object *parent)
{
	Evas_Object *cancel_button = elm_button_add(parent);
	Evas_Object *icon = NULL;
	WIFI_RET_VAL_IF_FAIL(cancel_button, NULL);

	elm_object_style_set(cancel_button, "popup/circle/left");
	evas_object_size_hint_weight_set(cancel_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_access_info_set(cancel_button, ELM_ACCESS_INFO, STR_CANCEL_BTN_FOR_TTS);

	icon = _create_cancel_button_icon(self, cancel_button);
	if (!icon) {
		evas_object_del(cancel_button);
		return NULL;
	}
	elm_object_part_content_set(cancel_button, "elm.swallow.content", icon);

	evas_object_smart_callback_add(cancel_button, "clicked",
				       self->destroy_cb.func, self->destroy_cb.data);
	return cancel_button;
}

static inline Evas_Object *_create_ok_button_icon(popup_change_ap_object *self, Evas_Object *parent)
{
	return _create_butotn_icon_use_image_file(parent, "tw_ic_popup_btn_check.png");
}

static Evas_Object *_create_ok_button(popup_change_ap_object *self, Evas_Object *parent)
{
	Evas_Object *ok_button = elm_button_add(parent);
	Evas_Object *icon = NULL;
	WIFI_RET_VAL_IF_FAIL(ok_button, NULL);

	elm_object_style_set(ok_button, "popup/circle/right");
	evas_object_size_hint_weight_set(ok_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_access_info_set(ok_button, ELM_ACCESS_INFO, STR_OK_BTN_FOR_TTS);

	icon = _create_ok_button_icon(self, ok_button);
	if (!icon) {
		evas_object_del(ok_button);
		return NULL;
	}
	elm_object_part_content_set(ok_button, "elm.swallow.content", icon);

	evas_object_smart_callback_add(ok_button, "clicked",
				       self->tap_ok_button_cb.func, self->tap_ok_button_cb.data);
	return ok_button;
}

static Evas_Object *_create_content_layout(popup_change_ap_object *self, Evas_Object *parent)
{
	Evas_Object *layout = create_layout_use_edj_file(parent, CUSTOM_GROUP_BODY_BUTTON);
	gchar **tokens;
	gchar *content_text;
	WIFI_RET_VAL_IF_FAIL(layout, NULL);

	tokens = g_strsplit(STR_CONNECT_Q, "\n", 3);
	content_text = g_strdup_printf("%s<br>%s<br>%s", tokens[0], tokens[1], self->rssi_text);
	g_strfreev(tokens);

	elm_object_part_text_set(layout, "elm.text.content", content_text);
	g_free(content_text);
	return layout;
}

static Evas_Object *_create_forget_button(popup_change_ap_object *self, Evas_Object *parent)
{
	Evas_Object *forget_button = elm_button_add(parent);
	WIFI_RET_VAL_IF_FAIL(forget_button, NULL);

	elm_object_style_set(forget_button, "popup/circle/body");
	elm_object_text_set(forget_button, STR_FORGET);
	evas_object_size_hint_weight_set(forget_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(forget_button, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_propagate_events_set(forget_button, EINA_FALSE);

	evas_object_smart_callback_add(forget_button, "clicked",
				       self->tap_forget_button_cb.func, self->tap_forget_button_cb.data);
	return forget_button;
}

static gboolean _popup_change_ap_create(popup_change_ap_object *self, gboolean is_show_forgetbutton)
{
	Evas_Object *window = NULL;

	WIFI_RET_VAL_IF_FAIL(self, FALSE);
	WIFI_RET_VAL_IF_FAIL(self->base, FALSE);

	window = view_base_get_window(self->base);
	WIFI_RET_VAL_IF_FAIL(window, FALSE);

	self->popup = _create_popup(self, window);
	if (!self->popup) {
		popup_change_ap_destroy(self);
		return FALSE;
	}

	self->cancel_button = _create_cancel_button(self, self->popup);
	if (!self->cancel_button) {
		popup_change_ap_destroy(self);
		return FALSE;
	}
	elm_object_part_content_set(self->popup, "button1", self->cancel_button);

	self->ok_button = _create_ok_button(self, self->popup);
	if (!self->ok_button) {
		popup_change_ap_destroy(self);
		return FALSE;
	}
	elm_object_part_content_set(self->popup, "button2", self->ok_button);

	self->content_layout = _create_content_layout(self, self->popup);
	if (!self->content_layout) {
		popup_change_ap_destroy(self);
		return FALSE;
	}
	elm_object_content_set(self->popup, self->content_layout);

	if (is_show_forgetbutton) {
		self->forget_button = _create_forget_button(self, self->content_layout);
		if (!self->forget_button) {
			popup_change_ap_destroy(self);
			return FALSE;
		}
		elm_object_part_content_set(self->content_layout,
					    "elm.swallow.button", self->forget_button);
	}

	self->dismiss_reason = CHANGE_AP_DISMISS_DEFAULT;

	return TRUE;
}

popup_change_ap_object *popup_change_ap_new(view_base_object *base)
{
	popup_change_ap_object *object = NULL;
	if (!base)
		return NULL;
	object = g_new0(popup_change_ap_object, 1);
	if (!object) {
		LOGE("popup_change_ap_new() failed.");
		return NULL;
	}
	object->base = base;
	return object;
}

void popup_change_ap_free(popup_change_ap_object *object)
{
	if (object)
		g_free(object);
}

// Refer to elm-demo-tizen/2.3-wearable-circle/src/popup.c
// _popup_text_body_button_cb()
gboolean popup_change_ap_create(popup_change_ap_object *self)
{
	return _popup_change_ap_create(self, TRUE);
}

gboolean popup_change_ap_create_hidden_forgetbutton(popup_change_ap_object *self)
{
	return _popup_change_ap_create(self, FALSE);
}

void popup_change_ap_destroy(popup_change_ap_object *self)
{
	WIFI_RET_IF_FAIL(self);

	if (self->rssi_text) {
		g_free(self->rssi_text);
		self->rssi_text = NULL;
	}
	if (self->forget_button) {
		evas_object_del(self->forget_button);
		self->forget_button = NULL;
	}
	if (self->content_layout) {
		evas_object_del(self->content_layout);
		self->content_layout = NULL;
	}
	if (self->cancel_button) {
		evas_object_del(self->cancel_button);
		self->cancel_button = NULL;
	}
	if (self->ok_button) {
		evas_object_del(self->ok_button);
		self->ok_button = NULL;
	}
	if (self->popup) {
		evas_object_del(self->popup);
		self->popup = NULL;
	}
}

void popup_change_ap_show(popup_change_ap_object *self)
{
	WIFI_RET_IF_FAIL(self);
	WIFI_RET_IF_FAIL(self->popup);

	evas_object_show(self->popup);
}

void popup_change_ap_dismiss(popup_change_ap_object *self,
			     popup_change_ap_dismiss_reason reason)
{
	WIFI_RET_IF_FAIL(self != NULL);
	WIFI_RET_IF_FAIL(self->popup != NULL);

	self->dismiss_reason = reason;
	elm_popup_dismiss(self->popup);
}

popup_change_ap_dismiss_reason popup_change_ap_get_dismiss_reason(popup_change_ap_object *self)
{
	WIFI_RET_VAL_IF_FAIL(self != NULL, CHANGE_AP_DISMISS_DEFAULT);

	return self->dismiss_reason;
}

void popup_change_ap_set_destroy_cb(popup_change_ap_object *self,
				    Ea_Event_Cb func, gpointer data)
{
	WIFI_RET_IF_FAIL(self);

	self->destroy_cb.func = func;
	self->destroy_cb.data = data;
}

void popup_change_ap_set_tap_ok_button_cb(popup_change_ap_object *self,
					  Evas_Smart_Cb func, gpointer data)
{
	WIFI_RET_IF_FAIL(self);

	self->tap_ok_button_cb.func = func;
	self->tap_ok_button_cb.data = data;
}

void popup_change_ap_set_tap_forget_button_cb(popup_change_ap_object *self,
					      Evas_Smart_Cb func, gpointer data)
{
	WIFI_RET_IF_FAIL(self);

	self->tap_forget_button_cb.func = func;
	self->tap_forget_button_cb.data = data;
}

void popup_change_ap_set_rssi_text(popup_change_ap_object *self,
				   const gchar *rssi_text)
{
	WIFI_RET_IF_FAIL(self);
	WIFI_RET_IF_FAIL(rssi_text);

	self->rssi_text = g_strdup(rssi_text);
}
