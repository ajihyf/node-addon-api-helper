{
    'includes': ['../naah.gypi'],
    'conditions': [
        ['OS=="mac"', {
            'cflags+': ['-fvisibility=hidden'],
            'xcode_settings': {
                'OTHER_CFLAGS': ['-fvisibility=hidden']
            }
        }]
    ],
    'cflags': ['-Wall', '-Wextra', '-Wpedantic', '-Wunused-parameter'],
    'cflags_cc': ['-Werror', '-Wall', '-Wextra', '-Wpedantic', '-Wunused-parameter']
}
