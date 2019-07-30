//#include <linux/config.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/resource.h>

#include <linux/console.h>
#include <asm/serial.h>

#include <linux/tty.h>
#include <linux/time.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/serial_8250.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>

#include <asm/mach-atheros/atheros.h>
#include <asm/delay.h>

#define ATH_DEFAULT_WD_TMO	(20ul * USEC_PER_SEC)

#ifdef ATH_WDT_TEST_CODE
#	define wddbg printk
#else
#	define wddbg(junk, ...)
#endif /* ATH_WDT_TEST_CODE 8 */

extern uint32_t ath_ahb_freq;

typedef struct {
	int open:1, can_close:1, panic_isr:1, tmo, action;
	wait_queue_head_t wq;
} ath_wdt_t;

static ath_wdt_t wdt_softc_array;

static ath_wdt_t *wdt = &wdt_softc_array;

#ifdef ATH_WDT_TEST_CODE
/* Return the value present in the watchdog register */
static inline uint32_t ath_get_wd_timer(void)
{
	uint32_t val;

	val = (uint32_t) ath_reg_rd(ATH_WATCHDOG_TMR);
	val = (val * USEC_PER_SEC) / ath_ahb_freq;

	return val;
}
#endif /* ATH_WDT_TEST_CODE */

/* Set the timeout value in the watchdog register */
void ath_set_wd_timer(uint32_t usec /* micro seconds */)
{
#ifdef CONFIG_MACH_AR934x
	usec = usec * (ath_ref_freq / USEC_PER_SEC);
#else
	usec = usec * (ath_ahb_freq / USEC_PER_SEC);
#endif

	wddbg("%s: 0x%08x\n", __func__, usec);

	ath_reg_wr(ATH_WATCHDOG_TMR, usec);
}

int ath_set_wd_timer_action(uint32_t val)
{
	if (val & ~ATH_WD_ACT_MASK) {
		return EINVAL;
	}

	wdt->action = val;

	/*
	 * bits  : 31 30 - 2 0-1
	 * access: RO  rsvd  Action
	 *
	 * Since bit 31 is read only and rest of the bits
	 * are zero, don't have to do a read-modify-write
	 */
	ath_reg_wr(ATH_WATCHDOG_TMR_CONTROL, val);
	return 0;
}

#ifdef ATH_WDT_TEST_CODE
static inline uint32_t ath_get_wd_timer_action(void)
{
	return (uint32_t) (ath_reg_rd(ATH_WATCHDOG_TMR_CONTROL) &
			   ATH_WD_ACT_MASK);
}

static inline uint32_t ath_get_wd_timer_last(void)
{
	return ((uint32_t) (ath_reg_rd(ATH_WATCHDOG_TMR_CONTROL) &
			    ATH_WD_LAST_MASK) >> ATH_WD_LAST_SHIFT);
}
#endif /* ATH_WDT_TEST_CODE */

irqreturn_t ath_wdt_panic_isr(int cpl, void *dev_id)
{
	wddbg("%s: invoked\n", __func__);

	/* Set the HARD watchdog restart later.
	 * Set it to 2 secs - a bit earlier than panic() expires.
	 * That will set WD flag for the next reboot.
	 */
	ath_set_wd_timer(2*1000*1000); /* micro-seconds */
	ath_set_wd_timer_action(ATH_WD_ACT_RESET);

	/* Mute the serial console - it is too slow */
	console_loglevel = 0;

	/* This is equivalent of SysRq-T (Tasks) */
	show_state_filter(0);

	show_mem();

	/* Set verbose console - just to see the panic message */
	console_loglevel = 7;
	panic("Watchdog Interrupt received");

	return IRQ_HANDLED;
}

static int athwdt_open(struct inode *inode, struct file *file)
{
	wddbg("%s: called\n", __func__);

	if (MINOR(inode->i_rdev) != WATCHDOG_MINOR) {
		return -ENODEV;
	}

	if (wdt->open) {
		return 0;
	}

	wdt->open = 1;
	wdt->tmo = ATH_DEFAULT_WD_TMO;
	wdt->action = ATH_WD_ACT_NONE;
	wdt->can_close = 0;
	init_waitqueue_head(&wdt->wq);

	ath_set_wd_timer(wdt->tmo);
        if (wdt->panic_isr)
            ath_set_wd_timer_action(ATH_WD_ACT_GP_INTR);
        else
            ath_set_wd_timer_action(ATH_WD_ACT_RESET);

	return nonseekable_open(inode, file);
}

static int athwdt_close(struct inode *inode, struct file *file)
{
	wddbg("%s: called\n", __func__);

	if (MINOR(inode->i_rdev) != WATCHDOG_MINOR) {
		return -ENODEV;
	}

	if (wdt->can_close) {
		wddbg("%s: clearing action\n", __func__);
		ath_set_wd_timer_action(ATH_WD_ACT_NONE);
	} else {
		wddbg("%s: not clearing action\n", __func__);
	}
	wdt->open = 0;
	return 0;
}

static ssize_t
athwdt_read(struct file *file, char *buf, size_t count, loff_t * ppos)
{
	int reset_st_bit=31;
	char wdt_str[]="0\n";
	int len = sizeof(wdt_str);

	wddbg("%s: called\n", __func__);

	if (count < len)
		return -EINVAL;

	if (*ppos != 0)
		return 0;

	if((ath_reg_rd(ATH_WATCHDOG_TMR_CONTROL) >> reset_st_bit) & 1) 
		wdt_str[0]='1';

	if (copy_to_user(buf, wdt_str,len)) 
		return -EFAULT;

	*ppos = len;

	return len;
}

static int
athwdt_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
		unsigned long arg)
{
	int ret = 0;

	wddbg("%s: called\n", __func__);

	switch (cmd) {
	default:
		ret = -EINVAL;
	}

	return ret;
}

static ssize_t
athwdt_write(struct file *file, const char *buf, size_t count, loff_t * ppos)
{
	int i;
	char c;

	wddbg("%s: called\n", __func__);

	for (i = 0; i != count; i++) {
		if (get_user(c, buf + i)) {
			return -EFAULT;
		}

		if (c == 'V') {
			wdt->can_close = 1;
			break;
		} else if (c == 'C') {
			wdt->panic_isr = 1;
                        ath_set_wd_timer(wdt->tmo);
                        ath_set_wd_timer_action(ATH_WD_ACT_GP_INTR);
			break;
		} else if (c == 'c') {
			wdt->panic_isr = 0;
                        ath_set_wd_timer(wdt->tmo);
                        ath_set_wd_timer_action(ATH_WD_ACT_RESET);
			break;
		}
	}

	if (i) {
		ath_set_wd_timer(wdt->tmo);
	}

	return count;
}

static struct file_operations athwdt_fops = {
      read:athwdt_read,
      write:athwdt_write,
      ioctl:athwdt_ioctl,
      open:athwdt_open,
      release:athwdt_close
};

static struct miscdevice athwdt_miscdev = {
	WATCHDOG_MINOR,
	"watchdog",
	&athwdt_fops
};

int __init athwdt_init(void)
{
	int ret;

	printk("%s: Registering WDT ", __func__);
	if ((ret = misc_register(&athwdt_miscdev))) {
		printk("failed %d\n", ret);
		return ret;
	} else {
		printk("success\n");
	}

	printk("Reset: %s\n", (ath_reg_rd(ATH_WATCHDOG_TMR_CONTROL)
				& (1 << 31)) ? "WD" : "Normal");

	/* Setup a handler that would generate crashlog */
	if ((ret = request_irq(ATH_MISC_IRQ_WATCHDOG,
							ath_wdt_panic_isr,
							0, "Watchdog Panic Handler", wdt))) {
		return ret;
	}
	ath_set_wd_timer_action(ATH_WD_ACT_RESET);
	return 0;
}

late_initcall(athwdt_init);
