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


#ifndef __UTIL_LOG_H__
#define __UTIL_LOG_H__

#include <dlog.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "WIFI_EFL"

#define WIFI_LOG(priority, format, args ...) \
	ALOG(priority, LOG_TAG, format, ## args)

#define WIFI_LOG_DBG(format, args ...) \
	do { WIFI_LOG(LOG_DEBUG, format, ## args); } while (0)
#define WIFI_LOG_INFO(format, args ...)	\
	do { WIFI_LOG(LOG_INFO, format, ## args); } while (0)
#define WIFI_LOG_WARN(format, args ...)	\
	do { WIFI_LOG(LOG_WARN, format, ## args); } while (0)
#define WIFI_LOG_ERR(format, args ...) \
	do { WIFI_LOG(LOG_ERROR, format, ## args); } while (0)

#define __WIFI_FUNC_ENTER__				\
	WIFI_LOG(LOG_INFO, "[Enter]")
#define __WIFI_FUNC_EXIT__					\
	WIFI_LOG(LOG_INFO, "[Quit]")

#define WIFI_RET_IF_FAIL(expr)				\
	do { if (expr) {} else {			       \
		     WIFI_LOG_ERR("'" # expr "' failed.");	      \
		     return; }							     \
	} while (0)

#define WIFI_RET_VAL_IF_FAIL(expr, val)		\
	do { if (expr) {} else {			       \
		     WIFI_LOG_ERR("'" # expr "' failed.");	      \
		     return (val); }					     \
	} while (0)

#ifdef __cplusplus
}
#endif

#endif /* __UTIL_LOG_H__ */
