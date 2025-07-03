/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <MQTT.h>
#include "board.h"
#include "fsl_silicon_id.h"

#include "lwip/opt.h"
#include "lwip/api.h"
#include "Drivers/mqtt.h"
#include "lwip/tcpip.h"

#include <stdio.h>
#include <stdlib.h>
#include "Drivers/LED.h"
#include "Drivers/GPIO.h"
#include "Drivers/BUTTON.h"

/*! @brief MQTT server host name or IP address. */
#ifndef EXAMPLE_MQTT_SERVER_HOST
#define EXAMPLE_MQTT_SERVER_HOST "broker.hivemq.com"
#endif

/*! @brief MQTT server port number. */
#ifndef EXAMPLE_MQTT_SERVER_PORT
#define EXAMPLE_MQTT_SERVER_PORT 1883
#endif

/*! @brief Stack size of the temporary lwIP initialization thread. */
#define INIT_THREAD_STACKSIZE 1024

/*! @brief Priority of the temporary lwIP initialization thread. */
#define INIT_THREAD_PRIO DEFAULT_THREAD_PRIO

/*! @brief Stack size of the temporary initialization thread. */
#define APP_THREAD_STACKSIZE 1024

/*! @brief Priority of the temporary initialization thread. */
#define APP_THREAD_PRIO DEFAULT_THREAD_PRIO

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void connect_to_mqtt(void *ctx);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief MQTT client data. */
static mqtt_client_t *mqtt_client;

/*! @brief MQTT client ID string. */
static char client_id[(SILICONID_MAX_LENGTH * 2) + 5];

/*! @brief MQTT client information. */
static const struct mqtt_connect_client_info_t mqtt_client_info = {
    .client_id   = (const char *)&client_id[0],
    .client_user = NULL,
    .client_pass = NULL,
    .keep_alive  = 100,
    .will_topic  = NULL,
    .will_msg    = NULL,
    .will_qos    = 0,
    .will_retain = 0,
#if LWIP_ALTCP && LWIP_ALTCP_TLS
    .tls_config = NULL,
#endif
};

/*! @brief MQTT broker IP address. */
static ip_addr_t mqtt_addr;

/*! @brief Indicates connection to MQTT broker. */
static volatile bool connected = false;

uint8_t received_topic;

uint8_t r,g,b;

uint8_t temp = 20;
/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Called when subscription request finishes.
 */
static void mqtt_topic_subscribed_cb(void *arg, err_t err)
{
    const char *topic = (const char *)arg;

    if (err == ERR_OK)
    {
        PRINTF("Subscribed to the topic \"%s\".\r\n", topic);
    }
    else
    {
        PRINTF("Failed to subscribe to the topic \"%s\": %d.\r\n", topic, err);
    }
}

static void check_topic(const char *topic){
	uint8_t i = 0;
	received_topic = 0;

	while(topic[i] != 0){
#if defined(DEVICE1) && !defined(DEVICE2)
		if(topic[i] == TOPIC4[i]){
			received_topic = 4;
		}
		else if(topic[i] == TOPIC6[i]){
			received_topic = 6;
		}
#endif
#if defined(DEVICE2) && !defined(DEVICE1)
		if(topic[i] == TOPIC3[i]){
			received_topic = 3;
		}
		else if(topic[i] == TOPIC5[i]){
			received_topic = 5;
		}
#endif
		i++;
	}
}

#if defined(DEVICE1) && !defined(DEVICE2)
void manage_smoke_topic(const uint8_t *data){
	if (strncmp(data, "NO_SMOKE", 8) == 0) {
//		LED_Set(LED_RED_COLOUR);
		GPIO_PIN_Set(GPIO10);
	}
	else{
//		LED_Set(LED_GREEN_COLOUR);
		GPIO_PIN_Clear(GPIO10);
	}
}

void manage_night_light(const uint8_t *data){
	r = g = b = 0;

	char buffer[32];
	strncpy(buffer, (char *)data, sizeof(buffer) - 1);
	buffer[sizeof(buffer) - 1] = '\0';

	if (strncmp(buffer, "rgb(", 4) != 0) {
		return;
	}

	char *ptr = buffer + 4;
	int values[3] = {0, 0, 0};
	int current_value = 0;
	int index = 0;

	while (*ptr && index < 3) {
		if (*ptr == ' ' || *ptr == ',') {
			ptr++;
			continue;
		}

		if (*ptr == ')') {
			break;
		}

		if (*ptr >= '0' && *ptr <= '9') {
			current_value = current_value * 10 + (*ptr - '0');
			ptr++;
		} else {
			return;
		}

		if (*ptr == ',' || *ptr == ' ' || *ptr == ')') {
			values[index] = current_value;
			current_value = 0;
			index++;
		}
	}

	(values[0] == 255) ? (r = LOGIC_LED_ON) : (r = LOGIC_LED_OFF);
	(values[1] == 255) ? (g = LOGIC_LED_ON) : (g = LOGIC_LED_OFF);
	(values[2] == 255) ? (b = LOGIC_LED_ON) : (b = LOGIC_LED_OFF);

	LED_Set(r, g, b);
}
#endif

#if defined(DEVICE2) && !defined(DEVICE1)
uint8_t stringToInt(const uint8_t *str) {
    uint8_t result = 0;

    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }

    return result;
}

void manage_temp_topic(const uint8_t *data){
	if(stringToInt(data) >= 28){
		GPIO_PIN_Clear(GPIO10);
	}
	else if(stringToInt(data) < 28){
		GPIO_PIN_Set(GPIO10);
	}

}
void manage_music_topic(const uint8_t *data){
	if (strncmp(data, "OFF", 2) == 0) {
		LED_Set(LED_RED_COLOUR);
//		GPIO_PIN_Clear(GPIO1);
	}
	else{
		LED_Set(LED_GREEN_COLOUR);
//		GPIO_PIN_Set(GPIO1);
	}
}
#endif

/*!
 * @brief Called when there is a message on a subscribed topic.
 */
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
    LWIP_UNUSED_ARG(arg);

    PRINTF("Received %u bytes from the topic \"%s\": \"", tot_len, topic);
    check_topic(topic);
}

/*!
 * @brief Called when recieved incoming published message fragment.
 */
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
    int i;

    LWIP_UNUSED_ARG(arg);

    for (i = 0; i < len; i++)
    {
        if (isprint(data[i]))
        {
            PRINTF("%c", (char)data[i]);
        }
        else
        {
            PRINTF("\\x%02x", data[i]);
        }
    }

#if defined(DEVICE1) && !defined(DEVICE2)
        if(received_topic == 4){
        	manage_smoke_topic(data);
        }
        else if(received_topic == 6){
        	manage_night_light(data);
        }
#endif
#if defined(DEVICE2) && !defined(DEVICE1)
        if(received_topic == 3){
        	manage_temp_topic(data);
        }
        else if(received_topic == 5){
        	manage_music_topic(data);
        }
#endif

    if (flags & MQTT_DATA_FLAG_LAST)
    {
        PRINTF("\"\r\n");
    }
}

/*!
 * @brief Subscribe to MQTT topics.
 */
static void mqtt_subscribe_topics(mqtt_client_t *client)
{
#if defined(DEVICE1) && !defined(DEVICE2)
    static const char *topics[] = {"smoke_detect/#", "night_light/#"};
#endif
#if defined(DEVICE2) && !defined(DEVICE1)
    static const char *topics[] = {"temp_measure/#", "relax_music/#"};
#endif

    int qos[]                   = {0, 0};
    err_t err;
    int i;

    mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb,
                            LWIP_CONST_CAST(void *, &mqtt_client_info));

    for (i = 0; i < ARRAY_SIZE(topics); i++)
    {
        err = mqtt_subscribe(client, topics[i], qos[i], mqtt_topic_subscribed_cb, LWIP_CONST_CAST(void *, topics[i]));

        if (err == ERR_OK)
        {
            PRINTF("Subscribing to the topic \"%s\" with QoS %d...\r\n", topics[i], qos[i]);
        }
        else
        {
            PRINTF("Failed to subscribe to the topic \"%s\" with QoS %d: %d.\r\n", topics[i], qos[i], err);
        }
    }
}

/*!
 * @brief Called when connection state changes.
 */
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
    const struct mqtt_connect_client_info_t *client_info = (const struct mqtt_connect_client_info_t *)arg;

    connected = (status == MQTT_CONNECT_ACCEPTED);

    switch (status)
    {
        case MQTT_CONNECT_ACCEPTED:
            PRINTF("MQTT client \"%s\" connected.\r\n", client_info->client_id);
            mqtt_subscribe_topics(client);
            break;

        case MQTT_CONNECT_DISCONNECTED:
            PRINTF("MQTT client \"%s\" not connected.\r\n", client_info->client_id);
            /* Try to reconnect 1 second later */
            sys_timeout(1000, connect_to_mqtt, NULL);
            break;

        case MQTT_CONNECT_TIMEOUT:
            PRINTF("MQTT client \"%s\" connection timeout.\r\n", client_info->client_id);
            /* Try again 1 second later */
            sys_timeout(1000, connect_to_mqtt, NULL);
            break;

        case MQTT_CONNECT_REFUSED_PROTOCOL_VERSION:
        case MQTT_CONNECT_REFUSED_IDENTIFIER:
        case MQTT_CONNECT_REFUSED_SERVER:
        case MQTT_CONNECT_REFUSED_USERNAME_PASS:
        case MQTT_CONNECT_REFUSED_NOT_AUTHORIZED_:
            PRINTF("MQTT client \"%s\" connection refused: %d.\r\n", client_info->client_id, (int)status);
            /* Try again 10 seconds later */
            sys_timeout(10000, connect_to_mqtt, NULL);
            break;

        default:
            PRINTF("MQTT client \"%s\" connection status: %d.\r\n", client_info->client_id, (int)status);
            /* Try again 10 seconds later */
            sys_timeout(10000, connect_to_mqtt, NULL);
            break;
    }
}

/*!
 * @brief Starts connecting to MQTT broker. To be called on tcpip_thread.
 */
static void connect_to_mqtt(void *ctx)
{
    LWIP_UNUSED_ARG(ctx);

    PRINTF("Connecting to MQTT broker at %s...\r\n", ipaddr_ntoa(&mqtt_addr));

    mqtt_client_connect(mqtt_client, &mqtt_addr, EXAMPLE_MQTT_SERVER_PORT, mqtt_connection_cb,
                        LWIP_CONST_CAST(void *, &mqtt_client_info), &mqtt_client_info);
}

/*!
 * @brief Called when publish request finishes.
 */
static void mqtt_message_published_cb(void *arg, err_t err)
{
    const char *topic = (const char *)arg;

    if (err == ERR_OK)
    {
        PRINTF("Published to the topic \"%s\".\r\n", topic);
    }
    else
    {
        PRINTF("Failed to publish to the topic \"%s\": %d.\r\n", topic, err);
    }
}

/*!
 * @brief Publishes a message. To be called on tcpip_thread.
 */
#if defined(DEVICE1) && !defined(DEVICE2)
static void publish_message1(void *ctx)
{
	static const char *topic1   = TOPIC1;
	static const char *message1 = "Movimiento detectado";

    LWIP_UNUSED_ARG(ctx);

    PRINTF("Going to publish to the topic \"%s\"...\r\n", topic1);

    mqtt_publish(mqtt_client, topic1, message1, strlen(message1), 1, 0, mqtt_message_published_cb, (void *)topic1);
}

static void publish_message2(void *ctx)
{
	static const char *topic2   = TOPIC3;
	char message2[] = "22";
	message2[0] = (temp / 10) + 0x30;
	message2[1] = (temp % 10) + 0x30;

    LWIP_UNUSED_ARG(ctx);

    PRINTF("Going to publish to the topic \"%s\"...\r\n", topic2);

    mqtt_publish(mqtt_client, topic2, message2, strlen(message2), 1, 0, mqtt_message_published_cb, (void *)topic2);
}
#endif

#if defined(DEVICE2) && !defined(DEVICE1)
static void publish_message1(void *ctx)
{
	static const char *topic1   = TOPIC2;
	static const char *message1 = "Ruido detectado";

    LWIP_UNUSED_ARG(ctx);

    PRINTF("Going to publish to the topic \"%s\"...\r\n", topic1);

    mqtt_publish(mqtt_client, topic1, message1, strlen(message1), 1, 0, mqtt_message_published_cb, (void *)topic1);
}

static void publish_message2(void *ctx)
{
	static const char *topic2   = TOPIC4;
	static const char *message2 = "SMOKE";
	static const char *message3 = "NO_SMOKE";

    LWIP_UNUSED_ARG(ctx);

    PRINTF("Going to publish to the topic \"%s\"...\r\n", topic2);

    mqtt_publish(mqtt_client, topic2, message2, strlen(message2), 1, 0, mqtt_message_published_cb, (void *)topic2);
}

static void publish_message3(void *ctx)
{
	static const char *topic2   = TOPIC4;
	static const char *message3 = "NO_SMOKE";

    LWIP_UNUSED_ARG(ctx);

    PRINTF("Going to publish to the topic \"%s\"...\r\n", topic2);

    mqtt_publish(mqtt_client, topic2, message3, strlen(message3), 1, 0, mqtt_message_published_cb, (void *)topic2);
}
#endif

/*!
 * @brief Application thread.
 */
static void app_thread(void *arg)
{
    struct netif *netif = (struct netif *)arg;
    err_t err;
    int i = 1;

    PRINTF("\r\nIPv4 Address     : %s\r\n", ipaddr_ntoa(&netif->ip_addr));
    PRINTF("IPv4 Subnet mask : %s\r\n", ipaddr_ntoa(&netif->netmask));
    PRINTF("IPv4 Gateway     : %s\r\n\r\n", ipaddr_ntoa(&netif->gw));

    /*
     * Check if we have an IP address or host name string configured.
     * Could just call netconn_gethostbyname() on both IP address or host name,
     * but we want to print some info if goint to resolve it.
     */
    if (ipaddr_aton(EXAMPLE_MQTT_SERVER_HOST, &mqtt_addr) && IP_IS_V4(&mqtt_addr))
    {
        /* Already an IP address */
        err = ERR_OK;
    }
    else
    {
        /* Resolve MQTT broker's host name to an IP address */
        PRINTF("Resolving \"%s\"...\r\n", EXAMPLE_MQTT_SERVER_HOST);
        err = netconn_gethostbyname(EXAMPLE_MQTT_SERVER_HOST, &mqtt_addr);
    }

    if (err == ERR_OK)
    {
        /* Start connecting to MQTT broker from tcpip_thread */
        err = tcpip_callback(connect_to_mqtt, NULL);
        if (err != ERR_OK)
        {
            PRINTF("Failed to invoke broker connection on the tcpip_thread: %d.\r\n", err);
        }
    }
    else
    {
        PRINTF("Failed to obtain IP address: %d.\r\n", err);
    }

    /* Publish some messages */
#if defined(DEVICE1) && !defined(DEVICE2)
    while(1){
		if(connected){
			if (BUTTON_IsPressed(BTN_GPIO_19)){
				err = tcpip_callback(publish_message1, NULL);
				if (err != ERR_OK)
				{
					PRINTF("Failed to invoke publishing of a message on the tcpip_thread: %d.\r\n", err);
				}
				sys_msleep(500);
			}
			else if (BUTTON_IsPressed(BTN_GPIO_7)){
				err = tcpip_callback(publish_message2, NULL);
				if (err != ERR_OK)
				{
					PRINTF("Failed to invoke publishing of a message on the tcpip_thread: %d.\r\n", err);
				}
				(temp == 33) ? (temp = 23) : (temp++);
				sys_msleep(500);
			}
		}
    }
#endif
#if defined(DEVICE2) && !defined(DEVICE1)
    while(1){
    	if(connected){
			if (BUTTON_IsPressed(BTN_GPIO_19)){
				err = tcpip_callback(publish_message1, NULL);
				if (err != ERR_OK)
				{
					PRINTF("Failed to invoke publishing of a message on the tcpip_thread: %d.\r\n", err);
				}
				sys_msleep(500);
			}
			else if (BUTTON_IsPressed(BTN_GPIO_7)){
				if(i == 1){
					err = tcpip_callback(publish_message2, NULL);
					if (err != ERR_OK)
					{
						PRINTF("Failed to invoke publishing of a message on the tcpip_thread: %d.\r\n", err);
					}
					i = 0;
				}
				else{
					err = tcpip_callback(publish_message3, NULL);
					if (err != ERR_OK)
					{
						PRINTF("Failed to invoke publishing of a message on the tcpip_thread: %d.\r\n", err);
					}
					i = 1;
				}
				sys_msleep(500);
			}
    	}
    }
#endif

    vTaskDelete(NULL);
}

static void button_pressed_callback(void)
{
    if (connected)
    {
        err_t err = tcpip_callback(publish_message1, NULL);
        if (err != ERR_OK)
        {
            PRINTF("Failed to invoke publishing of temperature message: %d.\r\n", err);
        }
    }
    else
    {
        PRINTF("Cannot publish: Not connected to MQTT broker.\r\n");
    }
}

static void generate_client_id(void)
{
    uint8_t silicon_id[SILICONID_MAX_LENGTH];
    const char *hex = "0123456789abcdef";
    status_t status;
    uint32_t id_len = sizeof(silicon_id);
    int idx         = 0;
    int i;
    bool id_is_zero = true;

    /* Get unique ID of SoC */
    status = SILICONID_GetID(&silicon_id[0], &id_len);
    assert(status == kStatus_Success);
    assert(id_len > 0U);
    (void)status;

    /* Covert unique ID to client ID string in form: nxp_hex-unique-id */

    /* Check if client_id can accomodate prefix, id and terminator */
    assert(sizeof(client_id) >= (5U + (2U * id_len)));

    /* Fill in prefix */
    client_id[idx++] = 'n';
    client_id[idx++] = 'x';
    client_id[idx++] = 'p';
    client_id[idx++] = '_';

    /* Append unique ID */
    for (i = (int)id_len - 1; i >= 0; i--)
    {
        uint8_t value    = silicon_id[i];
        client_id[idx++] = hex[value >> 4];
        client_id[idx++] = hex[value & 0xFU];

        if (value != 0)
        {
            id_is_zero = false;
        }
    }

    /* Terminate string */
    client_id[idx] = '\0';

    if (id_is_zero)
    {
        PRINTF(
            "WARNING: MQTT client id is zero. (%s)"
#ifdef OCOTP
            " This might be caused by blank OTP memory."
#endif
            "\r\n",
            client_id);
    }
}

/*!
 * @brief Create and run example thread
 *
 * @param netif  netif which example should use
 */
void mqtt_freertos_run_thread(struct netif *netif)
{
    LOCK_TCPIP_CORE();
    mqtt_client = mqtt_client_new();
    UNLOCK_TCPIP_CORE();
    if (mqtt_client == NULL)
    {
        PRINTF("mqtt_client_new() failed.\r\n");
        while (1)
        {
        }
    }
    GPIO_PIN_Init();

    LED_Init();
    LED_Set(LED_WHITE_COLOUR);

    generate_client_id();

    if (sys_thread_new("app_task", app_thread, netif, APP_THREAD_STACKSIZE, APP_THREAD_PRIO) == NULL)
    {
        LWIP_ASSERT("mqtt_freertos_start_thread(): Task creation failed.", 0);
    }
}

