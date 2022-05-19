import React, { useState } from 'react';
import { useSelector } from 'react-redux';
import { createUseStyles } from 'react-jss';
import { FaCheckCircle } from 'react-icons/fa';
import { IoCloudOfflineOutline } from 'react-icons/io5';

import { useInterval } from 'utils/hooks';
import { STATUS_OFFLINE, STATUS_ONLINE } from 'utils/constants/sensors';
import { isLive } from 'utils/helpers/status';

import styles from './styles';

const useStyles = createUseStyles(styles);

const Status = () => {
  const classes = useStyles();
  const lastUpdatedTime = useSelector((state) => state.sensorInfo.lastUpdatedTime);
  const [ value, setValue ] = useState(STATUS_OFFLINE);
  
  useInterval(() => {
    setValue(isLive(lastUpdatedTime) ? STATUS_ONLINE : STATUS_OFFLINE);
  }, 5000);
  
  return (
    <div className={classes.wrapper}>
      {value === STATUS_ONLINE ? (
        <div className={classes.wrapperOnline}>
          <FaCheckCircle />
        </div>
      ) : (
        <div className={classes.wrapperOffline}>
          <IoCloudOfflineOutline />
        </div>
      )}
      {value}
    </div>
  )
};

export default Status;
