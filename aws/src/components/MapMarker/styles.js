import { COLOR_WARNING_LEVEL_1 } from 'utils/constants/colors';

export default {
  wrapper: {
    position: 'relative',
    width: 40,
    height: 40,
    display: 'flex',
    justifyContent: 'center',
    alignItems: 'center'
  },
  wrapperOuter: {
    backgroundColor: 'transparent',
    borderWidth: 7,
    borderColor: COLOR_WARNING_LEVEL_1,
    borderStyle: 'solid',
    position: 'absolute',
    top: 0,
    left: 0,
    right: 0,
    bottom: 0,
    borderRadius: 20,
  },
  wrapperInner: {
    width: 18,
    height: 18,
    borderRadius: 8,
    backgroundColor: COLOR_WARNING_LEVEL_1,
    position: 'absolute',
  },
};
