CC = gcc
CFLAGS = -Wall -g -Iinclude
SRC = src/main.c src/common.c src/student.c src/appointment.c src/exam_sim.c src/file_io.c
OBJ = $(SRC:.c=.o)
TARGET = driving_exam

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

src/%.o: src/%.c include/*.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)

run: $(TARGET)
	./$(TARGET)
