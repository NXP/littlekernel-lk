// Copyright 2017 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#include <arch/arm64/smccc.h>
#include <dev/psci.h>

#include <string.h>

static uint64_t shutdown_args[3] = { 0, 0, 0 };
static uint64_t reboot_args[3] = { 0, 0, 0 };
static uint64_t reboot_bootloader_args[3] = { 0, 0, 0 };
static uint64_t reboot_recovery_args[3] = { 0, 0, 0 };

static uint64_t psci_smc_call(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3)
{
    return arm_smccc_smc(arg0, arg1, arg2, arg3, 0, 0, 0, 0).x0;
}

static uint64_t psci_hvc_call(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3)
{
    return arm_smccc_hvc(arg0, arg1, arg2, arg3, 0, 0, 0, 0).x0;
}

#if PSCI_USE_HVC
psci_call_proc do_psci_call = psci_hvc_call;
#else
psci_call_proc do_psci_call = psci_smc_call;
#endif

void psci_system_off(void)
{
    do_psci_call(PSCI64_SYSTEM_OFF, shutdown_args[0], shutdown_args[1], shutdown_args[2]);
}

void psci_system_reset(enum reboot_flags flags)
{
    uint64_t *args = reboot_args;

    if (flags == REBOOT_BOOTLOADER) {
        args = reboot_bootloader_args;
    }
    else if (flags == REBOOT_RECOVERY) {
        args = reboot_recovery_args;
    }

    do_psci_call(PSCI64_SYSTEM_RESET, args[0], args[1], args[2]);
}

void arm_psci_init(
    int use_hvc,
    const char *_shutdown_args,
    const char *_reboot_args,
    const char *_reboot_bootloader_args,
    const char *_reboot_recovery_args)
{

    do_psci_call = use_hvc ? psci_hvc_call : psci_smc_call;
    memcpy(shutdown_args, _shutdown_args, sizeof(shutdown_args));
    memcpy(reboot_args, _reboot_args, sizeof(reboot_args));
    memcpy(reboot_bootloader_args, _reboot_bootloader_args, sizeof(reboot_bootloader_args));
    memcpy(reboot_recovery_args, _reboot_recovery_args, sizeof(reboot_recovery_args));
}

#if WITH_LIB_CONSOLE
#include <lib/console.h>

static int cmd_psci(int argc, const cmd_args *argv)
{
    uint64_t arg0, arg1 = 0, arg2 = 0, arg3 = 0;

    if (argc < 2) {
        printf("not enough arguments\n");
        printf("%s arg0 [arg1] [arg2] [arg3]\n", argv[0].str);
        return -1;
    }

    arg0 = argv[1].u;
    if (argc >= 3) {
        arg1 = argv[2].u;
        if (argc >= 4) {
            arg2 = argv[3].u;
            if (argc >= 5) {
                arg3 = argv[4].u;
            }
        }
    }

    uint32_t ret = do_psci_call(arg0, arg1, arg2, arg3);
    printf("do_psci_call returned %u\n", ret);
    return 0;
}

STATIC_COMMAND_START
STATIC_COMMAND_MASKED("psci", "execute PSCI command", &cmd_psci, CMD_AVAIL_ALWAYS)
STATIC_COMMAND_END(psci);

#endif // WITH_LIB_CONSOLE
