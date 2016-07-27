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


#ifndef __LAYOUT_PROXY_SETTING_H__
#define __LAYOUT_PROXY_SETTING_H__

#include "view/base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	PROXY_SETTING_ITEM_NONE = -1,
	PROXY_SETTING_ITEM_TITLE,
	PROXY_SETTING_ITEM_ADDRESS,
	PROXY_SETTING_ITEM_PORT,
	PROXY_SETTING_ITEM_EMPTY,
	PROXY_SETTING_ITEM_SIZE
} proxy_setting_menu_type;

typedef struct _layout_proxy_setting_object layout_proxy_setting_object;

layout_proxy_setting_object *layout_proxy_setting_new(view_base_object *base);
void                         layout_proxy_setting_free(layout_proxy_setting_object *object);

gboolean layout_proxy_setting_create(layout_proxy_setting_object *self);
void     layout_proxy_setting_destroy(layout_proxy_setting_object *self);

void layout_proxy_setting_show(layout_proxy_setting_object *self);
void layout_proxy_setting_pop(layout_proxy_setting_object *self);
void layout_proxy_setting_pop_to(layout_proxy_setting_object *self);

void                    layout_proxy_setting_select_menu(layout_proxy_setting_object *self, proxy_setting_menu_type menu);
proxy_setting_menu_type layout_proxy_setting_get_selected_menu(layout_proxy_setting_object *self);

void                    layout_proxy_setting_update_menu(layout_proxy_setting_object *self, proxy_setting_menu_type type);
proxy_setting_menu_type layout_proxy_setting_get_menu_type(layout_proxy_setting_object *self,
							   Elm_Object_Item *item);

void layout_proxy_setting_activate_rotary_event(layout_proxy_setting_object *self);
void layout_proxy_setting_deactivate_rotary_event(layout_proxy_setting_object *self);

void layout_proxy_setting_set_del_cb(layout_proxy_setting_object *self,
				     Evas_Object_Event_Cb func, void *data);
const gchar *layout_proxy_setting_get_main_text(layout_proxy_setting_object *self,
						proxy_setting_menu_type type);
void layout_proxy_setting_set_menu_cb(layout_proxy_setting_object *self, proxy_setting_menu_type type,
				      Elm_Gen_Item_Text_Get_Cb text_get, Elm_Gen_Item_Content_Get_Cb content_get,
				      Elm_Gen_Item_State_Get_Cb state_get, Elm_Gen_Item_Del_Cb del, void *data);
void layout_proxy_setting_set_tap_menu_cb(layout_proxy_setting_object *self,
					  proxy_setting_menu_type type, Evas_Smart_Cb func, gpointer data);
void layout_proxy_setting_set_tap_save_button_cb(layout_proxy_setting_object *self,
						 Evas_Smart_Cb func, gpointer data);

#ifdef __cplusplus
}
#endif

#endif /*__LAYOUT_PROXY_SETTING_H__*/
