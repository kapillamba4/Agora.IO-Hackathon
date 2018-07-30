ifeq (${CXX},)
CXX=g++
endif
LINK=${CXX}
TOPDIR=`pwd`
COMMONPATH=`pwd`/../..
LIBPATH=${COMMONPATH}/libs
#set your jvm path!!!
#JNIINCLUDE = /usr/lib/jvm/java-9-openjdk-amd64/include/
STDFLAGS = -std=c++0x
LDFLAGS= -shared
FPIC = -fPIC
CXXFLAGS  = -pipe -std=c++0x -fPIC -g -fno-omit-frame-pointer \
			-DNDEBUG=1 -Wconversion -O3 -Wall -W #-fvisibility=hidden
					
LIB	   = -pthread -lpthread -L$(LIBPATH) -lrecorder -lrt
INCPATH =-I. -I${COMMONPATH}/include -I${COMMONPATH}/include/base -I${COMMONPATH}/samples/base -I${COMMONPATH}/samples/agorasdk -I${COMMONPATH}/samples
#JNIPATH = -I. -I/$(JNIINCLUDE) -I/$(JNIINCLUDE)/linux/

JNIPATH=-I${JNIINCLUDEPATH} -I${JNIINCLUDEPATH}/linux
OBJ = opt_parser.o

REALTARGET = exe
TARGET=librecording.so

.PHONY: all clean
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) -c ./native $(CXXFLAGS) $(INCPATH) ${COMMONPATH}/samples/agorasdk/AgoraSdk.cpp
	mv AgoraSdk.o ./bin
	
	$(LINK) ./native/AgoraJniProxy.cpp -o $@ $(LDFLAGS) $(FPIC) $(INCPATH) $(JNIPATH) $(STDFLAGS) ./bin/AgoraSdk.o ./bin/opt_parser.o $(LIB) -I.
	mv $@ ./bin

$(OBJ):
	$(CXX) -c ./bin $(CXXFLAGS) $(INCPATH) ${COMMONPATH}/samples/base/opt_parser.cpp
	mv opt_parser.o ./bin

clean:
	rm -f ./bin/$(TARGET)
	rm -f ${OBJ}
	rm -f ./bin/*.o
