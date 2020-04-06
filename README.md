# wasm-cppjieba
Build cppjieba to webassembly and work on browser ✂️

![demo](https://raw.githubusercontent.com/YingshanDeng/wasm-cppjieba/master/demo/example.png?token=ABXJGA2VUU6YOQ7FU3RERTK6RMPDI)

## Dependencies
Download and install emscripten [Guide](https://emscripten.org/docs/getting_started/downloads.html)

```shell
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk

# Download and install the latest SDK tools.
./emsdk install latest
# Make the "latest" SDK "active" for the current user.
./emsdk activate latest

# Activate PATH and other environment variables in the current terminal
source ./emsdk_env.sh
```

## Download & Compile

```shell
git clone --recurse-submodules git@github.com:YingshanDeng/wasm-cppjieba.git

npm run build
```

open this link in the browser: [http://localhost:8081/demo/index.html](http://localhost:8081/demo/index.html)

## Use Cases
[CodeMirror × wasm-cppjieba](https://github.com/YingshanDeng/CodeMirror/blob/master/README.md)
