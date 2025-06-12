#include <sdkconfig.h>

#ifdef CONFIG_OT_COAP_SERVER

#include <esp_log.h>

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

static char TAG[] = "OtCoapSrv";

static void storedata_request_handler(void *p_context, otMessage *p_message,
                                      const otMessageInfo *p_message_info);

static void storedata_response_send(otMessage *p_request_message,
                                    const otMessageInfo *p_message_info);

static char server_message_buffer[COAP_MESSAGE_SIZE];
static uint16_t server_message_len = 0;


static otCoapResource m_storedata_resource = {
    .mUriPath = "tel",
    .mHandler = storedata_request_handler,
    .mContext = NULL,
    .mNext = NULL};

/**@brief Function for handling a CoAP-request.
 *  Just store and print the text message.
 *  If it is a confirmable message it sends an acknowledgement. */
static void storedata_request_handler(void *p_context, otMessage *p_message,
                                      const otMessageInfo *p_message_info)
{
    ESP_LOGI(TAG,"Handling request");
    otCoapCode messageCode = otCoapMessageGetCode(p_message);
    otCoapType messageType = otCoapMessageGetType(p_message);

    do
    {
        if (messageType != OT_COAP_TYPE_CONFIRMABLE &&
            messageType != OT_COAP_TYPE_NON_CONFIRMABLE)
        {
            break;
        }
        if (messageCode != OT_COAP_CODE_PUT)
        {
            break;
        }

        server_message_len = otMessageRead(p_message, otMessageGetOffset(p_message),
                                      server_message_buffer, COAP_MESSAGE_SIZE - 1);
        server_message_buffer[server_message_len] = '\0';
#ifdef CONFIG_OT_COAP_CLI
        otCliOutputFormat(server_message_buffer);
        otCliOutputFormat("\r\n");
#endif
        if (messageType == OT_COAP_TYPE_CONFIRMABLE)
        {
            storedata_response_send(p_message, p_message_info);
        }
    } while (false);
}

/**@brief Function for sending a response to a request. */
static void storedata_response_send(otMessage *p_request_message,
                                    const otMessageInfo *p_message_info)
{
    ESP_LOGI(TAG,"Sending response back");
    otError error = OT_ERROR_NO_BUFS;
    otMessage *p_response;
    otInstance *p_instance = esp_openthread_get_instance();

    // Create new message
    p_response = otCoapNewMessage(p_instance, NULL);
    if (p_response == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate message for CoAP Request");
        return;
    }

    do
    {
        // Add CoAP type and code to the message
        error = otCoapMessageInitResponse(p_response, p_request_message,
                                          OT_COAP_TYPE_ACKNOWLEDGMENT,
                                          OT_COAP_CODE_CHANGED);
        if (error != OT_ERROR_NONE)
        {
            break;
        }

        // Send the response
        error = otCoapSendResponse(p_instance, p_response, p_message_info);
    } while (false);

    if (error != OT_ERROR_NONE)
    {
        ESP_LOGE(TAG, "Failed to send store data response: %d", error);
        otMessageFree(p_response);
    }
}

/**@brief Function for initializing the CoAP modul and register the ressource. */
void OtCoapServer_init(void)
{
    ESP_LOGW(TAG,"OtCoapServer_init");
    otError error;
    otInstance *p_instance = esp_openthread_get_instance();
    m_storedata_resource.mContext = p_instance;

    do
    {
        error = otCoapStart(p_instance, OT_DEFAULT_COAP_PORT);
        if (error != OT_ERROR_NONE)
        {
            break;
        }

        otCoapAddResource(p_instance, &m_storedata_resource);
    } while (false);

    if (error != OT_ERROR_NONE)
    {
        ESP_LOGE(TAG, "coap_init error: %d", error);
    }
}

#endif
