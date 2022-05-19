import { COLOR_INFO_LEVEL_1, COLOR_WARNING_LEVEL_2, COLOR_WHITE } from 'utils/constants/colors';

export default {
  wrapper: {
    display: 'flex',
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    backgroundColor: COLOR_WARNING_LEVEL_2,
    padding: '5px 5px',
    borderRadius: 8,
    marginBottom: 16,
  },
  wrapperEnvInfo: {
    display: 'flex',
    flexDirection: 'row',
    justifyContent: 'flex-start',
    alignItems: 'center',
    '& > div:first-child': {
      marginRight: 10,
    }
  },
  wrapperEnvItem: {
    width: 92,
    height: 40,
    backgroundColor: COLOR_WHITE,
    display: 'flex',
    flexDirection: 'row',
    justifyContent: 'flex-start',
    alignItems: 'center',
    borderRadius: 6,
    padding: `0 0 0 16px`,
    fontWeight: 600,
    fontSize: 18,
    cursor: 'default',
  },
  wrapperTime: {
    color: COLOR_INFO_LEVEL_1,
    fontSize: 16,
    fontWeight: 600,
    marginRight: 16,
  }
};
