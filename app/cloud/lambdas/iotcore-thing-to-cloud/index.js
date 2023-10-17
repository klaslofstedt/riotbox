// index.js
const { actionOtaurl } = require('./actionOtaurl');
const { actionValue } = require('./actionValue');
const { actionBootup } = require('./actionBootup');

exports.handler = async (event) => {
    console.log('EVENT:', event);
    console.log('ACTION: ', event.action);

    let success = true;

    if (success && (event.action == 'otaurl')) {
        success = await actionOtaurl(event);
    }

    if (success && (event.action == 'bootup')) {
        success = await actionBootup(event);
    }

    if (success && (event.action == 'value')) {
        success = await actionValue(event);
    }

    if (success) {
        return { statusCode: 200, body: 'Action successfully executed.' };
    } else {
        return { statusCode: 500, body: 'Action execution failed.' };
    }
};
