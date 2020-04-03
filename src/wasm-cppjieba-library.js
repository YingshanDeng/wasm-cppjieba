mergeInto(LibraryManager.library, {
    /**
     * 检查 jieba 词典
     * @param {*} type EXIST|DOWNLOAD_SUCCESS|DOWNLOAD_FAIL
     */
    afterCheckJiebaDictsCallback: function(type) {
        self.postMessage({
            cmd: 'dicts-check',
            data: UTF8ToString(type),
        });
    },

    // jieba 实例化完成
    afterInitJiebaCallback: function() {
        self.postMessage({
            cmd: 'jieba-ready',
        });
    },

    // jieba 分词完成
    afterCutSentenceCallback: function(addr, length) {
        var cuts = [];
        for (var i = 0; i < length; i++) {
            cuts[i] = Module.HEAP8[addr + i]
        }
        Module._free(addr);

        self.postMessage({
            cmd: 'cut-finish',
            data: cuts,
        });
    },

    // 执行耗时上报
    report: function(type, time) {
        self.postMessage({
            cmd: 'report',
            data: {
                type: UTF8ToString(type),
                time: time,
            }
        });
    },
});
