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


#ifndef __LAYOUT_MAIN_H__
#define __LAYOUT_MAIN_H__

#include "view/base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	MAIN_MENU_TITLE = -1,
	MAIN_MENU_POWER,
	MAIN_MENU_SCAN,
	MAIN_MENU_SIZE
} main_menu_type;

typedef struct _layout_main_object layout_main_object;

layout_main_object *layout_main_new(view_base_object *base);
void                layout_main_free(layout_main_object *object);

gboolean layout_main_create(layout_main_object *data);
void     layout_main_destroy(layout_main_object *data);

void layout_main_pop_to(layout_main_object *self);

void layout_main_show(layout_main_object *data);

void layout_main_menu_set_access_info(layout_main_object *self, main_menu_type menu_type,
				      Elm_Access_Info_Type access_type, const gchar *text);
void layout_main_menu_set_enable(layout_main_object *self, main_menu_type type,
				 Eina_Bool is_enable);
void layout_main_menu_update(layout_main_object *self, main_menu_type type);
void layout_main_menu_show(layout_main_object *self, main_menu_type type);
void layout_main_activate_rotary_event(layout_main_object *self);
void layout_main_deactivate_rotary_event(layout_main_object *self);

void layout_main_set_del_cb(layout_main_object *main_object,
			    Evas_Object_Event_Cb func, void *data);
void layout_main_set_menu_cb(layout_main_object *main_object, main_menu_type type,
			     Elm_Gen_Item_Text_Get_Cb text_get, Elm_Gen_Item_Content_Get_Cb content_get,
			     Evas_Smart_Cb tap, void *data);

#ifdef __cplusplus
}
#endif

#endif /*__LAYOUT_MAIN_H__*/
