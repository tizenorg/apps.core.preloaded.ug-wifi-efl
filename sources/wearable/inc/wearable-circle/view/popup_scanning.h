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


#ifndef __POPUP_SCANNING_H__
#define __POPUP_SCANNING_H__

#include "view/base.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CUSTOM_GROUP_SCANNING "progressbar_layout"

typedef struct _popup_scanning_object popup_scanning_object;

popup_scanning_object *popup_scanning_new(view_base_object *base);
void                   popup_scanning_free(popup_scanning_object *object);

gboolean popup_scanning_create(popup_scanning_object *self);
void     popup_scanning_destroy(popup_scanning_object *self);

void popup_scanning_show(popup_scanning_object *self);
void popup_scanning_dismiss(popup_scanning_object *self);

void popup_scanning_set_destroy_cb(popup_scanning_object *self,
				   Ea_Event_Cb func, void *data);

#ifdef __cplusplus
}
#endif

#endif /*__POPUP_SCANNING_H__*/
