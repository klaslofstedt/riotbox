import { View, Text, TouchableOpacity, StyleSheet } from "react-native";
import { Auth } from "aws-amplify";
import Ionicons from "react-native-vector-icons/Ionicons";
import { useEffect, useState } from "react";
import { colors, width, fontWeight, fonts, fontSize } from "../styles/Styles";
import { useAuth } from "../contexts/AuthContext";

const MeScreen = () => {
    const [user, setUser] = useState(null);
    const { checkAuthState } = useAuth();

    useEffect(() => {
        getUser();
    }, []);

    const getUser = async () => {
        const currentUser = await Auth.currentAuthenticatedUser();
        setUser(currentUser);
        console.log("Username: ", currentUser.username);
        console.log("Email: ", currentUser.attributes.email);
    };

    const signOut = async () => {
        try {
            await Auth.signOut();
            checkAuthState();
        } catch (error) {
            console.log("Error signing out: ", error);
        }
    };

    return (
        <View style={styles.containerMeScreen}>
            <Ionicons
                name={"person"}
                size={(2 / 3) * width}
                style={{ color: colors.colorBrighter }}
            />
            <Text style={styles.textEmail}>
                {user && user.attributes && user.attributes.email}
            </Text>
            <TouchableOpacity
                style={styles.buttonSignOut}
                onPress={() => signOut()}
            >
                <Text style={styles.textSignOut}>Sign out</Text>
            </TouchableOpacity>
        </View>
    );
};

const styles = StyleSheet.create({
    containerMeScreen: {
        flex: 1,
        backgroundColor: colors.colorDarkest,
        alignItems: "center",
        justifyContent: "center",
    },
    textEmail: {
        fontFamily: fonts.fontPrimary,
        color: colors.colorBrighter,
        fontSize: fontSize.medium,
        fontWeight: fontWeight.medium,
        marginBottom: 20,
    },
    textSignOut: {
        fontFamily: fonts.fontPrimary,
        color: colors.colorWhite,
        fontSize: fontSize.medium,
        fontWeight: fontWeight.bold,
        textTransform: "uppercase",
    },
    buttonSignOut: {
        marginVertical: 10,
        borderRadius: 25,
        justifyContent: "center",
        alignItems: "center",
        padding: 12,
        width: 0.6 * width,
        backgroundColor: colors.colorFocus,
    },
});

export default MeScreen;
