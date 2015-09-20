/**
 * cal_dex 0.1
 *
 * Debug exception support. (20150421, cal)
 */

#define CAL_DEX_VERSION "0.1"
#define CAL_DEX_DUMP_FORMAT "0.1"

//extern void output_redirect(const char *str);
//#define c_puts output_redirect

extern void* __real__xtos_set_exception_handler(int exno, void (*exhandler)());

extern char* system_get_sdk_version();

typedef struct
{
    int epc1;
    int ps;
    int sar;
    int xx1;
    int a0;
    int a2;
    int a3;
    int a4;
    int a5;
    int a6;
    int a7;
    int a8;
    int a9;
    int a10;
    int a11;
    int a12;
    int a13;
    int a14;
    int a15;
    int exccause;
} ex_regs;

/* use some link symbols to get a kind of firmware fingerprint */
extern char _text_start;
extern char _text_end;
extern char _rodata_start;
extern char _rodata_end;
extern char _data_start;
extern char _data_end;
extern char _bss_start;
extern char _bss_end;

static void (*orighandler)(int code);
static void cal_dex_handler(int exccause, ex_regs regs);

#define L(x) (((unsigned int)(x)) & 0xFFFF)

// did not work reliably for me (cal101)
#define USE_ETS_PRINTF 1

#if USE_ETS_PRINTF
#define c_puts(a) cal_tprintf(a)
extern void ets_uart_printf(char* fmt, ...);
//#define cal_tprintf printf
#define cal_tprintf printf
#else
static char TINYBUF[120];

#define cal_tprintfOFF(fmt,...) do { \
ets_vsprintf(TINYBUF, fmt, __VA_ARGS__); \
c_puts(TINYBUF); \
} while(0)
#define P4X(s,n) c_puts(s); print4x(n);
#define P8X(s,n) c_puts(s); print8x((int)(n));
#endif

#if !USE_ETS_PRINTF
// quick and dirty print hex bytes to avoid printf.
MEMSPACE
void print8x(unsigned int n)
{
    int i = 0;
    for (i = 0; i < 8; i++)
    {
        int nib = n & 0xf;
        TINYBUF[7-i] = (nib < 10) ? '0' + nib : 'a' - 10 + nib;
        n = n>>4;
    }
    TINYBUF[8] = 0;
    c_puts(TINYBUF);
}


MEMSPACE
void print4x(unsigned int n)
{
    int i = 0;
    for (i = 0; i < 4; i++)
    {
        int nib = n & 0xf;
        TINYBUF[3-i] = (nib < 10) ? '0' + nib : 'a' - 10 + nib;
        n = n>>4;
    }
    TINYBUF[4] = 0;
    c_puts(TINYBUF);
}


MEMSPACE
void print2d(unsigned int n)
{
    int i = 0;
    for (i = 0; i < 2; i++)
    {
        int nib = n % 10;
        TINYBUF[1-i] = '0' + nib;
        n = n / 10;
    }
    TINYBUF[2] = 0;
    c_puts(TINYBUF);
}
#endif

#define NUM 8

// TODO: could use TINYBUF per line
//__attribute__((noinline))
void cal_dex_dump_hex(int* p, int cnt)
{
    int i;
    for (i = 0; i < cnt; i++)
    {
#if USE_ETS_PRINTF
        if (i % NUM == 0)
        {
            printf("%08x ", (int )(p + i));
        }
        printf(" %08x", p[i]);
#else
        if (i % NUM == 0)
        {
            print8x((int )(p + i));
            c_puts(" ");
        }
        c_puts(" ");
        print8x(p[i]);
#endif
        if ((i + 1) % NUM == 0)
        {
            c_puts("\n");
        }
    }
}


int stackok(int* sp)
{
    int i = (unsigned int) sp;
    return (i > 0x3fffc000) && (i < 0x40000000) && ((i & 3) == 0);
}


//__attribute__((noinline))
void cal_dex_dump_stack(int* sp)
{
    int* p = sp - 8;
    if (stackok(p))
    {
        int n = (0x40000000 - (unsigned int)p);
        if (n > 512)
        {
            n = 512;
        }
#if defined(cal_tprintf)
        cal_tprintf("Stack: (%08x)\n", sp);
#else
        c_puts("Stack (");
        print8x((unsigned int)sp);
        c_puts(")\n");
#endif
        cal_dex_dump_hex(p, n / 4);
    }
    else
    {
#if defined(cal_tprintf)
        cal_tprintf("Stack pointer may be corrupt: %08x\n", sp);
#else
        c_puts("Stack pointer may be corrupt: ");
        print8x((unsigned int)sp);
        c_puts("\n");
#endif
    }
}


static int get_excvaddr()
{
    int v;
    asm volatile (
        "rsr.excvaddr %0\n\t"
        : "=r" (v)
        );
    return v;
}


static int get_depc()
{
    int v;
    asm volatile (
        "rsr.depc %0\n\t"
        : "=r" (v)
        );
    return v;
}


//__attribute__((noinline))
void cal_dex_dump(int* sp)
{
    ex_regs *regs = (ex_regs*) sp;
// stack pointer at exception place
    int* ex_sp = (sp + 256 / 4);
    int* p = ex_sp - 8;
    int usestack = stackok(p);

    c_puts("cal_dex " CAL_DEX_VERSION ", dump " CAL_DEX_DUMP_FORMAT "\n");
#if defined(cal_tprintf)
    cal_tprintf("Fatal Exception: %04x (%d), sp %08x\n", regs->exccause,
        regs->exccause, sp);
#else
    c_puts("Fatal Exception: ");
    print4x(regs->exccause);
    c_puts(" (");
    print2d(regs->exccause);
    c_puts("), sp ");
    print8x((int)sp);
    c_puts("\n");
#endif
    c_puts("SDK Version: ");
    c_puts(system_get_sdk_version());
    c_puts("\n");
    if (usestack)
    {
        int excvaddr = get_excvaddr();
        int depc = get_depc();
#if defined(cal_tprintf)
        cal_tprintf(
            "Fingerprint: 1/xh=%08x,t=%04x-%04x,d=%04x-%04x,b=%04x-%04x,ro=%04x-%04x\n",
            cal_dex_handler, L(&_text_start), L(&_text_end),
            L(&_data_start), L(&_data_end), L(&_bss_start), L(&_bss_end),
            L(&_rodata_start), L(&_rodata_end));
        cal_tprintf(
            " epc1: %08x  ps: %08x  sar: %08x  exccause: %08x  unk1: %08x\n",
            regs->epc1, regs->ps, regs->sar, regs->exccause, regs->xx1);
// a1 is stack at exception
        cal_tprintf(" a0 :  %08x  a1 :  %08x  a2 :  %08x  a3 :  %08x\n",
            regs->a0, (int )ex_sp, regs->a2, regs->a3);
        cal_tprintf(" a4 :  %08x  a5 :  %08x  a6 :  %08x  a7 :  %08x\n",
            regs->a4, regs->a5, regs->a6, regs->a7);
        cal_tprintf(" a8 :  %08x  a9 :  %08x  a10:  %08x  a11:  %08x\n",
            regs->a8, regs->a9, regs->a10, regs->a11);
        cal_tprintf(" a12:  %08x  a13:  %08x  a14:  %08x  a15:  %08x\n",
            regs->a12, regs->a13, regs->a14, regs->a15);
#else
        c_puts("Fingerprint: 1/");
        P8X("xh=", cal_dex_handler);
        P4X(",t=", L(&_text_start)); P4X("-", L(&_text_end));
        P4X(",d=", L(&_data_start)); P4X("-", L(&_data_end));
        P4X(",b=", L(&_bss_start)); P4X("-", L(&_bss_end));
        P4X(",ro=", L(&_rodata_start)); P4X("-", L(&_rodata_end));
        c_puts("\n");
        P8X(" epc1: ", regs->epc1); P8X("  exccause: ", regs->exccause); P8X("  excvaddr: ", excvaddr);
        P8X("  depc: ", depc);
        c_puts("\n");
        P8X(" ps  : ", regs->ps);   P8X("  sar     : ", regs->sar);      P8X("  unk1    : ", regs->xx1);
        c_puts("\n");
// a1 is stack at exception
        P8X(" a0 :  ", regs->a0);
        P8X("  a1 :  ", (int )ex_sp);
        P8X("  a2 :  ", regs->a2);
        P8X("  a3 :  ", regs->a3);
        c_puts("\n");
        P8X(" a4 :  ", regs->a4);
        P8X("  a5 :  ", regs->a5);
        P8X("  a6 :  ", regs->a6);
        P8X("  a7 :  ", regs->a7);
        c_puts("\n");
        P8X(" a8 :  ", regs->a8);
        P8X("  a9 :  ", regs->a9);
        P8X("  a10:  ", regs->a10);
        P8X("  a11:  ", regs->a11);
        c_puts("\n");
        P8X(" a12:  ", regs->a12);
        P8X("  a13:  ", regs->a13);
        P8X("  a14:  ", regs->a14);
        P8X("  a15:  ", regs->a15);
        c_puts("\n");
#endif
        cal_dex_dump_stack(p);
    }
    else
    {
#if defined(cal_tprintf)
        cal_tprintf("Stack pointer may be corrupted: %08x\n", sp);
#else
        c_puts("Stack pointer may be corrupted: ");
        print8x((int)sp);
        c_puts("\n");
#endif
    }
}


// TODO: parameter passing to orighandler is most probably wrong
/*

 __attribute__((interrupt,exception,exception_handler,isr,noreturn)) static void cal_dex_handler(ex_regs regs) {
 int* p = __builtin_frame_address(0);
 cal_dex_dump(p, &regs);
 //orighandler(regs.exccause);
 asm("jmp %0" :: "p" (orighandler));
 }
 */

/*
 * a2 contains exception cause, current stack contains register dump.
 * a3 and a4 may be used because they are not used by standard handler.
 */
//__attribute__((noinline))
void cal_dex_handler(int exccause, ex_regs regs)
{
    asm volatile (
        "addi a1, a1, -16\n\t"
        "s32i.n	a0, a1, 12\n\t"
        );
    asm volatile (
        "addi a2, a1, 16\n\t"
        "mov a0, %0\n\t"
        "callx0	a0\n\t"
        :: "r" (cal_dex_dump)
        );
    asm volatile (
        "mov a2, %1\n\t"
        "mov a3, %0\n\t"
        "l32i a0, a1, 12\n\t"
        "addi a1, a1, 16\n\t"
        "jx a3\n\t"
        :: "r" (orighandler), "r" (exccause)
        );
}


/**
 * This intercepts calls to _xtos_set_exception_handler which
 * is reached via symbol __real__xtos_set_exception_handler.
 *
 * Calls to exception handler are intercepted by cal_dex_handler
 * before delegating to original handler.
 */
void* __wrap__xtos_set_exception_handler(int exno, void (*exhandler)())
{
//int* sp = __builtin_frame_address(0);
    void *res = 0;
    if (exhandler != 0)
    {
        if (orighandler == 0)
        {
            orighandler = exhandler;
            res = __real__xtos_set_exception_handler(exno, exhandler);
        }
        else
        {
            if (orighandler != exhandler)
            {
                c_puts(
                    "__wrap__xtos_set_exception_handler: unexpected handler!");
                res = __real__xtos_set_exception_handler(exno, exhandler);
            }
            else
            {
//c_puts("installed exception_handler\n");
                res = __real__xtos_set_exception_handler(exno, cal_dex_handler);
            }
        }
    }
    else
    {
        void* p = __real__xtos_set_exception_handler(exno, exhandler);
        if (p == cal_dex_handler)
        {
            res = orighandler;
        }
        else
        {
            res = p;
        }
    }
    if (exno == 29)
    {
// provoke error
//*((int*)(0x1020)) = 0xdeadbeaf;
//cal_dex_dump_stack((int*) sp);
    }
    return res;
}
