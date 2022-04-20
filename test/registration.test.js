const { expect } = require('chai')
const binding = require('bindings')('registration.node')

describe('Registration', () => {
  it('register value', () => {
    expect(binding.num).to.eq(233)
    expect(binding.str).to.eq('hello world')
  })

  it('register function', () => {
    expect(binding.add(1, 2)).to.eq(3)
  })

  it('register template function', () => {
    expect(binding.addTpl(1, 2)).to.eq(3)
  })

  it('register custom object', () => {
    expect(binding.myObjectMethod({ str: 'hello' })).to.eql({
      str: 'hello world'
    })
    expect(binding.myObjectMethod({ str: 'hello', num: 42 })).to.eql({
      str: 'hello world',
      num: 43
    })
    expect(() => binding.myObjectMethod({})).to.throw(TypeError)
  })

  it('register class', () => {
    const calculator = new binding.Calculator(1)
    expect(calculator.num).to.eq(1)
    expect(calculator.add(2)).to.eq(3)
    expect(calculator.num).to.eq(3)
    expect(calculator.readonlyNum).to.eq(3)

    calculator.num = 42
    expect(calculator.num).to.eq(42)
    expect(calculator.readonlyNum).to.eq(42)
    calculator.readonlyNum = 233
    expect(calculator.num).to.eq(42)
    expect(calculator.readonlyNum).to.eq(42)
  })

  it('register class static', () => {
    const { Calculator } = binding

    Calculator.count = 42
    expect(Calculator.count).to.eq(42)
    expect(Calculator.readonlyCount).to.eq(42)
    expect(Calculator.readonlyCount).to.eq(42)

    Calculator.readonlyCount = 233
    expect(Calculator.count).to.eq(42)
    expect(Calculator.readonlyCount).to.eq(42)

    Calculator.add(5)
    expect(Calculator.count).to.eq(47)
    expect(Calculator.readonlyCount).to.eq(47)
  })

  it('return class instance', () => {
    const calculator = binding.Calculator.create(1)
    expect(calculator.num).to.eq(1)
    expect(calculator.add(2)).to.eq(3)
  })

  it('throws error for new in factory only class', () => {
    expect(() => new binding.FactorOnlyObject()).to.throw()
    expect(binding.FactorOnlyObject.create()).to.be.instanceOf(binding.FactorOnlyObject)
  })
})
