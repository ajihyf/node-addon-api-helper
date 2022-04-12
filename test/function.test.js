const { expect } = require('chai');
const bindings = require('bindings')('binding.node');

const repeat = (i) => [i, i];

describe('function', () => {
  it('args callback', () => {
    expect(bindings.function.argsCallback(1, 2, 'str', true)).to.equal(
      1 + 2 + 3 + 1
    );
  });

  it('throws for invalid arguments', () => {
    expect(() => bindings.function.argsCallback('str')).to.throw(
      TypeError,
      'bad arguments'
    );
  });

  it('supports customize bad arguments message', () => {
    expect(() =>
      bindings.function.customBadArgumentsCallbackTpl('str')
    ).to.throw(TypeError, 'you bad bad');
    expect(() => bindings.function.customBadArgumentsCallback('str')).to.throw(
      TypeError,
      'you bad bad'
    );
  });

  it('accept optional argument', () => {
    expect(bindings.function.argsCallback(1, 'hehe', 'str', false)).to.equal(
      1 + 42 + 3
    );
  });

  it('calls valueCallbackWithInfo', () => {
    expect(bindings.function.valueCallbackWithInfo(1, 2, 3, 4)).to.equal(1 + 4);
  });

  [
    [true, 'vectorCallback', [1, 2, 3], [2, 3, 4]],
    [true, 'tupleCallback', ['233'], [3, '42'], ['42', 233], [2, '233']],

    ['valueCallback', { a: 3 }, '[object Object]'],
    ['boolCallback', true, false],
    ['numberCallback', 1, 2],
    [
      'bigIntCallback',
      BigInt(Number.MAX_SAFE_INTEGER),
      BigInt(Number.MAX_SAFE_INTEGER) + 233n,
    ],
    [true, 'dateCallback', new Date(42), new Date(42 + 233)],
    [true, 'objectCallback', { a: 'b' }, { a: 'b', key: 'value' }],
    [true, 'arrayCallback', [1, 2, 3], [233, 2, 3]],
    ['stringCallback', 'hello', 'hello world'],
    ['symbolCallback', Symbol.for('test'), Symbol.for('test')],
    ['arrayBufferCallback', ...repeat(new ArrayBuffer(8))],
    ['typedArrayCallback', ...repeat(new Float32Array(8))],
    ['functionCallback', ...repeat(() => {})],
    ['promiseCallback', ...repeat(Promise.resolve(233))],
    ['dataViewCallback', ...repeat(new DataView(new ArrayBuffer(8)))],
    ['externalCallback', ...repeat(bindings.function.external)],
    [true, 'bufferCallback', new Buffer([1, 2, 3]), new Buffer([233, 2, 3])],

    [
      true,
      'int8ArrayCallback',
      new Int8Array([1, 2, 3]),
      new Int8Array([42, 2, 3]),
    ],
    [
      true,
      'uint8ArrayCallback',
      new Uint8Array([1, 2, 3]),
      new Uint8Array([42, 2, 3]),
    ],
    [
      true,
      'int16ArrayCallback',
      new Int16Array([1, 2, 3]),
      new Int16Array([42, 2, 3]),
    ],
    [
      true,
      'uint16ArrayCallback',
      new Uint16Array([1, 2, 3]),
      new Uint16Array([42, 2, 3]),
    ],
    [
      true,
      'int32ArrayCallback',
      new Int32Array([1, 2, 3]),
      new Int32Array([42, 2, 3]),
    ],
    [
      true,
      'uint32ArrayCallback',
      new Uint32Array([1, 2, 3]),
      new Uint32Array([42, 2, 3]),
    ],
    [
      true,
      'float32ArrayCallback',
      new Float32Array([1, 2, 3]),
      new Float32Array([42.233, 2, 3]),
    ],
    [
      true,
      'float64ArrayCallback',
      new Float64Array([1, 2, 3]),
      new Float64Array([42.233, 2, 3]),
    ],
    [
      true,
      'bigInt64ArrayCallback',
      new BigInt64Array([1n, 2n, 3n]),
      new BigInt64Array([42n, 2n, 3n]),
    ],
    [
      true,
      'bigUint64ArrayCallback',
      new BigUint64Array([1n, 2n, 3n]),
      new BigUint64Array([42n, 2n, 3n]),
    ],

    ['functionWithVariants', '42', 2, 233, '233'],
    ['voidCallback', 1, undefined],
    ['lambda', 1, 1 + 42],
    ['lambdaWithVoid', 1, undefined],
    ['lambdaWithCapture', 1, 1 + 233],
  ].forEach((args) => {
    const headIsBool = typeof args[0] === 'boolean';
    const deepEql = headIsBool ? args[0] : false;
    const fn = headIsBool ? args[1] : args[0];
    it('calls ' + fn, () => {
      for (let i = headIsBool ? 2 : 1; i < args.length; i += 2) {
        expect(bindings.function[fn](args[i])).to[deepEql ? 'eql' : 'eq'](
          args[i + 1]
        );
      }
    });
  });

  it('calls function throws', () => {
    expect(bindings.function.functionThrows(11)).to.equal('11');
    expect(() => bindings.function.functionThrows(64)).to.throw(
      RangeError,
      'bigger than 42'
    );
  });

  it('calls function throws manually', () => {
    expect(bindings.function.functionThrowsManually(42)).to.equal('42');
    expect(() => bindings.function.functionThrowsManually(256)).to.throw(
      RangeError,
      'bigger than 233'
    );
  });
});
