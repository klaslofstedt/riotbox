import React, { createContext, useContext, useState, useEffect } from "react";
import { Auth } from "aws-amplify";
import { AppLoading } from "../components/AppLoading";

const AuthContext = createContext();

export const useAuth = () => {
    const context = useContext(AuthContext);
    if (!context)
        throw new Error("useAuth must be used within an AuthProvider");
    return context;
};

export const AuthProvider = ({ children }) => {
    const [isUserLoggedIn, setUserLoggedIn] = useState(null);

    useEffect(() => {
        checkAuthState();
    }, []);

    const checkAuthState = async () => {
        try {
            await Auth.currentAuthenticatedUser();
            console.log("✅ User is signed in");
            setUserLoggedIn(true);
        } catch (err) {
            console.log("❌ User is not signed in");
            setUserLoggedIn(false);
        }
    };

    // The value that all children components will have access to
    const value = {
        isUserLoggedIn,
        checkAuthState,
    };

    return (
        <AuthContext.Provider value={value}>
            {isUserLoggedIn === null ? <AppLoading /> : children}
        </AuthContext.Provider>
    );
};
