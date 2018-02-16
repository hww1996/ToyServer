# ToyServer
### 测试版本，还有待完善
这个Server只是一个简单的Server，使用了一些Linux的功能函数，比如有：线程（在线程中使用了条件量和互斥量），fork，ipc，信号，socket，epoll等等的一些函数。还实现了一个小型的string。并且有文件的映射

下次应该会改变一些算法，如使用kmp算法来匹配字符串等。修正一些bug等等。

#### 编译:

	$ make

#### 使用：

在`include/source.h`中的url_maps中添加map_item即可

#### 示例：

&nbsp;&nbsp;`map_item("/static","~/ServerContent",1)`

#### 解释：

| 第一个参数     | 第二个参数                   | 第三个参数  |
| ------------- |:---------------------------:| ----------:|
| 映射的url     | 文件中的目录（最好是绝对路径） | 怎样去读    |

#### 第三个参数的解释

|数字|代表的意思  |
|----|-----------|
|0   |执行这个文件|
|1   |读取这个文件|

#### 原型：

    struct map_item{
      CharContent uri;
      CharContent file_path;
      int read_type; 
    };
#### 注意：
只能使用g++编译，clang编译不过。
