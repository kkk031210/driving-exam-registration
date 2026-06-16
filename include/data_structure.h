#ifndef DATA_STRUCTURE_H
#define DATA_STRUCTURE_H

#define MAX_SIZE 1000
#define MAX_VENUES 10
#define MAX_QUESTIONS 2000

#define SUBJECT_1 1
#define SUBJECT_4 4

#define TYPE_SINGLE 1
#define TYPE_JUDGE 2
#define TYPE_MULTI 3

typedef struct {
    int id;
    char id_card[19];
    char name[51];
    char phone[16];
    char car_type[5];
    char status[10];
    char password[33];
} Student;

typedef struct {
    int id;
    char username[33];
    char password[33];
} Admin;

typedef struct {
    int id;
    int student_id;
    char subject[10];
    char exam_date[11];
    char exam_time[10];
    int venue_id;
    char status[10];
} Appointment;

typedef struct {
    int id;
    char name[51];
    char address[101];
    int max_per_slot;
} Venue;

typedef struct {
    int id;
    int subject;
    int type;
    char content[256];
    char options[4][128];
    char answer[10];
    char explanation[256];
} Question;

typedef struct {
    int id;
    int student_id;
    int subject;
    int score;
    int total;
    int correct;
    int used_time;
    char exam_date[20];
} ExamScore;

extern Student students[MAX_SIZE];
extern int student_count;

extern Admin admins[MAX_SIZE];
extern int admin_count;

extern Appointment appointments[MAX_SIZE];
extern int appointment_count;

extern Venue venues[MAX_VENUES];
extern int venue_count;

extern Question questions[MAX_QUESTIONS];
extern int question_count;

extern ExamScore scores[MAX_SIZE];
extern int score_count;

extern int current_role;
extern int current_user_id;

#endif
