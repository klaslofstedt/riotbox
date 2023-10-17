import * as React from "react";
import HomeScreen from "../screens/HomeScreen";
import ThingScreen from "../screens/ThingScreen";
import ProvisionScreen from "../screens/ProvisionScreen";
import { createNativeStackNavigator } from "@react-navigation/native-stack";

const HomeStackNavigator = createNativeStackNavigator();

const HomeNavigator = () => {
    return (
        <HomeStackNavigator.Navigator>
            <HomeStackNavigator.Screen
                name="HomeScreen"
                component={HomeScreen}
                options={{ headerShown: false }}
            />
            <HomeStackNavigator.Screen
                name="ThingScreen"
                component={ThingScreen}
                options={{ headerShown: false }}
            />
            <HomeStackNavigator.Screen
                name="ProvisionScreen"
                component={ProvisionScreen}
                options={{ headerShown: false }}
            />
        </HomeStackNavigator.Navigator>
    );
};

export default HomeNavigator;
