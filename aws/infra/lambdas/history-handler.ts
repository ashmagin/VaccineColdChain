import {DynamoDB} from "aws-sdk";
import {DocumentClient} from 'aws-sdk/clients/dynamodb';

let ddb = new DynamoDB.DocumentClient();
const DEFAULT_LIMIT = 100

exports.handler = async (event: any) => {

  type response = {
    statusCode: number;
    headers: object;
    body?: string
  };

  let res: response = {
    "statusCode": 200,
    "headers": {
      "Access-Control-Allow-Origin": "*",
      "Access-Control-Allow-Methods": "OPTIONS,GET",
      "Content-Type": "application/json"
    }
  };

  // DISPLAY ALL ATTRIBUTES SENT FROM device_id
  console.log("Received event: " + JSON.stringify(event))

  if (!event["queryStringParameters"] || !event["queryStringParameters"]['deviceId']) {
    res["statusCode"] = 400
    res["body"] = JSON.stringify(getError("Empty request"))
    console.error("responded with: ", JSON.stringify(res))
    return res
  }

  const deviceId = event["queryStringParameters"]['deviceId']
  const nextToken = event["queryStringParameters"]['nextToken']
  const sortDesc = event["queryStringParameters"]['sortDesc']
  let limit = event["queryStringParameters"]['limit']
  if (!limit || isNaN(limit) || limit < 1 || limit > 1000) {
    limit = DEFAULT_LIMIT
  }

  try {
    const resp = await getFromDb(deviceId, nextToken, sortDesc, limit)
    res["body"] = JSON.stringify({
      data: resp.Items ?? [],
      nextToken: resp["LastEvaluatedKey"] ? resp.LastEvaluatedKey['timestamp'] : null
    })
    return res
  } catch (error) {
    console.error(error)
    res["statusCode"] = 500
    res["body"] = JSON.stringify(getError("error during the request" + error.message))
    console.error("responded with: ", JSON.stringify(res))
    return res
  }

};

async function getFromDb(device_id: string, nextToken: string, sortDesc: boolean, limit: number): Promise<any> {
  const table_name = process.env.DEVICE_TABLE_NAME || ""

  const params: DocumentClient.QueryInput = {
    TableName: table_name,
    KeyConditionExpression: 'device_id = :device_id',
    ExpressionAttributeValues: {
      ':device_id': device_id
    },
    ScanIndexForward: !sortDesc,
    Limit: limit
  };

  if (nextToken) {
    params.ExclusiveStartKey = {device_id: device_id, timestamp: nextToken}
  }
  return await ddb.query(params).promise();
}

function getError(msg: string): object {
  return {
    "message": msg
  }
}




