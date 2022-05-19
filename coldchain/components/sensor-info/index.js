import React from 'react';
import { StyleSheet, View, Text, Image } from 'react-native';
import { connect } from 'react-redux';
import { COLOR_WHITE, COLOR_WARNING_LEVEL_2, COLOR_GRAY_BACKGROUND } from '../../utils/constants/colors';

const styles = StyleSheet.create({
  mainContainer: {
    flexDirection: 'row',
    justifyContent:'space-between',
    paddingHorizontal: 10,
    paddingVertical: 7,
    backgroundColor: COLOR_WARNING_LEVEL_2,
    marginHorizontal: 15,
    marginBottom: 15,
    borderRadius: 8,
    height: 45, 
  },
  sensorInfoText: {
    color: COLOR_GRAY_BACKGROUND,
    fontSize: 14,
    fontWeight: 'bold',
    paddingLeft: 7,
  },
  sensorDataWrapper: {
    borderRadius: 8,
    backgroundColor: COLOR_WHITE,
    flexDirection: 'row',
    paddingHorizontal: 10,
    paddingVertical: 5,
  },
  sensorIcon: {
    width: 20,
    height: 20
  },
});

function mapStateToProps(state) {
  const sensorInfo = state.sensorInfo
  console.log("redux: " + JSON.stringify(sensorInfo));
  return { sensorInfo: sensorInfo };
}

class SensorInfo extends React.Component {
  constructor(props) {
    super(props);
    
    this.state = {
      sensorInfo: props.sensorInfo,
      temperature: "",
      humidity: "",
      lastSeenText: "Never seen",
    };
  }

  componentWillReceiveProps(nextProps) {
    if (nextProps.sensorInfo != null) {
      this.setState({
        sensorInfo: nextProps.sensorInfo,
      });

      if (this.state.sensorInfo.temperature != null) {
        this.setState({temperature: `${this.state.sensorInfo.temperature}ºC`});
      }

      if (this.state.sensorInfo.humidity != null) {
        this.setState({humidity: `${this.state.sensorInfo.humidity}%`});
      }
    }
  }

  componentDidMount() {
    updateFunc = this.updateLastSeenTime.bind(this);
    setInterval( () => {
      updateFunc.call();
    }, 1000);
  }

  updateLastSeenTime() {
    if (this.state.sensorInfo.lastUpdatedTime == null) {
      return;
    }

    var secondsSinceLastUpdate = (Date.now() - this.state.sensorInfo.lastUpdatedTime) / 1000;
    if (secondsSinceLastUpdate < 20) {
      this.setState({lastSeenText: "Just now"});
    } else if (secondsSinceLastUpdate < 60) {
      this.setState({lastSeenText: "Less than a minute ago"});
    } else if (secondsSinceLastUpdate < 3600) {
      var minutesAgo = Math.floor(secondsSinceLastUpdate / 60);
      this.setState({lastSeenText: `${minutesAgo} minutes ago`});
    } else {
      var hoursAgo = Math.floor(secondsSinceLastUpdate / 3600);
      this.setState({lastSeenText: `${hoursAgo} hours ago`});
    }
  }

  render() {
    return (
      <View style={styles.mainContainer}>

        <View style={{flexDirection: 'row'}}>
        {!!this.state.temperature && <View style={[styles.sensorDataWrapper,  {marginRight: 10}]}>
            <Image source={require('../../assets/temperature.png')} style={styles.sensorIcon}/>
            <View style={{heigh: '100%', justifyContent: 'center'}}>
              <Text style={styles.sensorInfoText}>{this.state.temperature}</Text>
            </View>
          </View>}
          {!!this.state.humidity && <View style={styles.sensorDataWrapper}>
            <Image source={require('../../assets/humidity.png')} style={styles.sensorIcon}/>
            <View style={{heigh: '100%', justifyContent: 'center'}}>
              <Text style={styles.sensorInfoText}>{this.state.humidity}</Text>
            </View>
          </View>}
        </View>
        <View style={{heigh: '100%', justifyContent: 'center'}}>
          <Text style={styles.sensorInfoText}>{this.state.lastSeenText}</Text>
        </View>
      </View>
    );
  }
}

export default connect(mapStateToProps)(SensorInfo);