const path = require('path')

module.exports = {
  include_dir: path.relative('.', __dirname),
  gypi: path.relative('.', path.join(__dirname, 'napi_helper.gypi'))
}
