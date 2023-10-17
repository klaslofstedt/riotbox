// actionValue.js
const AWS = require('aws-sdk');
const { AWSAppSyncClient } = require('aws-appsync');
const gql = require('graphql-tag');
const iot = new AWS.Iot();

const GRAPHQL_ENDPOINT = process.env.APPSYNC_API_URL;

const query = gql`
    query ThingGetByIdFromLambda($thingId: ID!) {
        thingGetByIdFromLambda(thingId: $thingId) {
            id
            owner
            type
            aes
            pop
            qr
            deployed
            provisioned
            synced
            value
            updater
        }
    }
`;

const actionBootup = async (event) => {
    try {
        const response = await iot.describeEndpoint({ endpointType: 'iot:Data-ATS' }).promise();
        const iotCoreEndpointUrl = response.endpointAddress;
        console.log('IoT Core endpoint:', iotCoreEndpointUrl);
        console.log("id: ", event.id);

        const variables = {
            thingId: event.id,
        };
    
        const client = new AWSAppSyncClient({
            url: GRAPHQL_ENDPOINT,
            region: process.env.AWS_REGION,
            auth: {
                type: 'AWS_IAM',
                credentials: new AWS.EnvironmentCredentials('AWS'),
            },
            disableOffline: true,
        });

        const dynamo = await client.query({
            query: query,
            variables: variables,
        });
        const value = dynamo.data.thingGetByIdFromLambda.value;
        const iotData = new AWS.IotData({ endpoint: iotCoreEndpointUrl });
        const params = {
            topic: 'thingsub/' + event.id + '/bootup',
            payload: value,
        }
        console.log("PAYLOAD:", value);
      
        const result = await iotData.publish(params).promise();
        console.log("IoT Publish: ", result);
        return true;
    } catch (error) {
        console.log('ERROR:', error);
        return false;
    }
};

module.exports = { actionBootup };