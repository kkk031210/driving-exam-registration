/*
 * student.c
 * 驾考报名管理系统 — 考生管理模块
 * 职责: 考生的增删改查 (CRUD)，以及考生权限下的个人信息查看
 * 权限控制: 管理员可操作所有考生；考生只能查看/修改自己的信息
 */

#include <stdio.h>          /* 标准输入输出 */
#include <stdlib.h>         /* 标准库 */
#include <string.h>         /* 字符串处理: memset, strcmp, strcpy, strstr, fgets */
#include "data_structure.h" /* 全局变量和结构体声明 */
#include "common.h"         /* 公共工具函数 */
#include "student.h"        /* 本模块的函数声明 */

/* ============================================
 * 函数: student_menu
 * 功能: 考生信息管理的菜单界面（管理员入口）
 * 循环显示子菜单，直到用户选择 0（返回上级）
 * 各选项调用对应的 CRUD 函数
 * ============================================ */
void student_menu(void) {
    int choice;
    do {
        clear_screen();                 /* 清屏，保持界面整洁 */
        print_title("考生信息管理");     /* 打印菜单标题 */
        printf("  [1] 添加考生信息\n"); /* 新增考生 */
        printf("  [2] 浏览所有考生\n"); /* 列表显示所有考生 */
        printf("  [3] 查询考生信息\n"); /* 按条件模糊查询 */
        printf("  [4] 修改考生信息\n"); /* 编辑考生资料 */
        printf("  [5] 删除考生信息\n"); /* 删除考生（级联删除关联数据） */
        printf("  [0] 返回上级菜单\n"); /* 返回管理员菜单 */
        choice = get_choice();         /* 读取用户输入的选项 */
        switch (choice) {
            case 1: add_student(); break;      /* 调用添加考生函数 */
            case 2: list_students(); break;    /* 调用浏览考生函数 */
            case 3: query_student(); break;    /* 调用查询考生函数 */
            case 4: modify_student(); break;   /* 调用修改考生函数 */
            case 5: delete_student(); break;   /* 调用删除考生函数 */
            case 0: break;                     /* 直接返回 */
            default: printf("无效选择！\n"); pause_screen(); /* 错误处理 */
        }
    } while (choice != 0);              /* 循环直到选择 0 */
}

/* ============================================
 * 函数: add_student
 * 功能: 添加新考生信息
 * 流程:
 *   1. 检查数组是否已满
 *   2. 自动生成 ID（最后一条记录的 ID + 1，或从 1 开始）
 *   3. 读取身份证号并校验格式（18位，前17位数字，末位数字或X）
 *   4. 检查身份证号是否已存在（防止重复录入）
 *   5. 读取姓名、电话、车型、密码
 *   6. 默认状态设为"已报名"
 *   7. 追加到数组末尾，count++
 * ============================================ */
void add_student(void) {
    /* 步骤1: 容量检查 */
    if (student_count >= MAX_SIZE) {    /* 数组已满，无法添加 */
        printf("\n[错误] 考生数量已达上限！\n");
        pause_screen();
        return;
    }

    Student s;                          /* 临时结构体，存放新考生数据 */
    memset(&s, 0, sizeof(s));            /* 清零，避免残留数据 */
    /* 步骤2: ID 自增策略
     * 若有记录则取最后一个的 ID+1，否则从 1 开始
     * 注意: 删除后不回收 ID，保证外键关联安全
     */
    s.id = (student_count > 0) ? students[student_count - 1].id + 1 : 1;

    printf("\n[添加考生信息]\n");
    /* 步骤3: 读取并校验身份证号 */
    printf("请输入身份证号：");
    scanf("%18s", s.id_card);            /* 限制 18 字符，防止溢出 */
    while (getchar() != '\n');           /* 清空缓冲区残留换行符 */

    if (!validate_id_card(s.id_card)) {  /* 格式校验 */
        printf("[错误] 身份证号格式不正确！\n");
        pause_screen();
        return;
    }
    /* 步骤4: 唯一性检查：遍历已有考生，比对身份证号 */
    for (int i = 0; i < student_count; i++) {
        if (strcmp(students[i].id_card, s.id_card) == 0) {
            printf("[错误] 该身份证号已存在！\n");
            pause_screen();
            return;
        }
    }

    /* 步骤5: 读取其他字段 */
    printf("请输入姓名：");
    scanf("%50s", s.name);
    while (getchar() != '\n');

    printf("请输入联系电话：");
    scanf("%15s", s.phone);
    while (getchar() != '\n');

    printf("请输入报考车型（C1/C2/A1/B2）：");
    scanf("%4s", s.car_type);
    while (getchar() != '\n');

    printf("请输入登录密码：");
    get_password(s.password, sizeof(s.password));  /* 隐藏输入 */

    /* 步骤6: 设置默认状态 */
    strcpy(s.status, "已报名");
    /* 步骤7: 追加到数组 */
    students[student_count++] = s;     /* 存入数组，计数器自增 */
    printf("\n[成功] 考生 %s 添加成功，编号 %d\n", s.name, s.id);
    pause_screen();
}

/* ============================================
 * 函数: list_students
 * 功能: 浏览所有考生信息，以表格形式打印
 * 格式: ID | 身份证号 | 姓名 | 电话 | 车型 | 状态
 * 注意: 若暂无记录则提示并返回
 * ============================================ */
void list_students(void) {
    clear_screen();
    print_title("所有考生信息");
    if (student_count == 0) {           /* 空数据检查 */
        printf("暂无考生记录。\n");
        pause_screen();
        return;
    }
    /* 打印表头 */
    printf("  %-4s %-18s %-10s %-12s %-8s %-10s\n",
           "ID", "身份证号", "姓名", "电话", "车型", "状态");
    print_line();                       /* 分隔线 */
    /* 逐行打印所有考生 */
    for (int i = 0; i < student_count; i++) {
        printf("  %-4d %-18s %-10s %-12s %-8s %-10s\n",
               students[i].id, students[i].id_card,
               students[i].name, students[i].phone,
               students[i].car_type, students[i].status);
    }
    pause_screen();
}

/* ============================================
 * 函数: query_student
 * 功能: 按身份证号或姓名模糊查询考生
 * 实现: 使用 strstr() 进行子串匹配，支持部分输入
 * 输出: 匹配的考生列表及总数
 * ============================================ */
void query_student(void) {
    char keyword[51];                   /* 查询关键词缓冲区 */
    printf("\n[查询考生信息]\n");
    printf("请输入身份证号或姓名：");
    scanf("%50s", keyword);
    while (getchar() != '\n');

    int found = 0;                      /* 匹配计数器 */
    /* 打印表头 */
    printf("\n  %-4s %-18s %-10s %-12s %-8s %-10s\n",
           "ID", "身份证号", "姓名", "电话", "车型", "状态");
    print_line();
    /* 遍历考生数组，使用 strstr 进行模糊匹配 */
    for (int i = 0; i < student_count; i++) 
    {
        if (strstr(students[i].id_card, keyword) ||  /* 身份证号包含关键词 */
            strstr(students[i].name, keyword))       /* 或姓名包含关键词 */
        {      
            printf("  %-4d %-18s %-10s %-12s %-8s %-10s\n",
                   students[i].id, students[i].id_card,
                   students[i].name, students[i].phone,
                   students[i].car_type, students[i].status);
            found++;
        }
    }
    if (!found) 
        printf("未找到匹配的考生。\n");
    else 
        printf("\n共找到 %d 条记录。\n", found);
    
    pause_screen();
}

/* ============================================
 * 函数: modify_student
 * 功能: 修改考生信息
 * 权限控制:
 *   - 管理员: 可输入任意考生 ID 进行修改
 *   - 考生: 只能修改自己的信息（ID 强制为 current_user_id）
 * 修改策略: 逐字段提示，直接回车保持原值
 * 可修改字段: 联系电话、报考车型、密码
 * ============================================ */
void modify_student(void) {
    if (student_count == 0) {           /* 空数据检查 */
        printf("\n暂无考生记录。\n");
        pause_screen();
        return;
    }

    int id;                             /* 目标考生 ID */
    /* 权限判断: 考生只能修改自己，管理员可修改任意 */
    if (current_role == 2) {           /* 考生登录 */
        id = current_user_id;            /* 强制使用当前登录 ID */
        printf("\n[修改我的信息]\n");
    } else {                            /* 管理员登录 */
        printf("\n[修改考生信息]\n");
        printf("请输入考生ID：");
        scanf("%d", &id);
        while (getchar() != '\n');
    }

    /* 在数组中查找目标考生 */
    int idx = -1;                       /* 目标索引 */
    for (int i = 0; i < student_count; i++) 
    {
        if (students[i].id == id) 
        {   idx = i; 
            break; 
        }
    }
    if (idx == -1)                         /* 未找到 */
    {                   
        printf("[错误] 未找到该考生！\n");
        pause_screen();
        return;
    }

    /* 二次权限校验（防止考生手动输入他人 ID） */
    if (current_role == 2 && students[idx].id != current_user_id) 
    {
        printf("[错误] 您只能修改自己的信息！\n");
        pause_screen();
        return;
    }

    /* 显示当前信息，供用户参考 */
    printf("当前信息：\n");
    printf("  姓名: %s, 电话: %s, 车型: %s\n",
           students[idx].name, students[idx].phone, students[idx].car_type);

    /* 修改电话: 使用 fgets 读取整行，首字符为换行则跳过（保持原值） */
    printf("\n请输入新的联系电话（直接回车保持原值）：");
    char phone[16];
    if (fgets(phone, sizeof(phone), stdin) && phone[0] != '\n') {
        phone[strcspn(phone, "\n")] = '\0';  /* 去掉末尾换行符 */
        strcpy(students[idx].phone, phone); /* 更新字段 */
    }

    /* 修改车型 */
    printf("请输入新的报考车型（直接回车保持原值）：");
    char car_type[5];
    if (fgets(car_type, sizeof(car_type), stdin) && car_type[0] != '\n') {
        car_type[strcspn(car_type, "\n")] = '\0';
        strcpy(students[idx].car_type, car_type);
    }

    /* 修改密码: 使用 get_password（隐藏输入），非空则更新 */
    printf("请输入新的密码（直接回车保持原值）：");
    char pwd[33];
    get_password(pwd, sizeof(pwd));
    if (strlen(pwd) > 0) {              /* 用户确实输入了内容 */
        strcpy(students[idx].password, pwd);
    }

    printf("\n[成功] 考生信息已更新。\n");
    pause_screen();
}

/* ============================================
 * 函数: delete_student
 * 功能: 删除考生信息（硬删除）
 * 级联删除:
 *   1. 遍历 appointments，删除所有 student_id 匹配的记录
 *   2. 遍历 scores，删除所有 student_id 匹配的记录
 *   3. 从 students 数组中移除目标考生，前移覆盖
 * 删除策略: 确认对话框，防止误操作
 * ============================================ */
void delete_student(void) {
    if (student_count == 0)             /* 空数据检查 */
    {           
        printf("\n暂无考生记录。\n");
        pause_screen();
        return;
    }

    int id;                             /* 目标考生 ID */
    printf("\n[删除考生信息]\n");
    printf("请输入考生ID：");
    scanf("%d", &id);
    while (getchar() != '\n');

    /* 查找目标考生索引 */
    int idx = -1;
    for (int i = 0; i < student_count; i++) 
    {
        if (students[i].id == id) 
        {   
            idx = i; 
            break; 
        }
    }
    if (idx == -1)                       /* 未找到 */
    {                   
        printf("[错误] 未找到该考生！\n");
        pause_screen();
        return;
    }

    /* 确认对话框 */
    printf("确认删除考生 %s（ID: %d）？（1=确认，0=取消）：",
           students[idx].name, id);
    int confirm;
    scanf("%d", &confirm);
    while (getchar() != '\n');
    if (confirm != 1)                    /* 用户取消 */
    {                 
        printf("已取消删除。\n");
        pause_screen();
        return;
    }

    /* 级联删除 1: 遍历 appointments 数组，从后往前删除关联记录
     * 从后往前遍历的原因: 删除后需前移后续元素，从后往前不会遗漏
     */
    for (int i = appointment_count - 1; i >= 0; i--) 
    {
        if (appointments[i].student_id == id) 
        {
            /* 前移覆盖: 将 i 之后的所有元素前移一位 */
            for (int j = i; j < appointment_count - 1; j++) 
            {
                appointments[j] = appointments[j + 1];
            }
            appointment_count--;        /* 总数量减一 */
        }
    }
    /* 级联删除 2: 遍历 scores 数组，删除关联记录 */
    for (int i = score_count - 1; i >= 0; i--) 
    {
        if (scores[i].student_id == id) 
        {
            for (int j = i; j < score_count - 1; j++) 
            {
                scores[j] = scores[j + 1];
            }
            score_count--;
        }
    }
    /* 步骤 3: 删除考生本身，前移覆盖 */
    for (int i = idx; i < student_count - 1; i++) 
    {
        students[i] = students[i + 1];   /* 后一个元素覆盖当前 */
    }
    student_count--;                     /* 总数量减一 */

    printf("\n[成功] 考生及相关预约、成绩已删除。\n");
    pause_screen();
}

/* ============================================
 * 函数: my_info
 * 功能: 考生查看自己的个人信息（考生菜单入口）
 * 显示: ID、姓名、身份证号、电话、车型、状态
 * 提供修改入口: 选择 [1] 调用 modify_student()（已限制只能改自己）
 * ============================================ */
void my_info(void) {
    /* 根据 current_user_id 查找自己的索引 */
    int idx = -1;
    for (int i = 0; i < student_count; i++) {
        if (students[i].id == current_user_id) { idx = i; break; }
    }
    if (idx == -1) {                    /* 异常: 未找到 */
        printf("\n[错误] 未找到您的信息！\n");
        pause_screen();
        return;
    }

    clear_screen();
    print_title("我的信息");
    /* 逐项打印个人信息 */
    printf("  ID:        %d\n", students[idx].id);
    printf("  姓名:      %s\n", students[idx].name);
    printf("  身份证号:  %s\n", students[idx].id_card);
    printf("  电话:      %s\n", students[idx].phone);
    printf("  报考车型:  %s\n", students[idx].car_type);
    printf("  状态:      %s\n", students[idx].status);

    /* 提供修改选项 */
    printf("\n  [1] 修改信息\n");
    printf("  [0] 返回\n");
    int choice = get_choice();
    if (choice == 1) 
    {
        modify_student();              /* 进入修改流程（已限制权限） */
    }
}
