#pragma once

// System definition files.
//
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/uio.h>

#define GKRELLM_TCP_PORT                    19200u
#define GKRELLM_WAIT_IF_CANNOT_ESTABLISH_SOCKET 500u

/**
 * Interval between updates in GKrellM session.
 */
#define GKRELLM_UPDATE_INTERVAL             250u    /**< Milliseconds. */

/**
 * Amount of time to wait for any input from GKrellM client before session
 * is mentioned as timed out.
 */
#define GKRELLM_SESSION_TIMEOUT             5000u   /**< Milliseconds. */

/**
 * Size of buffer to be used for network I/O in GKrellM session.
 */
#define GKRELLM_BUFFER_SIZE                 256u

/**
 * Specify how often reports should be sent.
 *   Frequency = 0 is not allowed.
 *   Frequency = 1 means produce report per each packet.
 *   Frequency = x means produce report once for each x packet.
 *
 * Recommended to keep the mentioned frequencies by greater than 1,
 * which will decrease amount of data sent over network.
 */
#define GKRELLM_FREQUENCY_FOR_SENSORS       4u      /**< Once per 4 packets. */
#define GKRELLM_FREQUENCY_FOR_FILESYSTEMS   4u      /**< Once per 4 packets. */

/**
 * Known sensor types.
 */
#define GKRELLM_SENSOR_TEMPERATURE          0u
#define GKRELLM_SENSOR_FAN                  1u
#define GKRELLM_SENSOR_POWER                2u

/**
 * Default GKrellM signature. Servus implementation of GKrellM does not require version number.
 */
#define GKRELLM_SIGNATURE                   "gkrellm"

/**
 * GKrellM service descriptor.
 */
struct GKrellM_Service
{
    /**
     * Thread descriptor of GKrellM listener thread.
     */
    pthread_t               thread;

    /**
     * Socket address record describing local network socket.
     */
    struct sockaddr_in      serverAddress;

    /**
     * Socket descriptor to listen for incoming connections.
     */
    int                     listeningSockFD;
};

/**
 * GKrellM session descriptor.
 */
struct GKrellM_Session
{
    /**
     * Lock mechanism to prevent several info updater from simultaneous network I/O.
     */
    struct
    {
        pthread_mutexattr_t mutexAttr;
        pthread_mutex_t     mutex;
    }
    transmission;

    /**
     * Timer of seconds hand.
     */
    timer_t                 secondsTimerId;

    /**
     * Timer of info updater.
     */
    timer_t                 updaterTimerId;

    /**
     * Socket descriptor opened GKrellM session.
     */
    int                     sessionSockFD;

    /**
     * Seconds need to be counted because GKrellM protocol expects one special datagram
     * to be sent once per second, those purpose is to keep internal clock
     * of GKrellM client session be synchronized with the clock of a server.
     */
    unsigned int            secondsCounter;

    /**
     * All datagrams, which are sent not on seconds timer, are counted sequentially.
     */
    unsigned int            datagramSequenceNumber;

    struct
    {
        unsigned int        mtn;
        unsigned int        mtdsRecords;
        unsigned int        mtdsPayloads;
        unsigned int        http;
        unsigned int        rtsp;
        unsigned int        tftp;
    }
    numberOfBuffersInUse;

    char                    buffer[GKRELLM_BUFFER_SIZE];
};

/**
 * @brief   Prepare resources of GKrellM server and start it as a thread.
 *
 * @param[in]   service     Pointer to GKrellM service descriptor.
 *
 * @return  0 upon successful completion.
 * @return  Error code (negative value), in case of error.
 */
int
GKrellM_Start(struct GKrellM_Service *service);
