/*
  This is not the fastest, most space-conserving, most portable, or
  most tunable malloc ever written. However it is among the fastest
  while also being among the most space-conserving, portable and tunable.
  Consistent balance across these factors results in a good general-purpose
  allocator for malloc-intensive programs.

  The main properties of the algorithms are:
  * For large (>= 512 bytes) requests, it is a pure best-fit allocator,
    with ties normally decided via FIFO (i.e. least recently used).
  * For small (<= 64 bytes by default) requests, it is a caching
    allocator, that maintains pools of quickly recycled chunks.
  * In between, and for combinations of large and small requests, it does
    the best it can trying to meet both goals at once.
  * For very large requests (>= 128KB by default), it relies on system
    memory mapping facilities, if supported.

  For a longer but slightly out of date high-level description, see
     http://gee.cs.oswego.edu/dl/html/malloc.html

  You may already by default be using a C library containing a malloc
  that is  based on some version of this malloc (for example in
  linux). You might still want to use the one in this file in order to
  customize settings or to avoid overheads associated with library
  versions.

* Contents, described in more detail in "description of public routines" below.

  Standard (ANSI/SVID/...)  functions:
    malloc(size_t n);
    calloc(size_t n_elements, size_t element_size);
    free(void* p);
    realloc(void* p, size_t n);
    memalign(size_t alignment, size_t n);
    valloc(size_t n);
    mallinfo()
    mallopt(int parameter_number, int parameter_value)

  Additional functions:
    independent_calloc(size_t n_elements, size_t size, void* chunks[]);
    independent_comalloc(size_t n_elements, size_t sizes[], void* chunks[]);
    pvalloc(size_t n);
    cfree(void* p);
    malloc_trim(size_t pad);
    malloc_usable_size(void* p);
    malloc_stats();

* Vital statistics:

  Supported pointer representation:       4 or 8 bytes
  Supported size_t  representation:       4 or 8 bytes 
       Note that size_t is allowed to be 4 bytes even if pointers are 8.
       You can adjust this by defining INTERNAL_SIZE_T

  Alignment:                              2 * sizeof(size_t) (default)
       (i.e., 8 byte alignment with 4byte size_t). This suffices for
       nearly all current machines and C compilers. However, you can
       define MALLOC_ALIGNMENT to be wider than this if necessary.

  Minimum overhead per allocated chunk:   4 or 8 bytes
       Each malloced chunk has a hidden word of overhead holding size
       and status information.

  Minimum allocated size: 4-byte ptrs:  16 bytes    (including 4 overhead)
                          8-byte ptrs:  24/32 bytes (including, 4/8 overhead)

       When a chunk is freed, 12 (for 4byte ptrs) or 20 (for 8 byte
       ptrs but 4 byte size) or 24 (for 8/8) additional bytes are
       needed; 4 (8) for a trailing size field and 8 (16) bytes for
       free list pointers. Thus, the minimum allocatable size is
       16/24/32 bytes.

       Even a request for zero bytes (i.e., malloc(0)) returns a
       pointer to something of the minimum allocatable size.

       The maximum overhead wastage (i.e., number of extra bytes
       allocated than were requested in malloc) is less than or equal
       to the minimum size, except for requests >= mmap_threshold that
       are serviced via mmap(), where the worst case wastage is 2 *
       sizeof(size_t) bytes plus the remainder from a system page (the
       minimal mmap unit); typically 4096 or 8192 bytes.

  Maximum allocated size:  4-byte size_t: 2^32 minus about two pages 
                           8-byte size_t: 2^64 minus about two pages

       It is assumed that (possibly signed) size_t values suffice to
       represent chunk sizes. `Possibly signed' is due to the fact
       that `size_t' may be defined on a system as either a signed or
       an unsigned type. The ISO C standard says that it must be
       unsigned, but a few systems are known not to adhere to this.
       Additionally, even when size_t is unsigned, sbrk (which is by
       default used to obtain memory from system) accepts signed
       arguments, and may not be able to handle size_t-wide arguments
       with negative sign bit.  Generally, values that would
       appear as negative after accounting for overhead and alignment
       are supported only via mmap(), which does not have this
       limitation.

       Requests for sizes outside the allowed range will perform an optional
       failure action and then return null. (Requests may also
       also fail because a system is out of memory.)

  Thread-safety: NOT thread-safe unless USE_MALLOC_LOCK defined

       When USE_MALLOC_LOCK is defined, wrappers are created to
       surround every public call with either a pthread mutex or
       a win32 spinlock (depending on WIN32). This is not
       especially fast, and can be a major bottleneck.
       It is designed only to provide minimal protection
       in concurrent environments, and to provide a basis for
       extensions.  If you are using malloc in a concurrent program,
       you would be far better off obtaining ptmalloc, which is
       derived from a version of this malloc, and is well-tuned for
       concurrent programs. (See http://www.malloc.de) Note that
       even when USE_MALLOC_LOCK is defined, you can can guarantee
       full thread-safety only if no threads acquire memory through 
       direct calls to MORECORE or other system-level allocators.

  Compliance: I believe it is compliant with the 1997 Single Unix Specification
       (See http://www.opennc.org). Also SVID/XPG, ANSI C, and probably 
       others as well.
*/

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include "malloc.h"

#define CHUNK_SIZE_T           unsigned long
#define PTR_UINT               unsigned long
#define INTERNAL_SIZE_T        size_t
#define SIZE_SZ                (sizeof(INTERNAL_SIZE_T))
#define MALLOC_ALIGNMENT       (2 * SIZE_SZ)
#define MALLOC_ALIGN_MASK      (MALLOC_ALIGNMENT - 1)
#define MORECORE               sbrk
#define MORECORE_FAILURE       (-1)
#define MMAP_AS_MORECORE_SIZE  (1024 * 1024) /* must be a multiple of page size.*/
#define DEFAULT_SYS_PAGE_SIZE  4096
#define DEFAULT_MXFAST         64
#define DEFAULT_TRIM_THRESHOLD (256 * 1024)
#define DEFAULT_TOP_PAD        (0)
#define DEFAULT_MMAP_THRESHOLD (256 * 1024)
#define DEFAULT_MMAP_MAX       (65536)

#define MMAP(addr, size, prot, flags) \
  (mmap((addr), (size), (prot), (flags)|MAP_ANONYMOUS, -1, 0))

/*
   malloc_chunk details:

    (The following includes lightly edited explanations by Colin Plumb.)

    Chunks of memory are maintained using a `boundary tag' method as
    described in e.g., Knuth or Standish.  (See the paper by Paul
    Wilson ftp://ftp.cs.utexas.edu/pub/garbage/allocsrv.ps for a
    survey of such techniques.)  Sizes of free chunks are stored both
    in the front of each chunk and at the end.  This makes
    consolidating fragmented chunks into bigger chunks very fast.  The
    size fields also hold bits representing whether chunks are free or
    in use.

    An allocated chunk looks like this:


    chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Size of previous chunk, if allocated            | |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Size of chunk, in bytes                         |P|
      mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             User data starts here...                          .
            .                                                               .
            .             (malloc_usable_space() bytes)                     .
            .                                                               |
nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Size of chunk                                     |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


    Where "chunk" is the front of the chunk for the purpose of most of
    the malloc code, but "mem" is the pointer that is returned to the
    user.  "Nextchunk" is the beginning of the next contiguous chunk.

    Chunks always begin on even word boundries, so the mem portion
    (which is returned to the user) is also on an even word boundary, and
    thus at least double-word aligned.

    Free chunks are stored in circular doubly-linked lists, and look like this:

    chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Size of previous chunk                            |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    `head:' |             Size of chunk, in bytes                         |P|
      mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Forward pointer to next chunk in list             |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Back pointer to previous chunk in list            |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Unused space (may be 0 bytes long)                .
            .                                                               .
            .                                                               |
nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    `foot:' |             Size of chunk, in bytes                           |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    The P (PREV_INUSE) bit, stored in the unused low-order bit of the
    chunk size (which is always a multiple of two words), is an in-use
    bit for the *previous* chunk.  If that bit is *clear*, then the
    word before the current chunk size contains the previous chunk
    size, and can be used to find the front of the previous chunk.
    The very first chunk allocated always has this bit set,
    preventing access to non-existent (or non-owned) memory. If
    prev_inuse is set for any given chunk, then you CANNOT determine
    the size of the previous chunk, and might even get a memory
    addressing fault when trying to do so.

    Note that the `foot' of the current chunk is actually represented
    as the prev_size of the NEXT chunk. This makes it easier to
    deal with alignments etc but can be very confusing when trying
    to extend or adapt this code.

    The two exceptions to all this are

     1. The special chunk `top' doesn't bother using the
        trailing size field since there is no next contiguous chunk
        that would have to index off it. After initialization, `top'
        is forced to always exist.  If it would become less than
        MINSIZE bytes long, it is replenished.

     2. Chunks allocated via mmap, which have the second-lowest-order
        bit (IS_MMAPPED) set in their size fields.  Because they are
        allocated one-by-one, each must contain its own trailing size field.

*/
struct malloc_chunk {
  INTERNAL_SIZE_T      prev_size;  /* Size of previous chunk (if free).  */
  INTERNAL_SIZE_T      size;       /* Size in bytes, including overhead. */

  struct malloc_chunk* fd;         /* double links -- used only if free. */
  struct malloc_chunk* bk;
};

typedef struct malloc_chunk* mchunkptr;

#define chunk2mem(p)   ((void*)((char*)(p) + 2*SIZE_SZ))
#define mem2chunk(mem) ((mchunkptr)((char*)(mem) - 2*SIZE_SZ))
#define MIN_CHUNK_SIZE (sizeof(struct malloc_chunk))
#define MINSIZE        (CHUNK_SIZE_T)(((MIN_CHUNK_SIZE+MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK))
#define aligned_OK(m)  (((PTR_UINT)((m)) & (MALLOC_ALIGN_MASK)) == 0)


/* 
   Check if a request is so large that it would wrap around zero when
   padded and aligned. To simplify some other code, the bound is made
   low enough so that adding MINSIZE will also not wrap around sero.
*/

#define REQUEST_OUT_OF_RANGE(req)                                 \
  ((CHUNK_SIZE_T)(req) >=                                        \
   (CHUNK_SIZE_T)(INTERNAL_SIZE_T)(-2 * MINSIZE))

/* pad request bytes into a usable size -- internal version */

#define request2size(req)                                         \
  (((req) + SIZE_SZ + MALLOC_ALIGN_MASK < MINSIZE)  ?             \
   MINSIZE :                                                      \
   ((req) + SIZE_SZ + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK)

/*  Same, except also perform argument check */

#define checked_request2size(req, sz)                             \
  if (REQUEST_OUT_OF_RANGE(req)) {                                \
    errno = ENOMEM;                                               \
    return 0;                                                     \
  }                                                               \
  (sz) = request2size(req);


/* size field is or'ed with PREV_INUSE when previous adjacent chunk in use */
#define PREV_INUSE 0x1

/* extract inuse bit of previous chunk */
#define prev_inuse(p)       ((p)->size & PREV_INUSE)


/* size field is or'ed with IS_MMAPPED if the chunk was obtained with mmap() */
#define IS_MMAPPED 0x2

/* check for mmap()'ed chunk */
#define chunk_is_mmapped(p) ((p)->size & IS_MMAPPED)

/* 
  Bits to mask off when extracting size 

  Note: IS_MMAPPED is intentionally not masked off from size field in
  macros for which mmapped chunks should never be seen. This should
  cause helpful core dumps to occur if it is tried by accident by
  people extending or adapting this malloc.
*/
#define SIZE_BITS (PREV_INUSE|IS_MMAPPED)

/* Get size, ignoring use bits */
#define chunksize(p)         ((p)->size & ~(SIZE_BITS))

/* Ptr to next physical malloc_chunk. */
#define next_chunk(p) ((mchunkptr)( ((char*)(p)) + ((p)->size & ~PREV_INUSE) ))

/* Ptr to previous physical malloc_chunk */
#define prev_chunk(p) ((mchunkptr)( ((char*)(p)) - ((p)->prev_size) ))

/* Treat space at ptr + offset as a chunk */
#define chunk_at_offset(p, s)  ((mchunkptr)(((char*)(p)) + (s)))

/* extract p's inuse bit */
#define inuse(p)\
((((mchunkptr)(((char*)(p))+((p)->size & ~PREV_INUSE)))->size) & PREV_INUSE)

/* set/clear chunk as being inuse without otherwise disturbing */
#define set_inuse(p)\
((mchunkptr)(((char*)(p)) + ((p)->size & ~PREV_INUSE)))->size |= PREV_INUSE

#define clear_inuse(p)\
((mchunkptr)(((char*)(p)) + ((p)->size & ~PREV_INUSE)))->size &= ~(PREV_INUSE)


/* check/set/clear inuse bits in known places */
#define inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((char*)(p)) + (s)))->size & PREV_INUSE)

#define set_inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((char*)(p)) + (s)))->size |= PREV_INUSE)

#define clear_inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((char*)(p)) + (s)))->size &= ~(PREV_INUSE))


/* Set size at head, without disturbing its use bit */
#define set_head_size(p, s)  ((p)->size = (((p)->size & PREV_INUSE) | (s)))

/* Set size/use field */
#define set_head(p, s)       ((p)->size = (s))

/* Set size at footer (only when chunk is not in use) */
#define set_foot(p, s)       (((mchunkptr)((char*)(p) + (s)))->prev_size = (s))


/*
  Bins

    An array of bin headers for free chunks. Each bin is doubly
    linked.  The bins are approximately proportionally (log) spaced.
    There are a lot of these bins (128). This may look excessive, but
    works very well in practice.  Most bins hold sizes that are
    unusual as malloc request sizes, but are more usual for fragments
    and consolidated sets of chunks, which is what these bins hold, so
    they can be found quickly.  All procedures maintain the invariant
    that no consolidated chunk physically borders another one, so each
    chunk in a list is known to be preceeded and followed by either
    inuse chunks or the ends of memory.

    Chunks in bins are kept in size order, with ties going to the
    approximately least recently used chunk. Ordering isn't needed
    for the small bins, which all contain the same-sized chunks, but
    facilitates best-fit allocation for larger chunks. These lists
    are just sequential. Keeping them in order almost never requires
    enough traversal to warrant using fancier ordered data
    structures.  

    Chunks of the same size are linked with the most
    recently freed at the front, and allocations are taken from the
    back.  This results in LRU (FIFO) allocation order, which tends
    to give each chunk an equal opportunity to be consolidated with
    adjacent freed chunks, resulting in larger free chunks and less
    fragmentation.

    To simplify use in double-linked lists, each bin header acts
    as a malloc_chunk. This avoids special-casing for headers.
    But to conserve space and improve locality, we allocate
    only the fd/bk pointers of bins, and then use repositioning tricks
    to treat these as the fields of a malloc_chunk*.  
*/

typedef struct malloc_chunk* mbinptr;

/* addressing -- note that bin_at(0) does not exist */
#define bin_at(m, i) ((mbinptr)((char*)&((m)->bins[(i)<<1]) - (SIZE_SZ<<1)))

/* analog of ++bin */
#define next_bin(b)  ((mbinptr)((char*)(b) + (sizeof(mchunkptr)<<1)))

/* Reminders about list directionality within bins */
#define first(b)     ((b)->fd)
#define last(b)      ((b)->bk)

/* Take a chunk off a bin list */
#define unlink(P, BK, FD) {                                            \
  FD = P->fd;                                                          \
  BK = P->bk;                                                          \
  FD->bk = BK;                                                         \
  BK->fd = FD;                                                         \
}

/*
  Indexing

    Bins for sizes < 512 bytes contain chunks of all the same size, spaced
    8 bytes apart. Larger bins are approximately logarithmically spaced:

    64 bins of size       8
    32 bins of size      64
    16 bins of size     512
     8 bins of size    4096
     4 bins of size   32768
     2 bins of size  262144
     1 bin  of size what's left

    The bins top out around 1MB because we expect to service large
    requests via mmap.
*/

#define NBINS              96
#define NSMALLBINS         32
#define SMALLBIN_WIDTH      8
#define MIN_LARGE_SIZE    256

#define in_smallbin_range(sz)  \
  ((CHUNK_SIZE_T)(sz) < (CHUNK_SIZE_T)MIN_LARGE_SIZE)

#define smallbin_index(sz)     (((unsigned)(sz)) >> 3)

/*
  Compute index for size. We expect this to be inlined when
  compiled with optimization, else not, which works out well.
*/
static int largebin_index(unsigned int sz) {
  unsigned int  x = sz >> SMALLBIN_WIDTH; 
  unsigned int m;            /* bit position of highest set bit of m */

  if (x >= 0x10000) return NBINS-1;

  {
    /*
      Based on branch-free nlz algorithm in chapter 5 of Henry
      S. Warren Jr's book "Hacker's Delight".
    */

    unsigned int n = ((x - 0x100) >> 16) & 8;
    x <<= n; 
    m = ((x - 0x1000) >> 16) & 4;
    n += m; 
    x <<= m; 
    m = ((x - 0x4000) >> 16) & 2;
    n += m; 
    x = (x << m) >> 14;
    m = 13 - n + (x & ~(x>>1));
  }

  /* Use next 2 bits to create finer-granularity bins */
  return NSMALLBINS + (m << 2) + ((sz >> (m + 6)) & 3);
}

#define bin_index(sz) \
 ((in_smallbin_range(sz)) ? smallbin_index(sz) : largebin_index(sz))

/*
  FIRST_SORTED_BIN_SIZE is the chunk size corresponding to the
  first bin that is maintained in sorted order. This must
  be the smallest size corresponding to a given bin.

  Normally, this should be MIN_LARGE_SIZE. But you can weaken
  best fit guarantees to sometimes speed up malloc by increasing value.
  Doing this means that malloc may choose a chunk that is 
  non-best-fitting by up to the width of the bin.

  Some useful cutoff values:
      512 - all bins sorted
     2560 - leaves bins <=     64 bytes wide unsorted  
    12288 - leaves bins <=    512 bytes wide unsorted
    65536 - leaves bins <=   4096 bytes wide unsorted
   262144 - leaves bins <=  32768 bytes wide unsorted
       -1 - no bins sorted (not recommended!)
*/

#define FIRST_SORTED_BIN_SIZE MIN_LARGE_SIZE

/*
  Unsorted chunks

    All remainders from chunk splits, as well as all returned chunks,
    are first placed in the "unsorted" bin. They are then placed
    in regular bins after malloc gives them ONE chance to be used before
    binning. So, basically, the unsorted_chunks list acts as a queue,
    with chunks being placed on it in free (and malloc_consolidate),
    and taken off (to be either used or placed in bins) in malloc.
*/

/* The otherwise unindexable 1-bin is used to hold unsorted chunks. */
#define unsorted_chunks(M)          (bin_at(M, 1))

/*
  Top

    The top-most available chunk (i.e., the one bordering the end of
    available memory) is treated specially. It is never included in
    any bin, is used only if no other chunk is available, and is
    released back to the system if it is very large (see
    M_TRIM_THRESHOLD).  Because top initially
    points to its own bin with initial zero size, thus forcing
    extension on the first malloc request, we avoid having any special
    code in malloc to check whether it even exists yet. But we still
    need to do so when getting memory from system, so we make
    initial_top treat the bin as a legal but unusable chunk during the
    interval between initialization and the first call to
    sysmalloc. (This is somewhat delicate, since it relies on
    the 2 preceding words to be zero during this interval as well.)
*/

/* Conveniently, the unsorted bin can be used as dummy top on first call */
#define initial_top(M)              (unsorted_chunks(M))

/*
  Binmap

    To help compensate for the large number of bins, a one-level index
    structure is used for bin-by-bin searching.  `binmap' is a
    bitvector recording whether bins are definitely empty so they can
    be skipped over during during traversals.  The bits are NOT always
    cleared as soon as bins are empty, but instead only
    when they are noticed to be empty during traversal in malloc.
*/

/* Conservatively use 32 bits per map word, even if on 64bit system */
#define BINMAPSHIFT      5
#define BITSPERMAP       (1U << BINMAPSHIFT)
#define BINMAPSIZE       (NBINS / BITSPERMAP)

#define idx2block(i)     ((i) >> BINMAPSHIFT)
#define idx2bit(i)       ((1U << ((i) & ((1U << BINMAPSHIFT)-1))))

#define mark_bin(m,i)    ((m)->binmap[idx2block(i)] |=  idx2bit(i))
#define unmark_bin(m,i)  ((m)->binmap[idx2block(i)] &= ~(idx2bit(i)))
#define get_binmap(m,i)  ((m)->binmap[idx2block(i)] &   idx2bit(i))

/*
  Fastbins

    An array of lists holding recently freed small chunks.  Fastbins
    are not doubly linked.  It is faster to single-link them, and
    since chunks are never removed from the middles of these lists,
    double linking is not necessary. Also, unlike regular bins, they
    are not even processed in FIFO order (they use faster LIFO) since
    ordering doesn't much matter in the transient contexts in which
    fastbins are normally used.

    Chunks in fastbins keep their inuse bit set, so they cannot
    be consolidated with other free chunks. malloc_consolidate
    releases all chunks in fastbins and consolidates them with
    other free chunks. 
*/

typedef struct malloc_chunk* mfastbinptr;

/* offset 2 to use otherwise unindexable first 2 bins */
#define fastbin_index(sz)        ((((unsigned int)(sz)) >> 3) - 2)

/* The maximum fastbin request size we support */
#define MAX_FAST_SIZE     80

#define NFASTBINS  (fastbin_index(request2size(MAX_FAST_SIZE))+1)

/*
  FASTBIN_CONSOLIDATION_THRESHOLD is the size of a chunk in free()
  that triggers automatic consolidation of possibly-surrounding
  fastbin chunks. This is a heuristic, so the exact value should not
  matter too much. It is defined at half the default trim threshold as a
  compromise heuristic to only attempt consolidation if it is likely
  to lead to trimming. However, it is not dynamically tunable, since
  consolidation reduces fragmentation surrounding loarge chunks even 
  if trimming is not used.
*/

#define FASTBIN_CONSOLIDATION_THRESHOLD  \
  ((unsigned long)(DEFAULT_TRIM_THRESHOLD) >> 1)

/*
  Since the lowest 2 bits in max_fast don't matter in size comparisons, 
  they are used as flags.
*/

/*
  ANYCHUNKS_BIT held in max_fast indicates that there may be any
  freed chunks at all. It is set true when entering a chunk into any
  bin.
*/

#define ANYCHUNKS_BIT        (1U)

#define have_anychunks(M)     (((M)->max_fast &  ANYCHUNKS_BIT))
#define set_anychunks(M)      ((M)->max_fast |=  ANYCHUNKS_BIT)
#define clear_anychunks(M)    ((M)->max_fast &= ~ANYCHUNKS_BIT)

/*
  FASTCHUNKS_BIT held in max_fast indicates that there are probably
  some fastbin chunks. It is set true on entering a chunk into any
  fastbin, and cleared only in malloc_consolidate.
*/

#define FASTCHUNKS_BIT        (2U)

#define have_fastchunks(M)   (((M)->max_fast &  FASTCHUNKS_BIT))
#define set_fastchunks(M)    ((M)->max_fast |=  (FASTCHUNKS_BIT|ANYCHUNKS_BIT))
#define clear_fastchunks(M)  ((M)->max_fast &= ~(FASTCHUNKS_BIT))

/* 
   Set value of max_fast. 
   Use impossibly small value if 0.
*/

#define set_max_fast(M, s) \
  (M)->max_fast = (((s) == 0)? SMALLBIN_WIDTH: request2size(s)) | \
  ((M)->max_fast &  (FASTCHUNKS_BIT|ANYCHUNKS_BIT))

#define get_max_fast(M) \
  ((M)->max_fast & ~(FASTCHUNKS_BIT | ANYCHUNKS_BIT))


/*
  morecore_properties is a status word holding dynamically discovered
  or controlled properties of the morecore function
*/

#define MORECORE_CONTIGUOUS_BIT  (1U)

#define contiguous(M) \
        (((M)->morecore_properties &  MORECORE_CONTIGUOUS_BIT))
#define noncontiguous(M) \
        (((M)->morecore_properties &  MORECORE_CONTIGUOUS_BIT) == 0)
#define set_contiguous(M) \
        ((M)->morecore_properties |=  MORECORE_CONTIGUOUS_BIT)
#define set_noncontiguous(M) \
        ((M)->morecore_properties &= ~MORECORE_CONTIGUOUS_BIT)


/*
   ----------- Internal state representation and initialization -----------
*/

struct malloc_state {

  /* The maximum chunk size to be eligible for fastbin */
  INTERNAL_SIZE_T  max_fast;   /* low 2 bits used as flags */

  /* Fastbins */
  mfastbinptr      fastbins[NFASTBINS];

  /* Base of the topmost chunk -- not otherwise kept in a bin */
  mchunkptr        top;

  /* The remainder from the most recent split of a small request */
  mchunkptr        last_remainder;

  /* Normal bins packed as described above */
  mchunkptr        bins[NBINS * 2];

  /* Bitmap of bins. Trailing zero map handles cases of largest binned size */
  unsigned int     binmap[BINMAPSIZE+1];

  /* Tunable parameters */
  CHUNK_SIZE_T     trim_threshold;
  INTERNAL_SIZE_T  top_pad;
  INTERNAL_SIZE_T  mmap_threshold;

  /* Memory map support */
  int              n_mmaps;
  int              n_mmaps_max;
  int              max_n_mmaps;

  /* Cache DEFAULT_SYS_PAGE_SIZE */
  unsigned int     pagesize;    

  /* Track properties of MORECORE */
  unsigned int     morecore_properties;

  /* Statistics */
  INTERNAL_SIZE_T  mmapped_mem;
  INTERNAL_SIZE_T  sbrked_mem;
  INTERNAL_SIZE_T  max_sbrked_mem;
  INTERNAL_SIZE_T  max_mmapped_mem;
  INTERNAL_SIZE_T  max_total_mem;
};

typedef struct malloc_state *mstate;

/* 
   There is exactly one instance of this struct in this malloc.
   If you are adapting this malloc in a way that does NOT use a static
   malloc_state, you MUST explicitly zero-fill it before using. This
   malloc relies on the property that malloc_state is initialized to
   all zeroes (as is true of C statics).
*/

static struct malloc_state av_;  /* never directly referenced */

/*
   All uses of av_ are via get_malloc_state().
   At most one "call" to get_malloc_state is made per invocation of
   the public versions of malloc and free, but other routines
   that in turn invoke malloc and/or free may call more then once.
*/

#define get_malloc_state() (&(av_))

/*
  Initialize a malloc_state struct.

  This is called only from within malloc_consolidate, which needs
  be called in the same contexts anyway.  It is never called directly
  outside of malloc_consolidate because some optimizing compilers try
  to inline it at all call points, which turns out not to be an
  optimization at all. (Inlining it in malloc_consolidate is fine though.)
*/

static void malloc_init_state(mstate av)
{
  int     i;
  mbinptr bin;
  
  /* Establish circular links for normal bins */
  for (i = 1; i < NBINS; ++i) { 
    bin = bin_at(av,i);
    bin->fd = bin->bk = bin;
  }

  av->top_pad        = DEFAULT_TOP_PAD;
  av->n_mmaps_max    = DEFAULT_MMAP_MAX;
  av->mmap_threshold = DEFAULT_MMAP_THRESHOLD;
  av->trim_threshold = DEFAULT_TRIM_THRESHOLD;

  set_contiguous(av);
  set_max_fast(av, DEFAULT_MXFAST);

  av->top            = initial_top(av);
  av->pagesize       = DEFAULT_SYS_PAGE_SIZE;
}

/* 
   Other internal utilities operating on mstates
*/

static void     malloc_consolidate(mstate);
static void** ialloc(size_t, size_t*, int, void**);


/* ----------- Routines dealing with system allocation -------------- */

/*
  sysmalloc handles malloc cases requiring more memory from the system.
  On entry, it is assumed that av->top does not have enough
  space to service request for nb bytes, thus requiring that av->top
  be extended or replaced.
*/

static void* sysmalloc(INTERNAL_SIZE_T nb, mstate av)
{
  mchunkptr       old_top;        /* incoming value of av->top */
  INTERNAL_SIZE_T old_size;       /* its size */
  char*           old_end;        /* its end address */

  long            size;           /* arg to first MORECORE or mmap call */
  char*           brk;            /* return value from MORECORE */

  long            correction;     /* arg to 2nd MORECORE call */
  char*           snd_brk;        /* 2nd return val */

  INTERNAL_SIZE_T front_misalign; /* unusable bytes at front of new space */
  INTERNAL_SIZE_T end_misalign;   /* partial page left at end of new space */
  char*           aligned_brk;    /* aligned offset into brk */

  mchunkptr       p;              /* the allocated/returned chunk */
  mchunkptr       remainder;      /* remainder from allocation */
  CHUNK_SIZE_T    remainder_size; /* its size */

  CHUNK_SIZE_T    sum;            /* for updating stats */

  size_t          pagemask  = av->pagesize - 1;

  /*
    If there is space available in fastbins, consolidate and retry
    malloc from scratch rather than getting memory from system.  This
    can occur only if nb is in smallbin range so we didn't consolidate
    upon entry to malloc. It is much easier to handle this case here
    than in malloc proper.
  */

  if (have_fastchunks(av)) {
    assert(in_smallbin_range(nb));
    malloc_consolidate(av);
    return malloc(nb - MALLOC_ALIGN_MASK);
  }

  /*
    If have mmap, and the request size meets the mmap threshold, and
    the system supports mmap, and there are few enough currently
    allocated mmapped regions, try to directly map this request
    rather than expanding top.
  */

  if ((CHUNK_SIZE_T)(nb) >= (CHUNK_SIZE_T)(av->mmap_threshold) &&
      (av->n_mmaps < av->n_mmaps_max)) {

    char* mm;             /* return value from mmap call*/

    /*
      Round up size to nearest page.  For mmapped chunks, the overhead
      is one SIZE_SZ unit larger than for normal chunks, because there
      is no following chunk whose prev_size field could be used.
    */
    size = (nb + SIZE_SZ + MALLOC_ALIGN_MASK + pagemask) & ~pagemask;

    /* Don't try if size wraps around 0 */
    if ((CHUNK_SIZE_T)(size) > (CHUNK_SIZE_T)(nb)) {

      mm = (char*)(MMAP(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE));
      if (mm != (char*)(MORECORE_FAILURE)) {
        /*
          The offset to the start of the mmapped region is stored
          in the prev_size field of the chunk. This allows us to adjust
          returned start address to meet alignment requirements here 
          and in memalign(), and still be able to compute proper
          address argument for later munmap in free() and realloc().
        */
        front_misalign = (INTERNAL_SIZE_T)chunk2mem(mm) & MALLOC_ALIGN_MASK;
        if (front_misalign > 0) {
          correction = MALLOC_ALIGNMENT - front_misalign;
          p = (mchunkptr)(mm + correction);
          p->prev_size = correction;
          set_head(p, (size - correction) |IS_MMAPPED);
        }
        else {
          p = (mchunkptr)mm;
          p->prev_size = 0;
          set_head(p, size|IS_MMAPPED);
        }

        /* update statistics */
        if (++av->n_mmaps > av->max_n_mmaps) 
          av->max_n_mmaps = av->n_mmaps;
        sum = av->mmapped_mem += size;
        if (sum > (CHUNK_SIZE_T)(av->max_mmapped_mem)) 
          av->max_mmapped_mem = sum;
        sum += av->sbrked_mem;
        if (sum > (CHUNK_SIZE_T)(av->max_total_mem)) 
          av->max_total_mem = sum;

        return chunk2mem(p);
      }
    }
  }

  /* Record incoming configuration of top */

  old_top  = av->top;
  old_size = chunksize(old_top);
  old_end  = (char*)(chunk_at_offset(old_top, old_size));

  brk = snd_brk = (char*)(MORECORE_FAILURE); 

  /* 
     If not the first time through, we require old_size to be
     at least MINSIZE and to have prev_inuse set.
  */

  assert((old_top == initial_top(av) && old_size == 0) || 
         ((CHUNK_SIZE_T) (old_size) >= MINSIZE &&
          prev_inuse(old_top)));

  /* Precondition: not enough current space to satisfy nb request */
  assert((CHUNK_SIZE_T)(old_size) < (CHUNK_SIZE_T)(nb + MINSIZE));

  /* Precondition: all fastbins are consolidated */
  assert(!have_fastchunks(av));


  /* Request enough space for nb + pad + overhead */

  size = nb + av->top_pad + MINSIZE;

  /*
    If contiguous, we can subtract out existing space that we hope to
    combine with new space. We add it back later only if
    we don't actually get contiguous space.
  */

  if (contiguous(av))
    size -= old_size;

  /*
    Round to a multiple of page size.
    If MORECORE is not contiguous, this ensures that we only call it
    with whole-page arguments.  And if MORECORE is contiguous and
    this is not first time through, this preserves page-alignment of
    previous calls. Otherwise, we correct to page-align below.
  */

  size = (size + pagemask) & ~pagemask;

  /*
    Don't try to call MORECORE if argument is so big as to appear
    negative. Note that since mmap takes size_t arg, it may succeed
    below even if we cannot call MORECORE.
  */

  if (size > 0) 
    brk = (char*)(MORECORE(size));

  /*
    If have mmap, try using it as a backup when MORECORE fails or
    cannot be used. This is worth doing on systems that have "holes" in
    address space, so sbrk cannot extend to give contiguous space, but
    space is available elsewhere.  Note that we ignore mmap max count
    and threshold limits, since the space will not be used as a
    segregated mmap region.
  */

  if (brk == (char*)(MORECORE_FAILURE)) {

    /* Cannot merge with old top, so add its size back in */
    if (contiguous(av))
      size = (size + old_size + pagemask) & ~pagemask;

    /* If we are relying on mmap as backup, then use larger units */
    if ((CHUNK_SIZE_T)(size) < (CHUNK_SIZE_T)(MMAP_AS_MORECORE_SIZE))
      size = MMAP_AS_MORECORE_SIZE;

    /* Don't try if size wraps around 0 */
    if ((CHUNK_SIZE_T)(size) > (CHUNK_SIZE_T)(nb)) {
      brk = (char*)(MMAP(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE));
      if (brk != (char*)(MORECORE_FAILURE)) {

        /* We do not need, and cannot use, another sbrk call to find end */
        snd_brk = brk + size;

        /*
           Record that we no longer have a contiguous sbrk region. 
           After the first time mmap is used as backup, we do not
           ever rely on contiguous space since this could incorrectly
           bridge regions.
        */
        set_noncontiguous(av);
      }
    }
  }

  if (brk != (char*)(MORECORE_FAILURE)) {
    av->sbrked_mem += size;

    /*
      If MORECORE extends previous space, we can likewise extend top size.
    */
    if (brk == old_end && snd_brk == (char*)(MORECORE_FAILURE)) {
      set_head(old_top, (size + old_size) | PREV_INUSE);
    }

    /*
      Otherwise, make adjustments:
      * If the first time through or noncontiguous, we need to call sbrk
        just to find out where the end of memory lies.

      * We need to ensure that all returned chunks from malloc will meet
        MALLOC_ALIGNMENT

      * If there was an intervening foreign sbrk, we need to adjust sbrk
        request size to account for fact that we will not be able to
        combine new space with existing space in old_top.

      * Almost all systems internally allocate whole pages at a time, in
        which case we might as well use the whole last page of request.
        So we allocate enough more memory to hit a page boundary now,
        which in turn causes future contiguous calls to page-align.
    */
    else {
      front_misalign = 0;
      end_misalign = 0;
      correction = 0;
      aligned_brk = brk;

      /*
        If MORECORE returns an address lower than we have seen before,
        we know it isn't really contiguous.  This and some subsequent
        checks help cope with non-conforming MORECORE functions and
        the presence of "foreign" calls to MORECORE from outside of
        malloc or by other threads.  We cannot guarantee to detect
        these in all cases, but cope with the ones we do detect.
      */
      if (contiguous(av) && old_size != 0 && brk < old_end) {
        set_noncontiguous(av);
      }

      /* handle contiguous cases */
      if (contiguous(av)) { 

        /* 
           We can tolerate forward non-contiguities here (usually due
           to foreign calls) but treat them as part of our space for
           stats reporting.
        */
        if (old_size != 0) 
          av->sbrked_mem += brk - old_end;
        
        /* Guarantee alignment of first new chunk made from this space */

        front_misalign = (INTERNAL_SIZE_T)chunk2mem(brk) & MALLOC_ALIGN_MASK;
        if (front_misalign > 0) {

          /*
            Skip over some bytes to arrive at an aligned position.
            We don't need to specially mark these wasted front bytes.
            They will never be accessed anyway because
            prev_inuse of av->top (and any chunk created from its start)
            is always true after initialization.
          */

          correction = MALLOC_ALIGNMENT - front_misalign;
          aligned_brk += correction;
        }
        
        /*
          If this isn't adjacent to existing space, then we will not
          be able to merge with old_top space, so must add to 2nd request.
        */
        
        correction += old_size;
        
        /* Extend the end address to hit a page boundary */
        end_misalign = (INTERNAL_SIZE_T)(brk + size + correction);
        correction += ((end_misalign + pagemask) & ~pagemask) - end_misalign;
        
        assert(correction >= 0);
        snd_brk = (char*)(MORECORE(correction));
        
        if (snd_brk == (char*)(MORECORE_FAILURE)) {
          /*
            If can't allocate correction, try to at least find out current
            brk.  It might be enough to proceed without failing.
          */
          correction = 0;
          snd_brk = (char*)(MORECORE(0));
        }
        else if (snd_brk < brk) {
          /*
            If the second call gives noncontiguous space even though
            it says it won't, the only course of action is to ignore
            results of second call, and conservatively estimate where
            the first call left us. Also set noncontiguous, so this
            won't happen again, leaving at most one hole.
            
            Note that this check is intrinsically incomplete.  Because
            MORECORE is allowed to give more space than we ask for,
            there is no reliable way to detect a noncontiguity
            producing a forward gap for the second call.
          */
          snd_brk = brk + size;
          correction = 0;
          set_noncontiguous(av);
        }

      }
      
      /* handle non-contiguous cases */
      else { 
        /* MORECORE/mmap must correctly align */
        assert(aligned_OK(chunk2mem(brk)));
        
        /* Find out current end of memory */
        if (snd_brk == (char*)(MORECORE_FAILURE)) {
          snd_brk = (char*)(MORECORE(0));
          av->sbrked_mem += snd_brk - brk - size;
        }
      }
      
      /* Adjust top based on results of second sbrk */
      if (snd_brk != (char*)(MORECORE_FAILURE)) {
        av->top = (mchunkptr)aligned_brk;
        set_head(av->top, (snd_brk - aligned_brk + correction) | PREV_INUSE);
        av->sbrked_mem += correction;
     
        /*
          If not the first time through, we either have a
          gap due to foreign sbrk or a non-contiguous region.  Insert a
          double fencepost at old_top to prevent consolidation with space
          we don't own. These fenceposts are artificial chunks that are
          marked as inuse and are in any case too small to use.  We need
          two to make sizes and alignments work out.
        */
   
        if (old_size != 0) {
          /* 
             Shrink old_top to insert fenceposts, keeping size a
             multiple of MALLOC_ALIGNMENT. We know there is at least
             enough space in old_top to do this.
          */
          old_size = (old_size - 3*SIZE_SZ) & ~MALLOC_ALIGN_MASK;
          set_head(old_top, old_size | PREV_INUSE);
          
          /*
            Note that the following assignments completely overwrite
            old_top when old_size was previously MINSIZE.  This is
            intentional. We need the fencepost, even if old_top otherwise gets
            lost.
          */
          chunk_at_offset(old_top, old_size          )->size =
            SIZE_SZ|PREV_INUSE;

          chunk_at_offset(old_top, old_size + SIZE_SZ)->size =
            SIZE_SZ|PREV_INUSE;

          /* 
             If possible, release the rest, suppressing trimming.
          */
          if (old_size >= MINSIZE) {
            INTERNAL_SIZE_T tt = av->trim_threshold;
            av->trim_threshold = (INTERNAL_SIZE_T)(-1);
            free(chunk2mem(old_top));
            av->trim_threshold = tt;
          }
        }
      }
    }
    
    /* Update statistics */
    sum = av->sbrked_mem;
    if (sum > (CHUNK_SIZE_T)(av->max_sbrked_mem))
      av->max_sbrked_mem = sum;
    
    sum += av->mmapped_mem;
    if (sum > (CHUNK_SIZE_T)(av->max_total_mem))
      av->max_total_mem = sum;
    p = av->top;
    size = chunksize(p);
    
    /* check that one of the above allocation paths succeeded */
    if ((CHUNK_SIZE_T)(size) >= (CHUNK_SIZE_T)(nb + MINSIZE)) {
      remainder_size = size - nb;
      remainder = chunk_at_offset(p, nb);
      av->top = remainder;
      set_head(p, nb | PREV_INUSE);
      set_head(remainder, remainder_size | PREV_INUSE);
      return chunk2mem(p);
    }

  }

  /* catch all failure paths */
  errno = ENOMEM;
  return 0;
}




/*
  systrim is an inverse of sorts to sysmalloc.  It gives memory back
  to the system (via negative arguments to sbrk) if there is unused
  memory at the `high' end of the malloc pool. It is called
  automatically by free() when top space exceeds the trim
  threshold. It is also called by the public malloc_trim routine.  It
  returns 1 if it actually released any memory, else 0.
*/

static int systrim(size_t pad, mstate av)
{
  long  top_size;        /* Amount of top-most memory */
  long  extra;           /* Amount to release */
  long  released;        /* Amount actually released */
  char* current_brk;     /* address returned by pre-check sbrk call */
  char* new_brk;         /* address returned by post-check sbrk call */
  size_t pagesz;

  pagesz = av->pagesize;
  top_size = chunksize(av->top);
  
  /* Release in pagesize units, keeping at least one page */
  extra = ((top_size - pad - MINSIZE + (pagesz-1)) / pagesz - 1) * pagesz;
  
  if (extra > 0) {
    
    /*
      Only proceed if end of memory is where we last set it.
      This avoids problems if there were foreign sbrk calls.
    */
    current_brk = (char*)(MORECORE(0));
    if (current_brk == (char*)(av->top) + top_size) {
      
      /*
        Attempt to release memory. We ignore MORECORE return value,
        and instead call again to find out where new end of memory is.
        This avoids problems if first call releases less than we asked,
        of if failure somehow altered brk value. (We could still
        encounter problems if it altered brk in some very bad way,
        but the only thing we can do is adjust anyway, which will cause
        some downstream failure.)
      */
      
      MORECORE(-extra);
      new_brk = (char*)(MORECORE(0));
      
      if (new_brk != (char*)MORECORE_FAILURE) {
        released = (long)(current_brk - new_brk);
        
        if (released != 0) {
          /* Success. Adjust top. */
          av->sbrked_mem -= released;
          set_head(av->top, (top_size - released) | PREV_INUSE);
          return 1;
        }
      }
    }
  }
  return 0;
}

/*
  ------------------------------ malloc ------------------------------
*/


void* malloc(size_t bytes)
{
  mstate av = get_malloc_state();

  INTERNAL_SIZE_T nb;               /* normalized request size */
  unsigned int    idx;              /* associated bin index */
  mbinptr         bin;              /* associated bin */
  mfastbinptr*    fb;               /* associated fastbin */

  mchunkptr       victim;           /* inspected/selected chunk */
  INTERNAL_SIZE_T size;             /* its size */
  int             victim_index;     /* its bin index */

  mchunkptr       remainder;        /* remainder from a split */
  CHUNK_SIZE_T    remainder_size;   /* its size */

  unsigned int    block;            /* bit map traverser */
  unsigned int    bit;              /* bit map traverser */
  unsigned int    map;              /* current word of binmap */

  mchunkptr       fwd;              /* misc temp for linking */
  mchunkptr       bck;              /* misc temp for linking */

  /*
    Convert request size to internal form by adding SIZE_SZ bytes
    overhead plus possibly more to obtain necessary alignment and/or
    to obtain a size of at least MINSIZE, the smallest allocatable
    size. Also, checked_request2size traps (returning 0) request sizes
    that are so large that they wrap around zero when padded and
    aligned.
  */

  checked_request2size(bytes, nb);

  /*
    Bypass search if no frees yet
   */
  if (!have_anychunks(av)) {
    if (av->max_fast == 0) /* initialization check */
      malloc_consolidate(av);
    goto use_top;
  }

  /*
    If the size qualifies as a fastbin, first check corresponding bin.
  */

  if ((CHUNK_SIZE_T)(nb) <= (CHUNK_SIZE_T)(av->max_fast)) { 
    fb = &(av->fastbins[(fastbin_index(nb))]);
    if ( (victim = *fb) != 0) {
      *fb = victim->fd;
      return chunk2mem(victim);
    }
  }

  /*
    If a small request, check regular bin.  Since these "smallbins"
    hold one size each, no searching within bins is necessary.
    (For a large request, we need to wait until unsorted chunks are
    processed to find best fit. But for small ones, fits are exact
    anyway, so we can check now, which is faster.)
  */

  if (in_smallbin_range(nb)) {
    idx = smallbin_index(nb);
    bin = bin_at(av,idx);

    if ( (victim = last(bin)) != bin) {
      bck = victim->bk;
      set_inuse_bit_at_offset(victim, nb);
      bin->bk = bck;
      bck->fd = bin;
      return chunk2mem(victim);
    }
  }

  /* 
     If this is a large request, consolidate fastbins before continuing.
     While it might look excessive to kill all fastbins before
     even seeing if there is space available, this avoids
     fragmentation problems normally associated with fastbins.
     Also, in practice, programs tend to have runs of either small or
     large requests, but less often mixtures, so consolidation is not 
     invoked all that often in most programs. And the programs that
     it is called frequently in otherwise tend to fragment.
  */

  else {
    idx = largebin_index(nb);
    if (have_fastchunks(av)) 
      malloc_consolidate(av);
  }

  /*
    Process recently freed or remaindered chunks, taking one only if
    it is exact fit, or, if this a small request, the chunk is remainder from
    the most recent non-exact fit.  Place other traversed chunks in
    bins.  Note that this step is the only place in any routine where
    chunks are placed in bins.
  */
    
  while ( (victim = unsorted_chunks(av)->bk) != unsorted_chunks(av)) {
    bck = victim->bk;
    size = chunksize(victim);
    
    /* 
       If a small request, try to use last remainder if it is the
       only chunk in unsorted bin.  This helps promote locality for
       runs of consecutive small requests. This is the only
       exception to best-fit, and applies only when there is
       no exact fit for a small chunk.
    */
    
    if (in_smallbin_range(nb) && 
        bck == unsorted_chunks(av) &&
        victim == av->last_remainder &&
        (CHUNK_SIZE_T)(size) > (CHUNK_SIZE_T)(nb + MINSIZE)) {
      
      /* split and reattach remainder */
      remainder_size = size - nb;
      remainder = chunk_at_offset(victim, nb);
      unsorted_chunks(av)->bk = unsorted_chunks(av)->fd = remainder;
      av->last_remainder = remainder; 
      remainder->bk = remainder->fd = unsorted_chunks(av);
      
      set_head(victim, nb | PREV_INUSE);
      set_head(remainder, remainder_size | PREV_INUSE);
      set_foot(remainder, remainder_size);
      
      return chunk2mem(victim);
    }
    
    /* remove from unsorted list */
    unsorted_chunks(av)->bk = bck;
    bck->fd = unsorted_chunks(av);
    
    /* Take now instead of binning if exact fit */
    
    if (size == nb) {
      set_inuse_bit_at_offset(victim, size);
      return chunk2mem(victim);
    }
    
    /* place chunk in bin */
    
    if (in_smallbin_range(size)) {
      victim_index = smallbin_index(size);
      bck = bin_at(av, victim_index);
      fwd = bck->fd;
    }
    else {
      victim_index = largebin_index(size);
      bck = bin_at(av, victim_index);
      fwd = bck->fd;
      
      if (fwd != bck) {
        /* if smaller than smallest, place first */
        if ((CHUNK_SIZE_T)(size) < (CHUNK_SIZE_T)(bck->bk->size)) {
          fwd = bck;
          bck = bck->bk;
        }
        else if ((CHUNK_SIZE_T)(size) >= 
                 (CHUNK_SIZE_T)(FIRST_SORTED_BIN_SIZE)) {
          
          /* maintain large bins in sorted order */
          size |= PREV_INUSE; /* Or with inuse bit to speed comparisons */
          while ((CHUNK_SIZE_T)(size) < (CHUNK_SIZE_T)(fwd->size)) 
            fwd = fwd->fd;
          bck = fwd->bk;
        }
      }
    }
      
    mark_bin(av, victim_index);
    victim->bk = bck;
    victim->fd = fwd;
    fwd->bk = victim;
    bck->fd = victim;
  }
  
  /*
    If a large request, scan through the chunks of current bin to
    find one that fits.  (This will be the smallest that fits unless
    FIRST_SORTED_BIN_SIZE has been changed from default.)  This is
    the only step where an unbounded number of chunks might be
    scanned without doing anything useful with them. However the
    lists tend to be short.
  */
  
  if (!in_smallbin_range(nb)) {
    bin = bin_at(av, idx);
    
    for (victim = last(bin); victim != bin; victim = victim->bk) {
      size = chunksize(victim);
      
      if ((CHUNK_SIZE_T)(size) >= (CHUNK_SIZE_T)(nb)) {
        remainder_size = size - nb;
        unlink(victim, bck, fwd);
        
        /* Exhaust */
        if (remainder_size < MINSIZE)  {
          set_inuse_bit_at_offset(victim, size);
          return chunk2mem(victim);
        }
        /* Split */
        else {
          remainder = chunk_at_offset(victim, nb);
          unsorted_chunks(av)->bk = unsorted_chunks(av)->fd = remainder;
          remainder->bk = remainder->fd = unsorted_chunks(av);
          set_head(victim, nb | PREV_INUSE);
          set_head(remainder, remainder_size | PREV_INUSE);
          set_foot(remainder, remainder_size);
          return chunk2mem(victim);
        } 
      }
    }    
  }

  /*
    Search for a chunk by scanning bins, starting with next largest
    bin. This search is strictly by best-fit; i.e., the smallest
    (with ties going to approximately the least recently used) chunk
    that fits is selected.
    
    The bitmap avoids needing to check that most blocks are nonempty.
  */
    
  ++idx;
  bin = bin_at(av,idx);
  block = idx2block(idx);
  map = av->binmap[block];
  bit = idx2bit(idx);
  
  for (;;) {
    
    /* Skip rest of block if there are no more set bits in this block.  */
    if (bit > map || bit == 0) {
      do {
        if (++block >= BINMAPSIZE)  /* out of bins */
          goto use_top;
      } while ( (map = av->binmap[block]) == 0);
      
      bin = bin_at(av, (block << BINMAPSHIFT));
      bit = 1;
    }
    
    /* Advance to bin with set bit. There must be one. */
    while ((bit & map) == 0) {
      bin = next_bin(bin);
      bit <<= 1;
      assert(bit != 0);
    }
    
    /* Inspect the bin. It is likely to be non-empty */
    victim = last(bin);
    
    /*  If a false alarm (empty bin), clear the bit. */
    if (victim == bin) {
      av->binmap[block] = map &= ~bit; /* Write through */
      bin = next_bin(bin);
      bit <<= 1;
    }
    
    else {
      size = chunksize(victim);
      
      /*  We know the first chunk in this bin is big enough to use. */
      assert((CHUNK_SIZE_T)(size) >= (CHUNK_SIZE_T)(nb));
      
      remainder_size = size - nb;
      
      /* unlink */
      bck = victim->bk;
      bin->bk = bck;
      bck->fd = bin;
      
      /* Exhaust */
      if (remainder_size < MINSIZE) {
        set_inuse_bit_at_offset(victim, size);
        return chunk2mem(victim);
      }
      
      /* Split */
      else {
        remainder = chunk_at_offset(victim, nb);
        
        unsorted_chunks(av)->bk = unsorted_chunks(av)->fd = remainder;
        remainder->bk = remainder->fd = unsorted_chunks(av);
        /* advertise as last remainder */
        if (in_smallbin_range(nb)) 
          av->last_remainder = remainder; 
        
        set_head(victim, nb | PREV_INUSE);
        set_head(remainder, remainder_size | PREV_INUSE);
        set_foot(remainder, remainder_size);
        return chunk2mem(victim);
      }
    }
  }

  use_top:    
  /*
    If large enough, split off the chunk bordering the end of memory
    (held in av->top). Note that this is in accord with the best-fit
    search rule.  In effect, av->top is treated as larger (and thus
    less well fitting) than any other available chunk since it can
    be extended to be as large as necessary (up to system
    limitations).
    
    We require that av->top always exists (i.e., has size >=
    MINSIZE) after initialization, so if it would otherwise be
    exhuasted by current request, it is replenished. (The main
    reason for ensuring it exists is that we may need MINSIZE space
    to put in fenceposts in sysmalloc.)
  */
  
  victim = av->top;
  size = chunksize(victim);
  
  if ((CHUNK_SIZE_T)(size) >= (CHUNK_SIZE_T)(nb + MINSIZE)) {
    remainder_size = size - nb;
    remainder = chunk_at_offset(victim, nb);
    av->top = remainder;
    set_head(victim, nb | PREV_INUSE);
    set_head(remainder, remainder_size | PREV_INUSE);
    return chunk2mem(victim);
  }
  
  /* 
     If no space in top, relay to handle system-dependent cases 
  */
  return sysmalloc(nb, av);    
}

/*
  ------------------------------ free ------------------------------
*/

void free(void* mem)
{
  mstate av = get_malloc_state();

  mchunkptr       p;           /* chunk corresponding to mem */
  INTERNAL_SIZE_T size;        /* its size */
  mfastbinptr*    fb;          /* associated fastbin */
  mchunkptr       nextchunk;   /* next contiguous chunk */
  INTERNAL_SIZE_T nextsize;    /* its size */
  int             nextinuse;   /* true if nextchunk is used */
  INTERNAL_SIZE_T prevsize;    /* size of previous contiguous chunk */
  mchunkptr       bck;         /* misc temp for linking */
  mchunkptr       fwd;         /* misc temp for linking */

  /* free(0) has no effect */
  if (mem != 0) {
    p = mem2chunk(mem);
    size = chunksize(p);

    /*
      If eligible, place chunk on a fastbin so it can be found
      and used quickly in malloc.
    */

    if ((CHUNK_SIZE_T)(size) <= (CHUNK_SIZE_T)(av->max_fast)) {
      set_fastchunks(av);
      fb = &(av->fastbins[fastbin_index(size)]);
      p->fd = *fb;
      *fb = p;
    }

    /*
       Consolidate other non-mmapped chunks as they arrive.
    */

    else if (!chunk_is_mmapped(p)) {
      set_anychunks(av);

      nextchunk = chunk_at_offset(p, size);
      nextsize = chunksize(nextchunk);

      /* consolidate backward */
      if (!prev_inuse(p)) {
        prevsize = p->prev_size;
        size += prevsize;
        p = chunk_at_offset(p, -((long) prevsize));
        unlink(p, bck, fwd);
      }

      if (nextchunk != av->top) {
        /* get and clear inuse bit */
        nextinuse = inuse_bit_at_offset(nextchunk, nextsize);
        set_head(nextchunk, nextsize);

        /* consolidate forward */
        if (!nextinuse) {
          unlink(nextchunk, bck, fwd);
          size += nextsize;
        }

        /*
          Place the chunk in unsorted chunk list. Chunks are
          not placed into regular bins until after they have
          been given one chance to be used in malloc.
        */

        bck = unsorted_chunks(av);
        fwd = bck->fd;
        p->bk = bck;
        p->fd = fwd;
        bck->fd = p;
        fwd->bk = p;

        set_head(p, size | PREV_INUSE);
        set_foot(p, size);
      }

      /*
         If the chunk borders the current high end of memory,
         consolidate into top
      */

      else {
        size += nextsize;
        set_head(p, size | PREV_INUSE);
        av->top = p;
      }

      /*
        If freeing a large space, consolidate possibly-surrounding
        chunks. Then, if the total unused topmost memory exceeds trim
        threshold, ask malloc_trim to reduce top.

        Unless max_fast is 0, we don't know if there are fastbins
        bordering top, so we cannot tell for sure whether threshold
        has been reached unless fastbins are consolidated.  But we
        don't want to consolidate on each free.  As a compromise,
        consolidation is performed if FASTBIN_CONSOLIDATION_THRESHOLD
        is reached.
      */

      if ((CHUNK_SIZE_T)(size) >= FASTBIN_CONSOLIDATION_THRESHOLD) { 
        if (have_fastchunks(av)) 
          malloc_consolidate(av);

        if ((CHUNK_SIZE_T)(chunksize(av->top)) >= 
            (CHUNK_SIZE_T)(av->trim_threshold))
          systrim(av->top_pad, av);
      }

    }
    /*
      If the chunk was allocated via mmap, release via munmap()
      Note that if HAVE_MMAP is false but chunk_is_mmapped is
      true, then user must have overwritten memory.
    */
    else {
      INTERNAL_SIZE_T offset = p->prev_size;
      av->n_mmaps--;
      av->mmapped_mem -= (size + offset);
      munmap((char*)p - offset, size + offset);
    }
  }
}

/*
  ------------------------- malloc_consolidate -------------------------

  malloc_consolidate is a specialized version of free() that tears
  down chunks held in fastbins.  Free itself cannot be used for this
  purpose since, among other things, it might place chunks back onto
  fastbins.  So, instead, we need to use a minor variant of the same
  code.
  
  Also, because this routine needs to be called the first time through
  malloc anyway, it turns out to be the perfect place to trigger
  initialization code.
*/

static void malloc_consolidate(mstate av)
{
  mfastbinptr*    fb;                 /* current fastbin being consolidated */
  mfastbinptr*    maxfb;              /* last fastbin (for loop control) */
  mchunkptr       p;                  /* current chunk being consolidated */
  mchunkptr       nextp;              /* next chunk to consolidate */
  mchunkptr       unsorted_bin;       /* bin header */
  mchunkptr       first_unsorted;     /* chunk to link to */

  /* These have same use as in free() */
  mchunkptr       nextchunk;
  INTERNAL_SIZE_T size;
  INTERNAL_SIZE_T nextsize;
  INTERNAL_SIZE_T prevsize;
  int             nextinuse;
  mchunkptr       bck;
  mchunkptr       fwd;

  /*
    If max_fast is 0, we know that av hasn't
    yet been initialized, in which case do so below
  */

  if (av->max_fast != 0) {
    clear_fastchunks(av);

    unsorted_bin = unsorted_chunks(av);

    /*
      Remove each chunk from fast bin and consolidate it, placing it
      then in unsorted bin. Among other reasons for doing this,
      placing in unsorted bin avoids needing to calculate actual bins
      until malloc is sure that chunks aren't immediately going to be
      reused anyway.
    */
    
    maxfb = &(av->fastbins[fastbin_index(av->max_fast)]);
    fb = &(av->fastbins[0]);
    do {
      if ( (p = *fb) != 0) {
        *fb = 0;
        
        do {
          nextp = p->fd;
          
          /* Slightly streamlined version of consolidation code in free() */
          size = p->size & ~PREV_INUSE;
          nextchunk = chunk_at_offset(p, size);
          nextsize = chunksize(nextchunk);
          
          if (!prev_inuse(p)) {
            prevsize = p->prev_size;
            size += prevsize;
            p = chunk_at_offset(p, -((long) prevsize));
            unlink(p, bck, fwd);
          }
          
          if (nextchunk != av->top) {
            nextinuse = inuse_bit_at_offset(nextchunk, nextsize);
            set_head(nextchunk, nextsize);
            
            if (!nextinuse) {
              size += nextsize;
              unlink(nextchunk, bck, fwd);
            }
            
            first_unsorted = unsorted_bin->fd;
            unsorted_bin->fd = p;
            first_unsorted->bk = p;
            
            set_head(p, size | PREV_INUSE);
            p->bk = unsorted_bin;
            p->fd = first_unsorted;
            set_foot(p, size);
          }
          
          else {
            size += nextsize;
            set_head(p, size | PREV_INUSE);
            av->top = p;
          }
          
        } while ( (p = nextp) != 0);
        
      }
    } while (fb++ != maxfb);
  }
  else {
    malloc_init_state(av);
  }
}

/*
  ------------------------------ realloc ------------------------------
*/

void* realloc(void* oldmem, size_t bytes)
{
  mstate av = get_malloc_state();

  INTERNAL_SIZE_T  nb;              /* padded request size */

  mchunkptr        oldp;            /* chunk corresponding to oldmem */
  INTERNAL_SIZE_T  oldsize;         /* its size */

  mchunkptr        newp;            /* chunk to return */
  INTERNAL_SIZE_T  newsize;         /* its size */
  void*          newmem;          /* corresponding user mem */

  mchunkptr        next;            /* next contiguous chunk after oldp */

  mchunkptr        remainder;       /* extra space at end of newp */
  CHUNK_SIZE_T     remainder_size;  /* its size */

  mchunkptr        bck;             /* misc temp for linking */
  mchunkptr        fwd;             /* misc temp for linking */

  CHUNK_SIZE_T     copysize;        /* bytes to copy */
  unsigned int     ncopies;         /* INTERNAL_SIZE_T words to copy */
  INTERNAL_SIZE_T* s;               /* copy source */ 
  INTERNAL_SIZE_T* d;               /* copy destination */

  /* realloc of null is supposed to be same as malloc */
  if (oldmem == 0) return malloc(bytes);

  checked_request2size(bytes, nb);

  oldp    = mem2chunk(oldmem);
  oldsize = chunksize(oldp);

  if (!chunk_is_mmapped(oldp)) {

    if ((CHUNK_SIZE_T)(oldsize) >= (CHUNK_SIZE_T)(nb)) {
      /* already big enough; split below */
      newp = oldp;
      newsize = oldsize;
    }

    else {
      next = chunk_at_offset(oldp, oldsize);

      /* Try to expand forward into top */
      if (next == av->top &&
          (CHUNK_SIZE_T)(newsize = oldsize + chunksize(next)) >=
          (CHUNK_SIZE_T)(nb + MINSIZE)) {
        set_head_size(oldp, nb);
        av->top = chunk_at_offset(oldp, nb);
        set_head(av->top, (newsize - nb) | PREV_INUSE);
        return chunk2mem(oldp);
      }
      
      /* Try to expand forward into next chunk;  split off remainder below */
      else if (next != av->top && 
               !inuse(next) &&
               (CHUNK_SIZE_T)(newsize = oldsize + chunksize(next)) >=
               (CHUNK_SIZE_T)(nb)) {
        newp = oldp;
        unlink(next, bck, fwd);
      }

      /* allocate, copy, free */
      else {
        newmem = malloc(nb - MALLOC_ALIGN_MASK);
        if (newmem == 0)
          return 0; /* propagate failure */
      
        newp = mem2chunk(newmem);
        newsize = chunksize(newp);
        
        /*
          Avoid copy if newp is next chunk after oldp.
        */
        if (newp == next) {
          newsize += oldsize;
          newp = oldp;
        }
        else {
          /*
            Unroll copy of <= 36 bytes (72 if 8byte sizes)
            We know that contents have an odd number of
            INTERNAL_SIZE_T-sized words; minimally 3.
          */
          
          copysize = oldsize - SIZE_SZ;
          s = (INTERNAL_SIZE_T*)(oldmem);
          d = (INTERNAL_SIZE_T*)(newmem);
          ncopies = copysize / sizeof(INTERNAL_SIZE_T);
          assert(ncopies >= 3);
          
          if (ncopies > 9)
            memcpy(d, s, copysize);
          
          else {
            *(d+0) = *(s+0);
            *(d+1) = *(s+1);
            *(d+2) = *(s+2);
            if (ncopies > 4) {
              *(d+3) = *(s+3);
              *(d+4) = *(s+4);
              if (ncopies > 6) {
                *(d+5) = *(s+5);
                *(d+6) = *(s+6);
                if (ncopies > 8) {
                  *(d+7) = *(s+7);
                  *(d+8) = *(s+8);
                }
              }
            }
          }
          
          free(oldmem);
          return chunk2mem(newp);
        }
      }
    }

    /* If possible, free extra space in old or extended chunk */

    assert((CHUNK_SIZE_T)(newsize) >= (CHUNK_SIZE_T)(nb));

    remainder_size = newsize - nb;

    if (remainder_size < MINSIZE) { /* not enough extra to split off */
      set_head_size(newp, newsize);
      set_inuse_bit_at_offset(newp, newsize);
    }
    else { /* split remainder */
      remainder = chunk_at_offset(newp, nb);
      set_head_size(newp, nb);
      set_head(remainder, remainder_size | PREV_INUSE);
      /* Mark remainder as inuse so free() won't complain */
      set_inuse_bit_at_offset(remainder, remainder_size);
      free(chunk2mem(remainder)); 
    }

    return chunk2mem(newp);
  }

  /*
    Handle mmap cases
  */

  else {
    /* Note the extra SIZE_SZ overhead. */
    if ((CHUNK_SIZE_T)(oldsize) >= (CHUNK_SIZE_T)(nb + SIZE_SZ)) 
      newmem = oldmem; /* do nothing */
    else {
      /* Must alloc, copy, free. */
      newmem = malloc(nb - MALLOC_ALIGN_MASK);
      if (newmem != 0) {
        memcpy(newmem, oldmem, oldsize - 2*SIZE_SZ);
        free(oldmem);
      }
    }
    return newmem;
  }
}

void* memalign(size_t alignment, size_t bytes)
{
  INTERNAL_SIZE_T nb;             /* padded  request size */
  char*           m;              /* memory returned by malloc call */
  mchunkptr       p;              /* corresponding chunk */
  char*           brk;            /* alignment point within p */
  mchunkptr       newp;           /* chunk to return */
  INTERNAL_SIZE_T newsize;        /* its size */
  INTERNAL_SIZE_T leadsize;       /* leading space before alignment point */
  mchunkptr       remainder;      /* spare room at end to split off */
  CHUNK_SIZE_T    remainder_size; /* its size */
  INTERNAL_SIZE_T size;

  /* If need less alignment than we give anyway, just relay to malloc */

  if (alignment <= MALLOC_ALIGNMENT) return malloc(bytes);

  /* Otherwise, ensure that it is at least a minimum chunk size */

  if (alignment <  MINSIZE) alignment = MINSIZE;

  /* Make sure alignment is power of 2 (in case MINSIZE is not).  */
  if ((alignment & (alignment - 1)) != 0) {
    size_t a = MALLOC_ALIGNMENT * 2;
    while ((CHUNK_SIZE_T)a < (CHUNK_SIZE_T)alignment) a <<= 1;
    alignment = a;
  }

  checked_request2size(bytes, nb);

  /*
    Strategy: find a spot within that chunk that meets the alignment
    request, and then possibly free the leading and trailing space.
  */


  /* Call malloc with worst case padding to hit alignment. */

  m  = (char*)(malloc(nb + alignment + MINSIZE));

  if (m == 0) return 0; /* propagate failure */

  p = mem2chunk(m);

  if ((((PTR_UINT)(m)) % alignment) != 0) { /* misaligned */

    /*
      Find an aligned spot inside chunk.  Since we need to give back
      leading space in a chunk of at least MINSIZE, if the first
      calculation places us at a spot with less than MINSIZE leader,
      we can move to the next aligned spot -- we've allocated enough
      total room so that this is always possible.
    */

    brk = (char*)mem2chunk((PTR_UINT)(((PTR_UINT)(m + alignment - 1)) &
                           -((signed long) alignment)));
    if ((CHUNK_SIZE_T)(brk - (char*)(p)) < MINSIZE)
      brk += alignment;

    newp = (mchunkptr)brk;
    leadsize = brk - (char*)(p);
    newsize = chunksize(p) - leadsize;

    /* For mmapped chunks, just adjust offset */
    if (chunk_is_mmapped(p)) {
      newp->prev_size = p->prev_size + leadsize;
      set_head(newp, newsize|IS_MMAPPED);
      return chunk2mem(newp);
    }

    /* Otherwise, give back leader, use the rest */
    set_head(newp, newsize | PREV_INUSE);
    set_inuse_bit_at_offset(newp, newsize);
    set_head_size(p, leadsize);
    free(chunk2mem(p));
    p = newp;

    assert (newsize >= nb &&
            (((PTR_UINT)(chunk2mem(p))) % alignment) == 0);
  }

  /* Also give back spare room at the end */
  if (!chunk_is_mmapped(p)) {
    size = chunksize(p);
    if ((CHUNK_SIZE_T)(size) > (CHUNK_SIZE_T)(nb + MINSIZE)) {
      remainder_size = size - nb;
      remainder = chunk_at_offset(p, nb);
      set_head(remainder, remainder_size | PREV_INUSE);
      set_head_size(p, nb);
      free(chunk2mem(remainder));
    }
  }

  return chunk2mem(p);
}

void* calloc(size_t n_elements, size_t elem_size)
{
  mchunkptr p;
  CHUNK_SIZE_T  clearsize;
  CHUNK_SIZE_T  nclears;
  INTERNAL_SIZE_T* d;

  void* mem = malloc(n_elements * elem_size);

  if (mem != 0) {
    p = mem2chunk(mem);

    if (!chunk_is_mmapped(p))
    {  
      /*
        Unroll clear of <= 36 bytes (72 if 8byte sizes)
        We know that contents have an odd number of
        INTERNAL_SIZE_T-sized words; minimally 3.
      */

      d = (INTERNAL_SIZE_T*)mem;
      clearsize = chunksize(p) - SIZE_SZ;
      nclears = clearsize / sizeof(INTERNAL_SIZE_T);
      assert(nclears >= 3);

      if (nclears > 9)
        memset(d, 0, clearsize);

      else {
        *(d+0) = 0;
        *(d+1) = 0;
        *(d+2) = 0;
        if (nclears > 4) {
          *(d+3) = 0;
          *(d+4) = 0;
          if (nclears > 6) {
            *(d+5) = 0;
            *(d+6) = 0;
            if (nclears > 8) {
              *(d+7) = 0;
              *(d+8) = 0;
            }
          }
        }
      }
    }
  }
  return mem;
}

void cfree(void *mem)
{
  free(mem);
}

void** independent_calloc(size_t n_elements, size_t elem_size, void* chunks[])
{
  size_t sz = elem_size; /* serves as 1-element array */
  /* opts arg of 3 means all elements are same size, and should be cleared */
  return ialloc(n_elements, &sz, 3, chunks);
}

void** independent_comalloc(size_t n_elements, size_t sizes[], void* chunks[])
{
  return ialloc(n_elements, sizes, 0, chunks);
}


/*
  ------------------------------ ialloc ------------------------------
  ialloc provides common support for independent_X routines, handling all of
  the combinations that can result.

  The opts arg has:
    bit 0 set if all elements are same size (using sizes[0])
    bit 1 set if elements should be zeroed
*/

static void** ialloc(size_t n_elements, 
                       size_t* sizes,  
                       int opts,
                       void* chunks[])
{
  mstate av = get_malloc_state();
  INTERNAL_SIZE_T element_size;   /* chunksize of each element, if all same */
  INTERNAL_SIZE_T contents_size;  /* total size of elements */
  INTERNAL_SIZE_T array_size;     /* request size of pointer array */
  void*         mem;            /* malloced aggregate space */
  mchunkptr       p;              /* corresponding chunk */
  INTERNAL_SIZE_T remainder_size; /* remaining bytes while splitting */
  void**        marray;         /* either "chunks" or malloced ptr array */
  mchunkptr       array_chunk;    /* chunk for malloced ptr array */
  int             mmx;            /* to disable mmap */
  INTERNAL_SIZE_T size;           
  size_t          i;

  /* Ensure initialization */
  if (av->max_fast == 0) malloc_consolidate(av);

  /* compute array length, if needed */
  if (chunks != 0) {
    if (n_elements == 0)
      return chunks; /* nothing to do */
    marray = chunks;
    array_size = 0;
  }
  else {
    /* if empty req, must still return chunk representing empty array */
    if (n_elements == 0) 
      return (void**) malloc(0);
    marray = 0;
    array_size = request2size(n_elements * (sizeof(void*)));
  }

  /* compute total element size */
  if (opts & 0x1) { /* all-same-size */
    element_size = request2size(*sizes);
    contents_size = n_elements * element_size;
  }
  else { /* add up all the sizes */
    element_size = 0;
    contents_size = 0;
    for (i = 0; i != n_elements; ++i) 
      contents_size += request2size(sizes[i]);     
  }

  /* subtract out alignment bytes from total to minimize overallocation */
  size = contents_size + array_size - MALLOC_ALIGN_MASK;
  
  /* 
     Allocate the aggregate chunk.
     But first disable mmap so malloc won't use it, since
     we would not be able to later free/realloc space internal
     to a segregated mmap region.
 */
  mmx = av->n_mmaps_max;   /* disable mmap */
  av->n_mmaps_max = 0;
  mem = malloc(size);
  av->n_mmaps_max = mmx;   /* reset mmap */
  if (mem == 0) 
    return 0;

  p = mem2chunk(mem);
  assert(!chunk_is_mmapped(p)); 
  remainder_size = chunksize(p);

  if (opts & 0x2) {       /* optionally clear the elements */
    memset(mem, 0, remainder_size - SIZE_SZ - array_size);
  }

  /* If not provided, allocate the pointer array as final part of chunk */
  if (marray == 0) {
    array_chunk = chunk_at_offset(p, contents_size);
    marray = (void**) (chunk2mem(array_chunk));
    set_head(array_chunk, (remainder_size - contents_size) | PREV_INUSE);
    remainder_size = contents_size;
  }

  /* split out elements */
  for (i = 0; ; ++i) {
    marray[i] = chunk2mem(p);
    if (i != n_elements-1) {
      if (element_size != 0) 
        size = element_size;
      else
        size = request2size(sizes[i]);          
      remainder_size -= size;
      set_head(p, size | PREV_INUSE);
      p = chunk_at_offset(p, size);
    }
    else { /* the final element absorbs any overallocation slop */
      set_head(p, remainder_size | PREV_INUSE);
      break;
    }
  }

  return marray;
}

void* valloc(size_t bytes)
{
  /* Ensure initialization */
  mstate av = get_malloc_state();
  if (av->max_fast == 0) malloc_consolidate(av);
  return memalign(av->pagesize, bytes);
}

void* pvalloc(size_t bytes)
{
  mstate av = get_malloc_state();
  size_t pagesz;

  /* Ensure initialization */
  if (av->max_fast == 0) malloc_consolidate(av);
  pagesz = av->pagesize;
  return memalign(pagesz, (bytes + pagesz - 1) & ~(pagesz - 1));
}

int malloc_trim(size_t pad)
{
  mstate av = get_malloc_state();
  /* Ensure initialization/consolidation */
  malloc_consolidate(av);

  return systrim(pad, av);
}

size_t malloc_usable_size(void* mem)
{
  mchunkptr p;
  if (mem != 0) {
    p = mem2chunk(mem);
    if (chunk_is_mmapped(p))
      return chunksize(p) - 2*SIZE_SZ;
    else if (inuse(p))
      return chunksize(p) - SIZE_SZ;
  }
  return 0;
}

/*
  ------------------------------ mallinfo ------------------------------
*/

struct mallinfo mallinfo()
{
  mstate av = get_malloc_state();
  struct mallinfo mi;
  int i;
  mbinptr b;
  mchunkptr p;
  INTERNAL_SIZE_T avail;
  INTERNAL_SIZE_T fastavail;
  int nblocks;
  int nfastblocks;

  /* Ensure initialization */
  if (av->top == 0)  malloc_consolidate(av);

  /* Account for top */
  avail = chunksize(av->top);
  nblocks = 1;  /* top always exists */

  /* traverse fastbins */
  nfastblocks = 0;
  fastavail = 0;

  for (i = 0; NFASTBINS-i>0; ++i) {
    for (p = av->fastbins[i]; p != 0; p = p->fd) {
      ++nfastblocks;
      fastavail += chunksize(p);
    }
  }

  avail += fastavail;

  /* traverse regular bins */
  for (i = 1; i < NBINS; ++i) {
    b = bin_at(av, i);
    for (p = last(b); p != b; p = p->bk) {
      ++nblocks;
      avail += chunksize(p);
    }
  }

  mi.smblks = nfastblocks;
  mi.ordblks = nblocks;
  mi.fordblks = avail;
  mi.uordblks = av->sbrked_mem - avail;
  mi.arena = av->sbrked_mem;
  mi.hblks = av->n_mmaps;
  mi.hblkhd = av->mmapped_mem;
  mi.fsmblks = fastavail;
  mi.keepcost = chunksize(av->top);
  mi.usmblks = av->max_total_mem;
  return mi;
}

/*
  ------------------------------ malloc_stats ------------------------------
*/

void malloc_stats(void)
{
  struct mallinfo mi = mallinfo();
  fprintf(stderr, "max system bytes = %10lu\n",
          (CHUNK_SIZE_T)(mi.usmblks));
  fprintf(stderr, "system bytes     = %10lu\n",
          (CHUNK_SIZE_T)(mi.arena + mi.hblkhd));
  fprintf(stderr, "in use bytes     = %10lu\n",
          (CHUNK_SIZE_T)(mi.uordblks + mi.hblkhd));
}

int mallopt(int param_number, int value)
{
  mstate av = get_malloc_state();
  /* Ensure initialization/consolidation */
  malloc_consolidate(av);

  switch(param_number) {
  case M_MXFAST:
    if (value >= 0 && value <= MAX_FAST_SIZE) {
      set_max_fast(av, value);
      return 1;
    }
    else
      return 0;

  case M_TRIM_THRESHOLD:
    av->trim_threshold = value;
    return 1;

  case M_TOP_PAD:
    av->top_pad = value;
    return 1;

  case M_MMAP_THRESHOLD:
    av->mmap_threshold = value;
    return 1;

  case M_MMAP_MAX:
    av->n_mmaps_max = value;
    return 1;

  default:
    return 0;
  }
}

