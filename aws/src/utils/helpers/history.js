import { FILTER_12HR, FILTER_1d, FILTER_1w, FILTER_6HR } from 'utils/constants/sensors';

const getDelta = (timestamp) => (Date.now() - timestamp) / 1000;

const getLatestHistory = (filterType, items) => {
  const cleanItems = items;
  
  if (filterType === FILTER_6HR) {
    return cleanItems.filter((item) => getDelta(item.timestamp) <= 6 * 60 * 60);
  }
  
  if (filterType === FILTER_12HR) {
    return cleanItems.filter((item) => getDelta(item.timestamp) <= 12 * 60 * 60);
  }
  
  if (filterType === FILTER_1d) {
    return cleanItems.filter((item) => getDelta(item.timestamp) <= 24 * 60 * 60);
  }
  
  if (filterType === FILTER_1w) {
    return cleanItems.filter((item) => getDelta(item.timestamp) <= 7 * 24 * 60 * 60);
  }
};

export {
  getLatestHistory,
}
