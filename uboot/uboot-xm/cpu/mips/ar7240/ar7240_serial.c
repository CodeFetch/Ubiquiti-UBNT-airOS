#include <asm/addrspace.h>
#include <asm/types.h>
#include <config.h>
#include <ar7240_soc.h>

#define		REG_OFFSET		4

/* === END OF CONFIG === */

/* register offset */
#define         OFS_RCV_BUFFER          (0*REG_OFFSET)
#define         OFS_TRANS_HOLD          (0*REG_OFFSET)
#define         OFS_SEND_BUFFER         (0*REG_OFFSET)
#define         OFS_INTR_ENABLE         (1*REG_OFFSET)
#define         OFS_INTR_ID             (2*REG_OFFSET)
#define         OFS_DATA_FORMAT         (3*REG_OFFSET)
#define         OFS_LINE_CONTROL        (3*REG_OFFSET)
#define         OFS_MODEM_CONTROL       (4*REG_OFFSET)
#define         OFS_RS232_OUTPUT        (4*REG_OFFSET)
#define         OFS_LINE_STATUS         (5*REG_OFFSET)
#define         OFS_MODEM_STATUS        (6*REG_OFFSET)
#define         OFS_RS232_INPUT         (6*REG_OFFSET)
#define         OFS_SCRATCH_PAD         (7*REG_OFFSET)

#define         OFS_DIVISOR_LSB         (0*REG_OFFSET)
#define         OFS_DIVISOR_MSB         (1*REG_OFFSET)

#define         MY_WRITE(y, z)  ((*((volatile u32*)(y))) = z)
#define         UART16550_READ(y)   ar7240_reg_rd((AR7240_UART_BASE+y))
#define         UART16550_WRITE(x, z)  ar7240_reg_wr((AR7240_UART_BASE+x), z)

void 
ar7240_sys_frequency(u32 *cpu_freq, u32 *ddr_freq, u32 *ahb_freq)
{
    u32 pll, pll_div, ref_div, ahb_div, ddr_div, freq;

    pll = ar7240_reg_rd(AR7240_CPU_PLL_CONFIG);

    pll_div = 
        ((pll & PLL_CONFIG_PLL_DIV_MASK) >> PLL_CONFIG_PLL_DIV_SHIFT);

    ref_div =
        ((pll & PLL_CONFIG_PLL_REF_DIV_MASK) >> PLL_CONFIG_PLL_REF_DIV_SHIFT);

    ddr_div = 
        ((pll & PLL_CONFIG_DDR_DIV_MASK) >> PLL_CONFIG_DDR_DIV_SHIFT) + 1;

    ahb_div = 
       (((pll & PLL_CONFIG_AHB_DIV_MASK) >> PLL_CONFIG_AHB_DIV_SHIFT) + 1)*2;

    freq = pll_div * ref_div * 5000000; 

    if (cpu_freq)
        *cpu_freq = freq;

    if (ddr_freq)
        *ddr_freq = freq/ddr_div;

    if (ahb_freq)
        *ahb_freq = freq/ahb_div;
}

/*
 * Detect if this is Ubiquiti POE Switch (check if Merlin radio is connected) and
 * configure GPIO FUNCTIONS
 */
#define AR7240_GPIO_FUNC_VALUE_DEFAULT	(AR7240_GPIO_FUNC_UART_EN | \
		AR7240_GPIO_FUNC_LED0_EN | AR7240_GPIO_FUNC_LED1_EN | \
		AR7240_GPIO_FUNC_LED2_EN | AR7240_GPIO_FUNC_LED3_EN | \
		AR7240_GPIO_FUNC_LED4_EN | \
		AR7240_GPIO_FUNC_ES_UART_DIS | AR7240_GPIO_FUNC_SPI_EN)

#ifdef CONFIG_UBIQUITI_SWITCH
/* GPIO functions for Ubiquiti POE Switch */
#define AR7240_GPIO_FUNC_VALUE_SWITCH		(AR7240_GPIO_FUNC_JTAG_DIS | \
		AR7240_GPIO_FUNC_LED4_EN | \
		AR7240_GPIO_FUNC_ES_UART_DIS | AR7240_GPIO_FUNC_SPI_EN)
#endif

#ifdef CONFIG_UBIQUITI_SWITCH
extern int UbntLEDsDisabled; /* in ap93.c */
extern int AR7240NoRadio; /* ar7240_pci.c */
extern int ResetButtonOnStartup;

extern int ar7240_reset_button(void);

extern int check_for_radio(void);
#endif

static void gpiofunc_preconfig(void)
{
#ifdef CONFIG_UBIQUITI_SWITCH
	/*
	 * check_for_radio will give three pci resets:
	 *  1. before relocation console_init_r()
	 *  2. after relocation - standard PCI initialization
	 *  3. after relocation, console_init_r()
	 *  Hopefuly this check_for_radio() function should detect
	 *  when PCI is up and skip the reset.
	 */
	if (check_for_radio())
		AR7240NoRadio = 1;
	else
		AR7240NoRadio = 0;

	ResetButtonOnStartup = ar7240_reset_button();

	if (AR7240NoRadio) {
		/* probably this is POE switch without radio */
		if (ResetButtonOnStartup)
			ar7240_reg_wr_nf(AR7240_GPIO_FUNC, AR7240_GPIO_FUNC_VALUE_DEFAULT);
		else
			ar7240_reg_wr_nf(AR7240_GPIO_FUNC, AR7240_GPIO_FUNC_VALUE_SWITCH);
	} else {
		ar7240_reg_wr_nf(AR7240_GPIO_FUNC, AR7240_GPIO_FUNC_VALUE_DEFAULT);
		UbntLEDsDisabled = 0; /* enable LED status */
	}
#else
	ar7240_reg_wr_nf(AR7240_GPIO_FUNC, AR7240_GPIO_FUNC_VALUE_DEFAULT);
#endif
}

int serial_init(void)
{
    u32 div,val;
    u32 ahb_freq, ddr_freq, cpu_freq;

    ar7240_sys_frequency(&cpu_freq, &ddr_freq, &ahb_freq);

    div  = ahb_freq/(16 * CONFIG_BAUDRATE);  

    /* decide which functions we need to enable - UART/JTAG/etc. */
    gpiofunc_preconfig();

	/**
	 * Assert SYS_RESET_OUT, so that GPS does not send anything
	 * on serial to stop U-boot from booting.
	 * It's deasserted back on SHOW_BOOT_PROGRESS(15), which is
	 * called right before transferring control to kernel.
	 */
	ar7240_reg_rmw_set(AR7240_RESET, AR7240_RESET_EXTERNAL);
	udelay(100);


    /* 
     * set DIAB bit 
     */
    UART16550_WRITE(OFS_LINE_CONTROL, 0x80);
        
    /* set divisor */
    UART16550_WRITE(OFS_DIVISOR_LSB, (div & 0xff));
    UART16550_WRITE(OFS_DIVISOR_MSB, ((div >> 8) & 0xff));

    /* clear DIAB bit*/ 
    UART16550_WRITE(OFS_LINE_CONTROL, 0x00);

    /* set data format */
    UART16550_WRITE(OFS_DATA_FORMAT, 0x3);

    UART16550_WRITE(OFS_INTR_ENABLE, 0);

	return 0;
}

int serial_tstc (void)
{
    return(UART16550_READ(OFS_LINE_STATUS) & 0x1);
}

u8 serial_getc(void)
{
    while(!serial_tstc());

    return UART16550_READ(OFS_RCV_BUFFER);
}


void serial_putc(u8 byte)
{
    if (byte == '\n') serial_putc ('\r');

    while (((UART16550_READ(OFS_LINE_STATUS)) & 0x20) == 0x0);
    UART16550_WRITE(OFS_SEND_BUFFER, byte);
}

void serial_setbrg (void)
{
}

void serial_puts (const char *s)
{
	while (*s)
	{
		serial_putc (*s++);
	}
}
