import { COLOR_INFO_LEVEL_4, COLOR_WHITE } from 'utils/constants/colors';

export default {
  wrapper: {
    display: 'flex',
    flexDirection: 'row',
    justifyContent: 'flex-start',
    alignItems: 'center',
    color: COLOR_WHITE,
    marginLeft: 15,
    paddingTop: 10,
    paddingBottom: 10,
  },
  wrapperItemsSup: {
    overflow: 'hidden',
  },
  wrapperItems: {
    display: 'flex',
    flexDirection: 'row',
    justifyContent: 'flex-start',
    alignItems: 'center',
    marginLeft: -2,
    '& > div': {
      flexGrow: 1,
      flexBasis: 'auto',
      margin: '.25em 0',
      padding: '0 1em',
      textAlign: 'center',
      borderLeft: `2px solid ${COLOR_INFO_LEVEL_4}`,
      fontSize: 14,
    }
  },
  label: {
    color: COLOR_WHITE,
    fontSize: 14,
  },
};
