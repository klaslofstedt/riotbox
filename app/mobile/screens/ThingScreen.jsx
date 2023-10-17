import React, { useRef, useState } from "react";
import { StyleSheet, Text, View, TouchableOpacity } from "react-native";
import Ionicons from "react-native-vector-icons/Ionicons";
import { useNavigation } from "@react-navigation/native";
import { colors, iconSize, fonts } from "../styles/Styles";
import { apiThingValueChange, apiThingDeprovision } from "../graphql/GraphqlApi";
import { getTypeComponent } from "../types/Type";
import { AppPopupEdit } from "../components/AppPopupEdit";
import { useUserThings } from "../contexts/UserThingsContext";

const ScreenState = {
    THING_OPTIONS: "thingOptions",
    EDIT_POPUP: "editPopup",
};

const ThingScreen = ({ route }) => {
    const navigation = useNavigation();
    const thing = route.params.thing;
    const thingId = thing.id;
    const thingType = thing.type;
    const [screenState, setScreenState] = useState(ScreenState.THING_OPTIONS);
    const thingValue = JSON.parse(thing.value);

    const { setUserThings } = useUserThings();
    const [placeholderTextColor, setPlaceholderTextColor] = useState(
        colors.colorBrightest
    );
    const [thingNickname, setThingNickname] = useState(
        thingValue.mobile_value.read.nickname
    );
    const [tempThingNickname, setTempThingNickname] = useState(
        thingValue.mobile_value.read.nickname
    );
    const nicknameInputRef = useRef(null);

    const updateThingNickname = async () => {
        console.log("tempThingNickname", tempThingNickname);
        console.log("thingNickname", thingNickname);

        try {
            if (tempThingNickname === "") {
                setPlaceholderTextColor(colors.colorRed);
                console.log("DEBUG: Invalid new nickname");
                return;
            }
            if (tempThingNickname === thingNickname) {
                setScreenState(ScreenState.THING_OPTIONS);
                console.log("DEBUG: Same nickname - dont bother updating");
                return;
            }
            setPlaceholderTextColor(colors.colorBrightest);
            setScreenState(ScreenState.THING_OPTIONS);
            setThingNickname(tempThingNickname);
            thingValue.mobile_value.read.nickname = tempThingNickname;
            await apiThingValueChange(thingId, JSON.stringify(thingValue));
        } catch (error) {
            console.log("Error: ", error);
        }
    };

    const deleteThing = async () => {
        setScreenState(ScreenState.THING_OPTIONS);
        try {
            await apiThingDeprovision(thingId);
            setUserThings((prevThings) =>
                prevThings.filter((thing) => thing.id !== thingId)
            );
            navigation.goBack();
        } catch (error) {
            console.log("Error: ", error);
        }
    };

    const ThingTypes = ({ type, id }) => {
        return getTypeComponent(type, id);
    };

    return (
        <View style={styles.containerScreen}>
            {(screenState === ScreenState.THING_OPTIONS ||
                screenState === ScreenState.EDIT_POPUP) && (
                <>
                    <View style={styles.containerHeader}>
                        <TouchableOpacity onPress={() => navigation.goBack()}>
                            <Ionicons
                                name={"chevron-back-circle"}
                                size={iconSize.large}
                                style={{ color: colors.colorFocus }}
                            />
                        </TouchableOpacity>
                        <Text style={styles.textHeader}>{thingNickname}</Text>
                        <TouchableOpacity
                            onPress={() =>
                                setScreenState(ScreenState.EDIT_POPUP)
                            }
                        >
                            <Ionicons
                                name={"list"}
                                size={iconSize.large}
                                style={{ color: colors.colorBrighter }}
                            />
                        </TouchableOpacity>
                    </View>
                    <ThingTypes type={thingType} id={thingId} />
                </>
            )}

            {screenState === ScreenState.EDIT_POPUP && (
                <AppPopupEdit
                    openPopup={() => setScreenState(ScreenState.EDIT_POPUP)}
                    closePopup={() => setScreenState(ScreenState.THING_OPTIONS)}
                    updateThingNickname={updateThingNickname}
                    setTempThingNickname={setTempThingNickname}
                    tempThingNickname={tempThingNickname}
                    deleteThing={() => deleteThing()}
                    placeholderTextColor={placeholderTextColor}
                    inputRef={nicknameInputRef}
                />
            )}
        </View>
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
    textHeader: {
        fontFamily: fonts.fontPrimary,
        fontSize: 30,
        fontWeight: "bold",
        color: colors.colorWhite,
        marginRight: 10,
        marginLeft: 10,
    },
});

export default ThingScreen;
