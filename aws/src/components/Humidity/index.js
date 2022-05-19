import React from 'react';
import PropTypes from 'prop-types';
import { createUseStyles } from 'react-jss';
import { ImDroplet } from 'react-icons/im';

import styles from './styles';

const useStyles = createUseStyles(styles);

const Humidity = ({ value }) => {
  const classes = useStyles();
  
  return (
    <div className={classes.wrapper}>
      <div className={classes.icon}>
        <ImDroplet />
      </div>
      {value}%
    </div>
  )
};

Humidity.propTypes = {
  value: PropTypes.number,
};

export default Humidity;
