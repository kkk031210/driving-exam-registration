#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data_structure.h"
#include "common.h"
#include "file_io.h"

#define FILE_STUDENTS   "data/students.dat"
#define FILE_ADMINS     "data/admins.dat"
#define FILE_APPOINTMENTS "data/appointments.dat"
#define FILE_VENUES     "data/venues.dat"
#define FILE_QUESTIONS  "data/questions.dat"
#define FILE_SCORES     "data/scores.dat"

void init_data(void) {
    FILE *fp = fopen(FILE_ADMINS, "rb");
    if (fp == NULL) {
        Admin admin = {1, "admin", "admin123"};
        FILE *fw = fopen(FILE_ADMINS, "wb");
        if (fw) {
            fwrite(&admin, sizeof(Admin), 1, fw);
            fclose(fw);
        }
        printf("[系统] 首次运行，已创建默认管理员 admin / admin123\n");
        pause_screen();
    } else {
        fclose(fp);
    }

    fp = fopen(FILE_VENUES, "rb");
    if (fp == NULL) {
        Venue v[] = {
            {1, "市北考场", "北环路88号", 30},
            {2, "市南考场", "南环路66号", 25},
            {3, "高新区考场", "高新大道100号", 40}
        };
        FILE *fw = fopen(FILE_VENUES, "wb");
        if (fw) {
            fwrite(v, sizeof(Venue), 3, fw);
            fclose(fw);
        }
    } else {
        fclose(fp);
    }
}

void load_all(void) {
    load_admins();
    load_students();
    load_appointments();
    load_venues();
    load_questions();
    load_scores();
}

void save_all(void) {
    save_admins();
    save_students();
    save_appointments();
    save_venues();
    save_questions();
    save_scores();
}

void load_admins(void) {
    FILE *fp = fopen(FILE_ADMINS, "rb");
    if (fp) {
        admin_count = fread(admins, sizeof(Admin), MAX_SIZE, fp);
        fclose(fp);
    } else {
        admin_count = 0;
    }
}

void save_admins(void) {
    FILE *fp = fopen(FILE_ADMINS, "wb");
    if (fp) {
        fwrite(admins, sizeof(Admin), admin_count, fp);
        fclose(fp);
    }
}

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
