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


#ifndef __LAYOUT_WEARABLE_INPUT_H__
#define __LAYOUT_WEARABLE_INPUT_H__

#include "view/base.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CUSTOM_GROUP_WEARABLE_INPUT "entry_layout"

typedef struct _layout_wearable_input_object layout_wearable_input_object;

layout_wearable_input_object *layout_wearable_input_new(view_base_object *base);
void                          layout_wearable_input_free(layout_wearable_input_object *object);

gboolean layout_wearable_input_create(layout_wearable_input_object *self);
void     layout_wearable_input_destroy(layout_wearable_input_object *self);

void layout_wearable_input_show(layout_wearable_input_object *self);
void layout_wearable_input_pop(layout_wearable_input_object *self);

void layout_wearable_input_prediction_on(layout_wearable_input_object *self);
void layout_wearable_input_prediction_off(layout_wearable_input_object *self);

void layout_wearable_input_set_input_type(layout_wearable_input_object *self,
					  Elm_Input_Panel_Layout type);
void layout_wearable_input_set_input_guide_text(layout_wearable_input_object *self,
						const gchar *text);
void layout_wearable_input_set_input_text(layout_wearable_input_object *self,
					  const gchar *text);
void layout_wearable_input_set_input_show(layout_wearable_input_object *self,
					  Eina_Bool is_show);
void layout_wearable_input_set_input_return_key_enable(layout_wearable_input_object *self,
						       Eina_Bool is_enable);
void layout_wearable_input_set_input_focus(layout_wearable_input_object *self,
					   Eina_Bool is_focus);
void layout_wearable_input_set_del_cb(layout_wearable_input_object *self,
				      Evas_Object_Event_Cb func, void *data);
void layout_wearable_input_set_input_changed_cb(layout_wearable_input_object *self,
						Evas_Smart_Cb func, void *data);
void layout_wearable_input_set_input_maxlength_reached_cb(layout_wearable_input_object *self,
							  int max_char_count, Evas_Smart_Cb func, void *data);
void layout_wearable_input_set_input_activated_cb(layout_wearable_input_object *self,
						  Evas_Smart_Cb func, void *data);

#ifdef __cplusplus
}
#endif

#endif /*__LAYOUT_WEARABLE_INPUT_H__*/
