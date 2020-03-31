mergeInto(LibraryManager.library, {
    // jieba 实例化完成
    afterInitJiebaCallback: function() {
        console.log('jieba 实例化完成！');

        // 调用分词方法，进行分词
        var str = "他来到了网易杭研大厦";

        var lengthBytes = lengthBytesUTF8(str)+1;
        var stringOnWasmHeap = Module._malloc(lengthBytes);
        stringToUTF8(str, stringOnWasmHeap, lengthBytes);

        Module._cutSentence(stringOnWasmHeap);
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
