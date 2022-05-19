import React, { useState } from 'react';
import { createUseStyles } from 'react-jss';
import { useSelector } from 'react-redux';

import Humidity from 'components/Humidity';
import Temperature from 'components/Temperature';
import { getDiffUpdate } from 'utils/helpers/time';
import { useInterval } from 'utils/hooks';
import { latestHistoryItemSelector } from 'slices/sensorInfoSlice';

import styles from './styles';

const useStyles = createUseStyles(styles);

const SensorInfo = () => {
  const classes = useStyles();
  const { lastUpdatedTime } = useSelector((state) => state.sensorInfo);
  const history = useSelector(latestHistoryItemSelector);
  const [ diffTime, setDiffTime ] = useState('');
  
  useInterval(() => {
    setDiffTime(getDiffUpdate(lastUpdatedTime));
  }, 5000);
  
  return (
    <div className={classes.wrapper}>
      <div className={classes.wrapperEnvInfo}>
        <div className={classes.wrapperEnvItem}>
          <Temperature value={history?.temperature || 0} />
        </div>
        <div className={classes.wrapperEnvItem}>
          <Humidity value={history?.humidity || 0} />
        </div>
      </div>
      <div className={classes.wrapperTime}>
        {diffTime}
      </div>
    </div>
  )
};


export default SensorInfo;
