import React from 'react';
import LinearGradient from 'react-native-linear-gradient';
import { COLOR_GRAY_LEVEL_3, COLOR_GRAY_LEVEL_5, COLOR_BACKGROUND } from '../../utils/constants/colors';

class LinearGradientView extends React.Component {
  render() {
    return (
      <LinearGradient colors={[COLOR_BACKGROUND , COLOR_BACKGROUND]} style={{flex: 1}}>
        {this.props.children}
      </LinearGradient>
    );
  };
}

export default LinearGradientView;