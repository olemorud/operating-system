#pragma once

#include <stddef.h>

void* kalloc(size_t size);

void kfree(void* ptr);

void* krealloc(void* ptr, size_t size);
