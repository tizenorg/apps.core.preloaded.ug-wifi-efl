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


#ifndef __UTIL_LOCALE_H__
#define __UTIL_LOCALE_H__

#include <glib.h>
#include <libintl.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline char *wifi_text(const char *msgid)
{
	return gettext(msgid);
}

#define STR_WIFI_NETWORKS                               wifi_text("WDS_WIFI_MBODY_WI_FI_NETWORKS_ABB")
#define STR_WIFI_NETWORKS_HEADER                               wifi_text("WDS_WIFI_HEADER_WI_FI_NETWORKS_ABB")
#define STR_CONNECT_UPPER                                             wifi_text("WDS_WIFI_ACBUTTON_CONNECT_ABB")
#define STR_OBTAINING_IP                                wifi_text("WDS_WIFI_SBODY_OBTAINING_IP_ADDRESS_ING_ABB")
#define STR_CONNECTING                                  wifi_text("WDS_WIFI_SBODY_CONNECTING_ING")
#define STR_NOT_CONNECTED                               wifi_text("WDS_WIFI_SBODY_NOT_CONNECTED_M_STATUS_ABB")
#define STR_FAILED_TO_OBTAIN_IP                 wifi_text("WDS_WIFI_TPOP_FAILED_TO_OBTAIN_IP_ADDRESS_ABB")
#define STR_INCORRECT_PASSWORD                                   wifi_text("WDS_WIFI_POP_INCORRECT_PASSWORD")
#define STR_CONNECTION_FAILED                                   wifi_text("WDS_WIFI_TPOP_FAILED_TO_CONNECT_TO_WI_FI_AP_ABB")
#define STR_INTERNET_NOT_AVAILABLE                                   wifi_text("WDS_WIFI_SBODY_INTERNET_NOT_AVAILABLE_ABB")
#define STR_UNABLE_SCAN                                 wifi_text("WDS_WIMAX_TPOP_GEAR_WILL_SCAN_FOR_WI_FI_NETWORKS_WHEN_DISCONNECTED_FROM_YOUR_MOBILE_DEVICE")
#define STR_SCANNING                                    wifi_text("WDS_WIFI_POP_SCANNING_ING")
#define STR_CONNECTED                                   wifi_text("WDS_WIFI_SBODY_CONNECTED_M_STATUS_ABB")
#define STR_DISCONNECTED                                wifi_text("WDS_WIFI_SBODY_DISCONNECTED_M_STATUS")
#define STR_FORGET                                              wifi_text("WDS_WIFI_BUTTON_FORGET_ABB")
#define STR_FORGET_UPPER                                              wifi_text("WDS_WIFI_ACBUTTON_FORGET_ABB")
#define STR_SIGNAL_STRENGTH                             wifi_text("WDS_WIFI_MBODY_SIGNAL_STRENGTH_ABB")
#define STR_LINK_SPEED                                  wifi_text("WDS_WIFI_MBODY_LINK_SPEED_ABB")
#define STR_LINK_SPEED_MBPS                                  wifi_text("WDS_WIFI_SBODY_PD_MBPS_ABB")
#define STR_STATUS                                              wifi_text("WDS_WIFI_OPT_STATUS")
#define STR_SECURED                                             wifi_text("WDS_WIFI_SBODY_SECURED_ABB")
#define STR_SECURED_EAP                                             wifi_text("WDS_WIFI_SBODY_SECURED_HEAP_ABB")
#define STR_SECURED_WPS                                             wifi_text("WDS_WIFI_SBODY_SECURED_HWPS_ABB")
#define STR_WIFI                                                wifi_text("WDS_WIFI_OPT_WI_FI")
#define STR_OPEN                                        wifi_text("WDS_WIFI_SBODY_OPEN_ABB")
#define STR_SAVED                                       wifi_text("WDS_WIFI_BODY_SAVED_M_STATUS")
#define STR_SHOW_PASSWORD                               wifi_text("WDS_WIFI_OPT_SHOW_PASSWORD")
#define STR_SCAN                                        wifi_text("WDS_WIFI_ACBUTTON_SCAN_ABB")
#define STR_VERY_STRONG                           wifi_text("WDS_WIFI_SBODY_VERY_STRONG_ABB")
#define STR_STRONG                           wifi_text("WDS_WIFI_SBODY_STRONG_ABB2")
#define STR_GOOD                                        wifi_text("WDS_WIFI_SBODY_GOOD_ABB")
#define STR_WEAK                                        wifi_text("WDS_WIFI_SBODY_WEAK_ABB")
#define STR_CONNECTED_VIA_BT                    wifi_text("WDS_WIFI_SBODY_CONNECTED_VIA_BLUETOOTH_ABB")
#define STR_SAVE                                                wifi_text("WDS_WIFI_ACBUTTON_SAVE_ABB")
#define STR_PASSWORD                                    wifi_text("WDS_WIFI_BODY_PASSWORD_ABB")
#define STR_PASSWORD_HEADER                                    wifi_text("WDS_WIFI_HEADER_PASSWORD_ABB")
#define STR_OK                                          wifi_text("WDS_WIFI_BUTTON_OK")
#define STR_CANCEL                                              wifi_text("WDS_WIFI_ACBUTTON_CANCEL_ABB")
#define STR_MAXIMUM_NUMBER                              wifi_text("WDS_WIFI_TPOP_MAXIMUM_NUMBER_OF_CHARACTERS_REACHED")
#define STR_AUTH_ERR                  wifi_text("WDS_WIFI_SBODY_AUTHENTICATION_ERROR_OCCURRED")
#define STR_AUTH_ERR_POPUP                  wifi_text("WDS_WIFI_TPOP_AUTHENTICATION_ERROR_OCCURRED_ABB")
#define STR_STATIC_IP                                   wifi_text("WDS_WIFI_MBODY_STATIC_IP_ABB")
#define STR_PROXY_SETTINGS                              wifi_text("WDS_WIFI_BODY_PROXY_SETTINGS")
#define STR_IP_ADDRESS                                  wifi_text("WDS_WIFI_BODY_IP_ADDRESS")
#define STR_SUBNETMASK                                  wifi_text("WDS_WIFI_MBODY_SUBNET_MASK_ABB")
#define STR_GATYEWAY                                    wifi_text("WDS_WIFI_MBODY_GATEWAY_ADDRESS_ABB")
#define STR_DNS                                                wifi_text("WDS_WIFI_MBODY_DNS_PD_ABB")
#define STR_PROXY_ADDRESS                               wifi_text("WDS_WIFI_MBODY_PROXY_ADDRESS_ABB")
#define STR_PROXY_PORT                                  wifi_text("WDS_WIFI_MBODY_PROXY_PORT_ABB")
#define STR_EAP_METHOD_TITLE                                  wifi_text("WDS_WIFI_HEADER_EAP_METHOD_ABB")
#define STR_EAP_METHOD_MENU                                  wifi_text("WDS_WIFI_MBODY_EAP_METHOD_ABB")
#define STR_TURNED_OFF                                  wifi_text("WDS_WIFI_SBODY_WI_FI_TURNED_OFF_ABB")
#define STR_WPS_METHOD                               wifi_text("WDS_WIFI_MBODY_WPS_METHOD_ABB")
#define STR_WPS_PIN_STR                                 wifi_text("WDS_WIFI_TPOP_ENTER_PIN_ON_ROUTER_NPS_ABB")
#define STR_WPS_BTN_STR                                 wifi_text("WDS_WIFI_BODY_PRESS_WPS_ON_YOUR_WI_FI_ACCESS_POINT_WITHIN_PD_MINUTES")
#define STR_WPS_TITLE                                   wifi_text("WDS_WIFI_HEADER_WPS_METHOD_ABB")
#define STR_WPS_PIN                                             wifi_text("WDS_WIFI_MBODY_WPS_PIN_ABB")
#define STR_WPS_BTN                                             wifi_text("WDS_WIFI_BODY_WPS_BUTTON")
#define STR_CONNECT_Q                                   wifi_text("WDS_WIFI_TPOP_CONNECT_Q_NSIGNAL_STRENGTH_NPS_ABB")
#define STR_RESTRICTS_USE_OF_WI_FI                                   wifi_text("WDS_WIFI_TPOP_SECURITY_POLICY_PREVENTS_USE_OF_WI_FI")
#define STR_NOT_SUPPORTED                                             wifi_text("WDS_WIFI_SBODY_NOT_SUPPORTED_ABB")
#define STR_NO_AP                                             wifi_text("WDS_WIFI_NPBODY_NO_WI_FI_ACCESS_POINT_FOUND")
#define STR_ON_FOR_TTS                                             wifi_text("WDS_PN_SBODY_ON_M_STATUS_ABB")
#define STR_OFF_FOR_TTS                                             wifi_text("WDS_PN_SBODY_OFF_M_STATUS_ABB")
#define STR_SWITCH_FOR_TTS                                           wifi_text("WDS_ALM_TBOPT_SWITCH")
#define STR_OK_BTN_FOR_TTS                                          wifi_text("WDS_WIFI_BUTTON_OK")
#define STR_CANCEL_BTN_FOR_TTS                                          wifi_text("WDS_WIFI_BUTTON_CANCEL")
#define STR_TICKBOX_FOR_TTS                                           wifi_text("IDS_COM_BODY_TICKBOX_T_TTS")
#define STR_TICK_FOR_TTS                                           wifi_text("IDS_COM_BODY_TICK_T_TTS")
#define STR_UNTICK_FOR_TTS                                           wifi_text("IDS_COM_BODY_UNTICK_T_TTS")
#define STR_EDITFIELD_FOR_TTS                                           wifi_text("IDS_TPLATFORM_BODY_EDIT_FIELD_M_NOUN_T_TTS")

#if defined TIZEN_WLAN_CHINA_WAPI
#define STR_WAPI_PSK                                    "WAPI PSK"
#define STR_WAPI_CERT                                   "WAPI CERT"
#endif

#ifdef __cplusplus
}
#endif

#endif /* __UTIL_LOCALE_H__ */
