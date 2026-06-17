# tools 目录结构说明

当前目录（`tools/`）专门存放与项目构建、维护、数据准备相关的辅助脚本和工具，与核心 C 源码分离。

## 目录结构

```
tools/
├── py_scripts/          # Python 脚本（题库生成、爬虫、数据处理）
│   ├── generate_question_txt.py   # 生成 150 题文本题库（从硬编码数据）
│   ├── crawl_questions.py         # 网络爬虫脚本（从驾考网站爬取）
│   └── setup_crawler.bat          # Windows 一键安装依赖脚本
│
├── data_sources/        # 原始数据源（爬取到的 HTML、JSON、CSV 等）
│   ├── raw_html/        # 从网页另存为的原始 HTML 文件
│   ├── raw_json/        # 从 API 或第三方获取的 JSON 题库
│   └── raw_csv/         # 从 Excel 或数据库导出的 CSV 题库
│
├── output/              # 脚本输出目录（生成后可直接导入系统的文件）
│   └── questions.txt    # 格式化后的题库文本（生成后复制到 ../data/）
│
└── scripts/             # 其他辅助脚本（Shell/Batch 等）
    └── init_data.sh     # Linux 一键初始化数据脚本（可选）
```

## 与核心项目的关联

```
驾考考试报名管理系统/
├── src/              # C 语言核心源码（已编译）
├── include/          # C 语言头文件
├── data/             # 运行时数据目录（.dat 文件）
│   ├── questions.dat     # 二进制题库（程序自动生成/导入）
│   └── questions.txt     # 文本题库（从 tools/output/ 复制过来）
├── tools/            # ← 辅助工具目录（你当前所在位置）
│   ├── py_scripts/
│   ├── data_sources/
│   ├── output/
│   └── scripts/
├── Makefile
└── driving_exam      # 编译生成的可执行文件
```

## 使用流程

### 1. 生成题库（Python 脚本）

```bash
# 进入 Python 脚本目录
cd tools/py_scripts

# 运行生成脚本
python generate_question_txt.py
# 输出到: tools/output/questions.txt
```

### 2. 复制到系统数据目录

```bash
# 将生成的题库复制到项目 data/ 目录
cp tools/output/questions.txt data/questions.txt
```

### 3. 导入系统

```bash
# 编译并运行 C 程序
make
./driving_exam
# 管理员登录 → [4] 导入题库 → 输入 data/questions.txt
```

## 文件迁移说明

你现有的文件应按以下方式整理：

| 原位置 | 新位置 | 说明 |
|--------|--------|------|
| `generate_question_txt.py` | `tools/py_scripts/generate_question_txt.py` | 题库生成脚本 |
| `crawl_questions.py` | `tools/py_scripts/crawl_questions.py` | 网络爬虫脚本 |
| `setup_crawler.bat` | `tools/py_scripts/setup_crawler.bat` | 依赖安装脚本 |
| `data/questions.txt`（生成后） | `tools/output/questions.txt` | 临时输出，再复制到 `data/` |

## 注意事项

- `tools/` 目录下的文件**不参与 C 程序编译**，仅用于数据准备和维护
- 核心项目运行时只读取 `data/` 目录下的 `.dat` 和 `.txt` 文件
- 如果你从网上下载了 HTML 题库页面，先放到 `tools/data_sources/raw_html/`，再用爬虫解析
