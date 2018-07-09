#undef GETRA
#define GETRA() 0

#define DATA_SIZE (1 << SHIFT)

#if DATA_SIZE == 8
#define SUFFIX q
#define LSUFFIX q
#define SDATA_TYPE  int64_t
#define DATA_TYPE  uint64_t
#elif DATA_SIZE == 4
#define SUFFIX l
#define LSUFFIX l
#define SDATA_TYPE  int32_t
#define DATA_TYPE  uint32_t
#elif DATA_SIZE == 2
#define SUFFIX w
#define LSUFFIX uw
#define SDATA_TYPE  int16_t
#define DATA_TYPE  uint16_t
#elif DATA_SIZE == 1
#define SUFFIX b
#define LSUFFIX ub
#define SDATA_TYPE  int8_t
#define DATA_TYPE  uint8_t
#else
#error unsupported data size
#endif


/* For the benefit of TCG generated code, we want to avoid the complication
   of ABI-specific return type promotion and always return a value extended
   to the register size of the host.  This is tcg_target_long, except in the
   case of a 32-bit host and 64-bit data, and for that we always have
   uint64_t.  Don't bother with this widened value for SOFTMMU_CODE_ACCESS.  */
#if defined(SOFTMMU_CODE_ACCESS) || DATA_SIZE == 8
# define WORD_TYPE  DATA_TYPE
# define USUFFIX    SUFFIX
#else
# define WORD_TYPE  tcg_target_ulong
# define USUFFIX    glue(u, SUFFIX)
# define SSUFFIX    glue(s, SUFFIX)
#endif

#ifdef SOFTMMU_CODE_ACCESS
#define READ_ACCESS_TYPE MMU_INST_FETCH
#define ADDR_READ addr_code
#else
#define READ_ACCESS_TYPE MMU_DATA_LOAD
#define ADDR_READ addr_read
#endif

#if DATA_SIZE == 8
# define BSWAP(X)  bswap64(X)
#elif DATA_SIZE == 4
# define BSWAP(X)  bswap32(X)
#elif DATA_SIZE == 2
# define BSWAP(X)  bswap16(X)
#else
# define BSWAP(X)  (X)
#endif

#ifdef TARGET_WORDS_BIGENDIAN
# define TGT_BE(X)  (X)
# define TGT_LE(X)  BSWAP(X)
#else
# define TGT_BE(X)  BSWAP(X)
# define TGT_LE(X)  (X)
#endif

#if DATA_SIZE == 1
# define helper_le_ld_name  glue(glue(helper_ret_ld, USUFFIX), MMUSUFFIX)
# define helper_be_ld_name  helper_le_ld_name
# define helper_le_lds_name glue(glue(helper_ret_ld, SSUFFIX), MMUSUFFIX)
# define helper_be_lds_name helper_le_lds_name
# define helper_le_st_name  glue(glue(helper_ret_st, SUFFIX), MMUSUFFIX)
# define helper_be_st_name  helper_le_st_name
#else
# define helper_le_ld_name  glue(glue(helper_le_ld, USUFFIX), MMUSUFFIX)
# define helper_be_ld_name  glue(glue(helper_be_ld, USUFFIX), MMUSUFFIX)
# define helper_le_lds_name glue(glue(helper_le_ld, SSUFFIX), MMUSUFFIX)
# define helper_be_lds_name glue(glue(helper_be_ld, SSUFFIX), MMUSUFFIX)
# define helper_le_st_name  glue(glue(helper_le_st, SUFFIX), MMUSUFFIX)
# define helper_be_st_name  glue(glue(helper_be_st, SUFFIX), MMUSUFFIX)
#endif

#ifdef TARGET_WORDS_BIGENDIAN
# define helper_te_ld_name  helper_be_ld_name
# define helper_te_st_name  helper_be_st_name
#else
# define helper_te_ld_name  helper_le_ld_name
# define helper_te_st_name  helper_le_st_name
#endif

void crete_todo_op_helper(); /* dummy declaration to indicate the todo works*/
uint64_t crete_get_dynamic_addr(uint64_t static_addr);

#ifndef SOFTMMU_CODE_ACCESS
static inline DATA_TYPE glue(io_read, SUFFIX)(CPUArchState *env,
                                              hwaddr physaddr,
                                              target_ulong addr,
                                              uintptr_t retaddr)
{
    crete_todo_op_helper();
    return 0;
}
#endif

#ifdef SOFTMMU_CODE_ACCESS
static __attribute__((unused))
#endif
WORD_TYPE helper_le_ld_name(CPUArchState *env, target_ulong addr, int mmu_idx,
                            uintptr_t retaddr)
{
    DATA_TYPE res;

    /* Handle slow unaligned access (it spans two pages or IO).  */
    if (DATA_SIZE > 1
        && ((addr & ~TARGET_PAGE_MASK) + DATA_SIZE - 1
                >= TARGET_PAGE_SIZE)) {
        target_ulong addr1, addr2;
        DATA_TYPE res1, res2;
        unsigned shift;

        addr1 = addr & ~(DATA_SIZE - 1);
        addr2 = addr1 + DATA_SIZE;
        /* Note the adjustment at the beginning of the function.
           Undo that for the recursion.  */
        res1 = helper_le_ld_name(env, addr1, mmu_idx, retaddr);
        res2 = helper_le_ld_name(env, addr2, mmu_idx, retaddr);
        shift = (addr & (DATA_SIZE - 1)) * 8;

        /* Little-endian combine.  */
        res = (res1 >> shift) | (res2 << ((DATA_SIZE * 8) - shift));

        return res;
    }

    uint64_t dynamic_addr = crete_get_dynamic_addr(addr);

#if DATA_SIZE == 1
    res = glue(glue(ld, LSUFFIX), _p)((uint8_t *)dynamic_addr);
#else
    res = glue(glue(ld, LSUFFIX), _le_p)((uint8_t *)dynamic_addr);
#endif
    return res;
}

#if DATA_SIZE > 1
#ifdef SOFTMMU_CODE_ACCESS
static __attribute__((unused))
#endif
WORD_TYPE helper_be_ld_name(CPUArchState *env, target_ulong addr, int mmu_idx,
                            uintptr_t retaddr)
{
    DATA_TYPE res;

    /* Handle slow unaligned access (it spans two pages or IO).  */
    if (DATA_SIZE > 1
        && ((addr & ~TARGET_PAGE_MASK) + DATA_SIZE - 1
                >= TARGET_PAGE_SIZE)) {
        target_ulong addr1, addr2;
        DATA_TYPE res1, res2;
        unsigned shift;

        addr1 = addr & ~(DATA_SIZE - 1);
        addr2 = addr1 + DATA_SIZE;
        /* Note the adjustment at the beginning of the function.
           Undo that for the recursion.  */
        res1 = helper_be_ld_name(env, addr1, mmu_idx, retaddr);
        res2 = helper_be_ld_name(env, addr2, mmu_idx, retaddr);
        shift = (addr & (DATA_SIZE - 1)) * 8;

        /* Big-endian combine.  */
        res = (res1 << shift) | (res2 >> ((DATA_SIZE * 8) - shift));

        return res;
    }

    // TODO: xxx BE (should not need to reverse the address,
    //               as long as it stay consistent wit memory monitoring)
    uint64_t dynamic_addr = crete_get_dynamic_addr(addr);
    res = glue(glue(ld, LSUFFIX), _be_p)((uint8_t *)dynamic_addr);
    return res;
}
#endif /* DATA_SIZE > 1 */

DATA_TYPE
glue(glue(helper_ld, SUFFIX), MMUSUFFIX)(CPUArchState *env, target_ulong addr,
                                         int mmu_idx)
{
    return helper_te_ld_name (env, addr, mmu_idx, GETRA());
}

#ifndef SOFTMMU_CODE_ACCESS

/* Provide signed versions of the load routines as well.  We can of course
   avoid this for 64-bit data, or for 32-bit data on 32-bit host.  */
#if DATA_SIZE * 8 < TCG_TARGET_REG_BITS
WORD_TYPE helper_le_lds_name(CPUArchState *env, target_ulong addr,
                             int mmu_idx, uintptr_t retaddr)
{
    return (SDATA_TYPE)helper_le_ld_name(env, addr, mmu_idx, retaddr);
}

# if DATA_SIZE > 1
WORD_TYPE helper_be_lds_name(CPUArchState *env, target_ulong addr,
                             int mmu_idx, uintptr_t retaddr)
{
    return (SDATA_TYPE)helper_be_ld_name(env, addr, mmu_idx, retaddr);
}
# endif
#endif

static inline void glue(io_write, SUFFIX)(CPUArchState *env,
                                          hwaddr physaddr,
                                          DATA_TYPE val,
                                          target_ulong addr,
                                          uintptr_t retaddr)
{
    crete_todo_op_helper();
}

void helper_le_st_name(CPUArchState *env, target_ulong addr, DATA_TYPE val,
                       int mmu_idx, uintptr_t retaddr)
{
    /* Handle slow unaligned access (it spans two pages or IO).  */
    if (DATA_SIZE > 1
        && ((addr & ~TARGET_PAGE_MASK) + DATA_SIZE - 1
                >= TARGET_PAGE_SIZE)) {
        int i;

        /* XXX: not efficient, but simple */
        /* Note: relies on the fact that tlb_fill() does not remove the
         * previous page from the TLB cache.  */
        for (i = DATA_SIZE - 1; i >= 0; i--) {
            /* Little-endian extract.  */
            uint8_t val8 = val >> (i * 8);
            /* Note the adjustment at the beginning of the function.
               Undo that for the recursion.  */
            glue(helper_ret_stb, MMUSUFFIX)(env, addr + i, val8,
                                            mmu_idx, retaddr);
        }
        return;
    }

    uint64_t dynamic_addr = crete_get_dynamic_addr(addr);
#if DATA_SIZE == 1
    glue(glue(st, SUFFIX), _p)((uint8_t *)dynamic_addr, val);
#else
    glue(glue(st, SUFFIX), _le_p)((uint8_t *)dynamic_addr, val);
#endif
}

#if DATA_SIZE > 1
void helper_be_st_name(CPUArchState *env, target_ulong addr, DATA_TYPE val,
                       int mmu_idx, uintptr_t retaddr)
{
    /* Handle slow unaligned access (it spans two pages or IO).  */
    if (DATA_SIZE > 1
        && ((addr & ~TARGET_PAGE_MASK) + DATA_SIZE - 1
                >= TARGET_PAGE_SIZE)) {
        int i;

        /* XXX: not efficient, but simple */
        /* Note: relies on the fact that tlb_fill() does not remove the
         * previous page from the TLB cache.  */
        for (i = DATA_SIZE - 1; i >= 0; i--) {
            /* Big-endian extract.  */
            uint8_t val8 = val >> (((DATA_SIZE - 1) * 8) - (i * 8));
            /* Note the adjustment at the beginning of the function.
               Undo that for the recursion.  */
            glue(helper_ret_stb, MMUSUFFIX)(env, addr + i, val8,
                                            mmu_idx, retaddr);
        }
        return;
    }

    // TODO: xxx BE (should not need to reverse the address,
    //               as long as it stay consistent wit memory monitoring)
    uint64_t dynamic_addr = crete_get_dynamic_addr(addr);
    glue(glue(st, SUFFIX), _be_p)((uint8_t *)dynamic_addr, val);
}

#endif /* DATA_SIZE > 1 */

void
glue(glue(helper_st, SUFFIX), MMUSUFFIX)(CPUArchState *env, target_ulong addr,
                                         DATA_TYPE val, int mmu_idx)
{
    helper_te_st_name(env, addr, val, mmu_idx, GETRA());
}

#endif /* !defined(SOFTMMU_CODE_ACCESS) */

#undef READ_ACCESS_TYPE
#undef SHIFT
#undef DATA_TYPE
#undef SUFFIX
#undef LSUFFIX
#undef DATA_SIZE
#undef ADDR_READ
#undef WORD_TYPE
#undef SDATA_TYPE
#undef USUFFIX
#undef SSUFFIX
#undef BSWAP
#undef TGT_BE
#undef TGT_LE
#undef CPU_BE
#undef CPU_LE
#undef helper_le_ld_name
#undef helper_be_ld_name
#undef helper_le_lds_name
#undef helper_be_lds_name
#undef helper_le_st_name
#undef helper_be_st_name
#undef helper_te_ld_name
#undef helper_te_st_name
