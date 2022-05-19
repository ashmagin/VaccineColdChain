import AWS from 'aws-sdk';
import { Auth, Signer } from 'aws-amplify';
import AWSIoTData from 'aws-iot-device-sdk';

const region = process.env.REACT_APP_AWS_REGION;
AWS.config.region = region;

Auth.configure({
  mandatorySignIn: true,
  region: process.env.REACT_APP_IDENTITY_POOL_ID.split(':')[0],
  userPoolId: process.env.REACT_APP_USER_POOL,
  userPoolWebClientId: process.env.REACT_APP_CLIENT_ID,
});

export const getAwsCredentials = (token) => (
  new Promise((resolve, reject) => {
    const providerKey = `cognito-idp.${region}.amazonaws.com/${process.env.REACT_APP_USER_POOL}`;
    
    AWS.config.credentials = new AWS.CognitoIdentityCredentials({
      IdentityPoolId: process.env.REACT_APP_IDENTITY_POOL_ID,
      Logins: {
        [providerKey]: token,
      },
    });

    AWS.config.credentials.get((error) => {
      if (error) {
        reject(error);
      }
      
      const { accessKeyId, secretAccessKey, sessionToken, identityId } = AWS.config.credentials;
      const credentialSubset = { accessKeyId, secretAccessKey, sessionToken, identityId };
      resolve(credentialSubset);
    });
  })
);

let credentials

async function refreshCredentials() {
  await AWS.config.credentials.refreshPromise();
  
  setTimeout(refreshCredentials, AWS.config.credentials.expireTime - new Date());
}

async function refreshCognito() {
  let currSession = await Auth.currentSession()
  const token = currSession.getIdToken().getJwtToken()
  credentials = await getAwsCredentials(token)

  await AWS.config.credentials.expireTime

  setTimeout(refreshCognito, AWS.config.credentials.expireTime - new Date() - 60000 );
}

const transformRequest = (url, resourceType) => {
  if (resourceType === 'Style' && !url.includes('://')) {
    url = `https://maps.geo.${AWS.config.region}.amazonaws.com/maps/v0/maps/${url}/style-descriptor`;
  }
  
  if (url.includes('amazonaws.com')) {
    return {
      url: Signer.signUrl(url, {
        access_key: credentials.accessKeyId,
        secret_key: credentials.secretAccessKey,
        session_token: credentials.sessionToken,
      }),
    };
  }
  
  return { url };
};


const attachPrincipalPolicy = (policyName, principal) => {
  new AWS.Iot().attachPrincipalPolicy({ policyName: policyName, principal: principal }, function (err) {
    if (err) {
      console.error(err); // an error occurred
    }
  });
}

const subscribeDeviceIoT = async () => {
  let currSession = await Auth.currentSession()
  const token = currSession.getIdToken().getJwtToken()
  
  credentials = await getAwsCredentials(token)
  
  const clientId = 'mqtt-stdevcon2021-' + (Math.floor((Math.random() * 100000) + 1));
  const essentialCredentials = Auth.essentialCredentials(credentials);
  attachPrincipalPolicy(process.env.REACT_APP_IOT_POLICY, credentials.identityId);
  
  const newMqttClient = AWSIoTData.thingShadow({
    region: process.env.REACT_APP_AWS_REGION,
    host: process.env.REACT_APP_IOT_ENDPOINT,
    clientId: clientId,
    protocol: 'wss',
    maximumReconnectTimeMs: 800000,
    debug: false,
    accessKeyId: essentialCredentials.accessKeyId,
    secretKey: essentialCredentials.secretAccessKey,
    sessionToken: essentialCredentials.sessionToken
  });
  
  newMqttClient.on('connect', function () {
    console.log('connected to AWS IoT.');
    newMqttClient.register(process.env.REACT_APP_THING_NAME, {
      persistentSubscribe: true
    });
  });
  
  return newMqttClient;
};

export const unsubscribeDevice = (client) => {
  client.unregister(process.env.REACT_APP_THING_NAME);
};

export {
  Auth,
  transformRequest,
  subscribeDeviceIoT,
  refreshCredentials,
  attachPrincipalPolicy,
  credentials,
  refreshCognito,
};
