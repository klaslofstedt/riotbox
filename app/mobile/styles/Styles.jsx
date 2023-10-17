import { Dimensions } from "react-native";

export const colors = {
    colorDarkest: "#222222",
    colorDarker: "#2f2f2f",
    colorBrighter: "#505050",
    colorBrightest: "#aaaaaa",
    colorFocus: "tomato",
    colorWhite: "#ffffff",
    colorRed: "#BD3025",
    colorGreen: "#449e51",
    colorYellow: "#f5d50a",
    colorOrange: "#db640f",
};

export const iconSize = {
    small: 25, // Tabs (home, me)
    medium: 40, // Device list and popup buttons
    large: 48, // Header buttons
};

export const fonts = {
    fontPrimary: "PingFangHK-Semibold",
};

export const fontWeight = {
    thin: "100",
    medium: "400",
    bold: "600",
};

export const fontSize = {
    largest: 30,
    large: 24,
    medium: 18,
    small: 16,
    smallest: 10,
};
export const { height, width } = Dimensions.get("window");
