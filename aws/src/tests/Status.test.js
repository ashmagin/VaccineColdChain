import { isLive } from 'utils/helpers/status';
import { getDiffUpdate } from 'utils/helpers/time';

it('isLive check should return false if device\'s lastUpdatedTime was no more than 5 minutes ago', () => {
  const sixMinutesAgo = Date.now() - 6 * 60 * 1000;
  
  expect(isLive(sixMinutesAgo)).toBeFalsy();
});

it('isLive check should return true if device\'s lastUpdatedTime was no less than 5 minutes ago', () => {
  const fourMinutesAgo = Date.now() - 4 * 60 * 1000;
  
  expect(isLive(fourMinutesAgo)).toBeTruthy();
});

it('should return 2 minutes ago', () => {
  const twoMinutesAgo = Date.now() - 2 * 60 * 1000;
  
  expect(getDiffUpdate(twoMinutesAgo)).toBe('2 minutes ago');
});

it('should return 5 minutes ago', () => {
  const fiveMinutesAgo = Date.now() - 5 * 60 * 1000;
  
  expect(getDiffUpdate(fiveMinutesAgo)).toBe('5 minutes ago');
});
