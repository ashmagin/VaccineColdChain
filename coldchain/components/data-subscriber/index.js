import React, { useEffect, useState } from 'react';
import { useDispatch } from 'react-redux';
import { subscribeDeviceIoT, unsubscribeDevice } from '../../helpers/aws';
import { handleFall, handleTemperature } from '../../helpers/alarms';
import { TYPE_FALL, TYPE_TEMPERATURE } from '../../utils/constants/sensors';
import {
  setAlerts,
  setFall,
  setHumidity,
  setLastUpdatedTime,
  setLocation,
  setPressure,
  setSpeed,
  setTemperature,
} from '../sensor-info/sensor-info-slice';

const DataSubscriber = () => {
  const dispatch = useDispatch();
  const [ client, setClient ] = useState();
  const updateData = ({ temperature, humidity, speed, fall, pressure, geoJSON }) => {
    handleFall(fall, () => {
      dispatch(setAlerts({ type: TYPE_FALL, timestamp: Date.now() }));
    });
    handleTemperature(temperature, () => {
      dispatch(setAlerts({ type: TYPE_TEMPERATURE, timestamp: Date.now() }));
    });
    
    if (temperature) {
      dispatch(setTemperature(temperature));
    }
    
    if (humidity) {
      dispatch(setHumidity(humidity));
    }
    
    if (speed) {
      dispatch(setSpeed(speed));
    }
    
    if (fall) {
      dispatch(setFall(fall));
    }
    
    if (pressure) {
      dispatch(setPressure(pressure));
    }
    
    if (geoJSON?.coordinates) {
      dispatch(setLocation(geoJSON?.coordinates));
    }
    
    dispatch(setLastUpdatedTime(Date.now()));
  };
  
  useEffect(() => {
    let clientLocal = null;
    
    subscribeDeviceIoT().then((newClient) => {
      clientLocal = newClient;
      setClient(newClient);
    });
    
    return () => {
      if (clientLocal) {
        unsubscribeDevice(clientLocal);
      }
    };
  }, []);
  
  useEffect(() => {
    if (!client) return;
    client.on('foreignStateChange', function (name, operation, stateObject) {
      if (stateObject?.state?.reported) {
        updateData(stateObject.state.reported);
        console.log('Received new data: ' + JSON.stringify(stateObject.state.reported));
      }
    });
    client.on('status', function (name, statusType, clientToken, stateObject) {
      if (stateObject?.state?.reported) {
        updateData(stateObject.state.reported);
        console.log('Received new data: ' + JSON.stringify(stateObject.state.reported));
      }
    });
    client.on('offline', function () {
      console.log('Disconnected from AWS IoT');
    });
    client.on('error', function (error) {
      console.log('MQTT client error: ' + error);
    });
  }, [ client ]);
  
  return (
    <></>
  );
};

export default DataSubscriber;
