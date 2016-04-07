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


#ifndef __POPUP_CHANGE_AP_H__
#define __POPUP_CHANGE_AP_H__

#include "view/base.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CUSTOM_GROUP_BODY_BUTTON "popup_body_button"

typedef enum {
	CHANGE_AP_DISMISS_DEFAULT,
	CHANGE_AP_DISMISS_CANCEL,
	CHANGE_AP_DISMISS_CONNECT,
	CHANGE_AP_DISMISS_FORGET
} popup_change_ap_dismiss_reason;

typedef struct _popup_change_ap_object popup_change_ap_object;

popup_change_ap_object *popup_change_ap_new(view_base_object *base);
void                    popup_change_ap_free(popup_change_ap_object *object);

gboolean popup_change_ap_create(popup_change_ap_object *self);
gboolean popup_change_ap_create_hidden_forgetbutton(popup_change_ap_object *self);
void     popup_change_ap_destroy(popup_change_ap_object *self);

void popup_change_ap_show(popup_change_ap_object *self);
void popup_change_ap_dismiss(popup_change_ap_object *self,
			     popup_change_ap_dismiss_reason reason);

popup_change_ap_dismiss_reason popup_change_ap_get_dismiss_reason(popup_change_ap_object *self);

void popup_change_ap_set_destroy_cb(popup_change_ap_object *self,
				    Ea_Event_Cb func, gpointer data);
void popup_change_ap_set_tap_ok_button_cb(popup_change_ap_object *self,
					  Evas_Smart_Cb func, gpointer data);
void popup_change_ap_set_tap_forget_button_cb(popup_change_ap_object *self,
					      Evas_Smart_Cb func, gpointer data);
void popup_change_ap_set_rssi_text(popup_change_ap_object *self,
				   const gchar *rssi_text);

#ifdef __cplusplus
}
#endif

#endif /*__POPUP_CHANGE_AP_H__*/
