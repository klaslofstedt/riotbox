// actionOtaurl.js
const AWS = require('aws-sdk');
const s3 = new AWS.S3();
const iot = new AWS.Iot();

const BUCKET_NAME = process.env.BUCKET_NAME;

const isVersionGreater = (version1, version2) => {
    const segments1 = version1.split('.').map(Number);
    const segments2 = version2.split('.').map(Number);

    for (let i = 0; i < segments1.length; i++) {
        if (segments1[i] > segments2[i]) {
            return true;
        } else if (segments1[i] < segments2[i]) {
            return false;
        }
    }
    return false;
};

const getPayload = (data, event) => {
    const payloadWithoutUrl = {
        do_ota: false,
        otaurl: '',
    };
    // Check if data.Contents is valid and has at least one item
    if (!data.Contents || data.Contents.length === 0) {
        console.error("No objects found in the bucket.");
        return payloadWithoutUrl;
    }

    // Get the last object in the bucket
    const lastObject = data.Contents[data.Contents.length - 1];

    if (!lastObject || !lastObject.Key) {
        console.error("Last object or its key is missing.");
        return payloadWithoutUrl;
    }

    // Get the last object in the bucket
    const lastObjectKey = lastObject.Key;
    const fwVersionS3 = lastObjectKey.replace('.bin', ''); // Strip the .bin to get the version
    const fwVersionThing = JSON.parse(event.payload.value).thing_value.read.fw_version;

    if (isVersionGreater(fwVersionS3, fwVersionThing)) {
        const url = s3.getSignedUrl('getObject', {
            Bucket: BUCKET_NAME,
            Key: lastObjectKey,
            Expires: 3600, // The URL will expire after 1 hour
        });
        const payloadWithUrl = {
            do_ota: true,
            otaurl: url,
        };
        // Return URL to OTA 
        return payloadWithUrl;
    }
    return payloadWithoutUrl;
}

const actionOtaurl = async (event) => {
    try {
        const response = await iot.describeEndpoint({ endpointType: 'iot:Data-ATS' }).promise();
        const iotCoreEndpointUrl = response.endpointAddress;
        console.log('IoT Core endpoint:', iotCoreEndpointUrl);
        console.log("id: ", event.id);
  
        const listParams = {
            Bucket: BUCKET_NAME,
        };

        // Get a list of objects in the bucket
        const data = await s3.listObjectsV2(listParams).promise();

        const payload = getPayload(data, event);

        const iotData = new AWS.IotData({ endpoint: iotCoreEndpointUrl });
        const params = {
            topic: 'thingsub/' + event.id + '/otaurl',
            payload: JSON.stringify(payload),
        }
        console.log("PAYLOAD:", JSON.stringify(payload));
      
        const result = await iotData.publish(params).promise();
        console.log("IoT Publish: ", result);

        return true;
    } catch (e) {
        console.error(e);
        return false;
    }
};

module.exports = { actionOtaurl };