#ifndef COMMON_H
#define COMMON_H

char getch(void);

void clear_screen(void);
void print_line(void);
void print_title(const char *title);
void pause_screen(void);
int get_choice(void);
void get_password(char *buf, int max_len);
int validate_id_card(const char *id_card);
int is_date_valid(const char *date);
int is_time_slot_valid(const char *slot);
void get_current_date(char *buf, int len);

#endif
