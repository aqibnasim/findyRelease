/* mbed Microcontroller Library
 * Copyright (c) 2017-2019 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "nrf_soc.h"

void power_save(void);

//#define DEBUG
#define VOL_OFFSET		0.2
#define VOL_LEVEL_2		0.2f + VOL_OFFSET
#define VOL_LEVEL_3		0.3f + VOL_OFFSET
#define VOL_LEVEL_4		0.4f + VOL_OFFSET
#define VOL_LEVEL_6		0.6f + VOL_OFFSET
