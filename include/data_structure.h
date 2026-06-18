/*
 * data_structure.h
 * 驾考报名管理系统 — 核心数据结构定义头文件
 * 本文件定义了系统所有实体结构体、常量宏和全局变量 extern 声明
 */
#ifndef DATA_STRUCTURE_H
#define DATA_STRUCTURE_H

/* ============================================
 * 容量与常量定义
 * ============================================ */
#define MAX_SIZE 1000        /* 考生、预约、成绩记录的最大容量 */
#define MAX_VENUES 10        /* 考试场地的最大数量 */
#define MAX_QUESTIONS 2000   /* 题库的最大题目数量 */

#define SUBJECT_1 1         /* 科目一（交通法规） */
#define SUBJECT_4 4         /* 科目四（安全文明驾驶） */

#define TYPE_SINGLE 1       /* 单选题 */
#define TYPE_JUDGE 2        /* 判断题 */
#define TYPE_MULTI 3        /* 多选题（预留） */

/* ============================================
 * 结构体定义
 * ============================================ */

/*
 * 考生信息结构体
 * 主键: id（自增，删除后不回收）
 * 关联: 作为 appointments 和 scores 的外键 student_id
 */
typedef struct {
    int id;                 /* 考生唯一编号，由系统自增分配 */
    char id_card[19];       /* 身份证号，18位+1个\0，作为登录标识 */
    char name[51];          /* 姓名，50字符+1个\0 */
    char phone[16];         /* 联系电话，15字符+1个\0 */
    char car_type[5];       /* 报考车型：C1 / C2 / A1 / B2 */
    char status[10];        /* 状态：已报名 / 已预约 / 已完成 */
    char password[33];      /* 登录密码，明文存储，32字符+1个\0 */
} Student;

/*
 * 管理员账号结构体
 * 主键: id（自增）
 * 预置: 首次运行时自动创建 admin / admin123
 */
typedef struct {
    int id;                 /* 管理员唯一编号 */
    char username[33];      /* 用户名 */
    char password[33];      /* 密码（明文） */
} Admin;

/*
 * 预约记录结构体
 * 主键: id（自增）
 * 外键: student_id → Student.id, venue_id → Venue.id
 * 硬删除策略: 取消预约时直接从数组移除，前后元素前移
 */
typedef struct {
    int id;                 /* 预约单号 */
    int student_id;         /* 关联考生 ID */
    char subject[10];       /* 科目：科目一 / 二 / 三 / 四 */
    char exam_date[12];     /* 预约日期，格式：YYYY-MM-DD */
    char exam_time[12];     /* 预约时段：08:30-10:30 / 13:00-15:00 / 15:30-17:30 */
    int venue_id;           /* 关联考试场地 ID */
    char status[10];        /* 状态：已预约（预留扩展） */
} Appointment;

/*
 * 考试场地结构体
 * 主键: id（预置，固定 3 个）
 * 容量校验: 同日期 + 同时段 + 同场地 的预约数 ≤ max_per_slot
 */
typedef struct {
    int id;                 /* 场地编号 */
    char name[51];          /* 场地名称 */
    char address[101];      /* 场地地址 */
    int max_per_slot;       /* 每时段最大可容纳人数 */
} Venue;

/*
 * 题库题目结构体
 * 主键: id
 * subject 字段: 1=科目一, 4=科目四
 * type 字段: 1=单选, 2=判断, 3=多选（预留）
 * 选项: 判断题只用 options[0] 和 options[1]
 */
typedef struct {
    int id;                 /* 题号 */
    int subject;            /* 所属科目（1 或 4） */
    int type;               /* 题型（1/2/3） */
    char content[256];      /* 题目内容文本 */
    char options[4][128];   /* 四个选项，判断题只填前两个 */
    char answer[10];        /* 正确答案，如 "A" 或 "AB" */
    char explanation[256];  /* 答案解析 */
} Question;

/*
 * 模拟考试成绩结构体
 * 主键: id（自增）
 * 外键: student_id → Student.id
 * 保存时机: 每次模拟考试交卷后自动追加
 */
typedef struct {
    int id;                 /* 成绩记录编号 */
    int student_id;         /* 关联考生 ID（管理员登录时记 0） */
    int subject;            /* 1=科目一, 4=科目四 */
    int score;              /* 实际得分 */
    int total;              /* 总分（100题×1分 或 50题×2分） */
    int correct;            /* 正确题数 */
    int used_time;          /* 用时（秒） */
    char exam_date[20];     /* 考试日期时间，格式：YYYY-MM-DD HH:MM:SS */
} ExamScore;

/* ============================================
 * 全局变量 extern 声明（实际定义在 main.c）
 * 使用 extern 让所有 .c 文件共享同一份内存数据
 * ============================================ */

extern Student students[MAX_SIZE];      /* 考生数组 */
extern int student_count;                /* 当前考生实际数量 */

extern Admin admins[MAX_SIZE];          /* 管理员数组 */
extern int admin_count;                  /* 当前管理员数量 */

extern Appointment appointments[MAX_SIZE]; /* 预约数组 */
extern int appointment_count;              /* 当前预约数量 */

extern Venue venues[MAX_VENUES];        /* 场地数组 */
extern int venue_count;                  /* 当前场地数量 */

extern Question questions[MAX_QUESTIONS]; /* 题库数组 */
extern int question_count;               /* 当前题目数量 */

extern ExamScore scores[MAX_SIZE];      /* 成绩数组 */
extern int score_count;                  /* 当前成绩记录数量 */

extern int current_role;                 /* 当前登录角色: 0=未登录, 1=管理员, 2=考生 */
extern int current_user_id;              /* 当前登录用户的 ID（管理员或考生） */

#endif /* DATA_STRUCTURE_H */
