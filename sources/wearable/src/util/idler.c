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


#include <glib.h>
#include "util/log.h"
#include "util/idler.h"

struct managed_idle_data {
	GSourceFunc func;
	gpointer user_data;
	guint id;
};

static GSList *managed_idler_list = NULL;

static gboolean __idler_util_managed_idle_cb(gpointer user_data)
{
	struct managed_idle_data *data = (struct managed_idle_data *)user_data;

	WIFI_RET_VAL_IF_FAIL(data != NULL, FALSE);

	return data->func(data->user_data);
}

guint idler_util_managed_idle_add(GSourceFunc func, gpointer user_data)
{
	guint id;
	struct managed_idle_data *data;

	WIFI_RET_VAL_IF_FAIL(func != NULL, 0);

	data = g_try_new0(struct managed_idle_data, 1);
	WIFI_RET_VAL_IF_FAIL(data != NULL, 0);

	data->func = func;
	data->user_data = user_data;

	id = g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, __idler_util_managed_idle_cb,
			     data, NULL);
	if (!id) {
		g_free(data);
		return id;
	}

	data->id = id;

	managed_idler_list = g_slist_append(managed_idler_list, data);

	return id;
}

void idler_util_managed_idle_cleanup(void)
{
	WIFI_RET_IF_FAIL(managed_idler_list != NULL);

	GSList *cur = managed_idler_list;
	struct managed_idle_data *data;

	while (cur) {
		GSList *next = cur->next;
		data = (struct managed_idle_data *)cur->data;
		if (data) {
			g_source_remove(data->id);
		}
		cur = next;
	}

	g_slist_free_full(managed_idler_list, g_free);
	managed_idler_list = NULL;
}
