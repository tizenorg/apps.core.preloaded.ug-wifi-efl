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


#ifndef __LAYOUT_AP_INFO_H__
#define __LAYOUT_AP_INFO_H__

#include "view/base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	AP_INFO_MENU_TITLE,
	AP_INFO_MENU_WPS,
	AP_INFO_MENU_EAP,
	AP_INFO_MENU_PASSWORD,
	AP_INFO_MENU_STATIC,
	AP_INFO_MENU_PROXY,
	AP_INFO_MENU_EMPTY,
	AP_INFO_MENU_SIZE,
} ap_info_menu_type;

typedef struct _layout_ap_info_object layout_ap_info_object;

layout_ap_info_object *layout_ap_info_new(view_base_object *base);
void                   layout_ap_info_free(layout_ap_info_object *object);

gboolean layout_ap_info_create(layout_ap_info_object *self);
void     layout_ap_info_destroy(layout_ap_info_object *self);

void layout_ap_info_pop_to(layout_ap_info_object *self);

void layout_ap_info_open_show(layout_ap_info_object *self, void *menu_data);
void layout_ap_info_wps_show(layout_ap_info_object *self, void *menu_data);
void layout_ap_info_eap_show(layout_ap_info_object *self, void *menu_data);
void layout_ap_info_security_show(layout_ap_info_object *self, void *menu_data);

void layout_ap_info_menu_set_access_info(layout_ap_info_object *self, ap_info_menu_type menu_type,
					 Elm_Access_Info_Type access_type, const gchar *text);
void layout_ap_info_menu_update(layout_ap_info_object *self, ap_info_menu_type type);
void layout_ap_info_set_connect_button_enable(layout_ap_info_object *self,
					      Eina_Bool is_enable);
void layout_ap_info_activate_rotary_event(layout_ap_info_object *self);
void layout_ap_info_deactivate_rotary_event(layout_ap_info_object *self);

void layout_ap_info_set_del_cb(layout_ap_info_object *self,
			       Evas_Object_Event_Cb func, void *data);
void layout_ap_info_set_menu_cb(layout_ap_info_object *self, ap_info_menu_type type,
				Elm_Gen_Item_Text_Get_Cb text_get, Elm_Gen_Item_Content_Get_Cb content_get,
				Elm_Gen_Item_State_Get_Cb state_get, Elm_Gen_Item_Del_Cb del,
				Evas_Smart_Cb tap, void *data);
void layout_ap_info_set_tap_connect_button_cb(layout_ap_info_object *self,
					      Evas_Smart_Cb func, void *data);

#ifdef __cplusplus
}
#endif

#endif /*__LAYOUT_AP_INFO_H__*/
