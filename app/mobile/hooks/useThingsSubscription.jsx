import { useEffect } from "react";
import {
    apiSubscribeThingProvisionChange,
    apiSubscribeThingValueChange,
} from "../graphql/GraphqlApi";

const useThingsSubscription = (user, setUserThings) => {
    useEffect(() => {
        if (!user) return;

        const handleThingProvisionChange = (thing) => {
            setUserThings((prevUserThings) => {
                return [...prevUserThings, thing];
            });
        };

        const handleThingValueChange = (thing) => {
            setUserThings((prevUserThings) => {
                const thingIndex = prevUserThings.findIndex(
                    (prevThing) => prevThing.id === thing.id
                );
                if (thingIndex === -1) {
                    return prevUserThings;
                }
                // NOTE: only update setUserThings if the DB update came from "thing"
                if (thing.updater === "mobile") {
                    return prevUserThings;
                }

                console.log("new Subscription");
                const newUserThings = [...prevUserThings];
                newUserThings[thingIndex] = thing;
                return newUserThings;
            });
        };

        const onError = (error, subscriptionType) => {
            console.error(`Error creating ${subscriptionType}:`, error);
        };

        const provisionSub = apiSubscribeThingProvisionChange(
            user.username,
            handleThingProvisionChange,
            (error) => onError(error, "onThingProvisionChange")
        );

        const valueChangeSub = apiSubscribeThingValueChange(
            user.username,
            handleThingValueChange,
            (error) => onError(error, "onThingValueChange")
        );

        return () => {
            provisionSub.unsubscribe();
            valueChangeSub.unsubscribe();
        };
    }, [user, setUserThings]);
};

export default useThingsSubscription;
