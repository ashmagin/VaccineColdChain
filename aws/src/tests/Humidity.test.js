import React from 'react';
import renderer from 'react-test-renderer';

import Humidity from 'components/Humidity';

it('renders correctly when there is an empty value', () => {
  const component = renderer.create(
    <Humidity />,
  );
  let tree = component.toJSON();
  expect(tree).toMatchSnapshot();
});

it('renders correctly when there is 40 as value', () => {
  const component = renderer.create(
    <Humidity value={40} />,
  );
  let tree = component.toJSON();
  expect(tree).toMatchSnapshot();
});
