DEFINES += -D LINUX
DEFINES += -D REPORT_SYSLOG
#DEFINES += -D REPORT_DEBUG
#DEFINES += -D REPORT_DUMP

CC := gcc
CPP := g++
LINK := g++

INCLUDES += -I ../

LIBS += -L../Communicator/
LIBS += -L../GPIO/
LIBS += -L../HTTP/
LIBS += -L../MODBUS/
LIBS += -L../RTSP/
LIBS += -L../Toolkit/

LIBS += -lCryptography
LIBS += -lGPIO
LIBS += -lHTTP
LIBS += -lMMPS
LIBS += -lMODBUS
LIBS += -lRTSP
LIBS += -lSignals
LIBS += -lToolkit
LIBS += -lCommunicator
LIBS += -pthread
LIBS += -lconfig++
LIBS += -lrt
LIBS += -lssl
LIBS += -lcrypto
LIBS += -lwiringPi
#LIBS += -lwiringPiDev

CFLAGS += -O2
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

CPPFLAGS += -O2
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

# ******************************************************************************

OBJECTS_ROOT          := Configuration.o GKrellM.o Kernel.o Main.o Parse.o
OBJECTS_DISPATCHER    := Dispatcher/Aviso.o Dispatcher/Communicator.o Dispatcher/Queue.o Dispatcher/Setup.o
OBJECTS_FABULATORIUM  := Fabulatorium/Fabulator.o Fabulatorium/Listener.o Fabulatorium/Session.o
OBJECTS_PÉRIPHÉRIQUE  := Peripherique/HumiditySensor.o Peripherique/HumidityStation.o Peripherique/ThermiqueSensor.o Peripherique/ThermiqueStation.o Peripherique/UPSDevice.o Peripherique/UPSDevicePool.o
OBJECTS_WWW           := WWW/Home.o WWW/Relay.o WWW/SessionManager.o WWW/SystemInformation.o WWW/Therma.o

all: Servus

Servus: $(OBJECTS_ROOT) $(OBJECTS_FABULATORIUM) $(OBJECTS_DISPATCHER) $(OBJECTS_PÉRIPHÉRIQUE) $(OBJECTS_WWW)
	$(LINK) $(LINKFLAGS) -o $@ $^ $(LIBS)

# ******************************************************************************

Configuration.o: Configuration.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

GKrellM.o: GKrellM.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

Kernel.o: Kernel.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

Main.o: Main.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

Parse.o: Parse.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

# ******************************************************************************

Dispatcher/Aviso.o: Dispatcher/Aviso.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

Dispatcher/Communicator.o: Dispatcher/Communicator.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

Dispatcher/Queue.o: Dispatcher/Queue.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

Dispatcher/Setup.o: Dispatcher/Setup.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

# ******************************************************************************

Fabulatorium/Fabulator.o: Fabulatorium/Fabulator.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

Fabulatorium/Listener.o: Fabulatorium/Listener.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

Fabulatorium/Session.o: Fabulatorium/Session.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

# ******************************************************************************

Peripherique/HumiditySensor.o: Peripherique/HumiditySensor.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

Peripherique/HumidityStation.o: Peripherique/HumidityStation.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

Peripherique/ThermiqueSensor.o: Peripherique/ThermiqueSensor.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

Peripherique/ThermiqueStation.o: Peripherique/ThermiqueStation.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

Peripherique/UPSDevice.o: Peripherique/UPSDevice.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

Peripherique/UPSDevicePool.o: Peripherique/UPSDevicePool.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

# ******************************************************************************

WWW/Home.o: WWW/Home.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

WWW/Relay.o: WWW/Relay.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

WWW/SessionManager.o: WWW/SessionManager.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

WWW/SystemInformation.o: WWW/SystemInformation.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

WWW/Therma.o: WWW/Therma.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $(DEFINES) $< -o $@

# ******************************************************************************

install:
	sudo install --owner=root --group=root --mode=0755 Servus.rc /etc/init.d/servus
	sudo install --owner=root --group=root --mode=0755 --directory /opt/castellum/
	sudo install --owner=root --group=root --mode=0755 --preserve-timestamps --strip Servus /opt/castellum/servus

setup:
	sudo install --owner=root --group=root --mode=0644 --preserve-timestamps Default.conf /opt/castellum/servus.conf

clean:
	rm -fv Servus
	find . -type f -name "*.o" | xargs rm -fv *.o

run:
	sudo killall --quiet --wait servus | echo "Stopped"
	sudo /opt/castellum/servus | echo "Started"
