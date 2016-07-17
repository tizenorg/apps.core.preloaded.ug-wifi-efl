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


#ifndef __LAYOUT_DETAIL_H__
#define __LAYOUT_DETAIL_H__

#include "view/base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	DETAIL_MENU_TITLE,
	DETAIL_MENU_STATUS,
	DETAIL_MENU_STRENGTH,
	DETAIL_MENU_SPEED,
	DETAIL_MENU_IP,
	DETAIL_MENU_EMPTY,
	DETAIL_MENU_SIZE
} layout_detail_menu_type;

typedef struct _layout_detail_object layout_detail_object;

layout_detail_object *layout_detail_new(view_base_object *base);
void                  layout_detail_free(layout_detail_object *object);

gboolean layout_detail_create(layout_detail_object *self);
gboolean layout_detail_create_hidden_forgetbutton(layout_detail_object *self);
void     layout_detail_destroy(layout_detail_object *self);

void layout_detail_show(layout_detail_object *self);

void layout_detail_activate_rotary_event(layout_detail_object *self);
void layout_detail_deactivate_rotary_event(layout_detail_object *self);

void layout_detail_set_del_cb(layout_detail_object *self,
			      Evas_Object_Event_Cb func, void *data);
void layout_detail_set_menu_cb(layout_detail_object *self, layout_detail_menu_type type,
			       Elm_Gen_Item_Text_Get_Cb text_get, Elm_Gen_Item_Content_Get_Cb content_get,
			       Elm_Gen_Item_State_Get_Cb state_get, Elm_Gen_Item_Del_Cb del, gpointer data);
void layout_detail_set_tap_forget_button_cb(layout_detail_object *self,
					    Evas_Smart_Cb func, gpointer data);

#ifdef __cplusplus
}
#endif

#endif /*__LAYOUT_DETAIL_H__*/
