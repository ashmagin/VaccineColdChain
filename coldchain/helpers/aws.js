import Amplify, { Auth } from 'aws-amplify';
import AWSIoTData from 'aws-iot-device-sdk';
import { COGNITO_IDENTITY_POOL_ID } from '../config/credentials';
import { BACKEND_REGION, AWS_IOT_ENDPOINT, DEMO_THING_NAME } from '../config/backend';


const subscribeDeviceIoT = async () => {
  Amplify.configure({
    region: BACKEND_REGION,
    identityPoolId: COGNITO_IDENTITY_POOL_ID
  });
  
  const clientId = 'mqtt-stdevcon2021-' + (Math.floor((Math.random() * 100000) + 1));
  const currentCredentials = await Auth.currentCredentials();
  const essentialCredentials = Auth.essentialCredentials(currentCredentials);
  const newMqttClient = AWSIoTData.thingShadow({
    region: BACKEND_REGION,
    host: AWS_IOT_ENDPOINT,
    clientId: clientId,
    protocol: 'wss',
    maximumReconnectTimeMs: 8000,
    debug: true,
    accessKeyId: essentialCredentials.accessKeyId,
    secretKey: essentialCredentials.secretAccessKey,
    sessionToken: essentialCredentials.sessionToken
  });

  newMqttClient.on('connect', function () {
    console.log('connected to AWS IoT.');
    newMqttClient.register(DEMO_THING_NAME, {
      persistentSubscribe: true
    },
    function(error, failedTopics) {
      if (error != null) {
        console.log('Failed to connect to AWS IoT: ' + error);
      }
    });
  });
  
  return newMqttClient;
};

const unsubscribeDevice = (client) => {
  client.unregister(DEMO_THING_NAME);
};

export {
  subscribeDeviceIoT,
  unsubscribeDevice
};
