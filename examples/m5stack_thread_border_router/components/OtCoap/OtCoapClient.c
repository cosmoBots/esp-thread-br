#include <sdkconfig.h>

#ifdef CONFIG_OT_COAP_CLIENT

#include <esp_log.h>

#include <iot_button.h>
#include <button_gpio.h>

#include "esp_openthread.h"
#include "esp_openthread_cli.h"
#include "esp_openthread_lock.h"
#include "esp_openthread_netif_glue.h"
#include "esp_openthread_types.h"

#include "openthread/instance.h"
#include "openthread/logging.h"
#include "openthread/tasklet.h"

#include <openthread/coap.h>

#ifdef CONFIG_OT_COAP_CLI
#include "openthread/cli.h"
#endif

#include "OtCoap.h"
#include "OtCoapDemo.h"

static char TAG[] = "OtCoapClient";

static void coap_send_data_request(void);
static void coap_send_data_response_handler(void *p_context, otMessage *p_message,
                                            const otMessageInfo *p_message_info, otError result);

//////// BEGIN BUTTON COMPONENT

// create gpio button
const button_config_t btn_cfg = {0};
const button_gpio_config_t btn_gpio_cfg = {
    .gpio_num = 9,
    .active_level = 0,
};
button_handle_t gpio_btn = NULL;

static void button_single_click_cb(void *arg, void *usr_data)
{
    ESP_LOGI(TAG, "BUTTON_SINGLE_CLICK");
    coap_send_data_request();
}

//////// END BUTTON COMPONENT

/**@brief Function for sending a data request. */
static void coap_send_data_request(void)
{
    otError error = OT_ERROR_NONE;
    otMessage *myMessage;
    otMessageInfo myMessageInfo;
    otInstance *p_instance = esp_openthread_get_instance();

    char myPayload[30];
    const char *serverIpAddr = OtCoapDemo_fill_request_payload(myPayload);
    do
    {
        // Create a new message
        myMessage = otCoapNewMessage(p_instance, NULL);
        if (myMessage == NULL)
        {
            ESP_LOGE(TAG, "Failed to allocate message for CoAP Request");
            return;
        }
        // Set CoAP type and code in the message
        otCoapMessageInit(myMessage, OT_COAP_TYPE_CONFIRMABLE, OT_COAP_CODE_PUT);

        // Add the URI path option in the message
        error = otCoapMessageAppendUriPathOptions(myMessage, "storedata");
        if (error != OT_ERROR_NONE)
        {
            break;
        }

        // Add the content format option in the message
        error = otCoapMessageAppendContentFormatOption(myMessage, OT_COAP_OPTION_CONTENT_FORMAT_JSON);
        if (error != OT_ERROR_NONE)
        {
            break;
        }

        // Set the payload delimiter in the message
        error = otCoapMessageSetPayloadMarker(myMessage);
        if (error != OT_ERROR_NONE)
        {
            break;
        }

        /// Append the payload to the message
        error = otMessageAppend(myMessage, myPayload, strlen(myPayload));
        if (error != OT_ERROR_NONE)
        {
            break;
        }

        // Set the UDP-destination port of the CoAP-server
        memset(&myMessageInfo, 0, sizeof(myMessageInfo));
        myMessageInfo.mPeerPort = OT_DEFAULT_COAP_PORT;

        // Set the IP-address of the CoAP-server
        error = otIp6AddressFromString(serverIpAddr, &myMessageInfo.mPeerAddr);
/*         ESP_LOGI(TAG,"%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
            myMessageInfo.mPeerAddr.mFields.m8[0],
            myMessageInfo.mPeerAddr.mFields.m8[1],
            myMessageInfo.mPeerAddr.mFields.m8[2],
            myMessageInfo.mPeerAddr.mFields.m8[3],
            myMessageInfo.mPeerAddr.mFields.m8[4],
            myMessageInfo.mPeerAddr.mFields.m8[5],
            myMessageInfo.mPeerAddr.mFields.m8[6],
            myMessageInfo.mPeerAddr.mFields.m8[7],
            myMessageInfo.mPeerAddr.mFields.m8[8],
            myMessageInfo.mPeerAddr.mFields.m8[9],
            myMessageInfo.mPeerAddr.mFields.m8[10],
            myMessageInfo.mPeerAddr.mFields.m8[11],
            myMessageInfo.mPeerAddr.mFields.m8[12],
            myMessageInfo.mPeerAddr.mFields.m8[13],
            myMessageInfo.mPeerAddr.mFields.m8[14],
            myMessageInfo.mPeerAddr.mFields.m8[15]);   */      
        if (error != OT_ERROR_NONE)
        {
            break;
        }

        // Send CoAP-request
        error = otCoapSendRequest(p_instance, myMessage, &myMessageInfo, coap_send_data_response_handler, NULL);
    } while (false);

    if (error != OT_ERROR_NONE)
    {
        ESP_LOGE(TAG, "Failed to send CoAP Request: %d", error);
        otMessageFree(myMessage);
    }
    else
    {
        otCliOutputFormat("CoAP data send.\r\n\0");
    }
}

/**@brief Function for handling the response of the request. */
static void coap_send_data_response_handler(void *p_context, otMessage *p_message,
                                            const otMessageInfo *p_message_info, otError result)
{
    if (result == OT_ERROR_NONE)
    {
        otCliOutputFormat("Delivery confirmed.\r\n\0");
    }
    else
    {
        ESP_LOGE(TAG, "Delivery not confirmed: %d", result);
    }
}

/**@brief Function for initializing the CoAP modul and register the ressource. */
void OtCoapClient_init(void)
{
    esp_err_t ret = iot_button_new_gpio_device(&btn_cfg, &btn_gpio_cfg, &gpio_btn);
    if (ret != ESP_OK || NULL == gpio_btn)
    {
        ESP_LOGE(TAG, "Button create failed");
    }

    iot_button_register_cb(gpio_btn, BUTTON_SINGLE_CLICK, NULL, button_single_click_cb, NULL);

    otError error;
    otInstance *p_instance = esp_openthread_get_instance();
    do
    {
        error = otCoapStart(p_instance, OT_DEFAULT_COAP_PORT);
        if (error != OT_ERROR_NONE)
        {
            break;
        }
    } while (false);

    if (error != OT_ERROR_NONE)
    {
        ESP_LOGE(TAG, "coap_init error: %d", error);
    }
    else
    {
        OtCoapDemo_init(p_instance);
    }
}

#endif
