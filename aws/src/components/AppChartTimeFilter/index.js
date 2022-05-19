import React from 'react';
import { createUseStyles } from 'react-jss';
import { useDispatch, useSelector } from 'react-redux';

import { FILTERS } from 'utils/constants/sensors';
import AppChartTimeFilterItem from 'components/AppChartTimeFilterItem';
import { setHistoryFilter } from 'slices/sensorInfoSlice';

import styles from './styles';

const useStyles = createUseStyles(styles);

const AppChartTimeFilter = () => {
  const classes = useStyles();
  const dispatch = useDispatch();
  const historyFilter = useSelector((state) => state.sensorInfo.historyFilter);
  const handleChange = (newFilter) => {
    dispatch(setHistoryFilter(newFilter));
  };
  
  return (
    <div className={classes.wrapper}>
      <div className={classes.label}>
        Select:
      </div>
      <div className={classes.wrapperItemsSup}>
        <div className={classes.wrapperItems}>
          {FILTERS.map((filter) => (
            <AppChartTimeFilterItem
              key={filter}
              filter={filter}
              isActive={historyFilter === filter}
              onClick={handleChange}
            />
          ))}
        </div>
      </div>
    </div>
  )
};

export default AppChartTimeFilter;
