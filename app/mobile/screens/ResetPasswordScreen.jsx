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
import { fonts, colors, fontWeight, fontSize } from "../styles/Styles";
import { AppTextInput } from "../components/AppTextInput";
import { useNavigation } from "@react-navigation/native";
import { AppPopupError } from "../components/AppPopupError";
import { AppPopupSuccess } from "../components/AppPopupSuccess";


const ScreenStateTop = {
    NONE: "none",
    SUCCESSFUL: "successful",
    NO_USER_ERR: "noUserErr",
    CODE_MISMATCH_ERR: "codeMismatchErr",
    INVALID_PASSWORD_ERR: "invalidPasswordErr",
    LIMIT_EXCEEDED_ERR: "limitExceededErr",
    GENERAL_ERR: "generalErr",
};

const ResetPasswordScreen = () => {
    const navigation = useNavigation();
    const [step, setStep] = useState(1); // 1 = request code, 2 = submit new password
    const [username, setUsername] = useState("");
    const [authCode, setAuthCode] = useState("");
    const [newPassword, setNewPassword] = useState("");
    const [screenStateTop, setScreenStateTop] = useState(
        ScreenStateTop.NONE
    );
    const emailInputRef = useRef(null);
    const authInputRef = useRef(null);
    const passwordInputRef = useRef(null);

    const requestPasswordResetCode = async () => {
        try {
            Keyboard.dismiss();
            await Auth.forgotPassword(username);
            console.log("✅ Password reset code sent");
            setStep(2);
        } catch (error) {
            console.log("❌ Error requesting password reset", error);

            switch (error.code) {
                case 'UserNotFoundException':
                    setScreenStateTop(ScreenStateTop.NO_USER_ERR);
                    break;
                case 'LimitExceededException':
                    setScreenStateTop(ScreenStateTop.LIMIT_EXCEEDED_ERR);
                    break;
                default:
                    setScreenStateTop(ScreenStateTop.GENERAL_ERR);
                    break;
            }
        }
    };

    const submitNewPassword = async () => {
        try {
            Keyboard.dismiss();
            await Auth.forgotPasswordSubmit(username, authCode, newPassword);
            console.log("✅ Password has been reset");
            setScreenStateTop(ScreenStateTop.SUCCESSFUL);
        } catch (error) {
            console.log("❌ Error resetting password", error);
    
            switch (error.code) {
                case 'CodeMismatchException':
                    setScreenStateTop(ScreenStateTop.CODE_MISMATCH_ERR);
                    break;
                case 'InvalidPasswordException':
                    setScreenStateTop(ScreenStateTop.INVALID_PASSWORD_ERR);
                    break;
                case 'LimitExceededException':
                    setScreenStateTop(ScreenStateTop.LIMIT_EXCEEDED_ERR);
                    break;
                default:
                    setScreenStateTop(ScreenStateTop.GENERAL_ERR);
                    break;
            }
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
                        {step === 1 ? (
                            <>
                                <Text style={styles.textTitle}>
                                    Password Reset
                                </Text>
                                <AppTextInput
                                    inputRef={emailInputRef}
                                    value={username}
                                    setValue={setUsername}
                                    placeholder={"Enter email"}
                                    isSecure={false}
                                    icon={"mail"}
                                    keyboardType={"email-address"}
                                />
                                <TouchableOpacity
                                    style={styles.buttonReset}
                                    onPress={requestPasswordResetCode}
                                >
                                    <Text style={styles.textReset}>
                                        Send verification code
                                    </Text>
                                </TouchableOpacity>
                            </>
                        ) : (
                            <>
                                <Text style={styles.textTitle}>
                                    Password Reset
                                </Text>
                                <AppTextInput
                                    inputRef={authInputRef}
                                    value={authCode}
                                    setValue={setAuthCode}
                                    placeholder={"Enter verification code"}
                                    isSecure={false}
                                    icon={"shield-checkmark"}
                                    keyboardType={"numeric"}
                                />
                                <AppTextInput
                                    inputRef={passwordInputRef}
                                    value={newPassword}
                                    setValue={setNewPassword}
                                    placeholder={"Enter new password"}
                                    isSecure={true}
                                    icon={"lock-closed"}
                                    keyboardType={"default"}
                                />
                                <TouchableOpacity
                                    style={styles.buttonReset}
                                    onPress={submitNewPassword}
                                >
                                    <Text style={styles.textReset}>
                                        Set new password
                                    </Text>
                                </TouchableOpacity>
                            </>
                        )}
                    </View>
                </SafeAreaView>
            </TouchableWithoutFeedback>
            {screenStateTop === ScreenStateTop.SUCCESSFUL && (
                <AppPopupSuccess
                    onClose={() => {
                        setScreenStateTop(ScreenStateTop.NONE);
                        navigation.navigate("SignInScreen");
                    }}
                    textMessage={"Password successfully set"}
                    textButton={"Okay"}
                />
            )}
            {screenStateTop === ScreenStateTop.LIMIT_EXCEEDED_ERR && (
                <AppPopupError
                    onClose={() => setScreenStateTop(ScreenStateTop.NONE)}
                    textMessage={"Attemp limit exceeded\nPlease try again later"}
                    textButton={"Okay"}
                />
            )}
            {screenStateTop === ScreenStateTop.CODE_MISMATCH_ERR && (
                <AppPopupError
                    onClose={() => setScreenStateTop(ScreenStateTop.NONE)}
                    textMessage={"Invalid Verification code"}
                    textButton={"Try Again"}
                />
            )}
            {screenStateTop === ScreenStateTop.NO_USER_ERR && (
                <AppPopupError
                    onClose={() => setScreenStateTop(ScreenStateTop.NONE)}
                    textMessage={
                        "No user with this email"
                    }
                    textButton={"Close"}
                />
            )}
            {screenStateTop === ScreenStateTop.INVALID_PASSWORD_ERR && (
                <AppPopupError
                    onClose={() => setScreenStateTop(ScreenStateTop.NONE)}
                    textMessage={
                        "Invalid password:\n- At least 8 characters -\n- Requires lower case  -\n- Requires digits         -"
                    }
                    textButton={"Try Again"}
                />
            )}
            {screenStateTop === ScreenStateTop.WRONG_CODE_ERR && (
                <AppPopupError
                    onClose={() => setScreenStateTop(ScreenStateTop.NONE)}
                    textMessage={
                        "Wrong verification code"
                    }
                    textButton={"Try Again"}
                />
            )}
            {screenStateTop === ScreenStateTop.GENERAL_ERR && (
                <AppPopupError
                    onClose={() => setScreenStateTop(ScreenStateTop.NONE)}
                    textMessage={
                        "Unknown error"
                    }
                    textButton={"Try Again"}
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
    buttonReset: {
        marginVertical: 15,
        justifyContent: "center",
        alignItems: "center",
    },
    textInput: {
        fontFamily: fonts.fontPrimary,
        color: colors.colorWhite,
        fontSize: fontSize.medium,
        fontWeight: fontWeight.medium,
        marginLeft: 10,
    },
    textReset: {
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

export default ResetPasswordScreen;