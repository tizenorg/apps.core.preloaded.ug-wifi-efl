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


#ifndef __LAYOUT_WPS_METHOD_H__
#define __LAYOUT_WPS_METHOD_H__

#include "view/base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	WPS_METHOD_MENU_TITLE = -1,
	WPS_METHOD_MENU_WPS_BUTTON,
	WPS_METHOD_MENU_WPS_PIN,
	WPS_METHOD_MENU_SIZE
} wps_method_menu_type;

typedef struct _layout_wps_method_object layout_wps_method_object;

layout_wps_method_object *layout_wps_method_new(view_base_object *base);
void                      layout_wps_method_free(layout_wps_method_object *object);

gboolean layout_wps_method_create(layout_wps_method_object *self, void *menu_data);
void     layout_wps_method_destroy(layout_wps_method_object *self);

void layout_wps_method_show(layout_wps_method_object *self);

void layout_wps_method_pop(layout_wps_method_object *self);

void layout_wps_method_activate_rotary_event(layout_wps_method_object *self);
void layout_wps_method_deactivate_rotary_event(layout_wps_method_object *self);

void layout_wps_method_set_del_cb(layout_wps_method_object *self,
				  Evas_Object_Event_Cb func, void *data);
void layout_wps_method_set_menu_cb(layout_wps_method_object *self, wps_method_menu_type type,
				   Elm_Gen_Item_Text_Get_Cb text_get, Elm_Gen_Item_Content_Get_Cb content_get,
				   Elm_Gen_Item_State_Get_Cb state_get, Elm_Gen_Item_Del_Cb del,
				   Evas_Smart_Cb tap, void *data);

#ifdef __cplusplus
}
#endif

#endif /*__LAYOUT_WPS_METHOD_H__*/
