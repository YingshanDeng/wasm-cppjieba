emcc src/dicts-idbfs.c -o dist/index.html -s FORCE_FILESYSTEM=1 -lidbfs.js
emrun --no_browser --port 8081 dist