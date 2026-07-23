/* ThaiOS Linker Script (x86_64) */
/* ================================ */

ENTRY(_start)

PHDRS
{
    text    PT_LOAD    FLAGS((1 << 0) | (1 << 2)); /* Read + Execute */
    rodata  PT_LOAD    FLAGS((1 << 0));             /* Read only */
    data    PT_LOAD    FLAGS((1 << 0) | (1 << 1) | (1 << 2)); /* Read + Write + Execute */
    dynamic PT_DYNAMIC;
}

SECTIONS
{
    . = 0xFFFFFFFF80000000;

    .text : {
        *(.text .text.*)
    } :text

    . = ALIGN(4096);
    .rodata : {
        *(.rodata .rodata.*)
    } :rodata

    . = ALIGN(4096);
    .data : {
        *(.data .data.*)
    } :data

    . = ALIGN(4096);
    .bss : {
        *(COMMON)
        *(.bss .bss.*)
    } :data

    . = ALIGN(4096);
    .dynamic : {
        *(.dynamic)
    } :data :dynamic

    . = ALIGN(4096);
    .got : {
        *(.got .got.*)
        *(.got.plt)
    } :data

    /DISCARD/ : {
        *(.eh_frame)
        *(.comment .comment.*)
        *(.note .note.*)
    }
}
