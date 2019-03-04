/*
 * ushort_buffer.c
 *
 *  Created on: 29.11.2016
 *      Author: leichelberger
 */


#include <stdint.h>
#include "ushort_buffer.h"


/**************************************************************
* Subroutines (export)
**************************************************************/

/*
 * Sets the indexes and the msg length to 0.
 */
void vUSBufClear( usBuf *c )
{
    c->wrIdx = 0;
    c->rdIdx = 0;
    c->msgLen = 0;
}
/*-----------------------------------------------------------*/

/*
 * Fills the character at buffer[idx] into char pointer and increments read idx.
 *
 * Returns:      0  success
 *              -1  msg already completely read
 */
int8_t cUSBufGetEntry( usBuf *c, uint16_t *data )
{
    if( c->rdIdx >= c->msgLen )
    {
        return -1;
    }
    else
    {
        *data = c->buffer[ c->rdIdx ];
        c->rdIdx++;
        return 0;
    }
}
/*-----------------------------------------------------------*/

/*
 * Fills data into the buffer and increments write idx afterwards.
 * Msg length is only increased if data is written to the very end of the buffer.
 *
 * Returns:      0  success
 *              -1  buffer full
 */
int8_t cUSBufPutEntry( usBuf *c, uint16_t data )
{
    if( c->msgLen + 1 == c->maxLen )
    {
        return -1;
    }
    else
    {
        c->buffer[ c->wrIdx ] = data;
        c->wrIdx++;
        if( c->wrIdx > c->msgLen )
        {
            c->msgLen = c->wrIdx;
        }
        return 0;
    }
}
/*-----------------------------------------------------------*/

/*
 * Sets read index to [3. arg] and fills the character at buffer[read index]
 * into [2. arg] pointer. The read index is increased afterwards.
 *
 * Returns:      0  success
 *              -1  out of buffer range
 *              -2  out of msg range
 */
int8_t cUSBufGetEntryAt( usBuf *c, uint16_t *data, uint8_t pos )
{
    if( pos >= c->maxLen )
    {
        return -1;
    }
    if( pos >= c->msgLen )
    {
        return -2;
    }
    else
    {
        c->rdIdx = pos;
        *data = c->buffer[ c->rdIdx ];
        c->rdIdx++;
        return 0;
    }
}
/*-----------------------------------------------------------*/

/*
 * Sets write index to [3. arg] and writes [2. arg] at buffer[write idx].
 * Note that the msg length is not adjusted and does also not increase with
 * subsequent cBufPutEntry() calls until the write index gets equal to msg length.
 * Note further that writing to a position greater than msg length does
 * lead to msg length = write index.
 *
 * Returns:      0  success
 *              -1  out of buffer range
 */
int8_t cUSBufPutEntryAt( usBuf *c, uint16_t data, uint8_t pos )
{
    if( pos >= c->maxLen )
    {
        return -1;
    }
    else
    {
        c->wrIdx = pos;
        c->buffer[ c->wrIdx ] = data;
        c->wrIdx++;
        if( c->wrIdx > c->msgLen )
        {
            c->msgLen = c->wrIdx;
        }
        return 0;
    }
}
/*-----------------------------------------------------------*/

/*
 * Returns:      1  value does already exist
 *               0  success
 *              -1  buffer full
 */
int8_t cUSBufPutEntryUnique( usBuf *c, uint16_t data )
{
uint8_t i = 0;
int8_t err = 0;

    for( i = 0; i < c->msgLen; i++ )
    {
        if( c->buffer[ i ] == data )
        {
            err = 1;
            break;
        }
    }
    if( !err )
    {
        err = cUSBufPutEntry( c, data );
    }
    return err;
}
/*-----------------------------------------------------------*/

/*
 * Removes an entry from the current end of the buffer and returns it.
 */
int8_t cUSBufPopEntry( usBuf *c, uint16_t *data )
{
int8_t err = 0;

    if( c->msgLen )
    {
        c->msgLen--;
        *data = c->buffer[ c->msgLen ];
        if( c->wrIdx > c->msgLen )
        {
            c->wrIdx = c->msgLen;
        }
    }
    else
    {
        err = -1;
    }
    return err;
}
/*-----------------------------------------------------------*/

/*
 * Returns the length of the stored msg.
 *  - By entering a value greater than msg length into $to, e.g. the buffer length,
 *      you get the length till the msg end.
 *  - By entering a negativ value into $to, the offset from the end of the msg is given.
 *  - By entering $from equal to $to, the length from the current read position
 *      to the $to value is returned, e.g. (-2,-2).
 *
 *  Returns:    length value
 */
int8_t cUSBufGetLen( usBuf *c, int8_t from, int8_t to )
{
    if( from == to )
    {
        from = c->rdIdx;
    }
    if( from > c->msgLen )
    {
        from = c->msgLen;
    }
    if( to > c->msgLen )
    {
        to = c->msgLen;
    }
    if( from < 0 )
    {
        from = c->msgLen - from;
    }
    if( to < 0 )
    {
        to = c->msgLen - to;
    }
    return ( to - from );
}
/*-----------------------------------------------------------*/

/*
 * copies source into destination if buffer is not to small.
 * Arguments:   destination pointer to destination struct.
 *              source      pointer to source struct.
 * Returns:      0  success
 *              -1  destination buffer to small
 */
int8_t cUSBufCopy( usBuf *destination, usBuf *source )
{
uint8_t i = 0;

    if( destination->maxLen > source->msgLen )
    {
        destination->wrIdx = source->wrIdx;
        destination->rdIdx = source->rdIdx;
        destination->msgLen = source->msgLen;
        for( i = 0; i <= source->msgLen; i++ )
        {
            destination->buffer[ i ] = source->buffer[ i ];
        }
    }
    else
    {
        return -1;
    }
    return 0;
}
/*-----------------------------------------------------------*/

/*
 * Sets the read pointer
 * Arguments:   c   the buffer struct
 *              pos position to set the pointer to, can be either
 *                  negative or positive. In case of negative pos,
 *                  the offset from the message end is supplied, so
 *                  that pos = -1 sets the pointer to the message end.
 * Returns:      0  success
 *              -1  desired position out of range
 */
int8_t cUSBufSetRdPtr( usBuf *c, int8_t pos )
{
    /* positive index */
    if( pos >= 0 )
    {
        if( pos < c->maxLen )
        {
            if ( pos < c->msgLen )
            {
                c->rdIdx = pos;
            }
            else
            {
                c->rdIdx = pos;
                return -2;
            }
        }
        else
        {
            return -1;
        }
    }
    /* negative index */
    else
    {
        if( ( c->msgLen + pos ) >= 0 )
        {
            c->rdIdx = c->msgLen + pos;
        }
        else
        {
            return -1;
        }
    }
    return 0;
}
/*-----------------------------------------------------------*/

/*
 * Sets the write pointer
 * Arguments:   c   the buffer struct
 *              pos position to set the pointer to, can be either
 *                  negative or positive.  In case of negative pos,
 *                  the offset from the message end is supplied,
 *                  so that pos = -1 sets the pointer to message length,
 *                  what means extending the buffer with the next put,
 *                  without overwritting anything.
 * Returns:      0  success
 *              -1  desired position out of range
 */
int8_t cUSBufSetWrPtr( usBuf *c, int8_t pos )
{
    /* positive index */
    if( pos >= 0 )
    {
        if( pos < c->maxLen )
        {
            c->wrIdx = pos;
        }
        else
        {
            return -1;
        }
    }
    /* negative index */
    else
    {
        if( ( c->msgLen + pos + 1 ) >= 0 )
        {
            c->rdIdx = c->msgLen + pos + 1;
        }
        else
        {
            return -1;
        }
    }
    return 0;
}
/*-----------------------------------------------------------*/
