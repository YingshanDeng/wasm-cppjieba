#include <emscripten.h>

int main() {
    EM_ASM(
        // Make a directory other than '/'
        FS.mkdir('/offline');
        // Then mount with IDBFS type
        FS.mount(IDBFS, {}, '/offline');

        var url = "https://raw.githubusercontent.com/yanyiwu/cppjieba/master/dict/user.dict.utf8";
        var request = new XMLHttpRequest;
        request.open("GET", url, true);
        request.responseType = "arraybuffer";
        request.url = url;
        request.onreadystatechange = function() {
            if (request.readyState == 4) {
                if (request.status == 200) {
                    var arraybuf = request.response;
                    var len = arraybuf.byteLength;
                    var dataView = new DataView(arraybuf);
                    var data = new Uint8Array(len);
                    for (var i = 0; i < len; i++) {
                        data[i] = dataView.getUint8(i)
                    }
                    var stream= FS.open('/offline/user.dict.utf8', 'w+');
                    FS.write(stream, data, 0, data.length, 0);
                    FS.close(stream);

                    FS.syncfs(false, function (err) {
                        console.log(FS.readFile('/offline/user.dict.utf8', { encoding: 'utf8' }));
                    });
                } else {
                    console.error('Error: ', request.url, request.status)
                }
            }
        };
        request.send();
    );
}
