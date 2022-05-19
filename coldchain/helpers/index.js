import {Platform} from 'react-native';
import {Dimensions} from 'react-native';

export const IS_ANDROID = Platform.OS === 'android';
export const IS_WEB_APP = Platform.OS === 'default';

export const SCREEN_WIDTH = Dimensions.get('window').width; //full width