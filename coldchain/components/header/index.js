import React from 'react';
import { StyleSheet, View, Text } from 'react-native';
import { connect } from 'react-redux';
import { COLOR_WHITE, COLOR_GRAY_BACKGROUND, COLOR_CONNECTED_BADGE } from '../../utils/constants/colors';
import { isLive } from '../../helpers/status';
import { DEMO_THING_NAME } from '../../config/backend';

const styles = StyleSheet.create({
  mainContainer: {
    flexDirection: 'row',
    justifyContent:'space-between',
    marginHorizontal: 15,
    marginVertical: 15,
  },
  deviceNameText: {
    fontSize: 18,
    fontWeight: 'bold',
    color: COLOR_WHITE,
  },
  connectionStatusText: {
    fontSize: 14,
    fontWeight: 'normal',
    color: COLOR_WHITE,
  },
  connectedBadge: {
    backgroundColor: COLOR_CONNECTED_BADGE,
    width: 16,
    height: 16,
    borderRadius: 8,
    justifyContent: 'center',
    alignItems: 'center',
    marginRight: 8,    
  },
  connectedBadgeText: {
    fontWeight: 'bold',
    color: COLOR_GRAY_BACKGROUND,
    fontSize: 14,
  }
});

function mapStateToProps(state) {
  const sensorInfo = state.sensorInfo
  console.log("redux: " + JSON.stringify(sensorInfo));
  return { sensorInfo: sensorInfo };
}

const offlineText = "OFFLINE";
const onlineText = "ONLINE";

class Header extends React.Component {
  constructor(props) {
    super(props);
    
    this.state = {
      sensorInfo: props.sensorInfo,
      connStatusText: offlineText,
      connected: false,
    };
  }

  componentWillReceiveProps(nextProps) {
    if (nextProps.sensorInfo != null) {
      this.setState({
        sensorInfo: nextProps.sensorInfo,
      });
    }

    this.updateConnStatusText();
  }

  componentDidMount() {
    updateFunc = this.updateConnStatusText.bind(this);
    setInterval( () => {
      updateFunc.call();
    }, 1000);
  }

  updateConnStatusText() {
    if (this.state.sensorInfo.lastUpdatedTime != null) {
      var connected = isLive(this.state.sensorInfo.lastUpdatedTime);
      if (connected) {
        this.setState({connStatusText: onlineText});
        this.setState({connected: true});
      } else {
        this.setState({connStatusText: offlineText});
        this.setState({connected: false});
      }
    } 
  }

  render() {
    return (
      <View style={styles.mainContainer}>
        <Text style={styles.deviceNameText}>{DEMO_THING_NAME}</Text>
        <View style={{flexDirection: 'row'}}>
          {this.state.connected && 
            <View style={styles.connectedBadge}>
              <Text style={styles.connectedBadgeText}>{"\u2713"}</Text>
            </View>  
          }
          <Text style={styles.connectionStatusText}>
            {this.state.connStatusText}
          </Text>
        </View>
      </View>
    );
  };
}

export default connect(mapStateToProps)(Header);