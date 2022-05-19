import { configureStore } from '@reduxjs/toolkit'

import sensorInfoReducer from 'slices/sensorInfoSlice';

export default configureStore({
  reducer: {
    sensorInfo: sensorInfoReducer,
  },
});
