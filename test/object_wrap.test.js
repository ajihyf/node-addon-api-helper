const { expect } = require('chai')
const { scriptWrappable } = require('bindings')('binding.node')

describe('ObjectWrap', () => {
  it('construct object', () => {
    expect(new scriptWrappable.TestObject(233)).to.be.an.instanceOf(scriptWrappable.TestObject)

    expect(() => new scriptWrappable.TestObject('233')).to.throw(TypeError, 'bad arguments')
  })

  it('calls instance method', () => {
    const obj = new scriptWrappable.TestObject(233)

    expect(obj.multiply(42)).to.eq(233 * 42)
  })

  it('calls method with callback info input', () => {
    const obj = new scriptWrappable.TestObject(233)

    expect(obj.getArgLength()).to.eq(0)
    expect(obj.getArgLength(0, 1, 2)).to.eq(3)
  })

  it('calls instance accessor', () => {
    const obj = new scriptWrappable.TestObject(233)
    expect(obj.num).to.eq(233)
    obj.num = 42
    expect(obj.num).to.eq(42)
    obj.add(42)
    expect(obj.num).to.eq(84)
  })

  it('calls symbol method', () => {
    const sym = scriptWrappable.TestObject.sym
    expect(sym).to.be.a('symbol')
    const obj = new scriptWrappable.TestObject(233)
    expect(obj[sym]('hello')).to.eq('hello??sym')
  })

  it('calls static accessor', () => {
    scriptWrappable.TestObject.count = 0
    expect(scriptWrappable.TestObject.count).to.eq(0)

    scriptWrappable.TestObject.count = 233
    expect(scriptWrappable.TestObject.count).to.eq(233)
  })

  it('calls static method', () => {
    scriptWrappable.TestObject.count = 0

    scriptWrappable.TestObject.add(42)
    expect(scriptWrappable.TestObject.count).to.eq(42)

    scriptWrappable.TestObject.add(233)
    expect(scriptWrappable.TestObject.count).to.eq(42 + 233)
  })

  it('calls method with ObjectWrap input', () => {
    const o1 = new scriptWrappable.TestObject(233)
    const o2 = new scriptWrappable.AnotherTestObject()
    const o3 = new scriptWrappable.AnotherTestObject()
    expect(() => o2.addTest({})).to.throw(TypeError)
    expect(() => o2.addTest(o3)).to.throw(TypeError)
    o2.addTest(o1, 42)
    expect(o2.getTestNum(o1)).to.eq(233 + 42)
  })

  it('can not set readonly field', () => {
    const o1 = new scriptWrappable.TestObject(233)
    expect(o1.readonlyNum).to.eq(233)
    o1.readonlyNum = 42
    expect(o1.readonlyNum).to.eq(233)

    scriptWrappable.TestObject.count = 42
    expect(scriptWrappable.TestObject.readonlyCount).to.eq(42)
    scriptWrappable.TestObject.readonlyCount = 233
    expect(scriptWrappable.TestObject.readonlyCount).to.eq(42)
  })
})
