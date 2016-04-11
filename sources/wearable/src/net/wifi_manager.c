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


#include <dlog.h>
#include <glib.h>

#include "util.h"
#include "net/util/vconf_helper.h"
#include "net/wifi_manager.h"

typedef enum {
	WIFI_MANAGER_CB_DEVICE_STATE_CHANGED,
	WIFI_MANAGER_CB_CONNECTION_STATE_CHANGED,
	WIFI_MANAGER_CB_BACKGROUND_SCAN
} wifi_manager_cb_type;

struct _wifi_manager_object {
	GList *cb_data_list;
	GList *config_list;
};

struct _wifi_ap_object {
	wifi_ap_h ap;
	gint rssi;
	bool wps_mode;

	wifi_error_e last_connection_error;
	gboolean is_captiveportal;
};

typedef struct _wifi_config_object {
	wifi_config_h handle;
	gchar *name;
	gchar *passphrase;
	wifi_security_type_e security_type;
	wifi_error_e last_error;
} wifi_config_object;

typedef struct {
	wifi_manager_object *manager;
	wifi_ap_object *ap;
	wifi_manager_cb_type type;
	wifi_manager_generic_cb cb;
	wifi_manager_generic_state_changed_cb state_changed_cb;
	gpointer user_data;
} wifi_manager_cb_data;

static wifi_manager_object *_last_initailized_wifi_manager;

static void            _update_wifi_config_list(wifi_manager_object *manager);
static wifi_ap_object *_create_ap_object_by_ap_h(wifi_ap_h ap);

static void __wifi_common_cb(wifi_error_e error_code, gpointer user_data)
{
	wifi_manager_cb_data *cb_data = (wifi_manager_cb_data *)user_data;
	WIFI_RET_IF_FAIL(cb_data);

    WIFI_LOG_INFO("common cb");

	if (cb_data->cb)
		cb_data->cb(cb_data->manager, cb_data->ap, error_code, cb_data->user_data);

	g_free(cb_data);
}

static void __wifi_background_scan_cb(wifi_error_e error_code, gpointer user_data)
{
	wifi_manager_cb_data *cb_data = (wifi_manager_cb_data *)user_data;
	WIFI_RET_IF_FAIL(cb_data != NULL);

	if (cb_data->cb)
		cb_data->cb(cb_data->manager, cb_data->ap, error_code, cb_data->user_data);
}

static void __wifi_device_state_changed_cb(wifi_device_state_e state, void *user_data)
{
	wifi_manager_cb_data *cb_data = (wifi_manager_cb_data *)user_data;
	WIFI_RET_IF_FAIL(cb_data);

	if (state == WIFI_DEVICE_STATE_ACTIVATED) {
		_update_wifi_config_list(cb_data->manager);
	}

	if (cb_data->state_changed_cb)
		cb_data->state_changed_cb(cb_data->manager,
					  state,
					  WIFI_CONNECTION_STATE_FAILURE, NULL,
					  WIFI_RSSI_LEVEL_0, cb_data->user_data);
}

static void __wifi_connection_state_changed_cb(wifi_connection_state_e state,
					       wifi_ap_h ap, void *user_data)
{
	wifi_manager_cb_data *cb_data = (wifi_manager_cb_data *)user_data;
	WIFI_RET_IF_FAIL(cb_data);

	if (state == WIFI_CONNECTION_STATE_CONNECTED ||
	    state == WIFI_CONNECTION_STATE_DISCONNECTED ||
	    state == WIFI_CONNECTION_STATE_FAILURE) {
		_update_wifi_config_list(cb_data->manager);
	}

	if (cb_data->state_changed_cb) {
		wifi_ap_object *ap_obj = _create_ap_object_by_ap_h(ap);
		cb_data->state_changed_cb(cb_data->manager,
					  WIFI_DEVICE_STATE_DEACTIVATED,
					  state, ap_obj,
					  WIFI_RSSI_LEVEL_0, cb_data->user_data);
		wifi_manager_ap_destroy(ap_obj);
	}
}

bool __wifi_config_list_cb(const wifi_config_h config, void *user_data)
{
    
    __WIFI_FUNC_ENTER__;
	wifi_manager_object *manager = user_data;
	wifi_config_object *config_obj;
	WIFI_RET_VAL_IF_FAIL(config != NULL, true);
	WIFI_RET_VAL_IF_FAIL(manager != NULL, true);

	config_obj = g_new0(wifi_config_object, 1);
	WIFI_RET_VAL_IF_FAIL(config_obj != NULL, true);

	wifi_config_clone(config, &(config_obj->handle));
	wifi_config_get_name(config, &(config_obj->name));
	wifi_config_get_security_type(config, &(config_obj->security_type));
	manager->config_list = g_list_append(manager->config_list, config_obj);

    __WIFI_FUNC_EXIT__;
	return true;
}

static void _wifi_config_destory(gpointer data)
{
	wifi_config_object *config = data;
	if (config->handle) {
		wifi_config_destroy(config->handle);
	}
	if (config->name) {
		g_free(config->name);
	}
	if (config->passphrase) {
		g_free(config->passphrase);
	}
	g_free(config);
}

static void _clear_wifi_config_list(wifi_manager_object *manager)
{
    
    __WIFI_FUNC_ENTER__;
	WIFI_RET_IF_FAIL(manager != NULL);
	WIFI_RET_IF_FAIL(manager->config_list != NULL);

	g_list_free_full(manager->config_list, _wifi_config_destory);
	manager->config_list = NULL;
    
    __WIFI_FUNC_EXIT__;
}

static void _update_wifi_config_list(wifi_manager_object *manager)
{
	wifi_error_e err;
	WIFI_RET_IF_FAIL(manager);

	__WIFI_FUNC_ENTER__;

	if (manager->config_list) {
		_clear_wifi_config_list(manager);
	}

	err = wifi_config_foreach_configuration(__wifi_config_list_cb, manager);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_config_foreach_configuration() is failed. error = %s",
			     wifi_error_to_string(err));
	}

    __WIFI_FUNC_EXIT__;
}

static wifi_config_object *_find_wifi_config_by_wifi_ap(wifi_manager_object *manager,
							wifi_ap_object *ap)
{
	GList *config_list;
	gchar *ssid;
	wifi_security_type_e sec_type;
	WIFI_RET_VAL_IF_FAIL(manager, NULL);
	WIFI_RET_VAL_IF_FAIL(ap, NULL);

	ssid = wifi_manager_ap_get_ssid(ap);
	WIFI_RET_VAL_IF_FAIL(ssid, NULL);
	sec_type = wifi_manager_ap_get_security_type(ap);

	config_list = manager->config_list;
	while (config_list) {
		wifi_config_object *config = config_list->data;
		if (!g_strcmp0(config->name, ssid) && (config->security_type == sec_type)) {
			g_free(ssid);
			return config;
		}
		config_list = config_list->next;
	}
	g_free(ssid);
	return NULL;
}

static wifi_ap_object *_create_ap_object_by_ap_h(wifi_ap_h ap)
{
	wifi_ap_object *ap_obj = g_new0(wifi_ap_object, 1);
	gboolean is_create_failed = FALSE;
	if (!ap_obj) {
		return NULL;
	}
	if (WIFI_ERROR_NONE != wifi_ap_clone(&(ap_obj->ap), ap)) {
		is_create_failed = TRUE;
	} else if (WIFI_ERROR_NONE != wifi_ap_get_rssi(ap, &(ap_obj->rssi))) {
		is_create_failed = TRUE;
	} else if (WIFI_ERROR_NONE != wifi_ap_is_wps_supported(ap, &(ap_obj->wps_mode))) {
		is_create_failed = TRUE;
	}

	if (is_create_failed) {
		wifi_manager_ap_destroy(ap_obj);
		return NULL;
	}
	ap_obj->last_connection_error = WIFI_ERROR_NONE;
	ap_obj->is_captiveportal = FALSE;
	return ap_obj;
}

gint wifi_manager_default_compare_ap(gconstpointer a, gconstpointer b)
{
	wifi_ap_object *ap_obj1 = (wifi_ap_object *)a;
	wifi_ap_object *ap_obj2 = (wifi_ap_object *)b;
	wifi_connection_state_e state1 = WIFI_CONNECTION_STATE_DISCONNECTED;
	wifi_connection_state_e state2 = WIFI_CONNECTION_STATE_DISCONNECTED;
	bool favorite1 = false;
	bool favorite2 = false;

	wifi_ap_get_connection_state(ap_obj1->ap, &state1);
	wifi_ap_get_connection_state(ap_obj2->ap, &state2);
	if (state1 != state2) {
		if (state1 == WIFI_CONNECTION_STATE_CONNECTED)
			return -1;
		if (state2 == WIFI_CONNECTION_STATE_CONNECTED)
			return 1;

		if (state1 == WIFI_CONNECTION_STATE_CONFIGURATION)
			return -1;
		if (state2 == WIFI_CONNECTION_STATE_CONFIGURATION)
			return 1;

		if (state1 == WIFI_CONNECTION_STATE_ASSOCIATION)
			return -1;
		if (state2 == WIFI_CONNECTION_STATE_ASSOCIATION)
			return 1;
	}

	favorite1 = wifi_manager_ap_is_favorite(
		_last_initailized_wifi_manager, ap_obj1);
	favorite2 = wifi_manager_ap_is_favorite(
		_last_initailized_wifi_manager, ap_obj2);
	if (favorite1 != favorite2) {
		if (favorite1)
			return -1;
		if (favorite2)
			return 1;
	}

	/* Alphabetical order */
	/*
	return strcasecmp((const char *) wifi_device1->ssid,
	                (const char *) wifi_device2->ssid);
	*/
	/* RSSI preferred */
	return((ap_obj1->rssi >= ap_obj2->rssi) ? -1 : 1);
}

#ifdef TIZEN_TELEPHONY_ENABLE
static bool __update_ap_list_foreach(wifi_ap_h ap, void *user_data)
{
	GList **ap_list = user_data;
	wifi_ap_object *ap_obj = NULL;
	WIFI_RET_VAL_IF_FAIL(ap_list, true);

	ap_obj = _create_ap_object_by_ap_h(ap);
	WIFI_RET_VAL_IF_FAIL(ap_obj, true);

	*ap_list = g_list_insert_sorted(*ap_list, ap_obj, wifi_manager_default_compare_ap);
	return true;
}
#else
// Find AP list except EAP type when WC1-S Bluetooth model(No SIM).
static bool __update_ap_list_foreach(wifi_ap_h ap, void *user_data)
{
	GList **ap_list = user_data;
	wifi_security_type_e sec_type = WIFI_SECURITY_TYPE_NONE;
	wifi_error_e err;
	wifi_ap_object *ap_obj = NULL;
	WIFI_RET_VAL_IF_FAIL(ap_list != NULL, true);

	err = wifi_ap_get_security_type(ap, &sec_type);
	WIFI_RET_VAL_IF_FAIL(err == WIFI_ERROR_NONE, true);
	if (sec_type == WIFI_SECURITY_TYPE_EAP) {
		WIFI_LOG_INFO("EAP type AP skip(No SIM model).");
		return true;
	}

	ap_obj = _create_ap_object_by_ap_h(ap);
	WIFI_RET_VAL_IF_FAIL(ap_obj != NULL, true);

	*ap_list = g_list_insert_sorted(*ap_list, ap_obj, wifi_manager_default_compare_ap);
	return true;
}
#endif

static void _delete_cb_data_by_type(wifi_manager_object *manager, wifi_manager_cb_type type)
{
	GList *cb_data_list = manager->cb_data_list;
	while (cb_data_list) {
		GList *next = cb_data_list->next;
		wifi_manager_cb_data *cb_data = cb_data_list->data;
		if (cb_data->type == type) {
			manager->cb_data_list =
				g_list_delete_link(manager->cb_data_list, cb_data_list);
			g_free(cb_data);
		}
		cb_data_list = next;
	}
}

static gchar *_ap_get_proxy_address(wifi_ap_object *ap_obj)
{
	gchar *proxy_address = NULL;
	wifi_error_e err;
	WIFI_RET_VAL_IF_FAIL(ap_obj != NULL, NULL);

	err = wifi_ap_get_proxy_address(ap_obj->ap,
					WIFI_ADDRESS_FAMILY_IPV4, &proxy_address);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_ap_get_proxy_address() is failed. error = %s",
			     wifi_error_to_string(err));
		if (proxy_address) {
			g_free(proxy_address);
		}
		return NULL;
	}
	if (!proxy_address || strlen(proxy_address) == 0) {
		WIFI_LOG_INFO("proxy_address is NULL.");
		if (proxy_address) {
			g_free(proxy_address);
		}
		return NULL;
	}
	return proxy_address;
}

/*
 * Wi-Fi Manager API
 */
wifi_manager_object *wifi_manager_new()
{
	return g_new0(wifi_manager_object, 1);
}

void wifi_manager_free(wifi_manager_object *manager)
{
	if (manager)
		g_free(manager);
}

wifi_error_e wifi_manager_init(wifi_manager_object *manager)
{
	wifi_error_e err;
	bool is_activate;
	WIFI_RET_VAL_IF_FAIL(manager, WIFI_ERROR_INVALID_PARAMETER);

	err = wifi_initialize();
	if (err != WIFI_ERROR_NONE) {
		return err;
	}
	err = wifi_manager_is_activated(manager, &is_activate);
	if (err == WIFI_ERROR_NONE) {
		if (is_activate) {
			_update_wifi_config_list(manager);
		}
	} else {
		WIFI_LOG_ERR("wifi_manager_is_activated() is failed. error = %s",
			     wifi_error_to_string(err));
	}
	_last_initailized_wifi_manager = manager;
	return err;
}

wifi_error_e wifi_manager_deinit(wifi_manager_object *manager)
{
	WIFI_RET_VAL_IF_FAIL(manager, WIFI_ERROR_INVALID_PARAMETER);

	_clear_wifi_config_list(manager);
	if (manager->cb_data_list) {
		GList *cb_data_list = manager->cb_data_list;
		while (cb_data_list) {
			wifi_manager_cb_data *cb_data = cb_data_list->data;
			if (cb_data->type == WIFI_MANAGER_CB_DEVICE_STATE_CHANGED) {
				wifi_unset_device_state_changed_cb();
			} else if (cb_data->type == WIFI_MANAGER_CB_CONNECTION_STATE_CHANGED) {
				wifi_unset_connection_state_changed_cb();
			} else if (cb_data->type == WIFI_MANAGER_CB_BACKGROUND_SCAN) {
				wifi_unset_background_scan_cb();
			}

			cb_data_list = cb_data_list->next;
		}
		g_list_free_full(manager->cb_data_list, g_free);
	}
	return wifi_deinitialize();
}

wifi_manager_object *wifi_manager_new_with_init(wifi_error_e *error)
{
	wifi_manager_object *manager = wifi_manager_new();
	*error = WIFI_ERROR_NONE;
	if (manager) {
		*error = wifi_manager_init(manager);
		if (*error != WIFI_ERROR_NONE) {
			wifi_manager_free(manager);
		}
	} else
		*error = WIFI_ERROR_OUT_OF_MEMORY;
	return manager;
}

wifi_error_e wifi_manager_free_with_deinit(wifi_manager_object *manager)
{
	wifi_error_e error = wifi_manager_deinit(manager);
	wifi_manager_free(manager);
	return error;
}

wifi_error_e wifi_manager_get_csccode(wifi_manager_object *manager, wifi_manager_csccode *code)
{
	WIFI_RET_VAL_IF_FAIL(manager != NULL, WIFI_ERROR_INVALID_PARAMETER);
	WIFI_RET_VAL_IF_FAIL(code != NULL, WIFI_ERROR_INVALID_PARAMETER);

	*code = WIFI_MANAGER_CSC_UNKNOWN;
#ifdef TIZEN_TELEPHONY_ENABLE
	if (vconf_is_skt()) {
		*code = WIFI_MANAGER_CSC_SKT;
	}
#endif
	return WIFI_ERROR_NONE;
}

wifi_error_e wifi_manager_scan(wifi_manager_object *manager,
			       wifi_manager_generic_cb callback, gpointer user_data)
{
	wifi_manager_cb_data *cb_data = NULL;
	WIFI_RET_VAL_IF_FAIL(manager != NULL, WIFI_ERROR_INVALID_PARAMETER);

	cb_data = g_new0(wifi_manager_cb_data, 1);
	if (cb_data) {
		cb_data->manager = manager;
		cb_data->cb = callback;
		cb_data->user_data = user_data;

		WIFI_LOG_INFO("Wi-Fi Scan start.");
		return wifi_scan(__wifi_common_cb, cb_data);
	}
	return WIFI_ERROR_OUT_OF_MEMORY;
}

wifi_error_e wifi_manager_get_found_ap_list(wifi_manager_object *manager,
					    GList **ap_list)
{
	wifi_error_e error = WIFI_ERROR_INVALID_PARAMETER;
	WIFI_RET_VAL_IF_FAIL(manager, WIFI_ERROR_INVALID_PARAMETER);

	error = wifi_foreach_found_aps(__update_ap_list_foreach, ap_list);
	if (error != WIFI_ERROR_NONE) {
		if (*ap_list) {
			g_list_free(*ap_list);
			*ap_list = NULL;
		}
	}
	return error;
}

wifi_error_e wifi_manager_activate(wifi_manager_object *manager,
				   wifi_manager_generic_cb callback, gpointer user_data)
{
	wifi_manager_cb_data *cb_data = NULL;
	WIFI_RET_VAL_IF_FAIL(manager, WIFI_ERROR_INVALID_PARAMETER);

    WIFI_LOG_ERR("+");

	cb_data = g_new0(wifi_manager_cb_data, 1);
	if (cb_data) {
		cb_data->manager = manager;
		cb_data->cb = callback;
		cb_data->user_data = user_data;
		return wifi_activate(__wifi_common_cb, cb_data);
	}
	return WIFI_ERROR_OUT_OF_MEMORY;
}

wifi_error_e wifi_manager_deactivate(wifi_manager_object *manager,
                    wifi_manager_generic_cb callback, gpointer user_data)
{
   	wifi_manager_cb_data *cb_data = NULL;
	WIFI_RET_VAL_IF_FAIL(manager, WIFI_ERROR_INVALID_PARAMETER);

    WIFI_LOG_ERR("+");

	cb_data = g_new0(wifi_manager_cb_data, 1);
	if (cb_data) {
		cb_data->manager = manager;
		cb_data->cb = callback;
		cb_data->user_data = user_data;
		return wifi_deactivate(__wifi_common_cb, cb_data);
	}
	return WIFI_ERROR_OUT_OF_MEMORY;
}

wifi_error_e wifi_manager_is_activated(wifi_manager_object *manager, bool *activated)
{
	WIFI_RET_VAL_IF_FAIL(manager, WIFI_ERROR_INVALID_PARAMETER);
	return wifi_is_activated(activated);
}

wifi_error_e wifi_manager_set_device_state_changed_cb(wifi_manager_object *manager,
						      wifi_manager_generic_state_changed_cb callback, void *user_data)
{
	wifi_manager_cb_data *cb_data = NULL;
	wifi_error_e err = WIFI_ERROR_INVALID_PARAMETER;
	WIFI_RET_VAL_IF_FAIL(manager, WIFI_ERROR_INVALID_PARAMETER);

	cb_data = g_new0(wifi_manager_cb_data, 1);
	WIFI_RET_VAL_IF_FAIL(cb_data, WIFI_ERROR_OUT_OF_MEMORY);

	cb_data->manager = manager;
	cb_data->type = WIFI_MANAGER_CB_DEVICE_STATE_CHANGED;
	cb_data->state_changed_cb = callback;
	cb_data->user_data = user_data;
	err = wifi_set_device_state_changed_cb(
		__wifi_device_state_changed_cb, cb_data);
	if (err == WIFI_ERROR_NONE)
		manager->cb_data_list = g_list_append(manager->cb_data_list, cb_data);
	return err;
}

wifi_error_e wifi_manager_unset_device_state_changed_cb(wifi_manager_object *manager)
{
	wifi_error_e err = WIFI_ERROR_INVALID_PARAMETER;
	WIFI_RET_VAL_IF_FAIL(manager, WIFI_ERROR_INVALID_PARAMETER);

	err = wifi_unset_device_state_changed_cb();
	_delete_cb_data_by_type(manager, WIFI_MANAGER_CB_DEVICE_STATE_CHANGED);
	return err;
}

wifi_error_e wifi_manager_set_background_scan_cb(wifi_manager_object *manager,
						 wifi_manager_generic_cb callback, void *user_data)
{
	wifi_manager_cb_data *cb_data = NULL;
	wifi_error_e err = WIFI_ERROR_INVALID_PARAMETER;
	WIFI_RET_VAL_IF_FAIL(manager != NULL, WIFI_ERROR_INVALID_PARAMETER);

	cb_data = g_new0(wifi_manager_cb_data, 1);
	WIFI_RET_VAL_IF_FAIL(cb_data != NULL, WIFI_ERROR_OUT_OF_MEMORY);

	cb_data->manager = manager;
	cb_data->type = WIFI_MANAGER_CB_BACKGROUND_SCAN;
	cb_data->cb = callback;
	cb_data->user_data = user_data;
	err = wifi_set_background_scan_cb(__wifi_background_scan_cb, cb_data);
	if (err == WIFI_ERROR_NONE)
		manager->cb_data_list = g_list_append(manager->cb_data_list, cb_data);
	return err;
}

wifi_error_e wifi_manager_unset_background_scan_cb(wifi_manager_object *manager)
{
	wifi_error_e err = WIFI_ERROR_INVALID_PARAMETER;
	WIFI_RET_VAL_IF_FAIL(manager, WIFI_ERROR_INVALID_PARAMETER);

	err = wifi_unset_background_scan_cb();
	_delete_cb_data_by_type(manager, WIFI_MANAGER_CB_BACKGROUND_SCAN);
	return err;
}

wifi_error_e wifi_manager_set_connection_state_changed_cb(wifi_manager_object *manager,
							  wifi_manager_generic_state_changed_cb callback, void *user_data)
{
	wifi_manager_cb_data *cb_data = NULL;
	wifi_error_e err = WIFI_ERROR_INVALID_PARAMETER;
	WIFI_RET_VAL_IF_FAIL(manager, WIFI_ERROR_INVALID_PARAMETER);

	cb_data = g_new0(wifi_manager_cb_data, 1);
	WIFI_RET_VAL_IF_FAIL(cb_data, WIFI_ERROR_OUT_OF_MEMORY);

	cb_data->manager = manager;
	cb_data->type = WIFI_MANAGER_CB_CONNECTION_STATE_CHANGED;
	cb_data->state_changed_cb = callback;
	cb_data->user_data = user_data;
	err = wifi_set_connection_state_changed_cb(
		__wifi_connection_state_changed_cb, cb_data);
	if (err == WIFI_ERROR_NONE)
		manager->cb_data_list = g_list_append(manager->cb_data_list, cb_data);
	return err;
}

wifi_error_e wifi_manager_unset_connection_state_changed_cb(wifi_manager_object *manager)
{
	wifi_error_e err = WIFI_ERROR_INVALID_PARAMETER;
	WIFI_RET_VAL_IF_FAIL(manager, WIFI_ERROR_INVALID_PARAMETER);

	err = wifi_unset_connection_state_changed_cb();
	_delete_cb_data_by_type(manager, WIFI_MANAGER_CB_CONNECTION_STATE_CHANGED);
	return err;
}

wifi_error_e wifi_manager_get_connection_state(wifi_manager_object *manager,
					       wifi_connection_state_e *connection_state)
{
	WIFI_RET_VAL_IF_FAIL(manager, WIFI_ERROR_INVALID_PARAMETER);
	return wifi_get_connection_state(connection_state);
}

wifi_error_e wifi_manager_connect(wifi_manager_object *manager,
				  wifi_ap_object *ap, wifi_manager_generic_cb callback, gpointer user_data)
{
	wifi_manager_cb_data *cb_data = NULL;
	WIFI_RET_VAL_IF_FAIL(manager, WIFI_ERROR_INVALID_PARAMETER);
	WIFI_RET_VAL_IF_FAIL(ap, WIFI_ERROR_INVALID_PARAMETER);

	cb_data = g_new0(wifi_manager_cb_data, 1);
	if (cb_data) {
		cb_data->manager = manager;
		cb_data->ap = ap;
		cb_data->cb = callback;
		cb_data->user_data = user_data;
		return wifi_connect(ap->ap, __wifi_common_cb, cb_data);
	}
	return WIFI_ERROR_OUT_OF_MEMORY;
}

wifi_error_e wifi_manager_connect_by_wps_pbc(wifi_manager_object *manager,
					     wifi_ap_object *ap, wifi_manager_generic_cb callback, gpointer user_data)
{
	wifi_manager_cb_data *cb_data = NULL;
	WIFI_RET_VAL_IF_FAIL(manager, WIFI_ERROR_INVALID_PARAMETER);
	WIFI_RET_VAL_IF_FAIL(ap, WIFI_ERROR_INVALID_PARAMETER);

	cb_data = g_new0(wifi_manager_cb_data, 1);
	if (cb_data) {
		cb_data->manager = manager;
		cb_data->ap = ap;
		cb_data->cb = callback;
		cb_data->user_data = user_data;
		return wifi_connect_by_wps_pbc(ap->ap, __wifi_common_cb, cb_data);
	}
	return WIFI_ERROR_OUT_OF_MEMORY;
}

wifi_error_e wifi_manager_connect_by_wps_pin(wifi_manager_object *manager,
					     wifi_ap_object *ap, const gchar *pin,
					     wifi_manager_generic_cb callback, gpointer user_data)
{
	wifi_manager_cb_data *cb_data = NULL;
	WIFI_RET_VAL_IF_FAIL(manager, WIFI_ERROR_INVALID_PARAMETER);
	WIFI_RET_VAL_IF_FAIL(ap, WIFI_ERROR_INVALID_PARAMETER);

	cb_data = g_new0(wifi_manager_cb_data, 1);
	if (cb_data) {
		cb_data->manager = manager;
		cb_data->ap = ap;
		cb_data->cb = callback;
		cb_data->user_data = user_data;
		return wifi_connect_by_wps_pin(ap->ap, pin, __wifi_common_cb, cb_data);
	}
	return WIFI_ERROR_OUT_OF_MEMORY;
}

wifi_error_e wifi_manager_disconnect(wifi_manager_object *manager,
				     wifi_ap_object *ap, wifi_manager_generic_cb callback, gpointer user_data)
{
	wifi_manager_cb_data *cb_data = NULL;
	WIFI_RET_VAL_IF_FAIL(manager, WIFI_ERROR_INVALID_PARAMETER);
	WIFI_RET_VAL_IF_FAIL(ap, WIFI_ERROR_INVALID_PARAMETER);

	cb_data = g_new0(wifi_manager_cb_data, 1);
	if (cb_data) {
		cb_data->manager = manager;
		cb_data->ap = ap;
		cb_data->cb = callback;
		cb_data->user_data = user_data;
		return wifi_disconnect(ap->ap, __wifi_common_cb, cb_data);
	}
	return WIFI_ERROR_OUT_OF_MEMORY;
}

wifi_error_e wifi_manager_get_connected_ap(wifi_manager_object *manager,
					   wifi_ap_object **ap)
{
	wifi_ap_h ap_h = NULL;
	wifi_error_e err = WIFI_ERROR_INVALID_PARAMETER;
	WIFI_RET_VAL_IF_FAIL(manager, WIFI_ERROR_INVALID_PARAMETER);

	err = wifi_get_connected_ap(&ap_h);
	if (err != WIFI_ERROR_NONE) {
		if (ap_h) {
			wifi_ap_destroy(ap_h);
		}
		return err;
	}
	*ap = _create_ap_object_by_ap_h(ap_h);
	wifi_ap_destroy(ap_h);
	if (!(*ap))
		return WIFI_ERROR_OUT_OF_MEMORY;
	return WIFI_ERROR_NONE;
}

/*
 * Wi-Fi AP API
 */
wifi_ap_object *wifi_manager_ap_clone(wifi_ap_object *ap_obj)
{
	WIFI_RET_VAL_IF_FAIL(ap_obj != NULL, NULL);

	return _create_ap_object_by_ap_h(ap_obj->ap);
}

void wifi_manager_ap_destroy(wifi_ap_object *ap_obj)
{
	WIFI_RET_IF_FAIL(ap_obj != NULL);

	wifi_ap_destroy(ap_obj->ap);
	g_free(ap_obj);
}

gboolean wifi_manager_ap_is_equals(wifi_ap_object *ap_obj1, wifi_ap_object *ap_obj2)
{
	gchar *ssid1, *ssid2;
	wifi_security_type_e sec_type1, sec_type2;
	gboolean is_equals = FALSE;
	WIFI_RET_VAL_IF_FAIL(ap_obj1 != NULL, FALSE);
	WIFI_RET_VAL_IF_FAIL(ap_obj2 != NULL, FALSE);

	ssid1 = wifi_manager_ap_get_ssid(ap_obj1);
	ssid2 = wifi_manager_ap_get_ssid(ap_obj2);

	sec_type1 = wifi_manager_ap_get_security_type(ap_obj1);
	sec_type2 = wifi_manager_ap_get_security_type(ap_obj2);

	is_equals = (!g_strcmp0(ssid1, ssid2)) && (sec_type1 == sec_type2);

	g_free(ssid1);
	g_free(ssid2);

	return is_equals;
}

void wifi_manager_ap_refresh(wifi_ap_object *ap_obj)
{
	WIFI_RET_IF_FAIL(ap_obj);

	wifi_error_e err = wifi_ap_refresh(ap_obj->ap);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_ap_refresh() is failed. error = %s",
			     wifi_error_to_string(err));
	}
}

wifi_ap_h wifi_manager_ap_get_ap_h(wifi_ap_object *ap_obj)
{
	WIFI_RET_VAL_IF_FAIL(ap_obj, NULL);

	return ap_obj->ap;
}

wifi_error_e wifi_manager_ap_get_last_connection_error(wifi_manager_object *manager,
						       wifi_ap_object *ap_obj)
{
	WIFI_RET_VAL_IF_FAIL(manager, WIFI_ERROR_INVALID_PARAMETER);
	WIFI_RET_VAL_IF_FAIL(ap_obj, WIFI_ERROR_INVALID_PARAMETER);

	if (wifi_manager_ap_is_favorite(manager, ap_obj)) {
		wifi_config_object *config = _find_wifi_config_by_wifi_ap(manager, ap_obj);
		if (config) {
			// Favorite ap's last connection error
			return config->last_error;
		}
	}
	// Connection failed ap's last connection error
	return ap_obj->last_connection_error;
}

void wifi_manager_ap_set_last_connection_error(wifi_ap_object *ap_obj, wifi_error_e error)
{
	WIFI_RET_IF_FAIL(ap_obj);

	ap_obj->last_connection_error = error;
}

gboolean wifi_manager_ap_is_captiveportal(wifi_manager_object *manager, wifi_ap_object *ap_obj)
{
	WIFI_RET_VAL_IF_FAIL(manager != NULL, FALSE);
	WIFI_RET_VAL_IF_FAIL(ap_obj != NULL, FALSE);

	return ap_obj->is_captiveportal;
}

void wifi_manager_ap_set_captiveportal(wifi_ap_object *ap_obj, gboolean is_captiveportal)
{
	WIFI_RET_IF_FAIL(ap_obj != NULL);

	ap_obj->is_captiveportal = is_captiveportal;
}

wifi_error_e wifi_manager_ap_forget(wifi_ap_object *ap_obj)
{
	wifi_error_e err;
	WIFI_RET_VAL_IF_FAIL(ap_obj != NULL, WIFI_ERROR_INVALID_PARAMETER);

	err = wifi_forget_ap(ap_obj->ap);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_forget_ap() is failed. error = %s",
			     wifi_error_to_string(err));
	}
	return err;
}

gchar *wifi_manager_ap_get_ssid(wifi_ap_object *ap_obj)
{
	gchar *ssid = NULL;
	WIFI_RET_VAL_IF_FAIL(ap_obj, NULL);

	wifi_error_e err = wifi_ap_get_essid(ap_obj->ap, &ssid);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_ap_get_essid() is failed. error = %s",
			     wifi_error_to_string(err));
		if (ssid) {
			g_free(ssid);
		}
		return NULL;
	}
	return ssid;
}

gint wifi_manager_ap_get_rssi(wifi_ap_object *ap_obj)
{
	WIFI_RET_VAL_IF_FAIL(ap_obj, 0);

	return ap_obj->rssi;
}

wifi_manager_ap_signal_strength wifi_manager_ap_get_signal_strength(wifi_ap_object *ap_obj)
{
	WIFI_RET_VAL_IF_FAIL(ap_obj, WIFI_MANAGER_SIGNAL_STRENGTH_NULL);

	/* Wi-Fi Signal Strength Display (dB / ConnMan normalized value)
	 *
	 * Excellent : -63 ~     / 57 ~
	 * Good:       -74 ~ -64 / 46 ~ 56
	 * Weak:       -82 ~ -75 / 38 ~ 45
	 * Very weak:      ~ -83 /    ~ 37
	 */
	if (ap_obj->rssi >= -63)
		return WIFI_MANAGER_SIGNAL_STRENGTH_EXCELLENT;
	else if (ap_obj->rssi >= -74)
		return WIFI_MANAGER_SIGNAL_STRENGTH_GOOD;
	else if (ap_obj->rssi >= -82)
		return WIFI_MANAGER_SIGNAL_STRENGTH_WEAK;
	else if (ap_obj->rssi >= -92)
		return WIFI_MANAGER_SIGNAL_STRENGTH_VERY_WEAK;
	else
		return WIFI_MANAGER_SIGNAL_STRENGTH_NULL;
}

gint wifi_manager_ap_get_max_speed(wifi_ap_object *ap_obj)
{
	gint max_speed = 0;
	WIFI_RET_VAL_IF_FAIL(ap_obj, 0);

	wifi_error_e err = wifi_ap_get_max_speed(ap_obj->ap, &max_speed);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_ap_get_max_speed() is failed. error = %s",
			     wifi_error_to_string(err));
		return 0;
	}
	return max_speed;
}

void wifi_manager_ap_set_password(wifi_ap_object *ap_obj, const gchar *password)
{
	wifi_error_e err;
	WIFI_RET_IF_FAIL(ap_obj);

	err = wifi_ap_set_passphrase(ap_obj->ap, password);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_ap_set_passphrase() is failed. error = %s",
			     wifi_error_to_string(err));
	}
}

wifi_eap_type_e wifi_manager_ap_get_eap_type(wifi_ap_object *ap_obj)
{
	wifi_eap_type_e eap_type = WIFI_EAP_TYPE_AKA;
	wifi_error_e err;
	WIFI_RET_VAL_IF_FAIL(ap_obj, eap_type);

	err = wifi_ap_get_eap_type(ap_obj->ap, &eap_type);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_ap_get_eap_type() is failed. error = %s",
			     wifi_error_to_string(err));
	}
	return eap_type;
}

void wifi_manager_ap_set_eap_type(wifi_ap_object *ap_obj, wifi_eap_type_e type)
{
	wifi_error_e err;
	WIFI_RET_IF_FAIL(ap_obj);

	err = wifi_ap_set_eap_type(ap_obj->ap, type);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_ap_set_eap_type() is failed. error = %s",
			     wifi_error_to_string(err));
	}
}

wifi_security_type_e wifi_manager_ap_get_security_type(wifi_ap_object *ap_obj)
{
	wifi_security_type_e sec_type = WIFI_SECURITY_TYPE_NONE;
	wifi_error_e err;
	WIFI_RET_VAL_IF_FAIL(ap_obj, sec_type);

	err = wifi_ap_get_security_type(ap_obj->ap, &sec_type);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_ap_get_security_type() is failed. error = %s",
			     wifi_error_to_string(err));
	}
	return sec_type;
}

gchar *wifi_manager_ap_get_security_type_text(wifi_ap_object *ap_obj)
{
	switch (wifi_manager_ap_get_security_type(ap_obj)) {
	case WIFI_SECURITY_TYPE_NONE:
		return g_strdup(STR_OPEN);

	case WIFI_SECURITY_TYPE_WEP:
	case WIFI_SECURITY_TYPE_WPA_PSK:
	case WIFI_SECURITY_TYPE_WPA2_PSK:
		return g_strdup(STR_SECURED);

	case WIFI_SECURITY_TYPE_EAP:
		return g_strdup(STR_SECURED_EAP);

#if defined TIZEN_WLAN_CHINA_WAPI
	case WIFI_SECURITY_TYPE_WAPI_PSK:
		return g_strdup(STR_WAPI_PSK);

	case WIFI_SECURITY_TYPE_WAPI_CERT:
		return g_strdup(STR_WAPI_CERT);
#endif
	default:
		return g_strdup("");
	}
}

wifi_connection_state_e wifi_manager_ap_get_connection_state(wifi_ap_object *ap_obj)
{
	wifi_connection_state_e conn_state = WIFI_CONNECTION_STATE_FAILURE;
	wifi_error_e err;
	WIFI_RET_VAL_IF_FAIL(ap_obj, WIFI_CONNECTION_STATE_FAILURE);
	err = wifi_ap_get_connection_state(ap_obj->ap, &conn_state);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_ap_get_connection_state() is failed. error = %s",
			     wifi_error_to_string(err));
	}
	return conn_state;
}

gchar *wifi_manager_ap_get_ip_address(wifi_ap_object *ap_obj)
{
	gchar *ip_address = NULL;
	WIFI_RET_VAL_IF_FAIL(ap_obj, NULL);

	wifi_error_e err = wifi_ap_get_ip_address(ap_obj->ap,
						  WIFI_ADDRESS_FAMILY_IPV4, &ip_address);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_ap_get_ip_address() is failed. error = %s",
			     wifi_error_to_string(err));
		if (ip_address) {
			g_free(ip_address);
		}
		return NULL;
	}
	return ip_address;
}

void wifi_manager_ap_set_ip_address(wifi_ap_object *ap_obj, const gchar *ip_address)
{
	wifi_error_e err;
	WIFI_RET_IF_FAIL(ap_obj);

	err = wifi_ap_set_ip_address(ap_obj->ap, WIFI_ADDRESS_FAMILY_IPV4, ip_address);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_ap_set_ip_address() is failed. error = %s",
			     wifi_error_to_string(err));
	}
}

gchar *wifi_manager_ap_get_gateway_address(wifi_ap_object *ap_obj)
{
	gchar *gateway = NULL;
	WIFI_RET_VAL_IF_FAIL(ap_obj, NULL);

	wifi_error_e err = wifi_ap_get_gateway_address(ap_obj->ap,
						       WIFI_ADDRESS_FAMILY_IPV4, &gateway);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_ap_get_gateway_address() is failed. error = %s",
			     wifi_error_to_string(err));
		if (gateway) {
			g_free(gateway);
		}
		return NULL;
	}
	return gateway;
}

void wifi_manager_ap_set_gateway_address(wifi_ap_object *ap_obj, const gchar *gateway_address)
{
	wifi_error_e err;
	WIFI_RET_IF_FAIL(ap_obj);

	err = wifi_ap_set_gateway_address(ap_obj->ap, WIFI_ADDRESS_FAMILY_IPV4, gateway_address);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_ap_set_gateway_address() is failed. error = %s",
			     wifi_error_to_string(err));
	}
}

gchar *wifi_manager_ap_get_subnet_mask(wifi_ap_object *ap_obj)
{
	gchar *subnet_mask = NULL;
	WIFI_RET_VAL_IF_FAIL(ap_obj, NULL);

	wifi_error_e err = wifi_ap_get_subnet_mask(ap_obj->ap,
						   WIFI_ADDRESS_FAMILY_IPV4, &subnet_mask);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_ap_get_subnet_mask() is failed. error = %s",
			     wifi_error_to_string(err));
		if (subnet_mask) {
			g_free(subnet_mask);
		}
		return NULL;
	}
	return subnet_mask;
}

void wifi_manager_ap_set_subnet_mask(wifi_ap_object *ap_obj, const gchar *subnet_mask)
{
	wifi_error_e err;
	WIFI_RET_IF_FAIL(ap_obj);

	err = wifi_ap_set_subnet_mask(ap_obj->ap, WIFI_ADDRESS_FAMILY_IPV4, subnet_mask);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_ap_set_subnet_mask() is failed. error = %s",
			     wifi_error_to_string(err));
	}
}

gchar *wifi_manager_ap_get_dns_address(wifi_ap_object *ap_obj, gint order)
{
	gchar *dns_address = NULL;
	WIFI_RET_VAL_IF_FAIL(ap_obj, NULL);

	wifi_error_e err = wifi_ap_get_dns_address(ap_obj->ap,
						   order, WIFI_ADDRESS_FAMILY_IPV4, &dns_address);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_ap_get_dns_address() is failed. error = %s",
			     wifi_error_to_string(err));
		if (dns_address) {
			g_free(dns_address);
		}
		return NULL;
	}
	return dns_address;
}

void wifi_manager_ap_set_dns_address(wifi_ap_object *ap_obj, const gchar *dns_address, gint order)
{
	wifi_error_e err;
	WIFI_RET_IF_FAIL(ap_obj);

	err = wifi_ap_set_dns_address(ap_obj->ap, order, WIFI_ADDRESS_FAMILY_IPV4, dns_address);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_ap_set_dns_address() is failed. error = %s",
			     wifi_error_to_string(err));
	}
}

wifi_ip_config_type_e wifi_manager_ap_get_ip_config(wifi_ap_object *ap_obj)
{
	wifi_ip_config_type_e config_type = WIFI_IP_CONFIG_TYPE_NONE;
	wifi_error_e err;
	WIFI_RET_VAL_IF_FAIL(ap_obj, WIFI_IP_CONFIG_TYPE_NONE);

	err = wifi_ap_get_ip_config_type(ap_obj->ap,
					 WIFI_ADDRESS_FAMILY_IPV4, &config_type);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_ap_get_ip_config_type() is failed. error = %s",
			     wifi_error_to_string(err));
	}
	return config_type;
}

void wifi_manager_ap_set_ip_config(wifi_ap_object *ap_obj, wifi_ip_config_type_e config_type)
{
	wifi_error_e err;
	WIFI_RET_IF_FAIL(ap_obj);
	err = wifi_ap_set_ip_config_type(ap_obj->ap,
					 WIFI_ADDRESS_FAMILY_IPV4, config_type);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_ap_set_ip_config_type() is failed. error = %s",
			     wifi_error_to_string(err));
	}
}

bool wifi_manager_ap_is_ip_config_static(wifi_ap_object *ap_obj)
{
	return wifi_manager_ap_get_ip_config(ap_obj) == WIFI_IP_CONFIG_TYPE_STATIC;
}

void wifi_manager_ap_set_ip_config_static(wifi_ap_object *ap_obj)
{
	wifi_manager_ap_set_ip_config(ap_obj, WIFI_IP_CONFIG_TYPE_STATIC);
}

wifi_proxy_type_e wifi_manager_ap_get_proxy_type(wifi_ap_object *ap_obj)
{
	wifi_proxy_type_e proxy_type = WIFI_PROXY_TYPE_AUTO;
	wifi_error_e err;
	WIFI_RET_VAL_IF_FAIL(ap_obj, WIFI_PROXY_TYPE_AUTO);

	err = wifi_ap_get_proxy_type(ap_obj->ap, &proxy_type);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_ap_get_proxy_type() is failed. error = %s",
			     wifi_error_to_string(err));
	}
	return proxy_type;
}

void wifi_manager_ap_set_proxy_type(wifi_ap_object *ap_obj, wifi_proxy_type_e proxy_type)
{
	wifi_error_e err;
	WIFI_RET_IF_FAIL(ap_obj);

	err = wifi_ap_set_proxy_type(ap_obj->ap, proxy_type);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_ap_set_proxy_type() is failed. error = %s",
			     wifi_error_to_string(err));
	}
}

bool wifi_manager_ap_is_proxy_manual(wifi_ap_object *ap_obj)
{
	return wifi_manager_ap_get_proxy_type(ap_obj) == WIFI_PROXY_TYPE_MANUAL;
}

void wifi_manager_ap_set_proxy_manual(wifi_ap_object *ap_obj)
{
	wifi_manager_ap_set_proxy_type(ap_obj, WIFI_PROXY_TYPE_MANUAL);
}

gchar *wifi_manager_ap_get_proxy_address(wifi_ap_object *ap_obj)
{
	gchar *proxy_address = NULL;
	char *saveptr = NULL;
	WIFI_RET_VAL_IF_FAIL(ap_obj != NULL, NULL);

	proxy_address = _ap_get_proxy_address(ap_obj);
	if (proxy_address == NULL) {
		WIFI_LOG_INFO("Proxy address is NULL.");
		return NULL;
	}

	return g_strdup(strtok_r(proxy_address, ":", &saveptr));
}

gchar *wifi_manager_ap_get_proxy_port(wifi_ap_object *ap_obj)
{
	gchar *proxy_address = NULL;
	char *saveptr = NULL;
	WIFI_RET_VAL_IF_FAIL(ap_obj != NULL, NULL);

	proxy_address = _ap_get_proxy_address(ap_obj);
	if (proxy_address == NULL) {
		WIFI_LOG_INFO("Proxy address is NULL.");
		return NULL;
	}

	strtok_r(proxy_address, ":", &saveptr);
	return g_strdup(strtok(NULL, ":"));
}

void wifi_manager_ap_set_proxy_address(wifi_ap_object *ap_obj, const gchar *proxy_address)
{
	wifi_error_e err;
	WIFI_RET_IF_FAIL(ap_obj);

	err = wifi_ap_set_proxy_address(ap_obj->ap, WIFI_ADDRESS_FAMILY_IPV4, proxy_address);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_ap_set_proxy_type() is failed. error = %s",
			     wifi_error_to_string(err));
	}
}

bool wifi_manager_ap_is_wps_mode(wifi_ap_object *ap_obj)
{
	return ap_obj->wps_mode;
}

bool wifi_manager_ap_is_favorite(wifi_manager_object *manager, wifi_ap_object *ap_obj)
{
	bool is_favorite = false;
	wifi_error_e err;
	WIFI_RET_VAL_IF_FAIL(ap_obj != NULL, false);

	err = wifi_ap_is_favorite(ap_obj->ap, &is_favorite);
	if (err != WIFI_ERROR_NONE) {
		WIFI_LOG_ERR("wifi_ap_is_favorite() is failed. error = %d", err);
	}
	return is_favorite;
}

gboolean wifi_manager_ap_has_configuration(wifi_manager_object *manager,
					   wifi_ap_object *ap_obj)
{
	WIFI_RET_VAL_IF_FAIL(manager != NULL, FALSE);
	WIFI_RET_VAL_IF_FAIL(ap_obj != NULL, FALSE);

	if (_find_wifi_config_by_wifi_ap(manager, ap_obj) == NULL) {
		WIFI_LOG_INFO("AP config profile not found.");
		return FALSE;
	}
	WIFI_LOG_INFO("AP has config profile.");
	return TRUE;
}

wifi_error_e wifi_manager_ap_remove_configuration(wifi_manager_object *manager,
						  wifi_ap_object *ap_obj)
{
	wifi_config_object *config;
	wifi_error_e error = WIFI_ERROR_NONE;
	WIFI_RET_VAL_IF_FAIL(manager != NULL, WIFI_ERROR_INVALID_PARAMETER);
	WIFI_RET_VAL_IF_FAIL(ap_obj != NULL, WIFI_ERROR_INVALID_PARAMETER);

	config = _find_wifi_config_by_wifi_ap(manager, ap_obj);
	WIFI_RET_VAL_IF_FAIL(config != NULL, WIFI_ERROR_INVALID_OPERATION);

	_update_wifi_config_list(manager);
	return error;
}

wifi_error_e wifi_manager_update_wifi_config_list(wifi_manager_object *manager)
{
	wifi_error_e error = WIFI_ERROR_NONE;

	WIFI_RET_VAL_IF_FAIL(manager != NULL, WIFI_ERROR_INVALID_PARAMETER);

	_update_wifi_config_list(manager);

	return error;
}

/*
 * Wearable connection API (weconn feature)
 */

gboolean wifi_manager_set_wifi_use(wifi_manager_object *manager, gboolean is_wifi_use)
{
	WIFI_RET_VAL_IF_FAIL(manager, FALSE);
	return vconf_set_wifi_use(is_wifi_use ? 1 : 0);
}

gboolean wifi_manager_is_wifi_use(wifi_manager_object *manager)
{
	WIFI_RET_VAL_IF_FAIL(manager, FALSE);
	return vconf_is_wifi_use();
}

gboolean wifi_manager_set_wifi_use_changed_cb(wifi_manager_object *manager,
					      vconf_callback_fn callback, void *data)
{
	WIFI_RET_VAL_IF_FAIL(manager, FALSE);
    WIFI_LOG_INFO("VCONF_WIFI_USE - notify wifi key changed");
	return vconf_notify_wifi_key_changed(callback, data);
}
