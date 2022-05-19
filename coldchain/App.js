import React from 'react';
import { createStackNavigator, TransitionPresets } from 'react-navigation-stack';
import { createAppContainer } from 'react-navigation';
import { Provider } from 'react-redux';
import store from './utils/store';
import Main from './scenes/Main';
import { StatusBar } from 'react-native';
import { SafeAreaProvider } from 'react-native-safe-area-context';

const AppStackNavigator = createStackNavigator(
  {
    Home: {screen: Main},
  },
  {
    initialRouteName: 'Home',
    
    navigationOptions: {
      ...TransitionPresets.SlideFromRightIOS,
    },
    defaultNavigationOptions: {
      headerShown: false,
    },
  },
);

const AppContainer = createAppContainer(AppStackNavigator);
  
class App extends React.Component {    
  render() {
    return (
      <SafeAreaProvider>
        <Provider store={store}>
          <AppContainer/>
          <StatusBar hidden={false}></StatusBar>
        </Provider>
      </SafeAreaProvider>
    );
  }
}

export default App;