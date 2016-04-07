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


#ifndef __TOAST_POPUP_H__
#define __TOAST_POPUP_H__

#include "view/base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _toast_popup_object toast_popup_object;

toast_popup_object *toast_popup_new(view_base_object *base);
void                toast_popup_free(toast_popup_object *object);

gboolean toast_popup_create(toast_popup_object *self, const gchar *msg);
void     toast_popup_destroy(toast_popup_object *self);

void toast_popup_show(toast_popup_object *self);

#ifdef __cplusplus
}
#endif

#endif /*__TOAST_POPUP_H__*/
