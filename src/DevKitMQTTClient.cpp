
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <EEPROM.h>
#include <AzureIoTHub.h>

#include "../inc/iot_configs.h"
 // TODO: remove device specific code once callbacks are done
#include "../inc/device.h"
#include "../inc/DevKitMQTTClient.h"

/*String containing Hostname, Device Id & Device Key in the format:             */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>"    */
static const char *connectionString = IOT_CONFIG_CONNECTION_STRING;

#define CONNECT_TIMEOUT_MS 30000
#define CHECK_INTERVAL_MS 5000
#define MQTT_KEEPALIVE_INTERVAL_S 120
#define SEND_EVENT_RETRY_COUNT 2
#define EVENT_TIMEOUT_MS 10000
#define EVENT_CONFIRMED -2
#define EVENT_FAILED -3

static int callbackCounter;
static IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle = NULL;
static int receiveContext = 0;
static int statusContext = 0;
static int trackingId = 0;
static int currentTrackingId = -1;
static bool clientConnected = false;
static bool resetClient = false;
static CONNECTION_STATUS_CALLBACK _connection_status_callback = NULL;
static SEND_CONFIRMATION_CALLBACK _send_confirmation_callback = NULL;
static MESSAGE_CALLBACK _message_callback = NULL;
static DEVICE_TWIN_CALLBACK _device_twin_callback = NULL;
static DEVICE_METHOD_CALLBACK _device_method_callback = NULL;
static REPORT_CONFIRMATION_CALLBACK _report_confirmation_callback = NULL;
static bool enableDeviceTwin = false;

static uint64_t iothub_check_ms;

static char *iothub_hostname = NULL;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Event handlers
static void ConnectionStatusCallback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void *userContextCallback)
{
    clientConnected = false;

    switch (reason)
    {
    case IOTHUB_CLIENT_CONNECTION_EXPIRED_SAS_TOKEN:
        printf(">>>Connection status: IOTHUB_CLIENT_CONNECTION_EXPIRED_SAS_TOKEN");
        if (result == IOTHUB_CLIENT_CONNECTION_UNAUTHENTICATED)
        {
            resetClient = true;
            printf(">>>Connection status: timeout.\r\n");
        }
        break;
    case IOTHUB_CLIENT_CONNECTION_DEVICE_DISABLED:
        printf(">>>Connection status: IOTHUB_CLIENT_CONNECTION_DEVICE_DISABLED.\r\n");
        break;
    case IOTHUB_CLIENT_CONNECTION_BAD_CREDENTIAL:
        printf(">>>Connection status: IOTHUB_CLIENT_CONNECTION_BAD_CREDENTIAL.\r\n");
        break;
    case IOTHUB_CLIENT_CONNECTION_RETRY_EXPIRED:
        printf(">>>Connection status: IOTHUB_CLIENT_CONNECTION_RETRY_EXPIRED.\r\n");
        break;
    case IOTHUB_CLIENT_CONNECTION_NO_NETWORK:
        printf(">>>Connection status: IOTHUB_CLIENT_CONNECTION_NO_NETWORK.\r\n");
        if (result == IOTHUB_CLIENT_CONNECTION_UNAUTHENTICATED)
        {
            resetClient = true;
            printf(">>>Connection status: disconnected");
        }
        break;
    case IOTHUB_CLIENT_CONNECTION_COMMUNICATION_ERROR:
        printf(">>>Connection status: IOTHUB_CLIENT_CONNECTION_COMMUNICATION_ERROR.\r\n");
        break;
    case IOTHUB_CLIENT_CONNECTION_OK:
        printf(">>>Connection status: IOTHUB_CLIENT_CONNECTION_OK.\r\n");
        if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED)
        {

            clientConnected = true;
            printf(">>>Connection status: connected.\r\n");

            /* Some device specific action code goes here... */
            Device::beep();
            //Green Led
            Device::setLed(1, 1);
            /* End of device specific code */
            
        }
        break;
    }

    if (_connection_status_callback)
    {
        _connection_status_callback(result, reason);
    }
}
static void SendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void *userContextCallback)
{
    EVENT_INSTANCE *event = (EVENT_INSTANCE *)userContextCallback;
    printf(">>>Confirmation[%d] received for message tracking id = %d with result = %s .\r\n", callbackCounter++, event->trackingId, ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));

    if (currentTrackingId == event->trackingId)
    {
        if (result == IOTHUB_CLIENT_CONFIRMATION_OK)
        {
            currentTrackingId = EVENT_CONFIRMED;
        }
        else
        {
            currentTrackingId = EVENT_FAILED;
        }
    }

    // Free the message
    IoTHubMessage_Destroy(event->messageHandle);
    free(event);

    if (_send_confirmation_callback)
    {
        _send_confirmation_callback(result);
    }
}

static IOTHUBMESSAGE_DISPOSITION_RESULT ReceiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void *userContextCallback)
{

    int *counter = (int *)userContextCallback;
    const char *buffer;
    size_t size;

    // Message content
    if (IoTHubMessage_GetByteArray(message, (const unsigned char **)&buffer, &size) != IOTHUB_MESSAGE_OK)
    {
        printf("unable to retrieve the message data.\r\n");
        return IOTHUBMESSAGE_REJECTED;
    }
    else
    {
        char *temp = (char *)malloc(size + 1);
        if (temp == NULL)
        {
            printf("Failed to malloc for command.\r\n");
            return IOTHUBMESSAGE_REJECTED;
        }
        memcpy(temp, buffer, size);
        temp[size] = '\0';
        printf(">>>Received Message [%d], Size=%d Message %s.\r\n", *counter, (int)size, temp);
        if (_message_callback)
        {
            _message_callback(temp, size);
        }
        free(temp);
    }

    // TODO: remove device specific code once callbacks are done 
    /* Some device specific action code goes here... */
    Device::beep();
    /* End of device specific code */

    (*counter)++;
    return IOTHUBMESSAGE_ACCEPTED;
}

static void DeviceTwinCallback(DEVICE_TWIN_UPDATE_STATE updateState, const unsigned char *payLoad, size_t size, void *userContextCallback)
{
     // TODO: remove device specific code once callbacks are done
    Device::beep();
    if (_device_twin_callback)
    {
        _device_twin_callback(updateState, payLoad, size);
    }
    {
        printf(">>>No twin callback defined \r\n");
    }
}

static int DeviceMethodCallback(const char *methodName, const unsigned char *payload, size_t size, unsigned char **response, size_t *response_size, void *userContextCallback)
{
     // TODO: remove device specific code once callbacks are done
    Device::beep();
    if (_device_method_callback)
    {
        return _device_method_callback(methodName, payload, size, response, (int *)response_size);
    }
    printf(">>>No method found \r\n");
    const char *responseMessage = "\"No method found\"";
    *response_size = strlen(responseMessage);
    *response = (unsigned char *)strdup("\"No method found\"");

    return 404;
}

static void ReportConfirmationCallback(int statusCode, void *userContextCallback)
{
    EVENT_INSTANCE *event = (EVENT_INSTANCE *)userContextCallback;
    printf(">>>Confirmation[%d] received for state tracking id = %d with state code = %d.\r\n", callbackCounter++, event->trackingId, statusCode);

    if (statusCode == 204)
    {
        if (currentTrackingId == event->trackingId)
        {
            currentTrackingId = EVENT_CONFIRMED;
        }
    }
    else
    {
        printf("Report confirmation failed with state code %d.\r\n", statusCode);
    }

    // Free the state
    free(event);

    if (_report_confirmation_callback)
    {
        _report_confirmation_callback(statusCode);
    }
}

// TODO: add timeout 
bool DevKitMQTTClient_Init(bool hasDeviceTwin)
{

    printf("DevKitMQTTClient>> IoT hub establishing connection.\r\n");
    if (iotHubClientHandle != NULL)
    {
        return true;
    }
    enableDeviceTwin = hasDeviceTwin;
    callbackCounter = 0;

    srand((unsigned int)time(NULL));
    trackingId = 0;
    // Create the IoTHub client
    if ((iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, MQTT_Protocol)) == NULL)
    {
        printf("IoT hub establish failed.\r\n");
        return false;
    }

    int keepalive = MQTT_KEEPALIVE_INTERVAL_S;
    IoTHubClient_LL_SetOption(iotHubClientHandle, "keepalive", &keepalive);
    bool traceOn = true;
    IoTHubClient_LL_SetOption(iotHubClientHandle, "Serial.println", &traceOn);
#ifdef SET_TRUSTED_CERT_IN_SAMPLES
    if (IoTHubClient_LL_SetOption(iotHubClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK)
    {
        printf("Failed to set option \"TrustedCerts\".\r\n");
        return false;
    }
#endif

    printf("IoTHubClient_LL_SetMessageCallback");
    /* Setting Message call back, so we can receive Commands. */
    if (IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, ReceiveMessageCallback, &receiveContext) != IOTHUB_CLIENT_OK)
    {
        printf("IoTHubClient_LL_SetMessageCallback..........FAILED!.\r\n");
        return false;
    }
    printf("IoTHubClient_LL_SetConnectionStatusCallback.\r\n");
    if (IoTHubClient_LL_SetConnectionStatusCallback(iotHubClientHandle, ConnectionStatusCallback, &statusContext) != IOTHUB_CLIENT_OK)
    {
        printf("IoTHubClient_LL_SetConnectionStatusCallback..........FAILED!.\r\n");
        return false;
    }

    if (enableDeviceTwin)
    {
        printf("IoTHubClient_LL_SetDeviceTwinCallback.\r\n");
        if (IoTHubClient_LL_SetDeviceTwinCallback(iotHubClientHandle, DeviceTwinCallback, NULL) != IOTHUB_CLIENT_OK)
        {
            printf("Failed on IoTHubClient_LL_SetDeviceTwinCallback.\r\n");
            return false;
        }
        printf("IoTHubClient_LL_SetDeviceMethodCallback");
        if (IoTHubClient_LL_SetDeviceMethodCallback(iotHubClientHandle, DeviceMethodCallback, NULL) != IOTHUB_CLIENT_OK)
        {
            printf("Failed on IoTHubClient_LL_SetDeviceMethodCallback.\r\n");
            return false;
        }
    }

    // TODO: add CONNECT_TIMEOUT_MS logic
    int wait_count = 0;
    while (true)
    {
        IoTHubClient_LL_DoWork(iotHubClientHandle);
        if (clientConnected)
        {
            break;
        }
        
        if (wait_count >= 1000)
        {
            // Time out, reset the client
            resetClient = true;
            return false;
        }
        ThreadAPI_Sleep(500);
        wait_count++;
    }
    
    return true;
}
void DevKitMQTTClient_DoWork()
{
    IoTHubClient_LL_DoWork(iotHubClientHandle);
}