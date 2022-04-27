const { expect } = require('chai')
const bindings = require('bindings')

const cb = (binding) => {
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

    it('support class inheritance', () => {
      const a = new binding.SubA(42)
      const b = new binding.SubB(233)
      expect(a).to.be.instanceOf(binding.SubA)
      expect(b).to.be.instanceOf(binding.SubB)

      expect(a).to.be.instanceOf(binding.Base)
      expect(a).to.be.instanceOf(binding.SubA)
      expect(a).not.to.be.instanceOf(binding.SubB)
      expect(b).to.be.instanceOf(binding.Base)
      expect(b).not.to.be.instanceOf(binding.SubA)

      expect(a.num).to.eq(42)
      expect(b.num).to.eq(233)
      a.add(1)
      b.add(1)
      expect(a.num).to.eq(43)
      expect(b.num).to.eq(234)

      a.sub(1)
      expect(a.num).to.eq(42)
      b.mul(2)
      expect(b.num).to.eq(468)

      expect(a.getReal()).to.eq('A')
      expect(b.getReal()).to.eq('B')
      expect(binding.Base.getReal(a)).to.eq('A')
      expect(binding.Base.getReal(b)).to.eq('B')
      expect(binding.SubA.getReal(a)).to.eq('A')
      expect(binding.SubB.getReal(b)).to.eq('B')
      expect(binding.SubB.getReal(a)).to.eq('A')
      expect(binding.SubA.getReal(b)).to.eq('B')
      expect(() => binding.Base.getReal({})).to.throw(TypeError)
      expect(binding.SubA.acceptA(a)).to.eq(42)
      expect(() => binding.SubA.acceptA(b)).to.throw(TypeError)
      expect(binding.SubB.acceptB(b)).to.eq(468)
      expect(() => binding.SubB.acceptB(a)).to.throw(TypeError)
    })
  })
}

describe('Exception', () => {
  cb(bindings('registration.node'))
})

describe('No Exception', () => {
  cb(bindings('registration_noexcept.node'))
})
