/*
 * ushort_buffer.h
 *
 *  Created on: 29.11.2016
 *      Author: leichelberger
 */

#ifndef USHORT_BUFFER_H_
#define USHORT_BUFFER_H_


#include <stdint.h>


typedef struct{
    uint16_t * const buffer;
    uint8_t wrIdx;
    uint8_t rdIdx;
    uint8_t msgLen;
    const uint8_t maxLen;
}usBuf;


/* Sets the indexes and the msg length to 0 */
extern void vUSBufClear( usBuf * );


/* Fills the character at buffer[read index] into [2. arg] pointer and
 * increments read index afterwards.
 *
 * Returns:      0  success
 *              -1  msg already completely read */
extern int8_t cUSBufGetEntry( usBuf *, uint16_t * );


/* Writes [2. arg] into buffer[write index] and increments write index afterwards.
 * Msg length is only increased if [2. arg] is written to the very end of the buffer.
 *
 * Returns:      0  success
 *              -1  buffer full */
extern int8_t cUSBufPutEntry( usBuf *, uint16_t );


/* Sets read index to [3. arg] and fills the character at buffer[read index]
 * into [2. arg] pointer. The read index is increased afterwards.
 *
 * Returns:      0  success
 *              -1  out of buffer range
 *              -2  out of msg range    */
extern int8_t cUSBufGetEntryAt( usBuf *, uint16_t *, uint8_t );


/* Sets write index to [3. arg] and writes [2. arg] at buffer[write idx].
 * Note that the msg length is not adjusted and does also not increase with
 * subsequent cBufPutChar() calls until the write index gets equal to msg length.
 * Note further that writing to a position greater than msg length does
 * lead to msg length = write index.
 *
 * Returns:      0  success
 *              -1  out of buffer range  */
extern int8_t cUSBufPutEntryAt( usBuf *, uint16_t, uint8_t );

/*
 *
 */
extern int8_t cUSBufPutEntryUnique( usBuf *, uint16_t );

/*
 *
 */
extern int8_t cUSBufPopEntry( usBuf *, uint16_t * );

/* Returns the length of the stored msg.
*   - By entering a value greater than msg length into $to, e.g. the buffer length, you get the length till the msg end.
*   - By entering a negativ value into $to, you enter the offset from the end of the msg.
*   - By entering $from equal to $to, the length from the current read position is returned, e.g. (-2,-2).
*
*   Returns:    length value    */
extern int8_t cUSBufGetLen( usBuf *, int8_t, int8_t );

/*
 *
 */
extern int8_t cUSBufCopy( usBuf *, usBuf * );

/*
 *
 */
extern int8_t cUSBufSetRdPtr( usBuf *, int8_t );

/*
 *
 */
extern int8_t cUSBufSetWrPtr( usBuf *, int8_t );


#endif /* USHORT_BUFFER_H_ */
