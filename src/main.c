/*
 * main.c
 * 驾考报名管理系统 — 程序入口与角色路由
 * 职责:
 *   1. 定义所有全局变量（结构体数组及其计数器）
 *   2. 初始化随机种子（用于模拟考试抽题）
 *   3. 加载预置数据和持久化数据
 *   4. 登录菜单（管理员 / 考生）
 *   5. 根据角色分发到对应功能菜单
 *   6. 退出时保存所有数据到文件
 */

#include <stdio.h>          /* 标准输入输出 */
#include <stdlib.h>         /* 标准库函数 */
#include <string.h>         /* 字符串处理 */
#include <time.h>           /* 时间函数: time(), srand() 种子 */
#include "data_structure.h" /* 全局变量和结构体声明 */
#include "common.h"         /* 公共工具函数 */
#include "student.h"        /* 考生管理模块接口 */
#include "appointment.h"    /* 预约管理模块接口 */
#include "exam_sim.h"       /* 模拟考试模块接口 */
#include "file_io.h"        /* 数据持久化模块接口 */

/* ============================================
 * 全局变量定义（唯一定义处，其他文件通过 extern 引用）
 * 所有模块共享同一份内存数据，读写操作直接作用于这些数组
 * ============================================ */

Student students[MAX_SIZE];         /* 考生信息数组，最多 MAX_SIZE 条 */
int  student_count = 0;               /* 当前实际考生数量，也是数组下标边界 */

Admin admins[MAX_SIZE];             /* 管理员账号数组 */
int admin_count = 0;                 /* 当前管理员数量 */

Appointment appointments[MAX_SIZE]; /* 预约记录数组 */
int appointment_count = 0;           /* 当前预约数量 */

Venue venues[MAX_VENUES];           /* 考试场地数组 */
int venue_count = 0;                 /* 当前场地数量 */

Question questions[MAX_QUESTIONS];  /* 题库数组 */
int question_count = 0;              /* 当前题目数量 */

ExamScore scores[MAX_SIZE];         /* 模拟考试成绩数组 */
int score_count = 0;                 /* 当前成绩记录数量 */

int current_role = 0;                /* 当前登录角色: 0=未登录, 1=管理员, 2=考生 */
int current_user_id = -1;            /* 当前登录用户的 ID（用于权限控制） */

/* ============================================
 * 静态函数声明（仅在本文件内使用，不对外暴露）
 * ============================================ */
static void login_menu(void);        /* 登录选择菜单 */
static int admin_login(void);        /* 管理员登录流程 */
static int student_login(void);      /* 考生登录流程 */
static void admin_menu(void);        /* 管理员功能菜单 */
static void student_menu_main(void); /* 考生功能菜单 */

/* ============================================
 * 主函数
 * ============================================ */
int main(void) {
    srand((unsigned)time(NULL));     /* 以当前时间戳初始化随机种子，保证每次运行抽题不同 */
    init_data();                      /* 首次运行时创建预置数据（管理员、场地、题库） */
    load_all();                       /* 从所有 .dat 文件加载数据到内存数组 */
    login_menu();                     /* 进入登录循环，直到用户选择退出 */
    save_all();                       /* 退出时将所有内存数据写回文件 */
    printf("\n[系统] 数据已保存，感谢使用！\n");
    return 0;
}

/* ============================================
 * 函数: login_menu
 * 功能: 系统入口菜单，提供管理员登录、考生登录、退出三种选择
 * 循环: 直到用户选择 0（退出系统）
 * 登录成功后，根据角色进入 admin_menu() 或 student_menu_main()
 * ============================================ */
static void login_menu(void) {
    int choice;
    do {
        clear_screen();               /* 清屏，保持界面整洁 */
        print_title("驾考考试报名管理系统");  /* 打印居中大标题 */
        printf("  [1] 管理员登录\n");      /* 管理员拥有全权限 */
        printf("  [2] 考生登录\n");        /* 考生只能管理自己的数据 */
        printf("  [0] 退出系统\n");        /* 退出并保存数据 */
        choice = get_choice();         /* 读取用户输入的选项 */
        switch (choice) {
            case 1:
                /* 调用 admin_login() 校验账号密码，成功则进入管理员菜单 */
                if (admin_login()) admin_menu();
                break;
            case 2:
                /* 调用 student_login() 校验身份证号和密码，成功则进入考生菜单 */
                if (student_login()) student_menu_main();
                break;
            case 0:
                /* 直接退出循环，main() 中会继续执行 save_all() */
                break;
            default:
                printf("无效选择！\n");
                pause_screen();         /* 提示错误并暂停，等待用户确认 */
        }
    } while (choice != 0);              /* 循环直到选择 0（退出） */
}

/* ============================================
 * 函数: admin_login
 * 功能: 管理员登录流程
 * 实现: 读取用户名和密码 → 遍历 admins 数组逐条比对 → 匹配则设置全局状态
 * 返回: 1=登录成功, 0=登录失败
 * ============================================ */
static int admin_login(void) {
    char username[33], password[33];  /* 缓冲区，大小匹配结构体字段 */
    printf("\n请输入管理员用户名：");
    scanf("%32s", username);           /* 限制读取 32 字符，防止溢出 */
    while (getchar() != '\n');        /* 清空缓冲区残留换行符 */
    printf("请输入密码：");
    get_password(password, sizeof(password));  /* 隐藏输入，显示 * 占位 */

    /* 遍历 admins 数组，查找匹配的用户名和密码 */
    for (register int i = 0; i < admin_count; i++) {
        if (strcmp(admins[i].username, username) == 0 &&   /* 用户名匹配 */
            strcmp(admins[i].password, password) == 0) {   /* 密码匹配 */
            current_role = 1;                               /* 设置角色为管理员 */
            current_user_id = admins[i].id;                  /* 记录当前管理员 ID */
            printf("\n[登录成功] 欢迎管理员 %s\n", username);
            pause_screen();                                 /* 暂停提示成功 */
            return 1;                                       /* 返回成功 */
        }
    }
    /* 遍历结束未匹配，说明用户名或密码错误 */
    printf("\n[登录失败] 用户名或密码错误！\n");
    pause_screen();
    return 0;
}

/* ============================================
 * 函数: student_login
 * 功能: 考生登录流程
 * 实现: 读取身份证号和密码 → 遍历 students 数组逐条比对 → 匹配则设置全局状态
 * 返回: 1=登录成功, 0=登录失败
 * ============================================ */
static int student_login(void) {
    char id_card[19], password[33];   /* 缓冲区: 身份证号 18 位+\0, 密码 32 位+\0 */
    printf("\n请输入身份证号：");
    scanf("%18s", id_card);            /* 限制 18 字符 */
    while (getchar() != '\n');        /* 清空缓冲区 */
    printf("请输入密码：");
    get_password(password, sizeof(password));  /* 隐藏密码输入 */

    /* 遍历 students 数组，查找匹配的身份证号和密码 */
    for (register int i = 0; i < student_count; i++) {
        if (strcmp(students[i].id_card, id_card) == 0 &&   /* 身份证号匹配 */
            strcmp(students[i].password, password) == 0) {   /* 密码匹配 */
            current_role = 2;                                 /* 设置角色为考生 */
            current_user_id = students[i].id;                  /* 记录当前考生 ID */
            printf("\n[登录成功] 欢迎考生 %s\n", students[i].name);
            pause_screen();
            return 1;
        }
    }
    printf("\n[登录失败] 身份证号或密码错误！\n");
    pause_screen();
    return 0;
}

/* ============================================
 * 函数: admin_menu
 * 功能: 管理员功能菜单，提供考生管理、预约管理、模拟考试三大模块入口
 * 循环: 直到选择 0（注销登录），返回登录菜单
 * 权限: 管理员可查看和操作所有数据
 * ============================================ */
static void admin_menu(void) {
    int choice;
    do {
        clear_screen();
        print_title("管理员功能菜单");
        printf("  [1] 考生信息管理\n");   /* 进入 student_menu()：全权限 CRUD */
        printf("  [2] 预约管理\n");       /* 进入 appointment_menu()：全权限 */
        printf("  [3] 模拟考试\n");       /* 进入 exam_sim_menu()：可查看所有成绩 */
        printf("  [4] 导入题库\n");       /* 从文本文件导入真实题库 */
        printf("  [0] 注销登录\n");       /* 退出到登录菜单 */
        choice = get_choice();
        switch (choice) {
            case 1: student_menu();       /* 调用考生管理模块 */
                    break;              
            case 2: appointment_menu();   /* 调用预约管理模块 */
                    break;  
            case 3: exam_sim_menu();       /* 调用模拟考试模块 */
                    break;     
            case 4:
                /* 导入题库: 读取文本文件并覆盖现有题库 */
                printf("\n[导入题库]\n");
                printf("请输入题库文件路径（默认 data/questions.txt）：");
                char path[256];
                if (fgets(path, sizeof(path), stdin) && path[0] != '\n') 
                {
                    path[strcspn(path, "\n")] = '\0';
                } else {
                            strcpy(path, "data/questions.txt");
                       }
                import_questions_from_txt(path);
                pause_screen();
                break;
            case 0:
                /* 注销: 重置角色状态，返回登录菜单 */
                current_role = 0;
                current_user_id = -1;
                break;
            default:
                printf("无效选择！\n");
                pause_screen();
        }
    } while (choice != 0);
}

/* ============================================
 * 函数: student_menu_main
 * 功能: 考生功能菜单，仅提供与自己相关的功能
 * 循环: 直到选择 0（注销登录），返回登录菜单
 * 权限: 考生只能查看和修改自己的信息、自己的预约、自己的成绩
 * ============================================ */
static void student_menu_main(void) {
    int choice;
    do {
        clear_screen();
        print_title("考生功能菜单");
        printf("  [1] 我的信息管理\n");   /* 查看自己的信息，可修改联系方式和密码 */
        printf("  [2] 我的预约管理\n");   /* 查看自己的预约列表 */
        printf("  [3] 模拟考试\n");       /* 进入模拟考试模块，成绩仅保存自己 */
        printf("  [0] 注销登录\n");       /* 退出到登录菜单 */
        choice = get_choice();
        switch (choice) {
            case 1: my_info(); break;          /* 查看并修改个人信息 */
            case 2: my_appointments(); break;  /* 查看个人预约记录 */
            case 3: exam_sim_menu(); break;     /* 模拟考试（成绩自动关联当前考生） */
            case 0:
                /* 注销: 重置角色状态 */
                current_role = 0;
                current_user_id = -1;
                break;
            default:
                printf("无效选择！\n");
                pause_screen();
        }
    } while (choice != 0);
}
