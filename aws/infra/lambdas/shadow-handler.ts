import {DynamoDB} from "aws-sdk";

let ddb = new DynamoDB.DocumentClient();

exports.handler = async (event: any) => {
  // DISPLAY ALL ATTRIBUTES SENT FROM DEVICE
  console.log("Received event: " + JSON.stringify(event))
  if (!event['reported'] || !event['device_id']) {
    return
  }

  try {
    await writeToDb(event['reported'], event['device_id'])
  } catch (error) {
    console.error(error)
  }

  return {};
};

async function writeToDb(values: string[], device_id: string): Promise<any> {
  const table_name = process.env.DEVICE_TABLE_NAME || ""
  const params = {
    Item: {"reported": values, "device_id": device_id, "timestamp": (new Date()).getTime()},
    ReturnConsumedCapacity: "TOTAL",
    TableName: table_name
  };

  return ddb.put(params).promise()
}
