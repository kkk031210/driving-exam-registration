/*
 * appointment.c
 * 驾考报名管理系统 — 预约管理模块
 * 职责: 考试预约的增删改查，包含容量冲突检测和权限控制
 * 核心校验: 考生存在、日期格式、时段合法、场地存在、容量不超限
 */

#include <stdio.h>          /* 标准输入输出 */
#include <stdlib.h>         /* 标准库 */
#include <string.h>         /* 字符串处理 */
#include "data_structure.h" /* 全局变量和结构体声明 */
#include "common.h"         /* 公共工具函数 */
#include "appointment.h"    /* 本模块的函数声明 */

/* ============================================
 * 函数: appointment_menu
 * 功能: 预约管理的菜单界面（管理员入口，考生通过 my_appointments 查看）
 * 循环显示子菜单，直到用户选择 0（返回上级）
 * ============================================ */
void appointment_menu(void)
{
    int choice;
    do
    {
        clear_screen();
        print_title("预约管理");
        printf("  [1] 预约考试\n");     /* 新增预约 */
        printf("  [2] 查看预约\n");     /* 列表显示（考生仅显示自己的） */
        printf("  [3] 查询预约信息\n"); /* 按单号或身份证号查询 */
        printf("  [4] 取消预约\n");     /* 硬删除 */
        printf("  [5] 更新预约信息\n"); /* 修改日期/时段/场地 */
        printf("  [0] 返回上级菜单\n"); /* 返回管理员菜单 */
        choice = get_choice();
        switch (choice)
        {
        case 1:
            book_appointment();
            break; /* 预约考试 */
        case 2:
            list_appointments();
            break; /* 查看预约列表 */
        case 3:
            query_appointment();
            break; /* 精确查询 */
        case 4:
            cancel_appointment();
            break; /* 取消预约 */
        case 5:
            update_appointment();
            break; /* 修改预约 */
        case 0:
            break; /* 直接返回 */
        default:
            printf("无效选择！\n");
            pause_screen();
        }
    } while (choice != 0);
}

/* ============================================
 * 函数: book_appointment
 * 功能: 预约考试，包含完整的校验链
 * 校验流程:
 *   1. 容量检查（appointments 数组是否满）
 *   2. 依赖检查（必须有考生和场地）
 *   3. 读取考生 ID（管理员输入 / 考生自动取 current_user_id）
 *   4. 校验考生存在
 *   5. 选择科目（1-4）
 *   6. 输入日期并校验格式（YYYY-MM-DD）
 *   7. 选择时段并校验（3 个固定时段）
 *   8. 选择场地并校验存在
 *   9. 容量冲突检测：统计同日期+同时段+同场地的已预约数，不能超过 max_per_slot
 *  10. 保存预约记录
 * ============================================ */
void book_appointment(void)
{
    /* 步骤1: 容量检查 */
    if (appointment_count >= MAX_SIZE)
    {
        printf("\n预约数量已达上限！\n");
        pause_screen();
        return;
    }
    /* 步骤2: 依赖检查：必须有考生才能预约 */
    if (student_count == 0)
    {
        printf("\n暂无考生，无法预约！\n");
        pause_screen();
        return;
    }
    /* 依赖检查：必须有场地才能预约 */
    if (venue_count == 0)
    {
        printf("\n暂无考试场地，无法预约！\n");
        pause_screen();
        return;
    }

    Appointment a;            /* 临时结构体存放新预约 */
    memset(&a, 0, sizeof(a)); /* 清零 */
    a.id = (appointment_count > 0) ? appointments[appointment_count - 1].id + 1 : 1;

    printf("\n[预约考试]\n");

    /* 步骤3: 确定考生 */
    int student_id;
    if (current_role == 2)
    { /* 考生登录：自动取自己的 ID */
        student_id = current_user_id;
        /* 显示当前考生姓名，方便确认（防止误操作） */
        printf("当前考生：%s（ID: %d）\n",
               (student_id > 0 && student_id <= student_count) ? students[student_id - 1].name : "",
               student_id);
    }
    else
    { /* 管理员登录：手动输入考生 ID */
        printf("请输入考生ID：");
        scanf("%d", &student_id);
        while (getchar() != '\n')
            ;
    }
    a.student_id = student_id;

    /* 步骤4: 校验考生存在（遍历 students 数组） */
    int found = 0;
    for (int i = 0; i < student_count; i++)
    {
        if (students[i].id == student_id)
        {
            found = 1;
            break;
        }
    }
    if (!found)
    {
        printf("[错误] 考生ID不存在！\n");
        pause_screen();
        return;
    }

    /* 步骤5: 选择科目 */
    printf("请选择科目：\n");
    printf("  [1] 科目一\n");
    printf("  [2] 科目二\n");
    printf("  [3] 科目三\n");
    printf("  [4] 科目四\n");
    int sub_choice;
    scanf("%d", &sub_choice);
    while (getchar() != '\n')
        ;
    switch (sub_choice)
    { /* 将数字映射为字符串 */
    case 1:
        strcpy(a.subject, "科目一");
        break;
    case 2:
        strcpy(a.subject, "科目二");
        break;
    case 3:
        strcpy(a.subject, "科目三");
        break;
    case 4:
        strcpy(a.subject, "科目四");
        break;
    default:
        printf("[错误] 无效科目！\n");
        pause_screen();
        return;
    }

    /* 步骤6: 输入日期并校验格式 */
    printf("请输入预约日期（YYYY-MM-DD）：");
    scanf("%10s", a.exam_date);
    while (getchar() != '\n')
        ;
    if (!is_date_valid(a.exam_date))
    { /* 调用 common.c 中的日期校验 */
        printf("[错误] 日期格式不正确！\n");
        pause_screen();
        return;
    }

    /* 步骤7: 选择时段并校验 */
    printf("请选择时段：\n");
    printf("  [1] 08:30-10:30\n");
    printf("  [2] 13:00-15:00\n");
    printf("  [3] 15:30-17:30\n");
    int slot_choice;
    scanf("%d", &slot_choice);
    while (getchar() != '\n')
        ;
    switch (slot_choice)
    {
    case 1:
        strcpy(a.exam_time, "08:30-10:30");
        break;
    case 2:
        strcpy(a.exam_time, "13:00-15:00");
        break;
    case 3:
        strcpy(a.exam_time, "15:30-17:30");
        break;
    default:
        printf("[错误] 无效时段！\n");
        pause_screen();
        return;
    }

    /* 步骤8: 选择场地并校验存在 */
    printf("\n可用场地：\n");
    for (int i = 0; i < venue_count; i++)
    {
        printf("  [%d] %s（容量：%d/场次）\n",
               venues[i].id, venues[i].name, venues[i].max_per_slot);
    }
    printf("请输入场地ID：");
    scanf("%d", &a.venue_id);
    while (getchar() != '\n')
        ;

    int venue_idx = -1; /* 场地在数组中的索引 */
    for (int i = 0; i < venue_count; i++)
    {
        if (venues[i].id == a.venue_id)
        {
            venue_idx = i;
            break;
        }
    }
    if (venue_idx == -1)
    { /* 未找到该场地 */
        printf("[错误] 场地ID不存在！\n");
        pause_screen();
        return;
    }

    /* 步骤9: 容量冲突检测
     * 统计同一日期 + 同一时段 + 同一场地 的已预约记录数
     * 若已预约数 ≥ 场地最大容量，则拒绝预约
     */
    int current_count = 0;
    for (int i = 0; i < appointment_count; i++)
    {
        if (strcmp(appointments[i].exam_date, a.exam_date) == 0 && /* 日期相同 */
            strcmp(appointments[i].exam_time, a.exam_time) == 0 && /* 时段相同 */
            appointments[i].venue_id == a.venue_id)
        { /* 场地相同 */
            current_count++;
        }
    }
    if (current_count >= venues[venue_idx].max_per_slot)
    {
        printf("[错误] 该时段该场地已约满！\n");
        pause_screen();
        return;
    }

    /* 步骤10: 保存预约 */
    strcpy(a.status, "已预约");            /* 设置默认状态 */
    appointments[appointment_count++] = a; /* 追加到数组 */
    printf("\n[成功] 预约成功，预约单号：%d\n", a.id);
    pause_screen();
}

/* ============================================
 * 函数: list_appointments
 * 功能: 浏览预约列表
 * 权限控制:
 *   - 管理员: 显示所有预约
 *   - 考生: 仅显示 student_id == current_user_id 的预约
 * 格式: 单号 | 考生ID | 科目 | 日期 | 时段 | 场地ID | 状态
 * ============================================ */
void list_appointments(void)
{
    clear_screen();
    print_title("预约信息");
    if (appointment_count == 0)
    { /* 空数据检查 */
        printf("暂无预约记录。\n");
        pause_screen();
        return;
    }
    int has_data = 0; /* 标记是否有数据可显示（考生权限下可能全被过滤） */
    /* 打印表头 */
    printf("  %-4s %-6s %-10s %-12s %-12s %-10s %-10s\n",
           "单号", "考生ID", "科目", "日期", "时段", "场地ID", "状态");
    print_line();
    for (int i = 0; i < appointment_count; i++)
    {
        /* 权限过滤: 考生只能看自己的预约 */
        if (current_role == 2 && appointments[i].student_id != current_user_id)
            continue;
        printf("  %-4d %-6d %-10s %-12s %-12s %-10d %-10s\n",
               appointments[i].id, appointments[i].student_id,
               appointments[i].subject, appointments[i].exam_date,
               appointments[i].exam_time, appointments[i].venue_id,
               appointments[i].status);
        has_data = 1;
    }
    if (!has_data)
        printf("暂无您的预约记录。\n"); /* 考生视角的空数据提示 */
    pause_screen();
}

/* ============================================
 * 函数: query_appointment
 * 功能: 查询预约信息，支持按单号或身份证号查询
 * 权限控制:
 *   - 按单号查询时，考生只能查自己的预约
 *   - 按身份证号查询时，考生只能查自己的身份证号
 * 流程: 先选择查询方式 → 输入条件 → 匹配并显示 → 权限校验
 * ============================================ */
void query_appointment(void)
{
    int choice;
    printf("\n[查询预约信息]\n");
    printf("  [1] 按预约单号查询\n");
    printf("  [2] 按考生身份证号查询\n");
    printf("  [0] 返回\n");
    choice = get_choice();
    if (choice == 0)
        return; /* 用户选择返回 */

    if (choice == 1)
    {
        /* 按单号查询 */
        int id;
        printf("请输入预约单号：");
        scanf("%d", &id);
        while (getchar() != '\n')
            ;
        int found = 0;
        for (int i = 0; i < appointment_count; i++)
        {
            if (appointments[i].id == id)
            {
                /* 权限校验: 考生只能看自己的预约 */
                if (current_role == 2 && appointments[i].student_id != current_user_id)
                {
                    printf("[错误] 无权查看该预约！\n");
                    pause_screen();
                    return;
                }
                /* 显示匹配记录 */
                printf("  单号: %d, 考生ID: %d, 科目: %s\n",
                       appointments[i].id, appointments[i].student_id, appointments[i].subject);
                printf("  日期: %s, 时段: %s, 场地ID: %d, 状态: %s\n",
                       appointments[i].exam_date, appointments[i].exam_time,
                       appointments[i].venue_id, appointments[i].status);
                found = 1;
                break; /* 单号唯一，找到即退出 */
            }
        }
        if (!found)
            printf("未找到该预约单号。\n");
    }
    else if (choice == 2)
    {
        /* 按身份证号查询 */
        char id_card[19];
        printf("请输入考生身份证号：");
        scanf("%18s", id_card);
        while (getchar() != '\n')
            ;

        /* 根据身份证号查找对应的考生 ID */
        int sid = -1;
        for (int i = 0; i < student_count; i++)
        {
            if (strcmp(students[i].id_card, id_card) == 0)
            {
                sid = students[i].id; /* 获取考生 ID */
                break;
            }
        }
        if (sid == -1)
        { /* 未找到该考生 */
            printf("未找到该考生。\n");
            pause_screen();
            return;
        }
        /* 权限校验: 考生只能查自己的预约 */
        if (current_role == 2 && sid != current_user_id)
        {
            printf("[错误] 无权查看他人预约！\n");
            pause_screen();
            return;
        }

        /* 遍历预约数组，显示该考生的所有预约 */
        int found = 0;
        for (int i = 0; i < appointment_count; i++)
        {
            if (appointments[i].student_id == sid)
            {
                printf("  单号: %d, 科目: %s, 日期: %s, 时段: %s, 场地: %d, 状态: %s\n",
                       appointments[i].id, appointments[i].subject,
                       appointments[i].exam_date, appointments[i].exam_time,
                       appointments[i].venue_id, appointments[i].status);
                found++;
            }
        }
        if (!found)
            printf("该考生暂无预约记录。\n");
        else
            printf("共找到 %d 条预约。\n", found);
    }
    pause_screen();
}

/* ============================================
 * 函数: cancel_appointment
 * 功能: 取消预约（硬删除）
 * 权限控制: 考生只能取消自己的预约
 * 删除方式: 从数组中移除该记录，后续元素前移，count--
 * 确认对话框: 防止误操作
 * ============================================ */
void cancel_appointment(void)
{
    if (appointment_count == 0)
    { /* 空数据检查 */
        printf("\n暂无预约记录。\n");
        pause_screen();
        return;
    }

    int id; /* 目标预约单号 */
    printf("\n[取消预约]\n");
    printf("请输入预约单号：");
    scanf("%d", &id);
    while (getchar() != '\n')
        ;

    /* 查找目标预约索引 */
    int idx = -1;
    for (int i = 0; i < appointment_count; i++)
    {
        if (appointments[i].id == id)
        {
            idx = i;
            break;
        }
    }
    if (idx == -1) /* 未找到 */
    {
        printf("[错误] 未找到该预约单号！\n");
        pause_screen();
        return;
    }

    /* 权限校验: 考生只能取消自己的预约 */
    if (current_role == 2 && appointments[idx].student_id != current_user_id)
    {
        printf("[错误] 您只能取消自己的预约！\n");
        pause_screen();
        return;
    }

    /* 确认对话框 */
    printf("确认取消预约单号 %d（科目：%s，日期：%s）？（1=确认，0=取消）：",
           id, appointments[idx].subject, appointments[idx].exam_date);
    int confirm;
    scanf("%d", &confirm);
    while (getchar() != '\n')
        ;
    if (confirm != 1)
    { /* 用户取消操作 */
        printf("已取消操作。\n");
        pause_screen();
        return;
    }

    /* 硬删除: 将 idx 之后的所有元素前移一位，覆盖被删除的记录 */
    for (int i = idx; i < appointment_count - 1; i++)
    {
        appointments[i] = appointments[i + 1];
    }
    appointment_count--; /* 总数减一 */

    printf("\n[成功] 预约已取消。\n");
    pause_screen();
}

/* ============================================
 * 函数: update_appointment
 * 功能: 修改预约信息（日期、时段、场地）
 * 权限控制: 考生只能更新自己的预约
 * 修改后重新校验容量冲突，防止修改后超载
 * ============================================ */
void update_appointment(void)
{
    if (appointment_count == 0)
    { /* 空数据检查 */
        printf("\n暂无预约记录。\n");
        pause_screen();
        return;
    }

    int id; /* 目标预约单号 */
    printf("\n[更新预约信息]\n");
    printf("请输入预约单号：");
    scanf("%d", &id);
    while (getchar() != '\n')
        ;

    /* 查找目标预约索引 */
    int idx = -1;
    for (int i = 0; i < appointment_count; i++)
    {
        if (appointments[i].id == id)
        {
            idx = i;
            break;
        }
    }
    if (idx == -1)
    { /* 未找到 */
        printf("[错误] 未找到该预约单号！\n");
        pause_screen();
        return;
    }

    /* 权限校验: 考生只能更新自己的预约 */
    if (current_role == 2 && appointments[idx].student_id != current_user_id)
    {
        printf("[错误] 您只能更新自己的预约！\n");
        pause_screen();
        return;
    }

    /* 显示当前预约信息，供参考 */
    printf("当前预约：\n");
    printf("  科目: %s, 日期: %s, 时段: %s, 场地ID: %d\n",
           appointments[idx].subject, appointments[idx].exam_date,
           appointments[idx].exam_time, appointments[idx].venue_id);

    /* 选择修改项目 */
    printf("\n请选择修改项目：\n");
    printf("  [1] 修改日期\n");
    printf("  [2] 修改时段\n");
    printf("  [3] 修改场地\n");
    printf("  [0] 返回\n");
    int choice = get_choice();

    if (choice == 1)
    {
        /* 修改日期 */
        printf("请输入新日期（YYYY-MM-DD）：");
        char date[11];
        scanf("%10s", date);
        while (getchar() != '\n')
            ;
        if (!is_date_valid(date))
        { /* 格式校验 */
            printf("[错误] 日期格式不正确！\n");
            pause_screen();
            return;
        }
        strcpy(appointments[idx].exam_date, date);
    }
    else if (choice == 2)
    {
        /* 修改时段 */
        printf("请选择新时段：\n");
        printf("  [1] 08:30-10:30\n");
        printf("  [2] 13:00-15:00\n");
        printf("  [3] 15:30-17:30\n");
        int s;
        scanf("%d", &s);
        while (getchar() != '\n')
            ;
        switch (s)
        {
            case 1:
                strcpy(appointments[idx].exam_time, "08:30-10:30");
                break;
            case 2:
                strcpy(appointments[idx].exam_time, "13:00-15:00");
                break;
            case 3:
                strcpy(appointments[idx].exam_time, "15:30-17:30");
                break;
            default:
                printf("[错误] 无效时段！\n");
                pause_screen();
                return;
        }
    }
    else if (choice == 3)
    {
        /* 修改场地 */
        printf("请输入新场地ID：");
        int vid;
        scanf("%d", &vid);
        while (getchar() != '\n');
        /* 校验新场地是否存在 */
        int found = 0;
        for (int i = 0; i < venue_count; i++)
        {
            if (venues[i].id == vid)
            {
                found = 1;
                break;
            }
        }
        if (!found)
        {
            printf("[错误] 场地ID不存在！\n");
            pause_screen();
            return;
        }
        appointments[idx].venue_id = vid;
    }
    else
    {
        return; /* 用户选择返回，不做任何修改 */
    }

    /* 修改后重新检查容量冲突
     * 统计除当前记录外的同日期+同时段+同场地预约数
     * 若已预约数 ≥ 场地容量，则拒绝修改
     */
    int current_count = 0;
    for (int i = 0; i < appointment_count; i++)
    {
        if (i == idx)
            continue; /* 跳过当前正在被修改的记录 */
        if (strcmp(appointments[i].exam_date, appointments[idx].exam_date) == 0 &&
            strcmp(appointments[i].exam_time, appointments[idx].exam_time) == 0 &&
            appointments[i].venue_id == appointments[idx].venue_id)
        {
            current_count++;
        }
    }
    int venue_idx = -1; /* 查找新场地在数组中的索引，获取容量 */
    for (int i = 0; i < venue_count; i++)
    {
        if (venues[i].id == appointments[idx].venue_id)
        {
            venue_idx = i;
            break;
        }
    }
    if (venue_idx >= 0 && current_count >= venues[venue_idx].max_per_slot)
    {
        printf("[错误] 修改后该时段场地已满，请重新选择！\n");
        pause_screen();
        return;
    }

    printf("\n[成功] 预约信息已更新。\n");
    pause_screen();
}

/* ============================================
 * 函数: my_appointments
 * 功能: 考生查看自己的预约列表（考生菜单入口）
 * 显示: 单号、科目、日期、时段、场地ID、状态
 * 仅显示 student_id == current_user_id 的记录
 * ============================================ */
void my_appointments(void)
{
    clear_screen();
    print_title("我的预约");
    if (appointment_count == 0)
    { /* 空数据检查 */
        printf("暂无预约记录。\n");
        pause_screen();
        return;
    }
    int found = 0; /* 匹配计数器 */
    /* 打印表头 */
    printf("  %-4s %-10s %-12s %-12s %-10s %-10s\n",
           "单号", "科目", "日期", "时段", "场地ID", "状态");
    print_line();
    for (int i = 0; i < appointment_count; i++)
    {
        if (appointments[i].student_id == current_user_id)  /* 仅显示自己的 */
        { 
            printf("  %-4d %-10s %-12s %-12s %-10d %-10s\n",
                   appointments[i].id, appointments[i].subject,
                   appointments[i].exam_date, appointments[i].exam_time,
                   appointments[i].venue_id, appointments[i].status);
            found++;
        }
    }
    if (!found)
    {
        printf("暂无您的预约记录。\n");
    }
    else
    {
        printf("\n共 %d 条预约。\n", found);
    }
    pause_screen();
}
