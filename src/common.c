/*
 * common.c
 * 驾考报名管理系统 — 公共工具函数实现
 * 包含：终端交互、密码输入、数据校验、时间获取等底层工具
 */

#include <stdio.h>      /* 标准输入输出: printf, scanf, getchar, fflush */
#include <stdlib.h>     /* 标准库: system, exit 等 */
#include <string.h>     /* 字符串处理: strlen, strcmp, strcpy, strstr 等 */
#include <time.h>       /* 时间处理: time, localtime, struct tm, time_t */
#include <termios.h>    /* 终端控制: tcgetattr, tcsetattr, struct termios */
#include <unistd.h>     /* 系统调用: STDIN_FILENO */
#include "common.h"     /* 本模块的函数声明 */

/*
 * 函数: getch
 * 功能: 读取一个字符，不回显到终端
 * 实现: 使用 termios 关闭终端的规范模式(ICANON)和回显(ECHO)，
 *       读取字符后立即恢复原始终端设置，保证不干扰后续输入。
 * 注意: 这是 Linux 替代 Windows <conio.h> 中 getch() 的实现。
 */
char getch(void) {
    struct termios old, new;   /* old: 保存原始终端设置; new: 临时设置 */
    char ch;
    tcgetattr(STDIN_FILENO, &old);          /* 获取当前终端属性 */
    new = old;                              /* 复制原始设置 */
    new.c_lflag &= ~(ICANON | ECHO);        /* 关闭: 规范模式(逐行) + 回显(显示输入) */
    tcsetattr(STDIN_FILENO, TCSANOW, &new); /* 立即应用新设置 */
    ch = getchar();                         /* 读取一个字符（此时不显示） */
    tcsetattr(STDIN_FILENO, TCSANOW, &old); /* 恢复原始终端设置 */
    return ch;
}

/*
 * 函数: clear_screen
 * 功能: 清空终端屏幕，并将光标移动到左上角
 * 实现: 发送 ANSI 转义序列 \033[2J（清屏）\033[H（光标归位），然后 fflush 刷新输出缓冲区。
 * 注意: 在 Windows 下应使用 system("cls")，此处为 Linux 适配版。
 */
void clear_screen(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

/*
 * 函数: print_line
 * 功能: 打印一条装饰分割线（双横线边框），用于美化菜单界面
 */
void print_line(void) {
    printf("══════════════════════════════════\n");
}

/*
 * 函数: print_title
 * 功能: 打印居中的标题（上下包裹分割线）
 * 参数: title — 标题字符串
 */
void print_title(const char *title) {
    print_line();
    printf("      %s\n", title);
    print_line();
}

/*
 * 函数: pause_screen
 * 功能: 暂停屏幕，等待用户按 Enter 键继续
 * 实现: 显示提示后调用 getchar() 阻塞，直到用户输入回车。
 * 注意: 调用前需确保输入缓冲区已清空（如 scanf 后用 while(getchar()!='\n')）。
 */
void pause_screen(void) {
    printf("\n按 Enter 键继续...");
    getchar();
}

/*
 * 函数: get_choice
 * 功能: 读取用户输入的整数菜单选项
 * 实现: 用 scanf 读取整数，然后用 while 循环清空缓冲区中的残留字符（包括换行符），
 *       避免残留影响后续输入。
 * 返回: 用户输入的整数选项
 */
int get_choice(void) {
    int choice;
    printf("\n请选择操作：");
    scanf("%d", &choice);           /* 读取整数 */
    while (getchar() != '\n');      /* 清空缓冲区残留字符，防止影响后续输入 */
    return choice;
}

/*
 * 函数: get_password
 * 功能: 隐藏密码输入，显示 * 占位符，支持退格/删除撤销
 * 参数: buf      — 输出缓冲区，存放用户输入的密码
 *       max_len  — 缓冲区最大长度（含 \0）
 * 实现:
 *   1. 循环调用 getch() 逐字符读取（不回显）
 *   2. 回车/换行 → 结束输入
 *   3. 退格(\b=8) 或删除(DEL=127) → 若已有字符则回删 * 和缓冲区内容
 *   4. 其他字符 → 写入缓冲区并打印 * 占位符
 *   5. 最后补 \0 作为字符串结尾
 */
void get_password(char *buf, int max_len) {
    int i = 0;          /* 当前已输入字符数 */
    char ch;
    while (1) {
        ch = getch();   /* 读取一个无回显字符 */
        if (ch == '\r' || ch == '\n') {     /* 回车结束输入 */
            break;
        } else if ((ch == '\b' || ch == 127) && i > 0) {  /* 退格/删除，且至少输入了一个字符 */
            i--;                           /* 缓冲区索引回退 */
            printf("\b \b");               /* 光标左移、空格覆盖、光标再左移，视觉上擦除 * */
        } else if (ch != '\b' && ch != 127 && i < max_len - 1) {  /* 非退格/删除，且未满 */
            buf[i++] = ch;               /* 写入缓冲区 */
            printf("*");                   /* 打印 * 占位符 */
        }
    }
    buf[i] = '\0';      /* 字符串结束符 */
    printf("\n");       /* 换行，光标移至下一行 */
}

/*
 * 函数: validate_id_card
 * 功能: 校验中华人民共和国身份证号格式
 * 规则:
 *   1. 长度必须恰好为 18 位
 *   2. 前 17 位必须全部为数字字符 '0'-'9'
 *   3. 第 18 位必须是数字或 'X'（最后一位校验码）
 * 返回: 1=格式合法, 0=格式非法
 */
int validate_id_card(const char *id_card) {
    int len = strlen(id_card);          /* 获取字符串长度 */
    if (len != 18) return 0;            /* 长度校验 */
    for (int i = 0; i < 17; i++) {      /* 前 17 位逐字符检查 */
        if (id_card[i] < '0' || id_card[i] > '9') return 0;
    }
    /* 第 18 位: 数字或 X */
    if (!((id_card[17] >= '0' && id_card[17] <= '9') || 
          (id_card[17] >= 'X' && id_card[17] <= 'X'))) return 0;
    return 1;
}

/*
 * 函数: is_date_valid
 * 功能: 校验日期字符串格式是否为 YYYY-MM-DD
 * 规则: 长度恰好 10，第 5 位和第 8 位为 '-' 分隔符
 * 返回: 1=格式合法, 0=格式非法
 * 注意: 仅做格式校验，不做日期逻辑校验（如 2 月 30 日）。
 */
int is_date_valid(const char *date) {
    if (strlen(date) != 10) return 0;        /* 长度必须是 10: YYYY-MM-DD */
    if (date[4] != '-' || date[7] != '-')    /* 分隔符位置校验 */
        return 1;
    return 0; 
}

/*
 * 函数: is_time_slot_valid
 * 功能: 校验考试时段是否为系统预置的合法时段
 * 合法时段: 08:30-10:30, 13:00-15:00, 15:30-17:30
 * 返回: 1=合法, 0=非法
 */
int is_time_slot_valid(const char *slot) 
{
    if (strcmp(slot, "08:30-10:30") == 0) 
        return 1;
    if (strcmp(slot, "13:00-15:00") == 0) 
        return 1;
    if (strcmp(slot, "15:30-17:30") == 0) 
        return 1;
    return 0;
}

/*
 * 函数: get_current_date
 * 功能: 获取当前系统时间，格式化为 YYYY-MM-DD HH:MM:SS 字符串
 * 参数: buf — 输出缓冲区; len — 缓冲区长度
 * 实现: 调用 time() 获取秒级时间戳 → localtime() 转换为本地时间结构 → snprintf 格式化字符串
 */
void get_current_date(char *buf, int len) {
    time_t now = time(NULL);                /* 获取当前时间戳（自 1970-01-01 起的秒数） */
    struct tm *t = localtime(&now);          /* 转换为本地时间结构体（含年月日时分秒） */
    snprintf(buf, len, "%04d-%02d-%02d %02d:%02d:%02d",
             t->tm_year + 1900,             /* tm_year 是自 1900 起的偏移 */
             t->tm_mon + 1,                  /* tm_mon 范围 0-11，需 +1 */
             t->tm_mday,                     /* 日: 1-31 */
             t->tm_hour,                     /* 时: 0-23 */
             t->tm_min,                      /* 分: 0-59 */
             t->tm_sec);                     /* 秒: 0-59 */
}
