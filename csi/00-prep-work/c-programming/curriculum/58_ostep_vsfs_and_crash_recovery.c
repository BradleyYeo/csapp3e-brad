/*
 * Exercise 58: Persistence — VSFS Bitmaps, Inodes, Directories, Checksums & WAL Recovery (OSTEP Ch. 39–44 / xv6)
 *
 * Compile and run:
 *   make -C curriculum 58_ostep_vsfs_and_crash_recovery && ./curriculum/58_ostep_vsfs_and_crash_recovery
 */
#define _POSIX_C_SOURCE 200809L
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L
#include <stdbool.h>
#ifndef nullptr
#define nullptr NULL
#endif
#endif

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
  VSFS_BLOCK_SIZE = 64U,
  VSFS_NUM_BLOCKS = 32U,
  VSFS_NDIRECT = 4U,
  VSFS_NINDIRECT = 16U, /* 16 * sizeof(int32_t) == 64 bytes == VSFS_BLOCK_SIZE */
  WAL_MAGIC = 0x57414C31U
};

typedef struct {
  uint8_t data_bitmap[VSFS_NUM_BLOCKS / 8U];
  uint8_t blocks[VSFS_NUM_BLOCKS][VSFS_BLOCK_SIZE];
} VsfsDisk;

typedef struct {
  uint32_t size;
  int32_t direct[VSFS_NDIRECT]; /* -1 means unallocated */
  int32_t indirect;             /* -1 means unallocated */
} VsfsInode;

typedef struct {
  uint16_t inum; /* 0 means unused slot */
  char name[14];
} VsfsDirent;

typedef struct {
  uint32_t valid_magic;
  bool committed;
  uint32_t target_block;
  uint16_t expected_checksum;
  uint8_t payload[VSFS_BLOCK_SIZE];
} WalTxn;

/*
 * Drill 1: Allocation Bitmap Operations (OSTEP Ch. 40 `vsfs.py`)
 *
 * Requirements:
 * - bitmap_test: return false if bm == nullptr || bit_idx >= num_bits.
 *   Return true iff bit `(bm[bit_idx / 8] >> (bit_idx % 8)) & 1U` is set.
 * - bitmap_alloc_first: return -1 if bm == nullptr || num_bits == 0.
 *   Find lowest `i` in `0..num_bits` with bit `i` clear, set it to 1, and return `(int32_t)i`.
 *   Return -1 if all bits are already set.
 * - bitmap_free: return false if bm == nullptr || bit_idx >= num_bits or bit is already 0.
 *   Clear bit `bit_idx` to 0 and return true.
 */
static bool bitmap_test(const uint8_t *bm, size_t num_bits, size_t bit_idx) {
  (void)bm;
  (void)num_bits;
  (void)bit_idx;
  return false; /* TODO: implement */
}

static int32_t bitmap_alloc_first(uint8_t *bm, size_t num_bits) {
  (void)bm;
  (void)num_bits;
  return -1; /* TODO: implement */
}

static bool bitmap_free(uint8_t *bm, size_t num_bits, size_t bit_idx) {
  (void)bm;
  (void)num_bits;
  (void)bit_idx;
  return false; /* TODO: implement */
}

/*
 * Drill 2: VSFS Inode Block Mapping (`bmap`, OSTEP Ch. 40 / xv6 `fs.c`)
 *
 * Maps a file's logical block index (`0..VSFS_NDIRECT + VSFS_NINDIRECT - 1`) to a
 * physical disk block number, optionally allocating missing data/indirect blocks.
 * Use `memcpy` when reading/writing `int32_t entries[VSFS_NINDIRECT]` in an indirect
 * disk block to avoid strict-aliasing undefined behavior on `uint8_t[]`.
 *
 * Requirements:
 * - Return false if disk == nullptr || ino == nullptr || phys_bn_out == nullptr.
 * - If `logical_bn < VSFS_NDIRECT`:
 *     - If `ino->direct[logical_bn] < 0`:
 *         - If `!alloc_if_missing`, return false.
 *         - Allocate a block via `bitmap_alloc_first(disk->data_bitmap, VSFS_NUM_BLOCKS)`.
 *         - If `< 0`, return false; zero `disk->blocks[blk]` and assign `ino->direct[logical_bn] = blk`.
 *     - Set `*phys_bn_out = ino->direct[logical_bn]` and return true.
 * - Else if `logical_bn < VSFS_NDIRECT + VSFS_NINDIRECT`:
 *     - `uint32_t idx = logical_bn - VSFS_NDIRECT;`
 *     - If `ino->indirect < 0`:
 *         - If `!alloc_if_missing`, return false.
 *         - Allocate indirect block; if `< 0`, return false.
 *         - Initialize all `VSFS_NINDIRECT` entries to `-1` and `memcpy` into `disk->blocks[ino->indirect]`.
 *     - `memcpy` indirect block into local `int32_t entries[VSFS_NINDIRECT]`.
 *     - If `entries[idx] < 0`:
 *         - If `!alloc_if_missing`, return false.
 *         - Allocate data block; if `< 0`, return false.
 *         - Zero `disk->blocks[blk]`, set `entries[idx] = blk`, and `memcpy` back to `disk->blocks[ino->indirect]`.
 *     - Set `*phys_bn_out = entries[idx]` and return true.
 * - Else return false.
 */
static bool vsfs_bmap(VsfsDisk *disk, VsfsInode *ino, uint32_t logical_bn,
                      bool alloc_if_missing, int32_t *phys_bn_out) {
  (void)disk;
  (void)ino;
  (void)logical_bn;
  (void)alloc_if_missing;
  (void)phys_bn_out;
  return false; /* TODO: implement */
}

/*
 * Drill 3: Directory Lookup & Entry Insertion (OSTEP Ch. 40)
 *
 * Requirements for vsfs_dir_lookup:
 * - Return false if entries == nullptr || name == nullptr || name[0] == '\0' || inum_out == nullptr.
 * - Scan `0..count`: if `entries[i].inum != 0 && strncmp(entries[i].name, name, sizeof(entries[i].name)) == 0`,
 *   set `*inum_out = entries[i].inum` and return true.
 * - Return false if not found.
 *
 * Requirements for vsfs_dir_link:
 * - Return false if entries == nullptr || name == nullptr || name[0] == '\0' ||
 *   strlen(name) >= sizeof(entries[0].name) || inum == 0.
 * - Reject duplicates: if `name` already exists among active entries (`inum != 0`), return false.
 * - Find first free slot (`entries[i].inum == 0`); if none, return false.
 * - Store `inum`, zero-fill `entries[i].name`, copy `strlen(name) + 1` bytes, return true.
 */
static bool vsfs_dir_lookup(const VsfsDirent *entries, size_t count,
                            const char *name, uint16_t *inum_out) {
  (void)entries;
  (void)count;
  (void)name;
  (void)inum_out;
  return false; /* TODO: implement */
}

static bool vsfs_dir_link(VsfsDirent *entries, size_t count,
                          const char *name, uint16_t inum) {
  (void)entries;
  (void)count;
  (void)name;
  (void)inum;
  return false; /* TODO: implement */
}

/*
 * Drill 4: Fletcher-16 Block Checksum & Write-Ahead Log Recovery (OSTEP Ch. 42 & 44 / xv6 `log.c`)
 *
 * Crash consistency requires verifying both transaction commit state AND payload
 * integrity before replaying logged blocks onto their home disk locations.
 *
 * Requirements for fletcher16_checksum:
 * - If data == nullptr && len > 0, return 0.
 * - Maintain `uint32_t sum1 = 0, sum2 = 0;`.
 * - For `i = 0 .. len - 1`:
 *     `sum1 = (sum1 + data[i]) % 255U;`
 *     `sum2 = (sum2 + sum1) % 255U;`
 * - Return `(uint16_t)((sum2 << 8) | sum1)`.
 *
 * Requirements for wal_recover:
 * - Return 0 if disk == nullptr || journal == nullptr.
 * - Iterate `0..num_txns`:
 *     - Replay `journal[i]` ONLY IF:
 *         `valid_magic == WAL_MAGIC && committed == true && target_block < VSFS_NUM_BLOCKS &&`
 *         `fletcher16_checksum(payload, VSFS_BLOCK_SIZE) == expected_checksum`.
 *     - Copy `payload` into `disk->blocks[target_block]` and increment replay count.
 * - Return number of replayed transactions.
 */
static uint16_t fletcher16_checksum(const uint8_t *data, size_t len) {
  (void)data;
  (void)len;
  return 0; /* TODO: implement */
}

static size_t wal_recover(VsfsDisk *disk, const WalTxn *journal, size_t num_txns) {
  (void)disk;
  (void)journal;
  (void)num_txns;
  return 0; /* TODO: implement */
}

int main(void) {
  /* Drill 1: Bitmap allocation & freeing */
  uint8_t bm[2] = {0};
  assert(bitmap_test(bm, 16, 0) == false);
  assert(bitmap_test(bm, 16, 16) == false);
  assert(bitmap_alloc_first(bm, 16) == 0);
  assert(bitmap_alloc_first(bm, 16) == 1);
  assert(bitmap_test(bm, 16, 0) == true);
  assert(bitmap_test(bm, 16, 1) == true);
  assert(bitmap_free(bm, 16, 0) == true);
  assert(bitmap_free(bm, 16, 0) == false); /* double free rejected */
  assert(bitmap_alloc_first(bm, 16) == 0); /* reuses freed slot 0 */

  uint8_t tiny_bm[1] = {0x03U}; /* 2-bit capacity, both set */
  assert(bitmap_alloc_first(tiny_bm, 2) == -1);

  /* Drill 2: VSFS bmap direct + single-indirect block allocation */
  VsfsDisk *disk = calloc(1, sizeof(*disk));
  assert(disk != nullptr);
  VsfsInode ino = {.size = 0, .direct = {-1, -1, -1, -1}, .indirect = -1};

  int32_t phys = -1;
  assert(vsfs_bmap(disk, &ino, 0, false, &phys) == false);
  assert(vsfs_bmap(disk, &ino, 0, true, &phys) == true && phys == 0);
  assert(vsfs_bmap(disk, &ino, 0, false, &phys) == true && phys == 0);

  /* Logical block 5 goes through indirect block (block 1 = indirect, block 2 = data) */
  assert(vsfs_bmap(disk, &ino, 5, true, &phys) == true);
  assert(ino.indirect == 1 && phys == 2);
  assert(vsfs_bmap(disk, &ino, 6, false, &phys) == false);
  assert(vsfs_bmap(disk, &ino, 6, true, &phys) == true && phys == 3);
  assert(vsfs_bmap(disk, &ino, VSFS_NDIRECT + VSFS_NINDIRECT, true, &phys) == false);

  /* Drill 3: Directory lookup & link */
  VsfsDirent dir[4] = {0};
  uint16_t found_inum = 0;
  assert(vsfs_dir_link(dir, 4, ".", 1) == true);
  assert(vsfs_dir_link(dir, 4, "..", 1) == true);
  assert(vsfs_dir_link(dir, 4, "readme.txt", 7) == true);
  assert(vsfs_dir_link(dir, 4, "readme.txt", 8) == false); /* duplicate name */
  assert(vsfs_dir_link(dir, 4, "too_long_filename_14", 9) == false);
  assert(vsfs_dir_link(dir, 4, "bad_inum", 0) == false);
  assert(vsfs_dir_lookup(dir, 4, "readme.txt", &found_inum) == true && found_inum == 7);
  assert(vsfs_dir_lookup(dir, 4, "missing", &found_inum) == false);

  /* Drill 4: Fletcher-16 checksum & WAL crash recovery */
  const uint8_t sample[] = "ostep";
  uint16_t f16 = fletcher16_checksum(sample, 5);
  assert(f16 != 0);

  memset(disk->blocks[10], 0xAA, VSFS_BLOCK_SIZE);
  memset(disk->blocks[11], 0xBB, VSFS_BLOCK_SIZE);
  memset(disk->blocks[12], 0xCC, VSFS_BLOCK_SIZE);

  WalTxn journal[3] = {0};
  /* Txn 0: Valid & committed -> MUST replay */
  journal[0].valid_magic = WAL_MAGIC;
  journal[0].committed = true;
  journal[0].target_block = 10;
  memset(journal[0].payload, 0x11, VSFS_BLOCK_SIZE);
  journal[0].expected_checksum = fletcher16_checksum(journal[0].payload, VSFS_BLOCK_SIZE);

  /* Txn 1: Uncommitted (power loss before commit block) -> MUST skip */
  journal[1].valid_magic = WAL_MAGIC;
  journal[1].committed = false;
  journal[1].target_block = 11;
  memset(journal[1].payload, 0x22, VSFS_BLOCK_SIZE);
  journal[1].expected_checksum = fletcher16_checksum(journal[1].payload, VSFS_BLOCK_SIZE);

  /* Txn 2: Committed, but payload corrupted (bit flip) -> MUST skip */
  journal[2].valid_magic = WAL_MAGIC;
  journal[2].committed = true;
  journal[2].target_block = 12;
  memset(journal[2].payload, 0x33, VSFS_BLOCK_SIZE);
  journal[2].expected_checksum = fletcher16_checksum(journal[2].payload, VSFS_BLOCK_SIZE);
  journal[2].payload[0] ^= 0x01U;

  assert(wal_recover(disk, journal, 3) == 1);
  assert(disk->blocks[10][0] == 0x11);
  assert(disk->blocks[11][0] == 0xBB);
  assert(disk->blocks[12][0] == 0xCC);

  free(disk);
  printf("[PASS] Exercise 58: OSTEP VSFS & WAL Crash Recovery verified.\n");
  return EXIT_SUCCESS;
}
