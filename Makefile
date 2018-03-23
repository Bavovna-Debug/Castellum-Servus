DEFINES += -D LINUX
#DEFINES += -D DEBUG
DEFINES += -D REPORT_DEBUG
#DEFINES += -D REPORT_DUMP
DEFINES += -D REPORT_SYSLOG

CC := gcc
CPP := g++
LINK := g++

INCLUDES += -I ../

LIBS += -L../Communicator/
LIBS += -L../GPIO/
LIBS += -L../HTTP/
LIBS += -L../MMPS/
LIBS += -L../MODBUS/
LIBS += -L../Signals/
LIBS += -L../Toolkit/

LIBS += -lCommunicator
LIBS += -lGPIO
LIBS += -lHTTP
LIBS += -lMMPS
LIBS += -lMODBUS
LIBS += -lSignals
LIBS += -lToolkit
LIBS += -pthread
LIBS += -lconfig++
LIBS += -lbz2
LIBS += -lm
LIBS += -lrt
LIBS += -lwiringPi
LIBS += -lwiringPiDev

CFLAGS := -c
CFLAGS += -std=c99
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Wbad-function-cast
CFLAGS += -Wdeprecated-declarations
CFLAGS += -Wimplicit-function-declaration
CFLAGS += -Wpointer-sign
CFLAGS += -Wsign-compare
CFLAGS += -Wtype-limits
CFLAGS += -Wunknown-pragmas
CFLAGS += -Wunused-but-set-variable
CFLAGS += -Wunused-parameter
CFLAGS += -Wunused-value
CFLAGS += -Wwrite-strings

CPPFLAGS := -c
CPPFLAGS += -std=c++11
CPPFLAGS += -Wall
CPPFLAGS += -Wextra
CPPFLAGS += -Wdeprecated-declarations
CPPFLAGS += -Wsign-compare
CPPFLAGS += -Wtype-limits
CPPFLAGS += -Wunknown-pragmas
CPPFLAGS += -Wunused-but-set-variable
CPPFLAGS += -Wunused-parameter
CPPFLAGS += -Wunused-value
CPPFLAGS += -Wwrite-strings

LINKFLAGS := -O1

OBJECTS_ROOT    := GKrellM.o Main.o Workspace.o
OBJECTS_WWW     := WWW/Home.o WWW/Relay.o WWW/SystemInformation.o WWW/Therma.o
OBJECTS_TOOLS  	:= Tools/Debug.o Tools/Memory.o

all: Servus

Servus: $(OBJECTS_ROOT) $(OBJECTS_WWW) $(OBJECTS_TOOLS)
	$(LINK) $(LINKFLAGS) -o $@ $^ $(LIBS)

GKrellM.o: GKrellM.cpp
	$(CPP) $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

Main.o: Main.cpp
	$(CPP) $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

Workspace.o: Workspace.cpp
	$(CPP) $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

# ****************************************
# WWW
# ****************************************

WWW/Home.o: WWW/Home.cpp
	$(CPP) $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

WWW/Relay.o: WWW/Relay.cpp
	$(CPP) $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

WWW/SystemInformation.o: WWW/SystemInformation.cpp
	$(CPP) $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

WWW/Therma.o: WWW/Therma.cpp
	$(CPP) $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

# ****************************************
# Tools
# ****************************************

Tools/Debug.o: Tools/Debug.cpp
	$(CPP) $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

Tools/Memory.o: Tools/Memory.cpp
	$(CPP) $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

install:
	sudo install --owner=root --group=root --mode=0755 Servus.rc /etc/init.d/servus
	sudo install --owner=root --group=root --mode=0755 --directory /opt/servus/
	sudo install --owner=root --group=root --mode=0755 --preserve-timestamps --strip Servus /opt/servus/servus

setup-erdbeere:
	sudo install --owner=root --group=root --mode=0644 --preserve-timestamps Setup/Erdbeere.conf /opt/servus/servus.conf

setup-himbeere:
	sudo install --owner=root --group=root --mode=0644 --preserve-timestamps Setup/Himbeere.conf /opt/servus/servus.conf

clean:
	rm -fv Servus
	find . -type f -name "*.o" | xargs rm -fv *.o

run:
	killall --quiet --wait servus | echo "Stopped"
	/opt/servus/servus | echo "Started"
