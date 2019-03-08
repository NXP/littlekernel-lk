SECTIONS {
    . = %KERNEL_BASE%;

    .cmdline : {
        __cmdline_start = .;
        __cmdline = .;
        . += 4096;
        __cmdline_end = .;
    }
}
INSERT BEFORE .text;