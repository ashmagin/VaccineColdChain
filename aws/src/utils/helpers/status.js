const isLive = (lastUpdatedTime) => {
  if (!lastUpdatedTime) return false;
  return ((Date.now() - lastUpdatedTime) / 1000) < 60 * 5; // If device was inactive for 5 minutes it means that it in offline state
};

export {
  isLive,
};
