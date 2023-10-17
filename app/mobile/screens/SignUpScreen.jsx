import {
    StyleSheet,
    View,
    Text,
    TouchableOpacity,
    SafeAreaView,
    TouchableWithoutFeedback,
    Keyboard,
} from "react-native";
import { useState, useRef } from "react";
import { Auth } from "aws-amplify";
import { colors, width, fontWeight, fonts, fontSize } from "../styles/Styles";
import { useNavigation } from "@react-navigation/native";
import { AppTextInput } from "../components/AppTextInput";
import { AppPopupError } from "../components/AppPopupError";


const ScreenStateTop = {
    NONE: "none",
    USERNAME_EXISTS_ERR: "noUserErr",
    INVALID_PASSWORD_ERR: "invalidPasswordErr",
    INVALID_PARAMETER_ERR: "invalidParameterErr",
    GENERAL_ERR: "generalErr",
};

const SignUpScreen = () => {
    const navigation = useNavigation();
    const [username, setUsername] = useState("");
    const [password, setPassword] = useState("");
    const [email, setEmail] = useState("");

    const emailInputRef = useRef(null);
    const passwordInputRef = useRef(null);

    const setEmailAsUsername = (email) => {
        setUsername(email);
        setEmail(email);
    };
    const [screenStateTop, setScreenStateTop] = useState(
        ScreenStateTop.NONE
    );

    const signUp = async () => {
        try {
            Keyboard.dismiss();
            await Auth.signUp({ username, password, attributes: { email } });
            console.log("✅ Sign-up Verified");
            navigation.navigate("VerifySignUpScreen", { email, password });
        } catch (error) {
            console.log("❌ Error signing up", error);
            switch (error.code) {
                case 'UsernameExistsException':
                    setScreenStateTop(ScreenStateTop.USERNAME_EXISTS_ERR);
                    break;
                case 'InvalidParameterException':
                    setScreenStateTop(ScreenStateTop.INVALID_PARAMETER_ERR);
                    break;
                case 'InvalidPasswordException':
                    setScreenStateTop(ScreenStateTop.INVALID_PASSWORD_ERR);
                    break;
                default:
                    setScreenStateTop(ScreenStateTop.GENERAL_SIGNUP_ERR);
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
                        <Text style={styles.textTitle}>Create a new account</Text>
                        <AppTextInput
                            inputRef={emailInputRef}
                            value={username}
                            setValue={setEmailAsUsername}
                            placeholder={"Enter email"}
                            isSecure={false}
                            icon={"mail"}
                            keyboardType={"email-address"}
                        />
                        <AppTextInput
                            inputRef={passwordInputRef}
                            value={password}
                            setValue={setPassword}
                            placeholder={"Enter password"}
                            isSecure={true}
                            icon={"lock-closed"}
                            keyboardType={"default"}
                        />
                        <TouchableWithoutFeedback onPress={signUp}>
                            <View style={styles.buttonSignUp}>
                                <Text style={styles.textSignUp}>Sign Up</Text>
                            </View>
                        </TouchableWithoutFeedback>
                        <TouchableOpacity
                            style={styles.buttonFooter}
                            onPress={() => navigation.navigate("SignInScreen")}
                        >
                            <Text style={styles.textFooter}>
                                Already have an account? Sign In
                            </Text>
                        </TouchableOpacity>
                    </View>
                </SafeAreaView>
            </TouchableWithoutFeedback>
            {screenStateTop === ScreenStateTop.USERNAME_EXISTS_ERR && (
                <AppPopupError
                    onClose={() => setScreenStateTop(ScreenStateTop.NONE)}
                    textMessage={
                        "A user already exists with this email"
                    }
                    textButton={"Close"}
                />
            )}
            {screenStateTop === ScreenStateTop.INVALID_PARAMETER_ERR && (
                <AppPopupError
                    onClose={() => setScreenStateTop(ScreenStateTop.NONE)}
                    textMessage={
                        "Invalid email address"
                    }
                    textButton={"Try Again"}
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
            {screenStateTop === ScreenStateTop.GENERAL_ERR && (
                <AppPopupError
                    onClose={() => setScreenStateTop(ScreenStateTop.NONE)}
                    textMessage={
                        "Error signing up"
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
    buttonSignUp: {
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
    textSignUp: {
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

export default SignUpScreen;
