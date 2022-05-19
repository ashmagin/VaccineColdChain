import { createSlice } from '@reduxjs/toolkit'

const initialState = {
  temperature: null,
  humidity: null,
  lastUpdatedTime: null,
  speed: 0,
  pressure: 0,
  fall: 0,
  location: null,
  alerts: [],
}

export const sensorInfoSlice = createSlice({
  name: 'sensorInfo',
  initialState,
  reducers: {
    setTemperature: (state, action) => {
      state.temperature = action.payload;
    },
    setHumidity: (state, action) => {
      state.humidity = action.payload;
    },
    setPressure: (state, action) => {
      state.pressure = action.payload;
    },
    setSpeed: (state, action) => {
      state.speed = action.payload;
    },
    setFall: (state, action) => {
      state.fall = action.payload;
    },
    setLastUpdatedTime: (state, action) => {
      state.lastUpdatedTime = action.payload;
    },
    setLocation: (state, action) => {
      state.location = action.payload;
    },
    setAlerts: (state, action) => {
      state.alerts.push(action.payload);
    },
  },
})

export const {
  setTemperature,
  setHumidity,
  setLastUpdatedTime,
  setSpeed,
  setPressure,
  setFall,
  setLocation,
  setAlerts,
} = sensorInfoSlice.actions

export default sensorInfoSlice.reducer