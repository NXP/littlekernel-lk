/*
 * Copyright (c) 2008 Travis Geiselbrecht
 * Copyright 2018 NXP - Remove omap3 related code
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <app.h>
#include <debug.h>
#include <err.h>
#include <string.h>
#include <dev/i2c.h>

#define LOCAL_TRACE 0

#if WITH_LIB_CONSOLE

#include <lib/console.h>

static int cmd_i2c(int argc, const cmd_args *argv);

STATIC_COMMAND_START
STATIC_COMMAND("i2c", "i2c read/write commands", &cmd_i2c )
STATIC_COMMAND_END(i2c);

static int cmd_i2c(int argc, const cmd_args *argv)
{
	int err;

	if (argc < 5) {
		printf("not enough arguments\n");
usage:
		printf("%s read_reg <bus> <i2c address> <register>\n", argv[0].str);
		printf("%s write_reg <bus> <i2c address> <register> <val>\n", argv[0].str);
		return -1;
	}

	int bus = argv[2].u;
	uint8_t i2c_address = argv[3].u;

	if (!strcmp(argv[1].str, "read_reg")) {
		uint8_t reg = argv[4].u;
		uint8_t val;

		err = i2c_read_reg(bus, i2c_address, reg, &val);
		printf("i2c_read_reg err %d, val 0x%hhx\n", err, val);
	} else if (!strcmp(argv[1].str, "write_reg")) {
		uint8_t reg = argv[4].u;
		uint8_t val = argv[5].u;
		err = i2c_write_reg(bus, i2c_address, reg, val);
		printf("i2c_write_reg err %d\n", err);
	} else {
		printf("unrecognized subcommand\n");
		goto usage;
	}

	return 0;
}

APP_START(i2c)
    .flags = 0,
APP_END

#endif // WITH_APP_CONSOLE


