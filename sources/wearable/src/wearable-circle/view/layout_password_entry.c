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
#include "view/layout_password_entry.h"

typedef enum {
	PASSWORD_ENTRY_ITEM_ENTRY,
	PASSWORD_ENTRY_ITEM_CHECKBOX,
	PASSWORD_ENTRY_ITEM_SIZE
} password_entry_item_type;

struct _layout_password_entry_object {
	view_base_object *base;
	Elm_Object_Item *naviframe_item;
	Evas_Object *layout;
	Evas_Object *password_entry;
	Evas_Object *password_checkbox;

	struct {
		Evas_Object_Event_Cb func;
		void *data;
	} del_cb;
	struct {
		Evas_Smart_Cb func;
		void *data;
	} menu_cb[PASSWORD_ENTRY_ITEM_SIZE];
};

static inline gboolean _is_in_naviframe(layout_password_entry_object *self)
{
	return self->naviframe_item != NULL;
}

static void __check_click(layout_password_entry_object *password_entry)
{
	if (elm_object_disabled_get(password_entry->password_checkbox)) {
		WIFI_LOG_ERR("Show password disabled.");
		return;
	}
	elm_check_state_set(password_entry->password_checkbox,
			    !elm_check_state_get(password_entry->password_checkbox));
	if (password_entry->menu_cb[PASSWORD_ENTRY_ITEM_CHECKBOX].func) {
		password_entry->menu_cb[PASSWORD_ENTRY_ITEM_CHECKBOX].func(
			password_entry->menu_cb[PASSWORD_ENTRY_ITEM_CHECKBOX].data,
			password_entry->password_checkbox, NULL);
	}
}

static void __check_text_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	layout_password_entry_object *password_entry = data;
	WIFI_RET_IF_FAIL(password_entry != NULL);

	__check_click(password_entry);
}

static Evas_Object *_create_layout(layout_password_entry_object *self, Evas_Object *parent)
{
	Evas_Object *layout;
	WIFI_RET_VAL_IF_FAIL(parent, NULL);

	layout = create_layout_use_edj_file(parent, CUSTOM_GROUP_PASSWORD_ENTRY);
	WIFI_RET_VAL_IF_FAIL(layout, NULL);

	elm_object_part_text_set(layout, "elm.text.title", STR_PASSWORD_HEADER);
	elm_object_part_text_set(layout, "elm.text.check", STR_SHOW_PASSWORD);
	edje_object_signal_callback_add(elm_layout_edje_get(layout), "clicked", "",
					__check_text_clicked_cb, self);

	return layout;
}

static Evas_Object *_create_entry(layout_password_entry_object *self, Evas_Object *parent)
{
	Evas_Object *entry = NULL;
	WIFI_RET_VAL_IF_FAIL(parent, NULL);

	entry = elm_entry_add(parent);
	WIFI_RET_VAL_IF_FAIL(entry != NULL, NULL);

	elm_object_style_set(entry, "editfield/password");
	elm_object_part_text_set(entry, "elm.guide", STR_PASSWORD_HEADER);
	elm_access_object_unregister(entry);

	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_entry_input_panel_enabled_set(entry, EINA_FALSE);
	elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_PASSWORD);
	elm_entry_prediction_allow_set(entry, EINA_FALSE);
	elm_entry_autocapital_type_set(entry, ELM_AUTOCAPITAL_TYPE_NONE);
	elm_entry_scrollable_set(entry, EINA_TRUE);
    elm_entry_input_panel_return_key_type_set(entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);
    elm_entry_input_panel_return_key_type_set(entry, EINA_TRUE);

	evas_object_smart_callback_add(entry, "clicked",
				       self->menu_cb[PASSWORD_ENTRY_ITEM_ENTRY].func,
				       self->menu_cb[PASSWORD_ENTRY_ITEM_ENTRY].data);
	return entry;
}

static Evas_Object *_create_checkbox(layout_password_entry_object *self, Evas_Object *parent)
{
	Evas_Object *checkbox;
	WIFI_RET_VAL_IF_FAIL(self != NULL, NULL);
	WIFI_RET_VAL_IF_FAIL(parent != NULL, NULL);

	checkbox = elm_check_add(parent);
	WIFI_RET_VAL_IF_FAIL(checkbox != NULL, NULL);

	elm_object_style_set(checkbox, "popup");
	elm_access_object_unregister(checkbox);
	elm_object_focus_allow_set(checkbox, EINA_FALSE);
	evas_object_propagate_events_set(checkbox, EINA_FALSE);
	evas_object_size_hint_weight_set(checkbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(checkbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_check_state_set(checkbox, EINA_FALSE);

	evas_object_smart_callback_add(checkbox, "changed",
				       self->menu_cb[PASSWORD_ENTRY_ITEM_CHECKBOX].func,
				       self->menu_cb[PASSWORD_ENTRY_ITEM_CHECKBOX].data);
	return checkbox;
}

static void _on_access_activate_entry(void *data, Evas_Object *part_obj, Elm_Object_Item *item)
{
	layout_password_entry_object *password_entry = data;
	WIFI_RET_IF_FAIL(password_entry != NULL);

	evas_object_smart_callback_call(password_entry->password_entry, "clicked", NULL);
}

static char *_on_access_info_check(void *data, Evas_Object *obj)
{
	layout_password_entry_object *password_entry = data;
	WIFI_RET_VAL_IF_FAIL(password_entry != NULL, NULL);

	return g_strdup_printf("%s, %s, %s", STR_SHOW_PASSWORD, STR_TICKBOX_FOR_TTS,
			       elm_check_state_get(password_entry->password_checkbox) ? STR_TICK_FOR_TTS : STR_UNTICK_FOR_TTS);
}

static void _on_access_activate_check(void *data, Evas_Object *part_obj, Elm_Object_Item *item)
{
	layout_password_entry_object *password_entry = data;
	WIFI_RET_IF_FAIL(password_entry != NULL);

	__check_click(password_entry);
}

static Evas_Object *_register_access_object_to_edje_part(Evas_Object *layout,
							 const gchar *part)
{
	Evas_Object *to = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(layout), part);
	return elm_access_object_register(to, layout);
}

static void _set_custom_focus_chain_set(layout_password_entry_object *self)
{
	Evas_Object *ao_title, *ao_entry, *ao_check;
	Eina_List *l = NULL;

	ao_title = _register_access_object_to_edje_part(self->layout, "elm.access.title");
	elm_access_info_set(ao_title, ELM_ACCESS_INFO, STR_PASSWORD_HEADER);

	ao_entry = _register_access_object_to_edje_part(self->layout, "elm.image.entry");
	elm_access_info_set(ao_entry, ELM_ACCESS_INFO, STR_EDITFIELD_FOR_TTS);
	elm_access_activate_cb_set(ao_entry, _on_access_activate_entry, self);

	ao_check = _register_access_object_to_edje_part(self->layout, "elm.access.check");
	elm_access_info_cb_set(ao_check, ELM_ACCESS_INFO, _on_access_info_check, self);
	elm_access_activate_cb_set(ao_check, _on_access_activate_check, self);

	l = eina_list_append(l, ao_title);
	l = eina_list_append(l, ao_entry);
	l = eina_list_append(l, ao_check);
	elm_object_focus_custom_chain_set(self->layout, l);
}

layout_password_entry_object *layout_password_entry_new(view_base_object *base)
{
	layout_password_entry_object *object = NULL;
	if (!base)
		return NULL;
	object = g_new0(layout_password_entry_object, 1);
	if (!object) {
		LOGE("layout_password_entry_new() failed.");
		return NULL;
	}
	object->base = base;
	return object;
}

void layout_password_entry_free(layout_password_entry_object *object)
{
	if (object)
		g_free(object);
}

gboolean layout_password_entry_create(layout_password_entry_object *self)
{
	Evas_Object *naviframe = NULL;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_VAL_IF_FAIL(self, FALSE);
	WIFI_RET_VAL_IF_FAIL(self->base, FALSE);

	naviframe = view_base_get_naviframe(self->base);
	WIFI_RET_VAL_IF_FAIL(naviframe, FALSE);

	self->layout = _create_layout(self, naviframe);
	WIFI_RET_VAL_IF_FAIL(self->layout, FALSE);

	self->password_entry = _create_entry(self, self->layout);
	if (!self->password_entry) {
		layout_password_entry_destroy(self);
		return FALSE;
	}
	elm_object_part_content_set(self->layout,
				    "elm.swallow.entry", self->password_entry);

	self->password_checkbox = _create_checkbox(self, self->layout);
	if (!self->password_checkbox) {
		layout_password_entry_destroy(self);
		return FALSE;
	}
	elm_object_part_content_set(self->layout,
				    "elm.swallow.check", self->password_checkbox);

	_set_custom_focus_chain_set(self);

	return TRUE;
}

void layout_password_entry_destroy(layout_password_entry_object *self)
{
	WIFI_RET_IF_FAIL(self);
}

void layout_password_entry_show(layout_password_entry_object *self)
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

void layout_password_entry_pop(layout_password_entry_object *self)
{
	WIFI_RET_IF_FAIL(self);
	WIFI_RET_IF_FAIL(_is_in_naviframe(self));

	view_base_naviframe_item_pop(self->base);
}

void layout_password_entry_set_show_password(layout_password_entry_object *self,
					     Eina_Bool is_show)
{
	WIFI_RET_IF_FAIL(self != NULL);

	elm_entry_password_set(self->password_entry, !is_show);
}

void layout_password_entry_set_entry_text(layout_password_entry_object *self,
					  const gchar *text)
{
	gchar *markup_text;
	WIFI_RET_IF_FAIL(self != NULL);
	WIFI_RET_IF_FAIL(text != NULL);

	markup_text = elm_entry_utf8_to_markup(text);
	WIFI_RET_IF_FAIL(markup_text != NULL);

	elm_entry_entry_set(self->password_entry, markup_text);
	g_free(markup_text);
}

Eina_Bool layout_password_entry_checkbox_is_checked(layout_password_entry_object *self)
{
	WIFI_RET_VAL_IF_FAIL(self, EINA_FALSE);

	return elm_check_state_get(self->password_checkbox);
}

void layout_password_entry_set_ckeckbox_enable(layout_password_entry_object *self,
					       Eina_Bool is_enable)
{
	WIFI_RET_IF_FAIL(self);

	elm_object_disabled_set(self->password_checkbox, !is_enable);
}

void layout_password_entry_set_del_cb(layout_password_entry_object *self,
				      Evas_Object_Event_Cb func, void *data)
{
	WIFI_RET_IF_FAIL(self);

	self->del_cb.func = func;
	self->del_cb.data = data;
}

void layout_password_entry_set_entry_clicked_cb(layout_password_entry_object *self,
						Evas_Smart_Cb func, void *data)
{
	WIFI_RET_IF_FAIL(self);

	self->menu_cb[PASSWORD_ENTRY_ITEM_ENTRY].func = func;
	self->menu_cb[PASSWORD_ENTRY_ITEM_ENTRY].data = data;
}

void layout_password_entry_set_checkbox_changed_cb(layout_password_entry_object *self,
						   Evas_Smart_Cb func, void *data)
{
	WIFI_RET_IF_FAIL(self);

	self->menu_cb[PASSWORD_ENTRY_ITEM_CHECKBOX].func = func;
	self->menu_cb[PASSWORD_ENTRY_ITEM_CHECKBOX].data = data;
}
