BUILD_DIR="build"
if [ ! -d "$BUILD_DIR" ]; then
    mkdir ${BUILD_DIR}
fi

DIST_DIR="dist"
if [ ! -d "$DIST_DIR" ]; then
    mkdir ${DIST_DIR}
fi

cd build
emcmake cmake ..
emmake make

em++ -O2 ./CMakeFiles/wasm-cppjieba.dir/src/wasm-cppjieba.cpp.o \
     -o ../dist/index.js \
     -s FORCE_FILESYSTEM=1 \
     -lidbfs.js \
     --js-library ../src/wasm-cppjieba-library.js \
     --pre-js ../src/wasm-cppjieba-pre.js \
     -s EXPORTED_FUNCTIONS="['_main', '_checkDict', '_initJiebaInstance', '_cutSentence']" \
     -s ALLOW_MEMORY_GROWTH=1

emrun --no_browser --port 8081 ..
