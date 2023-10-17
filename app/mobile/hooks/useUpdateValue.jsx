import { useState } from "react";
import Constants from "expo-constants";
import { apiThingValueChange } from "../graphql/GraphqlApi";
import { useUserThings } from "../contexts/UserThingsContext";

const useUpdateValue = (thingId) => {
    const { userThings, setUserThings } = useUserThings();
    const thing = userThings.find((thing) => thing.id === thingId);
    const initialThingValue = thing ? JSON.parse(thing.value) : null;
    const isNetworkOffline = initialThingValue && initialThingValue.mobile_value.readwrite.network === "offline";
    console.log("TEST");
    const [isUpdatingValue, setIsUpdatingValue] = useState(isNetworkOffline);
  
    const setUpdateValue = async (newThingValueStatus) => {
      const appVersion = Constants.manifest.version;
      console.log(appVersion);
      setIsUpdatingValue(true);
  
      const updatedThingValue = {
        ...initialThingValue,
        mobile_value: {
          ...initialThingValue.mobile_value,
          readwrite: {
            ...initialThingValue.mobile_value.readwrite,
            network: "offline",
          },
          read: {
            ...initialThingValue.mobile_value.read,
            sw_version: appVersion,
          },
        },
        thing_value: {
          ...initialThingValue.thing_value,
          readwrite: {
            ...initialThingValue.thing_value.readwrite,
            status: newThingValueStatus,
          },
        },
      };
  
      try {
        setUserThings((prevUserThings) =>
          prevUserThings.map((thing) =>
            thing.id === thingId
              ? { ...thing, value: JSON.stringify(updatedThingValue) }
              : thing
          )
        );
  
        await apiThingValueChange(
          thingId,
          JSON.stringify(updatedThingValue)
        );
      } catch (error) {
        console.log("Error: ", error);
      }
    };
  
    return [isUpdatingValue, setUpdateValue, initialThingValue.thing_value];
  };
  
  export default useUpdateValue;