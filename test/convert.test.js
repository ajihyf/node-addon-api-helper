const { expect } = require('chai')
const { forEachBinding } = require('./binding')

forEachBinding(({ convert }) => {
  describe('Convert', () => {
    it('convert value', () => {
      expect(convert.num).to.eq(42)
      expect(convert.hehe).to.eq('hehe')
      expect(convert.stringView).to.eq('stringView')
      expect(convert.u16stringView).to.eq('u16stringView')
    })

    it('convert function', () => {
      expect(convert.fnLambda(2)(3)).to.eq(5)
      expect(convert.fnFunction(4)(4)).to.eq(8)
    })

    it('convert error', () => {
      expect(convert.errorFunction()).to.be.instanceOf(Error)
      expect(convert.rangeErrorFunction()).to.be.instanceOf(RangeError)
      expect(convert.typeErrorFunction()).to.be.instanceOf(TypeError)
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
})
