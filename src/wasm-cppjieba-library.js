mergeInto(LibraryManager.library, {
    // jieba 实例化完成
    afterInitJiebaCallback: function() {
        console.log('jieba 实例化完成！')
    },
    // jieba 分词完成
    afterCutSentenceCallback: function() {
        // todo ...
    },
    // 执行耗时上报
    report: function(type, time) {
        console.log('执行耗时上报: ', UTF8ToString(type), ' : ', time);
    },
});
