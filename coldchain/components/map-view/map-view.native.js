import React from 'react';
import {StyleSheet, View, Text, SafeAreaView} from 'react-native';
import {IS_ANDROID, SCREEN_WIDTH} from '../../helpers';
import { COGNITO_IDENTITY_POOL_ID, MAPBOX_ACCESS_TOKEN } from '../../config/credentials';
import { BACKEND_REGION, AWS_MAP_NAME } from '../../config/backend';
import MapboxGL from '@react-native-mapbox-gl/maps'
import { connect } from 'react-redux';
import {NativeModules} from 'react-native';
import MapMarker from '../map-marker';
var MapboxSigner = NativeModules.MapboxSigner;

MapboxGL.setAccessToken(MAPBOX_ACCESS_TOKEN);

const styles = StyleSheet.create({
  map_container: {
    height: '30%',
    marginHorizontal: 15,
    marginBottom: 15,
    borderRadius: 8,
    overflow: 'hidden',
  },
  map: {
    flex: 1,
  }
});

function mapStateToProps(state) {
  const sensorInfo = state.sensorInfo
  return { sensorInfo: sensorInfo };
}

class MapView extends React.Component {
  constructor(props) {
    super(props);
    
    this.state = {
      isFetchingAndroidPermission: IS_ANDROID,
      isAndroidPermissionGranted: false,
      startingPoint: [-94.57276,39.09908],
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

    if (this.state.sensorInfo.location != null) {
      if (this.mapCamera != null) {
        this.mapCamera.moveTo(this.state.sensorInfo.location, 0);
      }
    } 
  }
  
  getMapStyleUrl(region, mapName) {
    return `https://maps.geo.${region}.amazonaws.com/maps/v0/maps/${mapName}/style-descriptor`;
  }

  componentDidMount() {
    if (IS_ANDROID) {
      MapboxGL.requestAndroidLocationPermissions().then(isGranted => {
        this.setState({
          isAndroidPermissionGranted: isGranted,
          isFetchingAndroidPermission: false,
        });
      });
    }
    
    if (MapboxSigner == null) {
      throw new Error("Cannot create request signer for Mapbox");
    }
    MapboxSigner.setupRequestSigner(COGNITO_IDENTITY_POOL_ID, (err) => {console.log(err)}, (msg) => {console.log(msg)} );
  }

  render() {
    if (IS_ANDROID && !this.state.isAndroidPermissionGranted) {
      if (this.state.isFetchingAndroidPermission) {
        return null;
      }
      return (
        <SafeAreaView
          style={[sheet.matchParent, {backgroundColor: colors.primary.blue}]}
          forceInset={{top: 'always'}}>
          <View style={sheet.matchParent}>
          <Text style={styles.noPermissionsText}>
            You need to accept location permissions in order to use this
            example applications
          </Text>
          </View>
        </SafeAreaView>
        );
    }
    if (this.state.sensorInfo == null) {
      return null;
    }
    return (
      <View style={styles.map_container}>
        <MapboxGL.MapView style={styles.map} styleURL={this.getMapStyleUrl(BACKEND_REGION, AWS_MAP_NAME)} compassViewPosition={0}>
          <MapboxGL.Camera
            zoomLevel={9}
            centerCoordinate={this.state.startingPoint}
            ref={c => (this.mapCamera = c)}
          />
            {this.state.sensorInfo.location != null &&
            <MapboxGL.MarkerView
              coordinate={this.state.sensorInfo.location}
              >
                <MapMarker/>
              </MapboxGL.MarkerView>}
            
        </MapboxGL.MapView>
      </View>
    );
  }
}
    
export default connect(mapStateToProps)(MapView);