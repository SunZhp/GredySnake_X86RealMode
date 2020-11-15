GredySnake_X86ReadMode 是一个不依赖操作系统，跑在x86实模式下的贪吃蛇小程序

[运行环境]
bochs

[语言]
汇编，C(c中使用了内联汇编)

[组织结构及功能]
mbr.S 主引导块代码，加载贪吃蛇程序
gredysnake_no_os.c 贪吃蛇主程序

[编译环境]
binutils
--------
Remember: always be careful before pasting walls of text from the internet. I recommend copying line by line.

```sh
mkdir /tmp/src
cd /tmp/src
curl -O http://ftp.gnu.org/gnu/binutils/binutils-2.24.tar.gz # If the link 404's, look for a more recent version
tar xf binutils-2.24.tar.gz
mkdir binutils-build
cd binutils-build
../binutils-2.24/configure --target=$TARGET --enable-interwork --enable-multilib --disable-nls --disable-werror --prefix=$PREFIX 2>&1 | tee configure.log
make all install 2>&1 | tee make.log
```

gcc
---
```sh
cd /tmp/src
curl -O https://ftp.gnu.org/gnu/gcc/gcc-4.9.1/gcc-4.9.1.tar.bz2
tar xf gcc-4.9.1.tar.bz2
mkdir gcc-build
cd gcc-build
../gcc-4.9.1/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --disable-libssp --enable-languages=c --without-headers
make all-gcc 
make all-target-libgcc 
make install-gcc 
make install-target-libgcc 
```
编译环境安装参考自:https://github.com/cfenollosa/os-tutorial.git

bochs
---
```
yum install bochs
```

[编译]
mbr
---
```
编译：nasm -f bin -o mbr.bin mbr.S
安装：dd if=mbr.bin of=hd60.img bs=512 conv=notrunk
```
GredySnake_X86RealMode
---
```
编译：
/usr/local/i386elfgcc/bin/i386-elf-gcc -m16 -ffreestanding  -c -o gredysnake_no_os.o gredysnake_no_os.c
/usr/local/i386elfgcc/bin/i386-elf-ld -o gredysnake_no_os.bin -Ttext 0x900 --oformat binary gredysnake_no_os.o

安装：dd if=gredysnake_no_os.bin of=hd60.img bs=512 conv=notrunc seek=2
```
[运行]
bochs -f bochsrc.disk

