SECTIONS {
    .tracelog : {
        __tracelog_start = .;
        KEEP (*(.tracelog))
        __tracelog_end = .;
    }
}
INSERT AFTER .data;
