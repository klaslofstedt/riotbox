import Ionicons from "react-native-vector-icons/Ionicons";
import { createBottomTabNavigator } from "@react-navigation/bottom-tabs";
import MeScreen from "../screens/MeScreen";
import HomeNavigator from "../navigators/HomeNavigator";
import { iconSize, colors } from "../styles/Styles";
import { StyleSheet } from "react-native";
import { UserThingsProvider } from "../contexts/UserThingsContext";

const LoggedInTabNavigator = createBottomTabNavigator();

const LoggedInNavigator = () => {
    return (
        <UserThingsProvider>
            <LoggedInTabNavigator.Navigator screenOptions={styles.tabBar}>
                <LoggedInTabNavigator.Screen
                    name="Home"
                    component={HomeNavigator}
                    options={{
                        tabBarIcon: ({ color }) => (
                            <Ionicons
                                name={"home"}
                                color={color}
                                size={iconSize.small}
                            />
                        ),
                    }}
                />
                <LoggedInTabNavigator.Screen
                    name="Me"
                    component={MeScreen}
                    options={{
                        tabBarIcon: ({ color }) => (
                            <Ionicons
                                name={"person"}
                                color={color}
                                size={iconSize.small}
                            />
                        ),
                    }}
                />
            </LoggedInTabNavigator.Navigator>
        </UserThingsProvider>
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

export default LoggedInNavigator;
