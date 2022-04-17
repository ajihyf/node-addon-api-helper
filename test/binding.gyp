{
  'targets': [
    {
      'target_name': 'binding',
      'includes': ['./common.gypi', './except.gypi'],
      'sources': ['multithread.cc', 'convert.cc', 'object_wrap.cc', 'function.cc', 'binding.cc']
    },
    {
      'target_name': 'registration',
      'includes': ['./common.gypi', './except.gypi'],
      'sources': ['registration.cc']
    },
  ]
}
