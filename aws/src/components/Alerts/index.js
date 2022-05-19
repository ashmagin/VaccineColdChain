import React from 'react';
import { useSelector } from 'react-redux';
import { createUseStyles } from 'react-jss';
import moment from 'moment';
import Timeline from '@material-ui/lab/Timeline';
import TimelineItem from '@material-ui/lab/TimelineItem';
import TimelineSeparator from '@material-ui/lab/TimelineSeparator';
import TimelineConnector from '@material-ui/lab/TimelineConnector';
import TimelineContent from '@material-ui/lab/TimelineContent';
import TimelineDot from '@material-ui/lab/TimelineDot';
import TimelineOppositeContent from '@material-ui/lab/TimelineOppositeContent';
import Typography from '@material-ui/core/Typography';

import AppScroll from 'components/AppScroll';
import { getTitleFromAlarm } from 'utils/helpers/alarms';
import { alertsSelector } from 'slices/sensorInfoSlice';

import styles from './styles';

const useStyles = createUseStyles(styles);

const Alerts = () => {
  const classes = useStyles();
  const alerts = useSelector(alertsSelector);
  
  return (
    <AppScroll>
      <div className={classes.wrapper}>
        <Timeline>
          {alerts.map((alert) => (
            <TimelineItem key={alert.timestamp}>
              <TimelineOppositeContent>
                <Typography color="textSecondary">
                  {moment(alert.timestamp).format('hh:mm:ss a')}
                </Typography>
              </TimelineOppositeContent>
              <TimelineSeparator>
                <TimelineDot />
                <TimelineConnector />
              </TimelineSeparator>
              <TimelineContent>
                <Typography>{getTitleFromAlarm(alert.type)}</Typography>
              </TimelineContent>
            </TimelineItem>
          ))}
        </Timeline>
        {alerts.length === 0 && (
          <div className={classes.noItems}>
            Relax, there are no alerts.
          </div>
        )}
      </div>
    </AppScroll>
  )
};

export default Alerts;
