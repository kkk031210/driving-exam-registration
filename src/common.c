#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include "common.h"

char getch(void) {
    struct termios old, new;
    char ch;
    tcgetattr(STDIN_FILENO, &old);
    new = old;
    new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &old);
    return ch;
}

void clear_screen(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

void print_line(void) {
    printf("══════════════════════════════════\n");
}

void print_title(const char *title) {
    print_line();
    printf("      %s\n", title);
    print_line();
}

void pause_screen(void) {
    printf("\n按 Enter 键继续...");
    getchar();
}

int get_choice(void) {
    int choice;
    printf("\n请选择操作：");
    scanf("%d", &choice);
    while (getchar() != '\n');
    return choice;
}

void get_password(char *buf, int max_len) {
    int i = 0;
    char ch;
    while (1) {
        ch = getch();
        if (ch == '\r' || ch == '\n') {
            break;
        } else if (ch == '\b' && i > 0) {
            i--;
            printf("\b \b");
        } else if (ch != '\b' && i < max_len - 1) {
            buf[i++] = ch;
            printf("*");
        }
    }
    buf[i] = '\0';
    printf("\n");
}

int validate_id_card(const char *id_card) {
    int len = strlen(id_card);
    if (len != 18) return 0;
    for (int i = 0; i < 17; i++) {
        if (id_card[i] < '0' || id_card[i] > '9') return 0;
    }
    if (!((id_card[17] >= '0' && id_card[17] <= '9') || 
          (id_card[17] >= 'X' && id_card[17] <= 'X'))) return 0;
    return 1;
}

int is_date_valid(const char *date) {
    if (strlen(date) != 10) return 0;
    if (date[4] != '-' || date[7] != '-') return 0;
    return 1;
}

int is_time_slot_valid(const char *slot) {
    if (strcmp(slot, "08:30-10:30") == 0) return 1;
    if (strcmp(slot, "13:00-15:00") == 0) return 1;
    if (strcmp(slot, "15:30-17:30") == 0) return 1;
    return 0;
}

void get_current_date(char *buf, int len) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    snprintf(buf, len, "%04d-%02d-%02d %02d:%02d:%02d",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);
}
