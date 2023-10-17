import {
    Text,
    View,
    TouchableOpacity,
    Modal,
    StyleSheet,
} from "react-native";
import Ionicons from "react-native-vector-icons/Ionicons";
import { iconSize, colors, width, fonts } from "../styles/Styles";
import { AppTextInput } from "./AppTextInput";

export const AppPopupEdit = ({
    openPopup,
    closePopup,
    updateThingNickname,
    setTempThingNickname,
    tempThingNickname,
    deleteThing,
    placeholderTextColor,
    inputRef,
}) => {
    return (
        <View style={styles.overlay}>
            <Modal
                animationType="slide"
                transparent={true}
                visible={true}
                hasBackdrop={true}
                backdropOpacity={10}
                backdropColor="black"
                onRequestClose={() => openPopup()}
            >
                <View style={styles.containerModalPosition}>
                    <View style={styles.containerModal}>
                        <View style={styles.containerModalHeader}>
                            <TouchableOpacity onPress={() => closePopup()}>
                                <Ionicons
                                    name={"close-circle"}
                                    size={iconSize.medium}
                                    style={{
                                        color: colors.colorBrighter,
                                    }}
                                />
                            </TouchableOpacity>
                            <Text style={styles.textMedium}>Edit</Text>
                            <TouchableOpacity
                                onPress={() => updateThingNickname()}
                            >
                                <Ionicons
                                    name={"checkmark-circle"}
                                    size={iconSize.medium}
                                    style={{ color: colors.colorFocus }}
                                />
                            </TouchableOpacity>
                        </View>
                        <AppTextInput
                            inputRef={inputRef}
                            value={tempThingNickname}
                            setValue={setTempThingNickname}
                            placeholder={"Name your thing"}
                            isSecure={false}
                            keyboardType={"default"}
                        />
                        <Text style={styles.textSmall}>
                            Note: Add a deleted thing by scanning the QR code
                            again.
                        </Text>
                        <TouchableOpacity
                            style={styles.buttonDelete}
                            onPress={() => deleteThing()}
                        >
                            <Text style={styles.textDelete}>Delete</Text>
                        </TouchableOpacity>
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
    textMedium: {
        fontFamily: fonts.fontPrimary,
        fontSize: 16,
        color: colors.colorWhite,
        textAlign: "center",
    },
    textDelete: {
        fontFamily: fonts.fontPrimary,
        color: colors.colorWhite,
        fontSize: 18,
        fontWeight: "600",
        textTransform: "uppercase",
    },
    textSmall: {
        fontFamily: fonts.fontPrimary,
        fontSize: 10,
        color: colors.colorBrighter,
        textAlign: "center",
    },
    buttonDelete: {
        marginVertical: 10,
        borderRadius: 25,
        justifyContent: "center",
        alignItems: "center",
        padding: 12,
        width: 0.8 * width,
        backgroundColor: colors.colorRed,
        alignSelf: "center",
    },
});
