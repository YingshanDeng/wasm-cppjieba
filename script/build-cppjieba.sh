DIR="build"
if [ ! -d "$DIR" ]; then
    ###  Create if $DIR does NOT exists ###
    echo "Create ${DIR} folder"
    mkdir ${DIR}
else
    echo "${DIR} folder exists"
fi

echo "Enter ${DIR} exec cmake .. && make"
cd build
emcmake cmake ..
emmake make
em++ ./CMakeFiles/wasm-cppjieba.dir/src/wasm-cppjieba.cpp.o \
     -o ../dist/index.html \
     -s FORCE_FILESYSTEM=1 \
     -lidbfs.js \
     --js-library ../src/wasm-cppjieba-library.js \
     -s EXPORTED_FUNCTIONS="['_main', '_initJiebaInstance', '_cutSentence']" \
     -s ALLOW_MEMORY_GROWTH=1
emrun --no_browser --port 8081 dist
