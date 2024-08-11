
#include "ring_buffer.h"

size_t ring_buffer_remaining(struct ring_buffer* q)
{
	return (RING_BUFFER_MAX - q->len) / q->entry_size;
}

bool ring_buffer_push(struct ring_buffer* q, const void* data)
{
	if (q->len + q->entry_size > RING_BUFFER_MAX) {
		return false;
	}
	q->len += q->entry_size;
	for (size_t i = 0; i < q->entry_size; i++) {
		q->data[q->tail] = ((uint8_t*)data)[i];
		q->tail += 1;
		q->tail %= RING_BUFFER_MAX;
	}
	return true;
}

bool ring_buffer_get(struct ring_buffer* q, void* out)
{
	if (q->len == 0) {
		return false;
	}
	q->len -= q->entry_size;
	for (size_t i = 0; i < q->entry_size; i++) {
		((uint8_t*)out)[i] = q->data[q->head];
		q->head += 1;
		q->head %= RING_BUFFER_MAX;
	}
	return true;
}
