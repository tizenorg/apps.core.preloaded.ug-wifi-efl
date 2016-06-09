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

#ifndef __WIFI_GENERATE_PIN_H__
#define __WIFI_GENERATE_PIN_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

typedef uint8_t u8;
typedef uint32_t u32;

struct SHA1Context {
	u32 state[5];
	u32 count[2];
	unsigned char buffer[64];
};

unsigned int wifi_generate_pin(void);

#ifdef __cplusplus
}
#endif

#endif
