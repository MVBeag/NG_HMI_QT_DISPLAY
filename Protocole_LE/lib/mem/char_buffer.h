/*
 * buffer.h
 *
 *  Created on: 26.07.2016
 *      Author: leichelberger
 */

#ifndef BUFFER_H_
#define BUFFER_H_


#include <stdint.h>


typedef struct{
    char * const buffer;
    uint8_t wrIdx;
    uint8_t rdIdx;
    uint8_t msgLen;
    const uint8_t maxLen;
}cBuf;


/* Sets the indexes and the msg length to 0 */
extern void vBufClear( cBuf * );


/* Fills the character at buffer[read index] into [2. arg] pointer and
 * increments read index afterwards.
 *
 * Returns:		 0	success
 * 				-1	msg already completely read */
extern int8_t cBufGetChar( cBuf *, char * );


/* Writes [2. arg] into buffer[write index] and increments write index afterwards.
 * Msg length is only increased if [2. arg] is written to the very end of the buffer.
 *
 * Returns:		 0	success
 * 				-1	buffer full */
extern int8_t cBufPutChar( cBuf *, char );


/* Sets read index to [3. arg] and fills the character at buffer[read index]
 * into [2. arg] pointer. The read index is increased afterwards.
 *
 * Returns:		 0	success
 * 				-1	out of buffer range
 * 				-2	out of msg range	*/
extern int8_t cBufGetCharAt( cBuf *, char *, uint8_t );


/* Sets write index to [3. arg] and writes [2. arg] at buffer[write idx].
 * Note that the msg length is not adjusted and does also not increase with
 * subsequent cBufPutChar() calls until the write index gets equal to msg length.
 * Note further that writing to a position greater than msg length does
 * lead to msg length = write index.
 *
 * Returns:		 0	success
 * 				-1	out of buffer range  */
extern int8_t cBufPutCharAt( cBuf *, char, uint8_t );

/* Returns the length of the stored msg.
* 	- By entering a value greater than msg length into $to, e.g. the buffer length, you get the length till the msg end.
* 	- By entering a negativ value into $to, you enter the offset from the end of the msg.
* 	- By entering $from equal to $to, the length from the current read position is returned, e.g. (-2,-2).
*
* 	Returns:	length value	*/
extern int8_t cBufGetLen( cBuf *, int8_t, int8_t );

/*
 *
 */
extern int8_t cBufCopy( cBuf *, cBuf * );

/*
 *
 */
extern int8_t cBufSetRdPtr( cBuf *, int8_t );

/*
 *
 */
extern int8_t cBufSetWrPtr( cBuf *, int8_t );


#endif /* BUFFER_H_ */
