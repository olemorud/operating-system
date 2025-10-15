#pragma once

#include "bitmap.h"
#include "str.h"

constexpr int FILE_NAME_MAX    = 31;
constexpr int FILE_MAP_SIZE    = 128;
constexpr int MAX_FILE_BLOCK_SIZE = 4096;

enum file_attr : uint32_t {
    FILE_PRESENT = 1<<0,
};

struct file_node {
    struct file_node* next;
    uint32_t          block_offset;
};


struct file_path {
    uint32_t attr;
    uint32_t size;

    uint32_t name_len;
    char name[FILE_NAME_MAX+1];

    struct file_node* node;
};

/* TODO: clean up this mess:*/
struct file_system {
    /* map of paths */
    struct file_path_map {
        struct file_path entries[FILE_MAP_SIZE];
        uint32_t         count;
    } file_map;
    
    /* map of free blocks */
    struct bitmap free_blocks_bitmap;

    int block_size;  
    int block_count;  

    /* virtual methods */
    int (*file_allocate_block)(struct file_system*);
    bool (*file_write_block)(struct file_system*, uint32_t, const uint8_t *data);
    bool (*file_read_block)(struct file_system*, uint32_t, uint8_t *output);
};


int test_file_system();

struct file_path* file_open(struct file_system* fs, struct str path);

bool file_write(struct file_system* fs, struct file_path* f, const uint8_t* data, size_t n);

bool file_read(struct file_system* fs, struct file_path* f, uint8_t* out, int n);


extern struct file_system g_ramdisk;
