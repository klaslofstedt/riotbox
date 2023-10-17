import { Text, View, TouchableOpacity, Modal, StyleSheet } from "react-native";
import {
    iconSize,
    colors,
    width,
    fonts,
    fontSize,
    fontWeight,
} from "../styles/Styles";
import Ionicons from "react-native-vector-icons/Ionicons";

export const AppPopupError = ({ onClose, textMessage, textButton }) => {
    console.log("AppPopupError:", textMessage);
    return (
        <View style={styles.overlay}>
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
                        <View style={styles.containerItems}>
                            <Ionicons
                                name={"sad-outline"}
                                size={iconSize.medium}
                                style={{ color: colors.colorBrighter }}
                            />
                            <Text style={styles.textHeader}>{"Oh snap!"}</Text>
                            <Text style={styles.textMessage}>
                                {textMessage}
                            </Text>

                            <TouchableOpacity
                                style={styles.buttomPrimary}
                                onPress={onClose}
                            >
                                <Text style={styles.textButtonPrimary}>
                                    {textButton}
                                </Text>
                            </TouchableOpacity>
                        </View>
                    </View>
                </View>
            </Modal>
        </View>
    );
};

const styles = StyleSheet.create({
    overlay: {
        position: "absolute",
        top: 0,
        left: 0,
        right: 0,
        bottom: 0,
        backgroundColor: "rgba(0, 0, 0, 0.4)",
        justifyContent: "center",
        alignItems: "center",
        zIndex: 100,
    },
    textButtonPrimary: {
        fontFamily: fonts.fontPrimary,
        color: colors.colorWhite,
        fontSize: fontSize.medium,
        fontWeight: fontWeight.bold,
        textTransform: "uppercase",
    },
    buttomPrimary: {
        marginVertical: 10,
        borderRadius: 25,
        justifyContent: "center",
        alignItems: "center",
        padding: 12,
        width: 0.6 * width,
        backgroundColor: colors.colorFocus,
    },
    containerItems: {
        alignItems: "center",
        justifyContent: "center",
    },
    containerModalPosition: {
        flex: 1,
        justifyContent: "center",
    },
    containerModal: {
        flexDirection: "column",
        margin: 50,
        backgroundColor: colors.colorDarker,
        width: width - 100,
        borderRadius: 25,
        padding: 15,
        shadowOpacity: 0.25,
        shadowRadius: 5,
        elevation: 5,
    },
    textHeader: {
        fontFamily: fonts.fontPrimary,
        fontSize: 30,
        fontWeight: "bold",
        color: colors.colorWhite,
        marginRight: 10,
        marginLeft: 10,
    },
    textMessage: {
        fontFamily: fonts.fontPrimary,
        fontWeight: fontWeight.medium,
        fontSize: 16,
        color: colors.colorBrightest,
        textAlign: "center",
    },
});
