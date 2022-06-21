# Frontend Development Document

| 小组成员                                                    |                                组内分工 |
| ----------------------------------------------------------- | --------------------------------------: |
| 尹良升([`@hnyls2002`](https://github.com/hnyls2002))        |            网页服务器、前后端交互、通信 |
| 董珅 ([`@RabbitCabbage`](https://github.com/RabbitCabbage)) | 前端静态部分（`html，css，JavaScript`） |



***

### 静态部分

网页的设计主要由董珅同学负责。

`html`

- `instruction.html`：用户的提示文档
- `log_in.html`：登录界面
- `register.html`：注册界面
- `query_user.html`，`profile.html`，`modify_profile.html`：用户信息的查询、展示、修改界面
- `manager.html`：后台管理员界面（添加、发布、删除火车）
- `query_train.html`，`show_train.html`：查询火车的界面以及查询结果的反馈
- `query_ticket.html`，`show_ticket.html`：查询票的界面以及查询结果的反馈
- `query_transfer.html`：对换乘票的查询
- `buy_ticket.html`：买票的界面
- `query_order.html`：用户所有订单的展示界面（当前用户只能看到自己的所有订单）
- `refund_ticket.html`：退票的界面，同时包含了订单的展示，退票完成之后直接刷新订单的状态

各个`html`网页之间的跳转关系基本符合**一个正常火车票购买系统**的逻辑，但还有可以改进的地方：

- 在用户查询完票之后应该直接在查询结果处提供购买选项，而不是再次重新指定车次和车站后再购票。
- 应该对什么是超级用户，什么是普通用户做更加详细合理的规定。

`css`

- 许多按钮和展示框采用了`Argon Design`免费提供的`css`模板。
- 同时组员自己也设计了很多`css`的样式。

`JavaScript`

- 主要用于警告的弹出、页面信息的动态修改、查询结果的展示
- `...`

`jQuery`

- `html`许多地方使用了`jQuery`小部件，比如说`datepicker`，`time-selector`等等

***

### 网页服务器

##### 服务器的获取

- 腾讯云学生机
- 公网`ip`以及最后展示的端口：[`81.68.197.175:8888`](http://81.68.197.175:8888)

***

### `Web`表单的提交

- 采用`python +flask`的方式，实现过程见`app.py`
- `html`中的表单全部采用`post`方式进行提交，用`flask`的`request`来获取表单，并进一步地传递给同火车票主体部分进行交互的进程。

### 同火车票主体的交互

- 采用`python + pipe`的方式，`python`的`subprocess`库支持通过`python`打开另外一个进程。
- 管道`pipe`接管`./code`进程的通讯，并自定义了读写类，直接通过文件输入输出流写入`./code`进程中。

### 展示结果的动态绘制、报错信息的返回

- 动态绘制不定长的结果时，直接在`html`中嵌套`flask`的循环语句。
- 报错信息的返回利用了后端写的**命令行交互**的`bonus`，在得到`-1`的结果之后将`-1`后面的部分返回给`html`，使之将信息直接显示在一个警告框中。

