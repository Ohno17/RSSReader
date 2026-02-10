#ifndef LIBPSL_H
#define LIBPSL_H

/* Define a dummy struct so psl_ctx_t* has a real type to point to */
typedef struct psl_ctx_st psl_ctx_t;

#define PSL_TYPE_ANY            0
#define PSL_TYPE_NO_STAR_RULE   0

/* * psl_latest returns psl_ctx_t*, so return NULL 
 */
static inline psl_ctx_t* psl_latest(const char* path) { 
    return (psl_ctx_t*)0; 
}

/* * libsoup expects this to return a boolean-like int. 
 * Returning 0 (false) is safe here.
 */
static inline int psl_is_public_suffix2(const psl_ctx_t* psl, const char* domain, int type) { 
    return 0; 
}

/* * These return const char*, so (const char*)0 or NULL is appropriate
 */
static inline const char* psl_unregistrable_domain(const psl_ctx_t* psl, const char* domain) { 
    return (const char*)0; 
}

static inline const char* psl_registrable_domain(const psl_ctx_t* psl, const char* domain) { 
    return (const char*)0; 
}

#endif