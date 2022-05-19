import React from 'react';
import renderer from 'react-test-renderer';

import Temperature from 'components/Temperature';

it('renders correctly when there is an empty value', () => {
  const component = renderer.create(
    <Temperature />,
  );
  let tree = component.toJSON();
  expect(tree).toMatchSnapshot();
});

it('renders correctly when there is 40 as value', () => {
  const component = renderer.create(
    <Temperature value={40} />,
  );
  let tree = component.toJSON();
  expect(tree).toMatchSnapshot();
});
