SECTIONS {
    .dyndbg : {
        __dyndbg_start = .;
        KEEP (*(.dyndbg))
        __dyndbg_end = .;
    }
}
INSERT AFTER .data;
