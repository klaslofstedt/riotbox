import {
    StyleSheet,
    View,
    Text,
    TouchableOpacity,
    TouchableWithoutFeedback,
    Keyboard,
    SafeAreaView,
} from "react-native";
import { useState, useRef } from "react";
import { Auth } from "aws-amplify";
import { colors, width, fontWeight, fonts, fontSize } from "../styles/Styles";
import Ionicons from "react-native-vector-icons/Ionicons";
import { AppLoading } from "../components/AppLoading";
import { AppTextInput } from "../components/AppTextInput";
import { useAuth } from "../contexts/AuthContext";
import { AppPopupError } from "../components/AppPopupError";
import { useNavigation } from "@react-navigation/native";


const ScreenStateTop = {
    NONE: "none",
    NO_USER_ERR: "noUserErr",
    WRONG_PASSWORD_ERR: "wrongPasswordErr",
    GENERAL_ERR: "generalErr"
};

const SignInScreen = () => {
    const navigation = useNavigation();
    const [username, setUsername] = useState("");
    const [password, setPassword] = useState("");
    const usernameInputRef = useRef(null);
    const passwordInputRef = useRef(null);
    const [loading, setLoading] = useState(false);
    const { checkAuthState } = useAuth();
    const [screenStateTop, setScreenStateTop] = useState(
        ScreenStateTop.NONE
    );

    const signIn = async () => {
        setLoading(true);
        Keyboard.dismiss();
        try {
            await Auth.signIn(username, password);
            console.log("✅ Success");
            checkAuthState();
        } catch (error) {
            console.log("❌ Error signing in...", error);
    
            switch (error.code) {
                case 'UserNotFoundException':
                    setScreenStateTop(ScreenStateTop.NO_USER_ERR);
                    break;
                case 'NotAuthorizedException':
                    setScreenStateTop(ScreenStateTop.WRONG_PASSWORD_ERR);
                    break;
                default:
                    setScreenStateTop(ScreenStateTop.GENERAL_ERR);
                    break;
            }
        }
        setLoading(false);
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
                        <Ionicons
                            name={"person"}
                            size={0.5 * width}
                            style={{ color: colors.colorBrighter }}
                        />
                        <AppTextInput
                            inputRef={usernameInputRef}
                            value={username}
                            setValue={setUsername}
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
                        <TouchableOpacity
                            style={styles.buttonForgotPassword}
                            onPress={() => navigation.navigate("ResetPasswordScreen")}
                        >
                            <Text style={styles.textForgotPassword}>
                                Forgot password?
                            </Text>
                        </TouchableOpacity>
                        <TouchableWithoutFeedback onPress={signIn}>
                            <View style={styles.buttonSignIn}>
                                <Text style={styles.textSignIn}>Login</Text>
                            </View>
                        </TouchableWithoutFeedback>
                        <TouchableOpacity
                            style={styles.buttonFooter}
                            onPress={() => navigation.navigate("SignUpScreen")}
                        >
                            <Text style={styles.textFooter}>
                                Don't have an account? Sign Up
                            </Text>
                        </TouchableOpacity>
                    </View>
                </SafeAreaView>
            </TouchableWithoutFeedback>
            {loading && <AppLoading />}
            {screenStateTop === ScreenStateTop.NO_USER_ERR && (
                <AppPopupError
                    onClose={() => setScreenStateTop(ScreenStateTop.NONE)}
                    textMessage={
                        "No user with this email"
                    }
                    textButton={"Close"}
                />
            )}
            {screenStateTop === ScreenStateTop.WRONG_PASSWORD_ERR && (
                <AppPopupError
                    onClose={() => setScreenStateTop(ScreenStateTop.NONE)}
                    textMessage={
                        "Wrong password"
                    }
                    textButton={"Try Again"}
                />
            )}
            {screenStateTop === ScreenStateTop.GENERAL_ERR && (
                <AppPopupError
                    onClose={() => setScreenStateTop(ScreenStateTop.NONE)}
                    textMessage={
                        "Error Logging in"
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
    buttonSignIn: {
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
    textSignIn: {
        fontFamily: fonts.fontPrimary,
        color: colors.colorWhite,
        fontSize: fontSize.medium,
        fontWeight: fontWeight.bold,
        textTransform: "uppercase",
    },
    textFooter: {
        fontFamily: fonts.fontPrimary,
        color: colors.colorFocus,
        fontSize: fontSize.medium,
        fontWeight: fontWeight.bold,
    },
    buttonForgotPassword: {
        marginBottom: 60,
        marginTop: 10,
        justifyContent: "center",
        alignItems: "center",
    },
    textForgotPassword: {
        fontFamily: fonts.fontPrimary,
        color: colors.colorBrightest,
        fontSize: fontSize.medium,
        fontWeight: fontWeight.medium,
    },
});

export default SignInScreen;
