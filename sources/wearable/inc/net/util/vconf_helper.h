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


#ifndef __UTIL_VCONF_HELPER_H__
#define __UTIL_VCONF_HELPER_H__

#include <glib.h>
#include <vconf.h>
#include "util/log.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VCONF_WIFI_USE          "db/private/wifi/wearable_wifi_use"
#define VCONF_WIFI_STATE        "memory/wifi/state"

static inline gboolean vconf_is_wifi_use()
{
	int wifi_use = -1;
	int result = vconf_get_int(VCONF_WIFI_STATE, &wifi_use);
	if (result < 0) {
		WIFI_LOG_ERR("vconf_get_int error. result = %d", result);
	}

    if (wifi_use > 0) return TRUE;
    else return FALSE;
}


static inline gboolean vconf_set_wifi_use(const int intval)
{
#if 0
	return vconf_set_int(VCONF_WIFI_USE, intval) == 0;
#endif
    return TRUE;
}


static inline gboolean vconf_is_skt()
{
	char *code = vconf_get_str(VCONFKEY_CSC_SALESCODE);
	if (code) {
		gboolean is_skt = !g_strcmp0(code, "SKC") || !g_strcmp0(code, "SKO");
		WIFI_LOG_INFO("Target salescode is %s.", code);
		g_free(code);
		return is_skt;
	}
	WIFI_LOG_ERR("vconf_get_str error.(result is NULL)");
	return FALSE;
}

static inline gboolean vconf_is_wearable_debugging_mode(void)
{
	int debugging_mode = 0;
	int result = vconf_get_bool(VCONFKEY_SETAPPL_USB_DEBUG_MODE_BOOL, &debugging_mode);
	if (result < 0) {
		WIFI_LOG_ERR("get VCONFKEY_SETAPPL_USB_DEBUG_MODE_BOOL error. "
			     "result = %d", result);
		return FALSE;
	}
	WIFI_LOG_INFO("VCONFKEY_SETAPPL_USB_DEBUG_MODE_BOOL = %d", debugging_mode);
	return debugging_mode == 1;
}

static inline gboolean vconf_notify_wifi_key_changed(vconf_callback_fn cb,
						     void *user_data)
{
	return vconf_notify_key_changed(VCONF_WIFI_USE, cb, user_data) == 0;
}

#ifdef __cplusplus
}
#endif

#endif /* __UTIL_VCONF_HELPER_H__ */
