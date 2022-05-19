import { COLOR_GRAY_LEVEL_3, COLOR_WARNING_LEVEL_2 } from 'utils/constants/colors';

export default {
  wrapper: {
    display: 'flex',
    flexDirection: 'row',
    justifyContent: 'flex-start',
    alignItems: 'center',
    marginLeft: 4,
    cursor: 'pointer',
    color: COLOR_GRAY_LEVEL_3,
    '&:hover': {
      color: COLOR_WARNING_LEVEL_2,
    },
  },
  wrapperActive: {
    color: COLOR_WARNING_LEVEL_2,
    fontWeight: 'bold',
  },
};
