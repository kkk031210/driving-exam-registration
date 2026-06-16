#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "data_structure.h"
#include "common.h"
#include "exam_sim.h"

void exam_sim_menu(void) {
    int choice;
    do {
        clear_screen();
        print_title("模拟考试");
        printf("  [1] 科目一模拟考试\n");
        printf("  [2] 科目四模拟考试\n");
        printf("  [3] 查看模拟成绩\n");
        printf("  [0] 返回上级菜单\n");
        choice = get_choice();
        switch (choice) {
            case 1: exam_subject1(); break;
            case 2: exam_subject4(); break;
            case 3: list_scores(); break;
            case 0: break;
            default: printf("无效选择！\n"); pause_screen();
        }
    } while (choice != 0);
}

void exam_subject1(void) {
    printf("\n[功能] 科目一模拟考试（100题）\n");
    pause_screen();
}

void exam_subject4(void) {
    printf("\n[功能] 科目四模拟考试（50题）\n");
    pause_screen();
}

void list_scores(void) {
    printf("\n[功能] 查看模拟成绩\n");
    pause_screen();
}
