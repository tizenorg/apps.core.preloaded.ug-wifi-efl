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


#ifndef __WIFI_MANAGER_H__
#define __WIFI_MANAGER_H__

#include <wifi.h>
#include <glib.h>

#include <vconf.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	WIFI_MANAGER_SIGNAL_STRENGTH_EXCELLENT,
	WIFI_MANAGER_SIGNAL_STRENGTH_GOOD,
	WIFI_MANAGER_SIGNAL_STRENGTH_WEAK,
	WIFI_MANAGER_SIGNAL_STRENGTH_VERY_WEAK,
	WIFI_MANAGER_SIGNAL_STRENGTH_NULL
} wifi_manager_ap_signal_strength;

typedef enum {
	WIFI_MANAGER_CSC_UNKNOWN,
	WIFI_MANAGER_CSC_SKT
} wifi_manager_csccode;

typedef struct _wifi_manager_object wifi_manager_object;
typedef struct _wifi_ap_object wifi_ap_object;

typedef void (*wifi_manager_generic_cb)(wifi_manager_object *manager,
					wifi_ap_object *ap, wifi_error_e error_code, gpointer user_data);
typedef void (*wifi_manager_generic_state_changed_cb)(wifi_manager_object *manager,
						      wifi_device_state_e device_state,
						      wifi_connection_state_e connection_state, wifi_ap_object *ap,
						      wifi_rssi_level_e rssi_level, gpointer user_data);

/*
 * Wi-Fi Manager API
 */
wifi_manager_object *wifi_manager_new();
void                 wifi_manager_free(wifi_manager_object *manager);

wifi_error_e wifi_manager_init(wifi_manager_object *manager);
wifi_error_e wifi_manager_deinit(wifi_manager_object *manager);

wifi_error_e wifi_manager_generate_wps_pin(wifi_manager_object *manager, char **pin);

wifi_manager_object *wifi_manager_new_with_init(wifi_error_e *error);
wifi_error_e         wifi_manager_free_with_deinit(wifi_manager_object *manager);

wifi_error_e wifi_manager_get_csccode(wifi_manager_object *manager, wifi_manager_csccode *code);

wifi_error_e wifi_manager_scan(wifi_manager_object *manager,
			       wifi_manager_generic_cb callback, gpointer user_data);
wifi_error_e wifi_manager_scan_request_to_connman(wifi_manager_object *manager);
wifi_error_e wifi_manager_set_autoscan(wifi_manager_object *manager, bool autoscan);
wifi_error_e wifi_manager_get_found_ap_list(wifi_manager_object *manager,
					    GList **ap_list);

wifi_error_e wifi_manager_activate(wifi_manager_object *manager,
				   wifi_manager_generic_cb callback, gpointer user_data);
wifi_error_e wifi_manager_deactivate(wifi_manager_object *manager,
				   wifi_manager_generic_cb callback, gpointer user_data);

wifi_error_e wifi_manager_is_activated(wifi_manager_object *manager, bool *activated);

wifi_error_e wifi_manager_set_device_state_changed_cb(wifi_manager_object *manager,
						      wifi_manager_generic_state_changed_cb callback, void *user_data);
wifi_error_e wifi_manager_unset_device_state_changed_cb(wifi_manager_object *manager);

wifi_error_e wifi_manager_set_background_scan_cb(wifi_manager_object *manager,
						 wifi_manager_generic_cb callback, void *user_data);
wifi_error_e wifi_manager_unset_background_scan_cb(wifi_manager_object *manager);

wifi_error_e wifi_manager_set_connection_state_changed_cb(wifi_manager_object *manager,
							  wifi_manager_generic_state_changed_cb callback, void *user_data);
wifi_error_e wifi_manager_unset_connection_state_changed_cb(wifi_manager_object *manager);
wifi_error_e wifi_manager_get_connection_state(wifi_manager_object *manager,
					       wifi_connection_state_e *connection_state);

wifi_error_e wifi_manager_connect(wifi_manager_object *manager,
				  wifi_ap_object *ap, wifi_manager_generic_cb callback, gpointer user_data);
wifi_error_e wifi_manager_connect_by_wps_pbc(wifi_manager_object *manager,
					     wifi_ap_object *ap, wifi_manager_generic_cb callback, gpointer user_data);
wifi_error_e wifi_manager_connect_by_wps_pin(wifi_manager_object *manager,
					     wifi_ap_object *ap, const gchar *pin,
					     wifi_manager_generic_cb callback, gpointer user_data);
wifi_error_e wifi_manager_disconnect(wifi_manager_object *manager,
				     wifi_ap_object *ap, wifi_manager_generic_cb callback, gpointer user_data);
wifi_error_e wifi_manager_get_connected_ap(wifi_manager_object *manager,
					   wifi_ap_object **ap);
wifi_error_e wifi_manager_update_wifi_config_list(wifi_manager_object *manager);

gint wifi_manager_default_compare_ap(gconstpointer a, gconstpointer b);

/*
 * Wi-Fi AP API
 */
wifi_ap_object *                wifi_manager_ap_clone(wifi_ap_object *ap_obj);
void                            wifi_manager_ap_destroy(wifi_ap_object *ap_obj);
gboolean                        wifi_manager_ap_is_equals(wifi_ap_object *ap_obj1, wifi_ap_object *ap_obj2);
void                            wifi_manager_ap_refresh(wifi_ap_object *ap_obj);
wifi_ap_h                       wifi_manager_ap_get_ap_h(wifi_ap_object *ap_obj);
wifi_error_e                    wifi_manager_ap_get_last_connection_error(wifi_manager_object *manager, wifi_ap_object *ap_obj);
void                            wifi_manager_ap_set_last_connection_error(wifi_ap_object *ap_obj, wifi_error_e error);
gboolean                        wifi_manager_ap_is_captiveportal(wifi_manager_object *manager, wifi_ap_object *ap_obj);
void                            wifi_manager_ap_set_captiveportal(wifi_ap_object *ap_obj, gboolean is_captiveportal);
wifi_error_e                    wifi_manager_ap_forget(wifi_ap_object *ap_obj);
gchar *                         wifi_manager_ap_get_ssid(wifi_ap_object *ap_obj);
gint                            wifi_manager_ap_get_rssi(wifi_ap_object *ap_obj);
wifi_manager_ap_signal_strength wifi_manager_ap_get_signal_strength(wifi_ap_object *ap_obj);
gint                            wifi_manager_ap_get_max_speed(wifi_ap_object *ap_obj);
void                            wifi_manager_ap_set_password(wifi_ap_object *ap_obj, const gchar *password);
wifi_eap_type_e                 wifi_manager_ap_get_eap_type(wifi_ap_object *ap_obj);
void                            wifi_manager_ap_set_eap_type(wifi_ap_object *ap_obj, wifi_eap_type_e type);
wifi_security_type_e            wifi_manager_ap_get_security_type(wifi_ap_object *ap_obj);
gchar *                         wifi_manager_ap_get_security_type_text(wifi_ap_object *ap_obj);
wifi_connection_state_e         wifi_manager_ap_get_connection_state(wifi_ap_object *ap_obj);
gchar *                         wifi_manager_ap_get_ip_address(wifi_ap_object *ap_obj);
void                            wifi_manager_ap_set_ip_address(wifi_ap_object *ap_obj, const gchar *ip_address);
gchar *                         wifi_manager_ap_get_gateway_address(wifi_ap_object *ap_obj);
void                            wifi_manager_ap_set_gateway_address(wifi_ap_object *ap_obj, const gchar *gateway_address);
gchar *                         wifi_manager_ap_get_subnet_mask(wifi_ap_object *ap_obj);
void                            wifi_manager_ap_set_subnet_mask(wifi_ap_object *ap_obj, const gchar *subnet_mask);
gchar *                         wifi_manager_ap_get_dns_address(wifi_ap_object *ap_obj, gint order);
void                            wifi_manager_ap_set_dns_address(wifi_ap_object *ap_obj, const gchar *dns_address, gint order);
wifi_ip_config_type_e           wifi_manager_ap_get_ip_config(wifi_ap_object *ap_obj);
void                            wifi_manager_ap_set_ip_config(wifi_ap_object *ap_obj, wifi_ip_config_type_e config_type);
bool                            wifi_manager_ap_is_ip_config_static(wifi_ap_object *ap_obj);
void                            wifi_manager_ap_set_ip_config_static(wifi_ap_object *ap_obj);
wifi_proxy_type_e               wifi_manager_ap_get_proxy_type(wifi_ap_object *ap_obj);
void                            wifi_manager_ap_set_proxy_type(wifi_ap_object *ap_obj, wifi_proxy_type_e proxy_type);
bool                            wifi_manager_ap_is_proxy_manual(wifi_ap_object *ap_obj);
void                            wifi_manager_ap_set_proxy_manual(wifi_ap_object *ap_obj);
gchar *                         wifi_manager_ap_get_proxy_address(wifi_ap_object *ap_obj);
gchar *                         wifi_manager_ap_get_proxy_port(wifi_ap_object *ap_obj);
void                            wifi_manager_ap_set_proxy_address(wifi_ap_object *ap_obj, const gchar *proxy_address);
bool                            wifi_manager_ap_is_wps_mode(wifi_ap_object *ap_obj);
bool                            wifi_manager_ap_is_favorite(wifi_manager_object *manager, wifi_ap_object *ap_obj);
gboolean                        wifi_manager_ap_has_configuration(wifi_manager_object *manager, wifi_ap_object *ap_obj);
wifi_error_e                    wifi_manager_ap_remove_configuration(wifi_manager_object *manager, wifi_ap_object *ap_obj);

/*
 * Only wearable API (weconn / vconf feature)
 */
gboolean wifi_manager_is_mobile_connected_via_bluetooth(wifi_manager_object *manager);
gboolean wifi_manager_set_wifi_use(wifi_manager_object *manager, gboolean is_wifi_use);
gboolean wifi_manager_is_wifi_use(wifi_manager_object *manager);
gboolean wifi_manager_set_wifi_use_changed_cb(wifi_manager_object *manager,
					      vconf_callback_fn callback, void *data);

static inline
const gchar *wifi_error_to_string(wifi_error_e err_type)
{
	switch (err_type) {
	case WIFI_ERROR_NONE:
		return "NONE";

	case WIFI_ERROR_INVALID_PARAMETER:
		return "INVALID_PARAMETER";

	case WIFI_ERROR_OUT_OF_MEMORY:
		return "OUT_OF_MEMORY";

	case WIFI_ERROR_INVALID_OPERATION:
		return "INVALID_OPERATION";

	case WIFI_ERROR_ADDRESS_FAMILY_NOT_SUPPORTED:
		return "ADDRESS_FAMILY_NOT_SUPPORTED";

	case WIFI_ERROR_OPERATION_FAILED:
		return "OPERATION_FAILED";

	case WIFI_ERROR_NO_CONNECTION:
		return "NO_CONNECTION";

	case WIFI_ERROR_NOW_IN_PROGRESS:
		return "NOW_IN_PROGRESS";

	case WIFI_ERROR_ALREADY_EXISTS:
		return "ALREADY_EXISTS";

	case WIFI_ERROR_OPERATION_ABORTED:
		return "OPERATION_ABORTED";

	case WIFI_ERROR_DHCP_FAILED:
		return "DHCP_FAILED";

	case WIFI_ERROR_INVALID_KEY:
		return "INVALID_KEY";

	case WIFI_ERROR_NO_REPLY:
		return "NO_REPLY";

	case WIFI_ERROR_SECURITY_RESTRICTED:
		return "SECURITY_RESTRICTED";

	case WIFI_ERROR_PERMISSION_DENIED:
		return "PERMISSION_DENIED";

	case WIFI_ERROR_NOT_SUPPORTED:
		return "NOT_SUPPORTED";

	default:
		return "UNKNOWN";
	}
}

static inline
const gchar *wifi_connection_state_to_string(wifi_connection_state_e state)
{
	switch (state) {
	case WIFI_CONNECTION_STATE_ASSOCIATION:
		return "ASSOCIATION";

	case WIFI_CONNECTION_STATE_CONFIGURATION:
		return "CONFIGURATION";

	case WIFI_CONNECTION_STATE_FAILURE:
		return "FAILURE";

	case WIFI_CONNECTION_STATE_CONNECTED:
		return "CONNECTED";

	case WIFI_CONNECTION_STATE_DISCONNECTED:
		return "DISCONNECTED";

	default:
		return "UNKNOWN";
	}
}

#ifdef __cplusplus
}
#endif

#endif /*__WIFI_MANAGER_H__*/
