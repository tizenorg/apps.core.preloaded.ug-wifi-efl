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
#include <wifi.h>

#include "util/log.h"
#include "net/util/wifi_address.h"

struct _wifi_address_object {
	gboolean is_static_ip;
	gboolean is_proxy_manual;
	wifi_security_type_e sec_type;
	wifi_eap_type_e eap_type;
	gchar *password;
	gchar *ip_address;
	gchar *gateway_address;
	gchar *subnet_mask;
	gchar *dns_address1;
	gchar *dns_address2;
	gchar *proxy_address;
	gchar *proxy_port;
};

wifi_address_object *wifi_address_new()
{
	return g_new0(wifi_address_object, 1);
}

void wifi_address_free(wifi_address_object *object)
{
	WIFI_RET_IF_FAIL(object);

	g_free(object->password);
	g_free(object->ip_address);
	g_free(object->gateway_address);
	g_free(object->subnet_mask);
	g_free(object->dns_address1);
	g_free(object->dns_address2);
	g_free(object->proxy_address);
	g_free(object->proxy_port);
	g_free(object);
}

wifi_address_object *wifi_address_clone(wifi_address_object *self)
{
	wifi_address_object *clone = wifi_address_new();
	WIFI_RET_VAL_IF_FAIL(clone, NULL);

	wifi_address_set_security_type(clone, self->sec_type);
	wifi_address_set_eap_type(clone, self->eap_type);
	wifi_address_set_password(clone, self->password);
	wifi_address_set_static_ip(clone, self->is_static_ip);
	if (self->is_static_ip) {
		wifi_address_set_ip_address(clone, self->ip_address);
		wifi_address_set_gateway_address(clone, self->gateway_address);
		wifi_address_set_subnet_mask(clone, self->subnet_mask);
		wifi_address_set_dns_address(clone, self->dns_address1, 1);
		wifi_address_set_dns_address(clone, self->dns_address2, 2);
	}
	wifi_address_set_proxy_address(clone, self->proxy_address);
	wifi_address_set_proxy_port(clone, self->proxy_port);
	return clone;
}

gboolean wifi_address_is_static_ip(wifi_address_object *self)
{
	WIFI_RET_VAL_IF_FAIL(self, FALSE);
	return self->is_static_ip;
}

void wifi_address_set_static_ip(wifi_address_object *self, gboolean is_static_ip)
{
	WIFI_RET_IF_FAIL(self);

	self->is_static_ip = is_static_ip;
}

gboolean wifi_address_is_proxy_manual(wifi_address_object *self)
{
	WIFI_RET_VAL_IF_FAIL(self, FALSE);
	return self->is_proxy_manual;
}

void wifi_address_set_proxy_manual(wifi_address_object *self, gboolean is_proxy_manual)
{
	WIFI_RET_IF_FAIL(self);

	self->is_proxy_manual = is_proxy_manual;
}

wifi_security_type_e wifi_address_get_security_type(wifi_address_object *self)
{
	WIFI_RET_VAL_IF_FAIL(self, WIFI_EAP_TYPE_SIM);
	return self->sec_type;
}

void wifi_address_set_security_type(wifi_address_object *self, wifi_security_type_e sec_type)
{
	WIFI_RET_IF_FAIL(self);

	self->sec_type = sec_type;
}

wifi_eap_type_e wifi_address_get_eap_type(wifi_address_object *self)
{
	WIFI_RET_VAL_IF_FAIL(self, WIFI_EAP_TYPE_AKA);
	return self->eap_type;
}

void wifi_address_set_eap_type(wifi_address_object *self, wifi_eap_type_e eap_type)
{
	WIFI_RET_IF_FAIL(self);

	self->eap_type = eap_type;
}

const gchar *wifi_address_get_password(wifi_address_object *self)
{
	WIFI_RET_VAL_IF_FAIL(self, NULL);
	return self->password;
}

void wifi_address_set_password(wifi_address_object *self, const gchar *password)
{
	WIFI_RET_IF_FAIL(self);
	WIFI_RET_IF_FAIL(password);

	if (self->password)
		g_free(self->password);
	self->password = g_strdup(password);
}

const gchar *wifi_address_get_ip_address(wifi_address_object *self)
{
	WIFI_RET_VAL_IF_FAIL(self, NULL);
	return self->ip_address;
}

void wifi_address_set_ip_address(wifi_address_object *self, const gchar *ip_address)
{
	WIFI_RET_IF_FAIL(self);
	WIFI_RET_IF_FAIL(ip_address);

	if (self->ip_address)
		g_free(self->ip_address);
	self->ip_address = g_strdup(ip_address);
}

const gchar *wifi_address_get_gateway_address(wifi_address_object *self)
{
	WIFI_RET_VAL_IF_FAIL(self, NULL);
	return self->gateway_address;
}

void wifi_address_set_gateway_address(wifi_address_object *self, const gchar *gateway_address)
{
	WIFI_RET_IF_FAIL(self);
	WIFI_RET_IF_FAIL(gateway_address);

	if (self->gateway_address)
		g_free(self->gateway_address);
	self->gateway_address = g_strdup(gateway_address);
}

const gchar *wifi_address_get_subnet_mask(wifi_address_object *self)
{
	WIFI_RET_VAL_IF_FAIL(self, NULL);
	return self->subnet_mask;
}

void wifi_address_set_subnet_mask(wifi_address_object *self, const gchar *subnet_mask)
{
	WIFI_RET_IF_FAIL(self);
	WIFI_RET_IF_FAIL(subnet_mask);

	if (self->subnet_mask)
		g_free(self->subnet_mask);
	self->subnet_mask = g_strdup(subnet_mask);
}

const gchar *wifi_address_get_dns_address(wifi_address_object *self, gint order)
{
	WIFI_RET_VAL_IF_FAIL(self, NULL);

	if (order == 1)
		return self->dns_address1;
	else if (order == 2)
		return self->dns_address2;
	WIFI_LOG_ERR("Invaild order[%d] of dns address.", order);
	return NULL;
}

void wifi_address_set_dns_address(wifi_address_object *self, const gchar *dns, gint order)
{
	WIFI_RET_IF_FAIL(self);
	WIFI_RET_IF_FAIL(dns);

	if (order == 1) {
		if (self->dns_address1) {
			g_free(self->dns_address1);
		}
		self->dns_address1 = g_strdup(dns);
	} else if (order == 2) {
		if (self->dns_address2) {
			g_free(self->dns_address2);
		}
		self->dns_address2 = g_strdup(dns);
	} else {
		WIFI_LOG_ERR("Invaild order[%d] of dns address.", order);
	}
}

const gchar *wifi_address_get_proxy_address(wifi_address_object *self)
{
	WIFI_RET_VAL_IF_FAIL(self, NULL);
	return self->proxy_address;
}

void wifi_address_set_proxy_address(wifi_address_object *self, const gchar *proxy_address)
{
	WIFI_RET_IF_FAIL(self);
	WIFI_RET_IF_FAIL(proxy_address);

	if (self->proxy_address)
		g_free(self->proxy_address);
	self->proxy_address = g_strdup(proxy_address);
}

const gchar *wifi_address_get_proxy_port(wifi_address_object *self)
{
	WIFI_RET_VAL_IF_FAIL(self, NULL);
	return self->proxy_port;
}

void wifi_address_set_proxy_port(wifi_address_object *self, const gchar *proxy_port)
{
	WIFI_RET_IF_FAIL(self);
	WIFI_RET_IF_FAIL(proxy_port);

	if (self->proxy_port)
		g_free(self->proxy_port);
	self->proxy_port = g_strdup(proxy_port);
}
