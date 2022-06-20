# Train-Ticket-System

****

ACM 2022 Data-Structure Homework Final!

历时不知道多少天，终于通过OJ数据啦！

### 组内分工：

逻辑部分：尹良升

B+树部分：董珅

### 各分支用途：

- `master` 用于每一个大版本的发布（其实也就是提交OJ）
- `dev_yls` 在没有`bptree`的时候，用`map`代替`bptree`的调试分支
- `dev_ds` B+树以及外存部分的开发分支，完全由董珅管理。
- `test` 用于合并`dev_ds`以及代码的调试
- `test_output_directly` 将`vector<std::string>`格式的代码输出改成直接输出，后合并至`test`
- `test_cached_in_function` 将`query_transfer`的缓存改在函数内（~~为了卡常数~~），会爆内存，后舍弃
- `test_rollback` 加了一个外存分块栈，用于记录回滚信息，已合并至`master`

### 缓存的实现机制：

- 对`bptree`的`find`操作，`modify`操作进行缓存。
- 用一个闭散列表来实现，有冲突时直接下放到外存。
- 发生修改的元素使用链表连接，在`bptree`中执行`lower_bound`操作时，先遍历链表执行`flush`操作。

### 回滚的实现机制：

- 每一颗`bptree`用一个外存的栈来记录操作。
- 外存的栈同样采用分块的形式，对象中自带一个大小约为`4096`的块进行缓存。

### 特别鸣谢：

- 感谢`wennitao`、`pks`、`RainyMemory`三位课程助教对我们组的帮助。
- 感谢董易昕学长、洪熠佳学长在卡常上的帮助。
- 感谢郭俊贤、范棋珈、冯悦洋、梁亚伦、刘祎禹、王俞皓等同学在后端实现过程中的帮助。
- `......`
