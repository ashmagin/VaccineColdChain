import { COLOR_ALERT, COLOR_SUCCESS, COLOR_WHITE } from 'utils/constants/colors';

export default {
  wrapper: {
    textTransform: 'capitalize',
    color: COLOR_WHITE,
    fontSize: 14,
    fontWeight: 600,
    display: 'flex',
    flexDirection: 'row',
    justifyContent: 'flex-start',
    alignItems: 'center',
    letterSpacing: 1,
    '& > div': {
      fontSize: 24,
      position: 'relative',
      top: 2,
      marginRight: 8,
    },
  },
  wrapperOnline: {
    color: COLOR_SUCCESS,
  },
  wrapperOffline: {
    color: COLOR_ALERT,
  },
};
