const { expect } = require('chai')
const { forEachBinding } = require('./binding')

forEachBinding(({ multithread }) => {
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

    it('use async worker', done => {
      multithread.asyncWorker(5, () => {
        done()
      })
    })

    it('use promise worker', async () => {
      expect(await multithread.promiseWorker(5)).to.eq(5 + 42)
    })

    it('use promise worker with reject', (done) => {
      multithread.promiseWorkerWithReject(100).catch(err => {
        expect(err).to.be.instanceOf(RangeError)
        done()
      })
    })

    it('async arraybuffer', done => {
      multithread.asyncArrayBuffer(5, (buf) => {
        expect(buf).to.be.instanceOf(ArrayBuffer)
        expect(buf.byteLength).to.eq(5)
        done()
      })
    })

    it('async typed array', done => {
      multithread.asyncTypedArray(5, (arr) => {
        expect(arr).to.be.instanceOf(Uint16Array)
        expect(arr).to.eql(new Uint16Array([0, 1, 2, 3, 4]))
        done()
      })
    })
  })
})
