import React from 'react';
import { StyleSheet, View } from 'react-native';
import { connect } from 'react-redux';
import { Grid, LineChart, AreaChart, YAxis } from "react-native-svg-charts"
import * as shape from 'd3-shape'
import {
  COLOR_WARNING_LEVEL_2,
  COLOR_INFO_LEVEL_3,
  COLOR_GRAY_BACKGROUND,
  COLOR_GRAPH_LINE_BLUE,
  COLOR_GRAPH_LINE_ORANGE
} from '../../utils/constants/colors';


const styles = StyleSheet.create({
  rootContainer: {
    height: '30%',
    flexDirection: 'row',
    backgroundColor: COLOR_GRAY_BACKGROUND,
    marginHorizontal: 15,
    marginBottom: 15,
    borderRadius: 8,
  },
  chartContainer: {
    flex: 1,
    marginLeft: 1,
    marginRight: 1, 
  },
  overlayChart: {
    width: '100%',
    height: '100%',
    position: 'absolute', 
  }
});

function mapStateToProps(state) {
  const sensorInfo = state.sensorInfo
  return { sensorInfo: sensorInfo };
}

class ChartView extends React.Component {
  dataHumidity = new Array();
  dataTemperature = new Array();
  firstRun = true;

  constructor(props) {
    super(props);
    
    this.state = {
      sensorInfo: props.sensorInfo,
    };

    this.mapContainer = null
  }

  componentWillReceiveProps(nextProps) {
    if (nextProps.sensorInfo != null) {
      this.setState({
        sensorInfo: nextProps.sensorInfo,
      });
    }

    if (this.state.sensorInfo.humidity != null && this.state.sensorInfo.temperature != null) {
      this.dataHumidity.push(this.state.sensorInfo.humidity);
      this.dataTemperature.push(this.state.sensorInfo.temperature);

      const arrayLengthLimit = 70;
      if (this.dataHumidity.length > arrayLengthLimit) {
        this.dataHumidity.shift();
      }
      if (this.dataTemperature.length > arrayLengthLimit) {
        this.dataTemperature.shift();
      }
    } 
  }

  render() {
    if (this.dataHumidity.length < 2 || this.dataTemperature.length < 2) {
      return null;
    }
    if (this.firstRun) {
      this.dataHumidity.shift();
      this.dataTemperature.shift();
      this.firstRun = false;
    }
    const chartDataHumidity = [
      {
        data: this.dataHumidity,
        svg: { 
          stroke: COLOR_GRAPH_LINE_ORANGE,
          strokeWidth: 2,
        },
        areaSvg: {
          fill: 'rgba(255, 153, 0, 0.05)',
        },
      }
    ];
    const chartDataTemperature = [
      {
        data: this.dataTemperature,
        svg: {
          stroke: COLOR_GRAPH_LINE_BLUE,
          strokeWidth: 2,
        },
        areaSvg: {
          fill: 'rgba(54, 178, 222, 0.05)',
        },
      },
    ];
    const contentInset = { top: 30, bottom: 30 };
    const ticks = 5;
    return (
      <View style={styles.rootContainer}>
        <YAxis
          style={{marginLeft: 2, width:40}}
          data={this.dataTemperature}
          contentInset={contentInset}
          svg={{
            fill: COLOR_GRAPH_LINE_BLUE,
            fontSize: 10,
          }}
          numberOfTicks={ticks}
          formatLabel={(value) => `${value}ºC`}
        />
  
        <View style={styles.chartContainer}>
          <LineChart
            style={{ flex: 1}}
            data={ chartDataHumidity }
            curve={shape.curveNatural}
            contentInset={contentInset}
            numberOfTicks={ticks}
          >
            <Grid svg={{stroke: '#303030'}} />
          </LineChart>
          <AreaChart
            style={styles.overlayChart}
            data={chartDataHumidity[0].data}
            curve={shape.curveNatural}
            contentInset={contentInset}
            numberOfTicks={ticks}
            svg={chartDataHumidity[0].areaSvg}
          />

          <LineChart
            style={styles.overlayChart}
            data={ chartDataTemperature }
            curve={shape.curveNatural}
            contentInset={contentInset}
            numberOfTicks={ticks}
          />
          <AreaChart
            style={styles.overlayChart}
            data={chartDataTemperature[0].data}
            curve={shape.curveNatural}
            contentInset={contentInset}
            numberOfTicks={ticks}
            svg={chartDataTemperature[0].areaSvg}
          />
        </View>
        <YAxis
          style={{marginRight: 2, width:40}}
          data={this.dataHumidity}
          contentInset={contentInset}
          svg={{
            fill: COLOR_GRAPH_LINE_ORANGE,
            fontSize: 10,
          }}
          numberOfTicks={ticks}
          formatLabel={(value) => `${value}%`}
        />
      </View>
    );
  }
}

export default connect(mapStateToProps)(ChartView);