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


#include <app.h>
#include <dlog.h>
#include <Elementary.h>
#include <Evas.h>
#include <efl_assist.h>
#include <efl_extension.h>
#include <glib.h>

#include "util/log.h"
#include "view/util/efl_helper.h"

static gboolean _make_path_append_to_app_resource_path(gchar *path, gsize path_length, const gchar *subdir, const gchar *file)
{
	gchar *res_path = app_get_resource_path();
	if (res_path) {
		if (subdir)
			g_snprintf(path, path_length, "%s%s/%s", res_path, subdir, file);
		else
			g_snprintf(path, path_length, "%s/%s", res_path, file);
		g_free(res_path);
		return TRUE;
	}
	return FALSE;
}

static gboolean _make_path_append_to_app_shared_resource_path(gchar *path, gsize path_length, const gchar *subdir, const gchar *file)
{
	gchar *res_path = app_get_shared_resource_path();
	if (res_path) {
		if (subdir)
			g_snprintf(path, path_length, "%s%s/%s", res_path, subdir, file);
		else
			g_snprintf(path, path_length, "%s/%s", res_path, file);
		g_free(res_path);
		return TRUE;
	}
	return FALSE;
}

gboolean get_color_table_path(gchar *table_path, gsize table_path_length)
{
	return _make_path_append_to_app_shared_resource_path(
		       table_path, table_path_length, TABLE_PATH, TABLE_COLOR_FILE);
}

gboolean get_font_table_path(gchar *table_path, gsize table_path_length)
{
	return _make_path_append_to_app_shared_resource_path(
		       table_path, table_path_length, TABLE_PATH, TABLE_FONT_FILE);
}

gboolean get_edj_path(gchar *edj_path, gsize edj_path_length)
{
	return _make_path_append_to_app_resource_path(
		       edj_path, edj_path_length, EDJ_PATH, EDJ_FILE);
}

gboolean get_image_path(gchar *image_path, gsize image_path_length,
			const gchar *image_file)
{
	return _make_path_append_to_app_resource_path(
		       image_path, image_path_length, IMAGE_PATH, image_file);
}

Elm_Genlist_Item_Class *create_genlist_itc(const char *style,
					   Elm_Gen_Item_Text_Get_Cb text_get,
					   Elm_Gen_Item_Content_Get_Cb content_get,
					   Elm_Gen_Item_State_Get_Cb state_get,
					   Elm_Gen_Item_Del_Cb del)
{
	Elm_Genlist_Item_Class *genlist_itc = elm_genlist_item_class_new();
	if (genlist_itc) {
		genlist_itc->item_style = style;
		genlist_itc->func.text_get = text_get;
		genlist_itc->func.content_get = content_get;
		genlist_itc->func.state_get = state_get;
		genlist_itc->func.del = del;
	}
	return genlist_itc;
}

Evas_Object *create_layout_use_edj_file(Evas_Object *parent, const gchar *group)
{
	Evas_Object *layout = NULL;
	gchar edj_path[PATH_MAX] = { 0, };
	WIFI_RET_VAL_IF_FAIL(parent, NULL);
	WIFI_RET_VAL_IF_FAIL(get_edj_path(edj_path, sizeof(edj_path)), NULL);

	layout = elm_layout_add(parent);
	WIFI_RET_VAL_IF_FAIL(layout, NULL);

	if (!elm_layout_file_set(layout, edj_path, group)) {
		WIFI_LOG_ERR("elm_layout_file_set() is failed.");
		evas_object_del(layout);
		return NULL;
	}
	return layout;
}

Evas_Object *create_icon_use_image_file(Evas_Object *parent, const gchar *image,
					const gchar *code)
{
	Evas_Object *icon = NULL;
	gchar image_path[PATH_MAX] = { 0, };

	WIFI_RET_VAL_IF_FAIL(image, NULL);
	WIFI_RET_VAL_IF_FAIL(get_image_path(image_path, sizeof(image_path), image), NULL);

	icon = elm_image_add(parent);
	WIFI_RET_VAL_IF_FAIL(icon, NULL);
	elm_image_file_set(icon, image_path, NULL);
	return icon;
}
