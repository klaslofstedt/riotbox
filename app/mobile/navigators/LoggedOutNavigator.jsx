import { createStackNavigator } from "@react-navigation/stack";
import SignInScreen from "../screens/SignInScreen";
import SignUpScreen from "../screens/SignUpScreen";
import VerifySignUpScreen from "../screens/VerifySignUpScreen";
import ResetPasswordScreen from "../screens/ResetPasswordScreen";
import { colors } from "../styles/Styles";
import { StyleSheet } from "react-native";

const LoggedOutStackNavigator = createStackNavigator();

const LoggedOutNavigator = () => {
    return (
        <LoggedOutStackNavigator.Navigator screenOptions={styles.tabBar}>
            <LoggedOutStackNavigator.Screen name="SignInScreen" component={SignInScreen} />
            <LoggedOutStackNavigator.Screen name="SignUpScreen" component={SignUpScreen} />
            <LoggedOutStackNavigator.Screen
                name="VerifySignUpScreen"
                component={VerifySignUpScreen}
            />
            <LoggedOutStackNavigator.Screen
                name="ResetPasswordScreen"
                component={ResetPasswordScreen}
            />
        </LoggedOutStackNavigator.Navigator>
    );
};

const styles = StyleSheet.create({
    tabBar: {
        tabBarStyle: {
            backgroundColor: colors.colorDarker,
            borderTopWidth: 0,
        },
        tabBarActiveTintColor: colors.colorFocus,
        tabBarInactiveTintColor: colors.colorBrighter,
        headerShown: false,
    },
});

export default LoggedOutNavigator;
