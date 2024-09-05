# system-call 

## trace

实现进程系统调用tarce跟踪，思路并不复杂，主要时熟悉系统调用的流程：

* **声明**

1. 在`user/user.h`添加系统调用的函数声明：
```C
int trace(int mask);
```

2. 在`user/usy.pl`中添加`stub`

3. 在`kernel/syscall.h`中添加系统调用编号
```C
#define SYS_trace 22
```

* **实现**

1. 在`kernel/sysproc.c`中添加`sys_trace()`函数，用于给进程注册`mask`
2. 修改内核代码实现trace:
   * 各种系统调用在内核的实现为`sys_xxx()`
   * `kernel`维护了一个函数指针数组，通过系统调用编号(从trapframe中的a7)可索引至相应函数
   * 执行相应系统调用，返回值被放在trapframe的a0中

***

执行测试`Test children`时，发现`pid=5`的进程一直无法`fork()`出新进程，原因如下：

注册tarce的mask的最顶级的父进程pid=3，这个进程fork出pid=4的子进程中，子进程调用exec执行任务

本测试为：`exec("usertests", {"forkforkfork"});`

进入其main函数(`user/usertests.c:3095`)，根据参数首先确定测试次数等；

进入`drivetests()` 会首先调用`countfree()` 在这个函数fork出pid=5的子进程

这个子进程并没有执行fork，而是调用sbrk，write等等

然后父进程调用`runtest()`执行相应的测试，就可以看到pid=4的进程fork出新的子进程

新的子进程又不断fork出新的子进程...

这里虽然再第一次fork出pid=5的子进程时，父进程会等待子进程退出在开始调用fork，但在分配进程id时`kernel/proc.c:allocpid()`的逻辑是这样的：

维护一个全局的nextpid，每创建一个新进程nextpid++，这就导致了，想要再次获取pid=5的子进程，需要轮一圈才可以，可是进程数在此之前就已经达到了上限。

* **solution**

原先的`user/trace.c`的设计有问题，正确做法应该是直接调用`exec()`替换pid=3的进程影响；

这样，刚好可以匹配其测试用例：

3进程执行任务，首先fork出4进程不断进行`sbrk()`系统调用；

然后3进程执行测试函数`forkforkfork()`，会创建5，6进程，匹配测试用例；

!["略"](img/pass-trace1.png)

## sysinfo

这个实现比较简单，只要搞清楚相关含义即可。

### 非`UNUSED`的进程数

所有进程都被放在一个数组中：
```C
struct proc proc[NPROC];
```

遍历所有进程，检验其`state`成员即可，注意*加锁*。

### 空闲内存数

由`kalloc()`的实现可知：

1. 所有内存由`kmem`对象管理，它只有一把锁，保护内存页链表`freelist`
2. 每一个内存页大小为`PGSIZE`

因此，原问题即求链表中结点个数。