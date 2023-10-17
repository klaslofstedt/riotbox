import { API, graphqlOperation } from "aws-amplify";
import {
    thingGetById,
    thingsGetByOwner,
    thingProvision,
    thingDeprovision,
    thingValueChange,
    onThingProvisionChange,
    onThingValueChange,
} from "./GraphqlOperations";

export const apiThingsList = async () => {
    try {
        const response = await API.graphql(graphqlOperation(thingsGetByOwner));
        // Filter the things to only include those where thing.provision = true
        const provisionedThings = response.data.thingsGetByOwner.filter(
            (thing) => thing.provision !== false
        );
        provisionedThings.forEach((thing) => {
            console.log("Things List:", thing.id);
        });
        return provisionedThings;
    } catch (error) {
        console.error("Error fetching Things:", error);
        return [];
    }
};

export const apiThingValueChange = async (thingId, value) => {
    const updatedThing = {
        id: thingId,
        value: value,
    };
    try {
        await API.graphql(
            graphqlOperation(thingValueChange, {
                thing: updatedThing,
            })
        );
        console.log("Updated thing");
    } catch (error) {
        console.error("Error updating thing:", error);
    }
};

export const apiSubscribeThingProvisionChange = (
    owner,
    callback,
    errorCallback
) => {
    return API.graphql(
        graphqlOperation(onThingProvisionChange, { owner })
    ).subscribe({
        next: ({ value }) => callback(value.data.onThingProvisionChange),
        error: errorCallback,
    });
};

export const apiSubscribeThingValueChange = (
    owner,
    callback,
    errorCallback
) => {
    return API.graphql(
        graphqlOperation(onThingValueChange, { owner })
    ).subscribe({
        next: ({ value }) => callback(value.data.onThingValueChange),
        error: errorCallback,
    });
};

export const apiThingGetById = async (thingId) => {
    try {
        const response = await API.graphql(
            graphqlOperation(thingGetById, {
                thingId: thingId,
            })
        );
        console.log("apiThingGetById:", response);
        if (response?.data?.thingGetById) {
            return response.data.thingGetById;
        } else {
            console.error("Error fetching Thing");
            return;
        }
    } catch (error) {
        console.error("Error fetching Thing:", error);
        return;
    }
};

export const apiThingProvision = async (thingId) => {
    try {
        const response = await API.graphql(
            graphqlOperation(thingProvision, {
                thingId: thingId,
            })
        );
        console.log("apiThingProvision:", response);
        return response.data.thingProvision;
    } catch (error) {
        console.error("Error provisioning thing:", error);
        return null;
    }
};

export const apiThingDeprovision = async (thingId) => {
    try {
        const response = await API.graphql(
            graphqlOperation(thingDeprovision, {
                thingId: thingId,
            })
        );
        console.log("apiThingDeprovision:", response);
        return response.data.thingDeprovision;
    } catch (error) {
        console.error("Error deprovisioning thing:", error);
        return null;
    }
};
