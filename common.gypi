{
  'conditions': [
    ['OS=="mac"', {
      'cflags+': [ '-fvisibility=hidden' ],
      'xcode_settings': {
        'CLANG_CXX_LIBRARY': 'libc++',
        'MACOSX_DEPLOYMENT_TARGET': '10.13',
        'OTHER_CFLAGS': [ '-fvisibility=hidden', '-std=c++17' ]
      }
    }],
    ['OS=="win"', {
      'msbuild_settings': {
        'ClCompile': {
          'LanguageStandard': 'stdcpp17'
        }
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
