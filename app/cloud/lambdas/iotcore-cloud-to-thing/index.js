const AWS = require("aws-sdk");
const iot = new AWS.Iot();

exports.handler = async (event) => {
    try {
        // Fetch the IoT core endpoint URL
        const response = await iot.describeEndpoint({ endpointType: 'iot:Data-ATS' }).promise();
        const iotCoreEndpointUrl = response.endpointAddress;
        console.log('IoT Core endpoint:', iotCoreEndpointUrl);

        for (const record of event.Records) {
            // Extract the required values from the record
            const id = record.dynamodb.NewImage.id.S;
            const newValue = record.dynamodb.NewImage.value.S;
            const updater = record.dynamodb.NewImage.updater.S;
            
            // NOTE: Only publish MQTT back to thing if DB update came from "mobile"
            if (updater === "thing"){
                return { statusCode: 200, body: `DB already updated from thing, do not publish.` };
            }

            console.log("id: ", id, "newValue: ", newValue);
            const iotData = new AWS.IotData({ endpoint: iotCoreEndpointUrl });
            const params = {
                topic: 'thingsub/' + id + '/value',
                payload: newValue,
            }

            const result = await iotData.publish(params).promise();
            console.log("IoT Publish: ", result);

            return { statusCode: 200, body: `IoT message published.` };
        }
    } catch (e) {
        console.error(e);
        return { statusCode: 500, body: `IoT message could not be published.` };
    }
};