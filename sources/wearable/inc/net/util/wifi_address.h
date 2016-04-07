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


#ifndef __WIFI_ADDRESS_H__
#define __WIFI_ADDRESS_H__

#include <wifi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_GUIDE_IP_ADDRESS                "192.168.1.128"
#define DEFAULT_GUIDE_SUBNET_MASK               "255.255.255.0"
#define DEFAULT_GUIDE_GATEWAY_ADDRESS   "192.168.1.1"
#define DEFAULT_GUIDE_DNS1                              "8.8.8.8"
#define DEFAULT_GUIDE_DNS2                              "8.8.4.4"

#define DEFAULT_GUIDE_PROXY_ADDRESS     "proxy.example.com"
#define DEFAULT_GUIDE_PROXY_PORT        "8080"

typedef struct _wifi_address_object wifi_address_object;

wifi_address_object *wifi_address_new();
void                 wifi_address_free(wifi_address_object *object);
wifi_address_object *wifi_address_clone(wifi_address_object *self);

gboolean wifi_address_is_static_ip(wifi_address_object *self);
void     wifi_address_set_static_ip(wifi_address_object *self, gboolean is_static_ip);

gboolean wifi_address_is_proxy_manual(wifi_address_object *self);
void     wifi_address_set_proxy_manual(wifi_address_object *self, gboolean is_proxy_manual);

wifi_security_type_e wifi_address_get_security_type(wifi_address_object *self);
void                 wifi_address_set_security_type(wifi_address_object *self, wifi_security_type_e sec_type);

wifi_eap_type_e wifi_address_get_eap_type(wifi_address_object *self);
void            wifi_address_set_eap_type(wifi_address_object *self, wifi_eap_type_e eap_type);

const gchar *wifi_address_get_password(wifi_address_object *self);
void         wifi_address_set_password(wifi_address_object *self, const gchar *password);

const gchar *wifi_address_get_ip_address(wifi_address_object *self);
void         wifi_address_set_ip_address(wifi_address_object *self, const gchar *ip_address);

const gchar *wifi_address_get_gateway_address(wifi_address_object *self);
void         wifi_address_set_gateway_address(wifi_address_object *self, const gchar *gateway_address);

const gchar *wifi_address_get_subnet_mask(wifi_address_object *self);
void         wifi_address_set_subnet_mask(wifi_address_object *self, const gchar *subnet_mask);

const gchar *wifi_address_get_dns_address(wifi_address_object *self, gint order);
void         wifi_address_set_dns_address(wifi_address_object *self, const gchar *dns, gint order);

const gchar *wifi_address_get_proxy_address(wifi_address_object *self);
void         wifi_address_set_proxy_address(wifi_address_object *self, const gchar *proxy_address);

const gchar *wifi_address_get_proxy_port(wifi_address_object *self);
void         wifi_address_set_proxy_port(wifi_address_object *self, const gchar *proxy_port);

#ifdef __cplusplus
}
#endif

#endif /*__WIFI_ADDRESS_H__*/
