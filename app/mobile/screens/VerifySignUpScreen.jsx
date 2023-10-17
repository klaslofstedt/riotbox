import * as React from "react";
import { Auth } from "aws-amplify";
import {
    StyleSheet,
    View,
    Text,
    SafeAreaView,
    TouchableOpacity,
    TouchableWithoutFeedback,
    Keyboard,
} from "react-native";
import { useState, useRef } from "react";
import { fonts, colors, width, fontWeight, fontSize } from "../styles/Styles";
import { AppTextInput } from "../components/AppTextInput";
import { AppPopupError } from "../components/AppPopupError";
import { AppPopupSuccess } from "../components/AppPopupSuccess";
import { useNavigation } from "@react-navigation/native";


const ScreenStateTop = {
    NONE: "none",
    WRONG_CODE_ERR: "wrongCodeErr",
    SUCCESSFUL: "successful",
};

const VerifySignUpScreen = ({ route }) => {
    const [authCode, setAuthCode] = useState("");
    const authInputRef = useRef(null);
    const navigation = useNavigation();
    const [screenStateTop, setScreenStateTop] = useState(
        ScreenStateTop.NONE
    );
    const username = route.params.email;
    // TODO send another verification code email
    const email = route.params.email;
    const password = route.params.password;

    const verifySignUp = async () => {
        try {
            Keyboard.dismiss();
            await Auth.confirmSignUp(username, authCode);
            console.log("✅ Code confirmed");
            setScreenStateTop(ScreenStateTop.SUCCESSFUL);
        } catch (error) {
            setScreenStateTop(ScreenStateTop.WRONG_CODE_ERR);
            console.log(
                "❌ Verification code does not match. Please enter a valid verification code.",
                error.code
            );
        }
    };

    return (
        <>
            <TouchableWithoutFeedback
                onPress={() => {
                    Keyboard.dismiss();
                }}
            >
                <SafeAreaView style={styles.containerSafeArea}>
                    <View style={styles.containerScreen}>
                        <Text style={styles.textTitle}>Check your email inbox</Text>
                        <AppTextInput
                            inputRef={authInputRef}
                            value={authCode}
                            setValue={setAuthCode}
                            placeholder={"Enter verification code"}
                            isSecure={false}
                            icon={"shield-checkmark"}
                            keyboardType={"numeric"}
                        />
                        <TouchableOpacity
                            style={styles.buttonVerifySignUp}
                            onPress={verifySignUp}
                        >
                            <Text style={styles.textVerifySignUp}>
                                Verify email
                            </Text>
                        </TouchableOpacity>
                    </View>
                </SafeAreaView>
            </TouchableWithoutFeedback>
            {screenStateTop === ScreenStateTop.WRONG_CODE_ERR && (
                <AppPopupError
                    onClose={() => setScreenStateTop(ScreenStateTop.NONE)}
                    textMessage={
                        "Wrong verification code"
                    }
                    textButton={"Try Again"}
                />
            )}
            {screenStateTop === ScreenStateTop.SUCCESSFUL && (
                <AppPopupSuccess
                    onClose={() => {
                        setScreenStateTop(ScreenStateTop.NONE);
                        navigation.navigate("SignInScreen");
                    }}
                    textMessage={"User successfully created"}
                    textButton={"Close"}
                />
            )}
        </>
    );
};

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

export default VerifySignUpScreen;
