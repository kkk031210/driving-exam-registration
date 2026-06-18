/*
 * file_io.c
 * 驾考报名管理系统 — 数据持久化模块
 * 职责: 所有二进制数据文件的读写、首次运行时预置数据的初始化
 * 数据格式: 直接 fwrite/fread 结构体数组，无额外文件头，由 count 变量控制有效条数
 */

#include <stdio.h>          /* 标准输入输出: fopen, fread, fwrite, fclose */
#include <stdlib.h>         /* 标准库 */
#include <string.h>         /* 字符串处理 */
#include "data_structure.h" /* 全局变量和结构体声明 */
#include "common.h"         /* 公共工具（pause_screen） */
#include "file_io.h"        /* 本模块的函数声明 */

/* ============================================
 * 数据文件路径宏定义
 * 所有 .dat 文件存放在 data/ 子目录下
 * 注意: 目录分隔符在 Linux 下为 /，Windows 下需改为 \\
 * ============================================ */
#define FILE_STUDENTS     "data/students.dat"     /* 考生信息表 */
#define FILE_ADMINS       "data/admins.dat"       /* 管理员账号表 */
#define FILE_APPOINTMENTS "data/appointments.dat" /* 预约记录表 */
#define FILE_VENUES       "data/venues.dat"       /* 考试场地表 */
#define FILE_QUESTIONS    "data/questions.dat"    /* 题库表 */
#define FILE_SCORES       "data/scores.dat"       /* 模拟成绩表 */

/* ============================================
 * 函数: init_data
 * 功能: 首次运行时检测数据文件是否存在，不存在则创建预置数据
 * 预置内容:
 *   1. 管理员账号: admin / admin123（ID=1）
 *   2. 考试场地: 3 个（市北、市南、高新区考场）
 *   3. 模拟题库: 20 题（科目一 10 题 + 科目四 10 题）
 * 检测方式: fopen(..., "rb") 若返回 NULL 说明文件不存在
 * ============================================ */
void init_data(void) {
    /* ---------- 1. 初始化管理员账号 ---------- */
    FILE *fp = fopen(FILE_ADMINS, "rb");  /* 尝试以只读二进制模式打开 */
    if (fp == NULL)    /* 文件不存在: 首次运行 */
    {                      
        /* 构造默认管理员结构体 */
        Admin admin = {1, "admin", "admin123"};
        /* 以只写二进制模式创建新文件 */
        FILE *fw = fopen(FILE_ADMINS, "wb");
        if (fw) {                           /* 文件成功创建 */
            fwrite(&admin, sizeof(Admin), 1, fw);  /* 写入 1 条 Admin 记录 */
            fclose(fw);                      /* 关闭文件 */
        }
        /* 提示用户默认账号，方便首次登录 */
        printf("[系统] 首次运行，已创建默认管理员 admin / admin123\n");
        pause_screen();                     /* 暂停，让用户看清提示 */
    } else {
        fclose(fp);                         /* 文件已存在，关闭即可 */
    }

    /* ---------- 2. 初始化考试场地 ---------- */
    fp = fopen(FILE_VENUES, "rb");
    if (fp == NULL) {
        /* 预置 3 个考试场地，硬编码在代码中 */
        Venue v[] = {
            {1, "市北考场", "北环路88号", 30},     /* ID=1, 容量 30 人/时段 */
            {2, "市南考场", "南环路66号", 25},     /* ID=2, 容量 25 人/时段 */
            {3, "高新区考场", "高新大道100号", 40} /* ID=3, 容量 40 人/时段 */
        };
        FILE *fw = fopen(FILE_VENUES, "wb");
        if (fw) {
            fwrite(v, sizeof(Venue), 3, fw);  /* 写入 3 条 Venue 记录 */
            fclose(fw);
        }
    } else {
        fclose(fp);
    }

    /* ---------- 3. 初始化模拟题库 ---------- */
    fp = fopen(FILE_QUESTIONS, "rb");
    if (fp == NULL) {
        /* 预置 20 道模拟题：科目一 10 题 + 科目四 10 题
         * 包含单选题(TYPE_SINGLE)和判断题(TYPE_JUDGE)
         * 每题有 4 个选项（判断题只用前两个）、正确答案和解析
         */
        Question q[] = {
            /* 科目一（交通法规） */
            {1, SUBJECT_1, TYPE_SINGLE, "驾驶机动车在道路上违反道路交通安全法的行为，属于什么性质？", {"违法行为", "违章行为", "过失行为", "违规行为"}, "A", "违反道路交通安全法的行为属于违法行为。"},
            {2, SUBJECT_1, TYPE_SINGLE, "机动车驾驶人初次申领驾驶证后的实习期是多长时间？", {"6个月", "12个月", "18个月", "24个月"}, "B", "初次申领驾驶证后的实习期为12个月。"},
            {3, SUBJECT_1, TYPE_JUDGE, "驾驶机动车在道路上超车时可以不使用转向灯。", {"正确", "错误", "", ""}, "B", "超车必须使用转向灯，提前开启左转向灯。"},
            {4, SUBJECT_1, TYPE_SINGLE, "驾驶机动车在高速公路上行驶，最高车速不得超过每小时多少公里？", {"100", "110", "120", "130"}, "C", "高速公路最高车速不得超过每小时120公里。"},
            {5, SUBJECT_1, TYPE_SINGLE, "机动车驾驶证有效期分为6年、10年和长期。", {"正确", "错误", "", ""}, "A", "驾驶证有效期分为6年、10年和长期三种。"},
            {6, SUBJECT_1, TYPE_JUDGE, "饮酒后驾驶机动车的，处暂扣6个月驾驶证，并处1000元以上2000元以下罚款。", {"正确", "错误", "", ""}, "A", "饮酒驾驶处罚为暂扣6个月驾驶证，并处罚款。"},
            {7, SUBJECT_1, TYPE_SINGLE, "交通信号灯黄灯闪烁时表示什么？", {"禁止通行", "准许通行", "警示", "停车让行"}, "C", "黄灯闪烁表示警示，提醒驾驶人注意。"},
            {8, SUBJECT_1, TYPE_SINGLE, "驾驶机动车在没有中心线的城市道路上，最高时速不得超过多少？", {"30", "40", "50", "60"}, "A", "没有中心线的城市道路最高时速30公里。"},
            {9, SUBJECT_1, TYPE_JUDGE, "机动车在夜间通过急弯、坡路、拱桥、人行横道时，应当交替使用远近光灯。", {"正确", "错误", "", ""}, "A", "夜间通过特殊路段应交替使用远近光灯示意。"},
            {10, SUBJECT_1, TYPE_SINGLE, "驾驶机动车在高速公路上发生故障时，警告标志应设置在来车方向多少米以外？", {"50", "100", "150", "200"}, "C", "高速公路故障警告标志应设置在来车方向150米以外。"},
            /* 科目四（安全文明驾驶） */
            {11, SUBJECT_4, TYPE_SINGLE, "驾驶机动车在雨天行驶，应如何使用灯光？", {"开启远光灯", "开启雾灯和危险报警闪光灯", "开启近光灯", "不需要使用灯光"}, "B", "雨天视线不佳，应开启雾灯和危险报警闪光灯。"},
            {12, SUBJECT_4, TYPE_SINGLE, "驾驶机动车在高速公路上遇到能见度小于200米时，车速不得超过多少？", {"40", "60", "80", "100"}, "B", "能见度小于200米时，车速不得超过60公里/小时。"},
            {13, SUBJECT_4, TYPE_JUDGE, "驾驶机动车在山区道路超车时，应选择宽阔的缓上坡路段。", {"正确", "错误", "", ""}, "A", "山区超车应选择宽阔缓上坡路段，视野好。"},
            {14, SUBJECT_4, TYPE_SINGLE, "驾驶机动车在冰雪路面上起步时，应如何操作？", {"猛踩油门", "缓踩油门", "不踩油门", "先踩离合"}, "B", "冰雪路面起步应缓踩油门，防止打滑。"},
            {15, SUBJECT_4, TYPE_JUDGE, "驾驶机动车通过铁路道口时，最高时速不得超过30公里。", {"正确", "错误", "", ""}, "A", "铁路道口限速30公里/小时。"},
            {16, SUBJECT_4, TYPE_SINGLE, "驾驶机动车在雾天行驶，应开启什么灯光？", {"远光灯", "雾灯和危险报警闪光灯", "近光灯", "示廓灯"}, "B", "雾天应开启雾灯和危险报警闪光灯。"},
            {17, SUBJECT_4, TYPE_SINGLE, "驾驶机动车遇到前方车辆停车排队时，应当如何行驶？", {"借道超车", "依次排队", "占用对向车道", "穿插等候车辆"}, "B", "前方车辆排队时应依次排队，不得穿插。"},
            {18, SUBJECT_4, TYPE_JUDGE, "驾驶机动车在隧道内行驶，可以临时停车。", {"正确", "错误", "", ""}, "B", "隧道内禁止停车。"},
            {19, SUBJECT_4, TYPE_SINGLE, "驾驶机动车在窄桥、窄路会车时，应当如何操作？", {"加速通过", "减速靠右", "占用对方车道", "鸣笛通过"}, "B", "窄桥窄路会车应减速靠右。"},
            {20, SUBJECT_4, TYPE_JUDGE, "驾驶机动车在通过没有交通信号灯控制的路口时，应当减速慢行，并让行人和优先通行的车辆先行。", {"正确", "错误", "", ""}, "A", "无信号灯路口应减速让行。"}
        };
        int n = sizeof(q) / sizeof(q[0]);     /* 计算题目数量: 20 */
        FILE *fw = fopen(FILE_QUESTIONS, "wb");
        if (fw) {
            fwrite(q, sizeof(Question), n, fw);  /* 写入全部 20 题 */
            fclose(fw);
        }
        printf("[系统] 已预置 %d 道模拟题\n", n);
        pause_screen();
    } else {
        fclose(fp);
    }
}

/* ============================================
 * 函数: load_all
 * 功能: 加载所有数据文件到内存
 * 调用时机: main() 中程序启动时，init_data() 之后
 * 实现: 依次调用各实体的 load_xxx() 函数
 * ============================================ */
void load_all(void) {
    load_admins();       /* 加载管理员账号 */
    load_students();     /* 加载考生信息 */
    load_appointments(); /* 加载预约记录 */
    load_venues();       /* 加载场地数据 */
    load_questions();    /* 加载题库 */
    load_scores();       /* 加载历史成绩 */
}

/* ============================================
 * 函数: save_all
 * 功能: 将内存中所有数据写回文件
 * 调用时机: 用户退出登录或退出程序时
 * 实现: 依次调用各实体的 save_xxx() 函数
 * 注意: 所有写入均用 "wb" 模式，覆盖原文件，确保删除操作生效
 * ============================================ */
void save_all(void) {
    save_admins();       /* 保存管理员账号 */
    save_students();     /* 保存考生信息 */
    save_appointments(); /* 保存预约记录 */
    save_venues();       /* 保存场地数据 */
    save_questions();    /* 保存题库 */
    save_scores();       /* 保存成绩记录 */
}

/* ============================================
 * 函数: load_admins / save_admins
 * 功能: 管理员账号的读写
 * 加载: 以 "rb" 打开文件，fread 读取全部 Admin 结构体到数组，返回值即为 count
 * 保存: 以 "wb" 打开文件，fwrite 写入 count 条 Admin 记录
 * ============================================ */
void load_admins(void) {
    FILE *fp = fopen(FILE_ADMINS, "rb");  /* 只读二进制 */
    if (fp) {                             /* 文件存在 */
        /* fread 返回实际读取到的元素个数，即为当前管理员数量 */
        admin_count = fread(admins, sizeof(Admin), MAX_SIZE, fp);
        fclose(fp);
    } else {
        admin_count = 0;                  /* 文件不存在，数量设为 0 */
    }
}

void save_admins(void) {
    FILE *fp = fopen(FILE_ADMINS, "wb");  /* 只写二进制，覆盖 */
    if (fp) {                             /* 文件成功打开 */
        fwrite(admins, sizeof(Admin), admin_count, fp);  /* 写入 count 条记录 */
        fclose(fp);
    }
    /* 文件打开失败时静默处理（无错误提示），数据保留在内存中 */
}

/* ============================================
 * 函数: load_students / save_students
 * 功能: 考生信息的读写，模式同上
 * ============================================ */
void load_students(void) {
    FILE *fp = fopen(FILE_STUDENTS, "rb");
    if (fp) {
        student_count = fread(students, sizeof(Student), MAX_SIZE, fp);
        fclose(fp);
    } else {
        student_count = 0;
    }
}

void save_students(void) {
    FILE *fp = fopen(FILE_STUDENTS, "wb");
    if (fp) {
        fwrite(students, sizeof(Student), student_count, fp);
        fclose(fp);
    }
}

/* ============================================
 * 函数: load_appointments / save_appointments
 * 功能: 预约记录的读写，模式同上
 * ============================================ */
void load_appointments(void) {
    FILE *fp = fopen(FILE_APPOINTMENTS, "rb");
    if (fp) {
        appointment_count = fread(appointments, sizeof(Appointment), MAX_SIZE, fp);
        fclose(fp);
    } else {
        appointment_count = 0;
    }
}

void save_appointments(void) {
    FILE *fp = fopen(FILE_APPOINTMENTS, "wb");
    if (fp) {
        fwrite(appointments, sizeof(Appointment), appointment_count, fp);
        fclose(fp);
    }
}

/* ============================================
 * 函数: load_venues / save_venues
 * 功能: 考试场地的读写
 * 注意: 场地上限为 MAX_VENUES(10)，不是 MAX_SIZE
 * ============================================ */
void load_venues(void) {
    FILE *fp = fopen(FILE_VENUES, "rb");
    if (fp) {
        venue_count = fread(venues, sizeof(Venue), MAX_VENUES, fp);
        fclose(fp);
    } else {
        venue_count = 0;
    }
}

void save_venues(void) {
    FILE *fp = fopen(FILE_VENUES, "wb");
    if (fp) {
        fwrite(venues, sizeof(Venue), venue_count, fp);
        fclose(fp);
    }
}

/* ============================================
 * 函数: load_questions / save_questions
 * 功能: 题库的读写
 * 注意: 题库上限为 MAX_QUESTIONS(2000)，不是 MAX_SIZE
 * ============================================ */
void load_questions(void) {
    FILE *fp = fopen(FILE_QUESTIONS, "rb");
    if (fp) {
        question_count = fread(questions, sizeof(Question), MAX_QUESTIONS, fp);
        fclose(fp);
    } else {
        question_count = 0;
    }
}

void save_questions(void) {
    FILE *fp = fopen(FILE_QUESTIONS, "wb");
    if (fp) {
        fwrite(questions, sizeof(Question), question_count, fp);
        fclose(fp);
    }
}

/* ============================================
 * 函数: load_scores / save_scores
 * 功能: 模拟考试成绩的读写
 * ============================================ */
void load_scores(void) {
    FILE *fp = fopen(FILE_SCORES, "rb");
    if (fp) {
        score_count = fread(scores, sizeof(ExamScore), MAX_SIZE, fp);
        fclose(fp);
    } else {
        score_count = 0;
    }
}

void save_scores(void) {
    FILE *fp = fopen(FILE_SCORES, "wb");
    if (fp) {
        fwrite(scores, sizeof(ExamScore), score_count, fp);
        fclose(fp);
    }
}

/* ============================================
 * 函数: import_questions_from_txt
 * 功能: 从文本文件导入题库，覆盖现有题库
 * 参数: filename — 文本文件路径
 * 格式: 编号|科目|题型|题目|选项A|选项B|选项C|选项D|答案|解析
 * 返回: 成功导入的题目数量，-1=文件打开失败
 * 注意: 导入后自动覆盖 questions.dat 文件
 * ============================================ */
int import_questions_from_txt(const char *filename) 
{
    FILE *fp = fopen(filename, "r");  /* 以文本模式打开 */
    if (!fp) 
    {  
          /* 文件不存在 */
        perror("无法打开文件");
        return -1;
    }

    char line[1024];                    /* 行缓冲区 */
    int count = 0;                      /* 成功导入计数 */

    /* 清空现有题库（覆盖导入） */
    question_count = 0;

    while (fgets(line, sizeof(line), fp)) /* 逐行读取 */
    {  
        /* 去掉末尾换行符 */
        line[strcspn(line, "\n")] = '\0';
        /* 跳过空行和注释行 */
        if (line[0] == '\0' || line[0] == '#') 
            continue;

        /* 检查容量 */
        if (question_count >= MAX_QUESTIONS) {
            printf("[警告] 题库已满，仅导入前 %d 题\n", count);
            break;
        }

        /* 按 | 分隔符解析字段
         * 格式: id|subject|type|content|optA|optB|optC|optD|answer|explanation
         */
        char *token = strtok(line, "|");
        if (token == NULL)         /* 无有效数据 */
            continue;          

        Question q;
        memset(&q, 0, sizeof(q));

        /* 字段1: 编号 */
        q.id = atoi(token);
        /* 字段2: 科目 */
        token = strtok(NULL, "|");
        if (token == NULL) 
            continue;
        q.subject = atoi(token);
        /* 字段3: 题型 */
        token = strtok(NULL, "|");
        if (token == NULL) 
            continue;
        q.type = atoi(token);
        /* 字段4: 题目内容 */
        token = strtok(NULL, "|");
        if (token == NULL) 
            continue;
        strncpy(q.content, token, sizeof(q.content) - 1);
        /* 字段5: 选项A */
        token = strtok(NULL, "|");
        if (token == NULL) 
            continue;
        strncpy(q.options[0], token, sizeof(q.options[0]) - 1);
        /* 字段6: 选项B */
        token = strtok(NULL, "|");
        if (token == NULL) 
            continue;
        strncpy(q.options[1], token, sizeof(q.options[1]) - 1);
        /* 字段7: 选项C */
        token = strtok(NULL, "|");
        if (token == NULL) 
            continue;
        strncpy(q.options[2], token, sizeof(q.options[2]) - 1);
        /* 字段8: 选项D */
        token = strtok(NULL, "|");
        if (token == NULL) 
            continue;
        strncpy(q.options[3], token, sizeof(q.options[3]) - 1);
        /* 字段9: 答案 */
        token = strtok(NULL, "|");
        if (token == NULL) 
            continue;
        strncpy(q.answer, token, sizeof(q.answer) - 1);
        /* 字段10: 解析 */
        token = strtok(NULL, "|");
        if (token == NULL) 
            continue;
        strncpy(q.explanation, token, sizeof(q.explanation) - 1);

        questions[question_count++] = q;  /* 追加到数组 */
        count++;
    }

    fclose(fp);

    if (count > 0) 
    {
        save_questions();               /* 自动保存到二进制文件 */
        printf("[成功] 导入 %d 道题目，已覆盖原有题库\n", count);
    } else 
        printf("[警告] 未导入任何题目，请检查文件格式\n");
    
    return count;
}
