#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* modem driver */
#include "HL7692/HL7692-Driver.h"

#define AFB_BINDING_VERSION 3
#include <afb/afb-binding.h>

/*
    Required file:
    1. rwserial.c and rwserial.h
    2. rwgpio.c and rwgpio.h
    3. HL7692-Driver.c and HL7692-Driver.h
    Compile syntax
    $ gcc -fPIC -shared modem-bindingv3.c HL7692/HL7692-Driver.c rwserial/rwserial.c rwgpio/rwgpio.c -o modem-bindingv3.so -Irwserial -Irwgpio $(pkg-config --cflags-only-I --libs afb-binding)
    Start Binder
    $ afb-binder --binding ./modem-bindingv3.so --port 3333
    make request from localhost
    $ afb-client -H localhost:3333/api
    make request from local network (started binder in ip : 192.168.0.10 with port 3333)
    $ afb-client -H ws://192.168.0.10:3333/api
*/

afb_event_t event_login, event_logout;

void login(afb_req_t req) {
    json_object *args, *user, *passwd;
    char *usr;

    args = afb_req_json(req);
    if (!json_object_object_get_ex(args, "user", &user) || !json_object_object_get_ex(args, "password", &passwd)) {
        AFB_REQ_ERROR(req, "login, bad request: %s", json_object_get_string(args));
        afb_req_reply(req, NULL, "bad-request", NULL);
    } else if (afb_req_context_get(req)) {
        AFB_REQ_ERROR(req, "login, bad state, logout first");
        afb_req_reply(req, NULL, "bad-state", NULL);
    } else if (strcmp(json_object_get_string(passwd), "please")) {
        AFB_REQ_ERROR(req, "login, unauthorized: %s", json_object_get_string(args));
        afb_req_reply(req, NULL, "unauthorized", NULL);
    } else {
        usr = strdup(json_object_get_string(user));
        AFB_REQ_NOTICE(req, "login user: %s", usr);
        afb_req_session_set_LOA(req, 1);
        afb_req_context_set(req, usr, free);
        afb_req_reply(req, NULL, NULL, NULL);
        afb_event_push(event_login, json_object_new_string(usr));
    }
}

void action(afb_req_t req) {
    json_object *args, *val;
    char *usr;

    args = afb_req_json(req);
    usr = afb_req_context_get(req);
    AFB_REQ_NOTICE(req, "action for user %s: %s", usr, json_object_get_string(args));
    if (json_object_object_get_ex(args, "subscribe", &val)) {
        if (json_object_get_boolean(val)) {
            AFB_REQ_NOTICE(req, "user %s subscribes to events", usr);
            afb_req_subscribe(req, event_login);
            afb_req_subscribe(req, event_logout);
        } else {
            AFB_REQ_NOTICE(req, "user %s unsubscribes to events", usr);
            afb_req_unsubscribe(req, event_login);
            afb_req_unsubscribe(req, event_logout);
        }
    }
    afb_req_reply(req, json_object_get(args), NULL, NULL);
}

void logout(afb_req_t req) {
    char *usr;

    usr = afb_req_context_get(req);
    AFB_REQ_NOTICE(req, "login user %s out", usr);
    afb_event_push(event_logout, json_object_new_string(usr));
    afb_req_session_set_LOA(req, 0);
    afb_req_context_clear(req);
    afb_req_reply(req, NULL, NULL, NULL);
}

void initModem(afb_req_t req) {
    char *pUsr;
    pUsr = afb_req_context_get(req);

    /* turn on module with GPIO */
    /* using lib gpiod */

    /* turn off echo */
    int ret = initHL7692();
    if (ret != 0) {
        afb_req_fail_f(req, "ERROR", "init modem failed with return code = %d", ret);
        return;
    }

    afb_req_success_f(req, NULL, "Modem Init OK");
}

void turnOnLedModem(afb_req_t req) {
    char *pUsr;
    pUsr = afb_req_context_get(req);

    /* main function */
    int ret = turnOnLed();
    if (ret != 0) {
        afb_req_fail_f(req, "ERROR", "turn on led failed with return code = %d", ret);
        return;
    } else
        afb_req_success_f(req, NULL, "Turn on modem led OK");
}

void turnOffLedModem(afb_req_t req) {
    char *pUsr;
    pUsr = afb_req_context_get(req);

    /* main function */
    int ret = turnOffLed();
    if (ret != 0) {
        afb_req_fail_f(req, "ERROR", "turn off led failed with return code = %d", ret);
        return;
    } else
        afb_req_success_f(req, NULL, "Turn off modem led OK");
}

void configModem(afb_req_t req) {
    char *pUsr;
    pUsr = afb_req_context_get(req);

    /* main function */
    int ret = configureNetwork();
    if (ret != 0) {
        afb_req_fail_f(req, "ERROR", "config modem failed with return code = %d", ret);
        return;
    } else
        afb_req_success_f(req, NULL, "config modem OK");
}

void connectModem(afb_req_t req) {
    char *pUsr;
    pUsr = afb_req_context_get(req);

    /* main function */
    int ret = connect();
    if (ret != 0) {
        afb_req_fail_f(req, "ERROR", "connect failed with return code = %d", ret);
        return;
    } else
        afb_req_success_f(req, NULL, "connect modem OK");
}

void disconnectModem(afb_req_t req) {
    char *pUsr;
    pUsr = afb_req_context_get(req);

    /* main function */
    int ret = disconnect();
    if (ret != 0) {
        afb_req_fail_f(req, "ERROR", "disconnect failed with return code = %d", ret);
        return;
    } else
        afb_req_success_f(req, NULL, "disconnect modem OK");
}

void sendApduModem(afb_req_t req) {
    json_object *args, *pSessionId, *pSendCmd;
    char *pUsr;

    args = afb_req_json(req);
    pUsr = afb_req_context_get(req);
    if (!json_object_object_get_ex(args, "session-id", &pSessionId) || !json_object_object_get_ex(args, "send-apdu", &pSendCmd)) {
        AFB_REQ_ERROR(req, "login, bad request: %s", json_object_get_string(args));
        afb_req_reply(req, NULL, "bad-request, please try {\"session-id\":\"0/1/2/3\",\"send-apdu\":\"your_apdu_command\"}", NULL);
    } else {
        /* get json data */
        int buf1 = json_object_get_int(pSessionId);
        char *pDataRecv = NULL;
        char *pBuf1 = strdup(json_object_get_string(pSendCmd));

        /* main function */
        int ret = sendRecvApdu(buf1, pBuf1, &pDataRecv);
        if (ret != 0) {
            afb_req_fail_f(req, "ERROR", "disconnect failed with return code = %d", ret);
        } else {
            afb_req_success_f(req, NULL, "%d char data received: %s", strlen(pDataRecv), pDataRecv);
        }
        afb_req_context_set(req, pBuf1, free);
        free(pDataRecv);
        pDataRecv = NULL;
    }
}

/* Pre-init */
int preinit(afb_api_t api) {
    AFB_API_NOTICE(api, "preinit");
    return 0;
}

/* int */
int init(afb_api_t api) {
    AFB_API_NOTICE(api, "init");
    event_login = afb_api_make_event(api, "login");
    event_logout = afb_api_make_event(api, "logout");
    if (afb_event_is_valid(event_login) && afb_event_is_valid(event_logout))
        return 0;
    AFB_API_ERROR(api, "Can't create events");
    return -1;
}

const afb_verb_t verbs[] = {
    {.verb = "initModem", .callback = initModem},
    {.verb = "turnOnLedModem", .callback = turnOnLedModem},
    {.verb = "turnOffLedModem", .callback = turnOffLedModem},
    {.verb = "configureModem", .callback = configModem},
    {.verb = "connectModem", .callback = connectModem},
    {.verb = "disconnectModem", .callback = disconnectModem},
    {.verb = "sendApduModem", .callback = sendApduModem},
    {.verb = "login", .callback = login},
    {.verb = "action", .callback = action, .session = AFB_SESSION_LOA_1},
    {.verb = "logout", .callback = logout, .session = AFB_SESSION_LOA_1},
    {.verb = NULL}};

const afb_binding_t afbBindingExport = {
    .api = "modembinding",
    .specification = NULL,
    .verbs = verbs,
    .preinit = preinit,
    .init = init,
    .noconcurrency = 0};
