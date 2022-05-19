
import { configureStore } from '@reduxjs/toolkit'

import sensorInfoReducer from '../components/sensor-info/sensor-info-slice';

export default configureStore({
  reducer: {
    sensorInfo: sensorInfoReducer,
  },
});