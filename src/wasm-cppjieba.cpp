#include <emscripten.h>
#include "cppjieba/Jieba.hpp"

using namespace std;

const char* const DICT_PATH = "offline/jieba.dict.utf8";
const char* const HMM_PATH = "offline/hmm_model.utf8";
const char* const USER_DICT_PATH = "offline/user.dict.utf8";
const char* const IDF_PATH = "offline/idf.utf8";
const char* const STOP_WORD_PATH = "offline/stop_words.utf8";

extern "C" {
    extern void afterCheckJiebaDictsCallback(const char* type);
    extern void afterInitJiebaCallback(void);
    extern void afterCutSentenceCallback(uint8_t* data, int dataLength);
    extern void report(const char* type, double time);

    void checkDict(const char* type) {
        afterCheckJiebaDictsCallback(type);
    }

    cppjieba::Jieba* jieba;

    void initJiebaInstance() {
        double start = emscripten_get_now();
        jieba = new cppjieba::Jieba(
            DICT_PATH,
            HMM_PATH,
            USER_DICT_PATH,
            IDF_PATH,
            STOP_WORD_PATH
        );
        double end = emscripten_get_now();
        afterInitJiebaCallback();
        report("init-jieba", end-start);
    }

    // http://www.zedwood.com/article/cpp-utf8-strlen-function
    int utf8Strlen(const string& str) {
        int c,i,ix,q;
        for (q=0, i=0, ix=str.length(); i < ix; i++, q++)
        {
            c = (unsigned char) str[i];
            if      (c>=0   && c<=127) i+=0;
            else if ((c & 0xE0) == 0xC0) i+=1;
            else if ((c & 0xF0) == 0xE0) i+=2;
            else if ((c & 0xF8) == 0xF0) i+=3;
            //else if (($c & 0xFC) == 0xF8) i+=4; // 111110bb //byte 5, unnecessary in 4 byte UTF-8
            //else if (($c & 0xFE) == 0xFC) i+=5; // 1111110b //byte 6, unnecessary in 4 byte UTF-8
            else return 0;//invalid utf8
        }
        return q;
    }

    void cutSentence(char* s) {
        double start = emscripten_get_now();

        vector<string> words;
        // cout << s << endl;
        // cout << "[demo] Cut With HMM" << endl;

        jieba->Cut(s, words, true);

        vector<string>::iterator it;
        int length = words.size();
        int index = 0;
        uint8_t* resArr = new uint8_t[length];
        memset(resArr, 0, length);
        for (it = words.begin(); it != words.end(); it++) {
            resArr[index++] = utf8Strlen(*it);
            // cout << *it << endl;
        }

        afterCutSentenceCallback(resArr, length);

        double end = emscripten_get_now();
        report("cut-sentence", end-start);
    }
}

// 挂载 DB
EM_JS(void, mountDB, (const char* db), {
    var DB_NAME = UTF8ToString(db);
    FS.mkdir(DB_NAME);
    FS.mount(IDBFS, {}, DB_NAME);
});

int main() {

    mountDB("/offline");

    EM_ASM(
        function xhrLoadDict(baseURL, dict) {
            var url = baseURL + dict;
            return new Promise(function(resolve, reject) {
                var request = new XMLHttpRequest;
                request.open("GET", url, true);
                request.responseType = "arraybuffer";
                request.url = url;
                request.onreadystatechange = function() {
                    if (request.readyState == 4) {
                        if (request.status == 200) {
                            resolve(request.response);
                        } else {
                            reject('XHR LOAD FAIL: ' + request.status + ' : ' + request.url);
                        }
                    }
                };
                request.send();
            });
        }

        function writeDictToDB(arraybuf, dictPath) {
            return new Promise(function(resolve, reject) {
                var len = arraybuf.byteLength;
                var dataView = new DataView(arraybuf);
                var data = new Uint8Array(len);
                for (var i = 0; i < len; i++) {
                    data[i] = dataView.getUint8(i);
                }
                var stream= FS.open(dictPath, 'w+');
                FS.write(stream, data, 0, data.length, 0);
                FS.close(stream);

                FS.syncfs(false, function (err) {
                    if (err) {
                        reject('WRITE TO DB FAIL: ' + dictPath);
                    } else {
                        resolve();
                    }
                });
            });
        }

        function loadDictAndWriteToDB(baseURL, dict, dbName) {
            return xhrLoadDict(baseURL, dict).then(
                function(arraybuf) {
                    return writeDictToDB(arraybuf, dbName + '/' + dict);
                },
                function(err) {
                    return Promise.reject(err);
                }
            );
        }

        function sequencePromise(tasks) {
            return tasks.reduce(function(promise, task) {
                return promise.then(task);
            }, Promise.resolve());
        }

        function loadJiebaDicts(dicts, baseURL, dbName) {
            var promises = dicts.map(function(dict) {
                return loadDictAndWriteToDB.bind(null, baseURL, dict, dbName);
            });
            return sequencePromise(promises);
        }

        // 检查jieba所需词典是否已经在DB中
        function checkJiebaDictsReady(dicts, dbName) {
            return new Promise(function(resolve, reject) {
                FS.syncfs(true, function (err) {
                    if (err) {
                        reject();
                    }
                    dicts.forEach(function(dict) {
                        var dictPath = dbName + '/' + dict;
                        if (!FS.analyzePath(dictPath).exists) {
                            reject();
                        }
                    });
                    resolve();
                });
            });
        }

        function toUTF8String(str) {
            var lengthBytes = lengthBytesUTF8(str)+1;
            var stringOnWasmHeap = Module._malloc(lengthBytes);
            stringToUTF8(str, stringOnWasmHeap, lengthBytes);
            return stringOnWasmHeap;
        }

        // 注意： array 需要 () 包裹，否则会编译报错
        var dicts = ([
            'jieba.dict.utf8',
            'hmm_model.utf8',
            'user.dict.utf8',
            'idf.utf8',
            'stop_words.utf8',
        ]);
        var BaseURL = 'https://raw.githubusercontent.com/yanyiwu/cppjieba/master/dict/';
        var DB_NAME = '/offline';

        checkJiebaDictsReady(dicts, DB_NAME).then(() => {
            Module._checkDict(toUTF8String('EXIST'));
        }, () => {
            var t0 = performance.now();
            // 加载jieba所需词典
            loadJiebaDicts(dicts, BaseURL, DB_NAME).then(function(res) {
                var t1 = performance.now();
                Module._checkDict(toUTF8String('DOWNLOAD_SUCCESS: ' + (t1-t0) + 'ms'));
            }, function(err) {
                Module._checkDict(toUTF8String('DOWNLOAD_FAIL'));
            });
        });
    );
}
