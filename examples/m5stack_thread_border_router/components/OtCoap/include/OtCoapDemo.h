#pragma once

#include <stdbool.h>

#include <OtCoap.h>

#ifdef OT_COAP_ACTIVE

#include "esp_openthread.h"

const char *OtCoapDemo_fill_request_payload(const char *payload);
void OtCoapDemo_init(otInstance *p_instance);

#endif