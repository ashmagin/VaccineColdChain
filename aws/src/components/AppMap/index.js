import React, { useEffect, useRef, useState } from 'react';
import { useSelector } from 'react-redux';
import ReactDOM from 'react-dom';

import { createUseStyles } from 'react-jss';
import 'maplibre-gl/dist/maplibre-gl.css';
import maplibregl from 'maplibre-gl';

import MapMarker from 'components/MapMarker';
import { credentials, transformRequest, refreshCognito } from 'utils/helpers/aws';
import { LAT, LON, ZOOM } from 'utils/constants/map';


import styles from './styles';

const useStyles = createUseStyles(styles);

const AppMap = () => {
  const classes = useStyles();
  const lon = useSelector((state) => state.sensorInfo.location[0]);
  const lat = useSelector((state) => state.sensorInfo.location[1]);
  const mapContainer = useRef(null);
  const map = useRef(null);
  const [ marker, setMarker ] = useState();
  
  useEffect(() => {
    async function initializeMap() {
      const el = document.createElement('div');

      if (!credentials) {
        await refreshCognito()
      }
      
      ReactDOM.render(
        <MapMarker />,
        el,
      );
      

      map.current = new maplibregl.Map({
        container: mapContainer.current,
        center: [ LON, LAT ],
        zoom: ZOOM,
        style: process.env.REACT_APP_MAP_NAME,
        transformRequest: transformRequest,
      });
      
      map.current.on('load', async () => {
        map.current.setPitch(25);
        map.current.resize();
        const marker = new maplibregl.Marker(el)
          .setLngLat([ LON, LAT ])
          .addTo(map.current);
        
        setMarker(marker);
      });
    }

    initializeMap();
  }, []);

  useEffect(() => {
    if (!map.current || !marker) return;
    
    marker.setLngLat([ lon, lat ]);
    map.current.panTo([ lon, lat ]);
  }, [ lat, lon ]);
  
  return (
    <div className={classes.wrapper}>
      <div
        ref={mapContainer}
        className={classes.map}
      />
    </div>
  )
};

export default AppMap;
