import AwsConfig from "./src/aws-exports";
import { CdkStack } from "./src/cdk-exports.json";
import { GestureHandlerRootView } from "react-native-gesture-handler";
import { StatusBar } from "expo-status-bar";
import { Amplify } from "aws-amplify";
import { NavigationContainer } from "@react-navigation/native";
import LoggedOutNavigator from "./navigators/LoggedOutNavigator";
import LoggedInNavigator from "./navigators/LoggedInNavigator";
import { AppLoading } from "./components/AppLoading";
import { AuthProvider, useAuth } from "./contexts/AuthContext";

// https://docs.amplify.aws/lib/client-configuration/configuring-amplify-categories/q/platform/js/#general-configuration
// https://docs.amplify.aws/lib/graphqlapi/existing-resources/q/platform/js/
const CdkConfig = {
    aws_cognito_region: CdkStack.awscognitoregion,
    aws_user_pools_id: CdkStack.awsuserpoolsid,
    aws_user_pools_web_client_id: CdkStack.awsuserpoolswebclientid,
    aws_cognito_identity_pool_id: CdkStack.awscognitoidentitypoolid,
    aws_mandatory_sign_in: CdkStack.awsmandatorysignin,
    aws_appsync_graphqlEndpoint: CdkStack.awsappsyncgraphqlendpoint,
    aws_appsync_region: CdkStack.awsappsyncregion,
    aws_appsync_authenticationType: CdkStack.awsappsyncauthenticationtype,
    aws_user_files_s3_bucket_region: CdkStack.awsuserfiless3bucketregion,
    aws_user_files_s3_bucket: CdkStack.awsuserfiless3bucket,
};

const configuration = {
    ...AwsConfig,
    ...CdkConfig,
};

Amplify.configure(configuration);

const App = () => {
    return (
        <GestureHandlerRootView style={{ flex: 1 }}>
            <NavigationContainer>
                <StatusBar style="light" />
                <AuthProvider>
                    <NavigationSelector />
                </AuthProvider>
            </NavigationContainer>
        </GestureHandlerRootView>
    );
};

const NavigationSelector = () => {
    const auth = useAuth();

    if (auth.isUserLoggedIn === null) return <AppLoading />;
    return auth.isUserLoggedIn ? <LoggedInNavigator /> : <LoggedOutNavigator />;
};

export default App;
