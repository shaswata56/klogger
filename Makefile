modname += klogger
obj-m += $(modname).o

KVER = $(shell uname -r)
KDIR += /lib/modules/$(KVER)/build

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

install:
	mkdir -p /lib/modules/$(KVER)/misc/$(modname)
	install -m 0755 -o root -g root $(modname).ko /lib/modules/$(KVER)/misc/$(modname)
	depmod -a

uninstall:
	rm /lib/modules/$(KVER)/misc/$(modname)/$(modname).ko
	rmdir /lib/modules/$(KVER)/misc/$(modname)
	rmdir /lib/modules/$(KVER)/misc
	depmod -a
