#pragma once

/**
 * @version 170201
 *
 * @brief   Low-level tools.
 */

// System definition files.
//
#include <sys/types.h>

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
void
CopyByte(void* destination, const void* source);

/**
 * @brief   Copy one double-word (4 bytes) virtual memory to virtual memory.
 *
 * This function is using explicitly the low-level PU instruction
 * to omit any kind of compile optimization and though to guarantee DMA compatibility.
 *
 * @param[in]   destination Address of a destination.
 * @param[in]   source      Address of source data.
 */
void
CopyDoubleword(void* destination, const void* source);

/**
 * @brief   Copy one quad-word (8 bytes) virtual memory to virtual memory.
 *
 * This function is using explicitly the low-level PU instruction
 * to omit any kind of compile optimization and though to guarantee DMA compatibility.
 *
 * @param[in]   destination Address of a destination.
 * @param[in]   source      Address of source data.
 */
void
CopyQuadword(void* destination, const void* source);

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
void
CopyDMA(void* destination, const void* source, unsigned int length);

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
void
CopyDMA64(void *destination, const void *source, unsigned int length);

#endif // INTEL

/**
 * @brief   Justify specified value up to a specified boundary.
 *
 * @param[in]   value       Original value.
 * @param[in]   boundary    Boundary (for example 1024 to justify value on a KB boundary).
 *
 * @return  Value justified to specified boundary.
 */
unsigned int
JustifyToBoundaryUp(const unsigned int value, const unsigned int boundary);

/**
 * @brief   Justify specified value down to a specified boundary.
 *
 * @param[in]   value       Original value.
 * @param[in]   boundary    Boundary (for example 1024 to justify value on a KB boundary).
 *
 * @return  Value justified to specified boundary.
 */
unsigned int
JustifyToBoundaryDown(const unsigned int value, const unsigned int boundary);
