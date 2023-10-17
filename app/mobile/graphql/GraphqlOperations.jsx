export const thingGetById = `
    query ThingGetById($thingId: ID!) {
        thingGetById(thingId: $thingId) {
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

export const thingsGetByOwner = `
    query ThingsGetByOwner {
        thingsGetByOwner {
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

export const thingProvision = `
    mutation ThingProvision($thingId: ID!) {
        thingProvision(thingId: $thingId) {
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

export const thingDeprovision = `
    mutation ThingDeprovision($thingId: ID!) {
        thingDeprovision(thingId: $thingId) {
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

export const thingValueChange = `
    mutation ThingValueChange($thing: ThingValueChangeInput!) {
        thingValueChange(thing: $thing) {
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

export const thingValueChangeFromLambda = `
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

export const onThingProvisionChange = `
    subscription OnThingProvisionChange($owner: String) {
        onThingProvisionChange(owner: $owner) {
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

export const onThingValueChange = `
    subscription OnThingValueChange($owner: String) {
        onThingValueChange(owner: $owner) {
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
