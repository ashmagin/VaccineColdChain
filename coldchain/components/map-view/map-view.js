import React from 'react';
import { StyleSheet, View } from 'react-native';
import { SCREEN_WIDTH } from '../../helpers';
import { MAPBOX_ACCESS_TOKEN, COGNITO_IDENTITY_POOL_ID } from '../../config/credentials';
import { BACKEND_REGION, AWS_MAP_NAME } from '../../config/backend';
import mapboxgl, { NavigationControl } from 'mapbox-gl';
import { getCredentialsForIdentityPool, createRequestTransformer } from 'amazon-location-helpers';

import 'mapbox-gl/dist/mapbox-gl.css';

mapboxgl.accessToken = MAPBOX_ACCESS_TOKEN;

const styles = StyleSheet.create({
  map_container: {
    height: 400,
    width: SCREEN_WIDTH,
  },
  map: {
    flex: 1
  }
});

class MapView extends React.Component {
  constructor(props) {
    super(props);
    
    this.state = {
      lng: -70.9,
      lat: 42.35,
      zoom: 9
    };
    this.mapContainer = React.createRef();
    this.transformRequest = null;
  }
  
  async componentDidMount() {
    var config = {
      credentials: await getCredentialsForIdentityPool(COGNITO_IDENTITY_POOL_ID),
      identityPoolId: COGNITO_IDENTITY_POOL_ID,
      region: BACKEND_REGION,
    };
    
    const { lng, lat, zoom } = this.state;
    this.transformRequest = await createRequestTransformer(config);
    const map = new mapboxgl.Map({
      container: this.mapContainer.current,
      style: AWS_MAP_NAME,
      center: [lng, lat],
      zoom: zoom,
      transformRequest: this.transformRequest,
    });
    map.addControl(new NavigationControl(), "top-left");
  }
  
  render() {
    return (
      <View style={styles.map_container} >
        <div ref={this.mapContainer} style={{height: '100%'}} className="map-container"></div>
      </View>
    );
  }
}
  
export default MapView;