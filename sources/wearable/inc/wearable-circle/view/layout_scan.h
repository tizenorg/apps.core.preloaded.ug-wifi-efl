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


#ifndef __LAYOUT_SCAN_H__
#define __LAYOUT_SCAN_H__

#include "view/base.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LAYOUT_SCAN_DATA_KEY_WIFI_AP_ITEM_SELECT "select-ap"

typedef enum {
	SCAN_MENU_TITLE = -1,
	SCAN_MENU_WIFI_AP_ITEM,
	SCAN_MENU_SCAN_BUTTON,
	SCAN_MENU_SIZE
} scan_menu_type;

typedef struct _layout_scan_object layout_scan_object;

layout_scan_object *layout_scan_new(view_base_object *base);
void                layout_scan_free(layout_scan_object *object);

gboolean layout_scan_create(layout_scan_object *self);
void     layout_scan_destroy(layout_scan_object *self);

void     layout_scan_pop_to(layout_scan_object *self);
gboolean layout_scan_is_top(layout_scan_object *self);

void layout_scan_no_ap_show(layout_scan_object *self);

void layout_scan_ap_list_show(layout_scan_object *self);
void layout_scan_ap_list_item_move_to_top(layout_scan_object *self,
					  Elm_Object_Item *item);
void layout_scan_ap_list_top_item_show(layout_scan_object *self);
void layout_scan_ap_list_update(layout_scan_object *self);
void layout_scan_ap_list_update_item_by_data(layout_scan_object *self,
					     gpointer data, GCompareFunc cmp);
Elm_Object_Item *layout_scan_ap_list_find_item_by_data(layout_scan_object *self,
						       gpointer data, GCompareFunc cmp);
gboolean layout_scan_ap_list_is_realized_item(layout_scan_object *self,
					      Elm_Object_Item *item);
void layout_scan_ap_list_set_data_list(layout_scan_object *self,
				       GList *list);
void layout_scan_ap_list_sort_data_list(layout_scan_object *self,
					GCompareFunc cmp);
void layout_scan_ap_list_append_data(layout_scan_object *self,
				     gpointer data);
void layout_scan_ap_list_clear_data(layout_scan_object *self);
void layout_scan_ap_list_activate_rotary_event(layout_scan_object *self);
void layout_scan_ap_list_deactivate_rotary_event(layout_scan_object *self);

void layout_scan_set_del_cb(layout_scan_object *self,
			    Evas_Object_Event_Cb func, void *data);
void layout_scan_set_ap_data_del_cb(layout_scan_object *self,
				    GDestroyNotify func);
void layout_scan_set_menu_cb(layout_scan_object *self, scan_menu_type type,
			     Elm_Gen_Item_Text_Get_Cb text_get, Elm_Gen_Item_Content_Get_Cb content_get,
			     Elm_Gen_Item_State_Get_Cb state_get, Elm_Gen_Item_Del_Cb del,
			     Evas_Smart_Cb tap, void *data);
void layout_scan_set_scan_button_tap_cb(layout_scan_object *self,
					Evas_Smart_Cb func, void *data);

#ifdef __cplusplus
}
#endif

#endif /*__LAYOUT_SCAN_H__*/
