// System definition files.
//
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/procfs.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

// Common definition files.
//
#include "GPIO/Therma.hpp"
#include "Toolkit/Report.h"
#include "Toolkit/Times.hpp"
#include "Toolkit/Types.h"

// Local definition files.
//
#include "Servus/GKrellM.hpp"
#include "Servus/Kernel.hpp"

static void*
GKrellM_ThreadHandler(void*);

static int
GKrellM_SocketOpenReopen(struct GKrellM_Service*);

static int
GKrellM_SocketClose(struct GKrellM_Service*);

static int
GKrellM_SocketListen(struct GKrellM_Service*);

static struct GKrellM_Session*
GKrellM_CreateSession(int sockFD);

static int
GKrellM_CancelSession(struct GKrellM_Session*);

static int
GKrellM_SetupSession(struct GKrellM_Session*);

static int
GKrellM_StartSessionTimers(struct GKrellM_Session*);

static int
GKrellM_StopSessionTimers(struct GKrellM_Session*);

static int
GKrellM_SendStatement(struct GKrellM_Session*);

static void
GKrellM_SecondsBell(void*);

static void
GKrellM_UpdaterBell(void*);

/**
 * @brief   Prepare resources of GKrellM server and start it as a thread.
 *
 * @param[in]   service     Pointer to GKrellM service descriptor.
 *
 * @return  0 upon successful completion.
 * @return  Error code (negative value), in case of error.
 */
int
GKrellM_Start(struct GKrellM_Service* service)
{
    int rc;

    // Reset socket descriptor. Otherwise, GKrellM_SocketOpenReopen()
    // may try to close the socket which is actually not open.
    //
    service->listeningSockFD = 0;

    // Start the thread for GKrellM server.
    //
    {
        rc = pthread_create(
                &service->thread,
                NULL,
                GKrellM_ThreadHandler,
                service);
        if (rc != 0)
        {
            ReportSoftAlert("[GKrellM] Cannot create thread for GKrellM server: " \
                    "rc=%d", rc);

            return -1;
        }
    }

    return 0;
}

/**
 * @brief   Thread handler for GKrellM server.
 *
 * @param[in]   arg         Pointer to GKrellM service descriptor.
 */
static void*
GKrellM_ThreadHandler(void* arg)
{
    struct GKrellM_Service* service;
    int                     rc;

    service = (struct GKrellM_Service*) arg;

    ReportInfo("[GKrellM] Started GKrellM server in thread: pid=%d, tid=%ld",
            getpid(),
            syscall(SYS_gettid));

    for (;;)
    {
        rc = GKrellM_SocketOpenReopen(service);
        if (rc != 0)
        {
            ReportWarning(
                    "[GKrellM] Wait for %d milliseconds before retry to create socket for GKrellM",
                    GKRELLM_WAIT_IF_CANNOT_ESTABLISH_SOCKET);

            usleep(GKRELLM_WAIT_IF_CANNOT_ESTABLISH_SOCKET *
                    MICROSECONDS_PER_MILLISECOND);

            continue;
        }

        rc = GKrellM_SocketListen(service);
        if (rc != 0)
        {
            // Listen should never break. But even if it happens, just try everything
            // from the beginning: recreate a socket and start listening again.
            //
            continue;
        }
    }

    pthread_exit(NULL);
}

/**
 * @brief   Open or reopen a socket of GKrellM server.
 *
 * @param   service         Pointer to GKrellM service descriptor.
 *
 * @return  0 upon successful completion.
 * @return  Error code (negative value), if socket cannot be opened.
 */
static int
GKrellM_SocketOpenReopen(struct GKrellM_Service* service)
{
    int         sockFD;
    const int   socketOptionReuseAddress = 1;
    const int   socketOptionNagleAlgorithm = 1;
    int         rc;

    // If socket is still opened then try to close it.
    //
    GKrellM_SocketClose(service);

    // Try to open a socket.
    //
    {
        sockFD = socket(AF_INET, SOCK_STREAM, 0);
        if (sockFD < 0)
        {
            ReportError("[GKrellM] Cannot open a socket for GKrellM server: errno=%d", errno);

            return -1;
        }

        // Since the socket is valid notice its descriptor on workspace
        // to make it available for error recovery routines.
        //
        service->listeningSockFD = sockFD;
    }

    // Notify the stack that the socket address needs to be reused.
    // Important for the case when the socket needed to be reopened.
    // If it does not work, then close the socket to let recovery go on.
    //
    {
        rc = setsockopt(sockFD,
                SOL_SOCKET,
                SO_REUSEADDR,
                (char*) &socketOptionReuseAddress,
                sizeof(socketOptionReuseAddress));
        if (rc == -1)
        {
            GKrellM_SocketClose(service);

            ReportError("[GKrellM] Cannot set socket options: errno=%d", errno);

            return -1;
        }
    }

    // Switch off the 'Nagle' algorithm to let the packets be sent immediately.
    //
    {
        rc = setsockopt(sockFD,
                IPPROTO_TCP,
                TCP_NODELAY,
                (char*) &socketOptionNagleAlgorithm,
                sizeof(socketOptionNagleAlgorithm));
        if (rc == -1)
        {
            GKrellM_SocketClose(service);

            ReportError("[GKrellM] Cannot set socket options: errno=%d", errno);

            return -1;
        }
    }

    // Bind GKrellM socket to TCP port.
    //
    {
        memset(&service->serverAddress, 0, sizeof(struct sockaddr_in));

        // Prepare parameters for bind. Bind to all addresses defined
        // on network interface.
        //
        service->serverAddress.sin_family = AF_INET;
        service->serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        service->serverAddress.sin_port = htons(GKRELLM_TCP_PORT);

        rc = bind(sockFD,
                (struct sockaddr*) &service->serverAddress,
                sizeof(struct sockaddr_in));
        if (rc == -1)
        {
            if (errno == EADDRINUSE)
            {
                // Bind refused: wait and try again.

                GKrellM_SocketClose(service);

                ReportError("[GKrellM] TCP port %u is still in use",
                        GKRELLM_TCP_PORT);

                return -1;
            }
            else
            {
                // Error: close socket, wait, and go to the beginning
                // of socket creation.

                GKrellM_SocketClose(service);

                ReportError("[GKrellM] Cannot bind to socket: errno=%d", errno);

                return -1;
            }
        }
    }

    // Start listening on GKrellM socket.
    //
    {
        rc = listen(sockFD, SOMAXCONN);
        if (rc == -1)
        {
            GKrellM_SocketClose(service);

            ReportError("[GKrellM] Cannot start listening on a socket: errno=%d", errno);

            return -1;
        }
    }

    return 0;
}

/**
 * @brief   Close a socket if it is open.
 *
 * @param[in]   service     Pointer to GKrellM service descriptor.
 *
 * @return  0 upon successful completion.
 * @return  Error code (negative value), if socket cannot be closed.
 */
static int
GKrellM_SocketClose(struct GKrellM_Service* service)
{
    int sockFD;
    int rc;

    sockFD = service->listeningSockFD;

    // If socket is still opened then try to close it.
    //
    if (sockFD != 0)
    {
        service->listeningSockFD = 0;

        rc = close(sockFD);
        if (rc == -1)
        {
            ReportError("[GKrellM] Error has occurred on socket close: " \
                    "errno=%d", errno);

            return -1;
        }
    }

    return 0;
}

/**
 * @brief   Start listening for incoming connection requests.
 *
 * @param[in]   service     Pointer to GKrellM service descriptor.
 *
 * @return  0 upon successful completion.
 * @return  Error code (negative value), in case of error in socket layer.
 */
static int
GKrellM_SocketListen(struct GKrellM_Service* service)
{
    struct GKrellM_Session* session;
    struct sockaddr_in      clientAddress;
    socklen_t               clientAddressLength;
    int                     connectedSockFD;
    int                     rc;

    clientAddressLength = sizeof(clientAddress);

    for (;;)
    {
        connectedSockFD = accept(
                service->listeningSockFD,
                (struct sockaddr*) &clientAddress,
                &clientAddressLength);
        if (connectedSockFD == -1)
        {
            ReportError("[GKrellM] Cannot accept new socket: errno=%d", errno);

            return -1;
        }

        ReportInfo("[GKrellM] Incoming connection from %s",
                inet_ntoa(clientAddress.sin_addr));

        session = GKrellM_CreateSession(connectedSockFD);
        if (session == NULL)
        {
            close(connectedSockFD);

            continue;
        }

        rc = GKrellM_SetupSession(session);
        if (rc != 0)
        {
            GKrellM_CancelSession(session);

            continue;
        }

        rc = GKrellM_StartSessionTimers(session);
        if (rc != 0)
        {
            GKrellM_CancelSession(session);

            continue;
        }
    }

    return 0;
}

/**
 * @brief   Allocate resources for new GKrellM session.
 *
 * @param[in]   connectedSockFD Socket descriptor of a new incomming connection.
 *
 * @return  0 upon successful completion.
 * @return  Error code (negative value), in case of error.
 */
static struct GKrellM_Session*
GKrellM_CreateSession(int connectedSockFD)
{
    struct GKrellM_Session* session;
    int                     rc;

    session = (struct GKrellM_Session*) malloc(sizeof(struct GKrellM_Session));
    if (session == NULL)
    {
        ReportSoftAlert("[GKrellM] Out of memory");

        return NULL;
    }

    memset(session, 0, sizeof(struct GKrellM_Session));

    // Initialize a lock to protect from simultaneous network transmit.
    //
    {
        pthread_mutexattr_init(&session->transmission.mutexAttr);

        pthread_mutexattr_setpshared(
                &session->transmission.mutexAttr,
                PTHREAD_PROCESS_PRIVATE);

        rc = pthread_mutex_init(
                &session->transmission.mutex,
                &session->transmission.mutexAttr);
        if (rc != 0)
        {
            ReportSoftAlert("[GKrellM] Cannot initialize mutex: rc=%d", rc);

            return NULL;
        }
    }

    session->sessionSockFD = connectedSockFD;

    session->secondsCounter = 0;

    return session;
}

/**
 * @brief   Stop session timers, disconnect from GKrellM client, and release session resources.
 *
 * @param[in]   session     Pointer to GKrellM session descriptor.
 *
 * @return  0 upon successful completion.
 * @return  Error code (negative value), in case of error.
 */
static int
GKrellM_CancelSession(struct GKrellM_Session* session)
{
    int rc;

    // Stop timers inside of lock to prevent that any timer is active upon session cancellation.
    //
    {
        rc = pthread_mutex_lock(&session->transmission.mutex);
        if (rc != 0)
        {
            ReportSoftAlert("[GKrellM] Error on mutex lock: rc=%d", rc);

            return -1;
        }

        GKrellM_StopSessionTimers(session);

        rc = pthread_mutex_unlock(&session->transmission.mutex);
        if (rc != 0)
        {
            ReportSoftAlert("[GKrellM] Error on mutex unlock: rc=%d", rc);

            return -1;
        }
    }

    // Wait one update interval long to make sure timer will not be fired any more.
    //
    usleep(GKRELLM_UPDATE_INTERVAL * MICROSECONDS_PER_MILLISECOND);

    // Hold the lock once more and release it immediately. This will make sure
    // that even if any timer has been still fired last time the execution of timer handler
    // has been completed.
    //
    {
        rc = pthread_mutex_lock(&session->transmission.mutex);
        if (rc != 0)
        {
            ReportSoftAlert("[GKrellM] Error on mutex lock: rc=%d", rc);

            return -1;
        }

        rc = pthread_mutex_unlock(&session->transmission.mutex);
        if (rc != 0)
        {
            ReportSoftAlert("[GKrellM] Error on mutex unlock: rc=%d", rc);

            return -1;
        }
    }

    // As long timers are not running any more, the mutex could be released.
    //
    {
        rc = pthread_mutex_destroy(&session->transmission.mutex);
        if (rc != 0)
        {
            ReportSoftAlert("Error on GKrellM mutex destroy: rc=%d", rc);

            return -1;
        }

        rc = pthread_mutexattr_destroy(&session->transmission.mutexAttr);
        if (rc != 0)
        {
            ReportSoftAlert("[GKrellM] Error on mutex attributes destroy: rc=%d", rc);

            return -1;
        }
    }

    // Explicitly close network connection. Do not check return code of close().
    // If network connection has already failed it does not matter.
    //
    if (session->sessionSockFD != 0)
    {
        close(session->sessionSockFD);
    }

    free(session);

    ReportInfo("[GKrellM] Connection closed");

    return 0;
}

/**
 * @brief   Send initial setting data to GKrellM client.
 *
 * @param[in]   session     Pointer to GKrellM session descriptor.
 *
 * @return  0 upon successful completion.
 * @return  Error code (negative value), in case of error.
 */
static int
GKrellM_SetupSession(struct GKrellM_Session* session)
{
    struct pollfd   pollFD;
    int             timeout;
    ssize_t         bytesReceived;
    time_t          now;
    struct tm*      tm;
    int             rc;

    Workspace::Kernel& kernel = Workspace::Kernel::SharedInstance();
    Therma::Service& thermaService = Therma::Service::SharedInstance();

    // 1st step: Wait if any data is suppose to come at all.
    // Quit the thread if nothing happens on a line.
    //
    {
        timeout = GKRELLM_SESSION_TIMEOUT * MILLISECONDS_PER_SECOND;

        pollFD.fd = session->sessionSockFD;
        pollFD.events = POLLIN;
        pollFD.revents = 0; // Just for debug purposes.

        rc = poll(&pollFD, 1, timeout);
        if (pollFD.revents & (POLLERR | POLLHUP | POLLNVAL))
        {
            ReportError("[GKrellM] " \
                    "Poll error on waiting for GKrellM client: revents=0x%04X",
                    pollFD.revents);

            return -1;
        }

        // If polling has timed out, then quit.
        //
        if (rc == 0)
        {
            ReportInfo("[GKrellM] Waiting for GKrellM client has timed out");

            return -1;
        }

        // If polling has failed, then quit.
        //
        if (rc != 1)
        {
            ReportError("[GKrellM] Poll error on waiting for GKrellM client");

            return -1;
        }
    }

    // 2nd step: receive session signature from GKrellM client.
    //
    {
        bytesReceived = recv(
                session->sessionSockFD,
                &session->buffer,
                GKRELLM_BUFFER_SIZE,
                0);
        if (bytesReceived < 0)
        {
            ReportError("[GKrellM] " \
                    "Error has occurred on receive from GKrellM client: " \
                    "errno=%d", errno);

            return -1;
        }

        // If nothing is received, then the connection is already closed.
        //
        if (bytesReceived == 0)
        {
            ReportNotice("[GKrellM] Connection lost");

            return -1;
        }

        if (strncmp(session->buffer, GKRELLM_SIGNATURE, strlen(GKRELLM_SIGNATURE)) != 0)
        {
            ReportNotice("[GKrellM] Connection is broken - wrong client signature");

            return -1;
        }

        // Set a string terminating null at the end to be able to handle
        // received data as a string.
        //
        while ((session->buffer[bytesReceived - 1] == '\r') || (session->buffer[bytesReceived - 1] == '\n'))
        {
            session->buffer[bytesReceived - 1] = '\0';
            bytesReceived--;

            // Break if string is empty.
            //
            if (bytesReceived == 0)
            {
                break;
            }
        }

        ReportInfo("[GKrellM] Established connection with '%s'", session->buffer);
    }

    // Setup GKrellM session.
    //
    {
        strcpy(session->buffer,
                "<gkrellmd_setup>\n");
        rc = GKrellM_SendStatement(session);
        if (rc != 0)
        {
            return -1;
        }

        {
            strcpy(session->buffer,
                    "<version>\n");
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return -1;
            }

            strcpy(session->buffer,
                    "Servus\n");
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return -1;
            }
        }

        {
            strcpy(session->buffer,
                    "<decimal_point>\n");
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return -1;
            }

            strcpy(session->buffer,
                    ".\n");
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return -1;
            }

            strcpy(session->buffer,
                    "\n");
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return -1;
            }
        }

        {
            strcpy(session->buffer,
                    "<sensors_setup>\n");
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return -1;
            }


            for (unsigned int sensorIndex = 0;
                 sensorIndex < thermaService.size();
                 sensorIndex++)
            {
                Therma::Sensor* sensor = thermaService[sensorIndex];

                sprintf(session->buffer,
                        "%u \"%s\" 0 0 0 0.0 0.0 \"NONE\" \"%s\" 0\n",
                        GKRELLM_SENSOR_TEMPERATURE,
                        sensor->deviceId,
                        sensor->name);
                rc = GKrellM_SendStatement(session);
                if (rc != 0)
                {
                    return -1;
                }
            }
        }

        {
            strcpy(session->buffer,
                    "<net_setup>\n");
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return -1;
            }

            strcpy(session->buffer,
                    "net_use_routed\n");
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return -1;
            }
        }

        {
            strcpy(session->buffer,
                    "<hostname>\n");
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return -1;
            }

            strcpy(session->buffer,
                    "Servus\n");
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return -1;
            }
        }

        {
            strcpy(session->buffer,
                    "<sysname>\n");
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return -1;
            }

            strcpy(session->buffer,
                    "Servus 1.0\n");
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return -1;
            }
        }

        {
            strcpy(session->buffer,
                    "<monitors>\n");
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return -1;
            }

            strcpy(session->buffer,
                    "uptime\n");
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return -1;
            }

            strcpy(session->buffer,
                    "sensors\n");
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return -1;
            }
        }

        {
            strcpy(session->buffer,
                    "<io_timeout>\n");
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return -1;
            }

            strcpy(session->buffer,
                    "0\n");
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return -1;
            }

            strcpy(session->buffer,
                    "\n");
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return -1;
            }
        }

        {
            strcpy(session->buffer,
                    "<reconnect_timeout>\n");
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return -1;
            }

            strcpy(session->buffer,
                    "5\n");
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return -1;
            }

            strcpy(session->buffer,
                    "\n");
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return -1;
            }
        }

        strcpy(session->buffer,
                "</gkrellmd_setup>\n");
        rc = GKrellM_SendStatement(session);
        if (rc != 0)
        {
            return -1;
        }
    }

    // Send initial values.
    //
    {
        strcpy(session->buffer,
                "<initial_update>\n");
        rc = GKrellM_SendStatement(session);
        if (rc != 0)
        {
            return -1;
        }

        // System date and time.
        //
        {
            strcpy(session->buffer,
                    "<time>\n");
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return -1;
            }

            time(&now);
            tm = localtime(&now);

            sprintf(session->buffer,
                    "%d %d %d %d %d %d %d %d %d\n",
                   tm->tm_sec,
                   tm->tm_min,
                   tm->tm_hour,
                   tm->tm_mday,
                   tm->tm_mon,
                   tm->tm_year,
                   tm->tm_wday,
                   tm->tm_yday,
                   tm->tm_isdst);
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return -1;
            }
        }

        // Uptime.
        //
        {
            strcpy(session->buffer,
                    "<uptime>\n");
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return -1;
            }

            sprintf(session->buffer,
                    "%lu\n",
                    (Toolkit::TimestampSeconds() - kernel.timestampOfStart) / SECOND_PER_MINUTE);
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return -1;
            }
        }

        // Sensors.
        //
        {
            strcpy(session->buffer,
                    "<sensors>\n");
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return -1;
            }

            for (unsigned int sensorIndex = 0;
                 sensorIndex < thermaService.size();
                 sensorIndex++)
            {
                Therma::Sensor* sensor = thermaService[sensorIndex];

                sprintf(session->buffer,
                        "%u \"%s\" 0 0 0 %.3f\n",
                        GKRELLM_SENSOR_TEMPERATURE,
                        sensor->deviceId,
                        sensor->temperature.current);
                rc = GKrellM_SendStatement(session);
                if (rc != 0)
                {
                    return -1;
                }
            }
        }

        strcpy(session->buffer,
                "</initial_update>\n");
        rc = GKrellM_SendStatement(session);
        if (rc != 0)
        {
            return -1;
        }
    }

    return 0;
}

/**
 * @brief   Start timers that will periodically transfer updates to GKrellM client.
 *
 * @param[in]   session     Pointer to GKrellM session descriptor.
 *
 * @return  0 upon successful completion.
 * @return  Error code (negative value), in case of error with system resources.
 */
static int
GKrellM_StartSessionTimers(struct GKrellM_Session* session)
{
    struct sigevent     event;
    timer_t             timerId;
    struct itimerspec   timerSpec;
    struct itimerspec   timerSpecSaved;
    int                 rc;

    // Create timer resource for GKrellM seconds.
    //
    {
        memset(&event, 0, sizeof(event));

        event.sigev_notify = SIGEV_THREAD;
        event.sigev_value.sival_ptr = session;
        event.sigev_notify_attributes = NULL;
        event.sigev_notify_function = (void (*)(sigval_t)) GKrellM_SecondsBell;

        rc = timer_create(CLOCK_REALTIME, &event, &timerId);
        if (rc < 0)
        {
            ReportError("[GKrellM] Cannot create timer: errno=%d", errno);

            return -1;
        }

        session->secondsTimerId = timerId;
    }

    // Create timer resource for GKrellM updater.
    //
    {
        memset(&event, 0, sizeof(event));

        event.sigev_notify = SIGEV_THREAD;
        event.sigev_value.sival_ptr = session;
        event.sigev_notify_attributes = NULL;
        event.sigev_notify_function = (void (*)(sigval_t)) GKrellM_UpdaterBell;

        rc = timer_create(CLOCK_REALTIME, &event, &timerId);
        if (rc < 0)
        {
            ReportError("[GKrellM] Cannot create timer: errno=%d", errno);

            return -1;
        }

        session->updaterTimerId = timerId;
    }

    // Configure and fire the timer for GKrellM seconds.
    //
    {
        timerSpec.it_value.tv_sec = 1;
        timerSpec.it_value.tv_nsec = 0;

        timerSpec.it_interval.tv_sec = 1;
        timerSpec.it_interval.tv_nsec = 0;

        rc = timer_settime(session->secondsTimerId, 0, &timerSpec, &timerSpecSaved);
        if (rc < 0)
        {
            ReportError("[GKrellM] Cannot start timer: errno=%d", errno);

            return -1;
        }
    }

    // Configure and fire the timer for GKrellM updater.
    //
    {
        timerSpec.it_value.tv_sec = 0;
        timerSpec.it_value.tv_nsec =
                GKRELLM_UPDATE_INTERVAL * NANOSECONDS_PER_MILLISECOND;

        timerSpec.it_interval.tv_sec = 0;
        timerSpec.it_interval.tv_nsec =
                GKRELLM_UPDATE_INTERVAL * NANOSECONDS_PER_MILLISECOND;

        rc = timer_settime(session->updaterTimerId, 0, &timerSpec, &timerSpecSaved);
        if (rc < 0)
        {
            ReportError("[GKrellM] Cannot start timer: errno=%d", errno);

            return -1;
        }
    }

    return 0;
}

/**
 * @brief   Stop all GKrellM session timers.
 *
 * @param[in]   session     Pointer to GKrellM session descriptor.
 *
 * @return  0 upon successful completion.
 * @return  Error code (negative value), in case of error with system resources.
 */
static int
GKrellM_StopSessionTimers(struct GKrellM_Session* session)
{
    struct itimerspec   timerSpec;
    struct itimerspec   timerSpecSaved;
    int                 rc;

    timerSpec.it_value.tv_sec = 0;
    timerSpec.it_value.tv_nsec = 0;

    timerSpec.it_interval.tv_sec = 0;
    timerSpec.it_interval.tv_nsec = 0;

    // Stop timer for GKrellM seconds.
    //
    if (session->secondsTimerId != NULL)
    {
        rc = timer_settime(session->secondsTimerId, 0, &timerSpec, &timerSpecSaved);
        if (rc < 0)
        {
            ReportError("[GKrellM] Cannot stop timer: errno=%d", errno);

            return -1;
        }

        rc = timer_delete(session->secondsTimerId);
        if (rc < 0)
        {
            ReportError("[GKrellM] Cannot delete timer: errno=%d", errno);

            return -1;
        }
    }

    // Stop timer for GKrellM updater.
    //
    if (session->updaterTimerId != NULL)
    {
        rc = timer_settime(session->updaterTimerId, 0, &timerSpec, &timerSpecSaved);
        if (rc < 0)
        {
            ReportError("[GKrellM] Cannot stop timer: errno=%d", errno);

            return -1;
        }

        rc = timer_delete(session->updaterTimerId);
        if (rc < 0)
        {
            ReportError("[GKrellM] Cannot delete timer: errno=%d", errno);

            return -1;
        }
    }

    return 0;
}

/**
 * @brief   Send statement to GKrellM client.
 *
 * The statement, that is to be sent, must be prepared in a buffer of GKrellM session descriptor
 * in as ASCII string terminated with CR.
 *
 * @param[in]   session     Pointer to GKrellM session descriptor.
 *
 * @return  0 upon successful completion.
 * @return  Error code (negative value), in case of network I/O error.
 */
static int
GKrellM_SendStatement(struct GKrellM_Session* session)
{
    size_t  bytesToSend;
    ssize_t bytesSent;

    bytesToSend = strlen(session->buffer);

    bytesSent = send(session->sessionSockFD, session->buffer, bytesToSend, MSG_NOSIGNAL);
    if (bytesSent == -1)
    {
        ReportError("[GKrellM] " \
                "Network I/O error has occurred in GKrellM session: errno=%d",
                errno);

        return -1;
    }
    else if ((unsigned int) bytesSent != bytesToSend)
    {
        ReportError("[GKrellM] Cannot send complete statement");

        return -1;
    }

    return 0;
}

static void
GKrellM_SecondsBell(void* arg)
{
    struct GKrellM_Session* session;
    time_t                  now;
    struct tm*              tm;
    int                     rc;

    Workspace::Kernel& kernel = Workspace::Kernel::SharedInstance();

    session = (struct GKrellM_Session*) arg;

    rc = pthread_mutex_trylock(&session->transmission.mutex);
    if (rc != 0)
    {
        // Quit if another instance is already/still running.
        //
        if (rc == EBUSY)
        {
            return;
        }

        ReportSoftAlert("[GKrellM] Error on mutex trylock: rc=%d", rc);

        GKrellM_CancelSession(session);

        return;
    }

    if (session->secondsCounter < 60)
    {
        sprintf(session->buffer,
                "<.%u>\n",
                session->secondsCounter);
        rc = GKrellM_SendStatement(session);
        if (rc != 0)
        {
            goto quit;
        }

        session->secondsCounter++;
    }
    else
    {
        // System date and time.
        //
        {
            strcpy(session->buffer,
                    "<time>\n");
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return;
            }

            time(&now);
            tm = localtime(&now);

            sprintf(session->buffer,
                    "%d %d %d %d %d %d %d %d %d\n",
                   tm->tm_sec,
                   tm->tm_min,
                   tm->tm_hour,
                   tm->tm_mday,
                   tm->tm_mon,
                   tm->tm_year,
                   tm->tm_wday,
                   tm->tm_yday,
                   tm->tm_isdst);
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return;
            }
        }

        // Uptime.
        //
        {
            strcpy(session->buffer,
                    "<uptime>\n");
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return;
            }

            sprintf(session->buffer,
                    "%lu\n",
                    (Toolkit::TimestampSeconds() - kernel.timestampOfStart) / SECOND_PER_MINUTE);
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return;
            }
        }
    }

quit:
    // Cancel session if something went wrong.
    //
    if (rc != 0)
    {
        pthread_mutex_unlock(&session->transmission.mutex);

        GKrellM_CancelSession(session);

        return;
    }

    rc = pthread_mutex_unlock(&session->transmission.mutex);
    if (rc != 0)
    {
        ReportSoftAlert("[GKrellM] Error on mutex unlock: rc=%d", rc);

        GKrellM_CancelSession(session);
    }
}

static void
GKrellM_UpdaterBell(void* arg)
{
    struct GKrellM_Session* session;
    int                     rc;

    session = (struct GKrellM_Session*) arg;

    Therma::Service& thermaService = Therma::Service::SharedInstance();

    rc = pthread_mutex_trylock(&session->transmission.mutex);
    if (rc != 0)
    {
        // Quit if another instance is already/still running.
        //
        if (rc == EBUSY)
        {
            return;
        }

        ReportSoftAlert("[GKrellM] Error on mutex trylock: rc=%d", rc);

        GKrellM_CancelSession(session);

        return;
    }

    // Sensors.
    //
    if ((session->datagramSequenceNumber % GKRELLM_FREQUENCY_FOR_SENSORS) == 0)
    {
        strcpy(session->buffer,
                "<sensors>\n");
        rc = GKrellM_SendStatement(session);
        if (rc != 0)
        {
            return;
        }

        for (unsigned int sensorIndex = 0;
             sensorIndex < thermaService.size();
             sensorIndex++)
        {
            Therma::Sensor* sensor = thermaService[sensorIndex];

            sprintf(session->buffer,
                    "%u \"%s\" 0 0 0 %.3f\n",
                    GKRELLM_SENSOR_TEMPERATURE,
                    sensor->deviceId,
                    sensor->temperature.current);
            rc = GKrellM_SendStatement(session);
            if (rc != 0)
            {
                return;
            }
        }
    }

    // Cancel session if something went wrong.
    //
    if (rc != 0)
    {
        pthread_mutex_unlock(&session->transmission.mutex);

        GKrellM_CancelSession(session);

        return;
    }

    session->datagramSequenceNumber++;

    rc = pthread_mutex_unlock(&session->transmission.mutex);
    if (rc != 0)
    {
        ReportSoftAlert("[GKrellM] Error on mutex unlock: rc=%d", rc);

        GKrellM_CancelSession(session);
    }
}
