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


#ifndef __VIEW_BASE_H__
#define __VIEW_BASE_H__

#include <Elementary.h>
#include <Evas.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _view_base_object view_base_object;

view_base_object *view_base_new();
void              view_base_free(view_base_object *self);

gboolean view_base_create(view_base_object *self);
void     view_base_destroy(view_base_object *self);

Evas_Object *view_base_get_window(view_base_object *self);
Evas_Object *view_base_get_naviframe(view_base_object *self);

void view_base_show(view_base_object *self);
void view_base_hide(view_base_object *self);

gboolean view_base_window_is_focus(view_base_object *self);

Elm_Object_Item *view_base_naviframe_push(view_base_object *self,
					  Evas_Object *layout, Evas_Object_Event_Cb del_cb, gpointer data);
void     view_base_naviframe_item_pop(view_base_object *self);
void     view_base_naviframe_item_pop_to(view_base_object *self, Elm_Object_Item *item);
gboolean view_base_frame_is_empty(view_base_object *self);
void     view_base_naviframe_add_transition_finished_cb(view_base_object *self,
							Evas_Smart_Cb func, const void *data);
void view_base_naviframe_del_transition_finished_cb(view_base_object *self,
						    Evas_Smart_Cb func);
void view_base_conformant_add_virtualkeypad_size_changed_cb(view_base_object *self,
							    Evas_Smart_Cb func, const void *data);
void view_base_conformant_del_virtualkeypad_size_changed_cb(view_base_object *self,
							    Evas_Smart_Cb func);

Evas_Object *view_base_add_genlist(view_base_object *self, Evas_Object *parent);
Evas_Object *view_base_add_genlist_for_circle(view_base_object *self, Evas_Object *parent, Evas_Object **circle_genlist);

Evas_Object *view_base_add_scroller(view_base_object *self, Evas_Object *parent);
Evas_Object *view_base_add_scroller_for_circle(view_base_object *self, Evas_Object *parent, Evas_Object **circle_scroller);

Evas_Object *view_base_add_progressbar(view_base_object *self, Evas_Object *parent);
Evas_Object *view_base_add_progressbar_for_circle(view_base_object *self, Evas_Object *parent);

Evas_Object *view_base_add_popup(view_base_object *self, Evas_Object *parent);
Evas_Object *view_base_add_popup_for_circle(view_base_object *self, Evas_Object *parent);

#ifdef __cplusplus
}
#endif

#endif /*__VIEW_BASE_H__*/
