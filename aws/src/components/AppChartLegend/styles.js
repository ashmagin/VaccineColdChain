import { COLOR_INFO_LEVEL_3 } from 'utils/constants/colors';

export default {
  wrapper: {
    display: 'flex',
    flexDirection: 'row',
    justifyContent: 'center',
    alignItems: 'center',
    '& > div': {
      borderRadius: 4,
      textAlign: 'center',
      fontSize: 14,
      backgroundColor: COLOR_INFO_LEVEL_3,
      padding: '2px 17px',
      fontWeight: 600
    },
    '& div:first-child': {
      marginRight: 12,
    },
  },
};
