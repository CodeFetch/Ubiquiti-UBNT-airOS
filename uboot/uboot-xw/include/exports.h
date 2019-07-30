#ifndef __EXPORTS_H__
#define __EXPORTS_H__

#ifndef __ASSEMBLY__

#ifndef UBNT_APP
#include <common.h>
#include <load_kernel.h>
#endif

/* These are declarations of exported functions available in C code */
unsigned long get_version(void);
int  getc(void);
int  tstc(void);
void putc(const char);
void puts(const char*);
void printf(const char* fmt, ...);
#ifndef UBNT_APP
void install_hdlr(int, interrupt_handler_t*, void*);
#endif
void free_hdlr(int);
void *malloc(size_t);
void free(void*);
void udelay(unsigned long);
unsigned long get_timer(unsigned long);
#ifndef UBNT_APP
void vprintf(const char *, va_list);
#endif
void do_reset (void);
#if (CONFIG_COMMANDS & CFG_CMD_I2C)
int i2c_write (uchar, uint, int , uchar* , int);
int i2c_read (uchar, uint, int , uchar* , int);
#endif	/* CFG_CMD_I2C */
#ifdef UBNT_APP
int find_mtd_part(const char *, struct mtd_device **, unsigned char *, struct part_info **);
char *getenv (char *);
void setenv (char *, char *);
#endif

void app_startup(char **);

#endif    /* ifndef __ASSEMBLY__ */

enum {
#define EXPORT_FUNC(x) XF_ ## x ,
#include <_exports.h>
#undef EXPORT_FUNC

	XF_MAX
};

#define XF_VERSION	2

#if defined(CONFIG_I386)
extern gd_t *global_data;
#endif

#endif	/* __EXPORTS_H__ */
