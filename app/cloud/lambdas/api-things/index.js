const AWS = require('aws-sdk');
const docClient = new AWS.DynamoDB.DocumentClient();

const thingGetById = async (id) => {
    const params = {
        TableName: process.env.DYNAMO_TABLE_THINGS_URL,
        Key: { id: id }
    }
    try {
        const { Item } = await docClient.get(params).promise()
        return Item;
    } catch (err) {
        console.log('Error thingGetById: ', err);
        return null;
    }
}

const thingValueChange = async (thing, updater) => {
  console.log('thingValueChange ', thing);
  const currentUTC = new Date().toISOString();
  try {
    const params = {
      TableName: process.env.DYNAMO_TABLE_THINGS_URL,
      Key: {
        id: thing.id
      },
      UpdateExpression: "set #value = :value, #synced = :synced, #updater = :updater",
      ExpressionAttributeNames: {
        "#value": "value",
        "#synced": "synced",
        "#updater": "updater"
      },
      ExpressionAttributeValues: {
        ":value": thing.value,
        ":synced": currentUTC,
        ":updater": updater,
      },
      ReturnValues: "UPDATED_NEW"
    };
    await docClient.update(params).promise();

  } catch (err) {
    console.log('Error thingValueChange: ', err)
    return null;
  }

  return thingGetById(thing.id);
}

const thingOwnerChange = async(thing) => {
  console.log('thingOwnerChange ', thing);
  try {
    const params = {
      TableName: process.env.DYNAMO_TABLE_THINGS_URL,
      Key: {
        id: thing.id
      },
      UpdateExpression: "set #owner = :owner, #provisioned = :provisioned",
      ExpressionAttributeNames: {
        "#owner": "owner",
        "#provisioned": "provisioned"
      },
      ExpressionAttributeValues: {
        ":owner": thing.owner,
        ":provisioned": thing.provisioned
      },
      ReturnValues: "UPDATED_NEW"
    };
    await docClient.update(params).promise();

  } catch (err) {
    console.log('Error thingOwnerChange: ', err);
    return null;
  }

  return thingGetById(thing.id);
}

const thingsGetByOwner = async(owner) => {
    const params = {
      TableName: process.env.DYNAMO_TABLE_THINGS_URL,
      IndexName: 'thingsGetByOwner',
      KeyConditionExpression: '#fieldName = :owner',
      ExpressionAttributeNames: { '#fieldName': 'owner' },
      ExpressionAttributeValues: { ':owner': owner },
    }
  
    try {
        const data = await docClient.query(params).promise();
        console.log('Items: ', data.Items);

        return data.Items || [];
    } catch (err) {
        console.log('Error thingsGetByOwner', err);
        return null;
    }
}

const checkOwnership = (user, owner) => {
  if (owner !== user) {
    console.log('checkOwnership:', user, owner);
    throw new Error("Unauthorized: You don't have permission to perform this operation.");
  }
}

const checkAvailability = (owner) => {
  if (owner !== 'false') {
    console.log('checkAvailability:', owner);
    throw new Error("Unauthorized: You don't have permission to perform this operation.");
  }
}

exports.handler = async (event) => {
  console.log('GRAPHQL EVENT:', event);
  let thing;
  switch (event.info.fieldName) {
    case "thingGetById":
      console.log('thingGetById');
      thing = await thingGetById(event.arguments.thingId);
      console.log('thingGetById thing:', thing);
      checkAvailability(thing.owner);
      return thing;
    case "thingGetByIdFromLambda":
      console.log('thingGetByIdFromLambda');
      thing = await thingGetById(event.arguments.thingId);
      console.log('thingGetByIdFromLambda thing:', thing);
      return thing;

    case "thingsGetByOwner":
      console.log('thingsGetByOwner');
      thing = await thingsGetByOwner(event.identity.username);
      console.log('thingsGetByOwner thing', thing);
      return thing;

    case "thingProvision":
      console.log('thingProvision');
      thing = await thingGetById(event.arguments.thingId);
      console.log('thingGetById thing:', thing);
      checkAvailability(thing.owner);
      thing.owner = event.identity.username;
      const currentUTC = new Date().toISOString();
      thing.provisioned = currentUTC;
      thing = await thingOwnerChange(thing);
      console.log('thingProvision thing', thing);
      return thing;

    case "thingDeprovision":
      console.log('thingDeprovision');
      thing = await thingGetById(event.arguments.thingId);
      checkOwnership(event.identity.username, thing.owner);
      thing.owner = 'false';
      thing.provisioned = 'false';
      thing = await thingOwnerChange(thing);
      console.log('thingDeprovision thing', thing);
      return thing;

    case "thingValueChange":
      console.log('thingValueChange');
      thing = await thingGetById(event.arguments.thing.id);
      checkOwnership(event.identity.username, thing.owner);
      thing.value = event.arguments.thing.value;
      thing = await thingValueChange(thing, "mobile");
      console.log('thingValueChange thing', thing);
      return thing;

    case "thingValueChangeFromLambda":
      console.log('thingValueChangeFromLambda');
      thing = await thingGetById(event.arguments.thing.id);
      thing.value = event.arguments.thing.value;
      thing = await thingValueChange(thing, "thing");
      console.log('thingValueChangeFromLambda thing', thing);
      return thing;

    default:
      console.log('Error: default operation');
      return null;
    }
};