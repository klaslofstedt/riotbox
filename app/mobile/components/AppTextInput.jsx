import * as React from "react";
import { StyleSheet, View, TextInput } from "react-native";
import { fonts, colors, width, fontWeight, fontSize } from "../styles/Styles";
import Ionicons from "react-native-vector-icons/Ionicons";

export const AppTextInput = ({
    inputRef,
    value,
    setValue,
    placeholder,
    isSecure,
    icon,
    keyboardType,
}) => (
    <View
        style={styles.containerInputText}
        onStartShouldSetResponder={() => true}
        onResponderGrant={() => {
            inputRef.current.focus();
        }}
    >
        <Ionicons name={icon} size={20} color={colors.colorBrightest} />
        <TextInput
            ref={inputRef}
            selectionColor={colors.colorFocus}
            style={styles.textInput}
            placeholderTextColor={colors.colorBrightest}
            value={value}
            onChangeText={(value) => setValue(value)}
            placeholder={placeholder}
            autoCapitalize="none"
            secureTextEntry={isSecure}
            keyboardType={keyboardType}
        />
    </View>
);

const styles = StyleSheet.create({
    containerScreen: {
        flex: 1,
        alignItems: "center",
    },
    containerSafeArea: {
        flex: 1,
        backgroundColor: colors.colorDarker,
    },
    containerInputText: {
        alignSelf: "center",
        width: 0.8 * width,
        backgroundColor: colors.colorBrighter,
        borderRadius: 25,
        flexDirection: "row",
        paddingVertical: 12,
        paddingHorizontal: 12,
        marginVertical: 10,
        alignItems: "center",
    },
    buttonVerifySignUp: {
        marginVertical: 10,
        borderRadius: 25,
        justifyContent: "center",
        alignItems: "center",
        padding: 12,
        width: 0.8 * width,
        backgroundColor: colors.colorFocus,
    },
    buttonFooter: {
        marginVertical: 15,
        justifyContent: "center",
        alignItems: "center",
    },
    textVerifySignUp: {
        fontFamily: fonts.fontPrimary,
        color: colors.colorWhite,
        fontSize: fontSize.medium,
        fontWeight: fontWeight.bold,
        textTransform: "uppercase",
    },
    textInput: {
        flex: 1,
        fontFamily: fonts.fontPrimary,
        color: colors.colorWhite,
        fontSize: fontSize.medium,
        fontWeight: fontWeight.medium,
        marginLeft: 10,
    },
    textFooter: {
        fontFamily: fonts.fontPrimary,
        color: colors.colorFocus,
        fontSize: fontSize.medium,
        fontWeight: fontWeight.bold,
    },
    textTitle: {
        fontFamily: fonts.fontPrimary,
        color: colors.colorWhite,
        fontSize: fontSize.large,
        fontWeight: fontWeight.bold,
        marginVertical: 15,
    },
});
