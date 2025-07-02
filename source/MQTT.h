/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MQTT_FREERTOS_H
#define MQTT_FREERTOS_H

#include "lwip/netif.h"

#define DEVICE1

//#define DEVICE2

#if defined(DEVICE1) && !defined(DEVICE2)
#define TOPIC1 "movement_detect"
#define TOPIC3 "temp_measure"
#define TOPIC4 "smoke_detect"
#define TOPIC6 "night_light"
#endif

#if defined(DEVICE2) && !defined(DEVICE1)
#define TOPIC2 "noise_detect"
#define TOPIC3 "temp_measure"
#define TOPIC4 "smoke_detect"
#define TOPIC5 "relax_music"
#endif

/*!
 * @brief Create and run example thread
 *
 * @param netif  netif which example should use
 */
void mqtt_freertos_run_thread(struct netif *netif);

#endif /* MQTT_FREERTOS_H */
