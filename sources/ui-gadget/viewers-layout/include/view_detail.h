/*
  * Copyright 2012  Samsung Electronics Co., Ltd
  *
  * Licensed under the Flora License, Version 1.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *    http://www.tizenopensource.org/license
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */



#ifndef __VIEW_DETAIL_H_
#define __VIEW_DETAIL_H_


void view_detail(wifi_device_info_t *device_info);

int view_detail_current_proxy_address_set(const char* proxy_address);
const char* view_detail_current_proxy_address_get(void);
int detailview_ip_and_dns_type_set_as_static();

int detailview_modified_ip_address_set(char* data);
int detailview_modified_gateway_address_set(char* data);
int detailview_modified_subnet_mask_set(char* data);
int detailview_modified_dns1_address_set(char* data);
int detailview_modified_dns2_address_set(char* data);

const char* detailview_modified_ip_address_get(void);
const char* detailview_modified_gateway_address_get(void);
const char* detailview_modified_subnet_mask_get(void);
const char* detailview_modified_dns1_address_get(void);
const char* detailview_modified_dns2_address_get(void);

#endif
