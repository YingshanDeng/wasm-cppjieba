// web worker

// Module['onRuntimeInitialized'] = function() {
//     console.log('onRuntimeInitialized');
// };

/**
 * worker 接收两种指令：
 *  1. 'init-jieba'
 *  2. 'cut-sentence'
 * 发送五种指令：
 *  1. 'dicts-check'
 *  2. 'jieba-ready'
 *  3. 'cut-finish'
 *  4. 'report'
 *  5. 'warn'
 */

function initJieba() {
    Module._initJiebaInstance();
}
function cutSentence(str) {
    var lengthBytes = lengthBytesUTF8(str)+1;
    var stringOnWasmHeap = Module._malloc(lengthBytes);
    stringToUTF8(str, stringOnWasmHeap, lengthBytes);

    Module._cutSentence(stringOnWasmHeap);
    Module._free(stringOnWasmHeap);
}

self.onmessage = function(e) {
    var cmd = e.data.cmd;
    switch (cmd) {
        case 'init-jieba':
            initJieba();
            break;
        case 'cut-sentence':
            var str = e.data.sentence;
            str && str.length && cutSentence(str);
            break;
        default:
            self.postMessage({
                cmd: 'warn',
                data: 'Unknown Command!',
            })
            break;
    }
};