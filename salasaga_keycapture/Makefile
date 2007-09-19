CPPFLAGS =	-DBUILD_DLL -O2 -g -Wall -fmessage-length=0

LIBS =

TARGET =	flame-keycapture.dll

$(TARGET):	$(OBJS)
	$(CXX) -shared -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
