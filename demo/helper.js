var worker = null;

var dictTask = null;
var initTask = null;
var cutTask = null;
var reportTask = null;

function createJieba(workerFilePath, dictStatusCb, reportCb) {
    dictTask = dictStatusCb;
    reportTask = reportCb;

    worker = new Worker(workerFilePath);
    worker.onmessage = function(e) {
        var obj = e.data;
        switch (obj.cmd) {
            case 'dicts-check':
                dictTask && dictTask(obj.data);
                break;
            case 'jieba-ready':
                initTask && initTask();
                break;
            case 'cut-finish':
                cutTask && cutTask(obj.data);
                break;
            case 'report':
                reportTask && reportTask(obj.data.type + ': ' + obj.data.time);
                break;
            case 'warn':
                console.log('unknown cmd !!!');
                break;
            default:
                console.error('what cmd ???', obj.cmd);
                break;
        }
    }
}

// init-jieba
function initJieba(callback) {
    initTask = callback || (() => { console.log('jieba 实例化完成！') });
    worker && worker.postMessage({
        cmd: 'init-jieba',
    });
}
// cut-sentence
function cutSentencePromise(str) {
    if (!str || !str.length) {
        return Promise.resolve([]);
    }
    var cutPromise = (resolve, reject) => {
        cutTask = (res) => {
            resolve(res);
        }
        worker && worker.postMessage({
            cmd: 'cut-sentence',
            sentence: str || '',
        });
    }
    return new Promise(cutPromise);
}
function cutSentenceCallback(str, callback) {
    if (!str || !str.length) {
        return callback && callback([]);
    }
    cutTask = callback || ((res) => { console.log(res) });
    worker && worker.postMessage({
        cmd: 'cut-sentence',
        sentence: str || "",
    });
}
