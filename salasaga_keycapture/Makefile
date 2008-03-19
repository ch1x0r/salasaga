CFLAGS =	-DBUILD_DLL -O2 -g -Wall -fmessage-length=0

LIBS =

TARGET =	salasaga_keycapture.dll

$(TARGET):
	$(CC) $(CFLAGS) -shared -o $(TARGET) -c salasaga_keycapture.c $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(TARGET)
