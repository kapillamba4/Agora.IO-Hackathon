ifeq (${CXX},)
CXX=g++
endif
LINK=${CXX}

TOPDIR=`pwd`/../..
COMMONPATH=`pwd`/..
LIBPATH=${TOPDIR}/libs
#-static-libstdc++ 
LDFLAGS= -static-libgcc -std=c++11 -Wl,-rpath,$(LIBPATH)
CXXFLAGS  = -pipe -std=c++0x -fPIC -g -fno-omit-frame-pointer \
			-DNDEBUG=1 -Wconversion -O3 -Wall -W -fvisibility=hidden
					
LIB	   = -pthread -lpthread -L$(LIBPATH) -lrecorder -lrt
INCPATH =-I. -I${TOPDIR}/include -I${COMMONPATH}

SRC =$(wildcard ../base/*.cpp ../agorasdk/*.cpp *.cpp)
OBJ=$(addsuffix .o, $(basename $(SRC)))
TARGET=recorder_local


.PHONY: all clean
all: $(TARGET)

$(TARGET): $(OBJ)
	$(LINK) $(LDFLAGS) $(INCPATH) $^ -o "$@" $(LIB)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

clean:
	rm -f $(TARGET)
	rm -f ${OBJ}
