import React, { useEffect, useState } from 'react';
import { useDispatch } from 'react-redux';

import { subscribeDeviceIoT, unsubscribeDevice } from 'utils/helpers/aws';
import {
  fetchHistory,
  setAlerts,
  setFall,
  setHumidity,
  setLastUpdatedTime,
  setLocation,
  setPressure,
  setSpeed,
  setTemperature,
} from 'slices/sensorInfoSlice';
import { handleFall, handleTemperature } from 'utils/helpers/alarms';
import { TYPE_FALL, TYPE_TEMPERATURE } from 'utils/constants/sensors';

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
    
    if (temperature !== undefined && temperature !== null) {
      dispatch(setTemperature(temperature));
    }
    
    if (humidity !== undefined && humidity !== null) {
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
    
    dispatch(fetchHistory());
    
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
      }
    });
    client.on('status', function (name, statusType, clientToken, stateObject) {
      if (stateObject?.state?.reported) {
        updateData(stateObject.state.reported);
      }
    });
  }, [ client ]);
  
  return (
    <></>
  );
};

export default DataSubscriber;
