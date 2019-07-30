include $(TOPDIR)/config.mk

LIB	= lib$(BOARD).a

OBJS	= $(BOARD).o flash.o ../common/ar7240_pci.o

OBJS	+= ../common/ar7240_flash.o

OBJS	+= ../common/athr8035_phy.o
OBJS	+= ../common/athr8032_phy.o
OBJS	+= ../common/athr8236_phy.o
OBJS	+= ../common/athr_s27_phy.o


OBJS	+= ../common/ubnt-board.o

SOBJS	= ../common/lowlevel_init_934x.o
SOBJS	+= ../common/lowlevel_init_934x-1.1.o

$(LIB):	.depend $(OBJS) $(SOBJS)
	$(AR) crv $@ $(OBJS) $(SOBJS)

#########################################################################

.depend:	Makefile $(SOBJS:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(SOBJS:.o=.S) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
