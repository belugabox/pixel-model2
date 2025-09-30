/* miniz_tinfl.h - upstream content (single clean copy) */
#pragma once

#include "miniz_common.h"

#ifndef MINIZ_NO_INFLATE_APIS

#ifdef __cplusplus
extern "C" {
#endif

/* Decompression flags used by tinfl_decompress(). */
enum { TINFL_FLAG_PARSE_ZLIB_HEADER = 1, TINFL_FLAG_HAS_MORE_INPUT = 2, TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF = 4, TINFL_FLAG_COMPUTE_ADLER32 = 8 };

/* High level decompression functions: */
MINIZ_EXPORT void *tinfl_decompress_mem_to_heap(const void *pSrc_buf, size_t src_buf_len, size_t *pOut_len, int flags);
MINIZ_EXPORT size_t tinfl_decompress_mem_to_mem(void *pOut_buf, size_t out_buf_len, const void *pSrc_buf, size_t src_buf_len, int flags);
MINIZ_EXPORT int tinfl_decompress_mem_to_callback(const void *pIn_buf, size_t *pIn_buf_size, int (*pPut_buf_func)(const void *pBuf, int len, void *pUser), void *pPut_buf_user, int flags);

typedef int (*tinfl_put_buf_func_ptr)(const void *pBuf, int len, void *pUser);

struct tinfl_decompressor_tag;
typedef struct tinfl_decompressor_tag tinfl_decompressor;

#ifndef MINIZ_NO_MALLOC
MINIZ_EXPORT tinfl_decompressor *tinfl_decompressor_alloc(void);
MINIZ_EXPORT void tinfl_decompressor_free(tinfl_decompressor *pDecomp);
#endif

/* Max size of LZ dictionary. */
#define TINFL_LZ_DICT_SIZE 32768

/* Return status. */
typedef enum { TINFL_STATUS_FAILED_CANNOT_MAKE_PROGRESS = -4, TINFL_STATUS_BAD_PARAM = -3, TINFL_STATUS_ADLER32_MISMATCH = -2, TINFL_STATUS_FAILED = -1, TINFL_STATUS_DONE = 0, TINFL_STATUS_NEEDS_MORE_INPUT = 1, TINFL_STATUS_HAS_MORE_OUTPUT = 2 } tinfl_status;

/* Initializes the decompressor to its initial state. */
#define tinfl_init(r) do { (r)->m_state = 0; } MZ_MACRO_END
#define tinfl_get_adler32(r) (r)->m_check_adler32

MINIZ_EXPORT tinfl_status tinfl_decompress(tinfl_decompressor *r, const mz_uint8 *pIn_buf_next, size_t *pIn_buf_size, mz_uint8 *pOut_buf_start, mz_uint8 *pOut_buf_next, size_t *pOut_buf_size, const mz_uint32 decomp_flags);

#ifdef __cplusplus
}
#endif

#endif /* MINIZ_NO_INFLATE_APIS */
