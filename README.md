# node-addon-api-helper<!-- omit in toc -->

[![NPM](https://img.shields.io/npm/v/node-addon-api-helper)](https://www.npmjs.com/package/node-addon-api-helper)
[![CI](https://github.com/ajihyf/node-addon-api-helper/workflows/ci/badge.svg)](https://github.com/ajihyf/node-addon-api-helper/actions/workflows/ci.yml)
[![lint](https://github.com/ajihyf/node-addon-api-helper/workflows/lint/badge.svg)](https://github.com/ajihyf/node-addon-api-helper/actions/workflows/lint.yml)
[![LICENSE](https://img.shields.io/github/license/ajihyf/node-addon-api-helper)](https://github.com/ajihyf/node-addon-api-helper/blob/main/LICENSE)

Based on [node-addon-api](https://github.com/nodejs/node-addon-api), **node-addon-api-helper** provides a more convenient, type-safe and boilerplate-less way to write Node.js C++ addons.

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

**node-addon-api-helper** heavily depends on **C++17**, if you haven't configured your project on C++17, have a look at [napi_helper.gypi](./napi_helper.gypi) as an example.

A hello world example :

```cpp
#include <napi_helper.h>

uint32_t add(uint32_t a, uint32_t b) {
  return a + b;
}

NAPI_HELPER_REGISTRATION {
  NapiHelper::Registration::Function<add>("add");
}

NAPI_HELPER_EXPORT
```

## Documentation

- [Exports](./doc/exports.md)
- [Function](./doc/function.md)
- [Class](./doc/class.md)
- [Mix with node-addon-api](./doc/mix_with_napi.md)
- [Error Handling](./doc/error_handling.md)

## Thanks

- Based on [node-addon-api](https://github.com/nodejs/node-addon-api).
- Inspired by [napi-rs](https://napi.rs), [emscripten](https://emscripten.org/) and [nbind](https://github.com/charto/nbind).
