import React from 'react';
import { createUseStyles } from 'react-jss';

import Header from 'components/Header';
import SensorInfo from 'components/SensorInfo';
import AppMap from 'components/AppMap';
import DataSubscriber from 'components/DataSubscriber';
import AppChart from 'components/AppChart';
import AppNotification from 'components/AppNotification';
import AppTimeline from 'components/Alerts';

import styles from './styles';

const useStyles = createUseStyles(styles);

const Dashboard = () => {
  const classes = useStyles();
  
  return (
    <div className={classes.wrapper}>
      <Header />
      <SensorInfo />
      <div className={classes.wrapperUIData}>
        <div className={classes.wrapperItems}>
          <div className={classes.wrapperLog}>
            <AppTimeline />
          </div>
          <div className={classes.wrapperMap}>
            <AppMap />
          </div>
        </div>
        <AppChart />
      </div>
      <DataSubscriber />
      <AppNotification />
    </div>
  );
};

export default Dashboard;

