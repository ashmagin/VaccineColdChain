import React from 'react';
import PropTypes from 'prop-types';
import { createUseStyles } from 'react-jss';
import { FaTemperatureHigh, FaTemperatureLow } from 'react-icons/fa';

import { LIMIT_TEMPERATURE } from 'utils/constants/sensors';

import styles from './styles';

const useStyles = createUseStyles(styles);

const Temperature = ({ value }) => {
  const classes = useStyles();
  
  return (
    <div className={classes.wrapper}>
      <div className={classes.icon}>
        {value > LIMIT_TEMPERATURE ? <FaTemperatureHigh /> : <FaTemperatureLow />}
      </div>
      {value}℃
    </div>
  )
};

Temperature.propTypes = {
  value: PropTypes.number,
};

export default Temperature;
