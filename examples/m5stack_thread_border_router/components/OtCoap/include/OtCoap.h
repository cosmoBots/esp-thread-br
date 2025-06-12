#pragma once


#ifdef CONFIG_OT_COAP_SERVER
void OtCoapServer_init(void);
#define OT_COAP_ACTIVE
#endif

#ifdef CONFIG_OT_COAP_CLIENT
void OtCoapClient_init(void);
#ifndef OT_COAP_ACTIVE
#define OT_COAP_ACTIVE
#endif
#endif

#ifdef OT_COAP_ACTIVE
#define COAP_MESSAGE_SIZE 30
#endif