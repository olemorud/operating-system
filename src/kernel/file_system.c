
#include "file_system.h"

#include <stdint.h>
#include <limits.h>
#include <stddef.h>

#include "malloc.h"
#include "libc.h"
#include "str.h"

#define MIN(a, b) ((a) > (b) ? (b) : (a))

/*
 * File node implementation
 * ========================
 * */
static void node_init(struct file_node* n, uint32_t block_offset)
{
    n->block_offset = block_offset;
    n->next = NULL;
} 

/**
 * [path -> file] mapping
 * ========
 */
static uint32_t djb2_hash(struct str key)
{
    /* credits: Daniel J. Bernstein */
    uint32_t hash = 5381;
    for (size_t i = 0; i < key.len; i++) {
        hash <<= 5;
        hash += (uint32_t)key.data[i];
    }
    return hash;
}

static struct file_path* file_map_at(struct file_path_map* file_map, struct str path)
{
    const uint32_t hash = djb2_hash(path) % FILE_MAP_SIZE;
    return &(file_map->entries[hash]);
}

static inline struct file_path* collision_alternative(struct file_path_map* file_map, struct file_path* f)
{
    /* i = (i + 1) % FILE_MAP_SIZE */
    return ((f - file_map->entries + 1) % FILE_MAP_SIZE) + file_map->entries;
}

struct file_path* file_map_get(struct file_path_map* file_map, struct str path)
{
    struct file_path* f = file_map_at(file_map, path);

    for (int i = 0; i < FILE_MAP_SIZE; i++) {
        if ((f->attr & FILE_PRESENT) == 0) {
            return NULL;
        }
        uint32_t min = MIN(path.len, f->name_len);
        if (memcmp(path.data, f->name, min) == 0) {
            return f;
        }
        f = collision_alternative(file_map, f);
    }

    return NULL;
}

struct file_path* file_map_insert(struct file_path_map* map, struct str path)
{
    if (map->count == FILE_MAP_SIZE) {
        return NULL;
    }
    struct file_path* f = file_map_at(map, path);

    for (int i = 0; i < FILE_MAP_SIZE; i++) {
        uint32_t min = MIN(path.len, f->name_len);
        if (memcmp(path.data, f->name, min) == 0) {
            return NULL /* file with name already exists */;
        }
        f = collision_alternative(map, f);
        if ((f->attr & FILE_PRESENT) == 0) {
            return f;
        }
    }
    return NULL;
}


/**
 * Ramdisk implementation
 * ===========
 */
constexpr int RAMDISK_BLOCK_SIZE  = 4096;
constexpr int RAMDISK_BLOCK_COUNT = 128;

uint8_t ramdisk_data[RAMDISK_BLOCK_SIZE * RAMDISK_BLOCK_COUNT];
uint32_t ramdisk_free_bitmap[RAMDISK_BLOCK_COUNT / (sizeof(uint32_t) * CHAR_BIT)];

/* returns block offset */
int ramdisk_allocate_block(struct file_system* fs)
{
    int free_pos = bitmap_find(&fs->free_blocks_bitmap, 0);
    if (free_pos == -1) {
        return -1;
    }
    if (bitmap_set(&fs->free_blocks_bitmap, free_pos)) {
        /* this means there's an error in the bitmap implementation */
        panic(str_attach("failed to set bitmap when allocating block"));
    }
    return free_pos * RAMDISK_BLOCK_SIZE;
}

bool ramdisk_write_block(struct file_system*, uint32_t block_offset, const uint8_t* buf)
{
    /* for now just use the memory as a file system */
    for (size_t i = 0; i < RAMDISK_BLOCK_SIZE; i++) {
        ramdisk_data[block_offset + i] = buf[i];
    }
    return true;
}

bool ramdisk_read_block(struct file_system*, uint32_t block_offset, uint8_t* output)
{
    for (size_t i = 0; i < RAMDISK_BLOCK_SIZE; i++) {
        output[i] = ramdisk_data[block_offset + i];
    }
    return true;
}

struct file_system g_ramdisk = {
    .free_blocks_bitmap = BITMAP_ATTACH(ramdisk_free_bitmap,
                                 sizeof(ramdisk_free_bitmap)),

    .block_size  = RAMDISK_BLOCK_SIZE,
    .block_count = RAMDISK_BLOCK_COUNT,

    .file_write_block    = ramdisk_write_block,
    .file_read_block     = ramdisk_read_block,
    .file_allocate_block = ramdisk_allocate_block,
};


/**
 * Higher level interfaces
 * =======================
 */ 
struct file_path* file_create(struct file_system* fs, struct str path)
{
    struct file_path* f = file_map_insert(&fs->file_map, path);

    memset(f, 0, sizeof *f); 
    f->attr |= FILE_PRESENT;
    memcpy(f->name, path.data, MIN(path.len, FILE_NAME_MAX));

    return f;
}

struct file_path* file_open(struct file_system* fs, struct str path)
{
    return file_map_get(&fs->file_map, path);
}

bool file_write(struct file_system* fs, struct file_path* f, const uint8_t* data, size_t n)
{
    struct file_node** node = &f->node;
    size_t written = 0;

    while (written < n) {
        if (*node == NULL) {
            struct file_node* new_node = kalloc(sizeof *new_node);
            if (new_node == NULL) {
                panic(str_attach("failed to allocate new node for file"));
            }
            int offset = fs->file_allocate_block(fs);
            if (offset == -1) {
                panic(str_attach("failed to allocate new block in disk"));
            }
            node_init(new_node, offset);
            *node = new_node;
        }

        bool ok = fs->file_write_block(fs, (*node)->block_offset, &data[written]);
        if (!ok) {
            panic(str_attach("file_write_byte failed"));
        }
        written += fs->block_size;

        node = &((*node)->next);
    }

    return true;
}

bool file_read(struct file_system* fs, struct file_path* f, uint8_t* out, int n)
{
    uint8_t buf[MAX_FILE_BLOCK_SIZE];
    static int32_t last_read_block = -1;

    struct file_node dummy = {.next = f->node};
    struct file_node* node = &dummy;
    int read = 0;

    while (read < n) {
        if (!node->next) {
            panic(str_attach("no next block found when reading"));
        }
        node = node->next;

        bool ok;
        int remaining = n - read;
        if (remaining > fs->block_size) {
            ok = fs->file_read_block(fs, node->block_offset, &out[read]);
            if (!ok) {
                panic(str_attach("file_read_block failed"));
            }
            read += fs->block_size;
        } else {
            ok = fs->file_read_block(fs, node->block_offset, buf);
            if (!ok) {
                panic(str_attach("file_read_block failed"));
            }
            memcpy(&out[read], buf, remaining);
            read += n - read;
        }
    }
    return true;
}

/* TODO: remove this hacky test technique! */

int test_file_system()
{
    struct file_path* f = file_create(&g_ramdisk, str_attach("test_file"));

    const uint8_t data[] = "hello world!!!!\n";
    printf(str_attach("trying to write to file {cstr}"), f->name);
    bool ok = file_write(&g_ramdisk, f, data, sizeof data);
    if (!ok) {
        printf(str_attach("\nfailed to write to file {cstr}\n"), f->name);
        return -1;
    }
    printf(str_attach(" - OK!\n"));

    printf(str_attach("trying to read from file {cstr}"), f->name);
    uint8_t read_buf[sizeof data + 1];
    read_buf[sizeof data] = '\0';
    ok = file_read(&g_ramdisk, f, read_buf, sizeof data);
    if (!ok) {
        printf(str_attach("\nfailed to read from file {cstr}\n"), f->name);
        return -1;
    }

    if (memcmp(read_buf, data, sizeof data) != 0) {
        printf(str_attach("data mismatch between write and read\n"));
        return -1;
    }
    printf(str_attach(" - OK!, output was:\n{cstr}\n"), read_buf);

    printf(str_attach("attempting to get a file that doesn't exist"));
    {
        struct file_path* f = file_map_get(&g_ramdisk.file_map, str_attach("asd"));
        if (f) {
            printf(str_attach("\ngot file when no such file should exist\n"));
            return -1;
        }
    }
    printf(str_attach(" - OK!\n"));

    uint8_t big_data[MAX_FILE_BLOCK_SIZE * 2 + 3] = {0};
    memset(big_data, 0xAB, sizeof big_data);
    f = file_create(&g_ramdisk, str_attach("test_big_file"));
    printf(str_attach("attempting to write more than a block to {cstr}"), f->name);
    ok = file_write(&g_ramdisk, f, big_data, sizeof big_data);
    if (!ok) {
        printf(str_attach("\nfailed to write data larger than a block\n"));
        return -1;
    }
    printf(str_attach(" - OK!\n"));

    printf(str_attach("trying to read from file {cstr}"), f->name);
    uint8_t big_read_buf[sizeof big_data];
    ok = file_read(&g_ramdisk, f, big_read_buf, sizeof big_read_buf);
    if (!ok) {
        printf(str_attach("\nfailed to read from file {cstr}\n"), f->name);
        return -1;
    }
    printf(str_attach(" - OK!\n"));

    return 0;
}
