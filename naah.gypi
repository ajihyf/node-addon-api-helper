{
  'conditions': [
    ['OS=="mac"', {
      'xcode_settings': {
        'CLANG_CXX_LIBRARY': 'libc++',
        'MACOSX_DEPLOYMENT_TARGET': '10.13', # std::visit requires this
        'OTHER_CFLAGS': [ '-std=c++17' ]
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
    ".",
    "<!(node -p \"require('node-addon-api').include_dir\")"
  ],
  'cflags_cc': [ '-std=c++17' ]
}
