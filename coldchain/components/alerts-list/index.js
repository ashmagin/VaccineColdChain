import React, { Component } from 'react'  
import { connect } from 'react-redux';
import { Text, View, StyleSheet } from 'react-native';
import { TYPE_FALL, TYPE_TEMPERATURE } from '../../utils/constants/sensors';
import { COLOR_WHITE, COLOR_WARNING_LEVEL_2, COLOR_GRAY, COLOR_GRAY_BACKGROUND } from '../../utils/constants/colors';
import moment from 'moment';
import Timeline from 'react-native-timeline-flatlist';
import { IS_ANDROID } from '../../helpers';
import PushNotification, {Importance} from 'react-native-push-notification';

const styles = StyleSheet.create({
  rootContainer: {
    flex: 1,
    paddingHorizontal: 20,
    marginHorizontal: 15,
    marginBottom: 15,
    borderRadius: 8,
    backgroundColor: COLOR_GRAY_BACKGROUND,
    paddingVertical: 15,
  },
  captionText: {
    fontSize: 22,
    fontWeight: 'bold',
    color: COLOR_WARNING_LEVEL_2,
    width: '100%',
    textAlign: 'center',
    paddingVertical: 10,
  },
  alarmText: {
    color: COLOR_WHITE,
    fontSize: 14,
  },
  timestampText: {
    color: COLOR_WARNING_LEVEL_2,
    fontSize: 14,
  }
});

function mapStateToProps(state) {
  const sensorInfo = state.sensorInfo
  return { sensorInfo: sensorInfo };
}

class AlertsList extends Component {  
  constructor(props) {
    super(props);
    
    this.state = {
      sensorInfo: props.sensorInfo,
    };

    this.processAlerts();
  }
  
  componentWillReceiveProps(nextProps) {
    if (nextProps.sensorInfo != null) {
      this.setState({
        sensorInfo: nextProps.sensorInfo,
      });
    }
    this.data = new Array();
    this.processAlerts();
  }

  componentDidMount() {
    if (IS_ANDROID) {
      PushNotification.createChannel(
        {
          channelId: "fcm_fallback_notification_channel", // (required)
          channelName: "Alerts", // (required)
          channelDescription: "A channel to categorise your notifications", // (optional) default: undefined.
          playSound: true, // (optional) default: true
          soundName: "default", // (optional) See `soundName` parameter of `localNotification` function
          importance: Importance.HIGH, // (optional) default: Importance.HIGH. Int value of the Android notification importance
          vibrate: true, // (optional) default: true. Creates the default vibration pattern if true.
        },
        (created) => console.log(`createChannel returned '${created}'`) // (optional) callback returns whether the channel was created, false means it already existed.
      );
    }
  }

  processAlerts() {
    if (this.state.sensorInfo.alerts != null) {
      this.data = new Array();
      this.state.sensorInfo.alerts.forEach(element => {
        var alertText = "Unknown alarm"
        if (element.type === TYPE_TEMPERATURE) {
          alertText = "Temperature alarm";
        } else if (element.type === TYPE_FALL) {
          alertText = "Fall alarm"
        }
        var timestampText = moment(element.timestamp).format('hh:mm:ss a')

        this.data.unshift({time: timestampText, title: alertText});
      });
    } 
  }

  render() {  
    if (this.data.length == 0) {
      return null;
    }
    return (
      <View style={styles.rootContainer}>  
        <Timeline
          data={this.data}
          titleStyle={styles.alarmText}
          timeStyle={styles.timestampText}
          lineColor={COLOR_GRAY}
          circleColor={COLOR_GRAY}
        />
      </View>
    );  
  }  
}  

export default connect(mapStateToProps)(AlertsList);