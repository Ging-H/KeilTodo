[中文](readme.md)

## Build Env

* Qt5.9 or later
* msvc2015 32-bit(64bit) or later/MinGW5.3 32bit(64bit) or later
* Qt Creator 5.0.3 or later

## Usage

```shell
Usage: Keiltodo [options] files
list keyword from Keil-MDKv5 project or source file

Options:
  -?, -h, --help  Displays this help.
  -v, --version   Displays version information.
  -k <keyword>    specific keyword to be scan
  -l              list all keyword in file
  -d              search hal/ll drivers

Arguments:
  files           Any number of text-files or one Keil-MDKv5 project file
```

#### Usage in cmd

* use default keyword

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

main.cpp(18): [bug] // @bug @ver1.5.1 @bugfix can use multi-keyword in one line
```

* use specific keyword

```sh
E:\QtApplication\KeilTodo\exe>KeilTodo "E:\MDK-ARM\YSF4_DA.uvprojx" -k=fixed
keyword - fixed
../Src/main.c(57): [fixed]   *          @fixed: 提前配置初始化DAC
../Src/main.c(58): [fixed]   *          @fixed: 修正DAC输出通道配置 v3.0.0 20241522
../Src/main.c(172): [fixed]         /** @fixed:"注释振镜报警,为了方便调试,实装应取消注释" */
```

#### Usage in Keil-MDK

* use default keyword(current file opened in editor)

![image-20240222194158993](readme.assets/image-20240222194158993.png)

* use specific keyword(current project)

![image-20240222194325838](readme.assets/image-20240222194325838.png)

​	the line of the keyword will be displayed at `Build Output windows`,double-click to jump to the line.

![image-20240307093731087](readme.assets/image-20240307093731087.png)



#### Add keywords

use `@+"Keyword"`to be searched by the software, and multiple keywords can be added to a row, but be careful to have at least one space between the keyword or context to distinguish and identify.
