#pragma once

#include <stdint.h>
#include <stddef.h>

constexpr size_t RING_BUFFER_MAX = 1024; 

struct ring_buffer {
	size_t head;
	size_t tail;
	size_t len;
	size_t entry_size;
	uint8_t data[RING_BUFFER_MAX];
};

#define make_ring_buffer(type) (struct ring_buffer){.entry_size = sizeof(type)}

size_t ring_buffer_remaining(struct ring_buffer* q);

bool ring_buffer_push(struct ring_buffer* q, const void* data);

bool ring_buffer_get(struct ring_buffer* q, void* out);
