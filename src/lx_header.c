/*



*/

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "semblance.h"
#include "lx.h"

static void print_lx_flags(dword flags) {
    char buffer[1024];
    strcpy(buffer,"");

    if (flags & 0x00000001UL) strcpy(buffer, "Single data");
    if ((flags & 0x00000004UL) == 0) strcat(buffer, ", Global Initialization");
    if ((flags & 0x00000004UL) == 1) strcat(buffer, ", Per-process Initialization");
    if (flags & 0x00000010UL) strcat(buffer, ", No internal fixup");
    if (flags & 0x00000020UL) strcat(buffer, ", No external fixup");
    if      ((flags & 0x00000300UL) == 0) strcat(buffer, ", Unknown");
    else if ((flags & 0x00000300UL) == 0) strcat(buffer, ", Incompatible with PM windowing");
    else if ((flags & 0x00000300UL) == 0) strcat(buffer, ", Compatible with PM windowing");
    else if ((flags & 0x00000300UL) == 0) strcat(buffer, ", Uses PM windowing API");
    if (flags & 0x000020000UL) strcat(buffer, ", Not executeable (Errors during link)");
    if ((flags & 0x000040000UL) == 0) strcat(buffer, ", Program file");
    if ((flags & 0x000040000UL) == 1) strcat(buffer, ", DLL");
    if (flags & 0x000100000UL) strcat(buffer, ", Protected memory library module");
    if (flags & 0x000200000UL) strcat(buffer, ", Device driver");
    if      ((flags & 0x400000000UL) == 0) strcat(buffer, ", DLL global termination");
    else if ((flags & 0x400000000UL) == 1) strcat(buffer, ", DLL Per-process termination");

    if (buffer[0] == ',') strcpy(buffer,buffer+2); // Fixing prepending comma

    printf("Module flags: 0x%08x (%s)\n", flags, buffer);   
}

static void print_lx_object_flags(word flags, byte exetype) {
    // exetype == 0 : LE
    // exetype == 1 : LX

    char buffer[1024];
    
    if (flags & 0x0001) strcpy(buffer, "Readable object"); // FIXME - comma problem
    if (flags & 0x0002) strcat(buffer, ", Writeable object");
    if (flags & 0x0004) strcat(buffer, ", Executeable object");
    if (flags & 0x0008) strcat(buffer, ", Resource object");
    if (flags & 0x0010) strcat(buffer, ", Discardable object");
    if (flags & 0x0020) strcat(buffer, ", Sharable object");
    if (flags & 0x0040) strcat(buffer, ", Object has preload pages");
    if (flags & 0x0080) strcat(buffer, ", Object has invalid pages");
    if      ((flags & 0x0100) & (exetype == 0)) strcat(buffer, ", Object is permanent and swapable"); // LE
    else if ((flags & 0x0100) & (exetype == 1)) strcat(buffer, ", Object has zero fill"); // LX
    if (flags & 0x0200) strcat(buffer, ", Object is permanent and resident");
    if ((flags & 0x0300) & (exetype == 1)) strcat(buffer, ", Object is permanent and contiguous");
    if (flags & 0x0400) strcat(buffer, ", Object is permanent and lockable");
    if (flags & 0x1000) strcat(buffer, ", 16:16 alias required");
    if      ((flags & 0x2000) == 0) strcat(buffer, ", Default setting");
    else if ((flags & 0x2000) == 1) strcat(buffer, ", Big setting");
    if (flags & 0x4000) strcat(buffer, ", Conforming for code");
    if      ((flags & 0x8000) == 0) strcat(buffer, ", Object IO privilege level: 0");
    else if ((flags & 0x8000) == 1) strcat(buffer, ", Object IO privilege level: 1");

    if (buffer[0] == ',') strcpy(buffer,buffer+2); // Fixing prepending comma

    printf("Flags: 0x%04x (%s)\n", flags, buffer);
}

static void print_lx_object_page_table_flags(word flags) {
    char buffer[1024];

    if (flags == 0x0000) strcpy(buffer, "Legal Physical Page in the module (Offset from Preload Page Section)");
    if (flags == 0x0001) strcpy(buffer, "Iterated Data Page (Offset from Iterated Data Pages Section)");
    if (flags == 0x0002) strcpy(buffer, "Invalid Page (zero)");
    if (flags == 0x0003) strcpy(buffer, "Zero Filled Page (zero)");
    if (flags == 0x0004) strcpy(buffer, "Unused");
    if (flags == 0x0005) strcpy(buffer, "Compressed Page (Offset from Preload Pages Section)");

    printf("Flags: 0x%04x (%s)\n", flags, buffer);
}

static void print_lx_text_order(char *text, byte order) {
    char buffer[20];

    if (order == 0x00) strcpy(buffer, "Little endian");
    else               strcpy(buffer, "Big endian");

    printf("%s: %s\n", text, buffer);
}

static void print_lx_cpu_type(word cputype) {
    char buffer[1024];

    if      (cputype == 0x01) strcpy(buffer, "Intel 80286");
    else if (cputype == 0x02) strcpy(buffer, "Intel 80386");
    else if (cputype == 0x03) strcpy(buffer, "Intel 80486");
    else if (cputype == 0x20) strcpy(buffer, "Intel i860 (N10)");
    else if (cputype == 0x21) strcpy(buffer, "Intel N11");
    else if (cputype == 0x40) strcpy(buffer, "MIPS Mk 1 (R2000, R3000)");
    else if (cputype == 0x41) strcpy(buffer, "MIPS Mk 2 (R6000)");
    else if (cputype == 0x42) strcpy(buffer, "MIPS Mk 3 (R4000)");
    else                      strcpy(buffer, "Unknown CPU type");

    printf("CPU Type: %s\n", buffer);
}

static void print_lx_target_os(word os) {
    char buffer[1024];

    if      (os == 0x01) strcpy(buffer, "OS/2");
    else if (os == 0x02) strcpy(buffer, "Windows");
    else if (os == 0x03) strcpy(buffer, "DOS 4.0 (European)");
    else if (os == 0x04) strcpy(buffer, "Windows 386");

     printf("Target OS: %s\n", buffer);
}

static void print_lx_header(const struct header_lx *header) {
    putchar('\n');
    //printf("Linker version: %d.%d\n", NULL);
    print_lx_text_order("Byte order: ", header->byte_order);
    print_lx_text_order("Word order: ", header->word_order);
    printf("EXE Format level: %d\n", header->level);
    print_lx_cpu_type(header->cpu_type);
    print_lx_target_os(header->os_type);
    printf("EXE version: 0x%04x\n", header->version);
    print_lx_flags(header->flags);
    printf("Stack size: %d (0x%08x)\n", header->stacksize);
    printf("Heap size: %d (0x%08x)\n", header->heapsize);
    printf("Number of memory pages: %d\n", header->num_pages);
    printf("Initial object CS: %d (0x%08x)\n", header->start_obj, header->start_obj);
    printf("EIP: %d (0x%08x)\n", header->eip, header->eip);
    printf("Initial object SS: %d (0x%08x)\n", header->stack_obj, header->stack_obj);
    printf("ESP: %d (0x%08x)\n", header->esp, header->esp);
    printf("Memory page size: %d (0x%08x)\n", header->page_size, header->page_size);
    if (header->signature == 0x454C) printf("Size of last page: %d (0x%08x)\n", header->l.last_page, header->l.last_page); // LE
    else                         printf("Left shift page offset: %d (0x%08x)\n", header->l.page_shift, header->l.page_shift); // LX
    printf("Fix-up section size: %d (0x%08x)\n", header->fixup_size, header->fixup_size);
    if ((header->os_type == 0x02) | (header->os_type == 0x03)) printf("Heap size (16 bit): %d (0x%08x)\n", header->heapsize, header->heapsize);
    if (header->os_type == 0x04) printf("Stack size (OS/2): %d (0x%08x)\n", header->stacksize, header->stacksize);
    if (header->flags & 0x000200000UL) { 
        printf("Windows VxD device ID: %d (0x%04x)\n");
        printf("Windows VxD DDK version: (0x%04x)\n");
    }
    printf("");
    printf("");
    printf("");
    printf("");
    printf("");
    printf("");
    printf("");
}

static void print_lx_specfile(struct lx *lx) {

}

static void freelx(struct lx *lx) {

}

static void get_lxentry() {

}

static void get_lx_objects(off_t offset_lx, struct lx *lx) {
    off_t offset;
    int i;

    lx->object_tables = malloc(lx->object_tables_count * sizeof(struct lx_object_table));
    offset = offset_lx + lx->header->objtab_off;

    for (i = 0; i < lx->object_tables_count; i++) {
        memcpy(&lx->object_tables[i], read_data(offset + i * sizeof(struct lx_object_table)), sizeof(struct lx_object_table));
    }
}

static void get_lx_export_table(off_t offset_lx, struct lx *lx) {
    off_t offset;
    int i;

    //lx->lx_exports_table = malloc();
}

static void get_lx_import_table(off_t offset_lx, struct lx *lx) {
    off_t offset;
    char tempname[1024];
    int i, j;
    byte tmp;

    lx->imports_table = malloc(lx->header->num_impmods * sizeof(struct lx_imports));
    offset = offset_lx + lx->header->impmod_off;
    for (i = 0; i < lx->imports_count; i++) {
        tmp = read_byte(offset);
        offset++;
        memset(tempname, 0, 1024);
        for (j = 0; j < tmp; j++) {
            tempname[j] = read_byte(offset + j);
        }
        offset = offset + tmp - 1;
        lx->imports_table[i].name = tempname;
        lx->imports_table[i].ordinal = 0;
        printf("%s, %d\n", lx->imports_table[i].name, lx->imports_table[i].ordinal);
        offset++;
    }
}

static void readlx(off_t offset_lx, struct lx *lx) {
    off_t offset;

    lx->header = read_data(offset_lx);
    
    lx->object_tables_count = lx->header->num_pages;
    if (lx->object_tables_count > 0) {
        get_lx_objects(offset_lx, lx);
    }
    
    lx->imports_count = lx->header->num_impmods;
    if (lx->imports_count > 0) {
        get_lx_import_table(offset_lx, lx);
    }

    get_lx_export_table(offset_lx, lx);

    
    if (lx->header->signature == 0x454C) {
        // LE
        lx->type = "LE";
    }
    else if (lx->header->signature == 0x584C) {
        // LX
        lx->type = "LX";
    }
    printf("Magical girl %#04X\n", lx->header->signature);
    for (int i = 0; i < lx->imports_count; i++) {
        printf("%s\n",lx->imports_table[i].name);
    }
    //printf("Byte order: %#2X\n", lx->header->byte_order);
    //printf("Object count %d\n", lx->object_tables_count);
}

void dumplx(off_t offset_lx) {
    struct lx lx = {0};
    int i;
    
    readlx(offset_lx, &lx);

    if (mode == SPECFILE) {
        print_lx_specfile(&lx);
        freelx(&lx);
        return;
    }

    
    printf("Module type: %s (Linear Executable)\n", lx.type);

    if (lx.name)
        printf("Module name: %s\n", lx.name);
    
    if (mode & DUMPHEADER)
        print_lx_header(lx.header);
    
    if (mode & DUMPEXPORT) {
        putchar('\n');
    }

    if (mode & DUMPIMPORT) {
        putchar('\n');
        printf("Imported functions:\n");
        for (i = 0; i < lx.imports_count; i++) {
            printf("%s\n",lx.imports_table[i].name);
        }
    }

    if (mode & DISASSEMBLE)
    {

    }

    freelx(&lx);
}

