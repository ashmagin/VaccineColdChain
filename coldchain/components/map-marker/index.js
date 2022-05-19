import React from 'react';
import { StyleSheet, View } from 'react-native';
import { COLOR_WARNING } from '../../utils/constants/colors';

const styles = StyleSheet.create({
  wrapper: {
    position: 'relative',
    width: 30,
    height: 30,
    display: 'flex',
    justifyContent: 'center',
    alignItems: 'center'
  },
  wrapperOuter: {
    backgroundColor: 'transparent',
    borderWidth: 4,
    borderColor: COLOR_WARNING,
    borderStyle: 'solid',
    position: 'absolute',
    top: 0,
    left: 0,
    right: 0,
    bottom: 0,
    borderRadius: 15,
    justifyContent: 'center',
    alignItems: 'center',
  },
  wrapperInner: {
    width: 12,
    height: 12,
    borderRadius: 4,
    backgroundColor: COLOR_WARNING,
    position: 'absolute',
  },
});

class MapMarker extends React.Component {
  render() {
    return (
      <View style={styles.wrapper}>
        <View style={styles.wrapperOuter}>
          <View style={styles.wrapperInner}></View>
        </View>
      </View>
    );
  }
}

export default MapMarker;