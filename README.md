GredySnake_X86ReadMode 是一个不依赖操作系统，跑在x86实模式下的贪吃蛇小程序

[运行环境]
bochs

[语言]
汇编，C(c中使用了内联汇编)

[组织结构及功能]
mbr.S 主引导块代码，加载贪吃蛇程序
gredysnake_no_os.c 贪吃蛇主程序
