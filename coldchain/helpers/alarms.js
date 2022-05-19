import PushNotification from 'react-native-push-notification';
import { LIMIT_TEMPERATURE, TYPE_FALL, TYPE_TEMPERATURE } from '../utils/constants/sensors';

function showNotification(text, title) {
  PushNotification.localNotification({
    title: title,
    message: text,
    channelId: 'fcm_fallback_notification_channel',
  });
}

const handleTemperature = (temperature, callback) => {
  if (temperature > LIMIT_TEMPERATURE) {
    showNotification('Detected temperature rise', 'Attention');
    callback();
  }
};

const handleFall = (fall, callback) => {
  if (fall) {
    showNotification('Fall detected', 'Attention');
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
