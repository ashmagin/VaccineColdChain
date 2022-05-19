import { createAsyncThunk, createSelector, createSlice } from '@reduxjs/toolkit'
import orderBy from 'lodash/orderBy';

import { getHistoryLast } from 'utils/api';
import { transformHistory } from 'utils/helpers/transforms';
import { FILTER_1w } from 'utils/constants/sensors';
import { getLatestHistory } from 'utils/helpers/history';

const initialState = {
  temperature: null,
  humidity: null,
  lastUpdatedTime: Date.now() - 7 * 60 * 1000, // 7 minutes ago, to simulate offline as default state
  speed: 0,
  pressure: 0,
  fall: 0,
  location: [ -94.57276, 39.09908 ], // lon, lat
  alerts: [],
  isLoadingHistory: undefined,
  history: [],
  historyFiltered: [],
  historyFilter: FILTER_1w,
}

export const fetchHistory = createAsyncThunk(
  'sensorInfo/fetchHistory',
  async () => {
    const response = await getHistoryLast()
    const data = response.data?.data || []
    data.sort((a, b) => a.timestamp - b.timestamp);
    return data
  }
);

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
    setHistoryFilter: (state, action) => {
      state.historyFilter = action.payload;
      state.historyFiltered = getLatestHistory(action.payload, state.history);
    },
    addHistoryItem: (state, action) => {
      state.history.push(action.payload);
      state.historyFiltered = getLatestHistory(state.historyFilter, state.history);
    },
  },
  extraReducers: (builder) => {
    builder.addCase(fetchHistory.fulfilled, (state, action) => {
      state.isLoadingHistory = false;
      state.history = orderBy(transformHistory(action.payload), [ 'timestamp' ], [ 'asc' ]);
      state.lastUpdatedTime = state.history[state.history.length - 1]?.timestamp || Date.now();
      state.historyFiltered = getLatestHistory(state.historyFilter, state.history);
      state.temperature = state.history[state.history.length - 1].temperature;
      state.humidity = state.history[state.history.length - 1].humidity;
    });
    builder.addCase(fetchHistory.pending, (state) => {
      state.isLoadingHistory = true;
      state.history = [];
    });
    builder.addCase(fetchHistory.rejected, (state) => {
      state.isLoadingHistory = false;
      state.history = [];
    });
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
  setHistoryFilter,
  addHistoryItem,
} = sensorInfoSlice.actions;

export const alertsSelector = createSelector(
  state => state.sensorInfo.alerts,
  alerts => orderBy(alerts, [ 'timestamp' ], [ 'desc' ])
);

export const latestHistoryItemSelector = createSelector(
  state => state.sensorInfo.historyFiltered,
  historyFiltered => orderBy(historyFiltered, [ 'timestamp' ], [ 'desc' ])[0]
);

export default sensorInfoSlice.reducer;
