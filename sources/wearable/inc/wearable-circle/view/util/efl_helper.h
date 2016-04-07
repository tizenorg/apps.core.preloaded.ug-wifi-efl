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


#ifndef __UTIL_EFL_HELPER_H__
#define __UTIL_EFL_HELPER_H__

#include <Elementary.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PACKAGE
#define PACKAGE "org.tizen.w-wifi"
#endif

#define TABLE_PATH "/tables"
#define TABLE_COLOR_FILE "color_table.xml"
#define TABLE_FONT_FILE "font_table.xml"

#define EDJ_PATH "/edje"
#define EDJ_FILE "wifi.edj"

#define IMAGE_PATH "/images"

gboolean get_color_table_path(gchar *table_path, gsize table_path_length);
gboolean get_font_table_path(gchar *table_path, gsize table_path_length);
gboolean get_edj_path(gchar *edj_path, gsize edj_path_length);
gboolean get_image_path(gchar *image_path, gsize image_path_length,
			const gchar *image_file);

Elm_Genlist_Item_Class *create_genlist_itc(const char *style,
					   Elm_Gen_Item_Text_Get_Cb text_get,
					   Elm_Gen_Item_Content_Get_Cb content_get,
					   Elm_Gen_Item_State_Get_Cb state_get,
					   Elm_Gen_Item_Del_Cb del);

Evas_Object *create_layout_use_edj_file(Evas_Object *parent, const gchar *group);

Evas_Object *create_icon_use_image_file(Evas_Object *parent, const gchar *image,
					const gchar *code);

#ifdef __cplusplus
}
#endif

#endif /* __UTIL_EFL_HELPER_H__ */
