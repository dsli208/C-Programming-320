#include "budprint.h"

// +43+16+7+6+5+5+ <- 89 chars

static void bk_print_1_row() {
    fprintf(stderr, "|%43s|%16s|%7s|%6s|%5s|%5s|", "unused2", "rsize", "unused1", "padded", "order", "alloc");
}

static void bk_print_2_row() {
    fprintf(stderr, "|%43s|%16s|%7s|%6s|%5s|%5s|", "", "", "", "", "", "");
}

static void bk_print_3_row(uint32_t unused2, uint16_t rsize, uint8_t unused1, uint8_t padded, uint8_t order, uint8_t alloc) {
    fprintf(stderr, "|%33s0x%08x|%10s0x%04x|%2s0x%03x|%5s%01x|%2s0x%01x|%4s%01x|",
        "", unused2, "", rsize, "", unused1, "", padded, "", order, "", alloc);
}

static void bk_print_art_row(const char *point_str) {
    fprintf(stderr, "+-------------------------------------------+----------------+-------+------+-----+-----+");
}

static void bk_print_header(bud_header *hdr) {
    bk_print_art_row("header");
    fputs("\n", stderr);
    bk_print_1_row();
    fputs("\n", stderr);
    bk_print_2_row();
    fputs("\n", stderr);
    bk_print_3_row(hdr->unused2, hdr->rsize, hdr->unused1,
        hdr->padded, hdr->order, hdr->allocated);
    fputs("\n", stderr);
    bk_print_art_row("");
    fputs("\n", stderr);
}

static void bk_print_allocated() {
    fprintf(stderr, "+---------------------------------------------------------------------------------------+\n");
    fprintf(stderr, "|%87s|\n", "");
    fprintf(stderr, "|%40sPAYLOAD%40s|\n", "", "");
    fprintf(stderr, "|%87s|\n", "");
    fprintf(stderr, "+---------------------------------------------------------------------------------------+\n");
}

static void bk_print_free(bud_free_block *hdr) {
    uint64_t next = (uint64_t)hdr->next;
    uint64_t prev = (uint64_t)hdr->prev;
    fprintf(stderr, "+---------------------------------------------------------------------------------------+\n");
    fprintf(stderr, "|%31sNEXT: 0x%016lx%32s|\n", "", next, "");
    fprintf(stderr, "|%31sPREV: 0x%016lx%32s|\n", "", prev, "");
    fprintf(stderr, "|%87s|\n", "");
    fprintf(stderr, "+---------------------------------------------------------------------------------------+\n");
    // 1 + 31 + 8 + 16 + 32 + 1 = 89
}

static void bk_print_middle(bud_header *hdr) {
    if(hdr->allocated) {
        bk_print_allocated();
    }
    else {
        bk_print_free((bud_free_block*)hdr);
    }
}


void bud_blockprint(bud_header *hdr) {
    // debug
    info("header: %lx @ (%p)", *(uint64_t*)hdr, hdr);

    bk_print_header(hdr);
    bk_print_middle(hdr);
}

void bud_listprint(int i) {
    bud_free_block* blk_next = free_list_heads[i].next;

    if(blk_next == &free_list_heads[i]) {
        fprintf(stderr, "free_list_heads[%d]: %p(head) -> NONE\n", i, &free_list_heads[i]);
        return;
    }

    fprintf(stderr, "free_list_heads[%d]: %p(head) -> ", i, &free_list_heads[i]);
    while(blk_next != &free_list_heads[i]) {
        fprintf(stderr, "%p -> ", blk_next);
        blk_next = blk_next->next;
    }
    fprintf(stderr, "%p(head)\n", &free_list_heads[i]);
}