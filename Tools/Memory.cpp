// System definition files.
//
#include <sys/types.h>

// Local definition files.
//
#include "Servus/Tools/Memory.h"

#ifdef INTEL

/**
 * @brief   Copy one byte virtual memory to virtual memory.
 *
 * This function is using explicitly the low-level PU instruction
 * to omit any kind of compile optimization and though to guarantee DMA compatibility.
 *
 * @param[in]   destination Address of a destination.
 * @param[in]   source      Address of source data.
 */
inline void
CopyByte(void *destination, const void *source)
{
    __asm__ volatile("movsb" : : "S" (source), "D" (destination));
}

/**
 * @brief   Copy one double-word (4 bytes) virtual memory to virtual memory.
 *
 * This function is using explicitly the low-level PU instruction
 * to omit any kind of compile optimization and though to guarantee DMA compatibility.
 *
 * @param[in]   destination Address of a destination.
 * @param[in]   source      Address of source data.
 */
inline void
CopyDoubleword(void *destination, const void *source)
{
    __asm__ volatile("movsl" : : "S" (source), "D" (destination));
}

/**
 * @brief   Copy one quad-word (8 bytes) virtual memory to virtual memory.
 *
 * This function is using explicitly the low-level PU instruction
 * to omit any kind of compile optimization and though to guarantee DMA compatibility.
 *
 * @param[in]   destination Address of a destination.
 * @param[in]   source      Address of source data.
 */
inline void
CopyQuadword(void *destination, const void *source)
{
    __asm__ volatile("movsq" : : "S" (source), "D" (destination));
}

/**
 * @brief   Copy a sequence of double-word values.
 *
 * This function is using explicitly the low-level PU instruction
 * to omit any kind of compile optimization and though to guarantee DMA compatibility.
 * Copying is done on double-word boundary. Caller is responcible to provide
 * both source and destination addresses be double-word aligned.
 *
 * @param[in]   destination Address of a destination.
 * @param[in]   source      Address of source data.
 * @param[in]   length      Length of data area in bytes.
 */
inline void
CopyDMA(void *destination, const void *source, unsigned int length)
{
    unsigned int numberOfDoubleWords;

    numberOfDoubleWords = length >> 2;

    __asm__ volatile("cld\n" "rep movsl" : : "S" (source), "D" (destination), "c" (numberOfDoubleWords));
}

/**
 * @brief   Copy a sequence of quad-word values.
 *
 * This function is using explicitly the low-level PU instruction
 * to omit any kind of compile optimization and though to guarantee DMA compatibility.
 * Copying is done on quad-word boundary. Caller is responcible to provide
 * both source and destination addresses be quad-word aligned.
 *
 * @param[in]   destination Address of a destination.
 * @param[in]   source      Address of source data.
 * @param[in]   length      Length of data area in bytes.
 */
inline void
CopyDMA64(void *destination, const void *source, unsigned int length)
{
    unsigned int numberOfQuadWords;

    numberOfQuadWords = length >> 3;

    __asm__ volatile("cld\n" "rep movsq" : : "S" (source), "D" (destination), "c" (numberOfQuadWords));
}

#endif // INTEL

/**
 * @brief   Justify specified value up to a specified boundary.
 *
 * @param[in]   value       Original value.
 * @param[in]   boundary    Boundary (for example 1024 to justify value on a KB boundary).
 *
 * @return  Value justified to specified boundary.
 */
inline unsigned int
JustifyToBoundaryUp(const unsigned int value, const unsigned int boundary)
{
    unsigned int rest;

    rest = value % boundary;

    if (rest == 0)
    {
        return value;
    }
    else
    {
        return value + (boundary - rest);
    }
}

/**
 * @brief   Justify specified value down to a specified boundary.
 *
 * @param[in]   value       Original value.
 * @param[in]   boundary    Boundary (for example 1024 to justify value on a KB boundary).
 *
 * @return  Value justified to specified boundary.
 */
inline unsigned int
JustifyToBoundaryDown(const unsigned int value, const unsigned int boundary)
{
    unsigned int rest;

    rest = value % boundary;

    return value - rest;
}
