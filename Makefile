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

LIBS += -lGPIO
LIBS += -lHTTP
LIBS += -lMMPS
LIBS += -lMODBUS
LIBS += -lSignals
LIBS += -lToolkit
LIBS += -lCommunicator
LIBS += -pthread
LIBS += -lconfig++
LIBS += -lbz2
LIBS += -lm
LIBS += -lrt
LIBS += -lwiringPi
LIBS += -lwiringPiDev

CFLAGS := -O2
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

CPPFLAGS := -O2
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

OBJECTS_ROOT    := GKrellM.o Kernel.o Main.o
OBJECTS_WWW     := WWW/Home.o WWW/Relay.o WWW/SystemInformation.o WWW/Therma.o

# ******************************************************************************

all: Servus

# ******************************************************************************

Servus: $(OBJECTS_ROOT) $(OBJECTS_WWW)
	$(LINK) $(LINKFLAGS) -o $@ $^ $(LIBS)

GKrellM.o: GKrellM.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

Kernel.o: Kernel.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

Main.o: Main.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

# ******************************************************************************

WWW/Home.o: WWW/Home.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

WWW/Relay.o: WWW/Relay.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

WWW/SystemInformation.o: WWW/SystemInformation.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

WWW/Therma.o: WWW/Therma.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

# ******************************************************************************

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
