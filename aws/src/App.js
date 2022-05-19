import React from 'react';
import { createUseStyles } from 'react-jss';
import { Provider } from 'react-redux';
import { withAuthenticator } from '@aws-amplify/ui-react'

import store from 'utils/store';
import AppRouter from 'features/AppRouter';

import styles from './AppStyles';

const useStyles = createUseStyles(styles);

function App() {
  useStyles();
  
  return (
    <Provider store={store}>
      <AppRouter />
    </Provider>
  );
}

export default withAuthenticator(App);
