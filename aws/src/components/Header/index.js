import React from 'react';
import { createUseStyles } from 'react-jss';

import Status from 'components/Status';

import styles from './styles';

const useStyles = createUseStyles(styles);

const Header = () => {
  const classes = useStyles();
  
  return (
    <div className={classes.wrapper}>
      <div className={classes.name}>
        {process.env.REACT_APP_DEVICE_NAME}
      </div>
      <Status />
    </div>
  )
};

export default Header;
