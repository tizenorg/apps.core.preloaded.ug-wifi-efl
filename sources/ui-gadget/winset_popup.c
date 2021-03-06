/*
*  Wi-Fi UG
*
* Copyright 2012  Samsung Electronics Co., Ltd

* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at

* http://www.tizenopensource.org/license

* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/



#include "wifi.h"
#include "common.h"
#include "common_utils.h"
#include "winset_popup.h"
#include "wlan_manager.h"
#include "i18nmanager.h"
#include "viewer_manager.h"


struct popup_manager_object {
	/* General popup attributes */
	Evas_Object* win;
	Evas_Object* popup;
	Evas_Object *popup_user_prompt;
	char *str_pkg_name;
};

static void _mobilehotspot_disable_cb(DBusGProxy *proxy, DBusGProxyCall *call, gpointer user_data)
{
	__COMMON_FUNC_ENTER__;

	GError *err = NULL;
	guint type;
	guint result;
	DBusGConnection	*bus = user_data;

	dbus_g_proxy_end_call(proxy, call, &err, G_TYPE_UINT, &type,
						G_TYPE_UINT, &result, G_TYPE_INVALID);
	if (err != NULL) {
		INFO_LOG(COMMON_NAME_LIB, "Error occured [%s]\n", err->message);
		g_error_free(err);
		viewer_manager_header_mode_set(HEADER_MODE_OFF);
	} else {
		INFO_LOG(COMMON_NAME_LIB, "TYPE = %d,  Result = %d\n", type, result);
		if (3 == type && (0 == result || 5 == result)) {
			INFO_LOG(COMMON_NAME_LIB, "OK\n");
			/* Tithering is now disabled. All OK to switch on Wi-Fi */
			power_control();
		} else {
			viewer_manager_header_mode_set(HEADER_MODE_OFF);
		}
	}

	g_pending_call.is_handled = TRUE;

	g_object_unref(proxy);
	dbus_g_connection_unref(bus);

	__COMMON_FUNC_EXIT__;
}

static void _retry_clicked_cb(void* data, Evas_Object* obj, void* event_info)
{
	__COMMON_FUNC_ENTER__;
	popup_manager_object_t *manager_object = (popup_manager_object_t *)data;
	if (!manager_object) {
		return;
	}

	INFO_LOG(UG_NAME_NORMAL, "Response OK");

	switch (wlan_manager_start()) {
	case WLAN_MANAGER_ERR_NONE:
		break;
	case WLAN_MANAGER_ERR_ALREADY_REGISTERED:
		winset_popup_mode_set(manager_object, POPUP_OPTION_REGISTER_FAILED_UNKNOWN, NULL);
		__COMMON_FUNC_EXIT__;
		return;
	case WLAN_MANAGER_ERR_UNKNOWN:
		winset_popup_mode_set(manager_object, POPUP_OPTION_REGISTER_FAILED_COMMUNICATION_FAILED, NULL);
		__COMMON_FUNC_EXIT__;
		return;
	default:
		__COMMON_FUNC_EXIT__;
		return;
	}

	char profile_name[NET_PROFILE_NAME_LEN_MAX+1] = "";
	switch (wlan_manager_state_get(profile_name)) {
	case WLAN_MANAGER_OFF:
		ERROR_LOG(UG_NAME_NORMAL, "current state is wifi-off\n");
		viewer_manager_header_mode_set(HEADER_MODE_OFF);
		viewer_manager_hide(VIEWER_WINSET_SUB_CONTENTS);
		break;
	case WLAN_MANAGER_UNCONNECTED:
	case WLAN_MANAGER_CONNECTING:
	case WLAN_MANAGER_CONNECTED:
		connman_request_scan_mode_set(WIFI_BGSCAN_MODE_PERIODIC);
		viewer_manager_header_mode_set(HEADER_MODE_SEARCHING);
		viewer_manager_hide(VIEWER_WINSET_SEARCHING);
		viewer_manager_show(VIEWER_WINSET_SUB_CONTENTS);
		break;
	case WLAN_MANAGER_ERROR:
	default:
		winset_popup_mode_set(manager_object, POPUP_OPTION_ETC_WLAN_STATE_GET_ERROR, NULL);
		break;
	}

	__COMMON_FUNC_EXIT__;
}

static void _back_clicked_cb(void* data, Evas_Object* obj, void* event_info)
{
	INFO_LOG(UG_NAME_NORMAL, "Response CANCEL");
	wifi_exit();
}

static int mobilehotspot_deactivate()
{
	__COMMON_FUNC_ENTER__;

	DBusGConnection *bus;
	DBusGProxy *proxy;
	GError *error= NULL;

	bus = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);
	if (error != NULL) {
		INFO_LOG(COMMON_NAME_LIB, "Couldn't connect to the system bus");
		g_error_free(error);
		return FALSE;
	}

	proxy =	dbus_g_proxy_new_for_name(bus,
					"com.samsung.mobileap",	/* name */
					"/MobileAP",			/* obj path */
					"com.samsung.mobileap");/* interface */
	if (proxy == NULL) {
		INFO_LOG(COMMON_NAME_LIB, "Couldn't create the proxy object");
		dbus_g_connection_unref(bus);
		return FALSE;
	}

	g_pending_call.pending_call = dbus_g_proxy_begin_call(proxy, "disable_wifi_tethering",
			_mobilehotspot_disable_cb, bus, NULL, G_TYPE_INVALID);

	g_pending_call.proxy = proxy;
	g_pending_call.is_handled = FALSE;

	__COMMON_FUNC_EXIT__;
	return TRUE;
}

static void _turn_off_mobileap_yes_clicked_cb(void* data, Evas_Object* obj, void* event_info)
{
	__COMMON_FUNC_ENTER__;
	popup_manager_object_t *manager_object = (popup_manager_object_t *)data;
	INFO_LOG(UG_NAME_NORMAL, "Response OK");
	if(manager_object && NULL != manager_object->popup_user_prompt) {
		evas_object_hide(manager_object->popup_user_prompt);
		evas_object_del(manager_object->popup_user_prompt);
		manager_object->popup_user_prompt = NULL;
	}

	if (FALSE != mobilehotspot_deactivate()) {
		INFO_LOG(UG_NAME_NORMAL, "Mobile AP return value TRUE");
	} else {
		INFO_LOG(UG_NAME_NORMAL, "Mobile AP return value FALSE");
	}

	__COMMON_FUNC_EXIT__;
}

static void _turn_off_mobileap_no_clicked_cb(void* data, Evas_Object* obj, void* event_info)
{
	__COMMON_FUNC_ENTER__;
	popup_manager_object_t *manager_object = (popup_manager_object_t *)data;

	INFO_LOG(UG_NAME_NORMAL, "Response CANCEL");

	if(manager_object && NULL != manager_object->popup_user_prompt) {
		evas_object_hide(manager_object->popup_user_prompt);
		evas_object_del(manager_object->popup_user_prompt);
		manager_object->popup_user_prompt = NULL;
	}
	__COMMON_FUNC_EXIT__;
}

#if 0
static void _winset_popup_close_cb(void *data, Evas_Object *obj, void *event_info)
{
	popup_manager_object_t *manager_object = (popup_manager_object_t *)data;
	if (manager_object && manager_object->popup) {
		evas_object_del(manager_object->popup);
		manager_object->popup = NULL;
	}
	return;
}
#endif

popup_manager_object_t *winset_popup_manager_create(Evas_Object* win, const char *str_pkg_name)
{
	popup_manager_object_t *manager_object;
	manager_object = (popup_manager_object_t *)g_malloc0(sizeof(popup_manager_object_t));
	manager_object->win = win;
	manager_object->str_pkg_name = (char *)str_pkg_name;

	return manager_object;
}

/**
 * FIX ME LATER
 *
 * This function had re-factored as elm_popup's bug
 *
 */
void winset_popup_mode_set(popup_manager_object_t *manager_object, POPUP_MODE_OPTIONS option, void *input_data)
{
	char *info_txt;
	popup_btn_info_t popup_btn_data;
	if (!manager_object)
		return;

	__COMMON_FUNC_ENTER__;
	if (!manager_object)
		return;

	if(NULL != manager_object->popup) {
		evas_object_hide(manager_object->popup);
		evas_object_del(manager_object->popup);
		manager_object->popup = NULL;
	}

	memset(&popup_btn_data, 0, sizeof(popup_btn_data));
	switch (option) {
	case POPUP_OPTION_REGISTER_FAILED_COMMUNICATION_FAILED:
		popup_btn_data.info_txt = "connman is not working now";
		popup_btn_data.btn1_cb = _retry_clicked_cb;
		popup_btn_data.btn2_cb = _back_clicked_cb;
		popup_btn_data.btn1_data = manager_object;
		popup_btn_data.btn1_txt = "Retry";
		popup_btn_data.btn2_txt = "Back";
		manager_object->popup = common_utils_show_info_popup(manager_object->win, &popup_btn_data);
		break;

	case POPUP_OPTION_REGISTER_FAILED_UNKNOWN:
		info_txt = _("[ERROR] wlan_client func<br>wlan_client_register");
		manager_object->popup = common_utils_show_info_ok_popup(manager_object->win, manager_object->str_pkg_name, info_txt);
		break;

	case POPUP_OPTION_POWER_ON_FAILED_MOBILE_HOTSPOT:
		if(NULL == manager_object->popup_user_prompt) {
			popup_btn_data.info_txt = "Connecting Wi-Fi will turn off Mobile hotspot. Continue?";
			popup_btn_data.btn1_cb = _turn_off_mobileap_yes_clicked_cb;
			popup_btn_data.btn2_cb = _turn_off_mobileap_no_clicked_cb;
			popup_btn_data.btn1_data = popup_btn_data.btn2_data = manager_object;
			popup_btn_data.btn1_txt = sc(manager_object->str_pkg_name, I18N_TYPE_Yes);
			popup_btn_data.btn2_txt = sc(manager_object->str_pkg_name, I18N_TYPE_No);
			manager_object->popup_user_prompt = common_utils_show_info_popup(manager_object->win, &popup_btn_data);
		}
		break;

	case POPUP_OPTION_CONNECTING_FAILED:
		if (input_data) {
			info_txt = g_strdup_printf("Unable to connect %s", (char *)input_data);
		} else {
			info_txt = g_strdup("Unable to connect");
		}
		manager_object->popup = common_utils_show_info_ok_popup(manager_object->win, manager_object->str_pkg_name, info_txt);
		g_free(info_txt);
		break;

	case POPUP_OPTION_ETC_WLAN_STATE_GET_ERROR:
		info_txt = _("[ERROR] wlan_client func<br>wlan_client_get_state");
		manager_object->popup = common_utils_show_info_ok_popup(manager_object->win, manager_object->str_pkg_name, info_txt);
		break;

	case POPUP_OPTION_HIDDEN_AP_SSID_LEN_ERROR:
		info_txt = _("SSID can be up to 32 letters.<br>Check your input.");
		manager_object->popup = common_utils_show_info_ok_popup(manager_object->win, manager_object->str_pkg_name, info_txt);
		break;

	case POPUP_OPTION_UNREG_WLAN_EVENT_ERROR:
		info_txt = _("[ERROR] Unregistered WLAN_EVENT number");
		manager_object->popup = common_utils_show_info_ok_popup(manager_object->win, manager_object->str_pkg_name, info_txt);
		break;

	case POPUP_OPTION_WEP_PSWD_LEN_ERROR:
		info_txt = WEP_WRONG_PASSWORD_LEN_ERR_MSG_STR;
		manager_object->popup = common_utils_show_info_ok_popup(manager_object->win, manager_object->str_pkg_name, info_txt);
		break;

	case POPUP_OPTION_WPA_PSWD_LEN_ERROR:
		info_txt = WPA_WRONG_PASSWORD_LEN_ERR_MSG_STR;
		manager_object->popup = common_utils_show_info_ok_popup(manager_object->win, manager_object->str_pkg_name, info_txt);
		break;

	default:
		break;
	}
	__COMMON_FUNC_EXIT__;
	return;
}

boolean winset_popup_manager_destroy(popup_manager_object_t *manager_object)
{
	if (!manager_object)
		return FALSE;

	if (NULL != manager_object->popup) {
		evas_object_del(manager_object->popup);
		manager_object->popup = NULL;
	}

	g_free(manager_object);
	return TRUE;
}

boolean winset_popup_hide_popup(popup_manager_object_t *manager_object)
{
	if (!manager_object)
		return FALSE;
	evas_object_hide(manager_object->popup);
	evas_object_del(manager_object->popup);
	manager_object->popup = NULL;

	if(manager_object && NULL != manager_object->popup_user_prompt) {
		evas_object_hide(manager_object->popup_user_prompt);
		evas_object_del(manager_object->popup_user_prompt);
		manager_object->popup_user_prompt = NULL;
	}
	return TRUE;
}
