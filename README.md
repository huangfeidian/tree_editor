# tree_editor
一个使用Qt5编写的树形结构编辑器，典型使用样例是用来作为公式编辑器和行为树编辑器。同时这个项目不仅提供了编辑器，还提供了调试器，包含了历史记录文件调试器和网络调试器两个部分。



## 依赖

1. Qt5 作为整体的界面编写和图形展示
2. [spdlog](https://github.com/gabime/spdlog) 用来处理log
3. [json](https://github.com/nlohmann/json) 作为数据的交换格式
4. [any_container](https://github.com/huangfeidian/any_container) 我自己写的一个小工具用来处理自定义对象与json之间的转换
5. [http_server](https://github.com/huangfeidian/http_server) 我自己写的一个小工具 用来当作一个简单的http server 

## 编译

当前项目使用cmake来管理，要求c++版本为17，编译与安装的流程与普通cmake库没啥不同，因此不做介绍。

## 编辑器

编辑器是cmake生成的editor项目。

![树结构界面](https://github.com/huangfeidian/tree_editor/blob/master/screenshot/math_example.png?raw=true)



![编辑窗口](https://github.com/huangfeidian/tree_editor/blob/master/screenshot/edit_dialog.png?raw=true)

本编辑器支持多文件以`tab`形式同时编辑， 这里可以有多`tab`页，以图形的格式展示整个行为树的结构。每个节点的子节点都在当前节点的右侧，这个跟`unreal`那种上下布局不太一样，准备以后支持两种布局。每个节点上面的`label`分为了三个部分：

1. 节点类型 `root sequence`等 一旦确定不可修改
2. 节点编号 这个编号在一颗树内是唯一的非负整数 不可编辑 根节点永远保持为0
3. 节点的展示信息 可以为此节点的自身信息或者`comment`信息所修改

每个节点有相关自带的信息，可以通过右键菜单或者双击节点来触发特定菜单的弹出。

节点编辑时，首先需要选中一个节点，然后按下对应的快捷键：

1. `Insert` 代表插入一个节点，作为当前节点排序最低的子节点
2. `Delete`代表删除一个节点， root节点不可删除
3. `MoveUp` 代表把提升当前节点在父节点里的排序
4. `MoveDown` 代表降低当前节点在父节点里的排序
5. `Copy` 代表把当前节点为根的子树复制
6. `Paste` 代表把上次复制的节点粘贴为当前节点排序最低的新的子节点
7. `Cut` 代表剪切当前节点

## 调试器

调试器的界面与编辑器有点不同，他增加了右侧的历史记录查看窗口。这个历史记录查看窗口分为了多列：

1. 第一列是时间戳列 代表这行历史记录发生在什么时候 双击此列会触发树形窗口展现当前的运行状况
2. 第二例是节点位置列 表明这行记录发生在哪一科树的哪一个节点上
3. 第三列是节点指令列，代表在这个节点上发生了什么
4. 第四列是指令参数列，用来补充提供发生内容的具体参数
5. 第五列是注释列 使用者可以双击此列进行编辑 来添加此行的注释

![调试窗口](https://github.com/huangfeidian/tree_editor/blob/master/screenshot/debugger_example.png?raw=true)

## 使用样例

最简单的使用样例参见example目录下的math_tree，用来编辑数学公式。

更复杂的使用样例是作为行为树编辑器和调试器，这个在我自己的行为树repo里面使用了，链接 https://github.com/huangfeidian/behavior_tree

