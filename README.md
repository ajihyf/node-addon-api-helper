# node-addon-api-helper<!-- omit in toc -->

[![NPM](https://img.shields.io/npm/v/node-addon-api-helper)](https://www.npmjs.com/package/node-addon-api-helper)
[![CI](https://github.com/ajihyf/node-addon-api-helper/workflows/ci/badge.svg)](https://github.com/ajihyf/node-addon-api-helper/actions/workflows/ci.yml)
[![lint](https://github.com/ajihyf/node-addon-api-helper/workflows/lint/badge.svg)](https://github.com/ajihyf/node-addon-api-helper/actions/workflows/lint.yml)
![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
[![LICENSE](https://img.shields.io/github/license/ajihyf/node-addon-api-helper)](https://github.com/ajihyf/node-addon-api-helper/blob/main/LICENSE)

![Windows Support](https://img.shields.io/static/v1?label=Windows&logo=windows&logoColor=white&message=support&color=success)
![Linux Support](https://img.shields.io/static/v1?label=Linux&logo=linux&logoColor=white&message=support&color=success)
![Mac Support](https://img.shields.io/static/v1?label=macOS&logo=apple&logoColor=white&message=support&color=success)

Based on [node-addon-api](https://github.com/nodejs/node-addon-api), **node-addon-api-helper(naah)** provides a more convenient, type-safe and boilerplate-less way to write Node.js C++ addons.

Features:

- Automatically transform values between JavaScript and C++
- Automatically register exports
- Class binding (with inheritance)
- Custom object type
- Thread safe function
- Create async work and return promise
- Can be used together with original node-addon-api, no need to rewrite all
- Pure header, can be easily integrated

## QuickStart

```bash
npm install node-addon-api node-addon-api-helper
```

Add following content to your GYP file.

```gyp
{
  'include_dirs': [
    "<!(node -p \"require('node-addon-api').include_dir\")",
    "<!(node -p \"require('node-addon-api-helper').include_dir\")",
  ]
}
```

**naah** heavily depends on **C++17**, if you haven't configured your project on C++17, have a look at [naah.gypi](./naah.gypi) as an example.

A hello world example :

```cpp
#include <naah.h>

uint32_t add(uint32_t a, uint32_t b) {
  return a + b;
}

NAAH_REGISTRATION {
  naah::Registration::Function<add>("add");
}

NAAH_EXPORT
```

## Documentation

- [Exports](./doc/exports.md)
- [Function](./doc/function.md)
- [Object](./doc/object.md)
- [Class](./doc/class.md)
- [Mix with node-addon-api](./doc/mix_with_napi.md)
- [Thread Safe Function](./doc/thread_safe_function.md)
- [Async Work](./doc/async_work.md)
- [Error Handling](./doc/error_handling.md)

## Examples

See `naah` directories in the [fork of node-addon-examples](https://github.com/ajihyf/node-addon-examples) for examples.

## Thanks

- Based on [node-addon-api](https://github.com/nodejs/node-addon-api).
- Inspired by [napi-rs](https://napi.rs), [emscripten](https://emscripten.org/) and [nbind](https://github.com/charto/nbind).
