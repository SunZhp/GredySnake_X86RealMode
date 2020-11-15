#define OS_NONE
#ifdef OS_NONE
asm (".code16gcc\n");
#endif

#ifndef OS_NONE
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#endif

#include "gredysnake_no_os.h"

#define PAINT_WALL '#'
#define PAINT_GROUND ' '
#define PAINT_SNAKE_BODY	'*'
#define PAINT_SNAKE_HEAD	'O'
#define PAINT_APPLE			'+'

void print_ch(int8_t str);
//int set_cursor(uint8_t x,uint8_t y);
void clear_screen();
int set_color(uint8_t x,uint8_t y,uint8_t color,int8_t ch);
uint8_t get_ch();

#define DEBUG
#ifdef DEBUG
#define gs_debug(str) print_ch(str)
#else
#define gs_debug(str)
#endif

//入口函数
#ifdef OS_NONE
void _start(){
#else
static struct termios stored_settings;

void set_keypress(void);
void reset_keypress(void);

int main(){
    set_keypress();
#endif
	start();
#ifndef OS_NONE
    reset_keypress();
#endif
}

int __ncol=0,__nrow=0;

#if 0
int set_cursor(uint8_t x,uint8_t y){
	if(x>WINDOW_X || y > WINDOW_Y) return -1;
	
	asm volatile ("		\
		movb $0,%%bh;\
		movb $0x02,%%ah;\
		movb %0,%%dh;\
		movb %1,%%dl;\
		int $0x10;"\
		::"m"(x),"m"(y)\
		:"ax","dx","bx");
	return 0;
}
#endif

#ifdef OS_NONE
//上卷屏幕,实现清屏操作
void clear_screen(){
    asm volatile(" \
    movw $0x0600,%%ax;  \
    movw $0x0700,%%bx;  \
    movw $0,%%cx;       \
    movw $0x0184f,%%dx; \
    int  $0x10;"        \
    :::"ax","bx","cx","dx");
}

#else
void clear_screen(){
    printf("\033[2J");
}
#endif

#ifdef OS_NONE
//设置输出字符坐标及颜色
int set_color(uint8_t x,uint8_t y,uint8_t color,int8_t ch){
    asm volatile (" \
        movb $0,%%bh;\
        movb $0x02,%%ah;\
        movb %0,%%dh;\
		movb %1,%%dl;\
		int $0x10;\
        \
		movb $0,%%bh;\
		movb $0x9,%%ah;\
		movb %2,%%al;\
		movb %3,%%bl;\
		movw $1,%%cx;\
		int $0x10;"\
		::"m"(x),"m"(y),"m"(ch),"m"(color)	\
		:"ax","cx","bx","dx");

	return 0;
}
#else 
int set_color(uint8_t x,uint8_t y,uint8_t color,int8_t ch){
    printf("\033[%d;%dH",x+1,y+1);
    printf("%c",ch);
    return 0;
}
#endif
//end

/*
 * 非阻塞字符输入,0x16中断1号功能无法持续获得输入，目前没找到原因，换0x9中断
 */
#ifdef OS_NONE
typedef union {
	uint32_t func32;
	uint16_t func16[2];
}addr_t;

#if 0
static uint16_t get_cs(){
    uint16_t cs = 0;
    asm volatile("  \
              movw %%cs,%%ax;"  \
              :"=a"(cs)::);
    return cs;
}
#endif

uint8_t ascii;
addr_t old_int9func;
void int9_func();
//将9号中断放在6号中断处
void set_int9_func(void){
    uint16_t func = (uint16_t)int9_func;
	*(uint16_t*)(0x06 * 4 + 2) =*(uint16_t*)(0x09 * 4 + 2);
	*(uint16_t*)(0x06 * 4) =*(uint16_t*)(0x09 * 4);

    asm("\
            movw $0x24,%%ax;\
            movw $0x26,%%bx;\
            movw %%cs,(%%eax);\
            movw %%dx,(%%ebx);\
        "::"d"(func):"ax","bx");
}

//从0x06老的0x09中断处理例程
void restore_int9_func(){
	*(uint16_t*)(0x09 * 4 + 2) =*(uint16_t*)(0x06 * 4 + 2);
	*(uint16_t*)(0x09 * 4) =*(uint16_t*)(0x06 * 4);
}

//新的0x9中断处理例程
//将结果放入0x500中
void int9_func(){
	asm volatile(" \
            sti;    \
            inw $0x60,%%ax;  \
            int $0x06;     \
            movb %%al,%0;   \
            cli;"            \
			:"=m"(ascii)     \
            ::);
}

uint8_t get_ch(){
	uint8_t ascii = 0;

#if 0
    //asm volatile("\
    //        inw $0x60,%%ax;"\
    //        :"=a"(ascii)::);
#if 0
	switch(ascii){
		case 0x1c:
			ascii='a';break;
		case 0x1b:
			ascii='s';break;
		case 0x23:
			ascii='d';break;
		case 0x1d:
			ascii='w';break;
	}
#endif
#else
	asm volatile (" \
        movb $0x1,%%ah; \
        movb $0x0,%%al; \
        int $0x16;   \
        jz end;      \
        movb $0x0,%%ah; \
        int $0x16;   \
        movb %%al,%0;  \
        end:"         \
		:"=m"(ascii)\
		::"ax","dx");
#endif
	return ascii;
}
#else

void set_keypress(void) {
    struct termios new_settings;

    tcgetattr(0,&stored_settings);

    new_settings = stored_settings;


    new_settings.c_lflag &= (~ICANON);
    new_settings.c_lflag &= (~ECHO);
    new_settings.c_cc[VTIME] = 0;
    new_settings.c_cc[VMIN] = 1;

    tcsetattr(0,TCSANOW,&new_settings);
    return;
}

void reset_keypress(void){
    tcsetattr(0,TCSANOW,&stored_settings);
    return;
}

static inline int kbhit(void)
{

   fd_set rfds;
   struct timeval tv;
   int retval;

   /* Watch stdin (fd 0) to see when it has input. */
   FD_ZERO(&rfds);
   FD_SET(0, &rfds);
   /* Wait up to five seconds. */
   tv.tv_sec  = 0;
   tv.tv_usec = 0;

   retval = select(1, &rfds, NULL, NULL, &tv);
   /* Don't rely on the value of tv now! */

   if (retval == -1) {
      perror("select()");
      return 0;
   } else if (retval)
      return 1;
   /* FD_ISSET(0, &rfds) will be true. */
   else
      return 0;
   return 0;
}

uint8_t get_ch(){
    uint8_t ascii = 0;
    if(kbhit())
        ascii = getchar();

    return ascii;
}
#endif 
//end

/*
 *获取时间,用于产生随机数
 */
#ifdef OS_NONE
uint32_t get_timestamp(){
    uint32_t timestamp = 0;

    asm volatile (" \
            movb $0,%%ah; \
            int $0x1a;\
            movw %%dx,%%bx;\
            shl $16,%%bx;\
            movw %%cx,%%bx;"\
            :"=b"(timestamp)\
            ::"ax","dx","cx");
    return timestamp;
}
#else
uint32_t get_timestamp(){
    uint32_t timestamp = 0;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    timestamp=tv.tv_sec;

    return timestamp;
}
#endif

#if 0
/*
 *定义内存分配相关函数,由于定义在数据段中，所以不需要free
 */
void* _malloc(uint32_t size){
    static int malloc_index = 0;
    void *addr = NULL;
    asm volatile (" \
            alloc $0 $1 ; "\
            :"a"(addr) \
            :"N"(malloc_index),"N",size);
    return addr;
}
//end
#endif

#ifdef OS_NONE
void print_ch(int8_t str){
    do{
        //while(*str++){
            __ncol = __ncol + 1;
			__nrow = __nrow + 1;
            //set_cursor(__nrow,__ncol);
            set_color(__nrow,__ncol,GREEN,str);
        //}

    }while(0);
}
#else
void print_ch(int8_t str){
    printf("%c",str);
}
#endif 

//apple_t apple = {{0,0}};
//gredy_snake_t snake = {0,NULL,NULL};
//动态分配内存的操作比较复杂，使用静态分配
gredy_snake_node_t snake_node[WINDOW_X*WINDOW_Y];
int8_t window[WINDOW_Y][WINDOW_X];

int paint_window(gredy_snake_t* snake,apple_t* apple){
    uint8_t i,j;
    if(!snake || !apple) return -1;

    for(i=0;i<WINDOW_Y;i++){
        for(j=0;j<WINDOW_X;j++){
            //set_cursor(i,j);
            if((i==0) || (j==0) || (i==(WINDOW_Y-1)) || (j==(WINDOW_X-1))){
                window[i][j]=PAINT_WALL;
                //set_color(i,j,GREEN,PAINT_WALL);
            }
			else{
				window[i][j]=PAINT_GROUND;
				//set_color(i,j,GREEN,PAINT_GROUND);
			}
		}
	}
	if(snake->head){
		//set_cursor(snake->head->cursor.y,snake->head->cursor.x);
		window[snake->head->cursor.y][snake->head->cursor.x]=PAINT_SNAKE_HEAD;
		//set_color(snake->head->cursor.y, 
        //  snake->head->cursor.x,RED,PAINT_SNAKE_HEAD);
		for(i=1;i<snake->body_len;i++){
			//set_cursor((snake->head+i)->cursor.x,(snake->head+i)->cursor.y);
			window[(snake->head+i)->cursor.y][(snake->head+i)->cursor.x]
                =PAINT_SNAKE_BODY;
			//set_color(snake->head->cursor.x,
            //snake->head->cursor.y,RED,PAINT_SNAKE_BODY);
		}
	}

	//set_cursor(apple->cursor.x,apple->cursor.y);
    if(apple->cursor.x >=0 && apple->cursor.y >=0){
        window[apple->cursor.y][apple->cursor.x]=PAINT_APPLE;
    }
	//set_color(apple->cursor.x,apple->cursor.y,RED,PAINT_APPLE);
    //
	for(i=0;i<WINDOW_Y;i++){
		for(j=0;j<WINDOW_X;j++){
            set_color(i,j,GREEN,window[i][j]);
        }
    }

	return 0;
}

int init_apple(apple_t* apple){
    apple->cursor.x = WINDOW_X/2 - 1;
    apple->cursor.y = WINDOW_Y/2 - 1;
    return 0;
}

int clear_apple(apple_t* apple){
    apple->cursor.x = -1;
    apple->cursor.y = -1;
    return 0;
}

int init_gredy_snake(gredy_snake_t* snake){
	gredy_snake_node_t *node = snake_node;

	if(!snake) return -1;

	snake->move_dire = MV_RIGHT;
	snake->body_len = 1;

	node->cursor.x = WINDOW_X/2;
	node->cursor.y = WINDOW_Y/2;
	//node->next = NULL;

	snake->head = node;
	snake->tail = node;
	return 0;
}

//往蛇屁股上增加一节
int grow_gredy_snake(gredy_snake_t* snake){
	if(!snake)	return -1;

	if(snake->body_len+1 == WINDOW_X*WINDOW_Y){
		return -1;
	}

	switch(snake->move_dire){
		case MV_UP:
			snake_node[snake->body_len].cursor.x=snake->tail->cursor.x;
			snake_node[snake->body_len].cursor.y=snake->tail->cursor.y+1;
			break;
		case MV_DOWN:
			snake_node[snake->body_len].cursor.x=snake->tail->cursor.x;
			snake_node[snake->body_len].cursor.y=snake->tail->cursor.y-1;
			break;
		case MV_LEFT:
			snake_node[snake->body_len].cursor.x=snake->tail->cursor.x+1;
			snake_node[snake->body_len].cursor.y=snake->tail->cursor.y;
			break;
		case MV_RIGHT:
		default:
			snake_node[snake->body_len].cursor.x=snake->tail->cursor.x-1;
			snake_node[snake->body_len].cursor.y=snake->tail->cursor.y;
			break;
	}
	snake->tail = &snake_node[snake->body_len];
	snake->body_len++;
	return 0;
}

//一次移动一步
int move_onestep(gredy_snake_t* snake){
	int i = 0;
	if(!snake)	return -1;

	for(i=snake->body_len-1;i>0;i--){
		(snake->head+i)->cursor.x = (snake->head+i-1)->cursor.x;
		(snake->head+i)->cursor.y = (snake->head+i-1)->cursor.y;
	}

	switch(snake->move_dire){
		case MV_UP:
			snake->head->cursor.y--;
			break;
		case MV_DOWN:
			snake->head->cursor.y++;
			break;
		case MV_LEFT:
			snake->head->cursor.x--;
			break;
		case MV_RIGHT:
		default:
			snake->head->cursor.x++;
			break;
	}
	return 0;
}

//设置移动方向，如果输入字符不是a,s,w,d则使用上次修改的方向
int set_direct(gredy_snake_t* snake){
	if(!snake)	return -1;

	switch(get_ch()){
		case 'a':
			snake->move_dire = MV_LEFT;
			break;
		case 's':
			snake->move_dire = MV_DOWN;
			break;
		case 'w':
			snake->move_dire = MV_UP;
			break;
		case 'd':
			snake->move_dire = MV_RIGHT;
			break;
		default:
			return -1;
	}
	return 0;
}

//使用时间戳作为伪随机数,随机位置产生一个苹果
int generate_apple(gredy_snake_t* snake,apple_t* apple){
	if(!snake || !apple) return -1;

	uint32_t i = 0,j=0;
	int wallnum = 4*(WINDOW_X+WINDOW_Y-1);
	int restnum = WINDOW_X*WINDOW_Y - snake->body_len - wallnum;
	uint32_t random = get_timestamp() % (uint32_t)restnum;
    uint32_t pos = 0;
    int found = 0;

	for(i=0;i<WINDOW_Y;i++){
		for(j=0;j<WINDOW_X;j++){
            if(window[i][j] ==' '){
                pos++ ;
            }
            if(pos == random){
                found = 1;
                break;
            }
		}
        if(found == 1) break;
	}

    if(!found ) return -1;

	apple->cursor.x = j;
	apple->cursor.y = i;

	return 0;
}

enum{
	SNAKE_ERROR,
	SNAKE_EATAPPLE,
	SNAKE_TOUCHWALL,
	SNAKE_MOVENORMAL,
	SNAKE_TOUCHSELF
};

static int touchself(gredy_snake_t* snake){
	int i=1;
	for(;i<snake->body_len;i++){
		if((snake->head->cursor.x==((gredy_snake_node_t *)(snake->head+i))->cursor.x)&&
				(snake->head->cursor.y==((gredy_snake_node_t *)(snake->head+i))->cursor.y))
			return 1;
	}
	return 0;
}
int jundge(gredy_snake_t* snake,apple_t* apple){
	if(!snake || !apple){
		return SNAKE_ERROR;
	}
	if((snake->head->cursor.x<=0 || snake->head->cursor.x>=WINDOW_X-1) ||
		(snake->head->cursor.y<=0 || snake->head->cursor.y>=WINDOW_Y-1)){
		return SNAKE_TOUCHWALL;
	}
	else if(touchself(snake)){
		return SNAKE_TOUCHSELF;
	}
	else if((snake->head->cursor.x == apple->cursor.x)&&
			(snake->head->cursor.y == apple->cursor.y)){
		return SNAKE_EATAPPLE;
	}
	else {
		return SNAKE_MOVENORMAL;
	}
}

//使用cpu运算作为延时函数，在操作系统中运行，需要调大i的值到7500以上
void delay(void)
{
	uint32_t i,j,k;
	for(i=450;i>0;i--){
		for(j=202;j>0;j--){
			for(k=81;k>0;k--){
				;
			}
		}
	}
}

int start(){
	int errcode = 0;
	gredy_snake_t snake;
	apple_t apple;
#ifdef OS_NONE
	//set_int9_func();
#endif

play:
    clear_screen();

    init_apple(&apple);
	init_gredy_snake(&snake);
    paint_window(&snake,&apple);

	while(1){
		delay();
		switch(jundge(&snake,&apple)){
			case SNAKE_ERROR:
				errcode = -1;
				break;
			case SNAKE_EATAPPLE:
                clear_apple(&apple);
				if(grow_gredy_snake(&snake)<0){
					errcode = -4;
					break;
				}
				move_onestep(&snake);
				generate_apple(&snake,&apple);
				errcode = 0;
				break;
			case SNAKE_TOUCHWALL:
				errcode = -2;
				break;
			case SNAKE_MOVENORMAL:
				move_onestep(&snake);
				errcode = 1;
				break;
			case SNAKE_TOUCHSELF:
				errcode = -3;
				break;
		}
		paint_window(&snake,&apple);
		set_direct(&snake);

		//死了就重新开始
		if(errcode < 0){
			//break;
            goto play;
		}
	}

#ifdef OS_NONE
	//restore_int9_func();
#endif

	return errcode;
}
