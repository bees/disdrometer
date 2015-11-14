//Modified version of pedvides ringbuffer. His license is reproduced in full below. 
//Modifications:
//  - Allow reading without changing the start index
//

/* Teensy 3.x, LC ADC library
 * https://github.com/pedvide/ADC
 * Copyright (c) 2015 Pedro Villanueva
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef CIRCBUFFER_H
#define CIRCBUFFER_H

// include new and delete
//#include <Arduino.h>

// THE SIZE MUST BE A POWER OF 2!!
#define CIRC_BUFFER_DEFAULT_BUFFER_SIZE 8



/** Class RingBuffer implements a circular buffer of fixed size (must be power of 2)
*   Code adapted from http://en.wikipedia.org/wiki/Circular_buffer#Mirroring
*/
class CircBuffer
{
    public:
        //! Default constructor, buffer has a size DEFAULT_BUFFER_SIZE
        CircBuffer();

        // Default constructor, buffer has a size DEFAULT_BUFFER_SIZE
        //CircBuffer(int size);

        /** Default destructor */
        virtual ~CircBuffer();

        //! Returns 1 (true) if the buffer is full
        int isFull();

        //! Returns 1 (true) if the buffer is empty
        int isEmpty();

        //! Write a value into the buffer
        void write(int value);

        //! Read a value from the buffer, modifies start
        int read();

    protected:
    private:

        int increase(int p);

        int b_size = CIRC_BUFFER_DEFAULT_BUFFER_SIZE;
        int b_start = 0;
        int b_end = 0;
        int elems[CIRC_BUFFER_DEFAULT_BUFFER_SIZE];
};



#endif // CIRCBUFFER_H
