import React from 'react';
import { createUseStyles } from 'react-jss';
import PropTypes from 'prop-types';

import styles from './styles';

const useStyles = createUseStyles(styles);

const AppChartLegend = ({ items }) => {
  const classes = useStyles();
  
  return (
    <div className={classes.wrapper}>
      {items.map((item) => (
        <div key={item.label} style={{ color: item.color }}>
          {item.label}
        </div>
      ))}
    </div>
  )
};

AppChartLegend.propTypes = {
  items: PropTypes.arrayOf(PropTypes.shape({
    label: PropTypes.string,
    color: PropTypes.string,
  })),
}

export default AppChartLegend;
