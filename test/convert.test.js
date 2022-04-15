const { expect } = require('chai')
const { convert } = require('bindings')('binding.node')

describe('Convert', () => {
  it('convert value', () => {
    expect(convert.num).to.eq(42)
    expect(convert.hehe).to.eq('hehe')
    expect(convert.stringView).to.eq('stringView')
    expect(convert.u16stringView).to.eq('u16stringView')
  })

  it('convert function', () => {
    expect(convert.callbackMethod(1)(1)).to.eq(2)
    expect(convert.callbackMethod(2)(2)).to.eq(4)
  })

  it('convert custom value', () => {
    expect(convert.customMethod({ str: 'hello' })).to.eql({
      str: 'hello world'
    })
    expect(convert.customMethod({ str: 'hello', num: 42 })).to.eql({
      str: 'hello world',
      num: 43
    })
    expect(() => convert.customMethod({})).to.throw(TypeError)
  })
})
