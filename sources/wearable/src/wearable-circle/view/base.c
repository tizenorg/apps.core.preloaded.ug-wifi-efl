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
#include <efl_util.h>
#include <efl_assist.h>
#include <efl_extension.h>
#include <glib.h>

#include "util.h"
#include "view/util/efl_helper.h"
#include "view/base.h"

struct _view_base_object {
	Evas_Object *window;
	Evas_Object *conformant;
	Evas_Object *bg;
	Evas_Object *naviframe;
	Eext_Circle_Surface *circle_surface;
};

static Evas_Object *_create_window(const gchar *name)
{
	Evas_Object *window = elm_win_add(NULL, name, ELM_WIN_BASIC);
	int width = 0, height = 0;
	WIFI_RET_VAL_IF_FAIL(window, NULL);

	elm_win_title_set(window, name);
	elm_win_borderless_set(window, EINA_TRUE);
	elm_win_screen_size_get(window, NULL, NULL, &width, &height);
	evas_object_resize(window, width, height);

	if (elm_win_wm_rotation_supported_get(window)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(window,
							    (const int *)(&rots), 4);
	}

	efl_util_set_notification_window_level(window, EFL_UTIL_NOTIFICATION_LEVEL_1);
	return window;
}

static Evas_Object *_create_conformant(Evas_Object *parent)
{
	Evas_Object *conformant = elm_conformant_add(parent);
	WIFI_RET_VAL_IF_FAIL(conformant, NULL);

	evas_object_size_hint_weight_set(conformant, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_layout_theme_set(conformant, "conformant", "base", "without_resize");
	return conformant;
}

static Eext_Circle_Surface *_create_circle_surface_from_naviframe(Evas_Object *naviframe)
{
	Eext_Circle_Surface *circle_surface;
	WIFI_RET_VAL_IF_FAIL(naviframe, NULL);

	circle_surface = eext_circle_surface_naviframe_add(naviframe);
	return circle_surface;
}

static Evas_Object *_create_bg(Evas_Object *parent)
{
	Evas_Object *bg = elm_bg_add(parent);
	if (bg) {
		evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_show(bg);
	}
	return bg;
}

static Evas_Object *_create_naviframe(Evas_Object *parent)
{
	Evas_Object *naviframe = elm_naviframe_add(parent);
	if (naviframe) {
		evas_object_size_hint_weight_set(naviframe, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		eext_object_event_callback_add(naviframe, EEXT_CALLBACK_BACK, eext_naviframe_back_cb, NULL);
	}
	return naviframe;
}

view_base_object *view_base_new()
{
	return g_new0(view_base_object, 1);
}

void view_base_free(view_base_object *self)
{
	WIFI_RET_IF_FAIL(self);

	g_free(self);
}

gboolean view_base_create(view_base_object *self)
{
	if (!self) {
		WIFI_LOG_ERR("view_base_object is NULL.");
		return FALSE;
	}

	self->window = _create_window(PACKAGE);
	if (!self->window) {
		WIFI_LOG_ERR("_create_window() is failed.");
		view_base_destroy(self);
		return FALSE;
	}
	evas_object_show(self->window);

	self->conformant = _create_conformant(self->window);
	if (!self->conformant) {
		WIFI_LOG_ERR("_create_conformant() is failed.");
		view_base_destroy(self);
		return FALSE;
	}
	elm_win_resize_object_add(self->window, self->conformant);
	elm_win_conformant_set(self->window, EINA_TRUE);
	evas_object_show(self->conformant);

	self->bg = _create_bg(self->conformant);
	if (!self->bg) {
		WIFI_LOG_ERR("_create_bg() is failed.");
		view_base_destroy(self);
		return FALSE;
	}
	elm_object_part_content_set(self->conformant, "elm.swallow.bg", self->bg);

	self->naviframe = _create_naviframe(self->conformant);
	if (!self->naviframe) {
		WIFI_LOG_ERR("_create_naviframe() is failed.");
		view_base_destroy(self);
		return FALSE;
	}
	elm_object_content_set(self->conformant, self->naviframe);

	self->circle_surface = _create_circle_surface_from_naviframe(self->naviframe);
	if (!self->circle_surface) {
		WIFI_LOG_ERR("_create_circle_surface_from_conformant() is failed.");
		view_base_destroy(self);
		return FALSE;
	}

	return TRUE;
}

void view_base_destroy(view_base_object *self)
{
	WIFI_RET_IF_FAIL(self);

	if (self->circle_surface) {
		eext_circle_surface_del(self->circle_surface);
		self->circle_surface = NULL;
	}

	evas_object_del(self->naviframe);
	evas_object_del(self->bg);
	evas_object_del(self->conformant);
	evas_object_del(self->window);
	self->naviframe = self->conformant = self->window = NULL;
}

Evas_Object *view_base_get_window(view_base_object *self)
{
	return self->window;
}

Evas_Object *view_base_get_naviframe(view_base_object *self)
{
	return self->naviframe;
}

void view_base_show(view_base_object *self)
{
	WIFI_RET_IF_FAIL(self);

	evas_object_show(self->window);
	evas_object_show(self->conformant);
	evas_object_show(self->bg);
	evas_object_show(self->naviframe);
}

void view_base_hide(view_base_object *self)
{
	WIFI_RET_IF_FAIL(self);

	evas_object_hide(self->naviframe);
	evas_object_hide(self->bg);
	evas_object_hide(self->conformant);
	evas_object_hide(self->window);
}

gboolean view_base_window_is_focus(view_base_object *self)
{
	Eina_Bool ret;
	WIFI_RET_VAL_IF_FAIL(self, FALSE);
	WIFI_RET_VAL_IF_FAIL(self->window, FALSE);

	ret = elm_win_focus_get(self->window);
	return ret == EINA_TRUE ? TRUE : FALSE;

}

Elm_Object_Item *view_base_naviframe_push(view_base_object *self,
					  Evas_Object *layout, Evas_Object_Event_Cb del_cb, gpointer data)
{
	Elm_Object_Item *item;
	WIFI_RET_VAL_IF_FAIL(self, NULL);
	WIFI_RET_VAL_IF_FAIL(self->naviframe, NULL);

	item = elm_naviframe_item_push(self->naviframe, NULL, NULL, NULL, layout, "empty");
	WIFI_RET_VAL_IF_FAIL(item, NULL);

	elm_naviframe_item_title_enabled_set(item, EINA_FALSE, EINA_FALSE);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_DEL, del_cb, data);
	return item;
}

void view_base_naviframe_item_pop(view_base_object *self)
{
	WIFI_RET_IF_FAIL(self);

	elm_naviframe_item_pop(self->naviframe);
}

void view_base_naviframe_item_pop_to(view_base_object *self, Elm_Object_Item *item)
{
	WIFI_RET_IF_FAIL(self);

	elm_naviframe_item_pop_to(item);
}

gboolean view_base_frame_is_empty(view_base_object *self)
{
	return self && !elm_naviframe_top_item_get(self->naviframe);
}

void view_base_naviframe_add_transition_finished_cb(view_base_object *self,
						    Evas_Smart_Cb func, const void *data)
{
	WIFI_RET_IF_FAIL(self);

	evas_object_smart_callback_add(self->naviframe,
				       "transition,finished", func, data);
}

void view_base_naviframe_del_transition_finished_cb(view_base_object *self,
						    Evas_Smart_Cb func)
{
	WIFI_RET_IF_FAIL(self);

	evas_object_smart_callback_del(self->naviframe,
				       "transition,finished", func);
}

void view_base_conformant_add_virtualkeypad_size_changed_cb(view_base_object *self,
							    Evas_Smart_Cb func, const void *data)
{
	WIFI_RET_IF_FAIL(self != NULL);

	evas_object_smart_callback_add(self->conformant,
				       "virtualkeypad,size,changed", func, data);
}

void view_base_conformant_del_virtualkeypad_size_changed_cb(view_base_object *self,
							    Evas_Smart_Cb func)
{
	WIFI_RET_IF_FAIL(self != NULL);

	evas_object_smart_callback_del(self->conformant,
				       "virtualkeypad,size,changed", func);
}

Evas_Object *view_base_add_genlist(view_base_object *self, Evas_Object *parent)
{
	Evas_Object *genlist = NULL;
	WIFI_RET_VAL_IF_FAIL(self, NULL);
	WIFI_RET_VAL_IF_FAIL(parent, NULL);

	genlist = elm_genlist_add(parent);
	WIFI_RET_VAL_IF_FAIL(genlist, NULL);

	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	return genlist;
}

Evas_Object *view_base_add_genlist_for_circle(view_base_object *self, Evas_Object *parent, Evas_Object **circle_genlist)
{
	Evas_Object *genlist = NULL;
	WIFI_RET_VAL_IF_FAIL(self, NULL);
	WIFI_RET_VAL_IF_FAIL(parent, NULL);

	genlist = view_base_add_genlist(self, parent);
	WIFI_RET_VAL_IF_FAIL(genlist, NULL);
#if 0
	uxt_genlist_set_bottom_margin_enabled(genlist, EINA_TRUE);
#endif
	*circle_genlist = eext_circle_object_genlist_add(genlist, self->circle_surface);
	if (!(*circle_genlist)) {
		evas_object_del(genlist);
		return NULL;
	}
	eext_circle_object_genlist_scroller_policy_set(*circle_genlist,
						       ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	eext_rotary_object_event_activated_set(*circle_genlist, EINA_TRUE);

	return genlist;
}

Evas_Object *view_base_add_scroller(view_base_object *self, Evas_Object *parent)
{
	WIFI_RET_VAL_IF_FAIL(self, NULL);
	WIFI_RET_VAL_IF_FAIL(parent, NULL);

	return elm_scroller_add(parent);
}

Evas_Object *view_base_add_scroller_for_circle(view_base_object *self, Evas_Object *parent, Evas_Object **circle_scroller)
{
	Evas_Object *scroller = NULL;
	WIFI_RET_VAL_IF_FAIL(self, NULL);
	WIFI_RET_VAL_IF_FAIL(parent, NULL);

	scroller = view_base_add_scroller(self, parent);
	WIFI_RET_VAL_IF_FAIL(scroller, NULL);

	*circle_scroller = eext_circle_object_scroller_add(scroller, self->circle_surface);
	if (!(*circle_scroller)) {
		evas_object_del(scroller);
		return NULL;
	}
	eext_circle_object_scroller_policy_set(*circle_scroller,
					       ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	eext_rotary_object_event_activated_set(*circle_scroller, EINA_TRUE);
	return scroller;
}

Evas_Object *view_base_add_progressbar(view_base_object *self, Evas_Object *parent)
{
	WIFI_RET_VAL_IF_FAIL(self, NULL);
	WIFI_RET_VAL_IF_FAIL(parent, NULL);

	return elm_progressbar_add(parent);
}

Evas_Object *view_base_add_progressbar_for_circle(view_base_object *self, Evas_Object *parent)
{
	WIFI_RET_VAL_IF_FAIL(self, NULL);
	WIFI_RET_VAL_IF_FAIL(parent, NULL);

	return eext_circle_object_progressbar_add(parent, self->circle_surface);
}

Evas_Object *view_base_add_popup(view_base_object *self, Evas_Object *parent)
{
	WIFI_RET_VAL_IF_FAIL(self, NULL);
	WIFI_RET_VAL_IF_FAIL(parent, NULL);

	return elm_popup_add(parent);
}

Evas_Object *view_base_add_popup_for_circle(view_base_object *self, Evas_Object *parent)
{
	Evas_Object *popup = NULL;
	WIFI_RET_VAL_IF_FAIL(self, NULL);
	WIFI_RET_VAL_IF_FAIL(parent, NULL);

	popup = elm_popup_add(parent);
	WIFI_RET_VAL_IF_FAIL(popup, NULL);

	elm_object_style_set(popup, "circle");
#if 0
	uxt_popup_set_rotary_event_enabled(popup, EINA_TRUE);
#endif
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	return popup;
}
