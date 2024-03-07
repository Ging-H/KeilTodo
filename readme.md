[Engilsh](readme.en.md)



## 编译环境

* Qt5.9 or later
* msvc2015 32-bit(64bit) or later/MinGW5.3 32bit(64bit) or later
* Qt Creator 5.0.3 or later

## 用法

```shell
Usage: keiltodo [options] files
list keyword from Keil-MDKv5 project or source file

Options:
  -?, -h, --help  Displays this help.
  -v, --version   Displays version information.
  -k <keyword>    specific keyword to be scan

Arguments:
  files           any files or Keil-MDKv5 project file
```

#### 在命令提示符cmd中使用

* 使用默认的关键字

```shell
E:\QtApplication\KeilTodo\exe>keiltodo main.cpp
keyword - warning todo fixme fixed info bug
main.cpp(7): [fixed]  * @fixed    line8
main.cpp(8): [fixed]  * @fixed:   line8
main.cpp(9): [fixed]  * @fixed:line9
main.cpp(10): [fixed]  * @fixed:"line10"
main.cpp(11): [fixed]  * @fixed    line11
main.cpp(16): [todo]  * @todo:    line10
main.cpp(17): [warning]  * @warning: line11
```

* 使用特定的关键字

```sh
E:\QtApplication\KeilTodo\exe>KeilTodo "E:\MDK-ARM\YSF4_DA.uvprojx" -k=fixed
keyword - fixed
../Src/main.c(57): [fixed]   *          @fixed: 提前配置初始化DAC
../Src/main.c(58): [fixed]   *          @fixed: 修正DAC输出通道配置 v3.0.0 20241522
../Src/main.c(172): [fixed]         /** @fixed:"注释振镜报警,为了方便调试,实装应取消注释" */
```

#### 在Keil-MDK中使用

* 使用默认的关键字(搜索当前打开的文件)

![image-20240222194158993](readme.assets/image-20240222194158993.png)

* 使用默认的关键字(搜索整个工程)

![image-20240222194325838](readme.assets/image-20240222194325838.png)

* 在关键字前添加 `'@'`, 将会在 `Build Output windows`显示出来,双击可以跳转到行![image-20240307093731087](readme.assets/image-20240307093731087.png)

在任意文件都可以使用自定义的关键字,而且关键字是可以是中文(只支持GB2312/utf8编码的文件)

![image-20240307095710673](readme.assets/image-20240307095710673.png)

