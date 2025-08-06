CC = gcc
CFLAGS  = -Wall

TARGET = generate_cc_lut

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c -lm

clean:
	$(RM) $(TARGET)
