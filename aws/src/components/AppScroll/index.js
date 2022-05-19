import React from 'react';
import PropTypes from 'prop-types';
import { Scrollbars } from 'react-custom-scrollbars';

const AppScroll = ({ children }) => {
  return (
    <Scrollbars hideTracksWhenNotNeeded>
      {children}
    </Scrollbars>
  )
};

AppScroll.propTypes = {
  children: PropTypes.node,
};

export default AppScroll;
