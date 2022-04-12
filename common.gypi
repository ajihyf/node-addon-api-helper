{
  'conditions': [
    ['OS=="mac"', {
      'cflags+': [ '-fvisibility=hidden' ],
      'xcode_settings': {
        'OTHER_CFLAGS': [ '-fvisibility=hidden' ]
      }
    }]
  ],
  'include_dirs': [
    "<!(node -p \"require('../').include_dir\")",
    "<!(node -p \"require('node-addon-api').include_dir\")"
  ],
  'cflags': [ '-Werror', '-Wall', '-Wextra', '-Wpedantic', '-Wunused-parameter' ],
  'cflags_cc': [ '-std=c++17', '-Werror', '-Wall', '-Wextra', '-Wpedantic', '-Wunused-parameter' ]
}
