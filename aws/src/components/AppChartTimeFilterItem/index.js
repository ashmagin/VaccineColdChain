import React from 'react';
import { createUseStyles } from 'react-jss';
import PropTypes from 'prop-types';

import styles from './styles';

const useStyles = createUseStyles(styles);

const AppChartTimeFilterItem = ({ filter, isActive, onClick }) => {
  const classes = useStyles();
  const handleClick = React.useCallback(() => {
    onClick(filter);
  }, [ onClick, filter ]);
  
  return (
    <div className={`${classes.wrapper} ${isActive && classes.wrapperActive}`} onClick={handleClick}>
      {filter}
    </div>
  );
};

AppChartTimeFilterItem.propTypes = {
  onClick: PropTypes.func,
  filter: PropTypes.string,
  isActive: PropTypes.bool,
};

export default AppChartTimeFilterItem;
