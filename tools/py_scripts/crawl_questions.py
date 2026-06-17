#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
crawl_questions.py
Windows/VS Code 驾考题库爬虫

Usage:
  VS Code 终端中运行:  python crawl_questions.py

依赖安装:
  pip install requests beautifulsoup4 lxml
"""

import requests
from bs4 import BeautifulSoup
import json
import csv
import re
import time
import random
import os

HEADERS = {
    "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
    "Accept-Language": "zh-CN,zh;q=0.9",
}

SOURCES = {
    "subject1": "https://www.jsyks.com/ks/c5bd49.htm",
    "subject4": "https://www.jsyks.com/ks/s4.htm",
}

OUTPUT_DIR = "data"
OUTPUT_TXT = os.path.join(OUTPUT_DIR, "questions.txt")
OUTPUT_JSON = os.path.join(OUTPUT_DIR, "questions.json")
OUTPUT_CSV = os.path.join(OUTPUT_DIR, "questions.csv")


def clean_text(text):
    if not text:
        return ""
    text = text.replace("\n", " ").replace("\r", " ").replace("\t", " ")
    text = re.sub(r"\s+", " ", text).strip()
    text = text.replace("|", "｜")
    return text


def random_sleep():
    time.sleep(random.uniform(1, 3))


def fetch_page(url):
    try:
        print(f"[Request] Fetching: {url}")
        resp = requests.get(url, headers=HEADERS, timeout=15)
        resp.encoding = "utf-8"
        if resp.status_code == 200:
            print(f"[OK] Status: {resp.status_code}")
            return BeautifulSoup(resp.text, "lxml")
        else:
            print(f"[Error] HTTP {resp.status_code}")
            return None
    except Exception as e:
        print(f"[Error] {e}")
        return None


def parse_questions(soup, subject_id):
    questions = []
    if not soup:
        return questions

    all_text = soup.get_text(separator="\n")
    lines = all_text.split("\n")
    current = None
    option_buffer = []

    for line in lines:
        line = line.strip()
        if not line:
            continue

        # Detect question number like: 1. xxx or 1．xxx
        if re.match(r"^\d+[\.\uff0e\u3001\s]+", line):
            if current:
                questions.append(current)
            content = re.sub(r"^\d+[\.\uff0e\u3001\s]+", "", line).strip()
            current = {
                "id": len(questions) + 1,
                "subject": subject_id,
                "type": 1,
                "content": clean_text(content),
                "options": ["", "", "", ""],
                "answer": "",
                "explanation": ""
            }
            option_buffer = []

        elif re.match(r"^[A-Da-d][\.\uff0e\u3001\s]+", line):
            if current:
                letter = line[0].upper()
                text = re.sub(r"^[A-Da-d][\.\uff0e\u3001\s]+", "", line).strip()
                idx = ord(letter) - ord('A')
                if 0 <= idx < 4:
                    current["options"][idx] = clean_text(text)

        elif line in ["Correct", "Wrong"]:
            if current:
                option_buffer.append(line)
                if len(option_buffer) <= 2:
                    idx = len(option_buffer) - 1
                    current["options"][idx] = line
                    current["type"] = 2

    if current:
        questions.append(current)

    print(f"[Parse] Extracted {len(questions)} questions (subject {subject_id})")
    return questions


def to_txt(questions, filename=OUTPUT_TXT):
    with open(filename, "w", encoding="utf-8") as f:
        f.write("# Questions file\n")
        f.write("# format: id|subject|type|content|optA|optB|optC|optD|answer|explanation\n")
        f.write("# ============================================\n")
        for q in questions:
            line = "|".join([
                str(q["id"]), str(q["subject"]), str(q["type"]),
                q["content"], q["options"][0], q["options"][1],
                q["options"][2], q["options"][3],
                q["answer"], q["explanation"]
            ]) + "\n"
            f.write(line)
    print(f"[Export] TXT: {filename}")


def to_json(questions, filename=OUTPUT_JSON):
    with open(filename, "w", encoding="utf-8") as f:
        json.dump(questions, f, ensure_ascii=False, indent=2)
    print(f"[Export] JSON: {filename}")


def to_csv(questions, filename=OUTPUT_CSV):
    with open(filename, "w", encoding="utf-8-sig", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["id", "subject", "type", "content", "A", "B", "C", "D", "answer", "explanation"])
        for q in questions:
            writer.writerow([
                q["id"], q["subject"], q["type"], q["content"],
                q["options"][0], q["options"][1], q["options"][2], q["options"][3],
                q["answer"], q["explanation"]
            ])
    print(f"[Export] CSV: {filename}")


def main():
    print("=" * 50)
    print("    Driving Exam Question Crawler")
    print("=" * 50)
    print()
    print("Select operation:")
    print("  [1] Crawl Subject 1 from web")
    print("  [2] Crawl Subject 4 from web")
    print("  [3] Crawl both Subject 1 + 4")
    print("  [4] Parse local HTML file")
    print("  [0] Exit")
    print()
    choice = input("Choice: ").strip()

    all_questions = []

    if choice == "1":
        soup = fetch_page(SOURCES["subject1"])
        all_questions = parse_questions(soup, 1)
    elif choice == "2":
        soup = fetch_page(SOURCES["subject4"])
        all_questions = parse_questions(soup, 4)
    elif choice == "3":
        for key, sid in [("subject1", 1), ("subject4", 4)]:
            soup = fetch_page(SOURCES[key])
            all_questions.extend(parse_questions(soup, sid))
            random_sleep()
    elif choice == "4":
        path = input("Local HTML file path: ").strip()
        try:
            with open(path, "r", encoding="utf-8") as f:
                soup = BeautifulSoup(f.read(), "lxml")
            sid = int(input("Subject (1=Sub1, 4=Sub4): ").strip())
            all_questions = parse_questions(soup, sid)
        except Exception as e:
            print(f"[Error] {e}")
            return
    elif choice == "0":
        print("Exit")
        return
    else:
        print("Invalid")
        return

    if not all_questions:
        print("[Warning] No questions found. Possible reasons:")
        print("  - Website structure changed")
        print("  - Anti-bot protection triggered")
        print("  - Network issue")
        return

    for i, q in enumerate(all_questions, 1):
        q["id"] = i

    print()
    print(f"[Stats] Total: {len(all_questions)} questions")
    s1 = sum(1 for q in all_questions if q["subject"] == 1)
    s4 = sum(1 for q in all_questions if q["subject"] == 4)
    print(f"       Subject 1: {s1}")
    print(f"       Subject 4: {s4}")
    print()

    os.makedirs(OUTPUT_DIR, exist_ok=True)
    to_txt(all_questions)
    to_json(all_questions)
    to_csv(all_questions)

    print()
    print("[Done] Files generated in data/:")
    print("  - questions.txt  (import format)")
    print("  - questions.json (debug)")
    print("  - questions.csv  (Excel)")


if __name__ == "__main__":
    main()
