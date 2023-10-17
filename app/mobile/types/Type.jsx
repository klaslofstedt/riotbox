import React from "react";
import DefaultScreen from "./DefaultScreen";
import SwitchScreen from "./SwitchScreen";


export const getTypeIcon = (type) => {
    switch (type) {
        case "SWITCH":
            return "power";
        default:
            return "help";
    }
};

export const getTypeComponent = (type, id) => {
    switch (type) {
        case "SWITCH":
            return <SwitchScreen thingId={id} />;
        default:
            return <DefaultScreen thingId={id} />;
    }
};
