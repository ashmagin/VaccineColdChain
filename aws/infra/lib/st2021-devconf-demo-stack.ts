import * as cdk from '@aws-cdk/core';
import * as timestream from '@aws-cdk/aws-timestream';
import * as lambda from '@aws-cdk/aws-lambda';
import * as dynamodb from '@aws-cdk/aws-dynamodb';
import * as iam from '@aws-cdk/aws-iam';
import * as iot from '@aws-cdk/aws-iot';
import * as cognito from '@aws-cdk/aws-cognito';
import * as apigateway from "@aws-cdk/aws-apigateway";
import {AuthorizationType} from "@aws-cdk/aws-apigateway";
import * as location from "@aws-cdk/aws-location";

import path from "path";

export class St2021DevconfDemoStack extends cdk.Stack {
  constructor(scope: cdk.Construct, id: string, props?: cdk.StackProps) {
    super(scope, id, props);

    const tag = {key: "project", value: "St2021Devconf"}
    const accountId = this.account
    const region = this.region

    const thingNameParam = new cdk.CfnParameter(this, "thingNameParam", {
      type: "String",
      description: "The name of the thing",
      default: 'container42'
    });

    const policyNameParam = new cdk.CfnParameter(this, "policyNameParam", {
      type: "String",
      description: "The name of the policy",
      default: 'cognito-iot-policy-St2021Devconf'
    });

    const mapNameParam = new cdk.CfnParameter(this, "mapNameParam", {
      type: "String",
      description: "The name of the map",
      default: 'STDevConMap-stack'
    });

    const thingName = thingNameParam.valueAsString

    // timestream table
    const dbName = "St2021Devconf-demo"
    let db = new timestream.CfnDatabase(this, "timeStreamDB", {
      databaseName: dbName,
      tags: [tag]
    })

    const tableName = "iot"
    let table = new timestream.CfnTable(this, "timeStreamDBTable", {
      databaseName: dbName,
      tableName: tableName
    })

    table.addDependsOn(db)

    const dynamoTableName = 'st2021devconf'
    let dynamoTable = new dynamodb.Table(this, "dynamodbTable", {
      tableName: dynamoTableName,
      partitionKey: {name: 'device_id', type: dynamodb.AttributeType.STRING},
      sortKey: {name: 'timestamp', type: dynamodb.AttributeType.NUMBER},

    })

    //create lambda handler
    const shadowHandlerName = `shadow-handler-${this.stackName}`
    const shadowHandler = new lambda.Function(this, 'ShadowHandler', {
      runtime: lambda.Runtime.NODEJS_14_X,
      handler: 'shadow-handler.handler',
      code: lambda.Code.fromAsset(path.join(__dirname, '/../.build/lambdas'), {exclude: ["**", `!shadow-handler.js`]}),
      functionName: shadowHandlerName,
      environment: {
        DEVICE_TABLE_NAME: dynamoTableName
      },
      timeout: cdk.Duration.seconds(10),
    });

    this.addLogsPolicyToFunction(shadowHandler, region, accountId, shadowHandlerName)

    shadowHandler.addToRolePolicy(new iam.PolicyStatement({
        effect: iam.Effect.ALLOW,
        actions: [
          "dynamodb:GetItem",
          "dynamodb:UpdateItem",
          "dynamodb:PutItem",
          "dynamodb:Query",

        ],
        resources: [
          `arn:aws:dynamodb:${region}:${accountId}:table/${dynamoTableName}`,
        ]
      })
    )

    const historyHandlerName = `history-handler-${this.stackName}`
    const historyHandler = new lambda.Function(this, 'historyHandle', {
      runtime: lambda.Runtime.NODEJS_14_X,
      handler: 'history-handler.handler',
      code: lambda.Code.fromAsset(path.join(__dirname, '/../.build/lambdas'), {exclude: ["**", `!history-handler.js`]}),
      functionName: historyHandlerName,
      environment: {
        DEVICE_TABLE_NAME: dynamoTableName
      },
      timeout: cdk.Duration.seconds(10),
    });

    this.addLogsPolicyToFunction(shadowHandler, region, accountId, shadowHandlerName)
    historyHandler.addToRolePolicy(new iam.PolicyStatement({
        effect: iam.Effect.ALLOW,
        actions: [
          "dynamodb:GetItem",
          "dynamodb:UpdateItem",
          "dynamodb:PutItem",
          "dynamodb:Query",

        ],
        resources: [
          `arn:aws:dynamodb:${region}:${accountId}:table/${dynamoTableName}`,
        ]
      })
    )

    // iot rule should be able to invoke lambda
    new lambda.CfnPermission(this, "LambdaRulePermission", {
      action: "lambda:InvokeFunction",
      functionName: shadowHandler.functionArn,
      principal: "iot.amazonaws.com"
    })

    const iotRoleTimestream = new iam.Role(this, "IotTimestreamRoleName", {
      assumedBy: new iam.ServicePrincipal('iot.amazonaws.com'),
      inlinePolicies: {
        'edit1': new iam.PolicyDocument({
          statements: [
            new iam.PolicyStatement({
              effect: iam.Effect.ALLOW,
              actions: [
                'timestream:WriteRecords',
              ],
              resources: [
                `arn:aws:timestream:${region}:${accountId}:database/${dbName}/table/${tableName}`
              ]
            })
          ]
        }),
        'edit2': new iam.PolicyDocument({
          statements: [
            new iam.PolicyStatement({
              effect: iam.Effect.ALLOW,
              actions: [
                'timestream:DescribeEndpoints',
              ],
              resources: [
                `*`
              ]
            })
          ]
        })
      }
    });

    let ruleTs = new iot.CfnTopicRule(this, `ShadowHandlerRule`, {
      topicRulePayload: {
        sql: `SELECT state.reported, topic(3) as device_id
              FROM '$aws/things/${thingName}/shadow/update/accepted'`,
        ruleDisabled: false,
        actions: [
          {lambda: {functionArn: shadowHandler.functionArn},},
          {
            timestream: {
              databaseName: dbName, tableName: tableName, dimensions: [{
                "name": "device_id",
                "value": "${topic(3)}"
              }],
              roleArn: iotRoleTimestream.roleArn
            }
          }
        ],
        awsIotSqlVersion: "2016-03-23",
      }
    })

    ruleTs.node.addDependency(iotRoleTimestream)

    const api = new apigateway.RestApi(this, "St2021DevconfDemoStackApi",
      {
        endpointConfiguration: {
          types: [apigateway.EndpointType.REGIONAL]
        },
        defaultCorsPreflightOptions: {
          allowHeaders: [
            'Content-Type',
            'X-Amz-Date',
            'Authorization',
            'X-Api-Key',
          ],
          allowMethods: apigateway.Cors.ALL_METHODS,
          allowCredentials: true,
          allowOrigins: apigateway.Cors.ALL_ORIGINS,
        }
      }
    );

    const iotRoleAuthCognito = new iam.Role(this, "iotRoleAuthCognito", {
      assumedBy: new iam.WebIdentityPrincipal('cognito-identity.amazonaws.com'),
      inlinePolicies: {
        'default': new iam.PolicyDocument({
          statements: [
            new iam.PolicyStatement({
              effect: iam.Effect.ALLOW,
              actions: [
                "mobileanalytics:PutEvents",
                "cognito-sync:*",
                "cognito-identity:*",
                'sts:TagSession'
              ],
              resources: [
                `*`
              ]
            })
          ]
        }),
        'iot': new iam.PolicyDocument({
          statements: [
            new iam.PolicyStatement({
              effect: iam.Effect.ALLOW,
              actions: [
                'iot:Connect',
                'iot:Publish',
                'iot:Subscribe',
                'iot:Receive',
                'iot:GetThingShadow',
                'iot:AttachPrincipalPolicy'
              ],
              resources: [
                `*`
              ]
            })
          ]
        }),
        'locations': new iam.PolicyDocument({
          statements: [
            new iam.PolicyStatement({
              effect: iam.Effect.ALLOW,
              actions: [
                "geo:GetMapGlyphs",
                "geo:GetMapSprites",
                "geo:GetMapStyleDescriptor",
                "geo:GetMapTile"
              ],
              resources: [
                //`*`,
                `arn:aws:geo:${region}:${accountId}:map/${mapNameParam.valueAsString}`
              ]
            })
          ]
        }),
        'api': new iam.PolicyDocument({
          statements: [
            new iam.PolicyStatement({
              effect: iam.Effect.ALLOW,
              actions: [
                "execute-api:Invoke"
              ],
              resources: [
                `arn:aws:execute-api:${region}:${accountId}:${api.restApiId}/*`,
              ]
            })
          ]
        }),
      }
    });

    // Add a managed policy to a role you can use
    iotRoleAuthCognito.addManagedPolicy(
      iam.ManagedPolicy.fromAwsManagedPolicyName('AWSIoTDataAccess'))

    const userPool = new cognito.UserPool(this, 'st2021devconfdemoUserPool', {
      selfSignUpEnabled: true,
      signInAliases: {email: true, preferredUsername: true, username: true},
      autoVerify: {
        email: true
      },

    });
    const cfnUserPool = userPool.node.defaultChild as cognito.CfnUserPool;
    cfnUserPool.policies = {
      passwordPolicy: {
        minimumLength: 8,
        requireLowercase: false,
        requireNumbers: false,
        requireUppercase: false,
        requireSymbols: false
      }
    };
    cfnUserPool.usernameConfiguration = {caseSensitive: false}

    const userPoolClient = new cognito.UserPoolClient(this, 'UserPoolClient', {
      generateSecret: false,
      userPool: userPool,
    });

    let cognitiIdp = new cognito.CfnIdentityPool(this, `CfnIdentityPool`, {
      allowUnauthenticatedIdentities: false,
      cognitoIdentityProviders: [{
        clientId: userPoolClient.userPoolClientId,
        providerName: userPool.userPoolProviderName,
      }]
    })

    cognitiIdp.node.addDependency(iotRoleAuthCognito)

    let roleAttachments = new cognito.CfnIdentityPoolRoleAttachment(this, `CfnIdentityPoolRoleAttachment`, {
      identityPoolId: cognitiIdp.ref,
      roles: {
        "authenticated": iotRoleAuthCognito.roleArn,
       // "unauthenticated": iotRoleAuthCognito.roleArn,
      }
    })

    roleAttachments.addDependsOn(cognitiIdp)


    const v1 = api.root.addResource('v1');
    const historyResource = v1.addResource('history');

    const authorizer = new apigateway.CognitoUserPoolsAuthorizer(this, "CognitoAuthorizer", {
      cognitoUserPools: [userPool]
    })
    const getHistory = historyResource.addMethod('GET', new apigateway.LambdaIntegration(historyHandler),
      {authorizationType: AuthorizationType.COGNITO, authorizer: authorizer})
    const deployment = new apigateway.Deployment(this, 'ApiDeployment', {
      api: api,
    });

    new iot.CfnPolicy(this, "cognitoIotPolicy", {
      policyDocument: this.getPolicyToAttach(accountId, region, thingName),
      policyName: policyNameParam.valueAsString
    });

    new location.CfnMap(this, "locationMap", {
      mapName: mapNameParam.valueAsString,
      configuration: {style: 'VectorEsriStreets'},
      pricingPlan: 'RequestBasedUsage'
    })


    new cdk.CfnOutput(this, 'iotPolicy', {value: policyNameParam.valueAsString});
    new cdk.CfnOutput(this, 'thingName', {value: thingName});
    new cdk.CfnOutput(this, 'mapName', {value: mapNameParam.valueAsString});
    new cdk.CfnOutput(this, 'apiEndpoint', {value: historyResource.url});
    new cdk.CfnOutput(this, 'userPool', {value: userPool.userPoolId});
    new cdk.CfnOutput(this, 'userPoolAppClientId', {value: userPoolClient.userPoolClientId});
    new cdk.CfnOutput(this, 'identityPool', {value: cognitiIdp.ref});


  }

  getPolicyToAttach(accountId: string, region: string, thingName: string) {
    return {
      "Version": "2012-10-17",
      "Statement": [
        {
          "Effect": "Allow",
          "Action": [
            "iot:Connect"
          ],
          "Resource": [
            "*"
          ]
        },
        {
          "Effect": "Allow",
          "Action": [
            "iot:Receive",
            "iot:Subscribe"
          ],
          "Resource": [
            `arn:aws:iot:${region}:${accountId}:topic/$aws/things/${thingName}/*`,
            `arn:aws:iot:${region}:${accountId}:topicfilter/$aws/things/${thingName}/*`,
          ]
        },
      ]
    }
  }

  addLogsPolicyToFunction(fn: lambda.Function, region: string, accountId: string, funcName: string) {
    fn.addToRolePolicy(new iam.PolicyStatement({
      effect: iam.Effect.ALLOW,
      actions: ["logs:CreateLogGroup"],
      resources: [`arn:aws:logs:${region}:${accountId}:*`]
    }))

    fn.addToRolePolicy(new iam.PolicyStatement({
        effect: iam.Effect.ALLOW,
        actions: [
          "logs:CreateLogStream",
          "logs:PutLogEvents"
        ],
        resources: [
          `arn:aws:logs:${region}:${accountId}:log-group:/aws/lambda/${funcName}:*`
        ]
      })
    )


  }
}
