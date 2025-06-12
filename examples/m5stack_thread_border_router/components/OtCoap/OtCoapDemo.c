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

#include <OtCoap.h>

#ifdef OT_COAP_ACTIVE

static char TAG[] = "OtCoapDemo";

//////// UNIQUE IDENTIFIERS TO THE TEST ///////////
//BR1 MAC: a63816dfcddd05f2
//OT1 MAC: 66bdf392366bd2c7

#define OT_DEMO_BR1_INDEX 0
#define OT_DEMO_OT1_INDEX 1

#define OT_DEMO_COAP_SERVER_INDEX 0

#define OT_DEMO_BR1_IPV6_UNIQUE_ADDR "fe80:0:0:0:a438:16df:cddd:5f2"
#define OT_DEMO_OT1_IPV6_UNIQUE_ADDR "fe80:0:0:0:64bd:f392:366b:d2c7"

#define OT_DEMO_BR1_MAC {0xa6, 0x38, 0x16, 0xdf, 0xcd, 0xdd, 0x05, 0xf2}
#define OT_DEMO_OT1_MAC {0x66, 0xbd, 0xf3, 0x92, 0x36, 0x6b, 0xd2, 0xc7}

const uint8_t demo_mac[][OT_EXT_ADDRESS_SIZE] = {
    OT_DEMO_BR1_MAC,
    OT_DEMO_OT1_MAC,
};

const char *demo_mac_names[] = {
    "BR1",
    "OT1",
};

const char *unique_addresses[] = {
    OT_DEMO_BR1_IPV6_UNIQUE_ADDR,
    OT_DEMO_OT1_IPV6_UNIQUE_ADDR
};

static int my_own_index = -1;
static int counter = 0;

const char *OtCoapDemo_fill_request_payload(char *payload)
{
    sprintf(payload,"%d:%s:%d", my_own_index, demo_mac_names[my_own_index],counter++);
    return unique_addresses[OT_DEMO_COAP_SERVER_INDEX];
}

void OtCoapDemo_init(otInstance *p_instance)
{
    int bees_len = sizeof(demo_mac) / OT_EXT_ADDRESS_SIZE;
    const otExtAddress *extAddr = otLinkGetExtendedAddress(p_instance);
    uint8_t *this_index = (uint8_t *)demo_mac;
    for (int i = 0; i < bees_len; i++)
    {
        ESP_LOGI(TAG,"Testing the mac at address %p", this_index);
        if (memcmp(this_index, extAddr->m8, OT_EXT_ADDRESS_SIZE) == 0)
        {
            my_own_index = i;
            ESP_LOGI(TAG,"This is my own address %d: %s",i, demo_mac_names[my_own_index]);
        }
        this_index += OT_EXT_ADDRESS_SIZE;
    }
}

#endif