import { useEffect, useState } from "react";
import {
    View,
    Text,
    TouchableOpacity,
    StyleSheet,
    RefreshControl,
} from "react-native";
import { Auth } from "aws-amplify";
import { ScrollView } from "react-native-gesture-handler";
import { useNavigation } from "@react-navigation/native";
import Ionicons from "react-native-vector-icons/Ionicons";
import { iconSize, colors, fonts } from "../styles/Styles";
import { apiThingsList, apiThingValueChange } from "../graphql/GraphqlApi";
import { AppLoading } from "../components/AppLoading";
import useThingsSubscription from "../hooks/useThingsSubscription";
import { getTypeIcon } from "../types/Type";
import { useUserThings } from "../contexts/UserThingsContext";

const HomeScreen = () => {
    const navigation = useNavigation();
    const [user, setUser] = useState(null);
    const [loading, setLoading] = useState(true);
    const [refreshing, setRefreshing] = useState(false);
    const { userThings, setUserThings } = useUserThings();
    useThingsSubscription(user, setUserThings);

    useEffect(() => {
        console.log("useEffect getUser");
        getUser();
    }, []);

    useEffect(() => {
        console.log("useEffect getUserThings");
        getUserThings();
    }, []);

    const onRefresh = () => {
        console.log("Ping devices");
        pingUserThings();
    };

    const getUserThings = async () => {
        try {
            const things = await apiThingsList();

            // Map over things and return a new array without mutating the original one.
            const updatedThings = things.map((thing) => {
                const thingValue = JSON.parse(thing.value);
                thingValue.mobile_value.readwrite.network = "offline";
                thing.value = JSON.stringify(thingValue);
                return thing;
            });

            setUserThings(updatedThings);
            await Promise.all(
                updatedThings.map((thing) =>
                    apiThingValueChange(thing.id, thing.value)
                )
            );
        } catch (error) {
            console.error("Error getting user things:", error);
        } finally {
            setLoading(false);
        }
    };

    const pingUserThings = async () => {
        setRefreshing(true);
        const things = await apiThingsList();
        const updatedThings = things.map((thing) => {
            const thingValue = JSON.parse(thing.value);
            thingValue.mobile_value.readwrite.network = "offline";
            thing.value = JSON.stringify(thingValue);
            return thing;
        });
        setUserThings(updatedThings);

        updatedThings.forEach(async (thing) => {
            await apiThingValueChange(thing.id, thing.value);
        });
        setRefreshing(false);
    };

    const getUser = async () => {
        const currentUser = await Auth.currentAuthenticatedUser();
        setUser(currentUser);
        console.log("Username: ", currentUser.username);
        console.log("Email: ", currentUser.attributes.email);
    };

    const ThingScrollList = ({ thing }) => {
        const navigation = useNavigation();
        const type = thing.type;
        const value = JSON.parse(thing.value);
        const nickname = value.mobile_value.read.nickname;
        const network = value.mobile_value.readwrite.network;

        return (
            <TouchableOpacity
                onPress={() => navigation.navigate("ThingScreen", { thing })}
            >
                <View
                    style={
                        network === "online"
                            ? styles.containerDeviceTopLayerOnline
                            : styles.containerDeviceTopLayerOffline
                    }
                >
                    <View style={styles.containerDeviceList}>
                        <View style={styles.containerDevice}>
                            <Ionicons
                                name={getTypeIcon(type)}
                                size={iconSize.medium}
                                style={{ color: colors.colorFocus }}
                            />
                            <View>
                                <Text style={styles.textMedium}>
                                    {nickname}
                                </Text>
                                <Text style={styles.textSmall}>{type}</Text>
                            </View>
                        </View>

                        <View style={styles.containerDevice}>
                            <Ionicons
                                name={
                                    network === "online"
                                        ? "cloud-done-outline"
                                        : "cloud-offline-outline"
                                }
                                size={iconSize.medium}
                                style={{
                                    color:
                                        network === "online"
                                            ? colors.colorGreen
                                            : colors.colorBrighter,
                                }}
                            />
                        </View>
                    </View>
                </View>
            </TouchableOpacity>
        );
    };

    return (
        <>
            <View style={styles.containerScreen}>
                <View style={styles.containerHeader}>
                    <Text style={styles.textHeader}>Home</Text>
                    <TouchableOpacity
                        onPress={() => navigation.navigate("ProvisionScreen")}
                    >
                        <Ionicons
                            name={"add-circle"}
                            size={iconSize.large}
                            style={{ color: colors.colorFocus }}
                        />
                    </TouchableOpacity>
                </View>

                <ScrollView
                    refreshControl={
                        <RefreshControl
                            refreshing={refreshing}
                            onRefresh={onRefresh}
                            colors={[colors.colorFocus]}
                            tintColor={colors.colorFocus}
                        />
                    }
                >
                    {userThings.map((thing) => {
                        return (
                            <ThingScrollList
                                thing={thing}
                                key={thing.id} // Must have a unique key
                            />
                        );
                    })}
                </ScrollView>
            </View>
            {loading && <AppLoading />}
        </>
    );
};

const styles = StyleSheet.create({
    containerScreen: {
        flex: 1,
        backgroundColor: colors.colorDarkest,
    },
    containerHeader: {
        marginVertical: 50,
        marginHorizontal: 10,
        flexDirection: "row",
        backgroundColor: colors.colorDarkest,
        justifyContent: "space-between",
        alignItems: "center",
    },
    containerDeviceList: {
        backgroundColor: colors.colorDarker,
        flexDirection: "row",
        marginHorizontal: 10,
        marginVertical: 5,
        borderRadius: 20,
        paddingVertical: 20,
        paddingHorizontal: 15,
        alignItems: "center",
        justifyContent: "space-between",

        shadowColor: "#000000",
        shadowOffset: {
            width: 0,
            height: 0,
        },
        shadowOpacity: 0.3,
        shadowRadius: 5,
        elevation: 5,
    },
    containerDevice: {
        flexDirection: "row",
        alignItems: "center",
    },
    containerDeviceTopLayerOffline: {
        color: colors.colorDarkest,
        opacity: 0.4,
    },
    containerDeviceTopLayerOnline: {},
    textHeader: {
        fontFamily: fonts.fontPrimary,
        fontSize: 30,
        fontWeight: "bold",
        color: colors.colorWhite,
        marginRight: 10,
        marginLeft: 10,
    },
    textMedium: {
        fontFamily: fonts.fontPrimary,
        fontSize: 16,
        color: colors.colorWhite,
        marginRight: 10,
        marginLeft: 10,
    },
    textSmall: {
        fontFamily: fonts.fontPrimary,
        fontSize: 10,
        color: colors.colorBrighter,
        marginRight: 10,
        marginLeft: 10,
    },
});

export default HomeScreen;
