# Sandnix 0.0.2 架构,模块及接口
***
[TOC]
***
## A. 文档说明
该文档在GPL v3协议下发布.
***
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
* early\_print
* kparam
* cpu
* io
* sys\_gate
* power
* exception
* debug
* rtl

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
* common
* driver
* linux

#### 代码风格要求(参照Linux kernel风格改编)
##### 缩进
缩进为4个空格,8个空格会使代码很靠右,尽量用空格而不是tab(这一点利用astyle可以很轻松的搞定)..因为gihub上面tab看起来相当的糟糕.switch语句里面的case应当缩进来使得代码层次清晰.
不要一行放多个语句.
由于标准的Linux终端是24行80列的,接近或大于80个字符的较长语句要折行写,折行后用空格和上面的表达式或参数对齐.
较长的字符串可以断成多个字符串然后分行书写,例如:
```c
printf("Thisis such a long sentence that " 
"it cannot be held within a line\n");
```
C编译器会自动把相邻的多个字符串接在一起,以上两个字符串相当于一个字符串"This is such a long sentence that it cannot be held within a line\n".
汇编代码里边函数名要顶格写,循环,分支等结构应当增加缩进以便阅读.
##### 空白
关键字if,while,,for与其后的控制表达式的(括号之间插入一个空格分隔,但括号内的表达式应紧贴括号.
双目运算符的两侧插入一个空格分隔,单目运算符和操作数之间不加空格.
后缀运算符和操作数之间也不加空格,例如取结构体成员`s.a,p_s->a`,函数调用`foo(arg1)`、取数组成员`a[i].`
,号和;号之后要加空格,这是英文的书写习惯,例如`for(i=1; i<10; i++), foo(arg1,arg2),`
##### 括号的位置
C语言风格中另外一个常见问题是大括号的放置。和缩进大小不同，选择或弃用某种放置策略并没有多少技术上的原因，不过首选的方式，就像Kernighan和Ritchie展示给我们的，是把起始大括号放在行尾，而把结束大括号放在行首.像这样:
```c
if (xxx) {
	xxxxxx;
} else {
	xxx;
}
```
如果小括号,中括号要分行写,也遵从上述规则.
不过,有一种特殊情况,命名函数.它们的起始大括号放置于下一行的开头,像这样：
```c
int func(int x)
{
	xxxx
}
```
##### 命名规则
请使用小写字母,数字和下划线给函数,变量以及数据结构命名,大写字母是留给宏使用的.
原则上宏应尽量使用大写字母命名,但是函数式宏定义除外,他们和函数遵守一样的命名规则.
函数,参数,变量的名字要能体现出它的作用.在不破坏可读性的前提条件下尽量使用缩写.
模块导出函数的命名应遵从以下格式:
	`层名_模块名_函数名`
函数名应尽量对每个单词使用缩写,要能描述出这个函数的作用是什么.构造函数是个例外.
结构体的定义遵守以下规范:
```c
typedef struct _结构体名 {
	成员表;
} 结构体名, *p结构体名;
```
一般的,结构体名,type定义的类型名要以\_t结尾,如果结构体为一个类的定义的话要以\_obj\_t结尾.u8,u16,u32这类出于跨平台兼容为目的考虑的对基本类型的重定义除外.
函数内部的标号要用下划线(\_)开头.
##### 面向对象
一个类名应当以\_obj结尾,类的声明方式如下:
```c
typedef struct _类名_t {
	父类对象;
	指向成员函数的指针列表;
	成员变量;
} 类名_t , *p类名_t ;
```
特别的,obj\_t是一切类的最终父类.
类的构造函数名遵从以下结构:
	`类名_后缀`
后缀是可选的,当存在需要重载构造函数的情况时后缀用于区别这些函数.
构造函数的职责包括为新对象分配内存,初始化成员变量并将成员函数的地址填到列表中.
引用和释放一个对象的时候别忘了增减引用计数.
##### 注释
注释的职责是让人知道代码是干啥的,而不是告诉别人代码是怎么干的.除非你写了一大堆烂代码.
如果一个函数/宏的调用有什么注意事项,或者是需要解释点烂七八糟协议之类的什么东西,一定要用注释写出来.
用`//TODO:`的形式标出未做的工作是个不错的主意,正常点的编辑器都会把它高亮的.
##### Astyle参数
```bash
astyle --suffix=none --style=linux --indent=spaces=4 --attach-namespaces --attach-classes --attach-inlines --attach-extern-c --indent-classes --indent-cases --indent-preproc-cond –indent-switches --indent-preproc-define --indent-preproc-block --indent-col1-comments --break-blocks=all --pad-oper --unpad-paren --mode=c $filename
```

* * *
###配置工具Skconfig
该工具由python编写,作用是为内核的配置和编译提供一个直观有效的配置工具,类似于linux的kconfig.
内核项目比较特殊,很多配置工具都是量身定做的,想通用很难.如果采用autotools,编译的时候那选项够你爽的.如果采用kconfig,linux关于模块的处理方式和本项目差异又很大,根本没法通用,至于自己写makefile,那工作量和维护难度我只能说是呵呵呵呵了,所以说还是要自己动手丰衣足食.
####如何管理项目的编译
由于该项目在编译的时候有多种架构,我把一个项目分成一下两个元素:
* 目标(target)
* 架构(arch)

两者都是可以嵌套的,架构嵌套架构,目标嵌套目标.每个target都会包含一个或者多个arch.工具在扫描配置文件的时候会根据arch里的信息生成编译用的命令模板,然后通过读target里的选项生成编译选项,最后生成编译用的命令.父target必须包含子target的所有arch.子target里有arch的话子target的arch会覆盖父target的arch,否则继承父target的arch.特别的,父target依赖于子target.
一个target的构建分为一下几个步骤:
* 构建依赖target
* 构建子target
* 执行编译前命令(pre-compile)
* 生成依赖(dep)
* 编译(compile)
* 链接(link)
* 执行编译后命令(after-compile)

每个target由一个target.xml描述,该文件放在每个target的根目录下.同时位于该目录下的还有sources,sources.$(arch)文件,用于记录需要编译的源文件路径,其中sources.$(arch)文件会优先被扫描.
在项目的根目录下执行./skconfig打开配置界面,./configuren生成makefile.每一个target在扫描和编译的时候当前目录都是target所在的目录.
sources文件内部可以用"#"来注释,"#"后面的内容将会被忽略.
target.xml文件结构如下:
```xml
<!--target标签的name属性是目标的名字,type为build或virtual-->
<?xml version="1.0" encoding="utf-8"?><target name="" type="">
    <!--输出文件名-->
    <output name="输出文件名" />
    <!--输出目录-->
    <outdir path="输出目录" />
    <!--中间文件目录-->
    <middir path="中间文件目录" />
    <!--说明-->
    <introduction>sandnix</introduction>
    <!--架构列表-->
    <archs actived="x86">
        <arch name="x86">
            <!--所有的命令都会被原样写到gnu makefile中,都对应着同名变量-->
            <!--没写的架构会继承父项目的-->
            <PREV>这里是编译前的命令</PREV>
            <DEP>生成依赖的工具</DEP>
            <DEPRULE>生成依赖的命令</DEPRULE>
            <CC>c编译器</CC>
            <CFLAGS>c编译选项</CFLAGS>
            <CCRULE>c编译命令</CCRULE>
            <AS>汇编编译器</AS>
            <ASFLAGS>汇编编译选项</ASFLAGS>
            <ASRULE>汇编编译命令</ASRULE>
            <LD>链接器</LD>
            <LDFLAGS>链接选项</LDFLAGS>
            <LDRULE>链接命令</LDRULE>
            <AFTER>编译后执行的命令</AFTER>
        </arch>
        <!--用"."表示层次关系-->
        <arch name="arm.raspberry">
            <PREV>这里是编译前的命令</PREV>
            <DEP>生成依赖的工具</DEP>
            <DEPRULE>生成依赖的命令</DEPRULE>
            <CC>c编译器</CC>
            <CFLAGS>c编译选项</CFLAGS>
            <CCRULE>c编译命令</CCRULE>
            <AS>汇编编译器</AS>
            <ASFLAGS>汇编编译选项</ASFLAGS>
            <ASRULE>汇编编译命令</ASRULE>
            <LD>链接器</LD>
            <LDFLAGS>链接选项</LDFLAGS>
            <LDRULE>链接命令</LDRULE>
            <AFTER>编译后执行的命令</AFTER>
        </arch>
        <!--其他的架构在这里-->
    </archs>
    <dependencies>
        <!--依赖的target的列表,填当前target目录的-->
        <dep path="sub target path" />
    </dependencies>
    <sub-targets>
        <!--子target列表-->
        <target enable="true" path="" />
    </sub-targets>
    <options>
        <!--单选框-->
        <option type="checkbox" name="Enable Debigging" value="-g" enable="true" target="CFLAGS|ASFLAGS|LDFLAGS" />
        <!--单选一-->
        <option type="list" name="Bootloader" selected="0" target="CFLAGS|ASFLAGS|LDFLAGS">
            <item name="grub2" value="-DGRUB" />
            <item name="uboot" value="-DUBOOT" />
        </option>
        <!--单行文本框-->
        <option type="input" name="Comment" marco="-DCOMMENT" value="" target="CFLAGS|ASFLAGS|LDFLAGS" />
        <!--菜单-->
        <option type="menu" name="Early print options">
            <!--这里是在该子菜单中的选项-->
        </option>
    </options>
</target>
```

###公共基本类型
代码位于src/common下,提供一些基本的宏及类型定义.
####基本数据类型
```c
//无符号8位数
u8

//无符号16位数
u16

//无符号32位数
u32

//无符号64位数
u64

//有符号8位数
s8

//有符号16位数
s16

//有符号32位数
s32

//有符号64位数
s64

//小端模式无符号8位数
le8

//小端模式无符号16位数
le16

//小端模式无符号32位数
le32

//小端模式无符号64位数
le64

//无符号大小
size_t

//无符号大小
ssize_t

//偏移量
off_t

//布尔类型
bool

//地址,该类型通常在对地址进行数学运算时使用
address_t

//状态
kstatus_t
```
###可引导映像头
代码位于src/sandnix/header下,包括定义映像头的数据结构的.S文件以及相应的生成脚本.
###内核主体
代码位于src/sandnix/kernel下,为内核主体部分代码所在位置.
####HAL层
HAL层负责将不同架构的硬件如CPU,中断控制器,MMU等抽象出统一的接口.该层包括以下模块:
#####init
该模块为整个内核的入口,负责准备内核运行的基本环境,调用其他模块的初始化接口,并在这之后将控制权转移到Core层.
######模块路径
```c
src/sandnix/kernel/hal/init
```
######接口数据结构
```c
//记录bootloader传来的物理内存信息,架构相关.
boot_mem_map_t;

//负责记录初始化内存盘的位置,physical memory map,启动参数和内核代码段的地址.
kernel_header_t:
```
######接口函数及宏:
```c
//初始化堆栈
void* init_stack;

//初始化堆栈大小
#define INIT_STACK_SIZE

//初始化
//整个内核的入口函数.
void _start():

//返回bootloader传来的memory map,格式为physical_memory_info_t,类型定义在mmu模块中.
list_t hal_init_get_boot_memory_map();

//返回初始化内存盘的位置和大小.
void hal_init_get_initrd_addr(
	void** p_addr,		//返回的起始地址
    size_t* p_size);	//返回的大小

//获得内核的命令行参数.
char* hal_init_get_kernel_cmdline();
```
######文件列表
```c
//c运行环境初始化代码
src/sandnix/kernel/hal/init/arch/$(arch)/init.s

//kernel_header初始化代码
src/sandnix/kernel/hal/init/arch/$(arch)/header.h
src/sandnix/kernel/hal/init/arch/$(arch)/header.c

//HAL层初始化代码
src/sandnix/kernel/hal/init/init.c

//接口头文件
src/sandnix/kernel/hal/init/init.h
```
#####kparam
负责解析内核参数.
######模块路径
```
src/sandnix/kernel/hal/kparam
```
######接口数据结构
```c

```
######接口函数及宏
```c
//初始化
//初始化模块,解析内核参数
void hal_kparam_init();

//获得内核参数
kstatus_t hal_kparam_get_value(char* key, char* buf, size_t size);
```
######文件列表
```c
//接口头文件
src/sandnix/kernel/hal/kparam/kparam.h

//参数解析,管理代码
src/sandnix/kernel/hal/kparam/kparam.c
```
#####mmu
负责初始化分页机制,管理mmu以及提供分页管理的接口.
######模块路径
```
src/sandnix/kernel/hal/mmu
```
######接口数据结构
```c
//物理内存信息
physical_memory_info_t
```
######接口函数及宏
```c
//物理内存属性
#define	PHYMEM_AVAILABLE	0x00
#define	PHYMEM_DMA			0x01
#define	PHYMEM_USED			0x02
#define	PHYMEM_DMA_USED		0x03
#define	PHYMEM_SYSTEM		0x04
#define	PHYMEM_RESERVED		0x05
#define	PHYMEM_BAD			0x06

//页面大小
#define	SANDNIX_KERNEL_PAGE_SIZE	4096

//初始化
//启动分页
void start_paging();

//添加临时页表映射页
void* hal_mmu_add_early_paging_addr(void* phy_addr);

//初始化mmu模块
void hal_mmu_init();

//初始化处理器核心
void hal_mmu_core_init(
	int cpuid);		//当前cpu的id
    
//释放处理器核心
void hal_mmu_core_release(
	int cpuid);		//当前cpu的id

//物理内存管理
//申请物理内存
kstatus_t hal_mmu_phymem_alloc(
	void** p_addr,		//起始地址
    address_t align,	//对齐
    bool is_dma,		//是否为DMA保留内存
	size_t page_num);	//页面数

//释放物理内存
void hal_mmu_phymem_free(
	void* addr);		//起始地址

//获得物理内存信息,返回所需内存大小
size_t hal_mmu_get_phymem_info(
	pphysical_memory_info_t p_buf,	//返回的内存信息缓冲区首地址
    size_t size);					//缓冲区大小

//分页管理
//获得内核地址范围
void hal_mmu_get_krnl_addr_range(
	void** p_base,		//指向首地址
    size_t* p_size);	//指向大小

//获得用户地址范围
void hal_mmu_get_usr_addr_range(
	void** p_base,		//首地址
    size_t* p_size);	//大小

//创建页表
kstatus_t hal_mmu_pg_tbl_create(
	u32* id);		//指向新页表id

//销毁页表
void hal_mmu_pg_tbl_destroy(
	u32 id);		//页表id

//设置页表条目
kstatus_t hal_mmu_pg_tbl_set(
	u32 id,
	void* virt_addr,				//线性地址
    u32 attribute,					//属性
    void* phy_addr);				//物理地址

//获得当前地址映射信息
void hal_mmu_pg_tbl_get(
	u32 id,
	void* virt_addr,		//线性地址
    void** phy_addr,		//指向返回的物理地址
    u32* p_attr);			//返回的属性

//刷新页表cache
void hal_mmu_pg_tbl_refresh(void* virt_addr);

//切换到指定页表
void hal_mmu_pg_tbl_switch(
	u32 id);			//切换到的页表id
```
######文件列表
```c
//接口头文件
src/sandnix/kernel/hal/mmu/mmu.h

//具体架构页表操作
src/sandnix/kernel/hal/mmu/paging/paging.h
src/sandnix/kernel/hal/mmu/paging/arch/$(arch)/page_table.h
src/sandnix/kernel/hal/mmu/paging/arch/$(arch)/paging.c

//具体架构物理内存操作
src/sandnix/kernel/hal/mmu/phymem/phymem.h
src/sandnix/kernel/hal/mmu/phymem/arch/$(arch)/phymem.c
```
#####early\_print
负责在tty设备无法使用的情况下内核信息的输出.
######模块路径
```
src/sandnix/kernel/hal/early_print
```
######接口数据结构
```c

```
######接口函数及宏
```c
//初始化
//初始化临时终端
void hal_early_print_init();

//清屏
void hal_early_print_cls();

//设置临时终端颜色
void hal_early_print_color(
	u32 fg,		//前景色
    u32 bg);	//背景色

//打印字符串
void hal_early_print_puts(
	char* str);	//被打印的字符串

//格式化输出
void hal_early_print_printf(char* fmt, ...);

//读取输出缓冲区
size_t hal_early_print_read(char* buf, size_t len);
```
######文件列表
```c
//接口头文件
src/sandnix/kernel/hal/early_print/early_print.h

//临时终端实现代码
src/sandnix/kernel/hal/early_print/arch/$(arch)/early_print.c
```
#####io
负责中断的管理,虚拟IRQ的映射,系统时钟的管理.
######模块路径
```
src/sandnix/kernel/hal/io
```
######接口数据结构
```c
//中断处理回调
int_callback_t
```
######接口函数及宏
```c
//初始化
//初始化模块
void hal_io_init()

//初始化处理器核心
void hal_io_core_init(
	u32 cpuid);		//当前cpu的id
    
//释放处理器核心
void hal_io_core_release(
	u32 cpuid);		//当前cpu的id

//中断管理
//禁止当前核心中断
void hal_io_int_disable();

//允许当前核心中断
void hal_io_int_enable();

//允许所有IRQ
void hal_io_irq_enable_all();

//禁止所有IRQ
void hal_io_irq_disable_all();

//允许IRQ
void hal_io_irq_enable(
	u32 num);		//IRQ中断号

//禁止IRQ
void hal_io_irq_disable(
	u32 num);		//IRQ中断号
    
//获得IRQ范围
void hal_io_get_irq_range(
	u32* p_begin,	//起始IRQ
    u32 num);		//IRQ个数

//注册/取消中断回调
void* hal_io_int_callback_reg(
	u32 num,					//中断号
    int_callback_t callback);	//回调函数

//注册时钟回调
void* hal_io_clock_callback_reg(
	int_callback_t callback);	//回调函数
    
//获得系统tick数
u64 hal_io_get_ticks();

//设置时钟中断频率
void hal_io_set_clock_freq(
	u32 freq);		//频率(Hz)

//获得时钟中断频率
u32 hal_io_get_clock_freq();

//IO管理
//只有部分平台可用
//IN
u8 hal_io_in_8(address_t port);
u16 hal_io_in_16(address_t port);
u32 hal_io_in_32(address_t port);
u64 hal_io_in_64(address_t port);

//INS
void hal_io_ins_8(void* dest, size_t count, address_t port);
void hal_io_ins_16(void* dest, size_t count, address_t port);
void hal_io_ins_32(void* dest, size_t count, address_t port);
void hal_io_ins_64(void* dest, size_t count, address_t port);

//OUT
void hal_io_out_8(address_t port, u8 data);
void hal_io_out_16(address_t port, u16 data);
void hal_io_out_32(address_t port, u32 data);
void hal_io_out_64(address_t port, u64 data);

//OUTS
void hal_io_outs_8(address_t port, size_t count, void* src);
void hal_io_outs_16(address_t port, size_t count, void* src);
void hal_io_outs_32(address_t port, size_t count, void* src);
void hal_io_outs_64(address_t port, size_t count, void* src);

```
######文件列表
```c
//接口头文件
src/sandnix/kernel/hal/io/io.h

//中断
src/sandnix/kernel/hal/io/interrupt/$(arch)/

//IRQ
src/sandnix/kernel/hal/io/irq/$(arch)/
```
#####cpu
负责管理cpu的状态,以及保护线程上下文.
######模块路径
```
src/sandnix/kernel/hal/cpu
```
######接口数据结构
```c
//线程上下文
context_t

//cpu信息
cpuinfo_t
```
######接口函数及宏
```c
//初始化
//初始化cpu模块
void hal_cpu_init();

//线程上下文管理
//保存线程上下文
#define hal_cpu_context_save(p_context)

//加载线程上下文
#define hal_cpu_context_load(p_context)

//多核心管理
//唤醒核心
void hal_cpu_core_awake(u32 cpuid);

//释放当前核心
void hal_cpu_core_release();

//cpu信息
void hal_cpu_get_info(pcpuinfo_t p_ret);

//性能管理
//读取cpu频率
u32 hal_cpu_get_frequency();

//设置cpu频率
u32 hal_cpu_set_frequency();
```
######文件列表
```c
//接口头文件
src/sandnix/kernel/hal/cpu/cpu.h

//上下文
src/sandnix/kernel/hal/cpu/context/$(arch)

//cpu信息
src/sandnix/kernel/hal/cpu/cpu-info/$(arch)
```
#####exception
负责处理kernel panic以及调用错误处理程序.
######模块路径
```
src/sandnix/kernel/hal/exception
```
######接口数据结构
```c
//void mem_fault_hndlr(u32 thread_id, address_t address, u32 operation)
typedef void	(*mem_fault_hndlr)(u32, address_t, u32);

//void privilege_fault_hndlr(u32 thread_id, address_t address)
typedef void	(*privilege_fault_hndlr)(u32, address_t);

//void undef_op_fault_hndlr(u32 thread_id, address_t address
typedef void	(*undef_op_fault_hndlr)(u32, address_t;

//错误处理程序链表
hndlr_info;
```
######接口函数及宏
```c
//初始化模块
void hal_exception_init();

//初始化处理器核心
void hal_exception_core_init(
	u32 cpuid);
    
//释放处理器核心
void hal_exception_core_release(
	u32 cpuid);

//报错并终止整个系统
void hal_exception_panic(
	u32 error_code,		//错误码
    char* fmt,			//格式化字符串
    ...);

//添加错误处理程序
//内存错误
void hal_exception_mem_fault_hndlr_set(
	phndlr_info p_hndlr_info);

//越权操作
void hal_exception_privilege_fault_hndlr_set(
	phndlr_info p_hndlr_info);

//未定义指令
void hal_exception_undef_op_fault_hndlr_set(
	phndlr_info p_hndlr_info);

//Assert
#define ASSERT(x)
```
######文件列表
```c
//接口头文件
src/sandnix/kernel/hal/exception/exception.h

//errno
src/sandnix/kernel/hal/exception/errno.h

//错误处理
src/sandnix/kernel/hal/exception/%(arch)/exception.c

//Panic
src/sandnix/kernel/hal/exception/panic.c
```
#####power
负责处理基本的电源操作,比如断电,复位,睡眠等.
######模块路径
```
src/sandnix/kernel/hal/power/
```
######接口数据结构
```c

```
######接口函数及宏
```c
//h初始化模块
void hal_power_init();

//初始化处理器核心
void hal_power_core_init(u32 cpuid);

//释放处理器核心
void hal_power_core_release(u32 cpuid);

//断电
void hal_power_off();

//睡眠
void hal_power_sleep();

//复位
void hal_power_reset();
```
######文件列表
```c
//接口头文件
src/sandnix/kernel/hal/power/power.h

//实现代码
src/sandnix/kernel/hal/power/$(arch)/power.c
```
#####sys\_gate
负责提供用户态和内核态之间的切换.
######模块路径
```
src/sandnix/kernel/hal/sys_gate/
```
######接口数据结构
```c

```
######接口函数及宏
```c
//初始化模块
void hal_sys_gate_init();

//初始化cpu核心
void hal_sys_gate_core_init();

//设置入口地址
void hal_sys_gate_set_entry(void* entry);

//返回到用户空间
void hal_sys_gate_ret(
	pcontext_t p_context);	//用户空间上下文

//转移到用户空间
void hal_sys_gate_go_to_usr(
	void* entry,		//地址
    int argc,			//参数个数
    char* argv[]);		//参数表
```
######文件列表
```c
//接口头文件
src/sandnix/kernel/hal/sys_gate/sys_gate.h

//实现文件
src/sandnix/kernel/hal/sys_gate/$(arch)/sys_gate.c
```
#####debug
调试
######模块路径
```
src/sandnix/kernel/hal/debug/
```
######接口数据结构
```c

```
######接口函数及宏
```c
//初始化模块
void hal_debug_init();

//是否开启调试
bool hal_debug_is_on_dbg());
```
######文件列表
```c
//接口头文件
src/sandnix/kernel/hal/debug/debug.h

//实现文件
src/sandnix/kernel/hal/debug/debug.c
```
#####rtl
运行时库中架构相关的部分
######模块路径
```
src/sandnix/kernel/hal/rtl/
```
######接口数据结构
```c

```
######接口函数及宏
```c
//Atomic
#define hal_rtl_atomic_xaddl(dest, src)
#define hal_rtl_atomic_cmpxchgl(dest, src, cmp)

//String
#define hal_rtl_string_movsb(dest, src, count)
#define hal_rtl_string_movsw(dest, src, count)
#define hal_rtl_string_movsl(dest, src, count)
#define hal_rtl_string_movsq(dest, src, count)

#define hal_rtl_string_movsb_back(dest, src, count)
#define hal_rtl_string_movsw_back(dest, src, count)
#define hal_rtl_string_movsl_back(dest, src, count)
#define hal_rtl_string_movsq_back(dest, src, count)

#define hal_rtl_string_setsb(dest, val, count)
#define hal_rtl_string_setsw(dest, val, count)
#define hal_rtl_string_setsl(dest, val, count)
#define hal_rtl_string_setsq(dest, val, count)

extern inline u64 rtl_math_div64(u64 dividend, u32 divisor);
extern inline u64 rtl_math_mod64(u64 dividend, u32 divisor);
```
######文件列表
```c
//接口头文件
src/sandnix/kernel/hal/rtl/rtl.h

//Atomic
src/sandnix/kernel/hal/rtl/atomic/$(arch)

//String
src/sandnix/kernel/hal/rtl/string/string.h
src/sandnix/kernel/hal/rtl/string/$(arch)/string.h

//Math
src/sandnix/kernel/hal/rtl/math/math.h
```
####Core层
#####main
Core层的入口
######模块路径
```
src/sandnix/kernel/core/main
```
######接口数据结构
```c

```
######接口函数及宏
```c
//BSP核心主函数
void core_main_main();

//其他核心主函数
void core_main_core_main(u32 cpuid);
```
######文件列表
```c
src/sandnix/kernel/core/main/main.h
src/sandnix/kernel/core/main/main.c

```
#####interrupt
中断管理
######模块路径
```
src/sandnix/kernel/core/interrupt
```
######接口数据结构
```c
//irq处理函数
irq_hndlr_t

```
######接口函数及宏
```c
//初始化
void core_interrupt_init();

//初始化cpu核心
void core_interrupt_core_init(u32 cpuid);

//释放cpu核心
void core_interrupt_core_release(u32 cpuid);

//注册回调函数
//0代表所有的
void core_interrupt_reg_hndlr(
	u32 irq,				//中断号
    irq_hndlr_t hndlr);		//回调函数

//释放回调函数
void core_interrupt_unreg_hndlr(
	u32 irq,				//中断号
    irq_hndlr_t hndlr);		//回调函数

//获得irq的优先级
u32 core_interrupt_get_irq_priority(u32 irq);

//设置irq的优先级
void core_interrupt_set_irq_priority(u32 irq, u32 priority);
```
######文件列表
```c
src/sandnix/kernel/core/interrupt/interrupt.h
src/sandnix/kernel/core/interrupt/interrupt.c
```
#####mm
内存管理,包括页面的分配,映射,释放,物理内存的分配,释放,堆管理等
######模块路径
```
src/sandnix/kernel/core/mm
```
######接口数据结构
```c
//通用页表
krnl_pg_tbl_t

//堆
heap_t

//页面对象
page_obj_t
```
######接口函数及宏
```c
//页面属性
#define	PAGE_AVAIL				0x00000001

#define PAGE_READABLE			0x00000002
#define PAGE_WRITABLE			0x00000004
#define PAGE_EXECUTABLE			0x00000008

#define PAGE_COPY_ON_WRITE		0x00000010
#define PAGE_ALLOC_ON_ACCESS	0x00000020

#define PAGE_SWAPPABLE			0x00000040
#define PAGE_SWAPPED			0x00000080

#define	PAGE_DMA				0x00000100

#define	PAGE_KERNEL				0x80000000

//堆属性
#define HEAP_MULITHREAD			0x00000001
#define HEAP_PREALLOC			0x00000002

//初始化
void core_mm_init();

//初始化处理器核心
void core_mm_core_init(u32 cpuid);

//释放处理器核心
void core_mm_core_release(u32 cpuid);

//切换页表
void core_mm_switch_to(u32 index);

//获得当前页表
u32 core_mm_get_current_pg_tbl_index();

//fork
u32 core_mm_pg_tbl_fork();

//释放当前页表
void core_mm_pg_tbl_release(u32 index);

//page_obj_t
//创建页对象
ppage_obj_t page_obj(u32 attribute, size_t size, address_t align);

//映射页对象
void* page_obj.map(void* base_addr, bool kernel_mem);

//取消页对象映射
kstatus_t page_obj.unmap(void* addr);

//heap
//创建堆
pheap_t core_mm_heap_create(
	u32 attribute,
    size_t scale);
    
pheap_t core_mm_heap_create_on_buf(
	u32 attribute,
    size_t scale,
    void* init_buf,
    size_t init_buf_size);

//从堆里分配内存
void* core_mm_heap_alloc(
	size_t size,
    pheap_t heap);

//释放内存
void core_mm_heap_free(
	void* p_mem,
    pheap_t heap);

//销毁堆
void core_mm_heap_destroy(
	size_t size,
    pheap_t heap;
    
//检查堆溢出
void core_mm_heap_chk(pheap_t heap);

#define HEAP_CHECK(heap)
```
######文件列表
```c
src/sandnix/kernel/core/mm/mm.h
src/sandnix/kernel/core/mm/mm.c
src/sandnix/kernel/core/mm/heap/heap.h
src/sandnix/kernel/core/mm/heap/heap.c
src/sandnix/kernel/core/mm/paging/paging.h
src/sandnix/kernel/core/mm/paging/paging.c
src/sandnix/kernel/core/mm/paging/page_table.h
src/sandnix/kernel/core/mm/paging/page_table.c
```
#####kconsole
内核控制台
######模块路径
```
src/sandnix/kernel/core/kconsole
```
######接口数据结构
```c

```
######接口函数及宏
```c
//输出信息级别
//该级别的消息会导致内核崩溃
#define PRINT_LEVEL_PANIC	0x00000001

//发生错误
#define PRINT_LEVEL_ERR		0x00000002

//警告
#define PRINT_LEVEL_WARNING	0x00000003

//提示信息
#define PRINT_LEVEL_INFO	0x00000004

//调试信息
#define PRINT_LEVEL_DEBUG	0x00000005

#define core_kconsole_print_panic(fmt, ...);
#define core_kconsole_print_err(fmt, ...);
#define core_kconsole_print_warning(fmt, ...);
#define core_kconsole_print_info(fmt, ...);
#define core_kconsole_print_debug(fmt, ...);

//初始化
void core_kconsole_init();

//打印内核信息
void core_kconsole_kprint(u32 level,char* fmt, ...);

//设置输出终端,NULL输出到early_print
kstatus_t core_kconsole_set_output(char* output_device);
```
######文件列表
```c
src/sandnix/kernel/core/kconsole/kconsole.h
src/sandnix/kernel/core/kconsole/kconsole.c
src/sandnix/kernel/core/kconsole/kmsgd/kmsgd.h
src/sandnix/kernel/core/kconsole/kmsgd/kmsgd.c
```
#####pm
进程管理
execve是由subsystem层负责实现的,毕竟不同的子系统可执行文件格式不同.
######模块路径
```
src/sandnix/kernel/core/pm
```
######接口数据结构
```c
//进程对象
process_obj_t

//线程对象
thread_obj_t

//线程函数
//void thread_func(u32 thread_id, void* p_arg);
void (*thread_func_t)(u32, void*);

//锁
//spinlock
spnlck_t
spnlck_rw_t
spnlck_rcu_t

//mutex
mutex_t
mutex_rw_t
mutex_rcu_t

//cond
cond_t

//semaphore
semaphore_t
```
######接口函数及宏
```c
#define PRIORITY_HIGHEST		0x000000FF
#define PRIORITY_LOWEST			0x00000000

#define PRIORITY_IDLE			0x00000000
#define PRIORITY_USER_NORMAL	0x00000014
#define PRIORITY_USER_HIGHEST	0x00000028

#define	PRIORITY_KRNL_NORMAL	0x00000030
#define PRIORITY_DISPATCH		0x00000040
#define PRIORITY_IRQ			0x00000050
#define	PRIORITY_EXCEPTION		0x000000FF

#define PROCESS_ALIVE			0x00000000
#define PROCESS_ZOMBIE			0x00000001

#define TASK_RUNNING			0x00000000
#define TASK_READY				0x00000001
#define TASK_SUSPEND			0x00000002
#define TASK_SLEEP				0x00000003
#define TASK_ZOMBIE				0x00000004

//初始化
void core_pm_init();

//初始化处理器核心
void core_pm_core_init(int cpuid);

//释放处理器核心
void core_pm_core_release(int cpuid);

//Process
//fork
u32 core_pm_fork(void* child_start_address);

//获得当前进程id
u32 core_pm_get_crrnt_proc_id();

//wait
u32 core_pm_wait(bool wait_pid, u32 process_id);

//获得进程子系统
u32 core_pm_get_subsys(u32 pid);

//设置进程子系统
kstatus_t core_pm_set_subsys(u32 pid, u32 subsys_id);

//获得进程uid
u32 core_pm_get_uid(u32 pid);

//获得进程gid
u32 core_pm_get_gid(u32 pid);

//获得进程euid
u32 core_pm_get_euid(u32 pid);

//设置进程euid
kstatus_t core_pm_set_euid(u32 pid, u32 euid);

//获得进程egid
u32 core_pm_get_egid(u32 pid);

//设置进程egid
kstatus_t core_pm_set_egid(u32 pid, u32 egid);

//Thread
//创建线程
u32 core_pm_thread_create(thread_func_t thread_func, void* p_arg);

//结束线程
void core_pm_exit(u32 exit_code);

//join
u32 core_pm_join(bool wait_threadid, u32 thread_id);

//暂停线程
void core_pm_suspend(u32 thread_id);

//睡眠线程
void core_pm_resume(u32 thread_id);

//获得当前线程id
u32 core_pm_get_crrnt_thread_id();

//获得线程优先级
u32 core_pm_get_thrd_priority(u32 thrd_id);

//设置线程优先级
void core_pm_set_thrd_priority(u32 thrd_id, u32 priority);

//调度
void core_pm_schedule();

//设置errno
void core_pm_set_errno(kstatus_t errno);

//获得errno
kstatus_t core_pm_get_errno();

//spinlock
//normal
void core_pm_spnlck_init(pspnlck_t p_lock);
void core_pm_spnlck_lock(pspnlck_t p_lock);
void core_pm_spnlck_raw_lock(pspnlck_t p_lock);
kstataus_t core_pm_spnlck_trylock(pspnlck_t p_lock);
kstataus_t core_pm_spnlck_raw_trylock(pspnlck_t p_lock);
void core_pm_spnlck_unlock(pspnlck_t p_lock);
void core_pm_spnlck_raw_unlock(pspnlck_t p_lock);

//r/w lock
void core_pm_spnlck_rw_init(pspnlck_rw_t p_lock);
void core_pm_spnlck_rw_r_lock(pspnlck_rw_t p_lock);
kstataus_t core_pm_spnlck_rw_r_trylock(pspnlck_rw_t p_lock);
void core_pm_spnlck_rw_r_unlock(pspnlck_rw_t p_lock);
void core_pm_spnlck_rw_w_lock(pspnlck_rw_t p_lock);
kstataus_t core_pm_spnlck_rw_w_trylock(pspnlck_rw_t p_lock);
void core_pm_spnlck_rw_w_unlock(pspnlck_rw_t p_lock);

//rcu
void core_pm_spnlck_rcu_init(pspnlck_rcu_t p_lock);
void core_pm_spnlck_rcu_r_lock(pspnlck_rcu_t p_lock);
kstataus_t core_pm_spnlck_rcu_r_trylock(pspnlck_rcu_t p_lock);
void core_pm_spnlck_rcu_r_unlock(pspnlck_rcu_t p_lock);
void core_pm_spnlck_rcu_w_lock(pspnlck_rcu_t p_lock);
kstataus_t core_pm_spnlck_rcu_w_trylock(pspnlck_rcu_t p_lock);
void core_pm_spnlck_rcu_w_sync(pspnlck_rcu_t p_lock);
void core_pm_spnlck_rcu_w_unlock(pspnlck_rcu_t p_lock);

//timeout为0则不会超时
//mutex
//normal
void core_pm_mutex_init(pmutex_t p_mutex);
void core_pm_mutex_lock(pmutex_t p_mutex, u32 timeout);
kstataus_t core_pm_mutex_trylock(pmutex_t p_mutex);
void core_pm_mutex_unlock(pmutex_t p_mutex);
void core_pm_mutex_destroy(pmutex_t p_mutex);

//r/w lock
void core_pm_mutex_rw_init(pmutex_rw_t p_mutex);
void core_pm_mutex_rw_r_lock(pmutex_rw_t p_mutex, u32 timeout);
kstatus_t core_pm_mutex_rw_r_trylock(pmutex_rw_t p_mutex);
void core_pm_mutex_rw_r_unlock(pmutex_rw_t p_mutex);
void core_pm_mutex_rw_w_lock(pmutex_rw_t p_mutex, u32 timeout);
kstatus_t core_pm_mutex_rw_w_trylock(pmutex_rw_t p_mutex);
void core_pm_mutex_rw_w_unlock(pmutex_rw_t p_mutex);
void core_pm_mutex_rw_release(pmutex_rw_t p_mutex);

//rcu
void core_pm_mutex_rcu_init(pmutex_rcu_t p_mutex);
void core_pm_mutex_rcu_r_lock(pmutex_rcu_t p_mutex, u32 timeout);
kstatus_t core_pm_mutex_rcu_r_trylock(pmutex_rcu_t p_mutex);
void core_pm_mutex_rcu_r_unlock(pmutex_rcu_t p_mutex);
void core_pm_mutex_rcu_w_lock(pmutex_rcu_t p_mutex, u32 timeout);
kstatus_t core_pm_mutex_rcu_w_trylock(pmutex_rcu_t p_mutex);
void core_pm_mutex_rcu_w_sync(pmutex_rcu_t p_mutex);
void core_pm_mutex_rcu_w_unlock(pmutex_rcu_t p_mutex);
void core_pm_mutex_rcu_release(pmutex_rcu_t p_mutex);

//cond
void core_pm_cond_init(pcond_t p_cond);
kstatus_t core_pm_cond_wait(pcond_t p_cond, u32 timeout);
kstatus_t core_pm_cond_trywait(pcond_t p_cond);
kstatus_t core_pm_cond_signal(pcond_t p_cond);
kstatus_t core_pm_cond_broadcast(pcond_t p_cond);
kstatus_t core_pm_cond_release(pcond_t p_cond);

//semaphore
void core_pm_semphore_init(psemphore_t p_sem);
void core_pm_semphore_acquire(psemphore_t p_sem);
void core_pm_semphore_tryacquire(psemphore_t p_sem);
void core_pm_semphore_release(psemphore_t p_sem);
void core_pm_semphore_destroy(psemphore_t p_sem);
```
######文件列表
```c
src/sandnix/kernel/core/pm/pm.h
src/sandnix/kernel/core/pm/pm.c

src/sandnix/kernel/core/pm/process/process.h
src/sandnix/kernel/core/pm/process/process.c

src/sandnix/kernel/core/pm/thread/thread.h
src/sandnix/kernel/core/pm/thread/thread.c
src/sandnix/kernel/core/pm/thread/schedule.c

src/sandnix/kernel/core/pm/lock/spinlock/spnlck.h
src/sandnix/kernel/core/pm/lock/spinlock/spnlck.c
src/sandnix/kernel/core/pm/lock/spinlock/spnlck_rw.h
src/sandnix/kernel/core/pm/lock/spinlock/spnlck_rw.c
src/sandnix/kernel/core/pm/lock/spinlock/spnlck_rcu.h
src/sandnix/kernel/core/pm/lock/spinlock/spnlck_rcu.c

src/sandnix/kernel/core/pm/lock/mutex/mutex.h
src/sandnix/kernel/core/pm/lock/mutex/mutex.c
src/sandnix/kernel/core/pm/lock/mutex/mutex_rw.h
src/sandnix/kernel/core/pm/lock/mutex/mutex_rw.c
src/sandnix/kernel/core/pm/lock/mutex/mutex_rcu.h
src/sandnix/kernel/core/pm/lock/mutex/mutex_rcu.c

src/sandnix/kernel/core/pm/lock/cond/cond.h
src/sandnix/kernel/core/pm/lock/cond/cond.c

src/sandnix/kernel/core/pm/lock/semaphore/semaphore.h
src/sandnix/kernel/core/pm/lock/semaphore/semaphore.c
```
#####ipc
POSIX进程通信
######模块路径
```
src/sandnix/kernel/core/ipc
```
######接口数据结构
```c
//signal处理函数
//void signal_hndlr_func(u32 signal);
void (*signal_hndlr_func_t)(u32);
```
######接口函数及宏
```c
void core_ipc_init();

void core_ipc_kill(u32 thread_id, u32 signal);

//hndlr为NULL则取消注册
signal_hndlr_func_t core_ipc_reg_signal_hndlr(
	u32 signal,
    signal_hndlr_func_t hndlr);
```
######文件列表
```c
src/sandnix/kernel/core/ipc/ipc.h
src/sandnix/kernel/core/ipc/ipc.c
```
#####msg
内核消息机制
######模块路径
```
src/sandnix/kernel/core/msg
```
######接口数据结构
```c
//消息队列
msg_queue_t

//消息
msg_t

msg_finish_t
msg_open_t
msg_read_t
msg_write_t
msg_close_t
msg_stat_t
msg_fcntl_t
msg_link_t
msg_unlink_t
msg_chmod_t
msg_chown_t
msg_mkdir_t
msg_access_t
msg_mknod_t
msg_ioctl_t
msg_mount_t
msg_umount_t

msg_match_t
msg_hot_plug_t

//消息状态
mstatus_t
```
######接口函数及宏
```c
//消息状态
#define MSG_STATUS_SUCCESS		0x00000000
#define MSG_STATUS_FAILED		0x00000001
#define MSG_STATUS_PENDING		0x00000002
#define MSG_STATUS_CANCEL		0x00000003

//消息类型
//主类型
//异步操作
#define	MSG_MJ_FINISH		0x00000000

//文件操作
#define MSG_MJ_OPEN			0x00010000
#define MSG_MJ_READ			0x00010001
#define MSG_MJ_WRITE		0x00010002
#define MSG_MJ_TRUNCATE		0x00010003
#define MSG_MJ_CLOSE		0x00010004
#define MSG_MJ_STAT			0x00010005
#define MSG_MJ_FCNTL		0x00010006
#define MSG_MJ_LINK			0x00010007
#define MSG_MJ_CHMOD		0x00010008
#define MSG_MJ_CHOWN		0x00010009
#define MSG_MJ_MKDIR		0x0001000A
#define MSG_MJ_ACCESS		0x0001000B
#define MSG_MJ_MKNOD		0x0001000C
#define MSG_MJ_IOCTL		0x0001000D
#define MSG_MJ_NOTIFY		0x0001000E
#define MSG_MJ_MOUNT		0x0001000F

//设备操作
#define MSG_MJ_MATCH			0x00020000
#define MSG_MJ_HOT_PLUG			0x00020001
#define MSG_MJ_POWER			0x00020002

//次类型
//MSG_MJ_FINISH
#define MSG_MN_COMPLETE			0x00000000
#define MSG_MN_CANCEL			0x00000001

//MSG_MJ_OPEN
#define MSG_MN_OPEN				0x00000000

//MSG_MJ_READ
#define MSG_MN_READ				0x00000000

//MSG_MJ_WRITE
#define MSG_MN_WRITE			0x00000000

//MSG_MJ_TRUNCATE
#define MSG_MN_TRUNCATE			0x00000000

//MSG_MJ_CLOSE
#define MSG_MN_CLOSE			0x00000000
#define MSG_MN_CLEANUP			0x00000001

//MSG_MJ_STAT
#define MSG_MN_STAT				0x00000000

//MSG_MJ_FCNTL
#define MSG_MN_FCNTL			0x00000000

//MSG_MJ_LINK
#define MSG_MN_SYMBOL_LINK		0x00000000
#define MSG_MN_HARD_LINK		0x00000001
#define MSG_MN_UNLINK			0x00000002

//MSG_MJ_CHMOD
#define MSG_MN_CHMOD			0x00000000

//MSG_MJ_CHOWN
#define MSG_MN_CHOWN			0x00000000

//MSG_MJ_MKDIR
#define MSG_MN_MKDIR			0x00000000

//MSG_MJ_ACCESS
#define MSG_MN_ACCESS			0x00000000

//MSG_MJ_MKNOD
#define MSG_MN_MKNOD			0x00000000

//MSG_MJ_IOCTL
#define MSG_MN_IOCTL			0x00000000

//MSG_MJ_NOTIFY
#define MSG_MN_NOTIFY			0x00000000
#define MSG_MN_NOTIFY_ADD		0x00000001
#define MSG_MN_NOTIFY_REMOVE	0x00000002

//MSG_MJ_MOUNT
#define MSG_MN_MOUNT			0x00000000
#define MSG_MN_UMOUNT			0x00000001

//MSG_MJ_MATCH
#define MSG_MN_MATCH			0x00000000
#define MSG_MN_UNMATCH			0x00000001

//MSG_MJ_HOT_PLUG
#define MSG_MN_PLUGIN			0x00000000
#define MSG_MN_PLUGOFF			0x00000001

//MSG_MJ_POWER
#define MSG_MN_POWEROFF			0x00000000
#define MSG_MN_SUSPEND			0x00000001
#define MSG_MN_RESUME			0x00000002
#define MSG_MN_HIBERNATE		0x00000003

//初始化
u32 core_msg_init();

//消息队列
//创建消息队列
u32 core_msg_queue_create();

//销毁消息队列
void core_msg_queue_destroy(u32 queue_id);

//消息
//创建
pmsg_t core_msg_create(size_t size);

//发送
mstatus_t core_msg_send(u32 queue, pmsg_t p_msg);

//接收
pmsg_t core_msg_recv(u32 queue, bool block);

//完成
void core_msg_complete(pmsg_t p_msg, bool succees);

//取消
void core_msg_cancel(pmsg_t p_msg);
```
######文件列表
```c
src/sandnix/kernel/core/msg/msg.h
src/sandnix/kernel/core/msg/messages.h
src/sandnix/kernel/core/msg/msg.c
```
#####vfs
虚拟文件系统以及初始化内存盘的驱动
对象的管理,设备的管理
设备驱动在加载的时候创建一个probe设备用来处理MSG_MATCH,match成功后接管具体的设备
每个设备对象会被sysfs以设备文件的形式呈现,sysfs是设备管理器对外的接口,类似于块设备一类的东西
像总线一类的有多个字节点的设备会以一个文件夹的形式呈现,文件夹里面会创建各个节点的设备文件和总线的控制设备文件.
设备分两大类,总线设备和普通设备,普通设备包括字符设备,块设备等
总线设备可以分两类,一类是芯片组驱动,负责和具体的端口,IRQ,DMA什么的打交道.另一类是像usb, i2c, ATA这类的驱动,负责以文件操作的方式通过芯片组驱动和具体的总线控制器打交道.
硬件中断的时候,IRQ会通知内核,内核给总线驱动发送signal,总线负责一层一层的将signal传递下去,直到通知到具体的设备.如果是设备插入的信号,发生插入事件的总线会为该设备创建节点,并且向所有的probe设备发送MSG_MATCH广播,如果有probe设备处理了该消息,则让创建probe设备的驱动锁定该节点.
n当有新的probe设备被注册的时候,设备管理器会向该设备发送MSG_MATCH来尝试匹配所有未匹配的设备.
从设备号0xFFFF为广播地址,主设备号0xFFFF为广播地址
######模块路径
```
src/sandnix/kernel/core/vfs
```
######接口数据结构
```c
file_obj_t
proc_ref_obj_t
file_state_obj_t

stat_t
pollfd_t
```
######接口函数及宏
```c
//Open flags
#define	O_RDONLY	0x00000000
#define	O_WRONLY	0x00000001
#define	O_RDWR		0x00000002

#define	O_CREAT		0x00000040
#define	O_EXCL		0x00000080
#define	O_NOCTTY	0x00000100
#define	O_TRUNC		0x00000200	//Not support
#define	O_APPEND	0x00000400
#define	O_NONBLOCK	0x00000800	//Not support
#define	O_NDELAY	O_NONBLOCK
#define	O_DSYNC		0x00001000	//Not support
#define	O_ASYNC		0x00002000	//Not support
#define	O_DIRECTORY	0x00010000
#define	O_NOFOLLOW	0x00020000
#define	O_CLOEXEC	0x00080000
#define	O_RSYNC		0x00101000	//Not support
#define	O_SYNC		0x00101000	//Not support

//Modes
#define	S_ISUID	0x00000800		//Set user ID
#define	S_ISGID	0x00000400		//Set group ID

#define	S_ISVTX	0x00000200		//Sticky bit

#define	S_IRWXU	0x000001c0		//Owner has read,write&execute permissions
#define	S_IRUSR	0x00000100		//Owner has read permission
#define	S_IWUSR	0x00000080		//Owner has write permission
#define	S_IXUSR	0x00000040		//Owner has execute permission

#define	S_IRWXG	0x00000038		//Group has read,write&execute permissions
#define	S_IRGRP	0x00000020		//Group has read permission
#define	S_IWGRP	0x00000010		//Group has write permission
#define	S_IXGRP	0x00000008		//Group has execute permission

#define	S_IRWXO	0x00000007		//Others has read,write&execute permissions
#define	S_IROTH	0x00000004		//Others has read permission
#define	S_IWOTH	0x00000002		//Others has write permission
#define	S_IXOTH	0x00000001		//Others has execute permission

//Access modes
#define	F_OK	0x00000000		//Exist
#define	X_OK	0x00000001		//Execute
#define	W_OK	0x00000002		//Write
#define	R_OK	0x00000004		//Read

//Seek
#define	SEEK_SET	0x00000000
#define	SEEK_CUR	0x00000001
#define	SEEK_END	0x00000002

#define	NAME_MAX	255
#define	PATH_MAX	2048

//文件系统
//初始化
void vfs_init();

//打开/创建文件
u32 core_vfs_open(pkstring_obj_t obj_t path, u32 flags, u32 mode);

//读文件
size_t core_vfs_read(u32 fd,u8* buf, size_t size);

//写文件
size_t core_vfs_write(u32 fd, u8* buf, size_t size);

//截断文件
kstatus_t core_vfs_truncate(u32 fd);

//移动文件指针
off_t core_vfs_lseek(u32 fd, ssize_t offset, u32 whence);

//等待文件状态
u32 core_vfs_poll(pollfd_t p_polls, size_t num, u32 timeout);

//fnctl
kstatus_t core_vfs_fcntl(u32 fd, int cmd, void* p_arg, size_t arg_size);

//ioctl
kstatus_t core_vfs_ioctl(u32 fd,u32 request, void* p_arg, size_t arg_size);

//设置文件权限
kstatus_t core_vfs_chmod(pkstring_obj_t path, u32 mode);

//设置文件所有者
kstatus_t core_vfs_chown((pkstring_obj_t path, u32 owner, u32 group);

//建立硬链接
kstatus_t core_vfs_link(pkstring_obj_t p_old_path, pkstring_obj_t p_new_path);

//建立符号链接
kstatus_t core_vfs_symlink(pkstring_obj_t p_target, pkstring_obj_t p_link;

//删除
kstatus_t core_vfs_unlink(pkstring_t path);

//建立设备节点
kstatus_t core_vfs_mknod(pkstring_obj_t path, u32 mod, u32 dev);

//获得文件属性
kstatus_t core_vfs_stat(pkstring_obj_t path, pstat_t p_stat);

//检查访问权限
kstatus_t core_vfs_access(pkstring_obj_t path, u32 mode);

//内存映射文件
kstatus_t core_vfs_mmap(u32 fd, off_t offset, ppage_obj_t p_page_obj);

//释放文件映射
kstatus_t core_vfs_munmap(ppage_obj_t p_page_obj);

//挂载块设备
kstatus_t core_vfs_mount(
	pkstring_obj_t src,
    pkstring_obj_t target,
    pkstring_obj_t fstype,
    u32 flags,
    pkstring_obj_t data);

//取消挂载
kstatus_t core_vfs_umount(
	pkstring_obj_t target);

//对象管理器
//进程引用对象管理
//复制当前进程对象管理器
u32 core_vfs_objmgr_fork();

//添加对象引用
u32 core_vfs_objmgr_add(u32 proc_id, proc_ref_obj_t p_object);

//获得对象
proc_ref_obj_t core_vfs_objmgr_add(u32 fd);

//释放对象引用
u32 core_vfs_objmgr_remove(u32 proc_id, proc_ref_obj_t p_object);

//清空进程对象管理器,为execve准备
kstatus_t core_vfs_objmgr_clear(u32 om_id);

//释放进程对象管理器
kstatus_t core_vfs_objmgr_release(u32 om_id);

//设备对象管理
//获得主设备号
//添加设备
u32 core_vfs_dev_add(pkstring_t class_name,u32 parent_dev);

//删除设备
kstatuc_t core_vfs_dev_remove(u32 dev_num);

//给设备发消息
mstatus_t core_vfs_dev_msg_send(u32 dest, pmsg_t p_msg);

//接收消息
pmsg_t core_vfs_dev_msg_recv(u32 dev_num);

```
######文件列表
```c
src/sandnix/kernel/core/vfs/vfs.h
src/sandnix/kernel/core/vfs/styles.h
src/sandnix/kernel/core/vfs/vfs.c

src/sandnix/kernel/core/vfs/objmgr/objmgr.h
src/sandnix/kernel/core/vfs/objmgr/objmgr.c

src/sandnix/kernel/core/vfs/objmgr/proc/proc.h
src/sandnix/kernel/core/vfs/objmgr/proc/proc.c

src/sandnix/kernel/core/vfs/objmgr/proc/procfs/procfs.h
src/sandnix/kernel/core/vfs/objmgr/proc/procfs/procfs.c

src/sandnix/kernel/core/vfs/objmgr/drvmgr/drvmgr.h
src/sandnix/kernel/core/vfs/objmgr/drvmgr/drvmgr.c

src/sandnix/kernel/core/vfs/objmgr/devmgr/devmgr.h
src/sandnix/kernel/core/vfs/objmgr/devmgr/devmgr.c

src/sandnix/kernel/core/vfs/objmgr/devmgr/sysfs/sysfs.h
src/sandnix/kernel/core/vfs/objmgr/devmgr/sysfs/sysfs.c

src/sandnix/kernel/core/vfs/initramfs/initramfs.h
src/sandnix/kernel/core/vfs/initramfs/initramfs.c
```
#####rtl
c运行库,面向对象以及各种数据结构
######模块路径
```
src/sandnix/kernel/core/rtl
```
######接口数据结构
```c
//可变参
va_list

//面向对象
//所有对象的祖宗
obj_t

//字符串对象
kstring_obj_t

//伪随机数发生器
random_obj_t

//数据结构
//链表
list_t

//动态数组
array_t

//哈希表
hash_table_t

//映射(红黑树)
map_t

//向量
vector_t

//队列
queue_t

//缓冲区
buffer_t

//回调函数类型
//销毁对象
//void item_destroyer(void* p_item, void* p_arg)
typedef void (*item_destroyer_t)(void*, void*);

//比较大小.item1 > item2返回值 > 0
//等于返回0
//小于返回值 < 0
//int item_compare(void* p_item1, void* p_item2);
typedef int (*item_compare_t)(void*, void*);

//哈希函数
//u32 hash_func(void* p_item)
typedef u32 (*hash_func_t)(void*)
```
######接口函数及宏
```c
//可变参
#define va_start
#define va_arg
#define va_end

//c运行库
//字符串处理函数,作用参照标准库
void* core_rtl_memccpy(void* dest, const void* src, u8 ch, size_t size);
void* core_rtl_memchr(const void* buf, u8 ch, size_t size);
int core_rtl_memcmp(const void* buf1, const void* buf2, size_t size);
void* core_rtl_memcpy(void* dest, const void* src, size_t size);
void* core_rtl_memmove(void* dest, const void* src, size_t size);
void* core_rtl_memset(void* dest, u8 value, size_t size);
char* core_rtl_strchr(const char* str, char c);
size_t core_rtl_strcspn(const char* str, const char* reject);
size_t core_rtl_strlen(const char* str);
char* core_rtl_strncat(char *dest, const char *src, size_t len);
int core_rtl_strncmp(const char* dest, const char* src, size_t len);
char* core_rtl_strncpy(char* dest, const char* src, size_t len);
char* core_rtl_strpbrk(const char* str1, const char* str2);
char* core_rtl_strrchr(const char* str, char ch);
size_t core_rtl_strspn(const char* str, const char* accept);
char* core_rtl_strstr(const char* str1, const char* str2);

//分割字符串
char* core_rtl_strsplit(const char *str, const char *delim, char* buf, size_t size);

//格式化字符串函数
char* core_rtl_snprintf(char* buf, size_t size, const char* fmt, ...);
char* core_rtl_vsnprintf(char* buf, size_t size, const char* fmt, va_list ap);

//格式化输出函数
char* core_rtl_kprintf(const char* fmt, ...);

//数学函数
long core_rtl_abs(long num);
double core_rtl_fabs(double);
double core_rtl_pow(double);
double core_rtl_sqrt(souble num);
s64 core_rtl_cell(double num);
s64 core_rtl_floor(double num);
s64 core_rtl_div(s64 divdend, s64 divisor);
s64 core_rtl_mod(s64 divdend, s64 divisor);

//随机数
//真随机数,熵池空则阻塞
void core_rtl_rand(u8* buf, size_t len);

//伪随机数
//random_obj_t
//构造函数
prandom_obj_t random_obj(u32 seed, pheap_t heap);

//获得伪随机数
void random_obj.rand(u8* buf, size_t len);

//安全校验
//md5
size_t core_rtl_md5sum(
	void* p_output,		//输出缓冲区
    size_t size,		//输出缓冲区大小
    void* p_data,		//数据
    size_t data_size);	//数据大小

//sha1
size_t core_rtl_sha1sum(
	void* p_output,		//输出缓冲区
    size_t size,		//输出缓冲区大小
    void* p_data,		//数据
    size_t data_size);	//数据大小

//sha224
size_t core_rtl_sha224sum(
	void* p_output,		//输出缓冲区
    size_t size,		//输出缓冲区大小
    void* p_data,		//数据
    size_t data_size);	//数据大小

//sha256
size_t core_rtl_sha256sum(
	void* p_output,		//输出缓冲区
    size_t size,		//输出缓冲区大小
    void* p_data,		//数据
    size_t data_size);	//数据大小

//sha384
size_t core_rtl_sha384sum(
	void* p_output,		//输出缓冲区
    size_t size,		//输出缓冲区大小
    void* p_data,		//数据
    size_t data_size);	//数据大小

//sha512
size_t core_rtl_sha512sum(
	void* p_output,		//输出缓冲区
    size_t size,		//输出缓冲区大小
    void* p_data,		//数据
    size_t data_size);	//数据大小

//数据结构
//链表
//初始化
#define core_rtl_list_init(p_list)

//检测是否为空
#define core_rtl_list_empty(p_list)

//在后面插入
plist_node_t core_rtl_list_insert_before(
	plist_node_t pos,	//位置
    plist_t p_list,		//链表地址
    void* p_item,		//元素
	pheap_t heap);		//堆

//在前面插入,参数同上
plist_node_t core_rtl_list_insert_after(
	plist_node_t pos,
    plist_t p_list,
    void* p_item,
	pheap_t heap);

//删除
void* core_rtl_list_remove(
	plist_node_t pos,	//位置
    plist_t p_list,		//链表地址
	pheap_t heap);		//堆

//销毁
void core_rtl_list_destroy(
	plist_t p_list,				//链表
    pheap_t heap,				//堆
    item_destroyer_t destroier,	//销毁元素用的回调函数
    void* arg);					//回调函数额外参数

//合并
void core_rtl_list_join(
	plist_t p_src,		//原链表
    plist_t p_dest,		//目的链表
	pheap_t src_heap,	//原链表所在堆
    pheap_t dest_heap);	//目的链表所在堆

//获得上一项的位置
#define core_rtl_list_prev(pos, p_list)

//获得下一项的位置
#define core_rtl_list_next(pos, p_list)

//得到链表项
#define core_rtl_list_get(pos)

//快速排序
void core_rtl_list_qsort(
	plist_t p_list,			//链表
    item_compare_t compare,	//比较大小用的回调函数
    bool b2s);				//true从大到小,false则反之

//动态数组
//初始化
kstatus_t core_rtl_array_init(
	parray_t p_array,		//数组地址
    u32 num,				//大小上限
	pheap_t heap);			//堆

//获得元素
void* core_rtl_array_get(
	parray_t p_array,		//数组地址
    u32 index);				//下标

//设置n元素值,设成NULL即remove
void* core_rtl_array_set(
	parray_t p_array,		//数组地址
    u32 index,				//下标
    void* value);			//值

//检测该元素是否被赋值
bool core_rtl_array_used(
	parray_t p_array,		//数组地址
    u32 index);				//下标

//获得当前元素个数
u32 core_rtl_array_size(
	parray_t p_array);		//数组地址

//获得索引
bool core_rtl_array_get_used_index(
    parray_t p_array,		//数组地址
    u32 begin,				//起始索引
    u32* ret);				//返回的下一个索引

//获得一个空闲的索引
u32 core_rtl_array_get_free_index(
	parray_t p_array);		//数组地址

//获得空闲索引个数
u32 core_rtl_array_get_free_index_num(
	parray_t p_array);		//数组地址

//销毁
void core_rtl_array_destroy(
	parray_t p_array,			//数组地址
    item_destroyer_t destroier,	//销毁元素回调
    void* arg);					//回调函数参数

//哈希表
//初始化
void core_rtl_hash_table_init(
	phash_table_t p_hash_table,	//哈希表地址
    u32 min_hash,				//最小哈希值
    u32 max_hash,				//最大哈希值
    hash_func_t hash_func,		//哈希函数
    pheap_t heap);				//堆

//获得键值
void* core_rtl_hash_table_get(
	phash_table_t p_hash_table,
    void* p_key)

//设置键值,设成NULL即remove
void* core_rtl_hash_table_set(
	phash_table_t p_hash_table,
    void* p_key,
    void* p_value);

//获得上一个键值
void* core_rtl_hash_table_prev(
	phash_table_t p_hash_table,
    void* p_key);
   
//获得下一个键值
void* core_rtl_hash_table_next(
	phash_table_t p_hash_table,
    void* p_key);

//销毁
void core_rtl_hash_table_destroy(
	phash_table_t p_hash_table,
    item_destroyer_t destroier,
    void* arg);

//映射
//初始化
void core_rtl_map_init(
	pmap_t p_map,
    item_compare_t compare_func,
    pheap_t heap);

//设置键值,设成NULL即remove
void* core_rtl_map_set(
	pmap_t p_map,
	void* p_key,
    void* p_value);

//获得键值
void* core_rtl_map_get(
	pmap_t p_map,
	void* p_key);

//获得下一个键值
void* core_rtl_map_next(
	pmap_t p_map,
    void* p_key);
    
//搜索键值
typedef int (*map_search_func_t)(void* p_condition, void* p_key, void* p_value);
void* core_rtl_map_search(
	pmap_t p_map,
    void* p_condition,
    map_search_func_t search_func);

//销毁
void core_rtl_map_destroy(
	pmap_t p_map,
    item_destroyer_t key_destroier,
    item_destroyer_t value_destroier,
    void* arg);

//向量
//初始化
void core_rtl_vector_init(
	pvector_t p_vector,
    size_t scale,
	pheap_t p_heap);

//压入元素
void core_rtl_vector_push(
	pvector_t p_vector,
    void* p_item);

//弹出元素
void* core_rtl_vector_pop(
	pvector_t p_vector);

//获得顶部元素
void* core_rtl_vector_top(
	pvector_t p_vector);

//获得元素
void* core_rtl_vector_get(
	pvector_t p_vector,
    u32 index);

//销毁
void* core_rtl_vector_destroy(
	pvector_t p_vector,
    item_destroyer_t destroier,
    void* arg);

//队列
//初始化
void core_rtl_queue_init(
	pqueue_t p_queue,
    pheap_t heap);

//压入元素
void core_rtl_queue_push(
	pqueue_t p_queue,
    void* p_item);

//弹出元素
void* core_rtl_queue_pop(
	pqueue_t p_queue);

//获得顶部元素
void* core_rtl_queue_front(
	pqueue_t p_queue);

//获得底部元素
void* core_rtl_queue_end(
	pqueue_t p_queue);

//获得元素
void* core_rtl_queue_get(
		pqueue_t p_queue,
        u32 index);
        
//删除元素
void core_rtl_queue_remove(
	pqueue_t p_queue,
    u32 index);

//缓冲区
//初始化
void core_rtl_buffer_init(
	pbuffer_t p_buffer,
    size_t size,
	void* buf);

//读
size_t core_rtl_buffer_read(
	pbuffer_t p_buffer,
    void* p_buf,
    size_t len_to_read,
    bool block);

//写
size_t core_rtl_buffer_write(
	pbuffer_t p_buffer,
    void* p_data,
    size_t len_to_write,
    bool block,
    bool overwrite);

//面向对象
//增加引用计数
#define INC_REF(p_obj)

//减少引用计数
#define DEC_REF(p_obj)

//格式化
#define TO_STRING(p_obj)

//class id
#define	CLASS_ID(p_obj)

//构造函数
void obj(pobj_t p_obj, u32 class_id, destructor_t destructor,
	compare_obj_t compare_func, to_string_t to_string_func,
    pheap_t heap);

//增加引用计数
void core_rtl_obj_inc_ref(pobj_t p_obj);

//减少引用计数
void core_rtl_obj_dec_ref(pobj_t p_obj);

//obj_t
//methods
//析构
void obj_t.destructor(pobj_t p_this);

//比较
int obj_t.compare(pobj_t p_this, pobj_t p_obj2);

//格式化
pkstring_obj_t obj_t.to_string(pobj_t p_this);

//kstring_obj_t
//methods
//构造函数
pkstring_obj_t kstring(char* str, pheap_t heap);

//获得长度
size_t kstring_obj.len(pkstring_obj_t p_this);

//复制
pkstring_obj_t kstring_obj.copy(pkstring_obj_t p_this, pheap_t heap);

//获得子串
pkstring_obj_t kstring_obj.substr(
	pkstring_obj_t p_this,
    u32 begin,			//起始
    u32 end);			//结束

//连接
kstatus_t kstring_obj.append(pkstring_obj_t p_this, pktring_obj_t p_str);

//所有字符大写
void kstring_obj.upper(pkstring_obj_t p_this);

//所有字符小写
void kstring_obj.lower(pkstring_obj_t p_this);

//搜索子串起始位置
kstatus_t kstring_obj.search(pkstring_obj_t p_this, pkstring_obj_t p_substr, u32* ret);
```
######文件列表
```c
src/sandnix/kernel/core/rtl/rtl.h
src/sandnix/kernel/core/rtl/rtl.c

src/sandnix/kernel/core/rtl/varg.h

src/sandnix/kernel/core/rtl/string/string.h
src/sandnix/kernel/core/rtl/string/string.c
src/sandnix/kernel/core/rtl/string/printf.c

src/sandnix/kernel/core/rtl/math/math.h
src/sandnix/kernel/core/rtl/math/math.c

src/sandnix/kernel/core/rtl/random/random.h
src/sandnix/kernel/core/rtl/random/random.c

src/sandnix/kernel/core/rtl/random/random_obj/random_obj.h
src/sandnix/kernel/core/rtl/random/random_obj/random_obj.c

src/sandnix/kernel/core/rtl/security/security.h
src/sandnix/kernel/core/rtl/security/security.c

src/sandnix/kernel/core/rtl/crypto/hash/md5sum.h
src/sandnix/kernel/core/rtl/crypto/hash/md5sum.c
src/sandnix/kernel/core/rtl/crypto/hash/sha1sum.h
src/sandnix/kernel/core/rtl/crypto/hash/sha1sum.c
src/sandnix/kernel/core/rtl/crypto/hash/sha224sum.h
src/sandnix/kernel/core/rtl/crypto/hash/sha224sum.c
src/sandnix/kernel/core/rtl/crypto/hash/sha256sum.h
src/sandnix/kernel/core/rtl/crypto/hash/sha256sum.c
src/sandnix/kernel/core/rtl/crypto/hash/sha384sum.h
src/sandnix/kernel/core/rtl/crypto/hash/sha384sum.c
src/sandnix/kernel/core/rtl/crypto/hash/sha512sum.h
src/sandnix/kernel/core/rtl/crypto/hash/sha512sum.c

src/sandnix/kernel/core/rtl/kstring/kstring.h
src/sandnix/kernel/core/rtl/kstring/kstring.c

src/sandnix/kernel/core/rtl/obj/obj.h
src/sandnix/kernel/core/rtl/obj/class_ids.h
src/sandnix/kernel/core/rtl/obj/obj.c

src/sandnix/kernel/core/rtl/container/list/list.h
src/sandnix/kernel/core/rtl/container/list/list.c

src/sandnix/kernel/core/rtl/container/array/array.h
src/sandnix/kernel/core/rtl/container/array/array.c

src/sandnix/kernel/core/rtl/container/hash_table/hash_table.h
src/sandnix/kernel/core/rtl/container/hash_table/hash_table.c

src/sandnix/kernel/core/rtl/container/map/map.h
src/sandnix/kernel/core/rtl/container/map/map.c

src/sandnix/kernel/core/rtl/container/vector/vector.h
src/sandnix/kernel/core/rtl/container/vector/vector.c

src/sandnix/kernel/core/rtl/container/queue/queue.h
src/sandnix/kernel/core/rtl/container/queue/queue.c

src/sandnix/kernel/core/rtl/container/buffer/buffer.h
src/sandnix/kernel/core/rtl/container/buffer/buffer.c
```
#####exception
异常处理
######模块路径
```
src/sandnix/kernel/core/exception
```
######接口数据结构
```c
//异常处理结果
except_stat_t

//异常处理回调
//except_stat_t except_hndlr_t(kstatus_t reason, pcontext_t context, void* p_arg);
except_stat_t (*except_hndlr_t)(kstatus_t, pcontext_t, void*);
```
######接口函数及宏
```c
#define EXCEPT_STATUS_FAILED		0x00000000
#define EXCEPT_STATUS_CONTINUE		0x00000001
#define EXCEPT_STATUS_PANIC			0x00000002

#define OPERATE_SUCCESS

//初始化模块
void core_exception_init();

//抛出异常
void core_exception_raise(
	kstatus_t reason,
    char* description,
    void* p_arg,
    char* src_file,
    char* line);

#define RAISE(reason, description, p_arg)

//压入错误处理函数
void core_exception_push_hndlr(except_hndlr_t hndlr);

//弹出最顶层错误处理函数
except_hndlr_t core_exception_pop_hndlr();
```
######文件列表
```c
src/sandnix/kernel/core/exception/exception.h
src/sandnix/kernel/core/exception/exception.c
```
####Subsystem层
#####common
subsystem层的一些公用的方法,包括跨权限缓冲区处理等调用
######模块路径
```
src/sandnix/kernel/subsystem/common
```
######接口数据结构
```c
//系统调用函数
//void* syscall(u32 arg_num, va_list arg_list);
void* (*syscall_t)(u32, va_list);

//子系统对象
subsys_obj_t
```
######接口函数及宏
```c
//初始化
void subsys_common_init();

//读用户空间内存
kstatus_t subsys_common_read_usr_mem(u8* dest, u8* src, size_t len);

//写用户空间内存
kstatus_t subsys_common_write_usr_mem(u8* dest, u8* src, size_t len);

//注册子系统
kstatus subsys_common_reg_subsys(psubsys_obj_t subsys);

//获得子系统id
u32 subsys_common_get_subsys_id(pkstring_obj_t name);

//获得系统调用地址
syscall_t subsys_common_get_syscall_addr(u32 subsys_id, u32 call_num);
```
######文件列表
```c
src/sandnix/kernel/subsystem/common/common.h
src/sandnix/kernel/subsystem/common/common.c
```
#####driver
为驱动程序提供系统调用的子系统
######模块路径
```
src/sandnix/kernel/subsystem/driver
```
######接口数据结构
```c

```
######接口函数及宏
```c
//初始化
void subsys_driver_init();
```
######系统调用
```c
//power
power_off
reboot

//memory
valloc
vfree
create_pg_obj
vmap
vunmap

//console
kprint

//process
fork
execve
wait
get_current_proc_id
get_uid
get_gid
get_euid
set_euid
get_egid
set_egid

//thread
create_thread
suspend_thread
resume_thread
exit_thread
join_thread
suspend_thread
resume_thread
get_current_thread_id
get_priority
set_priority
get_errno
set_errno

//mutex
//normal
create_mutex
mutex_lock
mutex_trylock
mutex_unlock

//r/w lock
create_mutex_rw
mutex_rw_r_lock
mutex_rw_r_trylock
mutex_rw_r_unlock
mutex_rw_w_lock
mutex_rw_w_trylock
mutex_rw_w_unlock

//rcu
create_mutex_rcu
mutex_rcu_r_lock
mutex_rcu_r_trylock
mutex_rcu_r_unlock
mutex_rcu_w_lock
mutex_rcu_w_trylock
mutex_rcu_w_sync
mutex_rcu_w_unlock

//semaphore
create_semaphore
semaphore_acquire
semaphore_tryacquire
semaphore_release

//ipc
sigaction
kill

//msg
msg_complete
msg_cancel

//fs
open
read
write
lseek
poll
fcntl
ioctl
chmod
chown
link
truncate
symlink
unlink
mknod
stat
access
mmap
munmap
mount
umount

//device
create_device
remove_device
msg_send
msg_recv

//exception
push_hndlr
pop_hndlr
raise

```
######文件列表
```c
src/sandnix/kernel/subsystem/driver/driver.h
src/sandnix/kernel/subsystem/driver/driver.c

src/sandnix/kernel/subsystem/driver/syscalls/power/power.h
src/sandnix/kernel/subsystem/driver/syscalls/power/power.c

src/sandnix/kernel/subsystem/driver/syscalls/console/console.h
src/sandnix/kernel/subsystem/driver/syscalls/console/console.c

src/sandnix/kernel/subsystem/driver/syscalls/process/process.h
src/sandnix/kernel/subsystem/driver/syscalls/process/process.c

src/sandnix/kernel/subsystem/driver/syscalls/thread/thread.h
src/sandnix/kernel/subsystem/driver/syscalls/thread/thread.c

src/sandnix/kernel/subsystem/driver/syscalls/lock/lock.h
src/sandnix/kernel/subsystem/driver/syscalls/lock/lock.c

src/sandnix/kernel/subsystem/driver/syscalls/ipc/ipc.h
src/sandnix/kernel/subsystem/driver/syscalls/ipc/ipc.c

src/sandnix/kernel/subsystem/driver/syscalls/msg/msg.h
src/sandnix/kernel/subsystem/driver/syscalls/msg/msg.c
src/sandnix/kernel/subsystem/driver/syscalls/msg/messages -> src/sandnix/kernel/core/msg/messages.h

src/sandnix/kernel/subsystem/driver/syscalls/fs/fs.h
src/sandnix/kernel/subsystem/driver/syscalls/fs/fs.c
src/sandnix/kernel/subsystem/driver/syscalls/fs/styles.h -> src/sandnix/kernel/core/vfs/styles.h

src/sandnix/kernel/subsystem/driver/syscalls/device.h
src/sandnix/kernel/subsystem/driver/syscalls/device.c

src/sandnix/kernel/subsystem/driver/syscalls/exception.h
src/sandnix/kernel/subsystem/driver/syscalls/exception.c
```
#####linux
为linux应用提供系统调用的子系统
######模块路径
```
src/sandnix/kernel/subsystem/linux
```
######接口数据结构
```c

```
######接口函数及宏
```c
//初始化
void subsys_linux_init();
```
######系统调用
```c

```
######文件列表
```c

```
#####驱动程序
位于src/drivers下.
