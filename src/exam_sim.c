/*
 * exam_sim.c
 * 驾考报名管理系统 — 模拟考试模块
 * 职责: 科目一/科目四模拟考试、随机抽题组卷、逐题作答、自动判分、保存成绩
 * 抽题算法: Fisher-Yates 洗牌，保证随机性和不重复
 */

#include <stdio.h>          /* 标准输入输出 */
#include <stdlib.h>         /* 标准库: malloc, free, rand */
#include <string.h>         /* 字符串处理: memset, strcmp, strcpy */
#include <time.h>           /* 时间函数: time, time_t */
#include "data_structure.h" /* 全局变量和结构体声明 */
#include "common.h"         /* 公共工具函数 */
#include "exam_sim.h"       /* 本模块的函数声明 */

/* 静态函数声明：exam() 仅在本文件内使用，不对外暴露 */
static int exam(int subject, int total_questions, int points_per_question);

/* ============================================
 * 函数: exam_sim_menu
 * 功能: 模拟考试模块的菜单界面
 * 循环显示子菜单，直到用户选择 0（返回上级）
 * ============================================ */
void exam_sim_menu(void) {
    int choice;
    do {
        clear_screen();
        print_title("模拟考试");
        printf("  [1] 科目一模拟考试（100题）\n");  /* 每题1分，总分100 */
        printf("  [2] 科目四模拟考试（50题）\n");   /* 每题2分，总分100 */
        printf("  [3] 查看模拟成绩\n");            /* 成绩记录列表 */
        printf("  [0] 返回上级菜单\n");            /* 返回角色菜单 */
        choice = get_choice();
        switch (choice) {
            case 1: exam(SUBJECT_1, 100, 1); break;  /* 科目一: 100题, 每题1分 */
            case 2: exam(SUBJECT_4, 50, 2); break;   /* 科目四: 50题, 每题2分 */
            case 3: list_scores(); break;          /* 查看成绩 */
            case 0: break;                          /* 直接返回 */
            default: printf("无效选择！\n"); pause_screen();
        }
    } while (choice != 0);
}

/* ============================================
 * 函数: exam（静态，仅本文件使用）
 * 功能: 执行一次完整的模拟考试流程
 * 参数:
 *   subject             — 科目编号（1=科目一, 4=科目四）
 *   total_questions     — 总题数（科目一100，科目四50）
 *   points_per_question — 每题分值（科目一1分，科目四2分）
 * 返回: 1=考试完成, 0=异常退出（题库为空或无对应科目题目）
 * 流程:
 *   1. 检查题库是否为空
 *   2. 统计该科目可用题目数
 *   3. 若可用题数不足，自动调整 total_questions
 *   4. 收集该科目所有题目索引到临时数组
 *   5. Fisher-Yates 洗牌算法随机打乱题目顺序
 *   6. 逐题显示并等待作答
 *   7. 自动判分：正确直接下一题；错误显示解析
 *   8. 计算得分、用时，保存成绩记录
 * ============================================ */
static int exam(int subject, int total_questions, int points_per_question) {
    /* 步骤1: 检查题库是否为空 */
    if (question_count == 0) {
        printf("\n[错误] 题库为空，无法进行模拟考试！\n");
        pause_screen();
        return 0;
    }

    /* 步骤2: 统计该科目可用题目数 */
    int available = 0;
    for (int i = 0; i < question_count; i++) {
        if (questions[i].subject == subject) available++;
    }
    if (available == 0) {               /* 该科目无题 */
        printf("\n[错误] 该科目暂无题目！\n");
        pause_screen();
        return 0;
    }

    /* 步骤3: 若题库题目少于请求数，自动调整 */
    if (total_questions > available) total_questions = available;

    /* 步骤4: 收集该科目所有题目的索引到临时数组
     * 不直接复制题目内容，只保存索引，节省内存
     */
    int *indices = malloc(sizeof(int) * available);  /* 动态分配临时数组 */
    int idx = 0;
    for (int i = 0; i < question_count; i++) {
        if (questions[i].subject == subject) indices[idx++] = i;
    }

    /* 步骤5: Fisher-Yates 洗牌算法（随机打乱数组）
     * 从最后一个元素开始，随机选择一个前面的元素交换
     * 保证每个排列的概率相等，且不会重复抽取同一题
     */
    for (int i = available - 1; i > 0; i--) {
        int j = rand() % (i + 1);       /* 随机选择 0~i 之间的索引 */
        int temp = indices[i];           /* 交换 indices[i] 和 indices[j] */
        indices[i] = indices[j];
        indices[j] = temp;
    }

    /* 打印考试开始信息 */
    printf("\n════════════════════════════════════\n");
    printf("      %s模拟考试\n", (subject == SUBJECT_1) ? "科目一" : "科目四");
    printf("════════════════════════════════════\n");
    printf("共 %d 题，每题 %d 分\n", total_questions, points_per_question);
    printf("按 Enter 键开始...\n");
    getchar();                          /* 等待用户确认 */

    /* 步骤6: 开始计时和答题循环 */
    time_t start_time = time(NULL);      /* 记录开始时间戳 */
    int correct = 0;                     /* 正确题数计数器 */
    char answer[10];                     /* 用户输入的答案缓冲区 */

    for (int i = 0; i < total_questions; i++) {
        Question *q = &questions[indices[i]];  /* 通过索引取到题目指针 */
        clear_screen();                      /* 每题清屏，保持界面整洁 */
        printf("════════════════════════════════════\n");
        printf("  第 %d / %d 题\n", i + 1, total_questions);  /* 显示进度 */
        printf("════════════════════════════════════\n");
        printf("%s\n\n", q->content);       /* 显示题目内容 */

        /* 根据题型显示选项：判断题只显示 A/B，单选题显示 A/B/C/D */
        if (q->type == TYPE_JUDGE) {
            printf("  A. %s\n", q->options[0]);  /* 判断题: 正确/错误 */
            printf("  B. %s\n", q->options[1]);
        } else {
            printf("  A. %s\n", q->options[0]);
            printf("  B. %s\n", q->options[1]);
            printf("  C. %s\n", q->options[2]);
            printf("  D. %s\n", q->options[3]);
        }

        /* 提示用户输入答案，根据题型给出不同提示 */
        printf("\n请输入答案（");
        if (q->type == TYPE_JUDGE) printf("A/B");
        else if (q->type == TYPE_SINGLE) printf("A/B/C/D");
        else printf("多选如AB，无需分隔");
        printf("）：");

        scanf("%9s", answer);           /* 读取答案（最多9字符，防止溢出） */
        while (getchar() != '\n');      /* 清空缓冲区 */

        /* 大小写统一转换：将 a-z 转为 A-Z，方便与正确答案比对 */
        for (int k = 0; answer[k]; k++) {
            if (answer[k] >= 'a' && answer[k] <= 'z') answer[k] -= 32;
        }

        /* 判分 */
        if (strcmp(answer, q->answer) == 0) {
            correct++;                  /* 答案正确，计数器加一 */
        } else {
            /* 答错时显示解析和正确答案，帮助学习 */
            printf("\n答案解析：%s\n", q->explanation);
            printf("正确答案：%s\n", q->answer);
            printf("按 Enter 继续...");
            getchar();                  /* 等待用户阅读后按 Enter */
        }
    }

    /* 步骤7: 计算结果 */
    time_t end_time = time(NULL);        /* 记录结束时间 */
    int used_seconds = (int)(end_time - start_time);  /* 总用时（秒） */
    int score = correct * points_per_question;        /* 实际得分 */
    int total_score = total_questions * points_per_question;  /* 满分 */

    /* 打印考试结果 */
    clear_screen();
    printf("════════════════════════════════════\n");
    printf("      考试结果\n");
    printf("════════════════════════════════════\n");
    printf("  总题数：    %d\n", total_questions);
    printf("  正确数：    %d\n", correct);
    printf("  错误数：    %d\n", total_questions - correct);
    printf("  得分：      %d / %d\n", score, total_score);
    printf("  用时：      %d分%d秒\n", used_seconds / 60, used_seconds % 60);

    /* 步骤8: 保存成绩记录到 scores 数组 */
    if (score_count < MAX_SIZE) {       /* 检查数组是否已满 */
        ExamScore es;
        memset(&es, 0, sizeof(es));      /* 清零结构体 */
        es.id = (score_count > 0) ? scores[score_count - 1].id + 1 : 1;  /* ID 自增 */
        /* 关联考生: 若管理员登录则记 0，考生登录则记 current_user_id */
        es.student_id = (current_role == 2) ? current_user_id : 0;
        es.subject = subject;            /* 1 或 4 */
        es.score = score;                /* 实际得分 */
        es.total = total_score;            /* 总分 */
        es.correct = correct;            /* 正确题数 */
        es.used_time = used_seconds;     /* 用时（秒） */
        get_current_date(es.exam_date, sizeof(es.exam_date));  /* 获取当前时间字符串 */
        scores[score_count++] = es;      /* 追加到数组，计数器加一 */
        printf("\n[成功] 成绩已保存。\n");
    }

    free(indices);                       /* 释放临时数组内存，防止内存泄漏 */
    pause_screen();
    return 1;
}

/* ============================================
 * 函数: exam_subject1
 * 功能: 科目一模拟考试入口（100题，每题1分）
 * 实际调用 exam() 函数，参数已写死
 * ============================================ */
void exam_subject1(void) {
    exam(SUBJECT_1, 100, 1);
}

/* ============================================
 * 函数: exam_subject4
 * 功能: 科目四模拟考试入口（50题，每题2分）
 * 实际调用 exam() 函数，参数已写死
 * ============================================ */
void exam_subject4(void) {
    exam(SUBJECT_4, 50, 2);
}

/* ============================================
 * 函数: list_scores
 * 功能: 查看模拟考试成绩记录
 * 权限控制:
 *   - 管理员: 显示所有成绩
 *   - 考生: 仅显示 student_id == current_user_id 的记录
 * 格式: 编号 | 考生ID | 科目 | 得分 | 总分 | 正确 | 用时 | 考试时间
 * ============================================ */
void list_scores(void) {
    clear_screen();
    print_title("模拟成绩记录");
    if (score_count == 0) {             /* 空数据检查 */
        printf("暂无成绩记录。\n");
        pause_screen();
        return;
    }
    int has_data = 0;                   /* 标记是否有数据可显示 */
    /* 打印表头 */
    printf("  %-4s %-6s %-8s %-6s %-6s %-6s %-12s %-20s\n",
           "编号", "考生ID", "科目", "得分", "总分", "正确", "用时", "考试时间");
    print_line();
    for (int i = 0; i < score_count; i++) {
        /* 权限过滤: 考生只能看自己的成绩 */
        if (current_role == 2 && scores[i].student_id != current_user_id) continue;
        /* 将科目编号转换为中文名称 */
        char sub_name[8];
        strcpy(sub_name, (scores[i].subject == SUBJECT_1) ? "科目一" : "科目四");
        /* 打印一行成绩记录，用时拆分为 分:秒 格式 */
        printf("  %-4d %-6d %-8s %-6d %-6d %-6d %2d分%2d秒    %-20s\n",
               scores[i].id, scores[i].student_id, sub_name,
               scores[i].score, scores[i].total, scores[i].correct,
               scores[i].used_time / 60, scores[i].used_time % 60,
               scores[i].exam_date);
        has_data = 1;
    }
    if (!has_data) printf("暂无您的成绩记录。\n");  /* 考生视角的空数据提示 */
    pause_screen();
}
