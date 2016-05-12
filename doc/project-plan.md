# Sandnix 0.0.2 架构,模块及接口

## A. 文档说明
该文档在GPL v3协议下发布.

## B. 整体架构及代码风格
### 整体架构
#### 设计原则
* 尽量避免在内核实现驱动程序,初始化用的ramdisk除外.
* 要严格遵守代码风格来保证代码有较好的可读性.
* 不要用全局变量来传参.模块内部使用的变量要加static.
* HAL层以上的代码(不包括HAL层)要尽量使用面向对象的方式来编写.
#### 项目架构
![项目架构图](http://7xsd89.com1.z0.glb.clouddn.com/project-arch.png)

##### HAL层
该层的作用是将管理cpu,mmu,中断以及内核传参等实现os基本功能所必须打交道的在不同架构下存在显著差异部分剥离出来,为工作在它上面的core层,和subsystem层提供一个与硬件环境无关的统一接口.模块列表如下:
* init
* mmu
* early_print
* kparam
* cpu
* io
* sys_gate
* power
* exception
##### core层
该层是内核的核心部分,大部分模块都位于此处.主要以面向对象的方式编写.模块列表如下:
* main
* interrupt
* mm
* kconsole
* pm
* ipc
* msg
* vfs
* rtl
* exception
##### subsystem层
负责提供系统调用,并将系统调用翻译成对内核函数的调用.模块列表如下:
* subsys_driver
* subsys_linux
#### 代码风格要求(参照Linux kernel风格改编)
##### 缩进
缩进为4个空格,8个空格会使代码很靠右,尽量用空格而不是tab(这一点利用astyle可以很轻松的搞定)..因为gihub上面tab看起来相当的糟糕.switch语句里面的case应当缩进来使得代码层次清晰.
不要一行放多个语句.
由于标准的Linux终端是24行80列的,接近或大于80个字符的较长语句要折行写,折行后用空格和上面的表达式或参数对齐.
较长的字符串可以断成多个字符串然后分行书写,例如:
printf("Thisis such a long sentence that " 
"itcannot be held within a line\n");
C编译器会自动把相邻的多个字符串接在一起,以上两个字符串相当于一个字符串"Thisissuch a long sentence that it cannot be held within a line\n".
汇编代码里边函数名要顶格写,循环,分支等结构应当增加缩进以便阅读.
##### 空白
关键字if,while,,for与其后的控制表达式的(括号之间插入一个空格分隔,但括号内的表达式应紧贴括号.
双目运算符的两侧插入一个空格分隔,单目运算符和操作数之间不加空格.
后缀运算符和操作数之间也不加空格,例如取结构体成员s.a,p_s->a,函数调用foo(arg1)、取数组成员a[i].
,号和;号之后要加空格,这是英文的书写习惯,例如for(i=1; i<10; i++), foo(arg1,arg2),
##### 括号的位置
C语言风格中另外一个常见问题是大括号的放置。和缩进大小不同，选择或弃用某种放置策略并没有多少技术上的原因，不过首选的方式，就像Kernighan和Ritchie展示给我们的，是把起始大括号放在行尾，而把结束大括号放在行首.像这样:
if (xxx) {
	xxxxxx;
} else {
	xxx;
}
如果小括号,中括号要分行写,也遵从上述规则.
不过,有一种特殊情况,命名函数.它们的起始大括号放置于下一行的开头,像这样：
int func(int x)
{
	xxxx
}
##### 命名规则
请使用小写字母,数字和下划线给函数,变量以及数据结构命名,大写字母是留给宏使用的.
原则上宏应尽量使用大写字母命名,但是函数式宏定义除外,他们和函数遵守一样的命名规则.
函数,参数,变量的名字要能体现出它的作用.在不破坏可读性的前提条件下尽量使用缩写.
模块导出函数的命名应遵从以下格式:
	模块名_函数名
函数名应尽量对每个单词使用缩写,要能描述出这个函数的作用是什么.构造函数是个例外.
结构体的定义遵守以下规范:
typedef	struct _结构体名 {
	成员表;
} 结构体名, *p结构体名;
一般的,结构体名,type定义的类型名要以_t结尾,如果结构体为一个类的定义的话要以_obj_t结尾.u8,u16,u32这类出于跨平台兼容为目的考虑的对基本类型的重定义除外.
函数内部的标号要用下划线(_)开头.
##### 面向对象
一个类名应当以_obj结尾,类的声明方式如下:
typedef	struct _类名_t {
	父类对象;
	指向成员函数的指针列表;
	成员变量;
} 类名_t , *p类名_t ;
特别的,obj_t是一切类的最终父类.
类的构造函数名遵从以下结构:
	类名_后缀
后缀是可选的,当存在需要重载构造函数的情况时后缀用于区别这些函数.
构造函数的职责包括为新对象分配内存,初始化成员变量并将成员函数的地址填到列表中.
引用和释放一个对象的时候别忘了增减引用计数.
##### 注释
注释的职责是让人知道代码是干啥的,而不是告诉别人代码是怎么干的.除非你写了一大堆烂代码.
如果一个函数/宏的调用有什么注意事项,或者是需要解释点烂七八糟协议之类的什么东西,一定要用注释写出来.
用//TODO:的形式标出未做的工作是个不错的注意,正常点的编辑器都会把它高亮的.
##### Astyle参数
astyle --suffix=none --style=linux --indent=spaces=4 --attach-namespaces --attach-classes --attach-inlines --attach-extern-c --indent-classes --indent-cases --indent-preproc-cond –indent-switches --indent-preproc-define --indent-preproc-block --indent-col1-comments --break-blocks=all --pad-oper --unpad-paren --mode=c $filename


## 模块及其接口
### HAL层
### Core层
### Subsystem层
