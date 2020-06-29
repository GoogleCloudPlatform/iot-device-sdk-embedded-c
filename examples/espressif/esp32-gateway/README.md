# ESP32 Gateway demo

This demo uses three inexpensive and readily available ESP32 devices to send and receive
data from Google Cloud. The device with a connection to Google Cloud (Gateway) communicates on behalf of other devices (delegate devices).

The gateway sends the data to IoT core and publish to PubSub.

For example, you can send temperature data from multiple delegate devices to Google Cloud and store that information to see
any drastic temperature changes in a room.

## Objectives

 - Install ESP-IDF
 - Set up ESP32
 - Attach Delegate Devices
 - Publish Delegate Telemetry Data from Gateway
 - Sending Commands from  IoT Core to ESP32

## Before you begin

### ESP-IDF Setup

Before we can setup IoT Core we must get ESP-IDF, which is the SDK for Espressif chips. You can download the [ESP-IDF](https://marketplace.visualstudio.com/items?itemName=espressif.esp-idf-extension) extension for Visual Studio Code, make sure you have all dependencies because if you don’t you will get errors and will need to redownload ESP-IDF.

You’ll need to have the following for ESP-IDF to work properly:
 - Python 3.5 or higher
 - Git
 - Cmake
 - Ninja

If you don't have these dependencies, you can install them using :

**For Mac :**

```bash
brew install python
brew install git
pip install ninja
pip install cmake
```

**For Windows :**

Python : https://www.python.org/downloads/windows/
Git : https://git-scm.com/download/win
ninja : https://github.com/ninja-build/ninja/releases
cmake : https://cmake.org/download/

Once you have all dependencies installed, configure ESP-IDF.

1. Select your git and python version
1. Select the location you want to download ESP-IDF
1. Click the download button to download the ESP-IDF tools
1. Run the tool check to verify your installation

If the tool check verification succeeds, you’re ready to continue.

Once ESP-IDF is completely installed, try out the hello-world example to see if everything is working properly, I suggest putting the command to initialize ESP-IDF into an alias:

```bash
alias get_idf='. $HOME/esp/esp-idf/export.sh'
```

in your $HOME/.profile file so you can just call get_idf. If you don't have a profile dotfile, then put the code above in $HOME/.bash_profile.
For more troubleshooting steps, see the [getting started](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/) page of the ESP-IDF.

### ESP32 Setup

We will be using the Espressif Systems ESP32 (ESP32), which is an inexpensive and easy to source microcontroller with WiFi and Bluetooth capabilities. 

The ESP32 will communicate with IoT Core using WiFi and communicate to the delegate devices using BLE Mesh, the delegate devices will relay their temperature data and then the gateway will publish the telemetry data using the MQTT protocol.

To get the internal temperature we will use the `temprature_sens_read function`. To correctly set the function you must give a forward declaration for the function:

```c
#ifdef __cplusplus

extern "C" {
  #endif

uint8_t temprature_sens_read();

#ifdef __cplusplus
}
#endif
```

### Cloning mqtt example

You will need to clone the repo to get the example code. In your terminal, go to a location you want to store the cloned repo and run the following command:

```bash
git clone git@github.com:GoogleCloudPlatform/iot-device-sdk-embedded-c.git --recurse-submodules
```

Recurse submodules is important because it will download any submodules that are included in the repo.

## Attaching Delegate Devices

When the Gateway device is booted it will search for any unprovisioned devices, when it finds them it will provision them and connect. 
Once they're successfully configured the Gateway device will try to connect to the IoT Core and then attach each device to the gateway. 

The first function that gets called after the Gateway is connected will be `attachAndSubscribe`

```c
void attachAndSubscribe(iotc_context_handle_t in_context_handle)
{
    for (int i = 0; i < 2; i++)
    {
        asprintf(&publish_topic_attach, PUBLISH_TOPIC_ATTACH, delegateDevices[i]);
        iotc_state_t st = iotc_publish(in_context_handle, publish_topic_attach, "{}", 1, subscribeConfig, /*user_data=*/NULL);
    }
}
```

This function will loop through the delegate devices and publish an attach topic, when the attach topic is published the function calls a callback to `subscribeConfig`

```c
void subscribeConfig(iotc_context_handle_t in_context_handle)
{
    for (int i = 0; i < 2; i++)
    {
        asprintf(&subscribe_topic_config, SUBSCRIBE_TOPIC_CONFIG, delegateDevices[i]);
        iotc_state_t attachSt = iotc_subscribe(in_context_handle, subscribe_topic_config, 1, subscribeCommand, /*user_data=*/NULL);
    }
}
```

Like the previous function `subscribeConfig` subscribe to the device configuration subscription topic, and when it is successfully subscribed the function calls a callback to `subscribeCommand`

```c
void subscribeCommand(iotc_context_handle_t in_context_handle)
{
    for (int i = 0; i < 2; i++)
    {
        asprintf(&subscribe_topic_command, SUBSCRIBE_TOPIC_COMMAND, delegateDevices[i]);
        iotc_state_t subSt = iotc_subscribe(in_context_handle, subscribe_topic_command, 0, iotc_mqttlogic_subscribe_callback, /*user_data=*/NULL);
        delayed_publish_task = iotc_schedule_timed_task(in_context_handle, publish_delegate_telemetry_event, 6, 1, /*user_data=*/NULL);
    }
}

```

`subscribeCommand` will subscribe to the device's command topic, and when it is successfully subscribed it will call publish_delegate_telemetry_event which will publish the telemetry data

## Publish Delegate Telemetry Data from Gateway

When the Gateway receives the forwarded message from the delegate device, the function `publish_delegate_telemetry_event` gets called which checks whether the message is a `state` or `event` and publish the message corresponding to the correct topic.  

```c
void publish_delegate_telemetry_event(iotc_context_handle_t context_handle, iotc_timed_task_handle_t timed_task, void *user_data)
{
    char *publish_topic = NULL;
    char *event = "";

    if (strcmp(bleRecievedMessage, "") != 0)
    {
        if (strcmp(command, "Get Temp") == 0)
        {
            event = "events";
        }
        else if (strcmp(command, "Get State") == 0)
        {
            event = "state";
        }

        asprintf(&publish_topic, PUBLISH_TOPIC, telemetryDevice, event);
        ESP_LOGI(TAG, "publishing msg \"%s\" to topic: \"%s\"\n", bleRecievedMessage, publish_topic);
        iotc_publish(context_handle, publish_topic, bleRecievedMessage, iotc_example_qos, iotc_mqttlogic_subscribe_callback, /*user_data=*/NULL);
    }
    free(publish_topic);
    command = "";
    device = "";
    bleRecievedMessage = "";
}
```

## Sending Commands from IoT Core to ESP32

The callback function is invoked when the device receives a message from the cloud. This is where the code will convert the string into uint8_t and send it to the corresponding delegate device. 

## Running the Sample

**To Connect to Cloud:**

1. Use menu configuration with `make`

**For Gateway Device:**
```c
cd /esp32-gateway/gateway-device/ make menuconfig
```

**For Delegate Device:**
```c
cd /esp32-gateway/delegate-device/ make menuconfig
```

1. Set up your WiFi and Google Cloud Project information, navigate to example configuration
1. Locate your ec_private.pem file and copy its contents into the private.pem file in the certs folder located at `(gateway or delegate)-device/main/certs`
1. Run `idf.py build` to build sources into firmware
1. Run `idf.py -p /dev/cu.usbserial-1440 flash` passing the path to your tty device to flash the firmware onto the device
1. Run `idf.py -p /dev/cu.usbserial-1440 monitor` passing the path to your tty device to monitor the device output

Note: if you make changes to the code, you will need to rebuild the program again before calling flash

You should now see that the Gateway device is looking for unprovisioned devices, it should connect to both of them ( if not connect one and then connect the other . After the devices are connected the Gateway device will connect to cloud and attach the delegate devices. Now you can send commands from IoT Core or view the data that is being submitted by the device.

If you want to exit the serial monitor use Ctrl + ]

**To send commands:**

1. Navigate to your registry and then to the device
1. Click on the send command button at the top
1. Send the following commands

```c
Get Temp
Get State
```

Note: you should see your delegate device LED flash once it sends the response to the gateway

**To view telemetry data:**

1. Navigate to your registry
1. Click on the PubSub topic
1. Click on the PubSub subscription
1. Click on view message at the top and pull your messages

## Next Steps

Now that you've got the basics down and you can connect to IoT Core, you can add your own spin on this project, try adding led's and other sensors to send their data to the cloud.