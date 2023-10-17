import { View, ActivityIndicator, StyleSheet } from "react-native";
import { colors } from "../styles/Styles";

export const AppLoading = () => {
    return (
        <View style={styles.loadingOverlay}>
            <ActivityIndicator size="large" color={colors.colorFocus} />
        </View>
    );
};

export const styles = StyleSheet.create({
    loadingOverlay: {
        position: "absolute",
        top: 0,
        left: 0,
        right: 0,
        bottom: 0,
        backgroundColor: "rgba(0, 0, 0, 0.4)", // Change the opacity value if you want the background to be darker or lighter
        justifyContent: "center",
        alignItems: "center",
        zIndex: 100, // Make sure the loading overlay is above all other elements
    },
});
