import React, { useState, useEffect, useRef } from "react";
import {
    Text,
    View,
    TouchableOpacity,
    Modal,
    StyleSheet,
    SafeAreaView,
} from "react-native";
import { BarCodeScanner } from "expo-barcode-scanner";
import Ionicons from "react-native-vector-icons/Ionicons";
import { useNavigation } from "@react-navigation/native";
import { BleManager } from "react-native-ble-plx";
import { apiThingGetById, apiThingProvision } from "../graphql/GraphqlApi";
import { ScrollView } from "react-native-gesture-handler";
import base64 from "react-native-base64";
import { iconSize, colors, width, fonts } from "../styles/Styles";
import { aesCrypto } from "../misc/aes";
import { AppPopupError } from "../components/AppPopupError";
import { AppLoading } from "../components/AppLoading";
import { AppPopupSuccess } from "../components/AppPopupSuccess";
import { Storage } from "aws-amplify";
import { EventEmitter } from "events";
import { AppTextInput } from "../components/AppTextInput";
import { v4 as uuidv4 } from "uuid";

const PROVISION_SERVICE_UUID = "ffff";
const PROVISION_CHAR_WRITE_POP_UUID = "ff01";
const PROVISION_CHAR_WRITE_WIFI_CREDS_UUID = "ff02";
const PROVISION_CHAR_WRITE_ROOT_CA_UUID = "ff03";
const PROVISION_CHAR_WRITE_THING_CERT_UUID = "ff04";
const PROVISION_CHAR_WRITE_THING_KEY_UUID = "ff05";
const PROVISION_CHAR_NOTIFY_UUID = "ff06";

const bleManager = new BleManager();

const ScreenStateBottom = {
    NONE: "none",
    BARCODE_SCAN: "barcodeScan",
    NETWORK_LIST: "networkList",
    NETWORK_DONE: "networkDone",
};

const ScreenStateTop = {
    NONE: "none",
    LOADING: "loading",
    PASSWORD_MODAL: "passwordModal",
    CAMERA_ERROR_MODAL: "cameraErrorModal",
    BLE_ERROR_MODAL: "bleErrorModal",
    BLE_PROVISIONED_MODAL: "bleProvisionedModal",
    QR_ERROR_MODAL: "qrErrorModal",
};

const PasswordInputModal = ({
    onClose,
    onSubmit,
    wifiSSID,
    wifiPassword,
    setWifiPassword,
    wifiPasswordRef,
}) => {
    return (
        <View style={styles.loadingOverlay}>
            <Modal
                animationType="slide"
                transparent={true}
                visible={true}
                hasBackdrop={true}
                backdropOpacity={10}
                backdropColor="black"
            >
                <View style={styles.containerModalPosition}>
                    <View style={styles.containerModal}>
                        <View style={styles.containerModalHeader}>
                            <TouchableOpacity onPress={onClose}>
                                <Ionicons
                                    name={"close-circle"}
                                    size={iconSize.medium}
                                    style={{ color: colors.colorBrighter }}
                                />
                            </TouchableOpacity>
                            <Text style={styles.textMedium}>{wifiSSID}</Text>
                            <TouchableOpacity onPress={onSubmit}>
                                <Ionicons
                                    name={"checkmark-circle"}
                                    size={iconSize.medium}
                                    style={{ color: colors.colorFocus }}
                                />
                            </TouchableOpacity>
                        </View>
                        <SafeAreaView>
                            <AppTextInput
                                inputRef={wifiPasswordRef}
                                value={wifiPassword}
                                setValue={setWifiPassword}
                                placeholder={"Enter password"}
                                isSecure={true}
                                icon={"lock-closed"}
                                keyboardType={"default"}
                            />
                        </SafeAreaView>
                    </View>
                </View>
            </Modal>
        </View>
    );
};

const NetworkScrollList = ({ ssid, rssi, done, onPress }) => {
    let wifiIconColour;
    if (rssi > -65) {
        wifiIconColour = colors.colorGreen;
    } else if (rssi > -75) {
        wifiIconColour = colors.colorYellow;
    } else if (rssi > -85) {
        wifiIconColour = colors.colorOrange;
    } else {
        wifiIconColour = colors.colorRed;
    }

    return (
        <TouchableOpacity onPress={() => onPress(ssid)}>
            <View
                style={
                    done === true
                        ? styles.containerDeviceTopLayerOnline
                        : styles.containerDeviceTopLayerOffline
                }
            >
                <View style={styles.containerWifiList}>
                    <View style={styles.containerDevice}>
                        <Ionicons
                            name={"wifi-outline"}
                            size={iconSize.medium}
                            style={{ color: wifiIconColour }}
                        />
                        <View>
                            <Text style={styles.textMedium}>{ssid}</Text>
                        </View>
                    </View>
                </View>
            </View>
        </TouchableOpacity>
    );
};

const ProvisionScreen = () => {
    const [networkList, setNetworkList] = useState([]);
    const [wifiPassword, setWifiPassword] = useState("");
    const navigation = useNavigation();
    const isThingProvisioned = useRef(false);
    const bleThingNameRef = useRef("");
    const wifiPasswordRef = useRef("");
    const bleThingIdRef = useRef("");
    const wifiSSIDRef = useRef("");
    const aesKeyRef = useRef("");

    const [screenStateBottom, setScreenStateBottom] = useState(
        ScreenStateBottom.NONE
    );
    const [screenStateTop, setScreenStateTop] = useState(
        ScreenStateTop.LOADING
    );
    const eventEmitterRef = useRef(new EventEmitter());

    useEffect(() => {
        const getBarCodeScannerPermissions = async () => {
            const { status } = await BarCodeScanner.requestPermissionsAsync();
            console.log("Camera permission: ", status);
            if (status === "granted") {
                setScreenStateTop(ScreenStateTop.NONE);
                setScreenStateBottom(ScreenStateBottom.BARCODE_SCAN);
            } else {
                setScreenStateTop(ScreenStateTop.CAMERA_ERROR_MODAL);
                setScreenStateBottom(ScreenStateBottom.NONE);
            }
        };
        getBarCodeScannerPermissions();
    }, []);

    const bleScanForThing = async () => {
        console.log("bleScanForThing");

        try {
            bleManager.startDeviceScan(null, null, blePairThing);
        } catch (error) {
            console.log("Error connecting to BLE Thing:", error);
        }
    };

    const blePairThing = async (error, bleThingFound) => {
        if (error) {
            console.log("Error: ", error);
            return;
        }
        if (!bleThingFound) {
            console.log("No BLE Thing Found");
            return;
        }
        console.log("bleThingFound.name:", bleThingFound.name);
        console.log("bleThingNameRef:", bleThingNameRef.current);

        if (error) {
            console.log("Error: ", error);
            return;
        }
        if (bleThingFound.name === bleThingNameRef.current) {
            console.log("Found: ", bleThingFound.name);
            const thing = await apiThingGetById(bleThingFound.name);
            if (!thing) {
                console.log("Some Thing fetching error");
                setScreenStateBottom(ScreenStateBottom.NONE);
                setScreenStateTop(ScreenStateTop.BLE_ERROR_MODAL);
                return;
            }
            aesKeyRef.current = thing.aes;
            bleThingIdRef.current = bleThingFound.id;

            await bleConnectThing();
            bleManager.stopDeviceScan();
            await blePopThing(thing.pop);
        }
    };

    const bleConnectThing = async () => {
        console.log("bleConnectThing: ", bleThingIdRef.current);
        try {
            const connectedThing = await bleManager.connectToDevice(bleThingIdRef.current);
            console.log("Connected to: ", bleThingIdRef.current);
            console.log("MTU: ", connectedThing.mtu);
            await bleManager.discoverAllServicesAndCharacteristicsForDevice(
                bleThingIdRef.current
            );
            await bleManager.monitorCharacteristicForDevice(
                bleThingIdRef.current,
                PROVISION_SERVICE_UUID,
                PROVISION_CHAR_NOTIFY_UUID,
                async (error, characteristic) => {
                    if (error) {
                        console.log("Scan Error: ", error);
                        // This error is expected when provisioning is done, hence do not display BLE error modal
                        if (isThingProvisioned.current) {
                            return;
                        }
                        setScreenStateBottom(ScreenStateBottom.NONE);
                        setScreenStateTop(ScreenStateTop.BLE_ERROR_MODAL);
                        return;
                    }
                    const bleNotification = JSON.parse(
                        base64.decode(characteristic.value)
                    );
                    await bleHandleNotification(bleNotification);
                }
            );
        } catch (e) {
            console.log("FAILED TO CONNECT", e);
        }
    };

    // TODO
    const bleDisconnectThing = async () => {
        console.log("device in useState: ", bleThingIdRef.current);
        try {
            //await bleManager.stopDeviceScan();
            //await bleManager.disconnect(bleThingName);
            //await bleManager.removePeripheral(bleThingName);
            //await bleManager.stopDeviceScan();
            //await bleManager.destroy();
            await bleManager.cancelDeviceConnection(bleThingIdRef.current);
        } catch (error) {
            console.log("Error turning off BLE:", error);
        }
    };

    const bleHandleNotification = async (notification) => {
        console.log("Received BLE notification: ", notification);
        if (notification.type === "wifi_scan") {
            if (notification.ssid.length) {
                setScreenStateBottom(ScreenStateBottom.NETWORK_LIST);
                setScreenStateTop(ScreenStateTop.NONE);
                setNetworkList((prevList) => [...prevList, notification]);
            }
            if (notification.count === 0) {
                console.log("Received all networks from esp32");
                setScreenStateBottom(ScreenStateBottom.NETWORK_DONE);
            }
        }
        if (notification.type === "provision") {
            if (notification.status === "done") {
                console.log("Device successfully provisioned!");
                isThingProvisioned.current = true;
                setScreenStateBottom(ScreenStateBottom.NONE);
                setScreenStateTop(ScreenStateTop.BLE_PROVISIONED_MODAL);
                await apiThingProvision(bleThingNameRef.current);
            }
            if (notification.status === "progress") {
                console.log("Device under provisioning!");
                eventEmitterRef.current.emit("provisionProgress");
            }
            if (notification.status === "fail") {
                console.log("Device failed provisioning");
                setScreenStateBottom(ScreenStateBottom.NONE);
                setScreenStateTop(ScreenStateTop.BLE_ERROR_MODAL);
            }
        }
    };

    const blePopThing = async (pop) => {
        console.log("Writing pop", pop);
        try {
            const { encryptedBytes, iv } = aesCrypto(pop, aesKeyRef.current);
            const dataToSend = new Uint8Array([...iv, ...encryptedBytes]);
            const data = Buffer.from(dataToSend).toString('base64');
            await bleManager.writeCharacteristicWithResponseForDevice(
                bleThingIdRef.current,
                PROVISION_SERVICE_UUID,
                PROVISION_CHAR_WRITE_POP_UUID,
                data
            );
            await new Promise((resolve) => {
                eventEmitterRef.current.once("provisionProgress", resolve);
            });
        } catch (e) {
            console.log("Pop failed", e);
            setScreenStateTop(ScreenStateTop.BLE_ERROR_MODAL);
        }
    };

    const pemToStrArray = (certificate) => {
        console.log("pemToStrArray");
        // Adding a newline at the end of each line, and split by line
        return certificate.split("\n").map((line) => line + "\n");
    };

    const fetchAwsThingCertificate = async () => {
        try {
            const urlSigned = await Storage.get("auth_aws_ota_thing_cert.pem", {
                download: false,
                level: "public",
            });
            const response = await fetch(urlSigned);
            const text = await response.text();
            return text;
        } catch (e) {
            console.log("Error fetchAwsThingCertificate", e);
            return;
        }
    };

    const fetchAwsThingKey = async () => {
        try {
            const urlSigned = await Storage.get("auth_aws_ota_thing_key.pem", {
                download: false,
                level: "public",
            });
            const response = await fetch(urlSigned);
            const text = await response.text();
            return text;
        } catch (error) {
            console.log("Error fetchAwsThingKey", error);
            throw error;
        }
    };

    const fetchAwsRootCa = async () => {
        try {
            const response = await fetch(
                "https://www.amazontrust.com/repository/AmazonRootCA1.pem"
            );
            const text = await response.text();
            return text; // Returns the contents of the certificate
        } catch (error) {
            console.error("Error:", error);
            throw error; // re-throw the error so it can be caught and handled where this function is called
        }
    };

    const bleSendAwsThingKey = async () => {
        // Instead of splitting by newlines, simply use the already split array
        const pem = await fetchAwsThingKey();
        const rows = pemToStrArray(pem);
        const rowsLength = rows.length;

        for (let index = 0; index < rowsLength; index++) {
            const isLastRow = index === rowsLength - 1;
            const dataJson = {
                type: "aws_thing_key",
                ready: isLastRow ? 1 : 0,
                row: rows[index],
            };
            const dataStr = JSON.stringify(dataJson);
            const { encryptedBytes, iv } = aesCrypto(dataStr, aesKeyRef.current);
            const dataToSend = new Uint8Array([...iv, ...encryptedBytes]);
            const data = Buffer.from(dataToSend).toString('base64');

            await bleManager.writeCharacteristicWithResponseForDevice(
                bleThingIdRef.current,
                PROVISION_SERVICE_UUID,
                PROVISION_CHAR_WRITE_THING_KEY_UUID,
                data
            );
            await new Promise((resolve) => {
                eventEmitterRef.current.once("provisionProgress", resolve);
            });
        }
    };

    const bleSendAwsThingCertificate = async () => {
        // Instead of splitting by newlines, simply use the already split array
        const pem = await fetchAwsThingCertificate();
        const rows = pemToStrArray(pem);
        const rowsLength = rows.length;

        for (let index = 0; index < rowsLength; index++) {
            const isLastRow = index === rowsLength - 1;
            const dataJson = {
                type: "aws_thing_certificate",
                ready: isLastRow ? 1 : 0,
                row: rows[index],
            };

            const dataStr = JSON.stringify(dataJson);
            const { encryptedBytes, iv } = aesCrypto(dataStr, aesKeyRef.current);
            const dataToSend = new Uint8Array([...iv, ...encryptedBytes]);
            const data = Buffer.from(dataToSend).toString('base64');
            await bleManager.writeCharacteristicWithResponseForDevice(
                bleThingIdRef.current,
                PROVISION_SERVICE_UUID,
                PROVISION_CHAR_WRITE_THING_CERT_UUID,
                data
            );
            await new Promise((resolve) => {
                eventEmitterRef.current.once("provisionProgress", resolve);
            });
        }
    };

    const bleSendAwsRootCa = async () => {
        // Instead of splitting by newlines, simply use the already split array
        const pem = await fetchAwsRootCa();
        const rows = pemToStrArray(pem);
        const rowsLength = rows.length;

        for (let index = 0; index < rowsLength; index++) {
            const isLastRow = index === rowsLength - 1;
            const dataJson = {
                type: "aws_root_ca",
                ready: isLastRow ? 1 : 0,
                row: rows[index],
            };
            console.log("dataJson:", dataJson);
            const dataStr = JSON.stringify(dataJson);
            const { encryptedBytes, iv } = aesCrypto(dataStr, aesKeyRef.current);
            const dataToSend = new Uint8Array([...iv, ...encryptedBytes]);
            const data = Buffer.from(dataToSend).toString('base64');
            await bleManager.writeCharacteristicWithResponseForDevice(
                bleThingIdRef.current,
                PROVISION_SERVICE_UUID,
                PROVISION_CHAR_WRITE_ROOT_CA_UUID,
                data
            );
            await new Promise((resolve) => {
                eventEmitterRef.current.once("provisionProgress", resolve);
            });
        }
    };

    const bleSendWifiCredentials = async () => {
        console.log("bleSendWifiCredentials");
        const dataJson = {
            type: "wifi_credentials",
            ssid: wifiSSIDRef.current,
            password: wifiPassword,
        };
        const dataStr = JSON.stringify(dataJson);
        const { encryptedBytes, iv } = aesCrypto(dataStr, aesKeyRef.current);
        const dataToSend = new Uint8Array([...iv, ...encryptedBytes]);
        const data = Buffer.from(dataToSend).toString('base64');

        await bleManager.writeCharacteristicWithResponseForDevice(
            bleThingIdRef.current,
            PROVISION_SERVICE_UUID,
            PROVISION_CHAR_WRITE_WIFI_CREDS_UUID,
            data
        );
        await new Promise((resolve) => {
            eventEmitterRef.current.once("provisionProgress", resolve);
        });
    };

    const sanityCheckThingId = (data) => {
        return data.substring(0, 2) === "id" && data.length === 14;
    };

    const handleBarCodeScanned = async ({ type, data }) => {
        console.log("QR scanned: ", type, data);
        if (sanityCheckThingId(data)) {
            setScreenStateTop(ScreenStateTop.LOADING);
            setScreenStateBottom(ScreenStateBottom.NONE);
            bleThingNameRef.current = data;
            await bleScanForThing();
        } else {
            setScreenStateBottom(ScreenStateBottom.BARCODE_SCAN);
            setScreenStateTop(ScreenStateTop.QR_ERROR_MODAL);
        }
    };

    const handleNetworkSelection = (ssid) => {
        console.log("network chosen: ", ssid);
        wifiSSIDRef.current = ssid;
        setScreenStateTop(ScreenStateTop.PASSWORD_MODAL);
        setScreenStateBottom(ScreenStateBottom.NETWORK_DONE);
    };

    const provisionThing = async () => {
        setScreenStateTop(ScreenStateTop.LOADING);
        setScreenStateBottom(ScreenStateBottom.NONE);
        try {
            console.log("AES key", aesKeyRef.current);
            console.log("Wifi SSID", wifiSSIDRef.current);
            console.log("Wifi password", wifiPassword);

            await bleSendWifiCredentials();
            console.log("Wifi credentials sent");
            await bleSendAwsRootCa();
            console.log("AWS root CA sent");
            await bleSendAwsThingCertificate();
            console.log("AWS thing certificate sent");
            await bleSendAwsThingKey();
            console.log("AWS thing key sent");
        } catch (error) {
            console.log("Provisioning failed", error);
            setScreenStateTop(ScreenStateTop.BLE_ERROR_MODAL);
            // TODO: Disconnect BLE and enable BARCODE_SCAN
        }
    };

    return (
        <>
            <View style={styles.containerScreen}>
                <View style={styles.containerHeader}>
                    <TouchableOpacity onPress={() => navigation.goBack()}>
                        <Ionicons
                            name={"chevron-back-circle"}
                            size={iconSize.large}
                            style={{ color: colors.colorFocus }}
                        />
                    </TouchableOpacity>
                    <Text style={styles.textHeader}>Scan QR</Text>
                    <Ionicons
                        name={"qr-code-outline"}
                        size={iconSize.large}
                        style={{ color: colors.colorBrighter }}
                    />
                </View>

                {screenStateBottom === ScreenStateBottom.BARCODE_SCAN && (
                    <BarCodeScanner
                        onBarCodeScanned={handleBarCodeScanned}
                        style={styles.containerCamera}
                    />
                )}
                {screenStateBottom === ScreenStateBottom.NETWORK_LIST && (
                    <ScrollView>
                        {networkList.map((network) => (
                            <NetworkScrollList
                                ssid={network.ssid}
                                rssi={network.rssi}
                                done={false}
                                onPress={handleNetworkSelection}
                                key={uuidv4()}
                            />
                        ))}
                    </ScrollView>
                )}
                {screenStateBottom === ScreenStateBottom.NETWORK_DONE && (
                    <ScrollView>
                        {networkList.map((network) => (
                            <NetworkScrollList
                                ssid={network.ssid}
                                rssi={network.rssi}
                                done={true}
                                onPress={handleNetworkSelection}
                                key={uuidv4()}
                            />
                        ))}
                    </ScrollView>
                )}
            </View>

            {screenStateTop === ScreenStateTop.LOADING && <AppLoading />}
            {screenStateTop === ScreenStateTop.PASSWORD_MODAL && (
                <PasswordInputModal
                    onClose={() => setScreenStateTop(ScreenStateTop.NONE)}
                    onSubmit={() => provisionThing()}
                    wifiSSID={wifiSSIDRef.current}
                    wifiPassword={wifiPassword}
                    setWifiPassword={setWifiPassword}
                    wifiPasswordRef={wifiPasswordRef}
                />
            )}
            {screenStateTop === ScreenStateTop.CAMERA_ERROR_MODAL && (
                <AppPopupError
                    onClose={() => navigation.goBack()}
                    textMessage={
                        "Unable to open the camera. Try again or restart the app."
                    }
                    textButton={"Okay"}
                />
            )}
            {screenStateTop === ScreenStateTop.BLE_ERROR_MODAL && (
                <AppPopupError
                    onClose={() => {
                        setScreenStateTop(ScreenStateTop.NONE);
                        setScreenStateBottom(ScreenStateBottom.BARCODE_SCAN);
                        setWifiPassword("");
                    }}
                    textMessage={"Something went wrong..."}
                    textButton={"Try again"}
                />
            )}
            {screenStateTop === ScreenStateTop.QR_ERROR_MODAL && (
                <AppPopupError
                    onClose={() => {
                        setScreenStateTop(ScreenStateTop.NONE);
                        setScreenStateBottom(ScreenStateBottom.BARCODE_SCAN);
                    }}
                    textMessage={"This is not a QR code we're familiar with."}
                    textButton={"Try again"}
                />
            )}
            {screenStateTop === ScreenStateTop.BLE_PROVISIONED_MODAL && (
                <AppPopupSuccess
                    onClose={() => {
                        setWifiPassword("");
                        setScreenStateTop(ScreenStateTop.NONE);
                        setScreenStateBottom(ScreenStateBottom.NONE);
                        navigation.goBack();
                    }}
                    textMessage={"You can now return to Home"}
                    textButton={"Okay"}
                />
            )}
        </>
    );
};

const styles = StyleSheet.create({
    containerScreen: {
        flex: 1,
        backgroundColor: colors.colorDarkest,
    },
    loadingOverlay: {
        position: "absolute",
        top: 0,
        left: 0,
        right: 0,
        bottom: 0,
        backgroundColor: "rgba(0, 0, 0, 0.4)", // Change the opacity value if you want the background to be darker or lighter
        justifyContent: "center",
        alignItems: "center",
        zIndex: 100, // Make sure the loading overlay is above all other elements
    },
    containerDeviceTopLayerOffline: {
        color: colors.colorDarkest,
        opacity: 0.4,
    },
    containerDeviceTopLayerOnline: {},
    iconPrimary: {
        color: colors.colorFocus,
    },
    containerCamera: {
        position: "absolute",
        left: 0,
        right: 0,
        top: 120,
        bottom: 0,
    },
    containerScreen: {
        flex: 1,
        backgroundColor: colors.colorDarkest,
    },
    containerDevice: {
        flexDirection: "row",
        alignItems: "center",
    },
    containerWifiList: {
        backgroundColor: colors.colorDarker,
        flexDirection: "row",
        marginHorizontal: 10,
        marginVertical: 5,
        borderRadius: 20,
        paddingVertical: 10,
        paddingHorizontal: 15,
        alignItems: "center",
        justifyContent: "space-between",

        shadowColor: "#000000",
        shadowOffset: {
            width: 0,
            height: 0,
        },
        shadowOpacity: 0.3,
        shadowRadius: 5,
        elevation: 5,
    },
    containerHeader: {
        marginVertical: 50,
        marginHorizontal: 10,
        flexDirection: "row",
        backgroundColor: colors.colorDarkest,
        justifyContent: "space-between",
        alignItems: "center",
    },
    containerModalPosition: {
        flex: 1,
        justifyContent: "center",
    },
    containerModal: {
        flexDirection: "column",
        margin: 15,
        backgroundColor: colors.colorDarker,
        width: width - 30,
        borderRadius: 25,
        padding: 15,
        shadowColor: "#000",
        shadowOffset: {
            //width: 2,
            //height: 5,
        },
        shadowOpacity: 0.25,
        shadowRadius: 5,
        elevation: 5,
    },
    textMedium: {
        fontFamily: fonts.fontPrimary,
        fontSize: 16,
        color: colors.colorWhite,
        marginRight: 10,
        marginLeft: 10,
    },
    containerModalHeader: {
        flexDirection: "row",
        justifyContent: "space-between",
        paddingHorizontal: 3,
        alignItems: "center",
    },
    textInputField: {
        alignSelf: "center",
        marginTop: 10,
        marginBottom: 20,
        borderRadius: 25,
        justifyContent: "center",
        alignItems: "center",
        width: 0.8 * width,
        backgroundColor: colors.colorBrighter,
        paddingBottom: 13,
        paddingTop: 13,
        paddingLeft: 20,
        fontSize: 18,
        color: colors.colorWhite,
    },
    textHeader: {
        fontFamily: fonts.fontPrimary,
        fontSize: 30,
        fontWeight: "bold",
        color: colors.colorWhite,
        marginRight: 10,
        marginLeft: 10,
    },
    textMedium: {
        fontFamily: fonts.fontPrimary,
        fontSize: 16,
        color: colors.colorWhite,
        textAlign: "center",
    },
});

export default ProvisionScreen;
