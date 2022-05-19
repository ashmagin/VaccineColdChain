import React, { useEffect } from 'react';
import { createUseStyles } from 'react-jss';
import { useDispatch, useSelector } from 'react-redux';
import { Area, AreaChart, CartesianGrid, ResponsiveContainer, Tooltip, XAxis, YAxis } from 'recharts';

import AppChartTimeFilter from 'components/AppChartTimeFilter';
import { addHistoryItem } from 'slices/sensorInfoSlice';
import { formatChartTime } from 'utils/helpers/time';
import {
  COLOR_INFO_LEVEL_5,
  COLOR_INFO_LEVEL_6,
  COLOR_WARNING_LEVEL_2,
  COLOR_WARNING_LEVEL_5
} from 'utils/constants/colors';
import AppChartLegend from 'components/AppChartLegend';
import { LEGEND } from 'utils/constants/chart';

import styles, { styleArea, styleAxis } from './styles';

const useStyles = createUseStyles(styles);

const AppChart = () => {
  const classes = useStyles();
  const dispatch = useDispatch();
  const {
    temperature,
    humidity,
    lastUpdatedTime,
    historyFiltered,
    isLoadingHistory
  } = useSelector((state) => state.sensorInfo);
  
  useEffect(() => {
    dispatch(addHistoryItem({
      timestamp: lastUpdatedTime,
      dateTime: formatChartTime(lastUpdatedTime),
      temperature,
      humidity,
    }));
  }, [ temperature, humidity, lastUpdatedTime, isLoadingHistory ]);
  
  return (
    <div className={classes.wrapper}>
      <AppChartTimeFilter />
      <ResponsiveContainer
        width="100%"
      >
        <AreaChart
          data={historyFiltered}
          margin={styleArea}
        >
          <CartesianGrid strokeDasharray="3 3" />
          <defs>
            <linearGradient
              id="colorTemp"
              x1="0"
              y1="0"
              x2="0"
              y2="1"
            >
              <stop
                offset="5%"
                stopColor={COLOR_INFO_LEVEL_5}
                stopOpacity={0.9}
              />
              <stop
                offset="95%"
                stopColor={COLOR_INFO_LEVEL_5}
                stopOpacity={0.1}
              />
            </linearGradient>
            <linearGradient
              id="colorHum"
              x1="0"
              y1="0"
              x2="0"
              y2="1"
            >
              <stop
                offset="5%"
                stopColor={COLOR_WARNING_LEVEL_5}
                stopOpacity={0.9}
              />
              <stop
                offset="95%"
                stopColor={COLOR_WARNING_LEVEL_5}
                stopOpacity={0.1}
              />
            </linearGradient>
          </defs>
          <XAxis dataKey="dateTime" tick={styleAxis} />
          <YAxis yAxisId="left" tick={styleAxis} />
          <YAxis yAxisId="right" orientation="right" tick={styleAxis} />
          <Tooltip />
          <Area
            type="monotone"
            dataKey="temperature"
            yAxisId="left"
            stackId="1"
            strokeWidth={3}
            stroke={COLOR_INFO_LEVEL_6}
            fill="url(#colorTemp)"
            legendType="none"
            connectNulls
          />
          <Area
            type="monotone"
            dataKey="humidity"
            yAxisId="right"
            stackId="1"
            strokeWidth={3}
            stroke={COLOR_WARNING_LEVEL_2}
            fill="url(#colorHum)"
            legendType="none"
            connectNulls
          />
        </AreaChart>
      </ResponsiveContainer>
      <AppChartLegend items={LEGEND} />
    </div>
  )
};

export default AppChart;
