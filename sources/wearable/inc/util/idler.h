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


#ifndef __UTIL_IDLER_H__
#define __UTIL_IDLER_H__

#include <glib.h>

#ifdef __cplusplus
extern "C"
{
#endif

guint idler_util_managed_idle_add(GSourceFunc func, gpointer user_data);
void  idler_util_managed_idle_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif /* __UTIL_IDLER_H__ */
