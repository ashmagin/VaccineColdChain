import { COLOR_GRAY_LEVEL_3, COLOR_WARNING_LEVEL_2, COLOR_WHITE } from 'utils/constants/colors';

export default {
  wrapper: {
    textAlign: 'left',
    fontSize: 16,
    marginLeft: -100,
    '& .MuiTimelineItem-oppositeContent .MuiTypography-colorTextSecondary': {
      color: COLOR_WARNING_LEVEL_2,
    },
    '& .MuiTimelineItem-content .MuiTypography-root': {
      color: COLOR_WHITE,
      textAlign: 'left',
    },
    '& .MuiTimelineDot-root, & .MuiTimelineConnector-root': {
      backgroundColor: COLOR_GRAY_LEVEL_3,
    }
  },
  noItems: {
    color: COLOR_WHITE,
    textAlign: 'center',
    fontSize: 16,
  },
}
