import React from 'react';
import {
  BrowserRouter as Router,
  Route,
  Switch
} from 'react-router-dom';

import Dashboard from 'features/Dashboard';

const AppRouter = () => {
  return (
    <Router>
      <Switch>
        <Route path="/">
          <Dashboard />
        </Route>
      </Switch>
    </Router>
  );
};

export default AppRouter;
