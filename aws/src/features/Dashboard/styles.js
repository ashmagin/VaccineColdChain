import { COLOR_INFO_LEVEL_1, COLOR_WHITE } from 'utils/constants/colors';

export default {
  wrapper: {
    height: 'calc(100vh - 10px)',
  },
  wrapperUIData: {
    height: 'calc(100% - 140px)',
  },
  wrapperItems: {
    display: 'flex',
    flexDirection: 'row',
    justifyContent: 'flex-start',
    alignItems: 'center',
    height: '50%',
    width: '100%',
    marginBottom: 16,
    '& > div': {
      height: '100%',
    },
    '& > div:first-child': {
      width: '408px',
    },
    '& > div:last-child': {
      width: 'calc(100% - 408px)',
    },
  },
  wrapperLog: {
    color: COLOR_WHITE,
    fontSize: 16,
    borderRadius: 8,
    marginRight: 16,
    backgroundColor: COLOR_INFO_LEVEL_1,
  },
};
