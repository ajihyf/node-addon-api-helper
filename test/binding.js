
const bindings = require('bindings')

exports.forEachBinding = cb => {
  describe('Exception', () => {
    cb(bindings('binding.node'))
  })

  describe('Exception with namespace', () => {
    cb(bindings('binding_namespace.node'))
  })

  describe('No Exception', () => {
    cb(bindings('binding_noexcept.node'))
  })
}
