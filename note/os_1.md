# introduce

* o/s Purposes
  * abstract hardware
  * multiplex
  * isolation
  * sharing
  * security
  * performance
  * range of different user

* API-kernel(system call)
  * open
  * write
  * fork

* why hard/interesting?
  * environments unforgiving(不可原谅的，难以饶恕的)
  * tensions
    1. effeicent - abstract
    2. powerful - simple
    3. flexible - secure
  * filefd = open(); pid = fork();

xv6, Rcsc-v, QEMU

## install

### riscv-gnu-toolchain

```shell
git clone https://github.com/riscv/riscv-gnu-toolchain

# RHEL OS
sudo yum install autoconf automake python3 libmpc-devel mpfr-devel gmp-devel gawk  bison flex texinfo patchutils gcc gcc-c++ zlib-devel expat-devel libslirp-devel
```

```shell
cd riscv-gnu-toolchain
./configure --prefix=opt/riscv
```

```shell
sudo make 
# error: makeinfo: command not found
# solution:

sudo dnf check-update
dnf --enablerepo=devel install texinfo
dnf --enablerepo=devel install libslirp-devel
```

```shell
sudo make 
# error: bison: command not found
#

sudo dnf install bison

# ...
```

ctr \n error

```shell
echo 'sudo yum install autoconf automake python3 libmpc-devel mpfr-devel gmp-devel gawk  bison flex texinfo patchutils gcc gcc-c++ zlib-devel expat-devel libslirp-devel' > conf.sh

sudo bash conf.sh

cd riscv-gnu-toolchain
sudo make clean
# stdout---ty.log 
# stderr---terminal
sudo make -j2 > ty.log 

riscv64-unknown-elf-gcc --version
# riscv64-unknown-elf-gcc (gc891d8dc23e) 13.2.0
# ...
```

### qemu

```shell
wget https://download.qemu.org/qemu-5.1.0.tar.xz
tar -xf qemu-5.1.0.tar.xz

cd qemu-5.1.0
./configure --disable-kvm --disable-werror --prefix=/opt/qemu --target-list="riscv64-softmmu"
# ERROR: Cannot find Ninja

make 
sudo make install # -> /opt/qemu/bin
```

```shell
mkdir ninja
cd ninja

git clone https://github.com/ninja-build/ninja.git
cd ninja

./configure.py --bootstrap
cd ..
mkdir bin
ln ../ninja/ninja ninja

# NINJA=~/os/ninja
ninja --version
# 1.13.1
```

### xv4

```shell
git clone git@github.com:mit-pdos/xv6-riscv.git

cd xv3-riscv
make qemu
# [Ctrl + a] then [x] for exit
```

### lab

* sleep
* pingpong
* primes
  * 并发版本的埃氏筛，用子进程来承载查找带查找列表中是当前素数的倍数的数字，如果不是，则送往下一个通道中查找，整体思路类似并发快速排序
* find
  * 注意fd不够用的问题即可
* xargs
* 可选
  * 主要想要实现*Tab补全*的功能
  * 如果顺利的话，可以实现*命令历史记录*

#### Tab Complement

* **kernel/console.c**

原本内核(默认)对TAB的处理是作为`\t`处理，这里并不希望输出`\t`：

在`consleintr()`中`switch`的`default`分支中，判断如果输入的`c == TAB`，则不再输入；

另外，期望`TAB`如同`\n`，会触发`read()`系统调用(即写入缓冲区)，因此判断`c == TAB`，则：
```C
cons.w = cons.e;
wakeup(&cons.r);
```

用户态要知道触发`TAB`，可以通过`read()`的返回值来确定：

修改`consoleread()`的逻辑，在将数据传递给用户态之前，判断如果是`TAB`，则返回`-101`不写入

这里注意返回之前要先发布`cons`

***

控制台`console`使用一个`cons`类对象来描述的，它有一段环形的缓冲区`buf`，

另外有读`r`、写`w`和编辑`e`三个用于指示的索引，

采用`acquire-release`的内存序保证并发安全。

* **user/sh.c**

修改原先的`getcmd()`函数，这里提供一个新的版本`getcmd_complement()`；

这个函数的实现思路其实并不复杂：

从标准输入中读取字符，若`read() == -101`，则没有触发`TAB`，同`getcmd()`;

如果触发，则执行`complement()`尝试补全：

此函数接收当前用户已经写入的命令`buf`，然后尝试打开当前目录`open(".")`

遍历目录中的每一项`read(dir, &de, sizeof(de))`，检测是否只有一个以`buf`为前缀的文件名：

如果不是，则会输出一个`\a`提示；

否则将目标文件名后附至`buf`，然后再终端上紧接用户输入打印后续字符串

写的时候，可以向console文件写，也可以向stdin写，目前看来两者效果是一致的；

* **TODO**

上述实现是不完整的，因此补全的部分用户是无法使用退格键回退的；

要解决这个问题，则需要更深入了解`cons`的行为，暂时记为TODO