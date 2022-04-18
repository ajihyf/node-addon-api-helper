const { expect } = require('chai')
const { multithread } = require('bindings')('binding.node')

describe('multithread', () => {
  it('calls function from other threads', (done) => {
    const arr = []
    let times = 0
    multithread.asyncNotify(3, i => {
      times++
      arr.push(i)
      if (times === 3) {
        expect(arr.sort()).to.eql([0, 1, 2])
        done()
      }
    })
  })

  it('handle errors', done => {
    multithread.asyncMinus3(5, (err, ret) => {
      expect(err).to.be.eq(undefined)
      expect(ret).to.eq(2)
      multithread.asyncMinus3(1, (err, ret) => {
        expect(err).to.be.instanceOf(RangeError)
        expect(ret).to.be.eq(undefined)
        done()
      })
    })
  })
})