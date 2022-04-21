{
    'target_defaults': {
        'variables': {
            'binding_sources': [
                'multithread.cc',
                'convert.cc',
                'object_wrap.cc',
                'function.cc',
                'binding.cc'
            ],
            'registration_sources': [
                'registration.cc'
            ]
        }
    },
    'targets': [
        {
            'target_name': 'binding',
            'includes': ['./common.gypi', './except.gypi'],
            'sources': ['>@(binding_sources)']
        },
        {
            'target_name': 'binding_noexcept',
            'includes': ['./common.gypi', './noexcept.gypi'],
            'sources': ['>@(binding_sources)']
        },
        {
            'target_name': 'registration',
            'includes': ['./common.gypi', './except.gypi'],
            'sources': ['>@(registration_sources)']
        },
        {
            'target_name': 'registration_noexcept',
            'includes': ['./common.gypi', './noexcept.gypi'],
            'sources': ['>@(registration_sources)']
        }
    ]
}
