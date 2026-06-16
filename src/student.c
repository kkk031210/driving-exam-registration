#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data_structure.h"
#include "common.h"
#include "student.h"

void student_menu(void) {
    int choice;
    do {
        clear_screen();
        print_title("考生信息管理");
        printf("  [1] 添加考生信息\n");
        printf("  [2] 浏览所有考生\n");
        printf("  [3] 查询考生信息\n");
        printf("  [4] 修改考生信息\n");
        printf("  [5] 删除考生信息\n");
        printf("  [0] 返回上级菜单\n");
        choice = get_choice();
        switch (choice) {
            case 1: add_student(); break;
            case 2: list_students(); break;
            case 3: query_student(); break;
            case 4: modify_student(); break;
            case 5: delete_student(); break;
            case 0: break;
            default: printf("无效选择！\n"); pause_screen();
        }
    } while (choice != 0);
}

void add_student(void) {
    printf("\n[功能] 添加考生信息\n");
    pause_screen();
}

void list_students(void) {
    printf("\n[功能] 浏览所有考生\n");
    pause_screen();
}

void query_student(void) {
    printf("\n[功能] 查询考生信息\n");
    pause_screen();
}

void modify_student(void) {
    printf("\n[功能] 修改考生信息\n");
    pause_screen();
}

void delete_student(void) {
    printf("\n[功能] 删除考生信息\n");
    pause_screen();
}
