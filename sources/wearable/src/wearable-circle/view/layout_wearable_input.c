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
#include "view/layout_wearable_input.h"

typedef enum {
	WEARABLE_INPUT_CB_TYPE_CHANGED,
	WEARABLE_INPUT_CB_TYPE_MAXLENGTH_REACHED,
	WEARABLE_INPUT_CB_TYPE_ACTIVATED,
	WEARABLE_INPUT_CB_TYPE_SIZE
} wearable_input_cb_type;

struct _layout_wearable_input_object {
	view_base_object *base;
	Elm_Object_Item *naviframe_item;
	Evas_Object *layout;
	Evas_Object *editfield;
	Elm_Input_Panel_Layout input_type;
	gchar *input_guide_text;
	gchar *input_text;
	int max_char_count;
	Eina_Bool is_input_show;

	struct {
		Evas_Object_Event_Cb func;
		void *data;
	} del_cb;
	struct {
		Evas_Smart_Cb func;
		void *data;
	} menu_cb[WEARABLE_INPUT_CB_TYPE_SIZE];
};

static inline gboolean _is_in_naviframe(layout_wearable_input_object *self)
{
	return self->naviframe_item != NULL;
}

static void __editfield_state_changed_cb(void *data, Ecore_IMF_Context *ctx, int value)
{
	layout_wearable_input_object *input_obj = data;

	if (!input_obj)
		return;

	if (value == ECORE_IMF_INPUT_PANEL_STATE_HIDE) {
		if (view_base_window_is_focus(input_obj->base)) {
			WIFI_LOG_INFO("Key pad is now closed by user");
			layout_wearable_input_pop(input_obj);
		} else {
			WIFI_LOG_INFO("Key pad is now closed by other window show");
		}
	}
}

static void __editfield_focused_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_entry_cursor_end_set(obj);
}

static void __editfield_access_highlighted_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_access_action(obj, ELM_ACCESS_ACTION_ACTIVATE, NULL);
}

static void __editfield_del_cb(void *data, Evas *e,
			       Evas_Object *obj, void *event_info)
{
	Evas_Object *editfield = data;
	Ecore_IMF_Context *imf_context =
		(Ecore_IMF_Context *)elm_entry_imf_context_get(editfield);
	if (imf_context) {
		ecore_imf_context_input_panel_event_callback_clear(imf_context);
	}
}

static Evas_Object *_create_layout(layout_wearable_input_object *self, Evas_Object *parent)
{
	Evas_Object *layout = NULL;
	WIFI_RET_VAL_IF_FAIL(parent, NULL);

	layout = create_layout_use_edj_file(parent, CUSTOM_GROUP_WEARABLE_INPUT);
	WIFI_RET_VAL_IF_FAIL(layout, NULL);

	evas_object_show(layout);
	return layout;
}

static Evas_Object *_create_editfield(layout_wearable_input_object *self, Evas_Object *parent)
{
	Evas_Object *editfield = NULL;
	Ecore_IMF_Context *imf_context = NULL;
	WIFI_RET_VAL_IF_FAIL(parent != NULL, NULL);

	editfield = elm_entry_add(parent);
	elm_entry_single_line_set(editfield, EINA_TRUE);

	WIFI_RET_VAL_IF_FAIL(editfield != NULL, NULL);

	if (self->input_guide_text) {
		gchar *markup_text = elm_entry_utf8_to_markup(self->input_guide_text);
		if (markup_text) {
			elm_object_part_text_set(editfield, "elm.guide", markup_text);
			g_free(markup_text);
		}
	}
	if (self->input_text) {
		gchar *markup_text = elm_entry_utf8_to_markup(self->input_text);
		if (markup_text) {
			elm_object_part_text_set(editfield, "elm.text", markup_text);
			g_free(markup_text);
		}
	}

	eext_entry_selection_back_event_allow_set(editfield, EINA_TRUE);
	elm_entry_scrollable_set(editfield, EINA_TRUE);
	elm_entry_single_line_set(editfield, EINA_TRUE);
	elm_entry_prediction_allow_set(editfield, EINA_FALSE);
	elm_entry_input_hint_set(editfield, ELM_INPUT_HINT_NONE);
	elm_entry_autocapital_type_set(editfield, ELM_AUTOCAPITAL_TYPE_NONE);
	elm_entry_input_panel_layout_set(editfield, self->input_type);
	elm_entry_input_panel_return_key_type_set(editfield, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);

	if (!self->is_input_show) {
		elm_entry_password_set(editfield, EINA_TRUE);
		elm_entry_input_panel_language_set(editfield, ELM_INPUT_PANEL_LANG_ALPHABET);
	}

	imf_context = (Ecore_IMF_Context *)elm_entry_imf_context_get(editfield);
	if (imf_context) {
		ecore_imf_context_input_panel_event_callback_add(imf_context,
								 ECORE_IMF_INPUT_PANEL_STATE_EVENT,
								 __editfield_state_changed_cb, self);
	}
	evas_object_event_callback_add(editfield, EVAS_CALLBACK_DEL,
				       __editfield_del_cb, editfield);
	evas_object_smart_callback_add(editfield, "focused",
				       __editfield_focused_cb, NULL);
	evas_object_smart_callback_add(editfield, "access,highlighted",
				       __editfield_access_highlighted_cb, NULL);
	if (self->menu_cb[WEARABLE_INPUT_CB_TYPE_CHANGED].func) {
		evas_object_smart_callback_add(editfield, "changed",
					       self->menu_cb[WEARABLE_INPUT_CB_TYPE_CHANGED].func,
					       self->menu_cb[WEARABLE_INPUT_CB_TYPE_CHANGED].data);
	}
	if (self->menu_cb[WEARABLE_INPUT_CB_TYPE_MAXLENGTH_REACHED].func) {
		Elm_Entry_Filter_Limit_Size limit_filter_data;
		limit_filter_data.max_char_count = self->max_char_count;
		elm_entry_markup_filter_append(editfield, elm_entry_filter_limit_size,
					       &limit_filter_data);
		evas_object_smart_callback_add(editfield, "maxlength,reached",
					       self->menu_cb[WEARABLE_INPUT_CB_TYPE_MAXLENGTH_REACHED].func,
					       self->menu_cb[WEARABLE_INPUT_CB_TYPE_MAXLENGTH_REACHED].data);
	}
	if (self->menu_cb[WEARABLE_INPUT_CB_TYPE_ACTIVATED].func) {
		evas_object_smart_callback_add(editfield, "activated",
					       self->menu_cb[WEARABLE_INPUT_CB_TYPE_ACTIVATED].func,
					       self->menu_cb[WEARABLE_INPUT_CB_TYPE_ACTIVATED].data);
	}

	return editfield;
}

layout_wearable_input_object *layout_wearable_input_new(view_base_object *base)
{
	layout_wearable_input_object *object = NULL;
	if (!base)
		return NULL;
	object = g_new0(layout_wearable_input_object, 1);
	if (!object) {
		LOGE("layout_wearable_input_new() failed.");
		return NULL;
	}
	object->base = base;
	return object;
}

void layout_wearable_input_free(layout_wearable_input_object *object)
{
	if (object)
		g_free(object);
}

gboolean layout_wearable_input_create(layout_wearable_input_object *self)
{
	Evas_Object *naviframe = NULL;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_VAL_IF_FAIL(self, FALSE);
	WIFI_RET_VAL_IF_FAIL(self->base, FALSE);

	naviframe = view_base_get_naviframe(self->base);
	WIFI_RET_VAL_IF_FAIL(naviframe, FALSE);

	self->layout = _create_layout(self, naviframe);
	WIFI_RET_VAL_IF_FAIL(self->layout, FALSE);

	self->editfield = _create_editfield(self, self->layout);
	if (!self->editfield) {
		layout_wearable_input_destroy(self);
		return FALSE;
	}
	elm_object_part_content_set(self->layout,
				    "elm.swallow.content", self->editfield);

	return TRUE;
}

void layout_wearable_input_destroy(layout_wearable_input_object *self)
{
	WIFI_RET_IF_FAIL(self);

	if (self->input_guide_text) {
		g_free(self->input_guide_text);
		self->input_guide_text = NULL;
	}
	if (self->input_text) {
		g_free(self->input_text);
		self->input_text = NULL;
	}
}

void layout_wearable_input_show(layout_wearable_input_object *self)
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

void layout_wearable_input_pop(layout_wearable_input_object *self)
{
	WIFI_RET_IF_FAIL(self);
	WIFI_RET_IF_FAIL(_is_in_naviframe(self));

	view_base_naviframe_item_pop(self->base);
}

void layout_wearable_input_prediction_on(layout_wearable_input_object *self)
{
	WIFI_RET_IF_FAIL(self != NULL);

	elm_object_signal_emit(self->layout, "prediction,on,layout", "");
}

void layout_wearable_input_prediction_off(layout_wearable_input_object *self)
{
	WIFI_RET_IF_FAIL(self != NULL);

	elm_object_signal_emit(self->layout, "prediction,off,layout", "");
}

void layout_wearable_input_set_input_type(layout_wearable_input_object *self,
					  Elm_Input_Panel_Layout type)
{
	WIFI_RET_IF_FAIL(self);

	self->input_type = type;
}

void layout_wearable_input_set_input_guide_text(layout_wearable_input_object *self,
						const gchar *text)
{
	WIFI_RET_IF_FAIL(self);

	self->input_guide_text = g_strdup(text);
}

void layout_wearable_input_set_input_text(layout_wearable_input_object *self,
					  const gchar *text)
{
	WIFI_RET_IF_FAIL(self);

	self->input_text = g_strdup(text);
}

void layout_wearable_input_set_input_show(layout_wearable_input_object *self,
					  Eina_Bool is_show)
{
	WIFI_RET_IF_FAIL(self);

	self->is_input_show = is_show;
}

void layout_wearable_input_set_input_return_key_enable(layout_wearable_input_object *self,
						       Eina_Bool is_enable)
{
	WIFI_RET_IF_FAIL(self);

	elm_entry_input_panel_return_key_disabled_set(self->editfield, !is_enable);
}

void layout_wearable_input_set_input_focus(layout_wearable_input_object *self,
					   Eina_Bool is_focus)
{
	WIFI_RET_IF_FAIL(self);

	elm_object_focus_set(self->editfield, is_focus);
}

void layout_wearable_input_set_del_cb(layout_wearable_input_object *self,
				      Evas_Object_Event_Cb func, void *data)
{
	WIFI_RET_IF_FAIL(self);

	self->del_cb.func = func;
	self->del_cb.data = data;
}

void layout_wearable_input_set_input_changed_cb(layout_wearable_input_object *self,
						Evas_Smart_Cb func, void *data)
{
	WIFI_RET_IF_FAIL(self);

	self->menu_cb[WEARABLE_INPUT_CB_TYPE_CHANGED].func = func;
	self->menu_cb[WEARABLE_INPUT_CB_TYPE_CHANGED].data = data;
}

void layout_wearable_input_set_input_maxlength_reached_cb(layout_wearable_input_object *self,
							  int max_char_count, Evas_Smart_Cb func, void *data)
{
	WIFI_RET_IF_FAIL(self != NULL);

	self->max_char_count = max_char_count;
	self->menu_cb[WEARABLE_INPUT_CB_TYPE_MAXLENGTH_REACHED].func = func;
	self->menu_cb[WEARABLE_INPUT_CB_TYPE_MAXLENGTH_REACHED].data = data;
}

void layout_wearable_input_set_input_activated_cb(layout_wearable_input_object *self,
						  Evas_Smart_Cb func, void *data)
{
	WIFI_RET_IF_FAIL(self);

	self->menu_cb[WEARABLE_INPUT_CB_TYPE_ACTIVATED].func = func;
	self->menu_cb[WEARABLE_INPUT_CB_TYPE_ACTIVATED].data = data;
}
