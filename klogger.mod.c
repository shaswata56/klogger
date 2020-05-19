#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0xd2f30559, "module_layout" },
	{ 0x9ed554b3, "unregister_keyboard_notifier" },
	{ 0x96554810, "register_keyboard_notifier" },
	{ 0x741a0b06, "debugfs_remove_recursive" },
	{ 0x8f2b8bbb, "debugfs_create_file" },
	{ 0xf527707d, "debugfs_create_dir" },
	{ 0xcbd4898c, "fortify_panic" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0x9166fada, "strncpy" },
	{ 0xa916b694, "strnlen" },
	{ 0x656e4a6e, "snprintf" },
	{ 0x98a981ca, "simple_read_from_buffer" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "616F4EBA2E3B6F7D8D41A2D");
