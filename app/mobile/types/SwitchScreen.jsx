import { Text, TouchableOpacity, StyleSheet, View } from "react-native";
import { colors, width, fonts } from "../styles/Styles";
import useUpdateValue from '../hooks/useUpdateValue';

const SwitchScreen = ({ thingId }) => {
    const [isUpdatingValue, setUpdateValue, thingValue] = useUpdateValue(thingId);
    
    if (!thingValue) return null;
    const thingStatus = thingValue.readwrite.status;

    const getButtonText = () => {
        if (isUpdatingValue) {
            return `Turning ${thingStatus ? "On" : "Off"}`;
        } else {
            return thingStatus ? "On" : "Off";
        }
    };

    return (
        <View
            style={
                isUpdatingValue
                ? styles.containerDeviceTopLayerOffline
                : styles.containerDeviceTopLayerOnline
            }
        >
            <TouchableOpacity
                style={styles.button}
                onPress={() => setUpdateValue(!thingStatus)}
                disabled={isUpdatingValue}
            >
                <Text style={styles.buttonText}>{getButtonText()}</Text>
            </TouchableOpacity>
        </View>
    );
};

const styles = StyleSheet.create({
    containerDeviceTopLayerOffline: {
        color: colors.colorDarkest,
        opacity: 0.4,
    },
    containerDeviceTopLayerOnline: {},
    buttonText: {
        fontFamily: fonts.fontPrimary,
        color: colors.colorWhite,
        fontSize: 18,
        fontWeight: "600",
        textTransform: "uppercase",
    },
    button: {
        marginVertical: 10,
        borderRadius: 25,
        justifyContent: "center",
        alignItems: "center",
        padding: 12,
        width: 0.8 * width,
        backgroundColor: colors.colorFocus,
        alignSelf: "center",
    },
});

export default SwitchScreen;