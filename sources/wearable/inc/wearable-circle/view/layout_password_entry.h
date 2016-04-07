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


#ifndef __LAYOUT_PASSWORD_ENTRY_H__
#define __LAYOUT_PASSWORD_ENTRY_H__

#include "view/base.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CUSTOM_GROUP_PASSWORD_ENTRY "password_layout"

typedef struct _layout_password_entry_object layout_password_entry_object;

layout_password_entry_object *layout_password_entry_new(view_base_object *base);
void                          layout_password_entry_free(layout_password_entry_object *object);

gboolean layout_password_entry_create(layout_password_entry_object *self);
void     layout_password_entry_destroy(layout_password_entry_object *self);

void layout_password_entry_show(layout_password_entry_object *self);
void layout_password_entry_pop(layout_password_entry_object *self);

void layout_password_entry_set_show_password(layout_password_entry_object *self,
					     Eina_Bool is_show);
void layout_password_entry_set_entry_text(layout_password_entry_object *self,
					  const gchar *text);
Eina_Bool layout_password_entry_checkbox_is_checked(layout_password_entry_object *self);
void      layout_password_entry_set_ckeckbox_enable(layout_password_entry_object *self,
						    Eina_Bool is_enable);

void layout_password_entry_set_del_cb(layout_password_entry_object *self,
				      Evas_Object_Event_Cb func, void *data);
void layout_password_entry_set_entry_clicked_cb(layout_password_entry_object *self,
						Evas_Smart_Cb func, void *data);
void layout_password_entry_set_checkbox_changed_cb(layout_password_entry_object *self,
						   Evas_Smart_Cb func, void *data);

#ifdef __cplusplus
}
#endif

#endif /*__LAYOUT_PASSWORD_ENTRY_H__*/
