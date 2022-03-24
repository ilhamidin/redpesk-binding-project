#define _GNU_SOURCE
#include <json-c/json.h>
#include <stdio.h>
#include <string.h>

// #include "wrap-json.h"
// gpio header
#include "gpio/rwgpio.h"

#define AFB_BINDING_VERSION 3
#include <afb/afb-binding.h>

/* COMPILE WITH COMMAND: gcc -fPIC -shared led-1-bindingv3.c -o led-1.so $(pkg-config --cflags-only-I afb-binding) */
/* COMPILE WITH gpio lib Command: gcc -fPIC -shared try-1.c -o try-1.so $(pkg-config --cflags --libs afb-binding) -lgpiod */
/* COMPILE without gpio lib (remove dependency on not-in-repo lib): gcc -fPIC -shared led-bindingv3.c gpio/rwgpio.c -o led-bindingv3.so $(pkg-config --cflags --libs afb-binding) -lgpiod */

static const char devicePath[] = "/dev/gpiochip0";

static const char API_NAME[] = "led-1";
static const char SUBSCRIBTION_VERB[] = "subscribe";
static const char COMMAND_TURN_ON[] = "turnOn";
static const char COMMAND_TURN_OFF[] = "turnOff";

static const char wrongparam[] = "wrong param !!!!";
static const char keynotfound[] = "Key \"light\" is not found !!!!";
static const char keynotstring[] = "Key is not string !!!!";

static afb_event_t event_lighton;
static afb_event_t event_lightoff;

static const char EVENT_SUBS_LIGHT_ON[] = "eventLightOn";
static const char EVENT_SUBS_LIGHT_OFF[] = "eventLightOff";

static const char SUBSCRIBED[] = "subscribed";

static const char LIGHT_ON_EVT_RECEIVED_PUSH[] = "LIGHT_ON_EVT_RECEIVED";
static const char LIGHT_OFF_EVT_RECEIVED_PUSH[] = "LIGHT_OFF_EVT_RECEIVED";

typedef struct HW_STATUS {
    int status;        // 0 means OK, 1 means not OK
    int dataReturned;  // if the status is 0, we put the data we read from the sensor to here
} HWStatus;

#define HW_STATUS_OK 0
#define HW_STATUS_NOK 1

HWStatus HW_HANDLER_turnLedOn() {
    HWStatus hwStatus;
    hwStatus.status = HW_STATUS_NOK;
    // Real Handling for hardware goes into here
    // For instance, we read a value from the sensor and we get 12345. So we returns 12345 from this method

    if ( writeToPin(devicePath, 16, 1) )
    {
        //Led successfully turned on
        hwStatus.status = HW_STATUS_OK;
        hwStatus.dataReturned = 12345;
    }
     
    return hwStatus;
}

HWStatus HW_HANDLER_turnLedOff() {
    HWStatus hwStatus;
    hwStatus.status = HW_STATUS_NOK;
    // Real Handling for hardware goes into here
    // For instance, we read a value from the sensor and we get 67890. So we returns 67890 from this method

    if ( writeToPin(devicePath, 16, 0) )
    {
        //Led successfully turned off
        hwStatus.status = HW_STATUS_OK;
        hwStatus.dataReturned = 67890;
    }
     
    return hwStatus;
}

void turnLedOn(afb_req_t req) {
    int listener;
    HWStatus dataFromSensor;

    // Reply OK.
    afb_req_success(req, NULL, COMMAND_TURN_ON);
    // In real world cases, when the user trigger this method we can validate it first. i.e check the triggering param, etc etc.
    // If the validation is correct, we reply OK.
    // Then we trigger the hardware/sensor, Get the data returned by the sensor and push it to the client and also push the EVENT.

    // Call Handling for hardware
    dataFromSensor = HW_HANDLER_turnLedOn();
    if (dataFromSensor.status == HW_STATUS_OK) {
        // PUSH EVENT AFTER LED SUCCESSFULLY TURNED ON
        listener = afb_event_push(event_lighton, json_object_new_int(dataFromSensor.dataReturned));
    }
}

void turnLedOff(afb_req_t req) {
    int listener;
    HWStatus dataFromSensor;

    // Reply OK.
    afb_req_success(req, NULL, COMMAND_TURN_OFF);
    // In real world cases, when the user trigger this method we can validate it first. i.e check the triggering param, etc etc.
    // If the validation is correct, we reply OK.
    // Then we trigger the hardware/sensor, Get the data returned by the sensor and push it to the client and also push the EVENT.

    // Call Handling for hardware
    dataFromSensor = HW_HANDLER_turnLedOff();
    if (dataFromSensor.status == HW_STATUS_OK) {
        // PUSH EVENT AFTER LED SUCCESSFULLY TURNED OFF
        listener = afb_event_push(event_lightoff, json_object_new_int(dataFromSensor.dataReturned));
    }
}

void subscribe(afb_req_t req) {
    AFB_REQ_DEBUG(req, "Called Subscribe");

    json_object *args = afb_req_json(req);
    json_object *res = json_object_new_object();
    json_object *tmpJ;
    char *currentEvent;

    // Only allow EVENT_SUBS_LIGHT_ON and EVENT_SUBS_LIGHT_OFF
    // For now we only interested in the json's argument. We ignore its key. i.e {argument : "key"}.
    // Argument must be either EVENT_SUBS_LIGHT_ON or EVENT_SUBS_LIGHT_OFF

    if (json_object_object_get_ex(args, EVENT_SUBS_LIGHT_ON, &tmpJ)) {
        event_lighton = afb_api_make_event(req->api, LIGHT_ON_EVT_RECEIVED_PUSH);
        if (afb_req_subscribe(req, event_lighton) < 0) {
            afb_req_fail(req, "ERROR", "Failed to subscribe event light On");
            return;
        }
        currentEvent = EVENT_SUBS_LIGHT_ON;
    }

    else if (json_object_object_get_ex(args, EVENT_SUBS_LIGHT_OFF, &tmpJ)) {
        event_lightoff = afb_api_make_event(req->api, LIGHT_OFF_EVT_RECEIVED_PUSH);
        if (afb_req_subscribe(req, event_lightoff) < 0) {
            afb_req_fail(req, "ERROR", "Failed to subscribe event light Off");
            return;
        }
        currentEvent = EVENT_SUBS_LIGHT_OFF;
    }

    else {
        afb_req_fail_f(req, "ERROR", "Event name must be '%s' or '%s'", EVENT_SUBS_LIGHT_ON, EVENT_SUBS_LIGHT_OFF);
        return;
    }

    // Subscription OK. Return Success to caller
    json_object_object_add(res, currentEvent, json_object_new_string(SUBSCRIBED));
    afb_req_success(req, res, NULL);
}

const afb_verb_t verbs[] = {
    {.verb = SUBSCRIBTION_VERB, .callback = subscribe},
    {.verb = COMMAND_TURN_ON, .callback = turnLedOn},
    {.verb = COMMAND_TURN_OFF, .callback = turnLedOff},
    {.verb = NULL}};

const afb_binding_t afbBindingExport = {
    .api = API_NAME,
    .verbs = verbs};
