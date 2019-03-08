SECTIONS {
    .boards : {
        __boards = .;
        KEEP (*(.boards))
        __boards_end = .;
    }
}
INSERT AFTER .rodata;
