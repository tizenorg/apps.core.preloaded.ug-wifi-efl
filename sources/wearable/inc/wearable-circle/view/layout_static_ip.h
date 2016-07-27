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


#ifndef __LAYOUT_STATIC_IP_H__
#define __LAYOUT_STATIC_IP_H__

#include "view/base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	STATIC_IP_ITEM_NONE = -1,
	STATIC_IP_ITEM_TITLE,
	STATIC_IP_ITEM_IP_ADDRESS,
	STATIC_IP_ITEM_SUBNET_MASK,
	STATIC_IP_ITEM_GATEWAY,
	STATIC_IP_ITEM_DNS1,
	STATIC_IP_ITEM_DNS2,
	STATIC_IP_ITEM_EMPTY,
	STATIC_IP_ITEM_SIZE
} static_ip_menu_type;

typedef struct _layout_static_ip_object layout_static_ip_object;

layout_static_ip_object *layout_static_ip_new(view_base_object *base);
void                     layout_static_ip_free(layout_static_ip_object *object);

gboolean layout_static_ip_create(layout_static_ip_object *self);
void     layout_static_ip_destroy(layout_static_ip_object *self);

void layout_static_ip_show(layout_static_ip_object *self);
void layout_static_ip_pop(layout_static_ip_object *self);
void layout_static_ip_pop_to(layout_static_ip_object *self);

void                layout_static_ip_select_menu(layout_static_ip_object *self, static_ip_menu_type menu);
static_ip_menu_type layout_static_ip_get_selected_menu(layout_static_ip_object *self);

void                layout_static_ip_update_menu(layout_static_ip_object *self, static_ip_menu_type type);
static_ip_menu_type layout_static_ip_get_menu_type(layout_static_ip_object *self,
						   Elm_Object_Item *item);

void layout_static_ip_save_button_set_enable(layout_static_ip_object *self);
void layout_static_ip_save_button_set_disable(layout_static_ip_object *self);

void layout_static_ip_activate_rotary_event(layout_static_ip_object *self);
void layout_static_ip_deactivate_rotary_event(layout_static_ip_object *self);

void layout_static_ip_set_del_cb(layout_static_ip_object *self,
				 Evas_Object_Event_Cb func, void *data);
const gchar *layout_static_ip_get_main_text(layout_static_ip_object *self,
					    static_ip_menu_type type);
void layout_static_ip_set_menu_cb(layout_static_ip_object *self, static_ip_menu_type type,
				  Elm_Gen_Item_Text_Get_Cb text_get, Elm_Gen_Item_Content_Get_Cb content_get,
				  Elm_Gen_Item_State_Get_Cb state_get, Elm_Gen_Item_Del_Cb del, void *data);
void layout_static_ip_set_tap_menu_cb(layout_static_ip_object *self,
				      static_ip_menu_type type, Evas_Smart_Cb func, gpointer data);
void layout_static_ip_set_tap_save_button_cb(layout_static_ip_object *self,
					     Evas_Smart_Cb func, gpointer data);

#ifdef __cplusplus
}
#endif

#endif /*__LAYOUT_STATIC_IP_H__*/
