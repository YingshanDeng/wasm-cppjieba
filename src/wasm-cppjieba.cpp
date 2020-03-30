#include <emscripten.h>
#include "cppjieba/Jieba.hpp"

using namespace std;

const char* const DICT_PATH = "offline/jieba.dict.utf8";
const char* const HMM_PATH = "offline/hmm_model.utf8";
const char* const USER_DICT_PATH = "offline/user.dict.utf8";
const char* const IDF_PATH = "offline/idf.utf8";
const char* const STOP_WORD_PATH = "offline/stop_words.utf8";

extern "C" {
    cppjieba::Jieba* jieba;

    void initJiebaInstance() {
        jieba = new cppjieba::Jieba(
            DICT_PATH,
            HMM_PATH,
            USER_DICT_PATH,
            IDF_PATH,
            STOP_WORD_PATH
        );
    }

    void test() {
        vector<string> words;
        // vector<cppjieba::Word> jiebawords;
        string s;
        // string result;
        s = "他来到了网易杭研大厦";
        cout << s << endl;
        cout << "[demo] Cut With HMM" << endl;
        jieba->Cut(s, words, true);
        cout << limonp::Join(words.begin(), words.end(), "/") << endl;
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

        var t0;
        var t1;
        checkJiebaDictsReady(dicts, DB_NAME).then(() => {
            // jieba所需词典已经存在 DB 中，可以直接初始化 jieba
            t0 = performance.now();
            Module._initJiebaInstance();
            t1 = performance.now();
            console.log('实例化Jieba耗时：', t1-t0, 'ms');

            t0 = performance.now();
            Module._test();
            t1 = performance.now();
            console.log('执行一次分词耗时：', t1-t0, 'ms');

        }, () => {
            t0 = performance.now();
            // 加载jieba所需词典
            loadJiebaDicts(dicts, BaseURL, DB_NAME).then(function(res) {
                console.log('All Dicts load and write to DB!!!', res);
                t1 = performance.now();
                console.log('加载词典耗时：', t1-t0, 'ms');

                // 初始化 jieba
                t0 = performance.now();
                Module._initJiebaInstance();
                t1 = performance.now();
                console.log('实例化Jieba耗时：', t1-t0, 'ms');

                t0 = performance.now();
                Module._test();
                t1 = performance.now();
                console.log('执行一次分词耗时：', t1-t0, 'ms');

            }, function(err) {
                console.log(err);
            });
        });
    );
}
