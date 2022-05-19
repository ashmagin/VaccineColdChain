import axios from 'axios';

import { Auth } from 'utils/helpers/aws';

// eslint-disable no-unused-vars
export const getHistory = async () => {
  let currSession = await Auth.currentSession()
  const token = currSession.getIdToken().getJwtToken()
  
  return axios.get(process.env.REACT_APP_HISTORY_ENDPOINT, {
    params: {
      deviceId: process.env.REACT_APP_THING_NAME,
    },
    headers: {
      'content-type': 'text/json',
      'Authorization': token
    },
  });
};

// returns last 500 records with descending sort by time
export const getHistoryLast = async () => {
  let currSession = await Auth.currentSession()
  const token = currSession.getIdToken().getJwtToken()

  return axios.get(process.env.REACT_APP_HISTORY_ENDPOINT, {
    params: {
      deviceId: process.env.REACT_APP_THING_NAME,
      sortDesc: true,
      limit: 500,
    },
    headers: {
      'content-type': 'text/json',
      'Authorization': token
    },
  });
};