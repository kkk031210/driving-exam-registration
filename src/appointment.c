#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data_structure.h"
#include "common.h"
#include "appointment.h"

void appointment_menu(void) {
    int choice;
    do {
        clear_screen();
        print_title("预约管理");
        printf("  [1] 预约考试\n");
        printf("  [2] 查看所有预约\n");
        printf("  [3] 查询预约信息\n");
        printf("  [4] 取消预约\n");
        printf("  [5] 更新预约信息\n");
        printf("  [0] 返回上级菜单\n");
        choice = get_choice();
        switch (choice) {
            case 1: book_appointment(); break;
            case 2: list_appointments(); break;
            case 3: query_appointment(); break;
            case 4: cancel_appointment(); break;
            case 5: update_appointment(); break;
            case 0: break;
            default: printf("无效选择！\n"); pause_screen();
        }
    } while (choice != 0);
}

void book_appointment(void) {
    printf("\n[功能] 预约考试\n");
    pause_screen();
}

void list_appointments(void) {
    printf("\n[功能] 查看所有预约\n");
    pause_screen();
}

void query_appointment(void) {
    printf("\n[功能] 查询预约信息\n");
    pause_screen();
}

void cancel_appointment(void) {
    printf("\n[功能] 取消预约（硬删除）\n");
    pause_screen();
}

void update_appointment(void) {
    printf("\n[功能] 更新预约信息\n");
    pause_screen();
}
