
const bindings = require('bindings')

exports.forEachBinding = cb => {
  describe('Exception', () => {
    cb(bindings('binding.node'))
  })

  describe('No Exception', () => {
    cb(bindings('binding_noexcept.node'))
  })
}
