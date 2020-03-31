mergeInto(LibraryManager.library, {
    // jieba 实例化完成
    afterInitJiebaCallback: function() {
        console.log('jieba 实例化完成！');

        // 调用分词方法，进行分词
        var str = "jack我是蓝翔技工拖拉机学院手扶拖拉机专业的不用多久我就会升职加薪当上总经理出任CEO迎娶白富美走上人生巅峰。";

        var lengthBytes = lengthBytesUTF8(str)+1;
        var stringOnWasmHeap = Module._malloc(lengthBytes);
        stringToUTF8(str, stringOnWasmHeap, lengthBytes);

        Module._cutSentence(stringOnWasmHeap);

        Module._free(stringOnWasmHeap);
    },
    // jieba 分词完成
    afterCutSentenceCallback: function(addr, length) {

        var cuts = [];
        for (var i = 0; i < length; i++) {
            cuts[i] = Module.HEAP8[addr + i]
        }
        Module._free(addr);

        console.log('===分词结果', cuts, length);
    },
    // 执行耗时上报
    report: function(type, time) {
        console.log('执行耗时上报: ', UTF8ToString(type), ' : ', time);
    },
});
