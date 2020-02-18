# Virtual-Machine

编译环境：VS2015，C++11

一个编译程序，一个汇编程序和一个简单的16位虚拟机

设置Parser工程为主项目，在所在目录的Text.txt文件中按照设计的语言写程序，运行工程即可编译该文件为Text.asm文件；
将Text.asm文件拷贝到Asm工程文件夹，在文本最后一行加上#作为结束符
将Asm工程作为主项目，运行工程即可将Text.asm文件汇编成对应虚拟机支持的名为Text.bin的可执行文件
运行开始会输出汇编过程，最后可以看到虚拟机执行过程

该虚拟机属于寄存器型虚拟机，可用8位寄存器256个
都是很简单的虚拟机，支持必要的add, sub, mul, div, mod, load, store, jmp, jne, jge, jbe, jg, jb, halt等指令
并实现了简单的按字、字节访存

TODO：
  在Asm中实现过程定义和调用指令，类似汇编，需要实现参数传递，方便Parser实现函数调用
  在Parser中实现带参函数调用，实现Break，Continue指令等，函数调用是重点
  因为未实现寄存器分配算法，所以最多用到256个局部和临时变量，亟待实现寄存器分配


# branch VM-1.2

* Virtual-Machine 1.2
 * 编译器：
  * 运算：支持双目和单目算术逻辑运算。
  * 变量：支持变量作用域，实现全局变量和局部变量定义。
  * 控制流：实现了for, do-while, while, switch-case语法，支持break和continue。
  * 异常处理：添加了try-catch-finally语法。
  * 函数：实现了函数定义和调用的语法分析。
 * 汇编器：
  * 支持条件转移
  * 支持函数定义和调用
