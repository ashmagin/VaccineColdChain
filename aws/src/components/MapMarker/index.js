import React from 'react';
import { createUseStyles } from 'react-jss';

import styles from './styles';

const useStyles = createUseStyles(styles);

const MapMarker = () => {
  const classes = useStyles();
  
  return (
    <div className={classes.wrapper}>
      <div className={classes.wrapperOuter} />
      <div className={classes.wrapperInner} />
    </div>
  )
};

export default MapMarker;
