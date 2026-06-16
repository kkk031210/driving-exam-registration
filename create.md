# 驾考考试报名管理系统 — 构建思路文档

> 本文档记录项目的整体架构设计、模块职责、数据流与关键实现决策，便于阅读代码时对照理解。

---

## 一、项目概述

**目标**：实现一个基于 C 语言控制台界面的驾考报名管理系统，覆盖考生管理、预约管理、模拟考试三大功能域，支持管理员与考生两种角色登录。

**技术栈**：
- 语言：C (GNU C11)
- 平台：Linux (POSIX)
- 界面：命令行控制台（ANSI 转义码清屏）
- 数据持久化：二进制文件读写（`.dat`）
- 内存结构：全局结构体数组 + 计数器

---

## 二、架构决策树（关键设计选择）

以下决策均在 `deep-probe` 追问模式下逐一确认：

| 序号 | 决策项 | 选择方案 | 理由 |
|------|--------|---------|------|
| 1 | 技术栈 | C + 控制台 + 二进制文件 | 粤嵌课程要求，无额外依赖 |
| 2 | 内存结构 | 结构体数组 + 计数器 | 数据量可控，随机访问高效，易于调试 |
| 3 | 模块拆分 | 6 个 `.c` + 5 个 `.h` | 职责单一，便于协作与维护 |
| 4 | 数据存储 | 多文件独立存储（6 个 `.dat`） | 各表独立管理，便于预置/重置 |
| 5 | 角色登录 | 管理员 + 考生双角色 | 管理员全权限，考生自限权限 |
| 6 | 密码输入 | 隐藏回显，显示 `*` | 隐私保护，符合交互习惯 |
| 7 | 密码存储 | 明文（课程简化） | 便于调试，非生产环境设计 |
| 8 | 管理员账号 | 预置 `admin` / `admin123` | 首次运行自动创建，无需手动配置 |
| 9 | 考试时段 | 3 个固定时段 | 符合实际驾考安排，菜单选择避免格式错误 |
| 10 | 取消预约 | 硬删除 | 数组直接移除，逻辑清晰 |
| 11 | 模拟成绩 | 仅显示结果（无及格判定） | 符合用户要求，仅记录得分/用时/正误 |
| 12 | 删除考生后 ID | 保持不变 | 外键安全，历史数据可追溯 |
| 13 | 退格键处理 | 同时支持 `\b` (8) 和 DEL (127) | 兼容不同终端键盘映射 |
| 14 | 跨平台 | 当前仅 Linux（用 `termios` 替代 `conio.h`） | POSIX 标准，MinGW 下需改回 Windows 版 |

---

## 三、目录结构

```
.
├── Makefile              # 编译脚本（Linux 版）
├── README.md             # 用户说明
├── create.md             # 本文件（构建思路）
├── data/                 # 运行时数据目录（程序自动生成）
│   ├── admins.dat        # 管理员账号
│   ├── students.dat      # 考生信息
│   ├── appointments.dat  # 预约记录
│   ├── venues.dat        # 考试场地（预置 3 个）
│   ├── questions.dat     # 题库（预置 20 题）
│   └── scores.dat        # 模拟成绩
├── include/              # 头文件
│   ├── data_structure.h  # 所有结构体 + 全局变量 extern
│   ├── common.h          # 公共工具函数声明
│   ├── student.h         # 考生管理模块接口
│   ├── appointment.h     # 预约管理模块接口
│   ├── exam_sim.h        # 模拟考试模块接口
│   └── file_io.h         # 数据持久化模块接口
└── src/                  # 源文件
    ├── main.c            # 入口 + 登录 + 菜单路由
    ├── common.c          # 清屏、输入、密码隐藏、校验
    ├── student.c         # 考生 CRUD + 级联删除
    ├── appointment.c     # 预约 + 容量冲突检测
    ├── exam_sim.c        # 抽题组卷 + 判分 + 保存成绩
    └── file_io.c         # 二进制文件读写 + 预置数据初始化
```

---

## 四、数据结构定义

定义集中在 `include/data_structure.h`，全局变量在 `main.c` 中定义，`extern` 声明在头文件中。

### 4.1 核心实体

| 结构体 | 主键 | 关键字段 | 说明 |
|--------|------|---------|------|
| `Student` | `id` (自增) | `id_card`, `name`, `phone`, `car_type`, `status`, `password` | 考生信息 |
| `Admin` | `id` | `username`, `password` | 管理员账号 |
| `Appointment` | `id` (自增) | `student_id`, `subject`, `exam_date`, `exam_time`, `venue_id`, `status` | 预约记录 |
| `Venue` | `id` | `name`, `address`, `max_per_slot` | 考试场地 |
| `Question` | `id` | `subject`, `type`, `content`, `options[4]`, `answer`, `explanation` | 题库 |
| `ExamScore` | `id` (自增) | `student_id`, `subject`, `score`, `total`, `correct`, `used_time`, `exam_date` | 成绩记录 |

### 4.2 全局变量

```c
Student students[MAX_SIZE];        int student_count;
Admin admins[MAX_SIZE];            int admin_count;
Appointment appointments[MAX_SIZE]; int appointment_count;
Venue venues[MAX_VENUES];          int venue_count;
Question questions[MAX_QUESTIONS]; int question_count;
ExamScore scores[MAX_SIZE];        int score_count;

int current_role;       // 0=未登录, 1=管理员, 2=考生
int current_user_id;    // 当前登录用户的 ID
```

### 4.3 容量宏

```c
#define MAX_SIZE 1000        // 考生、预约、成绩上限
#define MAX_VENUES 10        // 场地上限
#define MAX_QUESTIONS 2000   // 题库上限
```

---

## 五、数据持久化设计

### 5.1 文件映射

| 实体 | 文件名 | 读写方式 | 说明 |
|------|--------|---------|------|
| Admin | `data/admins.dat` | `fread`/`fwrite` 结构体数组 | 首次运行自动写入默认账号 |
| Student | `data/students.dat` | 同上 | — |
| Appointment | `data/appointments.dat` | 同上 | — |
| Venue | `data/venues.dat` | 同上 | 首次运行预置 3 个场地 |
| Question | `data/questions.dat` | 同上 | 首次运行预置 20 题 |
| ExamScore | `data/scores.dat` | 同上 | 每次模拟考试后追加 |

### 5.2 生命周期

```
程序启动
    │
    ▼
init_data() ──→ 检查文件存在？
    │             │ 不存在 → 创建预置数据（admins + venues + questions）
    │             │ 存在   → 跳过
    ▼
load_all() ──→ 逐个 `fopen(..., "rb")` → `fread` 到全局数组 → 记录 count
    │
    ▼
[业务交互] ──→ 内存操作（数组增删改查）
    │
    ▼
退出登录 / 退出程序
    │
    ▼
save_all() ──→ 逐个 `fopen(..., "wb")` → `fwrite` 数组 → `fclose`
```

### 5.3 关键设计点

- **写文件用 `"wb"`**：覆盖写入，而非追加。确保删除操作（硬删除）在文件中生效。
- **无文件头格式**：直接顺序写入结构体数组，依赖 `count` 变量控制条数。简单高效，但不可跨平台兼容（结构体对齐）。
- **首次运行检测**：通过 `fopen(..., "rb")` 是否成功来判断文件是否存在，不存在则创建预置数据。

---

## 六、模块职责详解

### 6.1 `main.c` — 入口与路由

**职责**：
- 定义所有全局变量（唯一定义处）
- 初始化随机种子（`srand(time(NULL))`）
- 调用数据加载（`init_data()` + `load_all()`）
- 登录菜单（管理员 / 考生 / 退出）
- 角色菜单分发（管理员菜单 vs 考生菜单）
- 退出时保存数据（`save_all()`）

**登录流程**：
```
登录菜单
  ├── 管理员登录 → 输入 username + password → 校验 admins 数组 → 设置 current_role=1, current_user_id=admin.id
  ├── 考生登录   → 输入 id_card + password → 校验 students 数组 → 设置 current_role=2, current_user_id=student.id
  └── 退出
```

**角色菜单差异**：

| 菜单 | 管理员（admin_menu） | 考生（student_menu_main） |
|------|---------------------|---------------------------|
| 我的信息 | 无 | `my_info()` → 调用 `modify_student()`（自限 ID） |
| 我的预约 | 无 | `my_appointments()`（仅显示自己的预约） |
| 考生管理 | `student_menu()`（全权限） | 无 |
| 预约管理 | `appointment_menu()`（全权限） | 无 |
| 模拟考试 | `exam_sim_menu()`（查看所有成绩） | `exam_sim_menu()`（仅查看自己的成绩） |

### 6.2 `common.c` — 公共工具

**核心函数**：

| 函数 | 作用 | 实现要点 |
|------|------|---------|
| `getch()` | 无回显读取单字符 | `termios` 关闭 `ICANON` + `ECHO`，读取后恢复原设置 |
| `clear_screen()` | 清屏 | ANSI 转义码 `\033[2J\033[H` |
| `get_password()` | 隐藏密码输入 | 循环 `getch()`，回车结束，退格/删除（`\b` 或 127）回删 `*` 和缓冲区 |
| `validate_id_card()` | 身份证号校验 | 长度 18，前 17 位数字，末位数字或 X |
| `is_date_valid()` | 日期格式校验 | 长度 10，位置 4 和 7 为 `-` |
| `is_time_slot_valid()` | 时段校验 | 硬编码 3 个有效时段字符串 |
| `get_current_date()` | 获取当前时间字符串 | `time()` + `localtime()` + `snprintf` |

### 6.3 `student.c` — 考生管理

**CRUD 流程**：

| 功能 | 关键逻辑 |
|------|---------|
| **添加** | 检查 `student_count < MAX_SIZE` → 自动生成 ID（最后一条+1）→ 校验身份证号格式 → 检查身份证号唯一性 → 输入姓名/电话/车型/密码 → 状态设为"已报名" → 追加到数组末尾 |
| **浏览** | 遍历数组，格式化打印所有字段 |
| **查询** | `strstr` 模糊匹配身份证号或姓名 |
| **修改** | 管理员：输入任意 ID；考生：强制 `current_user_id` → 查找索引 → 逐字段输入（直接回车跳过） |
| **删除** | 确认对话框 → 级联删除关联的 `appointments`（按 `student_id` 遍历移除）和 `scores` → 数组前移覆盖删除位置 |

**级联删除逻辑**：
```c
for (i = appointment_count-1; i >= 0; i--) {
    if (appointments[i].student_id == id) {
        // 前移覆盖，count--
    }
}
// 同样处理 scores
// 最后删除 student 本身
```

### 6.4 `appointment.c` — 预约管理

**核心校验**：

| 校验项 | 时机 | 实现 |
|--------|------|------|
| 考生存在 | 预约时 | 遍历 `students` 按 ID 匹配 |
| 日期格式 | 预约/修改 | `is_date_valid()` |
| 时段合法 | 预约/修改 | `is_time_slot_valid()` |
| 场地存在 | 预约/修改 | 遍历 `venues` 按 ID 匹配 |
| **容量冲突** | 预约/修改 | 统计同日期 + 同时段 + 同场地 的已预约数，与 `max_per_slot` 比较 |

**权限控制**：
- 管理员：可查看/操作所有预约
- 考生：列表仅显示自己的，`cancel`/`update` 时检查 `appointment.student_id == current_user_id`

### 6.5 `exam_sim.c` — 模拟考试

**抽题算法**（Fisher-Yates 洗牌）：
```c
// 1. 收集该科目所有题目索引到临时数组
for (i = 0; i < question_count; i++) {
    if (questions[i].subject == subject) indices[idx++] = i;
}
// 2. 随机洗牌
for (i = available-1; i > 0; i--) {
    j = rand() % (i + 1);
    swap(indices[i], indices[j]);
}
// 3. 取前 N 题（N = total_questions，若题库不足则取全部）
```

**答题流程**：
1. 逐题显示（清屏，显示题号、内容、选项）
2. 输入答案（大小写统一转大写）
3. 正确 → 直接下一题；错误 → 显示解析和正确答案，按 Enter 继续
4. 全部答完 → 计算得分、用时 → 保存成绩

**成绩保存**：
```c
ExamScore es;
es.id = (score_count > 0) ? scores[score_count-1].id + 1 : 1;
es.student_id = current_user_id;  // 考生登录时记录，管理员登录时记 0
es.subject = SUBJECT_1 / SUBJECT_4;
es.score = correct * points_per_question;
es.total = total_questions * points_per_question;
es.correct = correct;
es.used_time = (int)(end - start);
get_current_date(es.exam_date, ...);
scores[score_count++] = es;
```

### 6.6 `file_io.c` — 数据持久化

**init_data() 预置数据**：
- **Admin**：`{1, "admin", "admin123"}`
- **Venues**：3 个固定场地（市北、市南、高新区）
- **Questions**：20 题（科目一 10 题 + 科目四 10 题），含单选、判断、选项、答案、解析

**读写模式**：
- 加载：`fopen(path, "rb")` → `fread(array, sizeof(Type), MAX, fp)` → 返回值即 count
- 保存：`fopen(path, "wb")` → `fwrite(array, sizeof(Type), count, fp)`

---

## 七、菜单层级图

```
驾考考试报名管理系统
    ├── [1] 管理员登录
    │       └── 管理员功能菜单
    │               ├── [1] 考生信息管理
    │               │       ├── [1] 添加考生
    │               │       ├── [2] 浏览所有考生
    │               │       ├── [3] 查询考生
    │               │       ├── [4] 修改考生
    │               │       └── [5] 删除考生
    │               ├── [2] 预约管理
    │               │       ├── [1] 预约考试
    │               │       ├── [2] 查看所有预约
    │               │       ├── [3] 查询预约
    │               │       ├── [4] 取消预约
    │               │       └── [5] 更新预约
    │               ├── [3] 模拟考试
    │               │       ├── [1] 科目一模拟
    │               │       ├── [2] 科目四模拟
    │               │       └── [3] 查看成绩（所有考生）
    │               └── [0] 注销登录
    ├── [2] 考生登录
    │       └── 考生功能菜单
    │               ├── [1] 我的信息
    │               │       └── 显示信息 + [1] 修改信息（仅限自己）
    │               ├── [2] 我的预约
    │               │       └── 显示自己的预约列表
    │               ├── [3] 模拟考试
    │               │       ├── [1] 科目一模拟
    │               │       ├── [2] 科目四模拟
    │               │       └── [3] 查看成绩（仅自己）
    │               └── [0] 注销登录
    └── [0] 退出系统
            └── save_all() → 退出
```

---

## 八、关键数据流示例

### 8.1 添加考生 → 预约 → 模拟考试 → 查看成绩

```
[管理员] 添加考生
    │
    ▼
students.dat ← 写入新考生（ID 自增）
    │
    ▼
[管理员/考生] 预约考试
    │
    ▼
appointments.dat ← 写入新预约（校验容量冲突）
    │
    ▼
[考生] 科目一模拟考试
    │
    ▼
questions.dat → 读取题库 → 随机抽题 → 答题交互
    │
    ▼
scores.dat ← 写入成绩（得分、用时、日期）
    │
    ▼
[考生] 查看模拟成绩 → 从 scores.dat 读取并显示
```

### 8.2 删除考生时的级联删除

```
管理员删除考生 ID=2
    │
    ├──→ 遍历 appointments：移除所有 student_id=2 的记录
    │
    ├──→ 遍历 scores：移除所有 student_id=2 的记录
    │
    └──→ 从 students 数组中移除 ID=2 的元素，student_count--
    │
    ▼
退出时 save_all() → students.dat / appointments.dat / scores.dat 同步更新
```

---

## 九、编译与运行

```bash
cd /mnt/hgfs/驾考考试报名管理系统

# 编译
make

# 运行
./driving_exam

# 清理
make clean
```

**首次运行**：
- 终端提示：`[系统] 首次运行，已创建默认管理员 admin / admin123`
- 数据目录 `data/` 自动生成 3 个预置文件（`admins.dat`, `venues.dat`, `questions.dat`）

---

## 十、后续可扩展点

| 扩展方向 | 说明 |
|---------|------|
| 题库扩充 | 当前仅 20 题，可导入真实科目一/四题库（1000+ 题） |
| 计时器 | 模拟考试增加倒计时限制，超时自动交卷 |
| 预约冲突更精细 | 增加同一考生不能重复预约同一科目的校验 |
| 成绩统计 | 增加平均分、通过率等统计功能 |
| 跨平台 | 通过条件编译 `#ifdef _WIN32` 同时支持 Windows 和 Linux |
| 密码加密 | 引入 MD5 或 SHA256 哈希存储 |

---

> 文档版本：v1.0 | 对应代码版本：框架完成 + 业务逻辑填充完成
