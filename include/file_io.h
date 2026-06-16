#ifndef FILE_IO_H
#define FILE_IO_H

void init_data(void);
void load_all(void);
void save_all(void);
void load_admins(void);
void save_admins(void);
void load_students(void);
void save_students(void);
void load_appointments(void);
void save_appointments(void);
void load_venues(void);
void save_venues(void);
void load_questions(void);
void save_questions(void);
void load_scores(void);
void save_scores(void);

int import_questions_from_txt(const char *filename);

#endif
