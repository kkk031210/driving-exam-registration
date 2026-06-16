@echo off
chcp 65001 >nul
REM Windows batch script for setting up Python dependencies for the crawler

echo ============================================
echo   Driving Exam Question Crawler - Setup
echo ============================================
echo.

REM Check if Python is installed
python --version >nul 2>&1
if errorlevel 1 (
    echo [Error] Python is not installed or not in PATH.
    echo Please install Python from https://www.python.org/
    pause
    exit /b 1
)

echo [OK] Python detected
echo.

REM Install required packages
echo Installing dependencies: requests, beautifulsoup4, lxml...
pip install requests beautifulsoup4 lxml -i https://pypi.tuna.tsinghua.edu.cn/simple

if errorlevel 1 (
    echo [Error] Failed to install packages. Try: pip install requests beautifulsoup4 lxml
    pause
    exit /b 1
)

echo.
echo [Done] Dependencies installed.
echo.
echo Now run:  python crawl_questions.py
echo.
pause
