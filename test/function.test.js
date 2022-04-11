const { expect } = require('chai');
const bindings = require('bindings')('binding.node');

describe('function', () => {
  it('value callback', () => {
    expect(bindings.function.valueCallback(1, 2, 'str')).to.equal(1 + 2 + 3);
  });

  it('throws for invalid arguments', () => {
    expect(() => bindings.function.valueCallback('str')).to.throw(
      'bad arguments'
    );
  });

  it('accept optional argument', () => {
    expect(bindings.function.valueCallback(1, 'hehe', 'str')).to.equal(
      1 + 42 + 3
    );
  });

  it('void callback', () => {
    expect(bindings.function.voidCallback(1, 2, 'str')).to.equal(undefined);
  });

  it('valueCallbackWithInfo', () => {
    expect(bindings.function.valueCallbackWithInfo(1, 2, 3, 4)).to.equal(1 + 4);
  });
});
