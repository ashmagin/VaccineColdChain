import moment from 'moment';

import { transformStringToMulti } from 'utils/helpers/transforms';

const getDiffUpdate = (startTimeTs) => {
  let seconds = Math.floor((Date.now() - (startTimeTs)) / 1000);
  let minutes = Math.floor(seconds / 60);
  let hours = Math.floor(minutes / 60);
  const days = Math.floor(hours / 24);
  let result = '';
  
  hours = hours - (days * 24);
  minutes = minutes - (days * 24 * 60) - (hours * 60);
  
  if (hours > 0) {
    result += `${hours} ${transformStringToMulti('hour', hours)} `
  }
  
  result += `${minutes} ${transformStringToMulti('minute', minutes)}`;
  
  return `${result} ago`;
};

const formatChartTime = (timestamp) => {
  return moment.unix(timestamp % 1 === 0 ? timestamp / 1000 : Math.round(timestamp)).format('YYYY/MM/DD HH:mm');
}

export {
  getDiffUpdate,
  formatChartTime,
};
