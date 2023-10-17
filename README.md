# Riotbox

Riotbox is an easy, quick and flexible open-source project that simplifies the process of building secure IoT and Smart Home solutions whether it is for prototyping or production. Riotbox utilises a single Docker container to deploy:

- *Cloud*: Serverless backend using [AWS CDK](https://docs.aws.amazon.com/cdk/v2/guide/getting_started.html)
- *Mobile*: React Native iOS/Android app using [AWS Amplify](https://aws.amazon.com/amplify/getting-started/) 
- *Thing*: WiFi connected IoT device using [ESP32 with esp-idf](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/)

## Overview

Riotbox provides the essentials for WiFi connected IoT devices: secure user authentication, BLE provisoning, over-the-air (OTA) updates and authorized MQTT publish/subscribe operations.

Riotbox utilizes a set of easy-to-use helper scripts for initialising and destroying your project, building, flashing and deploying your firmware, and generating and rotating encryption keys and certificates used for authentication and authorization.

By default, Riotbox implements a simple "Switch" type which enables the user to toggle an LED from the mobile app, or from a button on the ESP32 and see the updates in realtime in the mobile app. For a developer using Riotbox it is easy to create custom types by implementing your device-specific JSON structure on the ESP32 (Thing) side, as well as on the React Native (Mobile) side.

*Note*: Although Riotbox is open source and free to use, AWS and App Store/Google Play deployment might have associated costs.

Looking for a quick way to add custom types? Go to [Add custom type](#add-custom-type).

**Compatibility Note**: As of now, Riotbox has been validated on Mac OSX for iPhone iOS. While it should theoretically support Windows/Linux for Android, such configurations haven't been tested yet.


# Getting started

Video on [YouTube](https://youtu.be/kQq34aRfYdk).

To get started you need an [AWS account](https://aws.amazon.com/console/), a project name and a Thing type. Here, lets use the pre-defined default `SWITCH` type, by creating an `.env`-file in project `root`. Use `.env-example` as a template but populate your own AWS credentials.

Your `.env` file should look something like this:

```Dotenv
# Static per project. Rebuild project if any of them are changed
PROJECT_NAME=MyProject
SECRETS_PASSWORD=mysecretpassword
AWS_ACCESS_KEY_ID=AKIAXXXXXXXXXXXXXXXX
AWS_SECRET_ACCESS_KEY_ID=XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
AWS_REGION=ap-southeast-2
# Dynamically used when flashing/deploying a Thing. These can be changed at any time.
THING_TYPE=SWITCH
THING_HW_VERSION=1.0.0
THING_SERIAL_PORT=/dev/tty.usbserial-0001
```

- `PROJECT_NAME`: Name of your mobile app. The resources in AWS will also be named with this prefix.
- `SECRETS_PASSWORD`: The password used to encrypt/decrypt your secrets inside `secrets`. Use the scripts inside `scripts/docker/secrets` to either decrypt or encrypt.
- `AWS_ACCESS_KEY_ID`: Your unique AWS access key ID which can be obtained from AWS console. The key is used to deploy the AWS resources.
- `AWS_SECRET_ACCESS_KEY_ID`: Your unique AWS access key ID which can be obtained from AWS console. The key is used to deploy the AWS resources.
- `AWS_REGION`: The region to which the AWS resources are deployed.
- `THING_TYPE`: The type of your `Thing`. This variable is sent to the ESP32 on secrets provisioning using `scripts/host/esp32/esp_provision_secrets.sh`. It is used by the ESP32 in `app/thing/main/app/types/type.c`.
- `THING_HW_VERSION`: The hardware revision of your `Thing`. This variable is sent to the ESP32 on secrets provisioning using `scripts/docker/aws/aws_deploy_thing.sh`. It is currently not being used for anything useful.
- `THING_SERIAL_PORT`: The serial port of your ESP32. Find which port your computer is using with e.g. Putty or CoolTerm.

The `static` variables are specific to your project and should not be changed. If you want to change them - destroy the project first using `/scripts/docker/riotbox_destroy.sh`, exit the Docker container and re-initialise the project.

The `dynamic` variables are used only to deploy a Thing to DynamoDB with `scripts/host/esp32/esp_provision_secrets.sh`. They can be changed at any time.

## Build and enter inside the Docker container
```Shell
./develop.sh
```
## Initialise Project

When inside the docker container, navigate to `scripts/docker/`. The next steps must be followed in order.

```Shell
cd scripts/docker
```
Build and deploy the AWS CDK `Cloud`:
```Shell
./riotbox_init1_cloud.sh
```
Build the React Native Amplify `Mobile` app:
```Shell
./riotbox_init2_mobile.sh
```
Build the ESP32 `Thing` firmware:
```Shell
./riotbox_init3_thing.sh
```

You now have the complete project setup. 

## Flash ESP firmware
To flash your ESP32 with the firmware, open a new terminal outside of the Docker container.

For this step you need `Python` installed on your computer, along with the `esptool` module. If you don't already have it, install it with:
```Shell
python3 -m pip install esptool
```

Next, navigate to `scripts/host/esp32`
```Shell
cd scripts/host/esp32
```
Erase the flash memory on the ESP32:
```Shell
./esp_erase.sh
```
Flash the ESP32 firmware:
```Shell
./esp_flash.sh
```
Provision the ESP32 secrets:
```Shell
./esp_provision_secrets.sh
```

## Run the mobile app
To build the mobile app, you need an account with Expo (https://expo.dev/eas) as well as with either Apple () or Google (), depending on which platform you're developing for. To run the mobile app, you once again need to be in the terminal outside of the Docker container. For this step, you need `npm` installed on your computer along with the `eas-cli` (Expo Application Services) package. If you don't already have it, install it with:
```Shell
npm install -g eas-cli
```

Next, navigate to `scripts/host/expo`
```Shell
cd scripts/host/expo
```
Build the app for your platform, e.g:
```Shell
./expo_build_ios.sh
```
Answer the propmt options:
```
Would you like to automatically create an EAS project for <user>: 
What would you like your iOS bundle identifier to be? <Return>
Do you want to log in to your Apple account? > Y
Would you like to reuse the profile? > Yes
Would you like to set up Push Notifications for your project? > No
```
Let the app build and scan the QR code that appears on the screen when finished. This will install the app on your phone.
Next start the development server:
```Shell
./expo_run.sh
```
Scan the QR code once again that appears on the screen. This will start and open the mobile app.

## Deploy the Thing
Before your ESP32 can be provisioned, you have to deploy your Thing from inside the Docker container. Navigate to `scripts/docker/aws`:
```Shell
cd scripts/docker/aws
```
Now, deploy the thing you have provisioned with secrets:
```Shell
./aws_deploy_thing.sh
```

## Provision the Thing
To provision the ESP32 Thing, use the mobile app. Do so by pressing the `+` button in the top-right corner and scan the QR code for your Thing. You can create a QR code with any QR generator, e.g. [qr-code-generator](https://www.qr-code-generator.com/). Paste the ID for the Thing into the text box. Find the Thing ID in either `secrets/decrypted/esp_deploy/esp_deploy_data.txt` or in the Dynamo Table in AWS console.

## Denitialise Project

If you want to deinitialise the project, it can be done with a single script.
Inside the docker container, navigate to `scripts/docker/` and run the script.

```Shell
cd scripts/docker
```
```Shell
./riotbox_deinit.sh
```

The script will:
- Remove all resources in AWS, including those used by Amplify (mobile)
- delete all secrets
- delete built firmware
- delete installed packages

The script will not:
- Delete any code, including any changes you have made
- Delete your changes to .env

After running the script the project will be reset and can afterwards be reinitialised at any point. To reinitialise the project, or start a new project based off of your already existing changes you need to exit the docker container with `exit` and run `./develop.sh` again.

# Add custom type

Adding a new custom type is designed to be easy for the developer. You need to add the type to the ESP32 Thing firmware and to the React Native Mobile software. The AWS Cloud does not need modifications. Follow the steps below.

## Update .env

Name your type something descriptive e.g. `TEMP_SENSOR` with maximum 31 characters. This is the string that will be displayed in the mobile app when a new Thing has been provisioned. Each Thing has a nickname you can change from the mobile app later.

```Dotenv
THING_TYPE=TEMP_SENSOR
```

## Update Thing firmware

Video on [YouTube](https://youtu.be/4Yh5uyhiEUQ).

### Create custom type .c and .h files

Locate the folder `app/thing/main/app/types`.
Create the files inside the folder, e.g. `temp-sensor.c` and `temp-sensor.h`. The `default` type acts as an minimal template so you can base your custom type on it, as well as the `switch` type.

Inside `temp-sensor.h`, create the following:

```C
#define TEMP_SENSOR_TYPE_STR "TEMP_SENSOR"
#define TEMP_SENSOR_TYPE_INT 2
```

Make sure the value of `TEMP_SENSOR_TYPE_STR` is written exactly like in `.env` and is 31 characters or shorter, and make sure `TEMP_SENSOR_TYPE_INT` is not the same value as any of the other types. `0` and `1` are already used in the `default.h` and `switch.h` type headers, so here we choose `2`.

Create the struct with as many `read` or `readwrite` properties as you need for the custom type. `TEMP_SENSOR` has a temperature sensor which value cannot be changed from the mobile app, hence it is a `read-only` property:

```C
typedef struct temp_sensor_readwrite_t {
    // Leave empty
} temp_sensor_readwrite_t;

typedef struct temp_sensor_read_t {
    int temperature;
} temp_sensor_read_t;

typedef struct temp_sensor_value_t {
    temp_sensor_readwrite_t readwrite;
    temp_sensor_read_t read;
} temp_sensor_value_t;
```

Inside `temp-sensor.c`, initialise your type struct:

```C
static default_value_t default_value_g = {
    .read = {
        .temperature = 0,
    },
    .readwrite = {
        // Leave empty
    }
};
```

Copy and rename the `default_set_value_json` and `default_set_value_json` functions to implement your type specific parser functions. Look at how it is done in `switch.c`.

The `callback_publish_value` argument is used to publish the type JSON over MQTT to be received by the mobile app.

### Add custom type to type.c

Locate the file `app/thing/main/app/types/type.c`. Some small changes are required to this file in order for the Thing firmware to use the custom type functions.

Include the custom type header:

```C
// Include your thing type
#include "app/types/type.h"
#include "app/types/switch.h"
#include "app/types/default.h"
#include "app/types/temp_sensor.h" // <- Add here
```

Add your custom type to all functions in `type.c`, e.g:

```C
// Add your new type in the if case
bool type_set_int(const char* type_str)
{
    if (strcmp(type_str, SWITCH_TYPE_STR) == 0){
        type_int = SWITCH_TYPE_INT;
        return true;
    }
    if (strcmp(type_str, TEMP_SENSOR_TYPE_STR) == 0){ // <- Add here
        type_int = TEMP_SENSOR_TYPE_INT;
        return true;
    }
    ESP_LOGE(TAG, "Error: type_set_int, use default type");
    return false;
}

// Add your new type in the switch case
bool type_init(callback_t callback_publish_value)
{   
    ESP_LOGI(TAG, "Initialise");
    switch(type_int){
        case SWITCH_TYPE_INT: return switch_init(callback_publish_value);
        case TEMP_SENSOR_TYPE_INT: return temp_sensor_init(callback_publish_value); // <- Add here
        default: return default_init(callback_publish_value);
    }
    ESP_LOGE(TAG, "Error: type_init");
    return false;
}
```

### Add custom type to CMakeLists.txt

Locate the file `app/thing/main/app/main/CMakeLists.txt`.
Add your custom .c file to the list here:
```C
"app/types/type.c"
"app/types/default.c" 
"app/types/switch.c"
"app/types/temp_sensor.c" // <- Add here
```

### Deploy custom type Thing

The custom type Thing firmware is now ready to be deployed. Do so by running following scripts.

From inside the Docker container:
```Shell
scripts/docker/esp32/esp_compile_thing.sh
```

From outside the Docker container:
```Shell
scripts/host/esp32/esp_erase.sh
scripts/host/esp32/esp_flash.sh
scripts/host/esp32/esp_provision_secrets.sh
```

From inside the Docker container:
```Shell
scripts/docker/aws/aws_deploy_thing.sh
```

Your Thing is now ready to be provisioned by the mobile app. As the `TEMP_SENSOR` has not yet been implemented in the mobile app code yet, the `Default` type will be used to display the JSON of the custom type until the type has been implemented on the mobile app side.

## Update Mobile React Native code

Video on [YouTube](https://youtu.be/XxPK1-4kSro).

### Create custom type .jsx file

Locate the folder `app/mobile/types`.
Create the file inside the folder, e.g. `TempSensorScreen.jsx`. The `DefaultScreen` type acts as an minimal template so you can base your custom type on it, as well as the `SwitchScreen` type.

To simply display the value of the temperature sensor, do this:

```Javascript
const TempSensorScreen = ({ thingId }) => {
    const [isUpdatingValue, setUpdateValue, thingValue] = useUpdateValue(thingId);
    
    if (!thingValue) return null;
    const temperature = thingValue.readwrite.temperature;

    return (
        <View>
            <Text style={styles.text}>temperature</Text>
        </View>
    );
};
```

As temperature is a read-only value, there are no Thing values we can change from the mobile app. If you want to see how values can be changed from the mobile app side, please have a look at the `SWITCH` type implementations.

### Add custom type to Type.jsx

Locate the file `app/mobile/types/Type.jsx`.
Import the custom type:

```Javascript
import DefaultScreen from "./DefaultScreen";
import SwitchScreen from "./SwitchScreen";
import SwitchScreen from "./TempSensorScreen"; // <- Add here
```

Add the icon you want displayed for your type inside `Type.jsx`, e.g:

```Javascript
export const getTypeIcon = (type) => {
    switch (type) {
        case "SWITCH":
            return "power";
        case "TEMP_SENSOR": // <- Add here
            return "thermometer";
        default:
            return "help";
    }
};
```
All available icons can be found [here](https://ionic.io/ionicons).

Next, add the `TempSensorScreen` file to the switch-case:

```Javascript
export const getTypeComponent = (type, id) => {
    switch (type) {
        case "SWITCH":
            return <SwitchScreen thingId={id} />;
        case "TEMP_SENSOR": // <- Add here
            return <TempSensorScreen thingId={id} />;
        default:
            return <DefaultScreen thingId={id} />;
    }
};
```
### Rebuild the mobile app

The mobile app is now ready to be rebuilt. Do so by running following scripts.

If you haven't already, build from outside the Docker container:
```Shell
scripts/host/expo/expo_build_ios.sh
```
or 
```Shell
scripts/host/expo/expo_build_android.sh
```
or both, and then start the app with:
```Shell
scripts/host/expo/expo_run.sh
```

You are now ready to provision your Thing over BLE in the mobile app.

# OTA firmware

The Thing checks for a new OTA firmware on reboot and every 24h. The OTA firmwares are stored in the S3 bucket called `<project-name>-bucket-thing-ota-fw-<aws-account-number>` and can be found in AWS console. The Thing will always download the newest firmware uploaded to the S3 bucket. The OTA update will proceed if the firmware `PROJECT_VER` is higher than the currently running firmware, or will else continue normal operation. The OTA firmware name equals the `PROJECT_VER`, e.g. `1.0.0` which is the name of the firmware OTA uploaded on project initialisation.

## Upload new OTA firmware

Before you upload a new OTA firmware to be downloaded by your ESP32 Thing(s), bump the value of `PROJECT_VER` found in `app/thing/CMakeLists.txt` from e.g. `set(PROJECT_VER "1.0.0")` to `set(PROJECT_VER "1.0.1")`.

After bumping the firmware version you can upload the firmware with from inside the Docker container. Navigate to `scripts/docker/aws`:
```Shell
cd scripts/docker/aws
```
Now, upload the firmware to the S3 bucket:
```Shell
./aws_upload_ota_to_s3.sh
```

The script will rebuild the firmware, rename it to `PROJECT_VER` and upload it to the S3 bucket storing the firmware images.

The firmware is pre-encrypted using the `rsa_key.pem` from `secrets/decrypted/esp_encryption/` and is decrypted automatically on the ESP32 after download.

As the Thing type is sent to the ESP32 on secrets provisioning it will not change on OTA updates, hence the same firmware can be used by different Thing types as more are added.

# Project folder stucture

In project `root` you'll find three folders `app`, `scripts` and `secrets`, as well as a few files. Your project configuration should be defined inside an `.env`-file that you need to create. It should be a copy of `.env-example` populated with your project specific values.
The most part of the project creation takes place inside a Docker container defined by `Dockerfile` and `docker-compose.yml`. Build and enter the docker container by running the `develop.sh`-script.

## App

Inside `app` is all the code for your project. 

### Cloud

Inside `app/cloud/` is the code used to build the cloud backend infrastructure. The backend is fully serverless for best scalability and cost and is built using AWS infrastructure-as-code (IAC) framework - AWS CDK in Javascript. Inside `app/cloud/lib/cdk-stack.js` you'll find the entire CDK stack describing the infrastructure. The lambda functions used by the stack is located inside `app/cloud/lambdas/`, and the declaration of the API used to update the Dynamo Database is located inside `app/cloud/graphql`.

The project is using three different lambda functions:

- api-things: This lambda reads and writes to the Dynamo Database table with your deployed ESP32 Things. It can be triggered by calling the graphql API from either the mobile App or another lambda function. Currently, it is used by `app/mobile/graphql/GraphqlOperations.jsx` and `app/cloud/lambdas/iotcore-thing-to-cloud/actionValue.js`.

- iotcore-thing-to-cloud: This lambda is triggered by any MQTT topic starting with `thingpub/` used to publish data from the ESP32 Thing(s) to the cloud. Currently it has three different topics it can receive data on: `thingpub/<Thing ID>/value`, `thingpub/<Thing ID>/otaurl` and `thingpub/<Thing ID>/bootup`. The topics are explained more in `<TODO>`.

- iotcore-cloud-to-thing: This lambda is triggered by updates to the Dynamo Database table. Any updates to the table by the user (mobile) are handled by sending the new value to the correct ESP32 Thing over MQTT. The topics are explained more in `<TODO>`.

### Mobile

Inside `app/mobile/` is the React Native code. As well as the code for the App frontend, necessary parameters used by Amplify, exported by the CDK cloud creation can be be found inside `app/mobile/src/ck-exports.json`, which are consumed by the App in `app/mobile/App.jsx`.

The mobile app utilises AWS Cognito for authentication and authorization of users, and a GraphQL API for reading, updating and subscribing to the Thing Dynamo database table. A Graphql API was chosen over a REST API because of the simplicity of subscribing to Dynamo changes rather than polling, ensuring the mobile App being both responsive and always in sync with the ESP32 Thing.

### Thing

Inside `app/thing/` is the ESP32 firmware. The ESP32 needs to have at least 4MB flash to fit certificates, keys and space for two partitions needed for OTA functionality.

## Scripts

Inside `app/scripts/` are all script located used by the initialisation and destroy process of the project, as well as scripts for every necessary operations to the project.

### Docker scripts

Inside `app/scripts/docker` are all the scripts that should be run from within the Docker container.
In this folder you can also find the main scripts used to initialise and destroy your project. Every script is safe to be run multiple times as they are overwriting. Scripts used for specific tasks are located into sub folders.

### Host scripts

Inside `app/scripts/host` are all scripts that should be run on the Host machine, outside of the docker container. These scripts will need local dependencies to work.

## Secrets

Inside `app/secrets/` are all project specific secret files. A clean download of Riotbox has no files inside the secrets folder as they are created on project initialisation. The unencrypted secrets folder is never committed to Git. If you want to commit your secrets to Git, you can do so by first encrypting them using `scripts/docker/secrets/secrets_encrypt.sh` script which is using the `SECRETS_PASSWORD` from `.env` to create encrypted files inside `app/secrets/encrypted`. These files are committed to Git, and can be decrypted by using `scripts/docker/secrets/secrets_decrypt.sh`.

# System arcitecture
![Alt text](./flowchart.png)

# Contact

If you have problems, questions, ideas or suggestions, please contact me on:
hello@klaslofstedt.se

Do you use and like Riotbox? Please [buy me a coffee](https://www.buymeacoffee.com/klaslofstedt) 🙏

