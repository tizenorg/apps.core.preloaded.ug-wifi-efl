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


#ifndef __LAYOUT_WPS_PROGRESS_H__
#define __LAYOUT_WPS_PROGRESS_H__

#include "view/base.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CUSTOM_GROUP_WPS_PROGRESS                               "wps_progress_layout"
#define CUSTOM_GROUP_WPS_PROGRESS_POPUP                               "wps_progress_popup_layout"
#define CUSTOM_GROUP_WPS_PROGRESS_TEXT_BLOCK    "wps_text_block_layout"

typedef struct _layout_wps_progress_object layout_wps_progress_object;

layout_wps_progress_object *layout_wps_progress_new(view_base_object *base);
void                        layout_wps_progress_free(layout_wps_progress_object *object);

gboolean layout_wps_progress_create(layout_wps_progress_object *self);
void     layout_wps_progress_destroy(layout_wps_progress_object *self);

void layout_wps_progress_show(layout_wps_progress_object *self);
void layout_wps_progress_dismiss(layout_wps_progress_object *self);

void layout_wps_progress_activate_rotary_event(layout_wps_progress_object *self);

void layout_wps_progress_set_show_finished_cb(layout_wps_progress_object *self,
					      Evas_Smart_Cb func, void *data);
void layout_wps_progress_set_destroy_cb(layout_wps_progress_object *self,
					Ea_Event_Cb func, void *data);
void layout_wps_progress_set_label_text(layout_wps_progress_object *self,
					const gchar *label_text);

#ifdef __cplusplus
}
#endif

#endif /*__LAYOUT_WPS_PROGRESS_H__*/
