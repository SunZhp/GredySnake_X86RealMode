#ifndef _GREDY_SNAKE_NO_OS_H_
#define _GREDY_SNAKE_NO_OS_H_

#define int8_t char
#define uint8_t	unsigned char
#define int16_t short
#define uint16_t unsigned short
#define int32_t long
#define uint32_t unsigned long

#define WINDOW_X 40
#define WINDOW_Y 20

/*
 * 定义字符输出相关的函数
 */
#define FG_BLACK (0x1)
#define FG_GREEN (0x2)
#define FG_RED	 (0x4)
#define FG_LIGHT (0x8)
#define BG_BLACK (FG_BLACK<<4)
#define BG_GREEN (FG_GREEN<<4)
#define BG_RED	 (FG_RED<<4)
#define BG_LIGHT (FG_LIGHT<<4)

#define GREEN (BG_BLACK|FG_GREEN)
#define RED	  (BG_BLACK|FG_RED)

int start();
void _start();

enum {
	MV_UP,
	MV_DOWN,
	MV_LEFT,
	MV_RIGHT,
};

typedef struct cursor{
	int x;
	int y;
} cursor_t ;

typedef struct apple{
	cursor_t cursor;
} apple_t;

typedef struct gredy_snake_node{
	cursor_t cursor;
	//struct gredy_snake_node *next;
} gredy_snake_node_t;

typedef struct gredy_snake{
	int	body_len;
	int move_dire;
	gredy_snake_node_t *head;
	gredy_snake_node_t *tail;
} gredy_snake_t;

#endif
