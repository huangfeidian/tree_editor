# tree_editor
一个使用Qt5编写的树形结构编辑器，典型使用样例是用来作为公式编辑器和行为树编辑器。



## 依赖

1. Qt5 作为整体的界面编写和图形展示
2. [spdlog](https://github.com/gabime/spdlog) 用来处理log
3. [json](https://github.com/nlohmann/json) 作为数据的交换格式
4. [any_container](https://github.com/huangfeidian/any_container) 我自己写的一个小工具用来处理自定义对象与json之间的转换
5. [http_server](https://github.com/huangfeidian/http_server) 我自己写的一个小工具 用来当作一个简单的http server 

## 编译

当前项目使用cmake来管理，要求c++版本为17，编译与安装的流程与普通cmake库没啥不同，因此不做介绍。

