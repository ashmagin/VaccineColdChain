import { formatChartTime } from 'utils/helpers/time';

const transformHistory = (rawHistory) => {
  return rawHistory.map((history) => ({
    temperature: history?.reported?.temperature || null,
    humidity: history?.reported?.humidity || null,
    dateTime: formatChartTime((history.timestamp || 0) / 1000),
    timestamp: history?.timestamp || Date.now(),
  }));
};

const transformStringToMulti = (basestring, count) => {
  if (count > 1) return `${basestring}s`;
  return basestring;
};

export {
  transformHistory,
  transformStringToMulti,
};
