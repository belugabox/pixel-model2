/* miniz_tdef.h - full upstream content */
#pragma once
#include "miniz_common.h"

#ifndef MINIZ_NO_DEFLATE_APIS

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------- Low-level Compression API Definitions */

/* Set TDEFL_LESS_MEMORY to 1 to use less memory (compression will be slightly
 * slower, and raw/dynamic blocks will be output more frequently).
 */
#ifndef TDEFL_LESS_MEMORY
#define TDEFL_LESS_MEMORY 0
#endif

/* tdefl_init() compression flags logically OR'd together (low 12 bits contain
 * the max. number of probes per dictionary search):
 */
enum { TDEFL_HUFFMAN_ONLY = 0, TDEFL_DEFAULT_MAX_PROBES = 128, TDEFL_MAX_PROBES_MASK = 0xFFF };

/* TDEFL_WRITE_ZLIB_HEADER: If set, the compressor outputs a zlib header before
 * the deflate data, and the Adler-32 of the source data at the end. Otherwise,
 * you'll get raw deflate data.
 */
enum { TDEFL_WRITE_ZLIB_HEADER = 0x01000, TDEFL_COMPUTE_ADLER32 = 0x02000, TDEFL_GREEDY_PARSING_FLAG = 0x04000, TDEFL_NONDETERMINISTIC_PARSING_FLAG = 0x08000, TDEFL_RLE_MATCHES = 0x10000, TDEFL_FILTER_MATCHES = 0x20000, TDEFL_FORCE_ALL_STATIC_BLOCKS = 0x40000, TDEFL_FORCE_ALL_RAW_BLOCKS = 0x80000 };

/* High level compression functions: */
MINIZ_EXPORT void *tdefl_compress_mem_to_heap(const void *pSrc_buf, size_t src_buf_len, size_t *pOut_len, int flags);
MINIZ_EXPORT size_t tdefl_compress_mem_to_mem(void *pOut_buf, size_t out_buf_len, const void *pSrc_buf, size_t src_buf_len, int flags);
MINIZ_EXPORT void *tdefl_write_image_to_png_file_in_memory_ex(const void *pImage, int w, int h, int num_chans, size_t *pLen_out, mz_uint level, mz_bool flip);
MINIZ_EXPORT void *tdefl_write_image_to_png_file_in_memory(const void *pImage, int w, int h, int num_chans, size_t *pLen_out);

/* Output stream interface. The compressor uses this interface to write
 * compressed data. It'll typically be called TDEFL_OUT_BUF_SIZE at a time.
 */
typedef mz_bool (*tdefl_put_buf_func_ptr)(const void *pBuf, int len, void *pUser);

enum { TDEFL_MAX_HUFF_TABLES = 3, TDEFL_MAX_HUFF_SYMBOLS_0 = 288, TDEFL_MAX_HUFF_SYMBOLS_1 = 32, TDEFL_MAX_HUFF_SYMBOLS_2 = 19, TDEFL_LZ_DICT_SIZE = 32768, TDEFL_LZ_DICT_SIZE_MASK = TDEFL_LZ_DICT_SIZE - 1, TDEFL_MIN_MATCH_LEN = 3, TDEFL_MAX_MATCH_LEN = 258 };

/* TDEFL_OUT_BUF_SIZE MUST be large enough to hold a single entire compressed
 * output block (using static/fixed Huffman codes).
 */
#if TDEFL_LESS_MEMORY
enum { TDEFL_LZ_CODE_BUF_SIZE = 24 * 1024, TDEFL_OUT_BUF_SIZE = (TDEFL_LZ_CODE_BUF_SIZE * 13) / 10, TDEFL_MAX_HUFF_SYMBOLS = 288, TDEFL_LZ_HASH_BITS = 12, TDEFL_LEVEL1_HASH_SIZE_MASK = 4095, TDEFL_LZ_HASH_SHIFT = (TDEFL_LZ_HASH_BITS + 2) / 3, TDEFL_LZ_HASH_SIZE = 1 << TDEFL_LZ_HASH_BITS };
#else
enum { TDEFL_LZ_CODE_BUF_SIZE = 64 * 1024, TDEFL_OUT_BUF_SIZE = (mz_uint)((TDEFL_LZ_CODE_BUF_SIZE * 13) / 10), TDEFL_MAX_HUFF_SYMBOLS = 288, TDEFL_LZ_HASH_BITS = 15, TDEFL_LEVEL1_HASH_SIZE_MASK = 4095, TDEFL_LZ_HASH_SHIFT = (TDEFL_LZ_HASH_BITS + 2) / 3, TDEFL_LZ_HASH_SIZE = 1 << TDEFL_LZ_HASH_BITS };
#endif

/* The low-level tdefl functions below may be used directly if the above helper
 * functions aren't flexible enough. The low-level functions don't make any heap
 * allocations, unlike the above helper functions.
 */
typedef enum { TDEFL_STATUS_BAD_PARAM = -2, TDEFL_STATUS_PUT_BUF_FAILED = -1, TDEFL_STATUS_OKAY = 0, TDEFL_STATUS_DONE = 1 } tdefl_status;

/* Must map to MZ_NO_FLUSH, MZ_SYNC_FLUSH, etc. enums */
typedef enum { TDEFL_NO_FLUSH = 0, TDEFL_SYNC_FLUSH = 2, TDEFL_FULL_FLUSH = 3, TDEFL_FINISH = 4 } tdefl_flush;

/* tdefl's compression state structure. */
typedef struct {
	tdefl_put_buf_func_ptr m_pPut_buf_func;
	void *m_pPut_buf_user;
	mz_uint m_flags, m_max_probes[2];
	int m_greedy_parsing;
	mz_uint m_adler32, m_lookahead_pos, m_lookahead_size, m_dict_size;
	mz_uint8 *m_pLZ_code_buf, *m_pLZ_flags, *m_pOutput_buf, *m_pOutput_buf_end;
	mz_uint m_num_flags_left, m_total_lz_bytes, m_lz_code_buf_dict_pos, m_bits_in, m_bit_buffer;
	mz_uint m_saved_match_dist, m_saved_match_len, m_saved_lit, m_output_flush_ofs, m_output_flush_remaining, m_finished, m_block_index, m_wants_to_finish;
	tdefl_status m_prev_return_status;
	const void *m_pIn_buf;
	void *m_pOut_buf;
	size_t *m_pIn_buf_size, *m_pOut_buf_size;
	tdefl_flush m_flush;
	const mz_uint8 *m_pSrc;
	size_t m_src_buf_left, m_out_buf_ofs;
	mz_uint8 m_dict[TDEFL_LZ_DICT_SIZE + TDEFL_MAX_MATCH_LEN - 1];
	mz_uint16 m_huff_count[TDEFL_MAX_HUFF_TABLES][TDEFL_MAX_HUFF_SYMBOLS];
	mz_uint16 m_huff_codes[TDEFL_MAX_HUFF_TABLES][TDEFL_MAX_HUFF_SYMBOLS];
	mz_uint8 m_huff_code_sizes[TDEFL_MAX_HUFF_TABLES][TDEFL_MAX_HUFF_SYMBOLS];
	mz_uint8 m_lz_code_buf[TDEFL_LZ_CODE_BUF_SIZE];
	mz_uint16 m_next[TDEFL_LZ_DICT_SIZE];
	mz_uint16 m_hash[TDEFL_LZ_HASH_SIZE];
	mz_uint8 m_output_buf[TDEFL_OUT_BUF_SIZE];
} tdefl_compressor;

/* Initializes the compressor. */
MINIZ_EXPORT tdefl_status tdefl_init(tdefl_compressor *d, tdefl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags);

/* Compresses a block of data, consuming as much of the specified input
 * buffer as possible, and writing as much compressed data to the specified
 * output buffer as possible.
 */
MINIZ_EXPORT tdefl_status tdefl_compress(tdefl_compressor *d, const void *pIn_buf, size_t *pIn_buf_size, void *pOut_buf, size_t *pOut_buf_size, tdefl_flush flush);

/* tdefl_compress_buffer() is only usable when the tdefl_init() is called with
 * a non-NULL tdefl_put_buf_func_ptr. tdefl_compress_buffer() always consumes
 * the entire input buffer.
 */
MINIZ_EXPORT tdefl_status tdefl_compress_buffer(tdefl_compressor *d, const void *pIn_buf, size_t in_buf_size, tdefl_flush flush);

MINIZ_EXPORT tdefl_status tdefl_get_prev_return_status(tdefl_compressor *d);
MINIZ_EXPORT mz_uint32 tdefl_get_adler32(tdefl_compressor *d);

MINIZ_EXPORT mz_bool tdefl_compress_mem_to_output(const void *pBuf, size_t buf_len, tdefl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags);
MINIZ_EXPORT mz_uint tdefl_create_comp_flags_from_zip_params(int level, int window_bits, int strategy);

#ifndef MINIZ_NO_MALLOC
MINIZ_EXPORT tdefl_compressor *tdefl_compressor_alloc(void);
MINIZ_EXPORT void tdefl_compressor_free(tdefl_compressor *pComp);
#endif

#ifdef __cplusplus
}
#endif

#endif /* MINIZ_NO_DEFLATE_APIS */
