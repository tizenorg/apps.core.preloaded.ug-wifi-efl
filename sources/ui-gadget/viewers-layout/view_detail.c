/*
 * Wi-Fi
 *
 * Copyright 2012 Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.tizenopensource.org/license
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "ug_wifi.h"
#include "view_detail.h"
#include "i18nmanager.h"
#include "viewer_manager.h"
#include "winset_popup.h"
#include "common_utils.h"
#include "common_ip_info.h"
#include "common_eap_connect.h"

typedef struct _view_detail_data {
	Evas_Object *win;
	char *ap_image_path;
	wifi_ap_h ap;
	eap_info_list_t *eap_info_list;
	ip_info_list_t *ip_info_list;
	Evas_Object *forget_confirm_popup;
	Evas_Object *view_detail_list;
} view_detail_data;

static int view_detail_end = TRUE;

/* function declaration */
static void detailview_sk_cb(void *data, Evas_Object *obj, void *event_info);
static void forget_sk_cb(void *data, Evas_Object *obj, void *event_info);

///////////////////////////////////////////////////////////////
// implementation
///////////////////////////////////////////////////////////////
static char *_view_detail_grouptitle_text_get(void *data,
		Evas_Object *obj, const char *part)
{
	__COMMON_FUNC_ENTER__;

	retvm_if(NULL == part, NULL);

	char *ret = NULL;

	if (!strncmp(part, "elm.text.2", strlen(part))) {
		ret = (char*) g_strdup(sc(PACKAGE, I18N_TYPE_Name));
	} else if (!strncmp(part, "elm.text.1", strlen(part))) {
		view_detail_data *detail_data = (view_detail_data *)data;
		retvm_if(NULL == detail_data, NULL);

		if (wifi_ap_get_essid(detail_data->ap, &ret) != WIFI_ERROR_NONE)
			ret = NULL;
	}

	__COMMON_FUNC_EXIT__;
	return ret;
}

static Evas_Object *_view_detail_grouptitle_content_get(void *data, Evas_Object *obj, const char *part)
{
	retvm_if(NULL == data || NULL == part, NULL);

	view_detail_data *detail_data = (view_detail_data *)data;
	Evas_Object* icon = NULL;

	if (detail_data->ap_image_path == NULL) {
		/* if there is no ap_image_path (NO AP Found situation) */
		DEBUG_LOG(UG_NAME_ERR, "Fatal: Image path is NULL");
	} else if (!strncmp(part, "elm.icon", strlen(part))) {
		/* for strength */
		icon = elm_image_add(obj);
		retvm_if(NULL == icon, NULL);

		elm_image_file_set(icon, detail_data->ap_image_path, NULL);
	}

	return icon;
}

static void _remove_all(view_detail_data *_detail_data)
{
	__COMMON_FUNC_ENTER__;

	if(_detail_data) {
		if (_detail_data->eap_info_list) {
			eap_info_remove(_detail_data->eap_info_list);
			_detail_data->eap_info_list = NULL;
		}

		ip_info_remove(_detail_data->ip_info_list);
		_detail_data->ip_info_list = NULL;

		evas_object_del(_detail_data->view_detail_list);
		_detail_data->view_detail_list = NULL;

		g_free(_detail_data->ap_image_path);
		g_free(_detail_data);

		_detail_data = NULL;
	}

	__COMMON_FUNC_EXIT__;
}

static gboolean __forget_wifi_ap(gpointer data)
{
	__COMMON_FUNC_ENTER__;

	wifi_ap_h ap = (wifi_ap_h)data;

	wlan_manager_forget(ap);

	wifi_ap_destroy(ap);

	__COMMON_FUNC_EXIT__;
	return FALSE;
}

static void ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	__COMMON_FUNC_ENTER__;

	wifi_ap_h ap = NULL;
	view_detail_data *_detail_data;

	if (view_detail_end == TRUE)
		return;

	view_detail_end = TRUE;
	_detail_data = (view_detail_data *)data;
	retm_if(NULL == _detail_data);

	wifi_ap_clone(&ap, _detail_data->ap);

	evas_object_del(_detail_data->forget_confirm_popup);
	_detail_data->forget_confirm_popup = NULL;

	_remove_all(_detail_data);

	elm_naviframe_item_pop(viewer_manager_get_naviframe());

	g_idle_add(__forget_wifi_ap, (gpointer)ap);

	__COMMON_FUNC_EXIT__;
}

static void cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	__COMMON_FUNC_ENTER__;

	view_detail_data *_detail_data = (view_detail_data *)data;
	retm_if(NULL == _detail_data);

	evas_object_del(_detail_data->forget_confirm_popup);
	_detail_data->forget_confirm_popup = NULL;

	__COMMON_FUNC_EXIT__;
}

static void forget_sk_cb(void *data, Evas_Object *obj, void *event_info)
{
	__COMMON_FUNC_ENTER__;

	view_detail_data *_detail_data = (view_detail_data *)data;
	retm_if(NULL == _detail_data);

	if (!_detail_data->forget_confirm_popup) {
		popup_btn_info_t popup_data;
		memset(&popup_data, 0, sizeof(popup_data));

		popup_data.info_txt = g_strdup(sc(PACKAGE, I18N_TYPE_Autonomous_connection_to_s_will_be_turned_off_Continue));
		popup_data.btn1_cb = ok_cb;
		popup_data.btn1_txt = sc(PACKAGE, I18N_TYPE_Ok);
		popup_data.btn1_data = _detail_data;
		popup_data.btn2_cb = cancel_cb;
		popup_data.btn2_txt = sc(PACKAGE, I18N_TYPE_Cancel);
		popup_data.btn2_data = _detail_data;
		_detail_data->forget_confirm_popup = common_utils_show_info_popup(_detail_data->win, &popup_data);

		evas_object_show(_detail_data->forget_confirm_popup);
	}

	__COMMON_FUNC_EXIT__;
}

static void title_back_btn_sk_cb(void *data, Evas_Object *obj, void *event_info)
{
	__COMMON_FUNC_ENTER__;

	if(view_detail_end == TRUE) {
		return;
	}
	view_detail_end = TRUE;
	view_detail_data *_detail_data = (view_detail_data *)data;
	retm_if(NULL == _detail_data);

	if (_detail_data->eap_info_list)
		eap_info_save_data(_detail_data->eap_info_list);

	ip_info_save_data(_detail_data->ip_info_list);
	_remove_all(_detail_data);
	elm_naviframe_item_pop(viewer_manager_get_naviframe());

	__COMMON_FUNC_EXIT__;
}

static void detailview_sk_cb(void *data, Evas_Object *obj, void *event_info)
{
	__COMMON_FUNC_ENTER__;
	if (view_detail_end == TRUE)
		return;
	view_detail_end = TRUE;

	view_detail_data *_detail_data = (view_detail_data *)data;
	retm_if(NULL == _detail_data);

	if (_detail_data->eap_info_list)
		eap_info_save_data(_detail_data->eap_info_list);

	ip_info_save_data(_detail_data->ip_info_list);
	_remove_all(_detail_data);

	__COMMON_FUNC_EXIT__;
}

static void __view_detail_imf_ctxt_evnt_cb(void *data, Ecore_IMF_Context *ctx, int value)
{
	if (!data)
		return;
	if (value == ECORE_IMF_INPUT_PANEL_STATE_SHOW) {
		DEBUG_LOG(UG_NAME_NORMAL, "Key pad is now open");
		elm_object_item_signal_emit(data, "elm,state,sip,shown", "");
	} else if (value == ECORE_IMF_INPUT_PANEL_STATE_HIDE) {
		DEBUG_LOG(UG_NAME_NORMAL, "Key pad is now closed");
		elm_object_item_signal_emit(data, "elm,state,sip,hidden", "");
	}
	return;
}

static Eina_Bool __view_detail_load_ip_info_list_cb(void *data)
{
	Elm_Object_Item *navi_it = NULL;
	Evas_Object *list = NULL;
	Evas_Object *layout;
	view_detail_data *_detail_data = (view_detail_data *)data;

	if (!_detail_data)
		return ECORE_CALLBACK_CANCEL;

	navi_it = elm_naviframe_top_item_get(viewer_manager_get_naviframe());
	layout = elm_object_item_part_content_get(navi_it, "elm.swallow.content");

	/* Create an EAP connect view list */
	list = elm_object_part_content_get(layout, "elm.swallow.content");

	/* Append ip info list */
	_detail_data->ip_info_list = ip_info_append_items(_detail_data->ap, PACKAGE, list, __view_detail_imf_ctxt_evnt_cb, navi_it);

	common_utils_add_dialogue_separator(list, "dialogue/separator");

	return ECORE_CALLBACK_CANCEL;
}

void view_detail(wifi_device_info_t *device_info, Evas_Object *win_main)
{
	__COMMON_FUNC_ENTER__;

	bool favorite = 0;
	wifi_ap_h ap;
	static Elm_Genlist_Item_Class grouptitle_itc;

	if (device_info == NULL) {
		ERROR_LOG(UG_NAME_NORMAL, "Failed: device_info is NULL");
		return;
	}
	Evas_Object *layout = NULL;
	Evas_Object* navi_frame = viewer_manager_get_naviframe();
	if (navi_frame == NULL) {
		ERROR_LOG(UG_NAME_NORMAL, "Failed to get naviframe");
		return;
	}

	view_detail_end = FALSE;

	view_detail_data *_detail_data = g_new0(view_detail_data, 1);
	retm_if(NULL == _detail_data);

	_detail_data->ap = ap = device_info->ap;
	wifi_ap_is_favorite(ap, &favorite);
	_detail_data->ap_image_path = g_strdup(device_info->ap_image_path);
	layout = common_utils_create_layout(navi_frame);
	evas_object_show(layout);

	Evas_Object *detailview_list = elm_genlist_add(layout);
	retm_if(NULL == detailview_list);

	elm_object_style_set(detailview_list, "dialogue");
	_detail_data->view_detail_list = detailview_list;

	grouptitle_itc.item_style = "dialogue/2text.1icon.5";
	grouptitle_itc.func.text_get = _view_detail_grouptitle_text_get;
	grouptitle_itc.func.content_get = _view_detail_grouptitle_content_get;
	grouptitle_itc.func.state_get = NULL;
	grouptitle_itc.func.del = NULL;

	common_utils_add_dialogue_separator(detailview_list, "dialogue/separator");

	/* AP name and signal strength icon */
	Elm_Object_Item* title = elm_genlist_item_append(detailview_list, &grouptitle_itc, _detail_data, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
	elm_object_item_disabled_set(title, TRUE);

	elm_object_part_content_set(layout, "elm.swallow.content", detailview_list);

	Elm_Object_Item* navi_it = elm_naviframe_item_push(navi_frame, sc(PACKAGE, I18N_TYPE_Details), NULL, NULL, layout, NULL);
	evas_object_data_set(navi_frame, SCREEN_TYPE_ID_KEY, (void *)VIEW_MANAGER_VIEW_TYPE_DETAIL);

	/* Toolbar Back button */
	Evas_Object* button_back = elm_object_item_part_content_get(navi_it, "prev_btn");
	evas_object_smart_callback_add(button_back, "clicked", detailview_sk_cb, _detail_data);

	/* Title Back button */
	button_back = elm_button_add(navi_frame);
	elm_object_style_set(button_back, "naviframe/back_btn/default");
	evas_object_smart_callback_add(button_back, "clicked", title_back_btn_sk_cb, _detail_data);
	elm_object_item_part_content_set(navi_it, "title_prev_btn", button_back);

	_detail_data->win = win_main;

	if (favorite) {
		/* Toolbar Forget button */
		Evas_Object* forget_button = elm_button_add(navi_frame);
		elm_object_style_set(forget_button, "naviframe/toolbar/default");
		elm_object_text_set(forget_button, sc(PACKAGE, I18N_TYPE_Forget));
		evas_object_smart_callback_add(forget_button, "clicked", forget_sk_cb, _detail_data);
		elm_object_item_part_content_set(navi_it, "toolbar_button1", forget_button);

		/* Title Forget button */
		forget_button = elm_button_add(navi_frame);
		elm_object_style_set(forget_button, "naviframe/toolbar/default");
		elm_object_text_set(forget_button, sc(PACKAGE, I18N_TYPE_Forget));
		evas_object_smart_callback_add(forget_button, "clicked", forget_sk_cb, _detail_data);
		elm_object_item_part_content_set(navi_it, "title_toolbar_button1", forget_button);
	}

	wifi_security_type_e type = WIFI_SECURITY_TYPE_NONE;
	wifi_ap_get_security_type(ap, &type);
	if (WIFI_SECURITY_TYPE_EAP == type) {
		wifi_connection_state_e connection_state;
		wifi_ap_get_connection_state(ap, &connection_state);
		if (favorite || WIFI_CONNECTION_STATE_CONNECTED == connection_state) {
			_detail_data->eap_info_list = eap_info_append_items(ap, detailview_list, PACKAGE, __view_detail_imf_ctxt_evnt_cb, navi_it);
		}
	}

	/* Append the ip info details */
	ecore_idler_add(__view_detail_load_ip_info_list_cb, _detail_data);

	__COMMON_FUNC_EXIT__;
}