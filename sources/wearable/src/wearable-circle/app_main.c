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
#include <vconf.h>
#include <wifi.h>
#include <dlog.h>
#include <Elementary.h>
#include <Evas.h>
#include <efl_assist.h>
#include <glib.h>
#include <gio/gio.h>

#include "util.h"
#include "view.h"
#include "view/util/efl_helper.h"
#include "net/wifi_manager.h"
#include "net/util/wifi_address.h"
#include "net/util/vconf_helper.h"

#define CUSTOM_GROUP_AP_SIGNAL_IMAGE_LAYOUT "ap_signal_image_layout"

#define KEYPAD_PREDICTION_ON_HEIGHT             290
#define KEYPAD_PREDICTION_OFF_HEIGHT    240
#define KEYPAD_PREDICTION_ON_HEIGHT_MIN ((KEYPAD_PREDICTION_OFF_HEIGHT) + (((KEYPAD_PREDICTION_ON_HEIGHT)-(KEYPAD_PREDICTION_OFF_HEIGHT)) / 2))
#define KEYPAD_PREDICTION_OFF_HEIGHT_MIN ((KEYPAD_PREDICTION_OFF_HEIGHT)-(((KEYPAD_PREDICTION_ON_HEIGHT)-(KEYPAD_PREDICTION_OFF_HEIGHT)) / 2))

// 1 second is more natural to other timeout.
#define TIMEOUT_FOR_CHECK_WIFI_POWER_CHANGED 1000

#define MIN_PASSWORD_LENGTH                             8
#define MAX_PASSWORD_LENGTH                             64
#define MAX_PASSWORD_LENGTH_FOR_UNICHAR_BYTES                                    MAX_PASSWORD_LENGTH * G_UNICHAR_MAX_DECOMPOSITION_LENGTH + 1
#define MAX_WEP_PASSWORD_LENGTH                 13
#define MAX_WEP_DIGIT_PASSWORD_LENGTH   26

typedef struct {
	// network module
	wifi_manager_object *wifi_manager;
	wifi_ap_object *selected_wifi_ap;
	wifi_ap_object *connecting_wifi_ap;
	wifi_ap_object *connect_reserve_wifi_ap;
	wifi_ap_object *disconnecting_wifi_ap;

	// network utility data for connect & view control
	wifi_address_object *address_for_connect;
	wifi_address_object *address_for_edit;
	gchar *wps_pin_string;
	gboolean is_first_scan;
	gboolean is_scan_finished;
	gboolean is_scanning_for_wifi_activate;

	// view
	view_base_object *base;
	layout_main_object *main;
	layout_scan_object *scan;
	layout_ap_info_object *ap_info;
	layout_password_entry_object *password_entry;
	layout_static_ip_object *static_ip;
	layout_proxy_setting_object *proxy_setting;
	layout_wearable_input_object *wearable_input;
	layout_eap_method_object *eap_method;
	layout_wps_method_object *wps_method;
	layout_wps_progress_object *wps_progress;
	layout_detail_object *detail;
	popup_change_ap_object *popup_change_ap;
	popup_scanning_object *popup_scanning;
	popup_unable_scan_object *popup_unable_scan;

	// view data
	gboolean is_main_power_check_clicked;

	// view data for tts
	Evas_Object *checkbox_power;
	Evas_Object *checkbox_proxy;
	Evas_Object *checkbox_static_ip;

	// app control
	app_control_h app_control;
} app_object;

static gboolean _app_network_init(app_object *app_obj);

static gboolean _app_network_callbacks_init_for_view(app_object *app_obj);

static gboolean _app_view_base_init(app_object *app_obj);

static gboolean __wifi_manager_scan_start(app_object *app_obj, gboolean is_first_scan);

static view_base_object *_view_base_create();

static popup_unable_scan_object *_popup_unable_scan_create(app_object *app_obj);

static layout_detail_object *_detail_create(app_object *app_obj, wifi_ap_object *ap);

static layout_ap_info_object *_ap_info_create(app_object *app_obj, wifi_ap_object *ap);
static void                   _ap_info_show(app_object *app_obj, wifi_ap_object *ap);

static void                _scan_callbacks_init(layout_scan_object *scan_obj, app_object *app_obj);
static layout_scan_object *_scan_create(view_base_object *base_obj, app_object *app_obj);

static void _popup_scanning_show(app_object *app_obj, gboolean is_scanning_for_wifi_activate);
static void __popup_scanning_destroy_cb(void *data, Evas_Object *obj, void *event_info);

static void                _main_callbacks_init(layout_main_object *main_obj, app_object *app_obj);
static layout_main_object *_main_create(view_base_object *base_obj, app_object *app_obj);

static void _app_release(app_object *app_obj);

static inline const gchar *_app_error_to_string(app_error_e err)
{
	switch (err) {
	case APP_ERROR_NONE:
		return "APP_ERROR_NONE";

	case APP_ERROR_INVALID_PARAMETER:
		return "APP_ERROR_INVALID_PARAMETER";

	case APP_ERROR_OUT_OF_MEMORY:
		return "APP_ERROR_OUT_OF_MEMORY";

	case APP_ERROR_INVALID_CONTEXT:
		return "APP_ERROR_INVALID_CONTEXT";

	case APP_ERROR_NO_SUCH_FILE:
		return "APP_ERROR_NO_SUCH_FILE";

	case APP_ERROR_ALREADY_RUNNING:
		return "APP_ERROR_ALREADY_RUNNING";

	default:
		return "UNKNOWN";
	}
}

static inline gboolean _wifi_manager_scan_start(app_object *app_obj)
{
	WIFI_LOG_INFO("_wifi_manager_scan_start");
	return __wifi_manager_scan_start(app_obj, FALSE);
}

static inline gboolean _wifi_manager_scan_start_for_wifi_activated(app_object *app_obj)
{
	WIFI_LOG_INFO("_wifi_manager_scan_start_for_wifi_activated");
	return __wifi_manager_scan_start(app_obj, TRUE);
}

static inline gboolean _is_default_proxy_address(const gchar *proxy_address)
{
	return !g_strcmp0(proxy_address, DEFAULT_GUIDE_PROXY_ADDRESS);
}

static inline gboolean _is_default_proxy_port(const gchar *proxy_port)
{
	return !g_strcmp0(proxy_port, DEFAULT_GUIDE_PROXY_PORT);
}

static inline gboolean _app_is_view_initialized(app_object *app_obj)
{
	WIFI_RET_VAL_IF_FAIL(app_obj, FALSE);
	return app_obj->main || app_obj->scan;
}

static gboolean _mdm_is_password_hidden(void)
{
	gboolean hidden_status = FALSE;
	return hidden_status;
}

static gboolean _app_network_init(app_object *app_obj)
{
	wifi_manager_object *wifi_manager = NULL;
	wifi_error_e err = WIFI_ERROR_NONE;

	__WIFI_FUNC_ENTER__;

	wifi_manager = wifi_manager_new_with_init(&err);
	if (!wifi_manager) {
		WIFI_LOG_ERR("wifi_manager_new_with_init() is failed. error = %s",
			     wifi_error_to_string(err));
		return FALSE;
	}

	app_obj->wifi_manager = wifi_manager;
	return TRUE;
}

static gint _compare_wifi_ap_object(gconstpointer a, gconstpointer b)
{
	return wifi_manager_ap_is_equals((wifi_ap_object *)a, (wifi_ap_object *)b)
	       ? 0 : -1;
}

typedef struct {
	gchar *ssid;
	wifi_security_type_e security_type;
} _wifi_ap_property;

static gint _compare_wifi_ap_object_properties(gconstpointer a, gconstpointer b)
{
	wifi_ap_object *ap = (wifi_ap_object *)a;
	_wifi_ap_property *prop = (_wifi_ap_property *)b;
	gchar *ssid;
	wifi_security_type_e security_type;
	gboolean is_equals;
	WIFI_RET_VAL_IF_FAIL(ap != NULL, -1);
	WIFI_RET_VAL_IF_FAIL(prop != NULL, -1);

	ssid = wifi_manager_ap_get_ssid(ap);
	security_type = wifi_manager_ap_get_security_type(ap);
	is_equals = (!g_strcmp0(ssid, prop->ssid)) && (security_type == prop->security_type);

	g_free(ssid);
	return is_equals ? 0 : -1;
}

static void _scan_ap_list_update_ap_states(app_object *app_obj, GList *ap_list)
{
	GList *l = ap_list;
	while (l) {
		wifi_ap_object *ap = l->data;
		Elm_Object_Item *found_ap_item = layout_scan_ap_list_find_item_by_data(
			app_obj->scan, ap, _compare_wifi_ap_object);
		if (found_ap_item) {
			wifi_ap_object *found_ap = elm_object_item_data_get(found_ap_item);
			wifi_manager_ap_set_last_connection_error(ap,
								  wifi_manager_ap_get_last_connection_error(app_obj->wifi_manager, found_ap));
			wifi_manager_ap_set_captiveportal(ap,
							  wifi_manager_ap_is_captiveportal(app_obj->wifi_manager, found_ap));
		}
		l = l->next;
	}
}

static void _scan_ap_list_update_and_show(app_object *app_obj)
{
	GList *ap_list = NULL;
	wifi_error_e err = wifi_manager_get_found_ap_list(app_obj->wifi_manager, &ap_list);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_manager_get_found_ap_list() is failed. error = %s",
			     wifi_error_to_string(err));
		return;
	}

	if (!app_obj->scan) {
		app_obj->scan = _scan_create(app_obj->base, app_obj);
	}
	if (g_list_length(ap_list) > 0) {
		_scan_ap_list_update_ap_states(app_obj, ap_list);
		layout_scan_ap_list_clear_data(app_obj->scan);
		layout_scan_ap_list_set_data_list(app_obj->scan, ap_list);
		layout_scan_ap_list_show(app_obj->scan);
		if (layout_scan_is_top(app_obj->scan)) {
			layout_scan_ap_list_activate_rotary_event(app_obj->scan);
		}
	} else {
		layout_scan_no_ap_show(app_obj->scan);
	}

	wifi_manager_update_wifi_config_list(app_obj->wifi_manager);
}

static inline gboolean _wifi_manager_scan_start_for_idle(gpointer user_data)
{
	app_object *app_obj = user_data;
	__WIFI_FUNC_ENTER__;
	if (!_wifi_manager_scan_start(app_obj)) {
		if (app_obj->popup_scanning) {
			popup_scanning_dismiss(app_obj->popup_scanning);
		}
	}
	return FALSE;
}

static void __wifi_manager_scan_finished_cb(wifi_manager_object *manager,
					    wifi_ap_object *ap, wifi_error_e error_code, gpointer user_data)
{
	app_object *app_obj = user_data;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_IF_FAIL(manager != NULL);
	WIFI_RET_IF_FAIL(app_obj != NULL);

	if (!app_obj->popup_scanning) {
		WIFI_LOG_INFO("Scanning progress is closed by user.");
		return;
	}

	if (app_obj->is_first_scan) {
		if (!app_obj->scan)
			app_obj->scan = _scan_create(app_obj->base, app_obj);

		if (idler_util_managed_idle_add(_wifi_manager_scan_start_for_idle,
					app_obj) > 0) {
			return;
		} else {
			WIFI_LOG_ERR("idler_util_managed_idle_add() is failed.");
		}
	}

	app_obj->is_scan_finished = TRUE;
	if (app_obj->popup_scanning) {
		popup_scanning_destroy(app_obj->popup_scanning);
		popup_scanning_free(app_obj->popup_scanning);
		app_obj->popup_scanning = NULL;
	}

	if (error_code != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("Wi-Fi Scan failed. error code = %s",
			     wifi_error_to_string(error_code));
		return;
	}

	_scan_ap_list_update_and_show(app_obj);
}

static gboolean __wifi_manager_scan_start(app_object *app_obj, gboolean is_first_scan)
{
	wifi_error_e err;

	app_obj->is_first_scan = is_first_scan;
	err = wifi_manager_scan(app_obj->wifi_manager,
				__wifi_manager_scan_finished_cb, app_obj);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_manager_scan() is failed. error = %s",
			     wifi_error_to_string(err));
		return FALSE;
	}

	app_obj->is_scan_finished = FALSE;
	return TRUE;
}

static gboolean _wifi_manager_scan_start_by_scan_button(app_object *app_obj)
{
	wifi_error_e err;

	app_obj->is_first_scan = FALSE;
	err = wifi_manager_scan(app_obj->wifi_manager,
				__wifi_manager_scan_finished_cb, app_obj);
	if (err == WIFI_ERROR_NOW_IN_PROGRESS) {
		WIFI_LOG_INFO("wifi_manager_scan() now in progress. scan request to connman.");
	} else if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_manager_scan() is failed. error = %s",
			     wifi_error_to_string(err));
		return FALSE;
	}

	app_obj->is_scan_finished = FALSE;
	return TRUE;
}

static void _release_popups(app_object *app_obj)
{
	WIFI_RET_IF_FAIL(app_obj != NULL);

	if (app_obj->popup_change_ap) {
		popup_change_ap_dismiss(app_obj->popup_change_ap, CHANGE_AP_DISMISS_CANCEL);
	}
	if (app_obj->popup_scanning) {
		popup_scanning_dismiss(app_obj->popup_scanning);
	}
}

static void __wifi_manager_device_state_changed_cb(wifi_manager_object *manager,
						   wifi_device_state_e device_state,
						   wifi_connection_state_e connection_state, wifi_ap_object *ap,
						   wifi_rssi_level_e rssi_level, gpointer user_data)
{
	app_object *app_obj = user_data;
	gboolean is_wifi_device_activated =
		(device_state == WIFI_DEVICE_STATE_ACTIVATED);

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	if (is_wifi_device_activated) {
		if (app_obj->is_scanning_for_wifi_activate && !app_obj->popup_scanning) {
			WIFI_LOG_ERR("Scanning popup(for Wi-Fi activate) clolsed by user.");
			wifi_error_e err = wifi_manager_scan(app_obj->wifi_manager, NULL, NULL);
			if (err != WIFI_ERROR_NONE) {
				WIFI_LOG_ERR("Wi-Fi Scan failed. error code = %s",
					     wifi_error_to_string(err));
			}
			return;
		}
		if (_wifi_manager_scan_start_for_wifi_activated(app_obj)) {
			_popup_scanning_show(app_obj, FALSE);
		}
	} else {
		if (app_obj->main) {
			_release_popups(app_obj);
			layout_main_pop_to(app_obj->main);
		}
	}

	layout_main_menu_update(app_obj->main, MAIN_MENU_POWER);
	layout_main_menu_update(app_obj->main, MAIN_MENU_SCAN);
	layout_main_menu_update(app_obj->main, MAIN_MENU_EMPTY);

	layout_main_menu_set_enable(app_obj->main, MAIN_MENU_POWER, EINA_TRUE);
	layout_main_menu_set_enable(app_obj->main,
				    MAIN_MENU_SCAN, is_wifi_device_activated);
}

static void _wifi_selected_ap_destroy(app_object *app_obj)
{
    WIFI_LOG_INFO("selected_ap_destroy");
	wifi_manager_ap_destroy(app_obj->selected_wifi_ap);
	app_obj->selected_wifi_ap = NULL;
}

static void _wifi_connecting_ap_destroy(app_object *app_obj)
{
	WIFI_LOG_INFO("connecting_ap_destroy");
	wifi_manager_ap_destroy(app_obj->connecting_wifi_ap);
	app_obj->connecting_wifi_ap = NULL;
}

static void _wifi_connect_reserve_ap_destroy(app_object *app_obj)
{
	WIFI_LOG_INFO("reserve_ap_destroy");
	wifi_manager_ap_destroy(app_obj->connect_reserve_wifi_ap);
	app_obj->connect_reserve_wifi_ap = NULL;
}

static void _wifi_disconnecting_ap_destroy(app_object *app_obj)
{
	WIFI_LOG_INFO("disconnecting_ap_destroy");
	wifi_manager_ap_destroy(app_obj->disconnecting_wifi_ap);
	app_obj->disconnecting_wifi_ap = NULL;
}

static gboolean _scan_scroll_to_top_for_idle(gpointer user_data)
{
	layout_scan_object *scan = user_data;
	__WIFI_FUNC_ENTER__;
	layout_scan_ap_list_top_item_show(scan);
	return FALSE;
}

static void __wifi_manager_connection_state_changed_cb(wifi_manager_object *manager,
						       wifi_device_state_e device_state,
						       wifi_connection_state_e connection_state, wifi_ap_object *ap,
						       wifi_rssi_level_e rssi_level, gpointer user_data)
{
	app_object *app_obj = user_data;
	gchar *ssid;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	ssid = wifi_manager_ap_get_ssid(ap);
	WIFI_LOG_INFO("[%s] connection state = %s", ssid,
		      wifi_connection_state_to_string(connection_state));
	g_free(ssid);

	// Main screen: Main menu update
	if (app_obj->main) {
		layout_main_menu_update(app_obj->main, MAIN_MENU_SCAN);
	}

	// Scan screen: AP list update, show
	if (app_obj->scan) {
		Elm_Object_Item *ap_item = layout_scan_ap_list_find_item_by_data(
			app_obj->scan, ap, _compare_wifi_ap_object);
		if (ap_item) {
			wifi_manager_ap_refresh(elm_object_item_data_get(ap_item));
		}
		switch (connection_state) {
		case WIFI_CONNECTION_STATE_CONNECTED:
			layout_scan_pop_to(app_obj->scan);
			layout_scan_ap_list_item_move_to_top(app_obj->scan, ap_item);
			//elm_genlist_item_update(ap_item);
			elm_genlist_item_fields_update(ap_item, "elm.text.1", ELM_GENLIST_ITEM_FIELD_TEXT);
			idler_util_managed_idle_add(_scan_scroll_to_top_for_idle,
						    app_obj->scan);

			_release_popups(app_obj);
			break;

		case WIFI_CONNECTION_STATE_ASSOCIATION:
		case WIFI_CONNECTION_STATE_CONFIGURATION:
			layout_scan_ap_list_item_move_to_top(app_obj->scan, ap_item);
			idler_util_managed_idle_add(_scan_scroll_to_top_for_idle,
						    app_obj->scan);
			break;

		case WIFI_CONNECTION_STATE_FAILURE:
		case WIFI_CONNECTION_STATE_DISCONNECTED:
			// for auto connect to favorite ap
			_wifi_manager_scan_start(app_obj);
			elm_genlist_item_update(ap_item);
			_release_popups(app_obj);
			break;
		}
	}
}

static void __wifi_manager_wifi_use_changed_cb(keynode_t *node, void *user_data)
{
	app_object *app_obj = user_data;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	layout_main_menu_update(app_obj->main, MAIN_MENU_POWER);
	layout_main_menu_update(app_obj->main, MAIN_MENU_SCAN);
	layout_main_menu_update(app_obj->main, MAIN_MENU_EMPTY);
}

static void __wifi_manager_background_scan_cb(wifi_manager_object *manager,
					      wifi_ap_object *ap, wifi_error_e error_code, gpointer user_data)
{
	app_object *app_obj = user_data;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_IF_FAIL(manager != NULL);
	WIFI_RET_IF_FAIL(app_obj != NULL);

	WIFI_RET_IF_FAIL(app_obj->scan != NULL);

	// Response from tap scan button(connman scan).
	if (app_obj->popup_scanning != NULL && !app_obj->is_first_scan) {
		app_obj->is_scan_finished = TRUE;
		popup_scanning_dismiss(app_obj->popup_scanning);
	}

	if (error_code != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("Wi-Fi Scan failed. error code = %s",
			     wifi_error_to_string(error_code));
		return;
	}

	_scan_ap_list_update_and_show(app_obj);
}

static gboolean _app_network_callbacks_init_for_view(app_object *app_obj)
{
	wifi_error_e err;
	gboolean is_success;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_VAL_IF_FAIL(app_obj, FALSE);

	err = wifi_manager_set_device_state_changed_cb(app_obj->wifi_manager,
						       __wifi_manager_device_state_changed_cb, app_obj);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_manager_set_device_state_changed_cb() failed. err = %s",
			     wifi_error_to_string(err));
		return FALSE;
	}
	err = wifi_manager_set_background_scan_cb(app_obj->wifi_manager,
						  __wifi_manager_background_scan_cb, app_obj);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_manager_set_background_scan_cb() failed. err = %s",
			     wifi_error_to_string(err));
		return FALSE;
	}
	err = wifi_manager_set_connection_state_changed_cb(app_obj->wifi_manager,
							   __wifi_manager_connection_state_changed_cb, app_obj);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_manager_set_connection_state_changed_cb() failed. err = %s",
			     wifi_error_to_string(err));
		return FALSE;
	}

	is_success = wifi_manager_set_wifi_use_changed_cb(app_obj->wifi_manager,
							  __wifi_manager_wifi_use_changed_cb, app_obj);
	if (!is_success) {
		WIFI_LOG_ERR("wifi_manager_set_wifi_use_changed_cb() failed.");
		return FALSE;
	}

	return TRUE;
}

static gboolean _app_view_base_init(app_object *app_obj)
{
	view_base_object *base_obj;

	__WIFI_FUNC_ENTER__;

	elm_app_base_scale_set(1.3);

	base_obj = _view_base_create();
	if (!base_obj) {
		WIFI_LOG_ERR("_create_view_base() is failed.");
		return FALSE;
	}

	app_obj->base = base_obj;
	return TRUE;
}

static gboolean _is_unable_to_scan_state(wifi_manager_object *manager)
{
	return false;
}

static void _main_scan_menu_enable_init(layout_main_object *main_obj,
					wifi_manager_object *manager)
{
	gboolean is_unable_to_scan;
	bool is_wifi_activate = false;
	wifi_error_e error = wifi_manager_is_activated(manager, &is_wifi_activate);
	if (error != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_manager_is_activated() is failed. error = %s",
			     wifi_error_to_string(error));
	}
	is_unable_to_scan = _is_unable_to_scan_state(manager);

	WIFI_LOG_INFO("[Wi-Fi] is_activate       = %s",
		      is_wifi_activate ? "Y" : "N");
	WIFI_LOG_INFO("[Wi-Fi] is unable to scan = %s",
		      is_unable_to_scan ? "Y" : "N");
	if (!is_wifi_activate || is_unable_to_scan) {
		layout_main_menu_set_enable(main_obj, MAIN_MENU_SCAN, EINA_FALSE);
	}
}

static gboolean _app_main_init(app_object *app_obj)
{
	layout_main_object *main_obj;

	__WIFI_FUNC_ENTER__;

	main_obj = _main_create(app_obj->base, app_obj);
	if (!main_obj) {
		WIFI_LOG_ERR("_main_create() is failed.");
		return FALSE;
	}
	_main_scan_menu_enable_init(main_obj, app_obj->wifi_manager);
	layout_main_show(main_obj);

	app_obj->main = main_obj;
	return TRUE;
}

static view_base_object *_view_base_create()
{
	view_base_object *base_obj = NULL;

	__WIFI_FUNC_ENTER__;

	base_obj = view_base_new();
	if (!base_obj) {
		WIFI_LOG_ERR("view_base_new() is failed.");
		__WIFI_FUNC_EXIT__;
		return NULL;
	}

	if (!view_base_create(base_obj)) {
		WIFI_LOG_ERR("view_base_create() is failed.");
		view_base_free(base_obj);
		__WIFI_FUNC_EXIT__;
		return NULL;
	}

	__WIFI_FUNC_EXIT__;
	return base_obj;
}

static Eina_Bool _homekey_press_cb(void *data, int type, void *event)
{
	__WIFI_FUNC_ENTER__;
	app_object *app_obj = data;
	Evas_Event_Key_Down *ev = event;

	WIFI_RET_IF_FAIL(app_obj != NULL);
	WIFI_RET_IF_FAIL(ev != NULL);

	if (app_obj->main && strcmp(ev->keyname, "XF86PowerOff") == 0) {
		_release_popups(app_obj);
		layout_main_pop_to(app_obj->main);
	}

	return ECORE_CALLBACK_RENEW;
}

static gboolean _app_view_base_callback_init_for_keygrab(app_object *app_obj)
{
	Evas_Object *window;

	__WIFI_FUNC_ENTER__;

	if (app_obj->base) {
		window = view_base_get_window(app_obj->base);
		if (window) {
			elm_win_keygrab_set(window, "XF86PowerOff", 0, 0, 0, ELM_WIN_KEYGRAB_SHARED);
			ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, _homekey_press_cb, app_obj);
			return TRUE;
		}
	}

	return FALSE;
}

static void __popup_unable_scan_destroy_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_IF_FAIL(app_obj != NULL);

	if (app_obj->popup_unable_scan) {
		popup_unable_scan_destroy(app_obj->popup_unable_scan);
		popup_unable_scan_free(app_obj->popup_unable_scan);
		app_obj->popup_unable_scan = NULL;
	}

	if (app_obj->main) {
		layout_main_activate_rotary_event(app_obj->main);
	}
}

static popup_unable_scan_object *_popup_unable_scan_create(app_object *app_obj)
{
	popup_unable_scan_object *popup = NULL;

	__WIFI_FUNC_ENTER__;

	popup = popup_unable_scan_new(app_obj->base);
	if (!popup) {
		WIFI_LOG_ERR("popup_unable_scan_new() is failed.");
		return NULL;
	}
	popup_unable_scan_set_destroy_cb(popup,
					 __popup_unable_scan_destroy_cb, app_obj);

	if (!popup_unable_scan_create(popup)) {
		WIFI_LOG_ERR("popup_unable_scan_create() is failed.");
		popup_unable_scan_free(popup);
		return NULL;
	}
	return popup;
}

static void __detail_del_cb(void *data, Evas *e,
			    Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_IF_FAIL(app_obj);

	if (app_obj->detail) {
		layout_detail_destroy(app_obj->detail);
		layout_detail_free(app_obj->detail);
		app_obj->detail = NULL;
	}

	//_wifi_selected_ap_destroy(app_obj);
	if (app_obj->scan) {
		layout_scan_ap_list_activate_rotary_event(app_obj->scan);
	}
}

static char *__general_title_menu_item_display_ssid_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	app_object *app_obj = data;
	gchar *ssid, *markup_ssid;
	if (!app_obj) {
		WIFI_LOG_ERR("app object is NULL");
		return NULL;
	}

	ssid = wifi_manager_ap_get_ssid(app_obj->selected_wifi_ap);
	markup_ssid = elm_entry_utf8_to_markup(ssid);
	g_free(ssid);
	return markup_ssid;
}

static char *__detail_menu_status_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	if (!g_strcmp0(part, "elm.text")) {
		return g_strdup(STR_STATUS);
	} else if (!g_strcmp0(part, "elm.text.1")) {
		app_object *app_obj = data;
		wifi_connection_state_e conn_state;
		if (!app_obj) {
			WIFI_LOG_ERR("app object is NULL");
			return NULL;
		}

		conn_state = wifi_manager_ap_get_connection_state(app_obj->selected_wifi_ap);
		switch (conn_state) {
		case WIFI_CONNECTION_STATE_CONNECTED:
			return g_strdup(STR_CONNECTED);

		case WIFI_CONNECTION_STATE_DISCONNECTED:
		case WIFI_CONNECTION_STATE_FAILURE:
			WIFI_LOG_ERR("ap connection state is failure");
			return g_strdup(STR_DISCONNECTED);

		case WIFI_CONNECTION_STATE_ASSOCIATION:
			WIFI_LOG_ERR("ap connection state is disconnected");
			return g_strdup(STR_DISCONNECTED);

		case WIFI_CONNECTION_STATE_CONFIGURATION:
			WIFI_LOG_ERR("ap connection state is configuration");
			return g_strdup(STR_DISCONNECTED);

		default:
			WIFI_LOG_ERR("ap connection state error");
			return g_strdup(STR_DISCONNECTED);
		}
	}
	return NULL;
}

static char *_wifi_manager_ap_get_signal_strength_text_for_display(wifi_ap_object *ap)
{
	wifi_manager_ap_signal_strength strength =
		wifi_manager_ap_get_signal_strength(ap);

	switch (strength) {
	case WIFI_MANAGER_SIGNAL_STRENGTH_EXCELLENT:
		return g_strdup(STR_VERY_STRONG);

	case WIFI_MANAGER_SIGNAL_STRENGTH_GOOD:
		return g_strdup(STR_STRONG);

	case WIFI_MANAGER_SIGNAL_STRENGTH_WEAK:
	case WIFI_MANAGER_SIGNAL_STRENGTH_VERY_WEAK:
	case WIFI_MANAGER_SIGNAL_STRENGTH_NULL:
		return g_strdup(STR_WEAK);

	default:
		return g_strdup(STR_WEAK);
	}
}

static char *__detail_menu_strength_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	if (!g_strcmp0(part, "elm.text")) {
		return g_strdup(STR_SIGNAL_STRENGTH);
	} else if (!g_strcmp0(part, "elm.text.1")) {
		app_object *app_obj = data;
		if (!app_obj) {
			WIFI_LOG_ERR("app object is NULL");
			return NULL;
		}

		return _wifi_manager_ap_get_signal_strength_text_for_display(app_obj->selected_wifi_ap);
	}
	return NULL;
}

static char *__detail_menu_speed_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	if (!g_strcmp0(part, "elm.text")) {
		return g_strdup(STR_LINK_SPEED);
	} else if (!g_strcmp0(part, "elm.text.1")) {
		app_object *app_obj = data;
		gint max_speed;
		if (!app_obj) {
			WIFI_LOG_ERR("app object is NULL");
			return NULL;
		}

		max_speed = wifi_manager_ap_get_max_speed(app_obj->selected_wifi_ap);
		return g_strdup_printf(STR_LINK_SPEED_MBPS, max_speed);
	}
	return NULL;
}

static char *__detail_menu_ip_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	if (!g_strcmp0(part, "elm.text")) {
		return g_strdup(STR_IP_ADDRESS);
	} else if (!g_strcmp0(part, "elm.text.1")) {
		app_object *app_obj = data;
		if (!app_obj) {
			WIFI_LOG_ERR("app object is NULL");
			return NULL;
		}
		return wifi_manager_ap_get_ip_address(app_obj->selected_wifi_ap);
	}
	return NULL;
}

static gboolean _is_removable_ap(wifi_manager_object *manager, wifi_ap_object *ap)
{
	wifi_manager_csccode code = WIFI_MANAGER_CSC_UNKNOWN;
	wifi_error_e error = wifi_manager_get_csccode(manager, &code);
	gchar *ssid = wifi_manager_ap_get_ssid(ap);
	wifi_security_type_e security_type = wifi_manager_ap_get_security_type(ap);
	gboolean is_removable_ap = TRUE;

	WIFI_RET_VAL_IF_FAIL(error == WIFI_ERROR_NONE, TRUE);
	WIFI_RET_VAL_IF_FAIL(ssid != NULL, TRUE);
	if (code == WIFI_MANAGER_CSC_SKT) {
		WIFI_LOG_INFO("Target is SKT.");
		if (!g_strcmp0(ssid, "SK_VoIP")) {
			if (security_type == WIFI_SECURITY_TYPE_WPA2_PSK) {
				is_removable_ap = FALSE;
			}
		} else if (!g_strcmp0(ssid, "T wifi zone")) {
			if (security_type == WIFI_SECURITY_TYPE_NONE) {
				is_removable_ap = FALSE;
			}
		} else if (!g_strcmp0(ssid, "T wifi zone_secure")) {
			if (security_type == WIFI_SECURITY_TYPE_EAP) {
				wifi_eap_type_e eap_type = wifi_manager_ap_get_eap_type(ap);
				if (eap_type == WIFI_EAP_TYPE_AKA) {
					is_removable_ap = FALSE;
				}
				WIFI_LOG_INFO("[%s] eap type enum value is %d.", ssid, eap_type);
			}
		}
	}
	if (is_removable_ap) {
		WIFI_LOG_INFO("[%s] is removable ap.(security type enum value is %d)",
			      ssid, security_type);
	} else {
		WIFI_LOG_INFO("[%s] is not removable ap.(security type enum value is %d)",
			      ssid, security_type);
	}
	g_free(ssid);
	return is_removable_ap;
}

static void __scan_ap_item_update_last_connection_error(app_object *app_obj,
							wifi_ap_object *ap, wifi_error_e connection_error)
{
	Elm_Object_Item *ap_item = layout_scan_ap_list_find_item_by_data(
		app_obj->scan, ap, _compare_wifi_ap_object);
	wifi_ap_object *found_ap;
	gchar *ssid;
	if (!ap_item) {
		WIFI_LOG_ERR("AP item not found in AP list.");
		return;
	}

	found_ap = elm_object_item_data_get(ap_item);
	if (!found_ap) {
		WIFI_LOG_ERR("AP object not found in AP item.");
		return;
	}

	ssid = wifi_manager_ap_get_ssid(found_ap);
	if (ssid) {
		WIFI_LOG_INFO("[%s] update last error.", ssid);
		g_free(ssid);
	} else {
		WIFI_LOG_INFO("Unknown ssid AP update last error.");
	}

	wifi_manager_ap_set_last_connection_error(found_ap, connection_error);
//	elm_genlist_item_fields_update(ap_item, "elm.text.1",
//				       ELM_GENLIST_ITEM_FIELD_TEXT);
	elm_genlist_item_update(ap_item);
}

static void __on_scan_pop_transition_finished_for_forget(void *data,
							 Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	view_base_naviframe_del_transition_finished_cb(app_obj->base,
						       __on_scan_pop_transition_finished_for_forget);

	if (app_obj->selected_wifi_ap) {
		wifi_manager_ap_forget(app_obj->selected_wifi_ap);
	} else {
		WIFI_LOG_ERR("Selected wifi ap released.");
	}
}

static void __detail_forget_button_tap_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	view_base_naviframe_add_transition_finished_cb(app_obj->base,
						       __on_scan_pop_transition_finished_for_forget, app_obj);
	layout_scan_pop_to(app_obj->scan);
}

static layout_detail_object *_detail_create(app_object *app_obj, wifi_ap_object *ap)
{
	layout_detail_object *detail_obj;
	gboolean is_removable_ap = _is_removable_ap(app_obj->wifi_manager, ap);

	__WIFI_FUNC_ENTER__;

	detail_obj = layout_detail_new(app_obj->base);
	WIFI_RET_VAL_IF_FAIL(detail_obj != NULL, NULL);

	layout_detail_set_del_cb(detail_obj, __detail_del_cb, app_obj);

	layout_detail_set_menu_cb(detail_obj, DETAIL_MENU_TITLE,
				  __general_title_menu_item_display_ssid_text_get_cb, NULL, NULL, NULL, app_obj);
	layout_detail_set_menu_cb(detail_obj, DETAIL_MENU_STATUS,
				  __detail_menu_status_text_get_cb, NULL, NULL, NULL, app_obj);
	layout_detail_set_menu_cb(detail_obj, DETAIL_MENU_STRENGTH,
				  __detail_menu_strength_text_get_cb, NULL, NULL, NULL, app_obj);
	layout_detail_set_menu_cb(detail_obj, DETAIL_MENU_SPEED,
				  __detail_menu_speed_text_get_cb, NULL, NULL, NULL, app_obj);
	layout_detail_set_menu_cb(detail_obj, DETAIL_MENU_IP,
				  __detail_menu_ip_text_get_cb, NULL, NULL, NULL, app_obj);
	/* Add empty item to avoid overlap */
	layout_ap_info_set_menu_cb(detail_obj, DETAIL_MENU_EMPTY,
				   NULL, NULL, NULL, NULL, NULL, NULL);


	if (is_removable_ap) {
		layout_detail_set_tap_forget_button_cb(detail_obj,
						       __detail_forget_button_tap_cb, app_obj);
		if (!layout_detail_create(detail_obj)) {
			layout_detail_free(detail_obj);
			return NULL;
		}
	} else {
		if (!layout_detail_create_hidden_forgetbutton(detail_obj)) {
			layout_detail_free(detail_obj);
			return NULL;
		}
	}
	return detail_obj;
}

static void __scan_del_cb(void *data, Evas *e,
			  Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_IF_FAIL(app_obj);

	if (app_obj->scan) {
		layout_scan_destroy(app_obj->scan);
		layout_scan_free(app_obj->scan);
		app_obj->scan = NULL;
	}

	if (app_obj->main) {
		layout_main_activate_rotary_event(app_obj->main);
	}
}

static gchar *_wifi_ap_ssid_strdup(wifi_manager_object *manager, wifi_ap_object *ap_obj)
{
	gchar *sec_type_text =
		wifi_manager_ap_get_security_type_text(ap_obj);
	if (wifi_manager_ap_is_favorite(manager, ap_obj)) {
		gchar *wifi_ssid = g_strdup_printf("%s, %s",
						   STR_SAVED, sec_type_text);
		g_free(sec_type_text);
		return wifi_ssid;
	} else {
		if (wifi_manager_ap_is_wps_mode(ap_obj)) {
			g_free(sec_type_text);
			return g_strdup(STR_SECURED_WPS);
		} else {
			return sec_type_text;
		}
	}
}

static char *__scan_menu_ap_item_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	wifi_ap_object *ap_obj = data;

	WIFI_RET_VAL_IF_FAIL(ap_obj != NULL, NULL);

	if (!g_strcmp0(part, "elm.text")) {
		gchar *ssid, *markup_ssid;
		ssid = wifi_manager_ap_get_ssid(ap_obj);
		markup_ssid = elm_entry_utf8_to_markup(ssid);
		g_free(ssid);
		return markup_ssid;
	} else if (!g_strcmp0(part, "elm.text.1")) {
		app_object *app_obj = evas_object_data_get(obj,
							   LAYOUT_SCAN_DATA_KEY_WIFI_AP_ITEM_SELECT);
		wifi_connection_state_e state;

		WIFI_RET_VAL_IF_FAIL(app_obj != NULL, NULL);

		wifi_manager_ap_refresh(ap_obj);
		state = wifi_manager_ap_get_connection_state(ap_obj);

		switch (state) {
		case WIFI_CONNECTION_STATE_CONNECTED:
			return g_strdup(STR_CONNECTED);

		case WIFI_CONNECTION_STATE_CONFIGURATION:
			return g_strdup(STR_OBTAINING_IP);

		case WIFI_CONNECTION_STATE_ASSOCIATION:
			return g_strdup(STR_CONNECTING);

		case WIFI_CONNECTION_STATE_FAILURE:
		case WIFI_CONNECTION_STATE_DISCONNECTED:
		default:
			if (wifi_manager_ap_is_captiveportal(app_obj->wifi_manager, ap_obj))
				return g_strdup(STR_NOT_SUPPORTED);

			wifi_error_e connection_error = wifi_manager_ap_get_last_connection_error(
				app_obj->wifi_manager, ap_obj);
			if (connection_error == WIFI_ERROR_INVALID_KEY)
				return g_strdup(STR_AUTH_ERR);
			if (connection_error == WIFI_ERROR_DHCP_FAILED)
				return g_strdup(STR_FAILED_TO_OBTAIN_IP);
			return _wifi_ap_ssid_strdup(app_obj->wifi_manager, ap_obj);
		}
	}
	return NULL;
}

static Evas_Object *_get_ap_signal_strength_image_for_signal(Evas_Object *parent,
							     bool is_security_mode, wifi_manager_ap_signal_strength signal_strength)
{
	gchar image_path[PATH_MAX] = { 0, };
	gsize image_length = sizeof(image_path);
	Evas_Object *signal_image;

	if (is_security_mode) {
		g_strlcat(image_path, "wifi_secured", image_length);
	} else {
		g_strlcat(image_path, "wifi_netwokrs", image_length);
	}

	switch (signal_strength) {
	case WIFI_MANAGER_SIGNAL_STRENGTH_EXCELLENT:
		g_strlcat(image_path, "_04", image_length);
		break;

	case WIFI_MANAGER_SIGNAL_STRENGTH_GOOD:
		g_strlcat(image_path, "_03", image_length);
		break;

	case WIFI_MANAGER_SIGNAL_STRENGTH_WEAK:
		g_strlcat(image_path, "_02", image_length);
		break;

	case WIFI_MANAGER_SIGNAL_STRENGTH_VERY_WEAK:
		g_strlcat(image_path, "_01", image_length);
		break;

	case WIFI_MANAGER_SIGNAL_STRENGTH_NULL:
	default:
		g_strlcat(image_path, "_off", image_length);
		break;
	}
	g_strlcat(image_path, ".png", image_length);

	signal_image = create_icon_use_image_file(parent, image_path, NULL);
	WIFI_RET_VAL_IF_FAIL(signal_image != NULL, NULL);

	evas_object_size_hint_weight_set(signal_image, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	return signal_image;
}

static Evas_Object *_get_ap_signal_strength_image_layout(wifi_ap_object *ap_obj,
							 Evas_Object *parent)
{
	bool is_security_mode =
		wifi_manager_ap_get_security_type(ap_obj) != WIFI_SECURITY_TYPE_NONE;
	wifi_manager_ap_signal_strength signal_strength =
		wifi_manager_ap_get_signal_strength(ap_obj);
	Evas_Object *signal_image;
	Evas_Object *image_layout = create_layout_use_edj_file(parent,
							       CUSTOM_GROUP_AP_SIGNAL_IMAGE_LAYOUT);
	WIFI_RET_VAL_IF_FAIL(image_layout != NULL, NULL);
	evas_object_size_hint_weight_set(image_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	signal_image = _get_ap_signal_strength_image_for_signal(image_layout,
								is_security_mode, signal_strength);
	if (signal_image == NULL) {
		evas_object_del(image_layout);
		return NULL;
	}
	elm_object_part_content_set(image_layout, "elm.image.signal", signal_image);

	if (is_security_mode) {
		Evas_Object *lock_image = create_icon_use_image_file(image_layout,
								     "wifi_secured_locker_ic.png", "AO018");
		if (lock_image == NULL) {
			evas_object_del(signal_image);
			evas_object_del(image_layout);
			return NULL;
		}
		evas_object_size_hint_weight_set(lock_image, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_object_part_content_set(image_layout, "elm.image.lock", lock_image);
	}

	return image_layout;
}

static Evas_Object *__scan_menu_ap_item_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
	wifi_ap_object *ap_obj = data;
	WIFI_RET_VAL_IF_FAIL(ap_obj != NULL, NULL);

	if (!g_strcmp0(part, "elm.icon")) {
		return _get_ap_signal_strength_image_layout(ap_obj, obj);
	}
	return NULL;
}

static void _toast_popup_show(app_object *app_obj, const gchar *text)
{
	toast_popup_object *toast = NULL;
	WIFI_RET_IF_FAIL(app_obj);
	WIFI_RET_IF_FAIL(app_obj->base);

	toast = toast_popup_new(app_obj->base);
	if (toast && toast_popup_create(toast, text)) {
		toast_popup_show(toast);
	}
}

static gboolean _is_wps_connecting_ap(app_object *app_obj, wifi_ap_object *ap)
{
	if (app_obj->wps_progress == NULL) {
		return FALSE;
	}
	return wifi_manager_ap_is_equals(app_obj->connecting_wifi_ap, ap);
}

static void __wifi_manager_connected_cb(wifi_manager_object *manager,
					wifi_ap_object *ap, wifi_error_e error_code, gpointer user_data)
{
	app_object *app_obj = user_data;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);
	WIFI_RET_IF_FAIL(ap != NULL);

	if (error_code != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("Wi-Fi connection error. %s",
			     wifi_error_to_string(error_code));
		if (error_code == WIFI_ERROR_INVALID_KEY) {
			WIFI_LOG_ERR("Wi-Fi connection Fail WIFI_ERROR_INVALID_KEY");
			_toast_popup_show(app_obj, STR_AUTH_ERR_POPUP);
		} else if (error_code == WIFI_ERROR_DHCP_FAILED) {
			WIFI_LOG_ERR("Wi-Fi connection Fail WIFI_ERROR_DHCP_FAILED");
			_toast_popup_show(app_obj, STR_FAILED_TO_OBTAIN_IP);
		} else if (error_code == WIFI_ERROR_OPERATION_FAILED) {
			WIFI_LOG_ERR("Wi-Fi connection Fail WIFI_ERROR_OPERATION_FAILED");
			_toast_popup_show(app_obj, STR_CONNECTION_FAILED);
		}
	}

	if (app_obj->scan) {
		__scan_ap_item_update_last_connection_error(app_obj, ap, error_code);
	}
	if (_is_wps_connecting_ap(app_obj, ap)) {
		layout_wps_progress_dismiss(app_obj->wps_progress);
	}
	_wifi_connecting_ap_destroy(app_obj);

	// View update where __wifi_manager_connection_state_changed_cb()
}

static gboolean __contains_none_digit_character(const gchar *password)
{
	gchar *p = (gchar *)password;
	gunichar ch = g_utf8_get_char(p);
	while (ch) {
		if (!g_unichar_isdigit(ch)) {
			return TRUE;
		}
		p = g_utf8_next_char(p);
		ch = g_utf8_get_char(p);
	}
	return FALSE;
}

static gboolean __is_wep_password_length_max_reached(const gchar *password,
						     glong password_len)
{
	if (password == NULL) {
		return FALSE;
	}
	if (password_len == 0) {
		return FALSE;
	}
	if (__contains_none_digit_character(password)) {
		return password_len > MAX_WEP_PASSWORD_LENGTH;
	} else {
		return password_len > MAX_WEP_DIGIT_PASSWORD_LENGTH;
	}
}

static gboolean __is_password_length_max_reached(const gchar *password,
						 wifi_security_type_e sec_type)
{
	glong password_len;
	WIFI_RET_VAL_IF_FAIL(password != NULL, FALSE);

	password_len = g_utf8_strlen(password, MAX_PASSWORD_LENGTH_FOR_UNICHAR_BYTES);
	if (sec_type == WIFI_SECURITY_TYPE_WEP) {
		return __is_wep_password_length_max_reached(password, password_len);
	} else {
		return password_len > MAX_PASSWORD_LENGTH;
	}
}

static gboolean __is_valid_password_length(const gchar *password,
					   wifi_security_type_e sec_type, gboolean is_for_return_key)
{
	size_t password_len;
	WIFI_RET_VAL_IF_FAIL(password != NULL, FALSE);

	password_len = g_utf8_strlen(password, MAX_PASSWORD_LENGTH_FOR_UNICHAR_BYTES);
	if (sec_type == WIFI_SECURITY_TYPE_WEP) {
		if (is_for_return_key) {
			return password_len > 0;
		} else {
			if (password_len == 0) {
				return FALSE;
			}
			if (password_len <= MAX_WEP_PASSWORD_LENGTH) {
				return TRUE;
			}
			if (password_len == MAX_WEP_DIGIT_PASSWORD_LENGTH) {
				return TRUE;
			}
			return FALSE;
		}
	} else {
		return password_len >= MIN_PASSWORD_LENGTH && password_len <= MAX_PASSWORD_LENGTH;
	}
}

static gboolean __is_valid_password_length_for_return_key(const gchar *password,
							  wifi_security_type_e sec_type)
{
	return __is_valid_password_length(password, sec_type, TRUE);
}

static gboolean __is_valid_password_length_for_connect_button(const gchar *password,
							      wifi_security_type_e sec_type)
{
	return __is_valid_password_length(password, sec_type, FALSE);
}

static gboolean _wifi_manager_ap_update_address_for_connect(wifi_manager_object *manager,
							    wifi_ap_object *ap, wifi_address_object *address)
{
	wifi_security_type_e sec_type = wifi_address_get_security_type(address);

	if (wifi_address_is_static_ip(address)) {
		const gchar *dns2_address;
		wifi_manager_ap_set_ip_config_static(ap);
		wifi_manager_ap_set_ip_address(ap, wifi_address_get_ip_address(address));
		wifi_manager_ap_set_gateway_address(ap, wifi_address_get_gateway_address(address));
		wifi_manager_ap_set_subnet_mask(ap, wifi_address_get_subnet_mask(address));
		wifi_manager_ap_set_dns_address(ap, wifi_address_get_dns_address(address, 1), 1);

		dns2_address = wifi_address_get_dns_address(address, 2);
		if (dns2_address != NULL) {
			wifi_manager_ap_set_dns_address(ap, dns2_address, 2);
		}
	}

	if (wifi_address_is_proxy_manual(address)) {
		gchar *proxy_address = g_strdup_printf("%s:%s",
						       wifi_address_get_proxy_address(address),
						       wifi_address_get_proxy_port(address));
		wifi_manager_ap_set_proxy_manual(ap);
		wifi_manager_ap_set_proxy_address(ap, proxy_address);
		g_free(proxy_address);
	}

	if (sec_type == WIFI_SECURITY_TYPE_EAP) {
		// TODO AP forget when it's favorite ap and invalid key connection error.
		// Require C API change.

		wifi_manager_ap_set_eap_type(ap, wifi_address_get_eap_type(address));
	} else if (sec_type != WIFI_SECURITY_TYPE_NONE) {
		const gchar *password = wifi_address_get_password(address);

		if (!password) {
			WIFI_LOG_ERR("AP passphrase is NULL.");
			return FALSE;
		}

		if (!__is_valid_password_length_for_connect_button(password, sec_type)) {
			WIFI_LOG_ERR("Invalid password length.");
			return FALSE;
		}

		if (wifi_manager_ap_is_favorite(manager, ap)) {
			wifi_manager_ap_forget(ap);
		}
		wifi_manager_ap_set_password(ap, password);
	}
	return TRUE;
}

static void __wifi_disconnected_cb(wifi_manager_object *manager,
				   wifi_ap_object *ap, wifi_error_e error_code, gpointer user_data)
{
	app_object *app_obj = user_data;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);
	WIFI_RET_IF_FAIL(ap != NULL);

	if (error_code != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("Wi-Fi disconnected error. %s",
			     wifi_error_to_string(error_code));
	}

	wifi_manager_ap_destroy(ap);

	// View update where __wifi_manager_connection_state_changed_cb()
}

static void __wifi_disconnected_for_connect_cb(wifi_manager_object *manager,
					       wifi_ap_object *ap, wifi_error_e error_code, gpointer user_data)
{
	app_object *app_obj = user_data;
	wifi_error_e err;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);
	WIFI_RET_IF_FAIL(ap != NULL);

	if (error_code != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("Wi-Fi disconnected error. %s",
			     wifi_error_to_string(error_code));
	}

	_wifi_disconnecting_ap_destroy(app_obj);

	app_obj->connecting_wifi_ap = app_obj->connect_reserve_wifi_ap;
	app_obj->connect_reserve_wifi_ap = NULL;
	err = wifi_manager_connect(app_obj->wifi_manager,
				   app_obj->connecting_wifi_ap, __wifi_manager_connected_cb, app_obj);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("Wi-Fi connect request error. %s", wifi_error_to_string(err));
		if (err == WIFI_ERROR_SECURITY_RESTRICTED) {
			_toast_popup_show(app_obj, STR_RESTRICTS_USE_OF_WI_FI);
		}
		_wifi_connecting_ap_destroy(app_obj);
	}

	// View update where __wifi_manager_connection_state_changed_cb()
}

static void __wifi_disconnected_for_connect_by_wps_pbc_cb(wifi_manager_object *manager,
							  wifi_ap_object *ap, wifi_error_e error_code, gpointer user_data)
{
	app_object *app_obj = user_data;
	wifi_error_e err;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);
	WIFI_RET_IF_FAIL(ap != NULL);

	if (error_code != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("Wi-Fi disconnected error. %s",
			     wifi_error_to_string(error_code));
	}

	_wifi_disconnecting_ap_destroy(app_obj);

	app_obj->connecting_wifi_ap = app_obj->connect_reserve_wifi_ap;
	app_obj->connect_reserve_wifi_ap = NULL;
	err = wifi_manager_connect_by_wps_pbc(app_obj->wifi_manager,
					      app_obj->connecting_wifi_ap,
					      __wifi_manager_connected_cb, app_obj);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_manager_connect_by_wps_pbc() is failed. error = %s", wifi_error_to_string(err));
		if (err == WIFI_ERROR_SECURITY_RESTRICTED) {
			_toast_popup_show(app_obj, STR_RESTRICTS_USE_OF_WI_FI);
		} else {
			_toast_popup_show(app_obj, STR_CONNECTION_FAILED);
		}
		_wifi_connecting_ap_destroy(app_obj);
		layout_wps_progress_dismiss(app_obj->wps_progress);
	}

	// View update where __wifi_manager_connection_state_changed_cb()
}

static void __wifi_disconnected_for_connect_by_wps_pin_cb(wifi_manager_object *manager,
							  wifi_ap_object *ap, wifi_error_e error_code, gpointer user_data)
{
	app_object *app_obj = user_data;
	wifi_error_e err;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);
	WIFI_RET_IF_FAIL(ap != NULL);

	if (error_code != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("Wi-Fi disconnected error. %s",
			     wifi_error_to_string(error_code));
	}

	_wifi_disconnecting_ap_destroy(app_obj);

	app_obj->connecting_wifi_ap = app_obj->connect_reserve_wifi_ap;
	app_obj->connect_reserve_wifi_ap = NULL;
	err = wifi_manager_connect_by_wps_pin(app_obj->wifi_manager,
					      app_obj->connecting_wifi_ap,
					      app_obj->wps_pin_string, __wifi_manager_connected_cb, app_obj);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_manager_connect_by_wps_pin() is failed. error = %s", wifi_error_to_string(err));
		if (err == WIFI_ERROR_SECURITY_RESTRICTED) {
			_toast_popup_show(app_obj, STR_RESTRICTS_USE_OF_WI_FI);
		} else {
			_toast_popup_show(app_obj, STR_CONNECTION_FAILED);
		}
		_wifi_connecting_ap_destroy(app_obj);
		layout_wps_progress_dismiss(app_obj->wps_progress);
	}

	// View update where __wifi_manager_connection_state_changed_cb()
}

static void _wifi_reserve_connect_to_selected_ap(app_object *app_obj,
						 wifi_manager_generic_cb disconnected_cb, gpointer user_data)
{
	WIFI_RET_IF_FAIL(app_obj != NULL);
	WIFI_RET_IF_FAIL(app_obj->wifi_manager != NULL);
	WIFI_RET_IF_FAIL(app_obj->selected_wifi_ap != NULL);
	WIFI_RET_IF_FAIL(wifi_manager_ap_is_equals(app_obj->selected_wifi_ap, app_obj->connecting_wifi_ap) == FALSE);

	if (app_obj->connect_reserve_wifi_ap) {
		_wifi_connect_reserve_ap_destroy(app_obj);
	}

	app_obj->connect_reserve_wifi_ap = wifi_manager_ap_clone(app_obj->selected_wifi_ap);
	WIFI_RET_IF_FAIL(app_obj->connect_reserve_wifi_ap != NULL);

	if (app_obj->address_for_connect) {
		if (!_wifi_manager_ap_update_address_for_connect(app_obj->wifi_manager,
								 app_obj->connect_reserve_wifi_ap, app_obj->address_for_connect)) {
			WIFI_LOG_ERR("AP address update failed.");
			_wifi_connect_reserve_ap_destroy(app_obj);
			return;
		}
	}

	if (app_obj->disconnecting_wifi_ap) {
		WIFI_LOG_INFO("Disconnecting AP exists.");
	} else {
		wifi_error_e err;

		app_obj->disconnecting_wifi_ap = wifi_manager_ap_clone(app_obj->connecting_wifi_ap);
		if (!app_obj->disconnecting_wifi_ap) {
			WIFI_LOG_ERR("AP clone failed.");
			_wifi_connect_reserve_ap_destroy(app_obj);
			return;
		}
		err = wifi_manager_disconnect(app_obj->wifi_manager,
					      app_obj->disconnecting_wifi_ap,
					      disconnected_cb, user_data);
		if (err != WIFI_ERROR_NONE) {
			WIFI_LOG_ERR("wifi_manager_disconnect() is failed. error = %s",
				     wifi_error_to_string(err));
			_wifi_connect_reserve_ap_destroy(app_obj);
			_wifi_disconnecting_ap_destroy(app_obj);
		}
	}
}

static void _wifi_connect_to_selected_ap(app_object *app_obj)
{
	wifi_error_e err;

	WIFI_RET_IF_FAIL(app_obj != NULL);
	WIFI_RET_IF_FAIL(app_obj->wifi_manager != NULL);
	WIFI_RET_IF_FAIL(app_obj->selected_wifi_ap != NULL);

	app_obj->connecting_wifi_ap = wifi_manager_ap_clone(app_obj->selected_wifi_ap);

	WIFI_RET_IF_FAIL(app_obj->connecting_wifi_ap != NULL);

	if (app_obj->address_for_connect) {
		if (!_wifi_manager_ap_update_address_for_connect(app_obj->wifi_manager,
								 app_obj->connecting_wifi_ap, app_obj->address_for_connect)) {
			WIFI_LOG_ERR("AP address update failed.");
			_wifi_connecting_ap_destroy(app_obj);
			return;
		}
	}

	err = wifi_manager_connect(app_obj->wifi_manager,
				   app_obj->connecting_wifi_ap, __wifi_manager_connected_cb, app_obj);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("Wi-Fi connect request error. %s", wifi_error_to_string(err));
		if (err == WIFI_ERROR_SECURITY_RESTRICTED) {
			_toast_popup_show(app_obj, STR_RESTRICTS_USE_OF_WI_FI);
		}
		_wifi_connecting_ap_destroy(app_obj);
	}
}

static gboolean _wifi_connect_to_selected_ap_by_wps_pbc(app_object *app_obj)
{
	wifi_error_e err;

	WIFI_RET_VAL_IF_FAIL(app_obj != NULL, FALSE);
	WIFI_RET_VAL_IF_FAIL(app_obj->wifi_manager != NULL, FALSE);
	WIFI_RET_VAL_IF_FAIL(app_obj->selected_wifi_ap != NULL, FALSE);

	app_obj->connecting_wifi_ap = wifi_manager_ap_clone(app_obj->selected_wifi_ap);
	WIFI_RET_VAL_IF_FAIL(app_obj->connecting_wifi_ap != NULL, FALSE);

	err = wifi_manager_connect_by_wps_pbc(app_obj->wifi_manager,
					      app_obj->connecting_wifi_ap,
					      __wifi_manager_connected_cb, app_obj);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_manager_connect_by_wps_pbc() is failed. error = %s", wifi_error_to_string(err));
		if (err == WIFI_ERROR_SECURITY_RESTRICTED) {
			_toast_popup_show(app_obj, STR_RESTRICTS_USE_OF_WI_FI);
		} else {
			_toast_popup_show(app_obj, STR_CONNECTION_FAILED);
		}
		_wifi_connecting_ap_destroy(app_obj);
		return FALSE;
	}
	return TRUE;
}

static gboolean _wifi_connect_to_selected_ap_by_wps_pin(app_object *app_obj)
{
	wifi_error_e err;

	WIFI_RET_VAL_IF_FAIL(app_obj != NULL, FALSE);
	WIFI_RET_VAL_IF_FAIL(app_obj->wifi_manager != NULL, FALSE);
	WIFI_RET_VAL_IF_FAIL(app_obj->selected_wifi_ap != NULL, FALSE);

	app_obj->connecting_wifi_ap = wifi_manager_ap_clone(app_obj->selected_wifi_ap);
	WIFI_RET_VAL_IF_FAIL(app_obj->connecting_wifi_ap != NULL, FALSE);

	err = wifi_manager_connect_by_wps_pin(app_obj->wifi_manager,
					      app_obj->connecting_wifi_ap,
					      app_obj->wps_pin_string, __wifi_manager_connected_cb, app_obj);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_manager_connect_by_wps_pin() is failed. error = %s", wifi_error_to_string(err));
		if (err == WIFI_ERROR_SECURITY_RESTRICTED) {
			_toast_popup_show(app_obj, STR_RESTRICTS_USE_OF_WI_FI);
		} else {
			_toast_popup_show(app_obj, STR_CONNECTION_FAILED);
		}
		_wifi_connecting_ap_destroy(app_obj);
		return FALSE;
	}
	return TRUE;
}

static void _wifi_ap_completely_forget(app_object *app_obj)
{
	// Step1. AP forget
	wifi_error_e error = wifi_manager_ap_forget(app_obj->selected_wifi_ap);
	if (error == WIFI_ERROR_NONE) {
		WIFI_LOG_INFO("wifi_manager_ap_forget() success.");
		__scan_ap_item_update_last_connection_error(app_obj,
							    app_obj->selected_wifi_ap, WIFI_ERROR_NONE);
		return;
	}
	WIFI_LOG_ERR("wifi_manager_ap_forget() is failed. "
		     "error = %s", wifi_error_to_string(error));
	if (error != WIFI_ERROR_OPERATION_FAILED) {
		return;
	}

	// Step2. Removes AP configuration.
	//        if AP forget fail as WIFI_ERROR_OPERATION_FAILED...
	error = wifi_manager_ap_remove_configuration(app_obj->wifi_manager,
						     app_obj->selected_wifi_ap);
	if (error == WIFI_ERROR_NONE) {
		WIFI_LOG_INFO("wifi_manager_ap_remove_configuration() success.");
		__scan_ap_item_update_last_connection_error(app_obj,
							    app_obj->selected_wifi_ap, WIFI_ERROR_NONE);
		return;
	}
	WIFI_LOG_ERR("wifi_manager_ap_remove_configuration() is failed. "
		     "error = %s", wifi_error_to_string(error));
}

static void __popup_change_ap_destroy_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;
	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	if (app_obj->popup_change_ap) {
		switch (popup_change_ap_get_dismiss_reason(app_obj->popup_change_ap)) {
		case CHANGE_AP_DISMISS_CONNECT:
			WIFI_LOG_INFO("[ChangeAP] popup dissmissed by ok button.");
			if (app_obj->connecting_wifi_ap) {
				_wifi_reserve_connect_to_selected_ap(app_obj,
								     __wifi_disconnected_for_connect_cb, app_obj);
			} else {
				_wifi_connect_to_selected_ap(app_obj);
			}
			break;

		case CHANGE_AP_DISMISS_FORGET:
			WIFI_LOG_INFO("[ChangeAP] popup dissmissed by forget button.");
			_wifi_ap_completely_forget(app_obj);
			break;

		case CHANGE_AP_DISMISS_DEFAULT:
		case CHANGE_AP_DISMISS_CANCEL:
		default:
			WIFI_LOG_INFO("[ChangeAP] popup dissmissed.");
			break;
		}
		popup_change_ap_destroy(app_obj->popup_change_ap);
		popup_change_ap_free(app_obj->popup_change_ap);
		app_obj->popup_change_ap = NULL;
	} else {
		WIFI_LOG_ERR("[ChangeAP] popup released.");
	}

	if (app_obj->selected_wifi_ap) {
		_wifi_selected_ap_destroy(app_obj);
	} else {
		WIFI_LOG_ERR("[ChangeAP] selected ap released.");
	}

	if (app_obj->scan) {
		layout_scan_ap_list_activate_rotary_event(app_obj->scan);
	} else if (app_obj->main) {
		layout_main_activate_rotary_event(app_obj->main);
	}
}

static void __popup_change_ap_ok_button_tap_cb(void *data,
					       Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	popup_change_ap_dismiss(app_obj->popup_change_ap, CHANGE_AP_DISMISS_CONNECT);
}

static void __popup_change_ap_forget_button_tap_cb(void *data,
						   Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	popup_change_ap_dismiss(app_obj->popup_change_ap, CHANGE_AP_DISMISS_FORGET);
}

static void _popup_change_ap_show_use_ap(app_object *app_obj, wifi_ap_object *ap)
{
	gchar *rssi_text;
	gboolean is_removable_ap = _is_removable_ap(app_obj->wifi_manager, ap);

	app_obj->popup_change_ap = popup_change_ap_new(app_obj->base);
	WIFI_RET_IF_FAIL(app_obj->popup_change_ap != NULL);

	app_obj->selected_wifi_ap = wifi_manager_ap_clone(ap); // seonah
	if (app_obj->selected_wifi_ap) {
        WIFI_LOG_INFO("selected_wifi_ap isn't null!!");
	} else
	    WIFI_LOG_INFO("selected_wifi_ap is null!!");

	popup_change_ap_set_destroy_cb(app_obj->popup_change_ap,
				       __popup_change_ap_destroy_cb, app_obj);
	popup_change_ap_set_tap_ok_button_cb(app_obj->popup_change_ap,
					     __popup_change_ap_ok_button_tap_cb, app_obj);
	rssi_text = _wifi_manager_ap_get_signal_strength_text_for_display(ap);
	popup_change_ap_set_rssi_text(app_obj->popup_change_ap, rssi_text);
	g_free(rssi_text);

	if (is_removable_ap) {
		popup_change_ap_set_tap_forget_button_cb(app_obj->popup_change_ap,
							 __popup_change_ap_forget_button_tap_cb, app_obj);
		if (!popup_change_ap_create(app_obj->popup_change_ap)) {
			WIFI_LOG_ERR("popup_change_ap_create() failed.");
			popup_change_ap_free(app_obj->popup_change_ap);
			app_obj->popup_change_ap = NULL;
			return;
		}
		popup_change_ap_show(app_obj->popup_change_ap);
	} else {
		if (!popup_change_ap_create_hidden_forgetbutton(app_obj->popup_change_ap)) {
			WIFI_LOG_ERR("popup_change_ap_create_hidden_forgetbutton() failed.");
			popup_change_ap_free(app_obj->popup_change_ap);
			app_obj->popup_change_ap = NULL;
			return;
		}
		popup_change_ap_show(app_obj->popup_change_ap);
	}
}

static void __ap_info_del_cb(void *data, Evas *e,
			     Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_IF_FAIL(app_obj);

	_wifi_selected_ap_destroy(app_obj);
	if (app_obj->address_for_connect) {
		wifi_address_free(app_obj->address_for_connect);
		app_obj->address_for_connect = NULL;
	}

	if (app_obj->ap_info) {
		layout_ap_info_destroy(app_obj->ap_info);
		layout_ap_info_free(app_obj->ap_info);
		app_obj->ap_info = NULL;
	}
	if (app_obj->scan) {
		layout_scan_ap_list_activate_rotary_event(app_obj->scan);
	}
}

static char *__ap_info_menu_eap_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	app_object *app_obj = data;

	if (!app_obj) {
		WIFI_LOG_ERR("app object is NULL");
		return NULL;
	}

	if (!g_strcmp0(part, "elm.text")) {
		return g_strdup(STR_EAP_METHOD_MENU);
	} else if (!g_strcmp0(part, "elm.text.1")) {
		wifi_eap_type_e eap_type =
			wifi_address_get_eap_type(app_obj->address_for_connect);
		if (eap_type == WIFI_EAP_TYPE_SIM) {
			return g_strdup("SIM");
		} else if (eap_type == WIFI_EAP_TYPE_AKA) {
			return g_strdup("AKA");
		}
	}
	return NULL;
}

static void __eap_method_del_cb(void *data, Evas *e,
				Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	if (app_obj->eap_method) {
		layout_eap_method_destroy(app_obj->eap_method);
		layout_eap_method_free(app_obj->eap_method);
		app_obj->eap_method = NULL;
	}

	if (app_obj->ap_info) {
		layout_ap_info_activate_rotary_event(app_obj->ap_info);
	} else if (app_obj->scan) {
		layout_scan_ap_list_activate_rotary_event(app_obj->scan);
	} else if (app_obj->main) {
		layout_main_activate_rotary_event(app_obj->main);
	}
}

static void __eap_method_menu_aka_tap_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	wifi_address_set_eap_type(app_obj->address_for_connect, WIFI_EAP_TYPE_AKA);
	layout_ap_info_menu_update(app_obj->ap_info, AP_INFO_MENU_EAP);
	layout_eap_method_pop(app_obj->eap_method);
}

static void __eap_method_menu_sim_tap_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	wifi_address_set_eap_type(app_obj->address_for_connect, WIFI_EAP_TYPE_SIM);
	layout_ap_info_menu_update(app_obj->ap_info, AP_INFO_MENU_EAP);
	layout_eap_method_pop(app_obj->eap_method);
}

static layout_eap_method_object *_eap_method_create(app_object *app_obj)
{
	layout_eap_method_object *eap_method = layout_eap_method_new(app_obj->base);
	if (!eap_method) {
		WIFI_LOG_ERR("layout_eap_method_new() is failed.");
		return NULL;
	}

	layout_eap_method_set_del_cb(eap_method,
				     __eap_method_del_cb, app_obj);
	layout_eap_method_set_menu_cb(eap_method, EAP_METHOD_MENU_AKA,
				      NULL, NULL, NULL, NULL,
				      __eap_method_menu_aka_tap_cb, app_obj);
	layout_eap_method_set_menu_cb(eap_method, EAP_METHOD_MENU_SIM,
				      NULL, NULL, NULL, NULL,
				      __eap_method_menu_sim_tap_cb, app_obj);

	if (!layout_eap_method_create(eap_method)) {
		WIFI_LOG_ERR("layout_eap_method_create() is failed.");
		layout_eap_method_free(eap_method);
		return NULL;
	}
	return eap_method;
}

static void __ap_info_menu_eap_tap_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;
	Elm_Object_Item *item = event_info;

	__WIFI_FUNC_ENTER__;

	if (!app_obj) {
		WIFI_LOG_ERR("app object is NULL");
		return;
	}
	elm_genlist_item_selected_set(item, EINA_FALSE);

	app_obj->eap_method = _eap_method_create(app_obj);
	if (app_obj->eap_method) {
		layout_eap_method_show(app_obj->eap_method);
	}
}

static void __wps_method_del_cb(void *data, Evas *e,
				Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_IF_FAIL(app_obj);

	if (app_obj->wps_method) {
		layout_wps_method_destroy(app_obj->wps_method);
		layout_wps_method_free(app_obj->wps_method);
		app_obj->wps_method = NULL;
	}

	if (app_obj->ap_info) {
		layout_ap_info_activate_rotary_event(app_obj->ap_info);
	} else if (app_obj->scan) {
		layout_scan_ap_list_activate_rotary_event(app_obj->scan);
	} else if (app_obj->main) {
		layout_main_activate_rotary_event(app_obj->main);
	}
}

static void __wps_progress_destroy_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_IF_FAIL(app_obj != NULL);

	if (app_obj->wps_progress) {
		layout_wps_progress_destroy(app_obj->wps_progress);
		layout_wps_progress_free(app_obj->wps_progress);
		app_obj->wps_progress = NULL;
	}
	if (app_obj->wps_method) {
		layout_wps_method_activate_rotary_event(app_obj->wps_method);
	} else if (app_obj->scan) {
		layout_scan_ap_list_activate_rotary_event(app_obj->scan);
	} else if (app_obj->main) {
		layout_main_activate_rotary_event(app_obj->main);
	}

	if (app_obj->wps_pin_string) {
		g_free(app_obj->wps_pin_string);
		app_obj->wps_pin_string = NULL;
	}


	WIFI_RET_IF_FAIL(app_obj->selected_wifi_ap != NULL);
	wifi_manager_ap_refresh(app_obj->selected_wifi_ap);
	if (wifi_manager_ap_get_connection_state(app_obj->selected_wifi_ap)
	    != WIFI_CONNECTION_STATE_CONNECTED) {
		wifi_error_e err;
		wifi_ap_object *ap_obj = wifi_manager_ap_clone(app_obj->selected_wifi_ap);
		WIFI_RET_IF_FAIL(ap_obj != NULL);

		err = wifi_manager_disconnect(app_obj->wifi_manager,
					      ap_obj, __wifi_disconnected_cb, app_obj);
		if (err != WIFI_ERROR_NONE) {
			WIFI_LOG_ERR("wifi_manager_disconnect() is failed. error = %s",
				     wifi_error_to_string(err));
			wifi_manager_ap_destroy(ap_obj);
		}
	}
}

static gchar *_wps_progress_get_wps_button_label_text()
{
	gchar *wps_text = g_strdup_printf(STR_WPS_BTN_STR, 2);
	gchar *wps_label_text = g_strdup_printf("<font_size=32><valign=center><align=center>%s</align></valign></font_size>", wps_text);
	g_free(wps_text);

	return wps_label_text;
}

static gchar *_wps_progress_generate_wps_pin_text(wifi_manager_object *manager)
{
	gchar *pin = NULL;
	if (wifi_manager_generate_wps_pin(manager, &pin) != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("WPS pin generate failed.");
		return NULL;
	}
	return pin;
}

static gchar *_wps_progress_get_wps_pin_label_text(gchar *origin_pin_text)
{
	gchar **tokens = g_strsplit(STR_WPS_PIN_STR, "\n", 2);
	gchar *pin_label_text = g_strdup_printf("<font_size=32><valign=center><align=center>%s<br>%s</align></valign></font_size>", tokens[0], origin_pin_text);
	g_strfreev(tokens);
	return pin_label_text;
}

static layout_wps_progress_object *_wps_progress_wps_button_create(app_object *app_obj)
{
	layout_wps_progress_object *wps_progress;
	gchar *wps_label_text;

	wps_progress = layout_wps_progress_new(app_obj->base);
	if (!wps_progress) {
		WIFI_LOG_ERR("WPS pin text generate failed.");
		return NULL;
	}

	wps_label_text = _wps_progress_get_wps_button_label_text();
	layout_wps_progress_set_label_text(wps_progress, wps_label_text);
	g_free(wps_label_text);

	layout_wps_progress_set_destroy_cb(wps_progress,
					   __wps_progress_destroy_cb, app_obj);
	if (!layout_wps_progress_create(wps_progress)) {
		WIFI_LOG_ERR("wps progress create failed.");
		layout_wps_progress_free(wps_progress);
		return NULL;
	}
	return wps_progress;
}

static layout_wps_progress_object *_wps_progress_wps_pin_create(app_object *app_obj)
{
	layout_wps_progress_object *wps_progress;
	gchar *pin_label_text;

	app_obj->wps_pin_string = _wps_progress_generate_wps_pin_text(app_obj->wifi_manager);
	if (!app_obj->wps_pin_string) {
		WIFI_LOG_ERR("WPS pin text generate failed.");
		return NULL;
	}

	wps_progress = layout_wps_progress_new(app_obj->base);
	if (!wps_progress) {
		WIFI_LOG_ERR("WPS pin progress alloc failed.");
		g_free(app_obj->wps_pin_string);
		app_obj->wps_pin_string = NULL;
		return NULL;
	}
	pin_label_text = _wps_progress_get_wps_pin_label_text(app_obj->wps_pin_string);
	layout_wps_progress_set_label_text(wps_progress,
					   pin_label_text);
	g_free(pin_label_text);

	layout_wps_progress_set_destroy_cb(wps_progress,
					   __wps_progress_destroy_cb, app_obj);
	if (!layout_wps_progress_create(wps_progress)) {
		WIFI_LOG_ERR("wps progress create failed.");
		g_free(app_obj->wps_pin_string);
		app_obj->wps_pin_string = NULL;
		layout_wps_progress_free(wps_progress);
		return NULL;
	}
	return wps_progress;
}

static void __wps_progress_wps_button_show_finished_cb(void *data,
						       Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	if (app_obj->connecting_wifi_ap) {
		_wifi_reserve_connect_to_selected_ap(app_obj,
						     __wifi_disconnected_for_connect_by_wps_pbc_cb, app_obj);
	} else {
		if (!_wifi_connect_to_selected_ap_by_wps_pbc(app_obj)) {
			layout_wps_progress_dismiss(app_obj->wps_progress);
		}
	}
}

static void __wps_method_menu_wps_button_tap_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;
	Elm_Object_Item *item = event_info;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	elm_genlist_item_selected_set(item, EINA_FALSE);

	app_obj->wps_progress = _wps_progress_wps_button_create(app_obj);
	if (app_obj->wps_progress) {
		layout_wps_progress_set_show_finished_cb(app_obj->wps_progress,
							 __wps_progress_wps_button_show_finished_cb, app_obj);
		layout_wps_progress_show(app_obj->wps_progress);
		layout_wps_progress_activate_rotary_event(app_obj->wps_progress);
	}
}

static void __wps_progress_wps_pin_show_finished_cb(void *data,
						    Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	if (app_obj->connecting_wifi_ap) {
		_wifi_reserve_connect_to_selected_ap(app_obj,
						     __wifi_disconnected_for_connect_by_wps_pin_cb, app_obj);
	} else {
		if (!_wifi_connect_to_selected_ap_by_wps_pin(app_obj)) {
			layout_wps_progress_dismiss(app_obj->wps_progress);
		}
	}
}

static void __wps_method_menu_wps_pin_tap_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;
	Elm_Object_Item *item = event_info;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	elm_genlist_item_selected_set(item, EINA_FALSE);

	app_obj->wps_progress = _wps_progress_wps_pin_create(app_obj);
	if (app_obj->wps_progress) {
		layout_wps_progress_set_show_finished_cb(app_obj->wps_progress,
							 __wps_progress_wps_pin_show_finished_cb, app_obj);
		layout_wps_progress_show(app_obj->wps_progress);
		layout_wps_progress_activate_rotary_event(app_obj->wps_progress);
	}
}

static layout_wps_method_object *_wps_method_create(app_object *app_obj)
{
	layout_wps_method_object *wps_method = layout_wps_method_new(app_obj->base);
	WIFI_RET_VAL_IF_FAIL(wps_method != NULL, NULL);

	layout_wps_method_set_del_cb(wps_method,
				     __wps_method_del_cb, app_obj);
	layout_wps_method_set_menu_cb(wps_method, WPS_METHOD_MENU_WPS_BUTTON,
				      NULL, NULL, NULL, NULL,
				      __wps_method_menu_wps_button_tap_cb, app_obj);
	layout_wps_method_set_menu_cb(wps_method, WPS_METHOD_MENU_WPS_PIN,
				      NULL, NULL, NULL, NULL,
				      __wps_method_menu_wps_pin_tap_cb, app_obj);

	if (!layout_wps_method_create(wps_method, app_obj)) {
		WIFI_LOG_ERR("layout_wps_method_create() is failed.");
		layout_wps_method_free(wps_method);
		return NULL;
	}
	return wps_method;
}

static void __ap_info_menu_wps_tap_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;
	Elm_Object_Item *item = event_info;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	elm_genlist_item_selected_set(item, EINA_FALSE);

	app_obj->wps_method = _wps_method_create(app_obj);
	if (app_obj->wps_method) {
		layout_wps_method_show(app_obj->wps_method);
	}
}

static void __password_entry_del_cb(void *data, Evas *e,
				    Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_IF_FAIL(app_obj);

	if (app_obj->password_entry) {
		layout_password_entry_destroy(app_obj->password_entry);
		layout_password_entry_free(app_obj->password_entry);
		app_obj->password_entry = NULL;
	}

	if (app_obj->ap_info) {
		layout_ap_info_activate_rotary_event(app_obj->ap_info);
	}
}

static void __virtualkeypad_size_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;
	Evas_Coord_Rectangle *rect = event_info;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);
	WIFI_LOG_INFO("[virtualkeypad,size,changed] height = %d", rect->h);

	if (rect->h > KEYPAD_PREDICTION_ON_HEIGHT_MIN) {
		layout_wearable_input_prediction_on(app_obj->wearable_input);
	} else if (rect->h > KEYPAD_PREDICTION_OFF_HEIGHT_MIN) {
		layout_wearable_input_prediction_off(app_obj->wearable_input);
	}
}

static void __wearable_input_del_cb(void *data, Evas *e,
				    Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	view_base_conformant_del_virtualkeypad_size_changed_cb(app_obj->base,
							       __virtualkeypad_size_changed_cb);

	if (app_obj->wearable_input) {
		layout_wearable_input_destroy(app_obj->wearable_input);
		layout_wearable_input_free(app_obj->wearable_input);
		app_obj->wearable_input = NULL;
	}

	if (app_obj->static_ip) {
		layout_static_ip_activate_rotary_event(app_obj->static_ip);
	} else if (app_obj->proxy_setting) {
		layout_proxy_setting_activate_rotary_event(app_obj->proxy_setting);
	}
}

// Remove next tags.
// <preedit><underline_color=#b2b2b2ff>
// </underline_color></preedit>
static void _password_filtering(gchar **password)
{
	gchar *first, *second;
	glong first_offset, second_offset;
	WIFI_RET_IF_FAIL(password != NULL);
	WIFI_RET_IF_FAIL(*password != NULL);

	first = g_strstr_len(*password, -1, "<preedit><underline_color=");
	if (first == NULL) {
		return;
	}
	second = g_strstr_len(first, -1, "</underline_color></preedit>");
	if (second == NULL) {
		return;
	}

	WIFI_LOG_INFO("Preedit, underline_color tags found.");
	first_offset = g_utf8_pointer_to_offset(*password, first);
	second_offset = g_utf8_pointer_to_offset(*password, second);
	first = g_utf8_substring(*password, 0, first_offset);
	second = g_utf8_substring(*password, second_offset - 1, second_offset);
	g_free(*password);
	*password = g_strdup_printf("%s%s", first, second);
	g_free(first);
	g_free(second);
}

static void __password_entry_wearable_input_clicked_cb(void *data,
						       Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;
	gchar *password;
	wifi_security_type_e sec_type;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	if (layout_password_entry_checkbox_is_checked(app_obj->password_entry)) {
		password = elm_entry_markup_to_utf8(elm_entry_entry_get(obj));
	} else {
		password = g_strdup(elm_entry_entry_get(obj));
	}
	_password_filtering(&password);
	sec_type = wifi_manager_ap_get_security_type(app_obj->selected_wifi_ap);
	layout_wearable_input_set_input_return_key_enable(app_obj->wearable_input,
							  __is_valid_password_length_for_return_key(password, sec_type));
	if (__is_password_length_max_reached(password, sec_type)) {
		_toast_popup_show(app_obj, STR_MAXIMUM_NUMBER);
	}
	g_free(password);
}

static void __password_entry_wearable_input_maxlength_reached_cb(void *data,
								 Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	_toast_popup_show(app_obj, STR_MAXIMUM_NUMBER);
}

static void __password_entry_wearable_input_activated_cb(void *data,
							 Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;
	gchar *password;
	wifi_security_type_e sec_type;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	if (layout_password_entry_checkbox_is_checked(app_obj->password_entry)) {
		password = elm_entry_markup_to_utf8(elm_entry_entry_get(obj));
	} else {
		password = g_strdup(elm_entry_entry_get(obj));
	}
	_password_filtering(&password);
	sec_type = wifi_manager_ap_get_security_type(app_obj->selected_wifi_ap);
	wifi_address_set_password(app_obj->address_for_connect, password);
    WIFI_LOG_INFO("Password[%s]", password);
	layout_ap_info_set_connect_button_enable(app_obj->ap_info,
						 __is_valid_password_length_for_connect_button(password, sec_type));
	layout_ap_info_pop_to(app_obj->ap_info);
	g_free(password);
}

static layout_wearable_input_object *_password_entry_wearable_input_create(app_object *app_obj)
{
	layout_wearable_input_object *wearable_input = layout_wearable_input_new(app_obj->base);
	int password_maxlen = MAX_PASSWORD_LENGTH;
	if (!wearable_input) {
		WIFI_LOG_ERR("layout_wearable_input_new() is failed.");
		return NULL;
	}

	layout_wearable_input_set_del_cb(wearable_input,
					 __wearable_input_del_cb, app_obj);

	layout_wearable_input_set_input_type(wearable_input, ELM_INPUT_PANEL_LAYOUT_PASSWORD);
	layout_wearable_input_set_input_guide_text(wearable_input, STR_PASSWORD_HEADER);
	layout_wearable_input_set_input_text(wearable_input,
					     wifi_address_get_password(app_obj->address_for_connect));
	layout_wearable_input_set_input_show(wearable_input,
					     layout_password_entry_checkbox_is_checked(app_obj->password_entry));

	layout_wearable_input_set_input_changed_cb(wearable_input,
						   __password_entry_wearable_input_clicked_cb, app_obj);
	layout_wearable_input_set_input_activated_cb(wearable_input,
						     __password_entry_wearable_input_activated_cb, app_obj);

	if (wifi_manager_ap_get_security_type(app_obj->selected_wifi_ap) == WIFI_SECURITY_TYPE_WEP) {
		password_maxlen = MAX_WEP_DIGIT_PASSWORD_LENGTH;
	}
	layout_wearable_input_set_input_maxlength_reached_cb(wearable_input,
							     password_maxlen, __password_entry_wearable_input_maxlength_reached_cb, app_obj);
	if (!layout_wearable_input_create(wearable_input)) {
		WIFI_LOG_ERR("layout_wearable_input_create() is failed.");
		layout_wearable_input_free(wearable_input);
		return NULL;
	}

	//layout_wearable_input_set_input_return_key_enable(wearable_input, EINA_FALSE);
	return wearable_input;
}

static void __password_entry_entry_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_IF_FAIL(app_obj);
	WIFI_RET_IF_FAIL(app_obj->wearable_input == NULL);

	app_obj->wearable_input = _password_entry_wearable_input_create(app_obj);
	if (app_obj->wearable_input) {
		view_base_conformant_add_virtualkeypad_size_changed_cb(app_obj->base,
								       __virtualkeypad_size_changed_cb, app_obj);
		layout_wearable_input_show(app_obj->wearable_input);
		layout_wearable_input_set_input_focus(app_obj->wearable_input, EINA_TRUE);
	}
}

static void __password_entry_checkbox_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	layout_password_entry_set_show_password(app_obj->password_entry, elm_check_state_get(obj));
}

static layout_password_entry_object *_password_entry_create(app_object *app_obj)
{
	layout_password_entry_object *password_entry = layout_password_entry_new(app_obj->base);
	if (!password_entry) {
		WIFI_LOG_ERR("layout_password_entry_new() is failed.");
		return NULL;
	}

	layout_password_entry_set_del_cb(password_entry,
					 __password_entry_del_cb, app_obj);
	layout_password_entry_set_entry_clicked_cb(password_entry,
						   __password_entry_entry_clicked_cb, app_obj);
	layout_password_entry_set_checkbox_changed_cb(password_entry,
						      __password_entry_checkbox_changed_cb, app_obj);

	if (!layout_password_entry_create(password_entry)) {
		WIFI_LOG_ERR("layout_password_entry_create() is failed.");
		layout_password_entry_free(password_entry);
		return NULL;
	}
	layout_password_entry_set_ckeckbox_enable(password_entry, !_mdm_is_password_hidden());
	layout_password_entry_set_entry_text(password_entry,
					     wifi_address_get_password(app_obj->address_for_connect));
	return password_entry;
}

static void __ap_info_menu_password_tap_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;
	Elm_Object_Item *item = event_info;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_IF_FAIL(app_obj != NULL);
	elm_genlist_item_selected_set(item, EINA_FALSE);

	app_obj->password_entry = _password_entry_create(app_obj);
	if (app_obj->password_entry) {
		if (app_obj->ap_info) {
			layout_ap_info_deactivate_rotary_event(app_obj->ap_info);
		}
		layout_password_entry_show(app_obj->password_entry);
		layout_password_entry_set_show_password(app_obj->password_entry, EINA_FALSE);
	}
}

static void __static_ip_del_cb(void *data, Evas *e,
			       Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj);

	if (app_obj->address_for_edit) {
		wifi_address_free(app_obj->address_for_edit);
		app_obj->address_for_edit = NULL;
	}

	if (app_obj->static_ip) {
		layout_static_ip_destroy(app_obj->static_ip);
		layout_static_ip_free(app_obj->static_ip);
		app_obj->static_ip = NULL;
	}

	if (app_obj->ap_info) {
		layout_ap_info_activate_rotary_event(app_obj->ap_info);
	}
}

static int _convert_colorcode_to_hex(const gchar *code)
{
	int r = 0, g = 0, b = 0, a = 0;
	return (r << 24) + (g << 16) + (b << 8) + a;
}

static gchar *_make_static_ip_menu_text(const gchar *text, const gchar *default_text)
{
	if (text != NULL) {
		return g_strdup_printf("<color=#%08x>%s</color>",
				       _convert_colorcode_to_hex("T022"), text);
	} else if (default_text != NULL) {
		return g_strdup_printf("<color=#%08x>%s</color>",
				       _convert_colorcode_to_hex("T022D"), default_text);
	} else {
		return NULL;
	}
}

static char *__static_ip_menu_ip_address_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	app_object *app_obj = data;
	WIFI_RET_VAL_IF_FAIL(app_obj != NULL, NULL);

	if (!g_strcmp0(part, "elm.text")) {
		return g_strdup(STR_IP_ADDRESS);
	} else if (!g_strcmp0(part, "elm.text.1")) {
		return _make_static_ip_menu_text(
			       wifi_address_get_ip_address(app_obj->address_for_edit),
			       DEFAULT_GUIDE_IP_ADDRESS);
	}
	return NULL;
}

static char *__static_ip_menu_gateway_address_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	app_object *app_obj = data;
	WIFI_RET_VAL_IF_FAIL(app_obj != NULL, NULL);

	if (!g_strcmp0(part, "elm.text")) {
		return g_strdup(STR_GATYEWAY);
	} else if (!g_strcmp0(part, "elm.text.1")) {
		return _make_static_ip_menu_text(
			       wifi_address_get_gateway_address(app_obj->address_for_edit),
			       DEFAULT_GUIDE_GATEWAY_ADDRESS);
	}
	return NULL;
}

static char *__static_ip_menu_subnet_mask_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	app_object *app_obj = data;
	WIFI_RET_VAL_IF_FAIL(app_obj != NULL, NULL);

	if (!g_strcmp0(part, "elm.text")) {
		return g_strdup(STR_SUBNETMASK);
	} else if (!g_strcmp0(part, "elm.text.1")) {
		return _make_static_ip_menu_text(
			       wifi_address_get_subnet_mask(app_obj->address_for_edit),
			       DEFAULT_GUIDE_SUBNET_MASK);
	}
	return NULL;
}

static char *__static_ip_menu_dns1_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	app_object *app_obj = data;
	WIFI_RET_VAL_IF_FAIL(app_obj != NULL, NULL);

	if (!g_strcmp0(part, "elm.text")) {
		return g_strdup_printf(STR_DNS, 1);
	} else if (!g_strcmp0(part, "elm.text.1")) {
		return _make_static_ip_menu_text(
			       wifi_address_get_dns_address(app_obj->address_for_edit, 1),
			       DEFAULT_GUIDE_DNS1);
	}
	return NULL;
}

static char *__static_ip_menu_dns2_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	app_object *app_obj = data;
	WIFI_RET_VAL_IF_FAIL(app_obj != NULL, NULL);

	if (!g_strcmp0(part, "elm.text")) {
		return g_strdup_printf(STR_DNS, 2);
	} else if (!g_strcmp0(part, "elm.text.1")) {
		return _make_static_ip_menu_text(
			       wifi_address_get_dns_address(app_obj->address_for_edit, 2),
			       DEFAULT_GUIDE_DNS2);
	}
	return NULL;
}

static gboolean __is_valid_static_ip_string_length(const gchar *text)
{
	if (!text) {
		WIFI_LOG_ERR("IP address text is NULL");
		return FALSE;
	} else if (strlen(text) == 0) {
		WIFI_LOG_ERR("IP address text string length = 0");
		return FALSE;
	}
	return TRUE;
}

static void __static_ip_wearable_input_changed_cb(void *data, Evas_Object *obj,
						  void *event_info)
{
	app_object *app_obj = data;
	int i = 0;
	int ip_addr[4] = { 0 };
	char entry_ip_text[16] = { 0, };
	gboolean fixed = FALSE;
	char *entry_text = NULL;
	char **ip_text = NULL;
	char saved = '\n';
	int length = 0;
	WIFI_RET_IF_FAIL(app_obj != NULL);
	WIFI_RET_IF_FAIL(obj != NULL);

	entry_text = elm_entry_markup_to_utf8(elm_entry_entry_get(obj));
	if (!__is_valid_static_ip_string_length(entry_text)) {
		WIFI_LOG_ERR("Invalid IP address.");
		layout_wearable_input_set_input_return_key_enable(app_obj->wearable_input, EINA_FALSE);
		free(entry_text);
		return;
	}

	ip_text = g_strsplit(entry_text, ".", 5);
	for (i = 0; i < 5; i++) {
		if (ip_text[i] == NULL)
			break;

		if (i == 4) {
			fixed = TRUE;
			break;
		}

		ip_addr[i] = atoi(ip_text[i]);
		if (ip_addr[i] > 255) {
			length = strlen(ip_text[i]);
			saved = ip_text[i][length - 1];
			ip_text[i][length - 1] = '\n';
			ip_addr[i] = atoi(ip_text[i]);
			fixed = TRUE;
		}

		if (i < 3) {
			if (saved == '\n')
				g_snprintf(entry_text, 5, "%d.", ip_addr[i]);
			else
				g_snprintf(entry_text, 6, "%d.%c", ip_addr[i], saved);
		} else
			g_snprintf(entry_text, 4, "%d", ip_addr[i]);

		g_strlcat(entry_ip_text, entry_text, sizeof(entry_ip_text));
	}

	if (fixed) {
		i = elm_entry_cursor_pos_get(obj);
		elm_entry_entry_set(obj, entry_ip_text);
		elm_entry_cursor_pos_set(obj, i + 1);
		layout_wearable_input_set_input_return_key_enable(app_obj->wearable_input,
								  __is_valid_static_ip_string_length(entry_ip_text));
	} else {
		layout_wearable_input_set_input_return_key_enable(app_obj->wearable_input,
								  __is_valid_static_ip_string_length(entry_text));
	}
	g_free(entry_text);
	g_strfreev(ip_text);
}

static gchar *_make_gateway_address_use_ip_address(const gchar *ip_address)
{
	gint ip_addr[4] = { 0 };
	sscanf(ip_address, "%d.%d.%d.%d", &ip_addr[0], &ip_addr[1],
	       &ip_addr[2], &ip_addr[3]);
	return g_strdup_printf("%d.%d.%d.1", ip_addr[0], ip_addr[1],
			       ip_addr[2]);
}

static void __static_ip_wearable_input_activated_cb(void *data, Evas_Object *obj,
						    void *event_info)
{
	app_object *app_obj = data;
	static_ip_menu_type selected_menu;
	gchar *ip_addr_text = NULL;
	gint ip_addr[4] = { 0 };

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	ip_addr_text = elm_entry_markup_to_utf8(elm_entry_entry_get(obj));
	if (!__is_valid_static_ip_string_length(ip_addr_text)) {
		WIFI_LOG_ERR("Invalid IP address.");
		free(ip_addr_text);
		return;
	}

	sscanf(ip_addr_text, "%d.%d.%d.%d", &ip_addr[0], &ip_addr[1],
	       &ip_addr[2], &ip_addr[3]);
	g_snprintf(ip_addr_text, 16, "%d.%d.%d.%d", ip_addr[0], ip_addr[1],
		   ip_addr[2], ip_addr[3]);

	selected_menu = layout_static_ip_get_selected_menu(app_obj->static_ip);
	switch (selected_menu) {
	case STATIC_IP_ITEM_IP_ADDRESS:
		wifi_address_set_ip_address(app_obj->address_for_edit, ip_addr_text);
		if (wifi_address_get_gateway_address(app_obj->address_for_edit) == NULL) {
			gchar *gateway_address = _make_gateway_address_use_ip_address(ip_addr_text);
			wifi_address_set_gateway_address(app_obj->address_for_edit, gateway_address);
			layout_static_ip_update_menu(app_obj->static_ip, STATIC_IP_ITEM_GATEWAY);
			g_free(gateway_address);
		}
		if (wifi_address_get_subnet_mask(app_obj->address_for_edit) == NULL) {
			wifi_address_set_subnet_mask(app_obj->address_for_edit, DEFAULT_GUIDE_SUBNET_MASK);
			layout_static_ip_update_menu(app_obj->static_ip, STATIC_IP_ITEM_SUBNET_MASK);
		}
		if (wifi_address_get_dns_address(app_obj->address_for_edit, 1) == NULL) {
			wifi_address_set_dns_address(app_obj->address_for_edit, DEFAULT_GUIDE_DNS1, 1);
			layout_static_ip_update_menu(app_obj->static_ip, STATIC_IP_ITEM_DNS1);
		}
		layout_static_ip_save_button_set_enable(app_obj->static_ip);
		break;

	case STATIC_IP_ITEM_GATEWAY:
		wifi_address_set_gateway_address(app_obj->address_for_edit, ip_addr_text);
		break;

	case STATIC_IP_ITEM_SUBNET_MASK:
		wifi_address_set_subnet_mask(app_obj->address_for_edit, ip_addr_text);
		break;

	case STATIC_IP_ITEM_DNS1:
		wifi_address_set_dns_address(app_obj->address_for_edit, ip_addr_text, 1);
		break;

	case STATIC_IP_ITEM_DNS2:
		wifi_address_set_dns_address(app_obj->address_for_edit, ip_addr_text, 2);
		break;

	default:
		WIFI_LOG_ERR("Invalid selected menu type = %d", selected_menu);
		g_free(ip_addr_text);
		layout_static_ip_pop_to(app_obj->static_ip);
		return;
	}
	layout_static_ip_update_menu(app_obj->static_ip, selected_menu);
	g_free(ip_addr_text);

	layout_static_ip_pop_to(app_obj->static_ip);
}

static layout_wearable_input_object *_static_ip_wearable_input_create(app_object *app_obj,
							       const gchar *text, gboolean is_guide_text)
{
	layout_wearable_input_object *wearable_input = layout_wearable_input_new(app_obj->base);
	if (!wearable_input) {
		WIFI_LOG_ERR("layout_wearable_input_new() is failed.");
		return NULL;
	}

	layout_wearable_input_set_del_cb(wearable_input,
					 __wearable_input_del_cb, app_obj);

	layout_wearable_input_set_input_type(wearable_input, ELM_INPUT_PANEL_LAYOUT_IP);
	if (is_guide_text) {
		layout_wearable_input_set_input_guide_text(wearable_input, text);
	} else {
		layout_wearable_input_set_input_text(wearable_input, text);
	}
	layout_wearable_input_set_input_show(wearable_input, EINA_TRUE);

	layout_wearable_input_set_input_changed_cb(wearable_input,
						   __static_ip_wearable_input_changed_cb, app_obj);
	layout_wearable_input_set_input_activated_cb(wearable_input,
						     __static_ip_wearable_input_activated_cb, app_obj);

	if (!layout_wearable_input_create(wearable_input)) {
		WIFI_LOG_ERR("layout_wearable_input_create() is failed.");
		layout_wearable_input_free(wearable_input);
		return NULL;
	}

	if (is_guide_text || !text || strlen(text) == 0) {
		layout_wearable_input_set_input_return_key_enable(wearable_input, EINA_FALSE);
	}
	return wearable_input;
}

static const gchar *_static_ip_get_menu_text_by_menu_type(wifi_address_object *address,
							  static_ip_menu_type type)
{
	switch (type) {
	case STATIC_IP_ITEM_IP_ADDRESS:
		return wifi_address_get_ip_address(address);

	case STATIC_IP_ITEM_SUBNET_MASK:
		return wifi_address_get_subnet_mask(address);

	case STATIC_IP_ITEM_GATEWAY:
		return wifi_address_get_gateway_address(address);

	case STATIC_IP_ITEM_DNS1:
		return wifi_address_get_dns_address(address, 1);

	case STATIC_IP_ITEM_DNS2:
		return wifi_address_get_dns_address(address, 2);

	default:
		WIFI_LOG_ERR("Invalid menu type. type = %d", type);
		return NULL;
	}
}

static const gchar *_static_ip_get_menu_guide_text_by_menu_type(static_ip_menu_type type)
{
	switch (type) {
	case STATIC_IP_ITEM_IP_ADDRESS:
		return DEFAULT_GUIDE_IP_ADDRESS;

	case STATIC_IP_ITEM_SUBNET_MASK:
		return DEFAULT_GUIDE_SUBNET_MASK;

	case STATIC_IP_ITEM_GATEWAY:
		return DEFAULT_GUIDE_GATEWAY_ADDRESS;

	case STATIC_IP_ITEM_DNS1:
		return DEFAULT_GUIDE_DNS1;

	case STATIC_IP_ITEM_DNS2:
		return DEFAULT_GUIDE_DNS2;

	default:
		WIFI_LOG_ERR("Invalid menu type. type = %d", type);
		return "0.0.0.0";
	}
}

static void __static_ip_menu_tap_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;
	Elm_Object_Item *item = event_info;
	static_ip_menu_type menu_item_type;
	const gchar *ip_address_text;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_IF_FAIL(app_obj);
	WIFI_RET_IF_FAIL(item);
	elm_genlist_item_selected_set(item, EINA_FALSE);

	WIFI_RET_IF_FAIL(app_obj->wearable_input == NULL);

	menu_item_type = layout_static_ip_get_menu_type(app_obj->static_ip, item);
	if (menu_item_type <= STATIC_IP_ITEM_NONE ||
	    menu_item_type >= STATIC_IP_ITEM_SIZE) {
		WIFI_LOG_ERR("Invalid menu type. type = %d", menu_item_type);
		return;
	}

	ip_address_text = _static_ip_get_menu_text_by_menu_type(app_obj->address_for_edit,
								menu_item_type);
	if (ip_address_text != NULL) {
		app_obj->wearable_input = _static_ip_wearable_input_create(app_obj,
									   ip_address_text,
									   FALSE);
	} else {
		app_obj->wearable_input = _static_ip_wearable_input_create(app_obj,
									   _static_ip_get_menu_guide_text_by_menu_type(menu_item_type),
									   TRUE);
	}

	if (app_obj->wearable_input) {
		view_base_conformant_add_virtualkeypad_size_changed_cb(app_obj->base,
								       __virtualkeypad_size_changed_cb, app_obj);
		layout_static_ip_select_menu(app_obj->static_ip, menu_item_type);
		layout_static_ip_deactivate_rotary_event(app_obj->static_ip);
		layout_wearable_input_show(app_obj->wearable_input);
		layout_wearable_input_set_input_focus(app_obj->wearable_input, EINA_TRUE);
	}
}

static void __static_ip_save_button_tap_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_IF_FAIL(app_obj != NULL);

	wifi_address_set_static_ip(app_obj->address_for_connect, TRUE);
	wifi_address_set_ip_address(app_obj->address_for_connect,
				    wifi_address_get_ip_address(app_obj->address_for_edit));
	wifi_address_set_gateway_address(app_obj->address_for_connect,
					 wifi_address_get_gateway_address(app_obj->address_for_edit));
	wifi_address_set_subnet_mask(app_obj->address_for_connect,
				     wifi_address_get_subnet_mask(app_obj->address_for_edit));
	wifi_address_set_dns_address(app_obj->address_for_connect,
				     wifi_address_get_dns_address(app_obj->address_for_edit, 1), 1);
	wifi_address_set_dns_address(app_obj->address_for_connect,
				     wifi_address_get_dns_address(app_obj->address_for_edit, 2), 2);

	layout_ap_info_pop_to(app_obj->ap_info);
	layout_ap_info_menu_update(app_obj->ap_info, AP_INFO_MENU_STATIC);
}

static layout_static_ip_object *_static_ip_create(app_object *app_obj)
{
	layout_static_ip_object *static_ip = layout_static_ip_new(app_obj->base);
	if (!static_ip) {
		WIFI_LOG_ERR("layout_static_ip_new() is failed.");
		return NULL;
	}

	layout_static_ip_set_del_cb(static_ip,
				    __static_ip_del_cb, app_obj);

	layout_static_ip_set_menu_cb(static_ip, STATIC_IP_ITEM_TITLE,
				     __general_title_menu_item_display_ssid_text_get_cb, NULL, NULL, NULL, app_obj);

	layout_static_ip_set_menu_cb(static_ip, STATIC_IP_ITEM_IP_ADDRESS,
				     __static_ip_menu_ip_address_text_get_cb, NULL, NULL, NULL, app_obj);
	layout_static_ip_set_tap_menu_cb(static_ip, STATIC_IP_ITEM_IP_ADDRESS,
					 __static_ip_menu_tap_cb, app_obj);

	layout_static_ip_set_menu_cb(static_ip, STATIC_IP_ITEM_SUBNET_MASK,
				     __static_ip_menu_subnet_mask_text_get_cb, NULL, NULL, NULL, app_obj);
	layout_static_ip_set_tap_menu_cb(static_ip, STATIC_IP_ITEM_SUBNET_MASK,
					 __static_ip_menu_tap_cb, app_obj);

	layout_static_ip_set_menu_cb(static_ip, STATIC_IP_ITEM_GATEWAY,
				     __static_ip_menu_gateway_address_text_get_cb, NULL, NULL, NULL, app_obj);
	layout_static_ip_set_tap_menu_cb(static_ip, STATIC_IP_ITEM_GATEWAY,
					 __static_ip_menu_tap_cb, app_obj);

	layout_static_ip_set_menu_cb(static_ip, STATIC_IP_ITEM_DNS1,
				     __static_ip_menu_dns1_text_get_cb, NULL, NULL, NULL, app_obj);
	layout_static_ip_set_tap_menu_cb(static_ip, STATIC_IP_ITEM_DNS1,
					 __static_ip_menu_tap_cb, app_obj);

	layout_static_ip_set_menu_cb(static_ip, STATIC_IP_ITEM_DNS2,
				     __static_ip_menu_dns2_text_get_cb, NULL, NULL, NULL, app_obj);
	layout_static_ip_set_tap_menu_cb(static_ip, STATIC_IP_ITEM_DNS2,
					 __static_ip_menu_tap_cb, app_obj);

	layout_static_ip_set_tap_save_button_cb(static_ip,
						__static_ip_save_button_tap_cb, app_obj);

	if (!layout_static_ip_create(static_ip)) {
		WIFI_LOG_ERR("layout_static_ip_create() is failed.");
		layout_static_ip_free(static_ip);
		return NULL;
	}
	return static_ip;
}

static void _wifi_address_reset_static_ip_use_ap(wifi_address_object *address, wifi_ap_object *ap)
{
	gboolean is_static_ip;

	WIFI_RET_IF_FAIL(address != NULL);
	WIFI_RET_IF_FAIL(ap != NULL);

	is_static_ip = wifi_manager_ap_is_ip_config_static(ap);
	wifi_address_set_static_ip(address, is_static_ip);
	if (is_static_ip) {
		gchar *text = wifi_manager_ap_get_ip_address(ap);
		wifi_address_set_ip_address(address, text);
		g_free(text);
		text = wifi_manager_ap_get_gateway_address(ap);
		wifi_address_set_gateway_address(address, text);
		g_free(text);
		text = wifi_manager_ap_get_subnet_mask(ap);
		wifi_address_set_subnet_mask(address, text);
		g_free(text);
		text = wifi_manager_ap_get_dns_address(ap, 1);
		wifi_address_set_dns_address(address, text, 1);
		g_free(text);
		text = wifi_manager_ap_get_dns_address(ap, 2);
		wifi_address_set_dns_address(address, text, 2);
		g_free(text);
	}
}

static void _wifi_address_reset_proxy_use_ap(wifi_address_object *address, wifi_ap_object *ap)
{
	gchar *text = NULL;

	WIFI_RET_IF_FAIL(address != NULL);
	WIFI_RET_IF_FAIL(ap != NULL);

	wifi_address_set_proxy_manual(address,
				      wifi_manager_ap_is_proxy_manual(ap));
	text = wifi_manager_ap_get_proxy_address(ap);
	if (!text)
		text = g_strdup(DEFAULT_GUIDE_PROXY_ADDRESS);
	wifi_address_set_proxy_address(address, text);
	g_free(text);
	text = wifi_manager_ap_get_proxy_port(ap);
	if (!text)
		text = g_strdup(DEFAULT_GUIDE_PROXY_PORT);
	wifi_address_set_proxy_port(address, text);
	g_free(text);
}

static void _wifi_address_reset_use_ap(wifi_address_object *address, wifi_ap_object *ap)
{
	wifi_security_type_e sec_type = wifi_manager_ap_get_security_type(ap);
	wifi_address_set_security_type(address, sec_type);
	if (sec_type == WIFI_SECURITY_TYPE_EAP)
		wifi_address_set_eap_type(address, wifi_manager_ap_get_eap_type(ap));

	_wifi_address_reset_static_ip_use_ap(address, ap);
	_wifi_address_reset_proxy_use_ap(address, ap);
}

static void _wifi_address_for_connect_init(app_object *app_obj)
{
	WIFI_RET_IF_FAIL(app_obj);

	if (app_obj->address_for_connect)
		wifi_address_free(app_obj->address_for_connect);
	app_obj->address_for_connect = wifi_address_new();
}

static void _wifi_address_for_edit_init_use_address(app_object *app_obj, wifi_address_object *address)
{
	WIFI_RET_IF_FAIL(app_obj != NULL);

	if (app_obj->address_for_edit)
		wifi_address_free(app_obj->address_for_edit);
	app_obj->address_for_edit = wifi_address_clone(address);
}

static void __ap_info_menu_static_checkbox_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_IF_FAIL(app_obj != NULL);

	if (elm_check_state_get(obj)) {
		app_obj->static_ip = _static_ip_create(app_obj);
		if (app_obj->static_ip) {
			_wifi_address_for_edit_init_use_address(app_obj,
								app_obj->address_for_connect);
			if (!wifi_address_is_static_ip(app_obj->address_for_edit)) {
				layout_static_ip_save_button_set_disable(app_obj->static_ip);
			}
			layout_static_ip_show(app_obj->static_ip);
		}
	} else {
		_wifi_address_reset_static_ip_use_ap(app_obj->address_for_connect,
						     app_obj->selected_wifi_ap);
	}
}

static char *__ap_info_menu_static_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	gchar *tts_text;
	app_object *app_obj = data;
	WIFI_RET_VAL_IF_FAIL(app_obj != NULL, NULL);

	if (elm_check_state_get(app_obj->checkbox_static_ip)) {
		tts_text = g_strdup_printf("%s, %s",
					   STR_ON_FOR_TTS, STR_SWITCH_FOR_TTS);
	} else {
		tts_text = g_strdup_printf("%s, %s",
					   STR_OFF_FOR_TTS, STR_SWITCH_FOR_TTS);
	}
	layout_ap_info_menu_set_access_info(app_obj->ap_info, AP_INFO_MENU_STATIC,
					    ELM_ACCESS_CONTEXT_INFO, tts_text);
	g_free(tts_text);
	return g_strdup(STR_STATIC_IP);
}

static Evas_Object *__ap_info_menu_static_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
	app_object *app_obj = data;
	WIFI_RET_VAL_IF_FAIL(app_obj != NULL, NULL);

	if (!g_strcmp0(part, "elm.icon")) {
		Evas_Object *checkbox = elm_check_add(obj);
		elm_object_style_set(checkbox, "on&off/list");
		elm_check_state_set(checkbox,
				    wifi_address_is_static_ip(app_obj->address_for_connect));
		elm_access_object_unregister(checkbox);

		evas_object_propagate_events_set(checkbox, EINA_FALSE);
		evas_object_size_hint_weight_set(checkbox,
						 EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(checkbox,
						EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_smart_callback_add(checkbox, "changed",
					       __ap_info_menu_static_checkbox_changed_cb, app_obj);

		app_obj->checkbox_static_ip = checkbox;
		return checkbox;
	}
	return NULL;
}

static void __ap_info_menu_static_tap_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;
	Elm_Object_Item *item = event_info;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_IF_FAIL(app_obj != NULL);
	elm_genlist_item_selected_set(item, EINA_FALSE);

	app_obj->static_ip = _static_ip_create(app_obj);
	if (app_obj->static_ip) {
		_wifi_address_for_edit_init_use_address(app_obj,
							app_obj->address_for_connect);
		if (!wifi_address_is_static_ip(app_obj->address_for_edit)) {
			layout_static_ip_save_button_set_disable(app_obj->static_ip);
		}
		layout_static_ip_show(app_obj->static_ip);
	}
}

static void __proxy_setting_del_cb(void *data, Evas *e,
				   Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_IF_FAIL(app_obj != NULL);

	if (app_obj->address_for_edit) {
		wifi_address_free(app_obj->address_for_edit);
		app_obj->address_for_edit = NULL;
	}

	if (app_obj->proxy_setting) {
		layout_proxy_setting_destroy(app_obj->proxy_setting);
		layout_proxy_setting_free(app_obj->proxy_setting);
		app_obj->proxy_setting = NULL;
	}

	if (app_obj->ap_info) {
		layout_ap_info_activate_rotary_event(app_obj->ap_info);
	}
}

static char *__proxy_setting_menu_address_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	app_object *app_obj = data;
	WIFI_RET_VAL_IF_FAIL(app_obj != NULL, NULL);

	if (!g_strcmp0(part, "elm.text")) {
		return g_strdup(STR_PROXY_ADDRESS);
	} else if (!g_strcmp0(part, "elm.text.1")) {
		return g_strdup(wifi_address_get_proxy_address(app_obj->address_for_edit));
	}
	return NULL;
}

static char *__proxy_setting_menu_port_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	app_object *app_obj = data;
	WIFI_RET_VAL_IF_FAIL(app_obj != NULL, NULL);

	if (!g_strcmp0(part, "elm.text")) {
		return g_strdup(STR_PROXY_PORT);
	} else if (!g_strcmp0(part, "elm.text.1")) {
		return g_strdup(wifi_address_get_proxy_port(app_obj->address_for_edit));
	}
	return NULL;
}

static gboolean __is_valid_proxy_setting_string_length(const gchar *text)
{
	if (!text) {
		WIFI_LOG_ERR("proxy text is NULL");
		return FALSE;
	} else if (strlen(text) == 0) {
		WIFI_LOG_ERR("proxy text string length = 0");
		return FALSE;
	}
	return TRUE;
}

static void __proxy_setting_wearable_input_changed_cb(void *data, Evas_Object *obj,
						      void *event_info)
{
	app_object *app_obj = data;
	gchar *proxy_text;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	proxy_text = elm_entry_markup_to_utf8(elm_entry_entry_get(obj));
	layout_wearable_input_set_input_return_key_enable(app_obj->wearable_input,
							  __is_valid_proxy_setting_string_length(proxy_text));
	g_free(proxy_text);
}

static void __proxy_setting_wearable_input_activated_cb(void *data, Evas_Object *obj,
							void *event_info)
{
	app_object *app_obj = data;
	proxy_setting_menu_type selected_menu;
	gchar *proxy_text = NULL;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	proxy_text = elm_entry_markup_to_utf8(elm_entry_entry_get(obj));
	if (!__is_valid_proxy_setting_string_length(proxy_text)) {
		WIFI_LOG_ERR("Invalid proxy text.");
		g_free(proxy_text);
		return;
	}

	selected_menu = layout_proxy_setting_get_selected_menu(app_obj->proxy_setting);
	switch (selected_menu) {
	case PROXY_SETTING_ITEM_ADDRESS:
		wifi_address_set_proxy_address(app_obj->address_for_edit, proxy_text);
		break;

	case PROXY_SETTING_ITEM_PORT:
		wifi_address_set_proxy_port(app_obj->address_for_edit, proxy_text);
		break;

	default:
		WIFI_LOG_ERR("Invalid selected menu type = %d", selected_menu);
		g_free(proxy_text);
		layout_proxy_setting_pop_to(app_obj->proxy_setting);
		return;
	}
	layout_proxy_setting_update_menu(app_obj->proxy_setting, selected_menu);
	g_free(proxy_text);

	layout_proxy_setting_pop_to(app_obj->proxy_setting);
}

static layout_wearable_input_object *_proxy_setting_wearable_input_create(app_object *app_obj,
								   Elm_Input_Panel_Layout input_type,
								   const gchar *text, gboolean is_guide_text)
{
	layout_wearable_input_object *wearable_input = layout_wearable_input_new(app_obj->base);
	if (!wearable_input) {
		WIFI_LOG_ERR("layout_wearable_input_new() is failed.");
		return NULL;
	}

	layout_wearable_input_set_del_cb(wearable_input,
					 __wearable_input_del_cb, app_obj);

	layout_wearable_input_set_input_type(wearable_input, input_type);
	if (is_guide_text) {
		layout_wearable_input_set_input_guide_text(wearable_input, text);
	} else {
		layout_wearable_input_set_input_text(wearable_input, text);
	}
	layout_wearable_input_set_input_show(wearable_input, EINA_TRUE);

	layout_wearable_input_set_input_changed_cb(wearable_input,
						   __proxy_setting_wearable_input_changed_cb, app_obj);
	layout_wearable_input_set_input_activated_cb(wearable_input,
						     __proxy_setting_wearable_input_activated_cb, app_obj);

	if (!layout_wearable_input_create(wearable_input)) {
		WIFI_LOG_ERR("layout_wearable_input_create() is failed.");
		layout_wearable_input_free(wearable_input);
		return NULL;
	}

	if (is_guide_text || !text || strlen(text) == 0) {
		layout_wearable_input_set_input_return_key_enable(wearable_input, EINA_FALSE);
	}
	return wearable_input;
}

static void __proxy_setting_menu_tap_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;
	Elm_Object_Item *item = event_info;
	proxy_setting_menu_type menu_item_type;
	Elm_Input_Panel_Layout input_type = ELM_INPUT_PANEL_LAYOUT_PASSWORD;
	const gchar *proxy_text;
	gboolean is_guide_text;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_IF_FAIL(app_obj);
	WIFI_RET_IF_FAIL(item);
	elm_genlist_item_selected_set(item, EINA_FALSE);

	WIFI_RET_IF_FAIL(app_obj->wearable_input == NULL);

	proxy_text = elm_object_item_part_text_get(item, "elm.text.1");
	menu_item_type = layout_proxy_setting_get_menu_type(app_obj->proxy_setting, item);
	switch (menu_item_type) {
	case PROXY_SETTING_ITEM_ADDRESS:
		input_type = ELM_INPUT_PANEL_LAYOUT_NORMAL;
		is_guide_text = _is_default_proxy_address(proxy_text);
		break;

	case PROXY_SETTING_ITEM_PORT:
		input_type = ELM_INPUT_PANEL_LAYOUT_NUMBER;
		is_guide_text = _is_default_proxy_port(proxy_text);
		break;

	default:
		WIFI_LOG_ERR("Invalid menu type. type = %d", menu_item_type);
		return;
	}

	app_obj->wearable_input = _proxy_setting_wearable_input_create(app_obj,
								       input_type, proxy_text, is_guide_text);
	if (app_obj->wearable_input) {
		view_base_conformant_add_virtualkeypad_size_changed_cb(app_obj->base,
								       __virtualkeypad_size_changed_cb, app_obj);
		layout_proxy_setting_select_menu(app_obj->proxy_setting, menu_item_type);
		layout_proxy_setting_deactivate_rotary_event(app_obj->proxy_setting);
		layout_wearable_input_show(app_obj->wearable_input);
		layout_wearable_input_set_input_focus(app_obj->wearable_input, EINA_TRUE);
	}
}

static void __proxy_setting_save_button_tap_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;
	Elm_Object_Item *item = event_info;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_IF_FAIL(app_obj != NULL);
	elm_genlist_item_selected_set(item, EINA_FALSE);

	wifi_address_set_proxy_manual(app_obj->address_for_connect, TRUE);
	wifi_address_set_proxy_address(app_obj->address_for_connect,
				       layout_proxy_setting_get_main_text(app_obj->proxy_setting, PROXY_SETTING_ITEM_ADDRESS));
	wifi_address_set_proxy_port(app_obj->address_for_connect,
				    layout_proxy_setting_get_main_text(app_obj->proxy_setting, PROXY_SETTING_ITEM_PORT));

	layout_ap_info_pop_to(app_obj->ap_info);
	layout_ap_info_menu_update(app_obj->ap_info, AP_INFO_MENU_PROXY);
}

static layout_proxy_setting_object *_proxy_setting_create(app_object *app_obj)
{
	layout_proxy_setting_object *proxy_setting = layout_proxy_setting_new(app_obj->base);
	if (!proxy_setting) {
		WIFI_LOG_ERR("layout_proxy_setting_new() is failed.");
		return NULL;
	}

	layout_proxy_setting_set_del_cb(proxy_setting,
					__proxy_setting_del_cb, app_obj);

	layout_proxy_setting_set_menu_cb(proxy_setting, PROXY_SETTING_ITEM_TITLE,
					 __general_title_menu_item_display_ssid_text_get_cb, NULL, NULL, NULL, app_obj);

	layout_proxy_setting_set_menu_cb(proxy_setting, PROXY_SETTING_ITEM_ADDRESS,
					 __proxy_setting_menu_address_text_get_cb, NULL, NULL, NULL, app_obj);
	layout_proxy_setting_set_tap_menu_cb(proxy_setting, PROXY_SETTING_ITEM_ADDRESS,
					     __proxy_setting_menu_tap_cb, app_obj);

	layout_proxy_setting_set_menu_cb(proxy_setting, PROXY_SETTING_ITEM_PORT,
					 __proxy_setting_menu_port_text_get_cb, NULL, NULL, NULL, app_obj);
	layout_proxy_setting_set_tap_menu_cb(proxy_setting, PROXY_SETTING_ITEM_PORT,
					     __proxy_setting_menu_tap_cb, app_obj);

	layout_proxy_setting_set_tap_save_button_cb(proxy_setting,
						    __proxy_setting_save_button_tap_cb, app_obj);

	if (!layout_proxy_setting_create(proxy_setting)) {
		WIFI_LOG_ERR("layout_proxy_setting_create() is failed.");
		layout_proxy_setting_free(proxy_setting);
		return NULL;
	}
	return proxy_setting;
}

static void __ap_info_menu_proxy_checkbox_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_IF_FAIL(app_obj != NULL);

	if (elm_check_state_get(obj)) {
		app_obj->proxy_setting = _proxy_setting_create(app_obj);
		if (app_obj->proxy_setting) {
			_wifi_address_for_edit_init_use_address(app_obj,
								app_obj->address_for_connect);
			layout_proxy_setting_show(app_obj->proxy_setting);
		}
	} else {
		_wifi_address_reset_proxy_use_ap(app_obj->address_for_connect,
						 app_obj->selected_wifi_ap);
	}
}

static char *__ap_info_menu_proxy_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	gchar *tts_text;
	app_object *app_obj = data;
	WIFI_RET_VAL_IF_FAIL(app_obj != NULL, NULL);

	if (elm_check_state_get(app_obj->checkbox_proxy)) {
		tts_text = g_strdup_printf("%s, %s",
					   STR_ON_FOR_TTS, STR_SWITCH_FOR_TTS);
	} else {
		tts_text = g_strdup_printf("%s, %s",
					   STR_OFF_FOR_TTS, STR_SWITCH_FOR_TTS);
	}
	layout_ap_info_menu_set_access_info(app_obj->ap_info, AP_INFO_MENU_PROXY,
					    ELM_ACCESS_CONTEXT_INFO, tts_text);
	g_free(tts_text);
	return g_strdup(STR_PROXY_SETTINGS);
}

static Evas_Object *__ap_info_menu_proxy_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
	app_object *app_obj = data;
	WIFI_RET_VAL_IF_FAIL(app_obj != NULL, NULL);

	if (!g_strcmp0(part, "elm.icon")) {
		Evas_Object *checkbox = elm_check_add(obj);
		elm_object_style_set(checkbox, "on&off/list");
		elm_check_state_set(checkbox,
				    wifi_address_is_proxy_manual(app_obj->address_for_connect));
		elm_access_object_unregister(checkbox);

		evas_object_propagate_events_set(checkbox, EINA_FALSE);
		evas_object_size_hint_weight_set(checkbox,
						 EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(checkbox,
						EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_smart_callback_add(checkbox, "changed",
					       __ap_info_menu_proxy_checkbox_changed_cb, app_obj);

		app_obj->checkbox_proxy = checkbox;
		return checkbox;
	}
	return NULL;
}

static void __ap_info_menu_proxy_tap_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;
	Elm_Object_Item *item = event_info;

	__WIFI_FUNC_ENTER__;

	if (!app_obj) {
		WIFI_LOG_ERR("app object is NULL");
		return;
	}
	elm_genlist_item_selected_set(item, EINA_FALSE);

	app_obj->proxy_setting = _proxy_setting_create(app_obj);
	if (app_obj->proxy_setting) {
		_wifi_address_for_edit_init_use_address(app_obj,
							app_obj->address_for_connect);
		layout_proxy_setting_show(app_obj->proxy_setting);
	}
}

static void __on_scan_pop_transition_finished_for_connect(void *data,
							  Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	view_base_naviframe_del_transition_finished_cb(app_obj->base,
						       __on_scan_pop_transition_finished_for_connect);

	if (app_obj->scan) {
		if (app_obj->connecting_wifi_ap) {
			_wifi_reserve_connect_to_selected_ap(app_obj,
							     __wifi_disconnected_for_connect_cb, app_obj);
		} else {
			_wifi_connect_to_selected_ap(app_obj);
		}
	} else {
		WIFI_LOG_ERR("AP list closed.");
	}
}

static void __ap_info_tap_connect_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	view_base_naviframe_add_transition_finished_cb(app_obj->base,
						       __on_scan_pop_transition_finished_for_connect, app_obj);
	layout_scan_pop_to(app_obj->scan);
}

static layout_ap_info_object *_ap_info_create(app_object *app_obj, wifi_ap_object *ap)
{
	wifi_security_type_e security_type = wifi_manager_ap_get_security_type(ap);
	layout_ap_info_object *ap_info_obj = layout_ap_info_new(app_obj->base);
	if (!ap_info_obj) {
		WIFI_LOG_ERR("layout_ap_info_new() is failed.");
		return NULL;
	}

	layout_ap_info_set_del_cb(ap_info_obj, __ap_info_del_cb, app_obj);
	layout_ap_info_set_menu_cb(ap_info_obj, AP_INFO_MENU_TITLE,
				   __general_title_menu_item_display_ssid_text_get_cb, NULL, NULL, NULL, NULL, NULL);
	if (security_type == WIFI_SECURITY_TYPE_NONE) {
		WIFI_LOG_INFO("AP is open");
	} else if (security_type == WIFI_SECURITY_TYPE_EAP) {
		WIFI_LOG_INFO("AP is eap");
		layout_ap_info_set_menu_cb(ap_info_obj, AP_INFO_MENU_EAP,
					   __ap_info_menu_eap_text_get_cb, NULL, NULL, NULL,
					   __ap_info_menu_eap_tap_cb, app_obj);
	} else if (wifi_manager_ap_is_wps_mode(ap)) {
		WIFI_LOG_INFO("AP is wps");
		layout_ap_info_set_menu_cb(ap_info_obj, AP_INFO_MENU_PASSWORD,
					   NULL, NULL, NULL, NULL,
					   __ap_info_menu_password_tap_cb, app_obj);
		layout_ap_info_set_menu_cb(ap_info_obj, AP_INFO_MENU_WPS,
					   NULL, NULL, NULL, NULL,
					   __ap_info_menu_wps_tap_cb, app_obj);
	} else {
		WIFI_LOG_INFO("AP is security");
		layout_ap_info_set_menu_cb(ap_info_obj, AP_INFO_MENU_PASSWORD,
					   NULL, NULL, NULL, NULL,
					   __ap_info_menu_password_tap_cb, app_obj);
	}
	layout_ap_info_set_menu_cb(ap_info_obj, AP_INFO_MENU_STATIC,
				   __ap_info_menu_static_text_get_cb,
				   __ap_info_menu_static_content_get_cb,
				   NULL, NULL,
				   __ap_info_menu_static_tap_cb, app_obj);
	layout_ap_info_set_menu_cb(ap_info_obj, AP_INFO_MENU_PROXY,
				   __ap_info_menu_proxy_text_get_cb,
				   __ap_info_menu_proxy_content_get_cb,
				   NULL, NULL,
				   __ap_info_menu_proxy_tap_cb, app_obj);

	/* Add empty item to avoid overlap */
	layout_ap_info_set_menu_cb(ap_info_obj, AP_INFO_MENU_EMPTY,
				   NULL, NULL, NULL, NULL, NULL, NULL);

	layout_ap_info_set_tap_connect_button_cb(ap_info_obj,
						 __ap_info_tap_connect_button_cb, app_obj);

	if (!layout_ap_info_create(ap_info_obj)) {
		WIFI_LOG_ERR("layout_ap_info_create() is failed.");
		layout_ap_info_free(ap_info_obj);
		return NULL;
	}

	return ap_info_obj;
}

static void _ap_info_show(app_object *app_obj, wifi_ap_object *ap)
{
	wifi_security_type_e security_type = wifi_manager_ap_get_security_type(ap);
	if (security_type == WIFI_SECURITY_TYPE_NONE) {
		layout_ap_info_open_show(app_obj->ap_info, app_obj);
	} else if (security_type == WIFI_SECURITY_TYPE_EAP) {
		layout_ap_info_eap_show(app_obj->ap_info, app_obj);
	} else if (wifi_manager_ap_is_wps_mode(ap)) {
		layout_ap_info_set_connect_button_enable(app_obj->ap_info, EINA_FALSE);
		layout_ap_info_wps_show(app_obj->ap_info, app_obj);
	} else {
		layout_ap_info_set_connect_button_enable(app_obj->ap_info, EINA_FALSE);
		layout_ap_info_security_show(app_obj->ap_info, app_obj);
	}
}

static void __scan_menu_ap_item_tap_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = (app_object *)data;
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	wifi_ap_object *ap = NULL;
	wifi_connection_state_e connection_state = WIFI_CONNECTION_STATE_FAILURE;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);
	elm_genlist_item_selected_set(item, EINA_FALSE);

	ap = elm_object_item_data_get(item);
	wifi_manager_ap_refresh(ap);
	connection_state = wifi_manager_ap_get_connection_state(ap);
	switch (connection_state) {
	case WIFI_CONNECTION_STATE_CONNECTED:
		WIFI_LOG_INFO("Connected AP");
		app_obj->detail = _detail_create(app_obj, ap);
		if (app_obj->detail) {
			app_obj->selected_wifi_ap = wifi_manager_ap_clone(ap);
			layout_detail_show(app_obj->detail);
		}
		break;

	case WIFI_CONNECTION_STATE_DISCONNECTED:
	case WIFI_CONNECTION_STATE_FAILURE:
		if (wifi_manager_ap_has_configuration(app_obj->wifi_manager, ap)) {
			WIFI_LOG_INFO("Not connected AP (Connected before)");
			_popup_change_ap_show_use_ap(app_obj, ap);
		} else {
			WIFI_LOG_INFO("Not connected AP (Not connected before)");
			app_obj->ap_info = _ap_info_create(app_obj, ap);
			if (app_obj->ap_info) {
				app_obj->selected_wifi_ap = wifi_manager_ap_clone(ap);
				_wifi_address_for_connect_init(app_obj);
				_wifi_address_reset_use_ap(app_obj->address_for_connect,
							   app_obj->selected_wifi_ap);
				_ap_info_show(app_obj, app_obj->selected_wifi_ap);
			}
		}
		break;

	case WIFI_CONNECTION_STATE_ASSOCIATION:
		WIFI_LOG_INFO("AP connection state is association.");
		break;

	case WIFI_CONNECTION_STATE_CONFIGURATION:
		WIFI_LOG_INFO("AP connection state is configuration.");
		break;

	default:
		WIFI_LOG_ERR("AP connection state is unknown.");
		break;
	}
}

static void __popup_scanning_destroy_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_IF_FAIL(app_obj != NULL);

	if (app_obj->popup_scanning) {
		popup_scanning_destroy(app_obj->popup_scanning);
		popup_scanning_free(app_obj->popup_scanning);
		app_obj->popup_scanning = NULL;
	}

	if (app_obj->scan) {
		layout_scan_ap_list_activate_rotary_event(app_obj->scan);
	} else if (app_obj->main) {
		layout_main_activate_rotary_event(app_obj->main);
	}
}

static void _popup_scanning_show(app_object *app_obj, gboolean is_scanning_for_wifi_activate)
{
	if (!app_obj->popup_scanning) {
		app_obj->popup_scanning = popup_scanning_new(app_obj->base);
		WIFI_RET_IF_FAIL(app_obj->popup_scanning != NULL);

		popup_scanning_set_destroy_cb(app_obj->popup_scanning,
					      __popup_scanning_destroy_cb, app_obj);

		if (!popup_scanning_create(app_obj->popup_scanning)) {
			WIFI_LOG_ERR("popup_scanning_create() is failed.");
			popup_scanning_free(app_obj->popup_scanning);
			app_obj->popup_scanning = NULL;
			return;
		}
	}
	app_obj->is_scanning_for_wifi_activate = is_scanning_for_wifi_activate;
	popup_scanning_show(app_obj->popup_scanning);
}

static void __scan_button_tap_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = (app_object *)data;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	if (app_obj->popup_scanning != NULL) {
		WIFI_LOG_ERR("Scanning now in progress...");
		return;
	}

	if (_wifi_manager_scan_start_by_scan_button(app_obj)) {
		_popup_scanning_show(app_obj, FALSE);
	}
}

static void __scan_ap_data_del_cb(gpointer data)
{
	wifi_ap_object *ap = data;
	WIFI_RET_IF_FAIL(ap);

	wifi_manager_ap_destroy(ap);
}

static void _scan_callbacks_init(layout_scan_object *scan_obj, app_object *app_obj)
{
	layout_scan_set_del_cb(scan_obj, __scan_del_cb, app_obj);
	layout_scan_set_ap_data_del_cb(scan_obj, __scan_ap_data_del_cb);
	layout_scan_set_menu_cb(scan_obj, SCAN_MENU_WIFI_AP_ITEM,
				__scan_menu_ap_item_text_get_cb, __scan_menu_ap_item_content_get_cb,
				NULL, NULL,
				__scan_menu_ap_item_tap_cb, app_obj);
	layout_scan_set_scan_button_tap_cb(scan_obj, __scan_button_tap_cb, app_obj);
}

static layout_scan_object *_scan_create(view_base_object *base_obj, app_object *app_obj)
{
	layout_scan_object *scan_obj = NULL;

	__WIFI_FUNC_ENTER__;

	scan_obj = layout_scan_new(app_obj->base);
	if (!scan_obj)
		return NULL;
	_scan_callbacks_init(scan_obj, app_obj);
	if (!layout_scan_create(scan_obj)) {
		layout_scan_free(scan_obj);
		return NULL;
	}
	return scan_obj;
}

static void __main_del_cb(void *data, Evas *e,
			  Evas_Object *obj, void *event_info)
{
	__WIFI_FUNC_ENTER__;
	ui_app_exit();
}

static char *__main_menu_power_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	gchar *tts_text;
	app_object *app_obj = data;
	WIFI_RET_VAL_IF_FAIL(app_obj != NULL, NULL);

	if (elm_check_state_get(app_obj->checkbox_power)) {
		tts_text = g_strdup_printf("%s, %s",
					   STR_ON_FOR_TTS, STR_SWITCH_FOR_TTS);
	} else {
		tts_text = g_strdup_printf("%s, %s",
					   STR_OFF_FOR_TTS, STR_SWITCH_FOR_TTS);
	}
	layout_main_menu_set_access_info(app_obj->main, MAIN_MENU_POWER,
					 ELM_ACCESS_CONTEXT_INFO, tts_text);
	g_free(tts_text);
	return g_strdup(STR_WIFI);
}

static gboolean __is_wifi_power_already_changed(wifi_manager_object *manager,
						gboolean is_on)
{
	gboolean is_wifi_use, is_wearable_debuging_mode;
	bool is_wifi_activate = FALSE;
	wifi_error_e error;

	is_wifi_use = wifi_manager_is_wifi_use(manager);
	is_wearable_debuging_mode = vconf_is_wearable_debugging_mode();
	error = wifi_manager_is_activated(manager, &is_wifi_activate);
	if (error != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_manager_is_activated() is failed. error = %s",
			     wifi_error_to_string(error));
	}

	WIFI_LOG_INFO("[Wi-Fi] debuging mode = %s",
		      is_wearable_debuging_mode ? "Y" : "N");
	WIFI_LOG_INFO("[Wi-Fi] use vconf     = %s",
		      is_wifi_use ? "Y" : "N");
	WIFI_LOG_INFO("[Wi-Fi] activate      = %s",
		      is_wifi_activate ? "Y" : "N");
    WIFI_LOG_INFO("[Wi-Fi] is_on      = %s",
		      is_on ? "Y" : "N");

	if (is_on) {
		return is_wifi_use && is_wifi_activate;
	} else {
		return !is_wifi_use && !is_wifi_activate;
	}
}

static gboolean __check_wifi_already_power_on_for_main_menu_enable(gpointer user_data)
{
	app_object *app_obj = user_data;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_VAL_IF_FAIL(app_obj != NULL, FALSE);
	WIFI_RET_VAL_IF_FAIL(app_obj->wifi_manager != NULL, FALSE);
	WIFI_RET_VAL_IF_FAIL(app_obj->main != NULL, FALSE);

	if (__is_wifi_power_already_changed(app_obj->wifi_manager, TRUE)) {
		WIFI_LOG_INFO("Wi-Fi is already power on.");
		layout_main_menu_set_enable(app_obj->main, MAIN_MENU_POWER, EINA_TRUE);
		if (_is_unable_to_scan_state(app_obj->wifi_manager)) {
			WIFI_LOG_INFO("Wi-Fi is unable to scan.");
		} else {
			WIFI_LOG_INFO("Wi-Fi is able to scan.");
			layout_main_menu_set_enable(app_obj->main, MAIN_MENU_SCAN, EINA_TRUE);
		}
	}
	return FALSE;
}

static gboolean __check_wifi_already_power_off_for_main_menu_enable(gpointer user_data)
{
	app_object *app_obj = user_data;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_VAL_IF_FAIL(app_obj != NULL, FALSE);
	WIFI_RET_VAL_IF_FAIL(app_obj->wifi_manager != NULL, FALSE);
	WIFI_RET_VAL_IF_FAIL(app_obj->main != NULL, FALSE);

	if (__is_wifi_power_already_changed(app_obj->wifi_manager, FALSE)) {
		WIFI_LOG_INFO("Wi-Fi is already power off.");
		layout_main_menu_set_enable(app_obj->main, MAIN_MENU_POWER, EINA_TRUE);
	}

    __WIFI_FUNC_EXIT__;
	return FALSE;
}

static void _main_menu_disable_for_wifi_power_on(app_object *app_obj)
{
	layout_main_menu_set_enable(app_obj->main, MAIN_MENU_POWER, EINA_FALSE);
	layout_main_menu_set_enable(app_obj->main, MAIN_MENU_SCAN, EINA_FALSE);
	g_timeout_add(TIMEOUT_FOR_CHECK_WIFI_POWER_CHANGED,
		      __check_wifi_already_power_on_for_main_menu_enable, app_obj);

	if (_is_unable_to_scan_state(app_obj->wifi_manager)) {
		app_obj->popup_unable_scan = _popup_unable_scan_create(app_obj);
        WIFI_LOG_INFO("Create popup");
		if (app_obj->popup_unable_scan) {
			popup_unable_scan_show(app_obj->popup_unable_scan);
            WIFI_LOG_INFO("popup unable scan show");
		}
	}
}

static void _main_menu_disable_for_wifi_power_off(app_object *app_obj)
{
	layout_main_menu_set_enable(app_obj->main, MAIN_MENU_POWER, EINA_FALSE);
	layout_main_menu_set_enable(app_obj->main, MAIN_MENU_SCAN, EINA_FALSE);
	g_timeout_add(TIMEOUT_FOR_CHECK_WIFI_POWER_CHANGED,
		      __check_wifi_already_power_off_for_main_menu_enable, app_obj);
}

#if 0
static void __main_menu_power_checkbox_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = data;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	layout_main_menu_show(app_obj->main, MAIN_MENU_POWER);

	app_obj->is_main_power_check_clicked = TRUE;
	if (elm_check_state_get(obj)) {
            WIFI_LOG_INFO("menu disable for wifi_power_on");
			_main_menu_disable_for_wifi_power_on(app_obj);
			wifi_manager_set_wifi_use(app_obj->wifi_manager, TRUE);
	} else {
	    WIFI_LOG_INFO("menu disable for wifi_power_off");
		_main_menu_disable_for_wifi_power_off(app_obj);
		wifi_manager_set_wifi_use(app_obj->wifi_manager, FALSE);
	}
}
#endif

static Evas_Object *__main_menu_power_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
	app_object *app_obj = data;
	WIFI_RET_VAL_IF_FAIL(app_obj != NULL, NULL);

	if (!g_strcmp0(part, "elm.icon")) {
		Evas_Object *checkbox = elm_check_add(obj);
		elm_object_style_set(checkbox, "on&off/list");
		elm_check_state_set(checkbox, wifi_manager_is_wifi_use(app_obj->wifi_manager));
		elm_access_object_unregister(checkbox);

#if 0
		evas_object_smart_callback_add(checkbox, "changed",
					       __main_menu_power_checkbox_changed_cb, app_obj);
#endif

		app_obj->checkbox_power = checkbox;
		return checkbox;
	}
	return NULL;
}

static void __main_menu_power_tap_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	app_object *app_obj = data;
    int err = 0;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_IF_FAIL(app_obj != NULL);
	elm_genlist_item_selected_set(item, EINA_FALSE);

	if (app_obj->is_main_power_check_clicked) {
		WIFI_LOG_INFO("Power checkbox clicked.");
		app_obj->is_main_power_check_clicked = FALSE;
		return;
	}

	if (wifi_manager_is_wifi_use(app_obj->wifi_manager)) {
        WIFI_LOG_INFO("disable for wifi_power_off");
		_main_menu_disable_for_wifi_power_off(app_obj);
		wifi_manager_set_wifi_use(app_obj->wifi_manager, FALSE);
         WIFI_LOG_INFO("After deactive setting");

         err = wifi_manager_deactivate(app_obj->wifi_manager, NULL, NULL);
        if (err != WIFI_ERROR_NONE) {
            WIFI_LOG_ERR("wifi_manager_deactivate() is failed. error = %s",
                wifi_error_to_string(err));
			return;
		}

	} else {
	        WIFI_LOG_INFO("disable_for_wifi_power_on");
			_main_menu_disable_for_wifi_power_on(app_obj);
			wifi_manager_set_wifi_use(app_obj->wifi_manager, TRUE);

            WIFI_LOG_INFO("After active setting");
            /* Seonah Moon: For activating wifi */
            err = wifi_manager_activate(app_obj->wifi_manager, NULL, NULL);
			if (err != WIFI_ERROR_NONE) {
				WIFI_LOG_ERR("wifi_manager_activate() is failed. error = %s",
					     wifi_error_to_string(err));
				return;
			}
	}
}

static char *__main_menu_scan_get_connected_state_text(wifi_manager_object *wifi_manager)
{
	wifi_ap_object *connected_ap = NULL;
	gchar *ssid = NULL, *ssid_full_text = NULL;

	wifi_manager_get_connected_ap(wifi_manager, &connected_ap);
	if (!connected_ap)
		return g_strdup(STR_NOT_CONNECTED);

	ssid = wifi_manager_ap_get_ssid(connected_ap);
	wifi_manager_ap_destroy(connected_ap);
	if (!ssid)
		return g_strdup(STR_NOT_CONNECTED);

	ssid_full_text = elm_entry_utf8_to_markup(ssid);
	g_free(ssid);

	return ssid_full_text;
}

static char *__main_menu_scan_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	app_object *app_obj = data;
	wifi_error_e err = WIFI_ERROR_INVALID_PARAMETER;
	wifi_connection_state_e connection_state = WIFI_CONNECTION_STATE_FAILURE;

	if (!app_obj) {
		WIFI_LOG_ERR("app object is NULL");
		return NULL;
	}

	if (!g_strcmp0(part, "elm.text"))
		return g_strdup(STR_WIFI_NETWORKS);

	if (!g_strcmp0(part, "elm.text.1")) {
		if (!wifi_manager_is_wifi_use(app_obj->wifi_manager))
			return g_strdup(STR_TURNED_OFF);

		err = wifi_manager_get_connection_state(
			app_obj->wifi_manager, &connection_state);
		if (err != WIFI_ERROR_NONE) {
			WIFI_LOG_ERR("wifi_manager_get_connection_state() is failed. error = %s",
				     wifi_error_to_string(err));
		}

		switch (connection_state) {
		case WIFI_CONNECTION_STATE_FAILURE:
		case WIFI_CONNECTION_STATE_DISCONNECTED:
			return g_strdup(STR_NOT_CONNECTED);

		case WIFI_CONNECTION_STATE_ASSOCIATION:
			return g_strdup(STR_CONNECTING);

		case WIFI_CONNECTION_STATE_CONFIGURATION:
			return g_strdup(STR_OBTAINING_IP);

		case WIFI_CONNECTION_STATE_CONNECTED:
			return __main_menu_scan_get_connected_state_text(app_obj->wifi_manager);

		default:
			return g_strdup(STR_NOT_CONNECTED);
		}
	}
	return NULL;
}

static void __main_menu_scan_tap_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_object *app_obj = (app_object *)data;
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	bool is_activated = false;
	wifi_error_e err = WIFI_ERROR_NONE;

	__WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(app_obj != NULL);

	elm_genlist_item_selected_set(item, EINA_FALSE);

	err = wifi_manager_is_activated(app_obj->wifi_manager, &is_activated);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_manager_is_activated() is failed. error = %s",
			     wifi_error_to_string(err));
		return;
	}

	if (is_activated) {
		_scan_ap_list_update_and_show(app_obj);
	} else {
		WIFI_LOG_INFO("wifi is deactivated");
		if (wifi_manager_is_wifi_use(app_obj->wifi_manager)) {
			err = wifi_manager_activate(app_obj->wifi_manager, NULL, NULL);
			if (err != WIFI_ERROR_NONE) {
				WIFI_LOG_ERR("wifi_manager_activate() is failed. error = %s",
					     wifi_error_to_string(err));
				return;
			}
			_popup_scanning_show(app_obj, TRUE);
			if (!app_obj->popup_scanning) {
				WIFI_LOG_ERR("Scanning popup create failed.");
			}
		} else {
			WIFI_LOG_ERR("wifi use vconf disabled.");
		}
	}
	// Wi-Fi networks menu(main_scan_menu) is dimmed when Wi-Fi Off
}

static void _main_callbacks_init(layout_main_object *main_obj,
				 app_object *app_obj)
{
	layout_main_set_del_cb(main_obj, __main_del_cb, app_obj);

	layout_main_set_menu_cb(main_obj, MAIN_MENU_POWER,
				__main_menu_power_text_get_cb, __main_menu_power_content_get_cb,
				__main_menu_power_tap_cb, app_obj);
	layout_main_set_menu_cb(main_obj, MAIN_MENU_SCAN,
				__main_menu_scan_text_get_cb, NULL,
				__main_menu_scan_tap_cb, app_obj);
	layout_main_set_menu_cb(main_obj, MAIN_MENU_EMPTY, NULL, NULL, NULL, NULL);
}

static layout_main_object *_main_create(view_base_object *base_obj,
					app_object *app_obj)
{
	layout_main_object *main_obj = NULL;

	__WIFI_FUNC_ENTER__;

	main_obj = layout_main_new(base_obj);
	if (!main_obj) {
		WIFI_LOG_ERR("layout_main_new() is failed.");
		return NULL;
	}

	app_obj->is_main_power_check_clicked = FALSE;
	_main_callbacks_init(main_obj, app_obj);
	if (!layout_main_create(main_obj)) {
		WIFI_LOG_ERR("layout_main_create() is failed.");
		layout_main_free(main_obj);
		return NULL;
	}
	return main_obj;
}

static gboolean __app_control_transient_app(app_control_h service, app_object *app_obj)
{
	int ret = app_control_clone(&(app_obj->app_control), service);
	if (ret != APP_CONTROL_ERROR_NONE) {
		WIFI_LOG_ERR("app_control_clone() is failed. error = %d", ret);
		return FALSE;
	}
	return TRUE;
}

static void _app_service_handle_launch_requested(app_control_h service,
						 app_object *app_obj, gboolean is_scanlist_requested)
{
	if (_app_is_view_initialized(app_obj)) {
		WIFI_LOG_INFO("app view is initialized.");
		return;
	}

	if (!_app_view_base_init(app_obj)) {
		WIFI_LOG_ERR("_app_view_base_init() is failed.");
	}

	if (!_app_view_base_callback_init_for_keygrab(app_obj)) {
		WIFI_LOG_ERR("_app_view_base_callback_init_for_keygrab() is failed");
	}

	if (is_scanlist_requested) {
		_popup_scanning_show(app_obj, FALSE);
		if (!app_obj->popup_scanning) {
			WIFI_LOG_ERR("_popup_scanning_show() is failed.");
		}
	} else {
		if (!_app_main_init(app_obj)) {
			WIFI_LOG_ERR("_app_main_init() is failed.");
		}
	}

	if (!_app_network_callbacks_init_for_view(app_obj)) {
		WIFI_LOG_ERR("_app_network_callbacks_for_view_init() is failed.");
	}

	if (is_scanlist_requested) {
		if (!_wifi_manager_scan_start_for_wifi_activated(app_obj)) {
			WIFI_LOG_ERR("_wifi_manager_scan_start_for_wifi_activated() is failed.");
		}
	} else {
		bool activated = false;
		wifi_error_e err = wifi_manager_is_activated(app_obj->wifi_manager,
							     &activated);
		if (err != WIFI_ERROR_NONE) {
			WIFI_LOG_ERR("wifi_manager_is_activated() is failed. error = %s",
				     wifi_error_to_string(err));
		}
		if (activated) {
			if (!_wifi_manager_scan_start(app_obj)) {
				WIFI_LOG_ERR("_wifi_manager_scan_start() is failed.");
			}
		}

		// Wi-Fi app launch request from Setting app.
		if (!__app_control_transient_app(service, app_obj)) {
			WIFI_LOG_ERR("__app_control_transient_app() is failed.");
		}
	}
}

static void _app_release(app_object *app_obj)
{
	__WIFI_FUNC_ENTER__;

	WIFI_RET_IF_FAIL(app_obj);

	idler_util_managed_idle_cleanup();

	_wifi_selected_ap_destroy(app_obj);
	if (app_obj->wifi_manager) {
		wifi_manager_free_with_deinit(app_obj->wifi_manager);
		app_obj->wifi_manager = NULL;
	}

	if (app_obj->address_for_connect) {
		wifi_address_free(app_obj->address_for_connect);
		app_obj->address_for_connect = NULL;
	}
	if (app_obj->address_for_edit) {
		wifi_address_free(app_obj->address_for_edit);
		app_obj->address_for_edit = NULL;
	}

	if (app_obj->wps_pin_string) {
		g_free(app_obj->wps_pin_string);
		app_obj->wps_pin_string = NULL;
	}

	if (app_obj->popup_unable_scan) {
		popup_unable_scan_destroy(app_obj->popup_unable_scan);
		popup_unable_scan_free(app_obj->popup_unable_scan);
		app_obj->popup_unable_scan = NULL;
	}
	if (app_obj->popup_scanning) {
		popup_scanning_destroy(app_obj->popup_scanning);
		popup_scanning_free(app_obj->popup_scanning);
		app_obj->popup_scanning = NULL;
	}
	if (app_obj->popup_change_ap) {
		popup_change_ap_destroy(app_obj->popup_change_ap);
		popup_change_ap_free(app_obj->popup_change_ap);
		app_obj->popup_change_ap = NULL;
	}
	if (app_obj->detail) {
		layout_detail_destroy(app_obj->detail);
		layout_detail_free(app_obj->detail);
		app_obj->detail = NULL;
	}
	if (app_obj->wps_progress) {
		layout_wps_progress_destroy(app_obj->wps_progress);
		layout_wps_progress_free(app_obj->wps_progress);
		app_obj->wps_progress = NULL;
	}
	if (app_obj->wps_method) {
		layout_wps_method_destroy(app_obj->wps_method);
		layout_wps_method_free(app_obj->wps_method);
		app_obj->wps_method = NULL;
	}
	if (app_obj->eap_method) {
		layout_eap_method_destroy(app_obj->eap_method);
		layout_eap_method_free(app_obj->eap_method);
		app_obj->eap_method = NULL;
	}
	if (app_obj->proxy_setting) {
		layout_proxy_setting_destroy(app_obj->proxy_setting);
		layout_proxy_setting_free(app_obj->proxy_setting);
		app_obj->proxy_setting = NULL;
	}
	if (app_obj->static_ip) {
		layout_static_ip_destroy(app_obj->static_ip);
		layout_static_ip_free(app_obj->static_ip);
		app_obj->static_ip = NULL;
	}
	if (app_obj->password_entry) {
		layout_password_entry_destroy(app_obj->password_entry);
		layout_password_entry_free(app_obj->password_entry);
		app_obj->password_entry = NULL;
	}
	if (app_obj->wearable_input) {
		layout_wearable_input_destroy(app_obj->wearable_input);
		layout_wearable_input_free(app_obj->wearable_input);
		app_obj->wearable_input = NULL;
	}
	if (app_obj->ap_info) {
		layout_ap_info_destroy(app_obj->ap_info);
		layout_ap_info_free(app_obj->ap_info);
		app_obj->ap_info = NULL;
	}
	if (app_obj->scan) {
		layout_scan_destroy(app_obj->scan);
		layout_scan_free(app_obj->scan);
		app_obj->scan = NULL;
	}
	if (app_obj->main) {
		layout_main_destroy(app_obj->main);
		layout_main_free(app_obj->main);
		app_obj->main = NULL;
	}
	if (app_obj->base) {
		view_base_destroy(app_obj->base);
		view_base_free(app_obj->base);
		app_obj->base = NULL;
	}

	if (app_obj->app_control) {
		app_control_h appctrl;

		app_control_create(&appctrl);

		app_control_reply_to_launch_request(appctrl,
						    app_obj->app_control, APP_CONTROL_ERROR_NONE);

		app_control_destroy(appctrl);
		app_control_destroy(app_obj->app_control);
	}
}

static bool app_create(void *user_data)
{
	app_object *app_obj = user_data;

	__WIFI_FUNC_ENTER__;

	if (!app_obj) {
		WIFI_LOG_ERR("app object is NULL");
		return false;
	}

	if (!_app_network_init(app_obj)) {
		WIFI_LOG_ERR("_app_network_init() is failed.");
		return false;
	}

	__WIFI_FUNC_EXIT__;
	return true;
}

static void app_pause(void *user_data)
{
	app_object *app_obj = user_data;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_IF_FAIL(app_obj != NULL);
	WIFI_RET_IF_FAIL(app_obj->wifi_manager != NULL);

}

static void app_resume(void *user_data)
{
	app_object *app_obj = user_data;

	__WIFI_FUNC_ENTER__;

	WIFI_RET_IF_FAIL(app_obj != NULL);
	WIFI_RET_IF_FAIL(app_obj->wifi_manager != NULL);
}

static void app_service(app_control_h service, void *user_data)
{
	app_object *app_obj = user_data;
	char *extradata = NULL;

	__WIFI_FUNC_ENTER__;

	if (app_control_get_extra_data(service, "viewtype", &extradata) == APP_CONTROL_ERROR_NONE) {
		gboolean is_scanlist_requested = !g_strcmp0(extradata, "scanlist");

		if (is_scanlist_requested) {
			free(extradata);
			_app_service_handle_launch_requested(service, app_obj, is_scanlist_requested);
		} else {
			if (extradata) {
				WIFI_LOG_ERR("Not supported view type [%s]", extradata);
				free(extradata);
			} else {
				WIFI_LOG_ERR("Not supported view type ['NULL']");
			}
		}
	} else if (app_control_get_extra_data(service, "disconnect", &extradata) == APP_CONTROL_ERROR_NONE) {
		if (!g_strcmp0(extradata, "captiveportal")) {
			char *ssid = NULL, *security_type = NULL;
			free(extradata);

			if (!app_obj->scan) {
				WIFI_LOG_ERR("scan view not shown.");
				return;
			}
			if (app_control_get_extra_data(service, "ssid", &ssid) != APP_CONTROL_ERROR_NONE ||
			    ssid == NULL) {
				WIFI_LOG_ERR("get [ssid] extra data failed.");
				return;
			}
			if (app_control_get_extra_data(service, "security_type", &security_type) != APP_CONTROL_ERROR_NONE ||
			    security_type == NULL) {
				WIFI_LOG_ERR("get [security_type] extra data failed.");
				free(ssid);
				return;
			}
			// TODO create dummy ap by ssid / security type
			if (app_obj->scan) {
				Elm_Object_Item *found_ap_item;
				_wifi_ap_property prop;

				prop.ssid = ssid;
				if (!g_strcmp0(security_type, "none")) {
					prop.security_type = WIFI_SECURITY_TYPE_NONE;
				} else if (!g_strcmp0(security_type, "wep")) {
					prop.security_type = WIFI_SECURITY_TYPE_WEP;
				} else if (!g_strcmp0(security_type, "psk")) {
					prop.security_type = WIFI_SECURITY_TYPE_WPA_PSK;
				} else if (!g_strcmp0(security_type, "ieee8021x")) {
					prop.security_type = WIFI_SECURITY_TYPE_EAP;
				} else {
					WIFI_LOG_ERR("Invalid wifi security type [%s]", security_type);
					prop.security_type = WIFI_SECURITY_TYPE_NONE;
				}

				found_ap_item = layout_scan_ap_list_find_item_by_data(
					app_obj->scan, &prop, _compare_wifi_ap_object_properties);
				if (found_ap_item) {
					wifi_ap_object *found_ap = elm_object_item_data_get(found_ap_item);
					wifi_manager_ap_set_captiveportal(found_ap, TRUE);

					elm_genlist_item_fields_update(found_ap_item, "elm.text.1",
								       ELM_GENLIST_ITEM_FIELD_TEXT);
				}
			}
			free(ssid);
			free(security_type);
		} else {
			if (extradata) {
				WIFI_LOG_ERR("Not supported disconnect extra data [%s]", extradata);
				free(extradata);
			} else {
				WIFI_LOG_ERR("Not supported disconnect extra data ['NULL']");
			}
		}
	} else {
		_app_service_handle_launch_requested(service, app_obj, FALSE);
	}
}

static void app_terminate(void *user_data)
{
	app_object *app_obj = user_data;

	__WIFI_FUNC_ENTER__;

	_app_release(app_obj);
}

VISIBILITY_DEFAULT int main(int argc, char *argv[])
{
	app_object app_obj;
	ui_app_lifecycle_callback_s event_callback = {0, };
	int ret;

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_service;

	memset(&app_obj, 0x0, sizeof(app_object));

	ret = ui_app_main(argc, argv, &event_callback, &app_obj);
	if (APP_ERROR_NONE != ret) {
		WIFI_LOG_ERR("app_main() is failed %d", ret);
	}

	return ret;
}



#if 0 /* Not used */
VISIBILITY_DEFAULT int main(int argc, char *argv[])
{
	app_object app_obj;
	app_event_callback_s event_callback;

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_service;

	event_callback.low_memory = NULL;
	event_callback.low_battery = NULL;
	event_callback.device_orientation = NULL;
	event_callback.language_changed = app_language_changed;
	event_callback.region_format_changed = NULL;

	memset(&app_obj, 0x0, sizeof(app_object));

	int ret = app_efl_main(&argc, &argv, &event_callback, &app_obj);
	if (ret != APP_ERROR_NONE) {
		WIFI_LOG_ERR("app_efl_main() is failed. err = %s",
			     _app_error_to_string(ret));
	}

	return ret;
}
#endif
