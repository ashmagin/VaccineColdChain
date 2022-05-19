import React from 'react';
import LinearGradientView from '../components/gradient-view';
import PropTypes from 'prop-types';
import Header from '../components/header';
import SensorInfo from '../components/sensor-info';
import MapView from '../components/map-view/map-view';
import ChartView from '../components/chart-view/chart-view';
import DataSubscriber from '../components/data-subscriber';
import AlertsList from '../components/alerts-list';
import { SafeAreaView } from 'react-native-safe-area-context';
import { COLOR_BACKGROUND } from '../utils/constants/colors';

class Main extends React.Component {
    static propTypes = {
      navigation: PropTypes.shape({navigate: PropTypes.func}),
    };
  
    render() {
        //TODO: implement navigation if needed
      //const {navigation} = this.props;
      //const items = navigation.getParam('items');
      return (
        <SafeAreaView style={{flex: 1, backgroundColor: COLOR_BACKGROUND}}>
          <LinearGradientView>
            <Header/>
            <SensorInfo/>
            <MapView/>
            <ChartView/>
            <AlertsList/>
            <DataSubscriber/>
          </LinearGradientView>
        </SafeAreaView>
      );
    }
}
  
export default Main;
