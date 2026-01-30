#include "ring_buffer.h"

#include <errno.h>
#include <stdint.h>
#include <string.h>

#ifndef min
#define min(a, b) ((a) < (b) ? (a):(b))
#endif

/**
 * ring_buffer_init - Initialize a generic ring buffer
 * @rb: pointer to ring buffer structure
 * @buf: pointer to backing storage buffer
 * @size: total buffer size in bytes
 * @esize: size of a single element in bytes
 *
 * This function initializes a ring buffer that stores fixed-size elements.
 * Internally, the buffer operates on element counts rather than raw bytes.
 *
 * The total number of elements is calculated as:
 *
 *     element_count = size / esize
 *
 * The resulting element count must be a power of two, which allows efficient
 * wrap-around using a bitmask instead of modulo operations.
 *
 * Head and tail indices are logical counters that monotonically increase and
 * are masked only when accessing the underlying buffer.
 *
 * Return: 0 on success, -EINVAL if the element count is invalid
 */
int ring_buffer_init(ring_t *rb, void *buf, uint32_t size, const uint32_t esize)
{
    size /= esize;

    /* check that size is a power of 2 */
    if ((size & (size - 1)) != 0)
        return -EINVAL;

    rb->buffer = buf;
    rb->size = size;
    rb->esize = esize;
    rb->head = rb->tail = 0;

    if (size < 2) {
        rb->mask = 0;
        return -EINVAL;
    }
    rb->mask = size - 1;

    return 0;
}

/**
 * ring_space - Get number of free elements in the ring buffer
 * @rb: pointer to ring buffer
 *
 * Return: number of elements available for writing
 *
 * The returned value is expressed in elements, not bytes.
 */
size_t ring_space(const ring_t *rb)
{
    /* head - tail gives number of used bytes, subtract from size */
    return rb->size - (rb->head - rb->tail);
}

/**
 * ring_used - Get number of used elements in the ring buffer
 * @rb: pointer to ring buffer
 *
 * Return: number of elements available for reading
 */
size_t ring_used(const ring_t *rb)
{
    return rb->head - rb->tail;
}

static void __ring_put(ring_t *rb, const void *src, uint32_t len, uint32_t offset)
{
    uint32_t size = rb->size;
    uint32_t esize = rb->esize;
    uint32_t l;

    offset &= rb->mask;
    if (esize != 1) {
        offset *= esize;
        size *= esize;
        len *= esize;
    }

    l = min(len, size - offset);

    memcpy(rb->buffer + offset, src, l);
    memcpy(rb->buffer, src + l, len - l);
}

/**
 * ring_put - Write elements into the ring buffer
 * @rb: pointer to ring buffer
 * @buf: pointer to source data
 * @len: number of elements to write
 *
 * Writes up to @len elements into the ring buffer, stopping early if the
 * buffer becomes full.
 *
 * Return: number of elements actually written
 */
uint32_t ring_put(ring_t *rb, const void *buf, uint32_t len)
{
    uint32_t l;

    l = ring_space(rb);
    len = min(len, l);

    __ring_put(rb, buf, len, rb->head);
    rb->head += len;

    return len;
}

static void __ring_get(ring_t *rb, void *dst, uint32_t len, uint32_t offset)
{
    uint32_t size = rb->size;
    uint32_t esize = rb->esize;
    uint32_t l;

    offset &= rb->mask;
    if (esize != 1) {
        offset *= esize;
        size *= esize;
        len *= esize;
    }

    l = min(len, size - offset);
    memcpy(dst, rb->buffer + offset, l);
    memcpy(dst + l, rb->buffer, len - l);
}

/**
 * ring_get - Read elements from the ring buffer
 * @rb: pointer to ring buffer
 * @buf: pointer to destination buffer
 * @len: number of elements to read
 *
 * Reads up to @len elements from the ring buffer, stopping early if the
 * buffer becomes empty.
 *
 * Return: number of elements actually read
 */
size_t ring_get(ring_t *rb, void *buf, uint32_t len)
{
   uint32_t l;

    l = ring_used(rb);
    len = min(l, len);

    __ring_get(rb, buf, len, rb->tail);
    rb->tail += len;
    return len;
}
