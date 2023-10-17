import { Text, StyleSheet, View } from "react-native";
import { colors, fonts } from "../styles/Styles";
import useUpdateValue from '../hooks/useUpdateValue';

const DefaultScreen = ({ thingId }) => {
    const [isUpdatingValue, setUpdateValue, thingValue] = useUpdateValue(thingId);
    
    if (!thingValue) return null;

    return (
        <View>
            <Text style={styles.text}>{JSON.stringify(thingValue)}</Text>
        </View>
    );
};

const styles = StyleSheet.create({
    text: {
        fontFamily: fonts.fontPrimary,
        color: colors.colorWhite,
        fontSize: 12,
        fontWeight: "600",
        textTransform: "uppercase",
    },
});

export default DefaultScreen;
