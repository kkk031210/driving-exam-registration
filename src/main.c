#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "data_structure.h"
#include "common.h"
#include "student.h"
#include "appointment.h"
#include "exam_sim.h"
#include "file_io.h"

Student students[MAX_SIZE];
int student_count = 0;

Admin admins[MAX_SIZE];
int admin_count = 0;

Appointment appointments[MAX_SIZE];
int appointment_count = 0;

Venue venues[MAX_VENUES];
int venue_count = 0;

Question questions[MAX_QUESTIONS];
int question_count = 0;

ExamScore scores[MAX_SIZE];
int score_count = 0;

int current_role = 0;
int current_user_id = -1;

static void login_menu(void);
static int admin_login(void);
static int student_login(void);
static void admin_menu(void);
static void student_menu_main(void);

int main(void) {
    srand((unsigned)time(NULL));
    init_data();
    load_all();
    login_menu();
    save_all();
    printf("\n[系统] 数据已保存，感谢使用！\n");
    return 0;
}

static void login_menu(void) {
    int choice;
    do {
        clear_screen();
        print_title("驾考考试报名管理系统");
        printf("  [1] 管理员登录\n");
        printf("  [2] 考生登录\n");
        printf("  [0] 退出系统\n");
        choice = get_choice();
        switch (choice) {
            case 1:
                if (admin_login()) admin_menu();
                break;
            case 2:
                if (student_login()) student_menu_main();
                break;
            case 0: break;
            default:
                printf("无效选择！\n");
                pause_screen();
        }
    } while (choice != 0);
}

static int admin_login(void) {
    char username[33], password[33];
    printf("\n请输入管理员用户名：");
    scanf("%32s", username);
    while (getchar() != '\n');
    printf("请输入密码：");
    get_password(password, sizeof(password));

    for (int i = 0; i < admin_count; i++) {
        if (strcmp(admins[i].username, username) == 0 &&
            strcmp(admins[i].password, password) == 0) {
            current_role = 1;
            current_user_id = admins[i].id;
            printf("\n[登录成功] 欢迎管理员 %s\n", username);
            pause_screen();
            return 1;
        }
    }
    printf("\n[登录失败] 用户名或密码错误！\n");
    pause_screen();
    return 0;
}

static int student_login(void) {
    char id_card[19], password[33];
    printf("\n请输入身份证号：");
    scanf("%18s", id_card);
    while (getchar() != '\n');
    printf("请输入密码：");
    get_password(password, sizeof(password));

    for (int i = 0; i < student_count; i++) {
        if (strcmp(students[i].id_card, id_card) == 0 &&
            strcmp(students[i].password, password) == 0) {
            current_role = 2;
            current_user_id = students[i].id;
            printf("\n[登录成功] 欢迎考生 %s\n", students[i].name);
            pause_screen();
            return 1;
        }
    }
    printf("\n[登录失败] 身份证号或密码错误！\n");
    pause_screen();
    return 0;
}

static void admin_menu(void) {
    int choice;
    do {
        clear_screen();
        print_title("管理员功能菜单");
        printf("  [1] 考生信息管理\n");
        printf("  [2] 预约管理\n");
        printf("  [3] 模拟考试\n");
        printf("  [0] 注销登录\n");
        choice = get_choice();
        switch (choice) {
            case 1: student_menu(); break;
            case 2: appointment_menu(); break;
            case 3: exam_sim_menu(); break;
            case 0:
                current_role = 0;
                current_user_id = -1;
                break;
            default:
                printf("无效选择！\n");
                pause_screen();
        }
    } while (choice != 0);
}

static void student_menu_main(void) {
    int choice;
    do {
        clear_screen();
        print_title("考生功能菜单");
        printf("  [1] 我的信息管理\n");
        printf("  [2] 我的预约管理\n");
        printf("  [3] 模拟考试\n");
        printf("  [0] 注销登录\n");
        choice = get_choice();
        switch (choice) {
            case 1:
                printf("\n[功能] 我的信息\n");
                pause_screen();
                break;
            case 2:
                printf("\n[功能] 我的预约\n");
                pause_screen();
                break;
            case 3: exam_sim_menu(); break;
            case 0:
                current_role = 0;
                current_user_id = -1;
                break;
            default:
                printf("无效选择！\n");
                pause_screen();
        }
    } while (choice != 0);
}
