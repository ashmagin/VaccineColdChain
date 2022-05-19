import { COLOR_INFO_LEVEL_1, COLOR_WHITE } from 'utils/constants/colors';

export const styleAxis = {
  fill: COLOR_WHITE,
  fontSize: 10,
};

export const styleArea = {
  top: 15,
  right: 20,
  left: 0,
  bottom: 15,
};

export default {
  wrapper: {
    height: '50%',
    width: '100%',
    position: 'relative',
    backgroundColor: COLOR_INFO_LEVEL_1,
    borderRadius: 5,
    marginBottom: 0,
    '& .recharts-responsive-container': {
      height: 'calc(100% - 82px) !important',
    }
  },
};
