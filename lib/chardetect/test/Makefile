TARGET=test-chardetect

all: $(TARGET)

$(TARGET): test-chardetect.c
	$(CC) \
		-O2 \
		-o $(TARGET) \
		test-chardetect.c \
		-I../include \
		-ldl

clean:
	rm -f $(TARGET)
