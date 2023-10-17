import React, { createContext, useContext, useState } from "react";

const UserThingsContext = createContext({ userThings: [] });

export const useUserThings = () => {
    const context = useContext(UserThingsContext);
    if (!context)
        throw new Error(
            "useUserThings must be used within a UserThingsProvider"
        );
    return context;
};

export const UserThingsProvider = ({ children }) => {
    const [userThings, setUserThings] = useState([]);
    return (
        <UserThingsContext.Provider value={{ userThings, setUserThings }}>
            {children}
        </UserThingsContext.Provider>
    );
};
