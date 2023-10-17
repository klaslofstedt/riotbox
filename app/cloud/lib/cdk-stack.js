const { Stack, RemovalPolicy, CfnOutput, Duration } = require('aws-cdk-lib');
const { UserPool, UserPoolClient, AccountRecovery } = require('aws-cdk-lib/aws-cognito');
const { GraphqlApi, SchemaFile, AuthorizationType, FieldLogLevel, Resolver } = require('aws-cdk-lib/aws-appsync');
const { Function, Runtime, Code } = require('aws-cdk-lib/aws-lambda');
const { Table, AttributeType, BillingMode, StreamViewType } = require('aws-cdk-lib/aws-dynamodb');
const { DynamoEventSource } = require('aws-cdk-lib/aws-lambda-event-sources');
const { LogGroup } = require('aws-cdk-lib/aws-logs');
const { PolicyStatement, Effect, Policy } = require('aws-cdk-lib/aws-iam');
const { StartingPosition } = require('aws-cdk-lib/aws-lambda');
const { TopicRule, IotSql } = require('@aws-cdk/aws-iot-alpha');
const { CfnThing } = require('aws-cdk-lib/aws-iot');
const { LambdaFunctionAction } = require('@aws-cdk/aws-iot-actions-alpha');
const { Bucket, BlockPublicAccess } = require('aws-cdk-lib/aws-s3');
const { IdentityPool, UserPoolAuthenticationProvider } = require('@aws-cdk/aws-cognito-identitypool-alpha');

const prefix = process.env.PROJECT_NAME;

class CdkStack extends Stack {
  /**
   *
   * @param {Construct} scope
   * @param {string} id
   * @param {StackProps=} props
   */
  constructor(scope, id, props) {
    super(scope, id, props);

    /********************************************************************************/

    // Create Cognito for creation and handling of user accounts
    const cognitoUserPool = new UserPool(this, `${prefix}CognitoUserPool`, {
      userPoolName: `${prefix}CognitoUserPool`,
      selfSignUpEnabled: true,
      signInCaseSensitive: false,
      signInAliases: {
        email: true
      },
      autoVerify: {
        email: true
      },
      passwordPolicy: {
        minLength: 8,
        requireLowercase: true,
        requireDigits: true,
        tempPasswordValidity: Duration.days(7)
      },
      accountRecovery: AccountRecovery.EMAIL_ONLY,
      userVerification: {
        emailSubject: `${prefix} verification`,
        emailBody: `Your ${prefix} account verification code is {####}`,
      },
    });

    // Create Cognito client
    const cognitoUserPoolClient = new UserPoolClient(this, `${prefix}CognitoUserPoolClient`, {
      userPool: cognitoUserPool,
      generateSecret: false,
    });

    const cognitoIdentityPool = new IdentityPool(this, `${prefix}CognitoIdentityPool`, {
      identityPoolName: `${prefix}CognitoIdentityPool`,
      allowUnauthenticatedIdentities: false,
      authenticationProviders: {
        userPools: [
          new UserPoolAuthenticationProvider({
            userPool: cognitoUserPool,
            userPoolClient: cognitoUserPoolClient,
          })
        ]
      }
    });
    
    /********************************************************************************/
    
    // Create AppSync API for handling of DynamoDB updates
    const appSyncApiThings = new GraphqlApi(this, `${prefix}AppSyncApiThings`, {
      name: `${prefix}AppSyncApiThings`,
      logConfig: {
        fieldLogLevel: FieldLogLevel.ALL,
      },
      schema: SchemaFile.fromAsset('./graphql/schema.graphql'),
      authorizationConfig: {
        defaultAuthorization: {
          authorizationType: AuthorizationType.USER_POOL, // API access for mobile users
          userPoolConfig: {
            userPool: cognitoUserPool
          }
        },
        additionalAuthorizationModes: [
        {
          authorizationType: AuthorizationType.IAM, // API access for internal services
        }]
      },
    });

    // Create Lambda function for resolving API requests
    const lambdaResolverApiThings = new Function(this, `${prefix}LambdaResolverApiThings`, {
      functionName: `${prefix}LambdaResolverApiThings`,
      removalPolicy: RemovalPolicy.DESTROY,
      runtime: Runtime.NODEJS_18_X,
      handler: 'index.handler',
      code: Code.fromAsset('lambdas/api-things'),
      memorySize: 1024,
    });

    // Appoint Lambda as resolver of AppSync 
    const dataSourceApiThings = appSyncApiThings.addLambdaDataSource('dataSourceApiThings', lambdaResolverApiThings);
    
    // Create the API specific resolvers
    new Resolver(this, 'ResolverThingGetById', {
      api: appSyncApiThings,
      dataSource: dataSourceApiThings,
      typeName: "Query",
      fieldName: "thingGetById"
    });

    new Resolver(this, 'ResolverThingGetByIdFromLambda', {
      api: appSyncApiThings,
      dataSource: dataSourceApiThings,
      typeName: "Query",
      fieldName: "thingGetByIdFromLambda"
    });
    
    new Resolver(this, 'ResolverThingsGetByOwner', {
      api: appSyncApiThings,
      dataSource: dataSourceApiThings,
      typeName: "Query",
      fieldName: "thingsGetByOwner"
    });
    
    new Resolver(this, 'ResolverThingProvision', {
      api: appSyncApiThings,
      dataSource: dataSourceApiThings,
      typeName: "Mutation",
      fieldName: "thingProvision"
    });
    
    new Resolver(this, 'ResolverThingDeprovision', {
      api: appSyncApiThings,
      dataSource: dataSourceApiThings,
      typeName: "Mutation",
      fieldName: "thingDeprovision"
    });
    
    new Resolver(this, 'ResolverThingValueChange', {
      api: appSyncApiThings,
      dataSource: dataSourceApiThings,
      typeName: "Mutation",
      fieldName: "thingValueChange"
    });

    new Resolver(this, 'ResolverThingValueChangeFromLambda', {
      api: appSyncApiThings,
      dataSource: dataSourceApiThings,
      typeName: "Mutation",
      fieldName: "thingValueChangeFromLambda"
    });

    /********************************************************************************/

    // Create a Dynamo DB that stores all Thing data
    const dynamoTableThings = new Table(this, `${prefix}DynamoTableThings`, {
      tableName: `${prefix}DynamoTableThings`,
      billingMode: BillingMode.PAY_PER_REQUEST,
      removalPolicy: RemovalPolicy.DESTROY,
      partitionKey: {
        name: 'id',
        type: AttributeType.STRING,
      },
      stream: StreamViewType.NEW_AND_OLD_IMAGES,
    });

    // Add a global secondary index to enable another data access pattern
    dynamoTableThings.addGlobalSecondaryIndex({
      indexName: "thingsGetByOwner",
      partitionKey: {
        name: "owner",
        type: AttributeType.STRING,
      }
    });

    /********************************************************************************/

    // Create policy for Lambdas to publish to IoT Core 
    const policyIotCoreLambda = new Policy(this, `${prefix}PolicyIotCoreLambda`, {
      statements: [
        new PolicyStatement({
          actions: ['iot:Publish'],
          resources: [`arn:aws:iot:${this.region}:*:*`],
          effect: Effect.ALLOW,
        }),
      ],
    });
    
    // Create policy to fetch IoT Core broker endpoint URL
    const policyIotDescribeEndpoint = new PolicyStatement({
      effect: Effect.ALLOW,
      actions: ['iot:DescribeEndpoint'],
      resources: ['*'],
    });
    
    // Create Lambda for sending data on Dynamo updates to Thing over IoT Core
    const lambdaIotCoreCloudToThing = new Function(this, `${prefix}LambdaIotCoreCloudToThing`, {
      functionName: `${prefix}LambdaIotCoreCloudToThing`,
      runtime: Runtime.NODEJS_18_X,
      handler: 'index.handler',
      code: Code.fromAsset('lambdas/iotcore-cloud-to-thing'),
      memorySize: 1024,
      removalPolicy: RemovalPolicy.DESTROY,
    });

    // Trigger the Lambda function with a DynamoDB Stream
    const dynamoTableThingsEvent = new DynamoEventSource(dynamoTableThings, {
      startingPosition: StartingPosition.TRIM_HORIZON,
      batchSize: 1,
      bisectBatchOnError: true,
      retryAttempts: 10,
    });

    // Create an IoT Core broker
    new CfnThing(this, `${prefix}IoTCoreThing`, {
      thingName: `${prefix}IoTCoreThing`,
    });

    // Create an S3 bucket for OTA firmware files
    const bucketThingOtaFw = new Bucket(this, `${prefix}BucketThingOtaFw`, {
      bucketName: `${prefix.toLowerCase()}-bucket-thing-ota-fw-${this.account}`,
      blockPublicAccess: BlockPublicAccess.BLOCK_ALL,
      removalPolicy: RemovalPolicy.DESTROY,
      autoDeleteObjects: true,
    });

    const bucketThingSecrets = new Bucket(this, `${prefix}BucketThingSecrets`, {
      bucketName: `${prefix.toLowerCase()}-bucket-thing-secrets-${this.account}`,
      blockPublicAccess: BlockPublicAccess.BLOCK_ALL,
      removalPolicy: RemovalPolicy.DESTROY,
      autoDeleteObjects: true,
    });

    // Lambda to transfer data from Thing to Cloud
    const lambdaIotCoreThingToCloud = new Function(this, `${prefix}LambdaIotCoreThingToCloud`, {
      functionName: `${prefix}LambdaIotCoreThingToCloud`,
      runtime: Runtime.NODEJS_18_X,
      handler: 'index.handler',
      code: Code.fromAsset('lambdas/iotcore-thing-to-cloud'),
      memorySize: 1024,
      removalPolicy: RemovalPolicy.DESTROY,
      environment: {
        APPSYNC_API_URL: appSyncApiThings.graphqlUrl,
        APPSYNC_API_ARN: appSyncApiThings.arn,
        BUCKET_NAME: bucketThingOtaFw.bucketName,
      },
    });

    // Format incoming Thing message for the Lambda function to handle
    const sql = "SELECT topic(2) as id, topic(3) as action, * as payload FROM 'thingpub/+/+'";
    new TopicRule(this, `${prefix}IotCoreMessageRoutingRule`, {
      topicRuleName: `${prefix}IotCoreMessageRoutingRule`,
      sql: IotSql.fromStringAsVer20151008(sql),
      actions: [new LambdaFunctionAction(lambdaIotCoreThingToCloud)],
    });

    /********************************************************************************/

    // Grant Lambda permissions to fetch IoT Core broker endpoint URL
    lambdaIotCoreThingToCloud.role.addToPolicy(policyIotDescribeEndpoint);
    // Grant Lambda to permissions to use IoT Core
    lambdaIotCoreThingToCloud.role.attachInlinePolicy(policyIotCoreLambda);
    // Grant Lambda permissions to fetch IoT Core broker endpoint URL
    lambdaIotCoreCloudToThing.role.addToPolicy(policyIotDescribeEndpoint);
    // Grant Lambda to permissions to use IoT Core
    lambdaIotCoreCloudToThing.role.attachInlinePolicy(policyIotCoreLambda);
    // Grant the Lambda permissions to read from the S3 bucket
    bucketThingOtaFw.grantRead(lambdaIotCoreThingToCloud);
    // Grant the Lambda permissions to perform AppSync mutations using IAM
    appSyncApiThings.grantMutation(lambdaIotCoreThingToCloud);
    // Grant the Lambda permissions to perform AppSync queries using IAM
    appSyncApiThings.grantQuery(lambdaIotCoreThingToCloud);
    // Trigger Lambda function on DynamoDB updates
    lambdaIotCoreCloudToThing.addEventSource(dynamoTableThingsEvent);
    // Grant Lambda full access to Dynamo (using Cognito)
    dynamoTableThings.grantFullAccess(lambdaIotCoreCloudToThing);
    // Grant Lambda full access to Dynamo (using IAM)
    dynamoTableThings.grantFullAccess(lambdaResolverApiThings);
    // Grant Cognito Role read access to Thing secrets S3 bucket
    bucketThingSecrets.grantRead(cognitoIdentityPool.authenticatedRole);
    // Create environment variable used by Lambda
    lambdaResolverApiThings.addEnvironment('DYNAMO_TABLE_THINGS_URL', dynamoTableThings.tableName);

    /********************************************************************************/

    new LogGroup(this, `${prefix}CloudWatchLambdaResolverApiThings`, {
      logGroupName: `/aws/lambda/${lambdaResolverApiThings.functionName}`,
      removalPolicy: RemovalPolicy.DESTROY,
    });
    new LogGroup(this, `${prefix}CloudWatchLambdaIotCoreThingToCloud`, {
      logGroupName: `/aws/lambda/${lambdaIotCoreThingToCloud.functionName}`,
      removalPolicy: RemovalPolicy.DESTROY,
    });
    new LogGroup(this, `${prefix}CloudWatchLambdaIotCoreCloudToThing`, {
      logGroupName: `/aws/lambda/${lambdaIotCoreCloudToThing.functionName}`,
      removalPolicy: RemovalPolicy.DESTROY,
    });

    /********************************************************************************/
    
    new CfnOutput(this, 'aws_cognito_region', {
      value: this.region
    });
    new CfnOutput(this, 'aws_user_pools_id', {
      value: cognitoUserPool.userPoolId
    });
    new CfnOutput(this, 'aws_user_pools_web_client_id', {
      value: cognitoUserPoolClient.userPoolClientId
    });
    new CfnOutput(this, "aws_cognito_identity_pool_id", {
      value: cognitoIdentityPool.identityPoolId,
    });
    new CfnOutput(this, 'aws_mandatory_sign_in', {
      value: 'enable'
    });
    new CfnOutput(this, 'aws_appsync_graphql_endpoint', {
      value: appSyncApiThings.graphqlUrl
    });
    new CfnOutput(this, 'aws_appsync_region', {
      value: this.region
    });
    new CfnOutput(this, 'aws_appsync_authentication_type', {
      value: 'AMAZON_COGNITO_USER_POOLS'
    });
    new CfnOutput(this, 'aws_user_files_s3_bucket', {
      value: bucketThingSecrets.bucketName
    });
    new CfnOutput(this, 'aws_user_files_s3_bucket_region', {
      value: this.region
    });
  }
}

module.exports = { CdkStack }