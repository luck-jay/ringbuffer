#ifndef RING_RING_BUFFER_H
#define RING_RING_BUFFER_H

#include <stdint.h>

struct ring {
    void *buffer;
    uint32_t size;
    uint32_t mask;
    uint32_t esize;
    uint32_t head;
    uint32_t tail;
};

typedef struct ring ring_t;
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
int ring_buffer_init(ring_t *rb, void *buf, uint32_t size, uint32_t esize);

/**
 * ring_space - Get number of free elements in the ring buffer
 * @rb: pointer to ring buffer
 *
 * Return: number of elements available for writing
 *
 * The returned value is expressed in elements, not bytes.
 */
size_t ring_space(const ring_t *rb);

/**
 * ring_used - Get number of used elements in the ring buffer
 * @rb: pointer to ring buffer
 *
 * Return: number of elements available for reading
 */
size_t ring_used(const ring_t *rb);

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
uint32_t ring_put(ring_t *rb, const void *src, uint32_t len);

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
size_t ring_get(ring_t *rb, void *buf, uint32_t len);

#endif //RING_RING_BUFFER_H
