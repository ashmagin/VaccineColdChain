import { NotificationManager } from 'react-notifications';

import { LIMIT_TEMPERATURE, TYPE_FALL, TYPE_TEMPERATURE } from 'utils/constants/sensors';

const handleTemperature = (temperature, callback) => {
  if (temperature > LIMIT_TEMPERATURE) {
    NotificationManager.error('Detected temperature rise', 'Attention', 3000);
    callback();
  }
};

const handleFall = (fall, callback) => {
  if (fall) {
    NotificationManager.error('Fall detected', 'Attention', 3000);
    callback();
  }
};

const getTitleFromAlarm = (type) => {
  switch (type) {
    case TYPE_FALL:
      return 'Fall alarm';
    case TYPE_TEMPERATURE:
      return 'Temperature alarm';
    default:
      return 'Another alarm';
  }
};

export {
  getTitleFromAlarm,
  handleTemperature,
  handleFall,
};
