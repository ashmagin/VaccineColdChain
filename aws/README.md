# ColdChain AWS Infrastructure

## Demo AWS Account

Console: https://944167459456.signin.aws.amazon.com/console
Region: `eu-central-1`
IoT Endpoint: `a1fr79kqcc2q4h-ats.iot.eu-central-1.amazonaws.com`
Thing Name: [container42](https://eu-central-1.console.aws.amazon.com/iot/home?region=eu-central-1#/thing/container42/namedShadow/Classic%20Shadow)

## Device Shadow Structure (Classic)

```
{
  "state": {
    "desired": {
      "welcome": "aws-iot"
    },
    "reported": {
      "fall": false,
      "location": "39.09326,-94.62767",
      "geoJSON": {
        "type": "Point",
        "coordinates": [
          -94.62767,
          39.09326
        ]
      },
      "pressure": 112,
      "humidity": 243,
      "temperature": 39,
      "luminance": 2877,
    }
  }
}
```

## Simulator

```
cd ./simulator
npm i
npm start
```

## Backend (CDK)

####Technologies and libraries
- aws cli
- npm
- cdk
- typescript

```
cd ./infra
npm i
npm run build
cdk deploy
```


### IoT Defender

```
npm run add-defender
```

## Frontend (Amplify)

```
amplify init # default options 
amplify add hosting # choose Manual Deployment
amplify publish
```

For setting up a local dev folder, from an existing amplify repo, use an 

```
amplify dev pull
```