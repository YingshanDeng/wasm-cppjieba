#include <emscripten.h>

// 挂载 DB
EM_JS(void, mountDB, (const char* db), {
    var DB_NAME = UTF8ToString(db);
    FS.mkdir(DB_NAME);
    FS.mount(IDBFS, {}, DB_NAME);
});

// todo 检查日历文件在 DB 中是否存在
EM_JS(int, checkDicts, (), {
    // todo ...
    return true;
});

int main() {

    mountDB("/offline");
    checkDicts();

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

        function loadJiebaDicts() {
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

            var promises = dicts.map(function(dict) {
                return loadDictAndWriteToDB.bind(null, BaseURL, dict, DB_NAME);
            });
            return sequencePromise(promises);
        }

        // 加载jieba所需词典
        loadJiebaDicts().then(function(res) {
            console.log('All Dicts load and write to DB!!!', res);
        }, function(err) {
            console.log(err);
        });
    );
}
