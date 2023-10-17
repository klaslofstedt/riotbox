// actionValue.js
const AWS = require('aws-sdk');
const { AWSAppSyncClient } = require('aws-appsync');
const gql = require('graphql-tag');

const GRAPHQL_ENDPOINT = process.env.APPSYNC_API_URL;

const query = gql`
    mutation ThingValueChangeFromLambda($thing: ThingValueChangeInput!) {
        thingValueChangeFromLambda(thing: $thing) {
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

const actionValue = async (event) => {
    const variables = {
        thing: {
            id: event.id,
            value: event.payload.value,
        },
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

    try {
        const result = await client.mutate({
            mutation: query,
            variables: variables,
        });
        console.log('RESULT:', result);
        return true;
    } catch (error) {
        console.log('ERROR:', error);
        return false;
    }
};

module.exports = { actionValue };