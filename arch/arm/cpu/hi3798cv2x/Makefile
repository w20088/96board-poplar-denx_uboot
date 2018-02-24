

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(CPU).o

START   := start.o
END	:= tail.o
#COBJS   := chip.o
#COBJS   += cpu.o
#COBJS   += timer.o
#COBJS   += usb.o
SOBJS   += uart.o
SOBJS   += arm_exception.o
#SOBJS   += slave_bootup.o

SRCS	:= $(START:.o=.S) $(SOBJS:.o=.S) $(COBJS:.o=.c) $(END:.o=.S)
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))
START	:= $(addprefix $(obj),$(START))
END	:= $(addprefix $(obj),$(END))
ifdef CONFIG_COMPRESS_BOOT
AFLAGS += -DCONFIG_NO_HEAD
endif

all:	$(obj).depend $(START) $(LIB) $(END)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
