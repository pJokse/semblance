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
    if (flags & 0x00000100UL) strcat(buffer, ", Incompatible with PM windowing");
    if (flags & 0x00000200UL) strcat(buffer, ", Compatible with PM windowing");
    if (flags & 0x00000300UL) strcat(buffer, ", Uses PM windowing API");
    if (flags & 0x00000400UL) strcat(buffer, ", Reserved");
    if (flags & 0x00000800UL) strcat(buffer, ", Reserved");
    if (flags & 0x00001000UL) strcat(buffer, ", Reserved");
    if (flags & 0x00002000UL) strcat(buffer, ", Module is not loadable");
    if (flags & 0x00004000UL) strcat(buffer, ", Reserved");
    if ((flags & 0x00038000UL) == 0x00000000) strcat(buffer, ", Program module");
    if ((flags & 0x00038000UL) == 0x00008000) strcat(buffer, ", Library module");
    if ((flags & 0x00038000UL) == 0x00018000) strcat(buffer, ", Protected Memory Library Module");
    if ((flags & 0x00038000UL) == 0x00020000) strcat(buffer, ", Physical Device Driver module");
    if ((flags & 0x00038000UL) == 0x00028000) strcat(buffer, ", Virtual Device Driver module");
    if ((flags & 0x00040000UL) == 0) strcat(buffer, ", Program file");
    if ((flags & 0x00040000UL) == 1) strcat(buffer, ", DLL");
    if (flags & 0x00004000UL) strcat(buffer, ", Reserved");
    if (flags & 0x00100000UL) strcat(buffer, ", Protected memory library module");
    if (flags & 0x00200000UL) strcat(buffer, ", Device driver");
    if      ((flags & 0x40000000UL) == 0) strcat(buffer, ", DLL global termination");
    else if ((flags & 0x40000000UL) == 1) strcat(buffer, ", DLL Per-process termination");

    if (buffer[0] == ',') strcpy(buffer,buffer+2); // Fixing prepending comma

    printf("Module flags: 0x%08x (%s)\n", flags, buffer);   
}

static void print_lx_object_flags(word flags, byte exetype) {
    // exetype == 0 : LE
    // exetype == 1 : LX

    char buffer[1024];
    
    if (flags & 0x0001) strcpy(buffer, "Readable object");
    if (flags & 0x0002) strcat(buffer, ", Writeable object");
    if (flags & 0x0004) strcat(buffer, ", Executeable object");
    if (flags & 0x0008) strcat(buffer, ", Resource object");
    if (flags & 0x0010) strcat(buffer, ", Discardable object");
    if (flags & 0x0020) strcat(buffer, ", Sharable object");
    if (flags & 0x0040) strcat(buffer, ", Object has preload pages");
    if (flags & 0x0080) strcat(buffer, ", Object has invalid pages");
    if (flags & 0x0100) {
        if (exetype == 0) strcat(buffer, ", Object is permanent and swapable"); // LE
        else if (exetype == 1) strcat(buffer, ", Object has zero fill"); // LX
    }
    if (flags & 0x0200) strcat(buffer, ", Object is permanent and resident");
    if ((flags & 0x0300) & (exetype == 1)) strcat(buffer, ", Object is permanent and contiguous");
    if (flags & 0x0400) strcat(buffer, ", Object is permanent and lockable");
    if (flags & 0x1000) strcat(buffer, ", 16:16 alias required");
    if ((flags & 0x2000)) strcat(buffer, ", Big setting");
    else strcat(buffer, ", Default setting");
    if (flags & 0x4000) strcat(buffer, ", Conforming for code");
    if (flags & 0x1000) {
        if (flags & 0x8000) strcat(buffer, ", Object IO privilege level: 1");
        else strcat(buffer, ", Object IO privilege level: 0"); 
    }

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
    printf("Stack size: %d (0x%08x)\n", header->stacksize, header->stacksize);
    printf("Heap size: %d (0x%08x)\n", header->heapsize, header->heapsize);
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
        //printf("Windows VxD device ID: %u (0x%04x)\n");
        //printf("Windows VxD DDK version: (0x%04x)\n");
    }
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

    offset = offset_lx + lx->header->objtab_off;
    lx->object_tables = malloc(lx->object_tables_count * sizeof(struct lx_object_table));
    for (i = 0; i < lx->header->num_objects; i++) {
        lx->object_tables[i].bytes_in_segment = read_dword(offset);
        offset = offset + 4;
        lx->object_tables[i].relocation_base_address = read_dword(offset);
        offset = offset + 4;
        lx->object_tables[i].flags = read_dword(offset);
        offset = offset + 4;
        lx->object_tables[i].page_map_index = read_dword(offset);
        offset = offset + 4;
        lx->object_tables[i].page_map_entries = read_dword(offset);
        offset = offset + 4;
        lx->object_tables[i].reserved = read_dword(offset);
        offset = offset + 4;
        //printf("object %lu 0x%08x 0x%08x 0x%08x ", i, lx->object_tables[i].bytes_in_segment, lx->object_tables[i].relocation_base_address, lx->object_tables[i].flags);
        //printf("0x%08x 0x%08x 0x%08x\n", lx->object_tables[i].page_map_index, lx->object_tables[i].page_map_entries, lx->object_tables[i].reserved);
    }
}

static void get_lx_object_pages(off_t offset_lx, struct lx *lx) {
    off_t offset;
    int i, j;
    char tmp[3];

    offset = offset_lx + lx->header->objmap_off;
    //printf("marffset 0x%08x 0x%08x 0x%08x\n", offset_lx, offset, lx->header->objmap_off);
    //printf("maffle waffle 0x%08x\n",lx->header->page_off);
    //printf("Ganz genau so broken\n"); // FIX THIS FUNCTION, IT'S BROKEN FOR AT LEAST SOME LE FILES
    lx->object_page_tables = malloc(lx->objects_page_count * sizeof(struct lx_object_page_table));
    for (i = 0; i < lx->objects_page_count; i++) {
        if (lx->header->signature == 0x584C) { // LX
            lx->object_page_tables[i].page_data_offset = read_dword(offset);
            offset = offset + 4;
            lx->object_page_tables[i].data_size = read_word(offset);
            offset = offset + 2;
            lx->object_page_tables[i].flags = read_word(offset);
            offset = offset + 2;
        }
        else if (lx->header->signature == 0x454C) { // LE
            tmp[0] = read_byte(offset);
            tmp[1] = read_byte(offset + 1);
            tmp[2] = read_byte(offset + 2);
            offset = offset + 4;
            lx->object_page_tables[i].page_data_offset = (dword)(tmp[0] << 24 | tmp[1] << 16 | tmp[2] << 8) >> 8;
            lx->object_page_tables[i].flags = tmp[3];
            lx->object_page_tables[i].data_size = 0;
            //printf("LE %08x\n", (tmp[0] << 24 | tmp[1] << 16 | tmp[2] << 8) >> 8);
        }
        //printf("object page 0x%02x 0x%08x 0x%04x 0x%04x\n", i + 1, lx->object_page_tables[i].page_data_offset, lx->object_page_tables[i].data_size, lx->object_page_tables[i].flags);
    }

}

static void get_lx_export_tables(off_t offset_lx, struct lx *lx) {
    off_t offset;
    int count, i, j, len, ordinal;
    char buffer[1024];
    count = 0;

    offset = offset_lx + lx->header->resname_off;
    if (lx->header->resname_off > 0) {
        while (read_byte(offset) != '\0') {
            len = read_byte(offset);
            offset++;
            for (i = 0;i < len; i++) {
                buffer[i] = read_byte(offset);
                offset++;
            }
            ordinal = read_word(offset);
            offset = offset + 2;
            count++;
        }
        lx->resident_exports_count = count;
        lx->resident_exports_table = malloc(lx->resident_exports_count * sizeof(struct lx_exports));
    
        memset(buffer, 0, 1024);
        offset = offset_lx + lx->header->resname_off;
        for (i = 0; i < lx->resident_exports_count; i++) {
            len = read_byte(offset);
            offset++;
            for (j = 0; j < len; j++) {
                buffer[j] = read_byte(offset);
                offset++;
            }
            ordinal = read_word(offset);
            offset = offset + 2;
            lx->resident_exports_table[i].name = strdup(buffer);
            lx->resident_exports_table[i].ordinal = ordinal;
            memset(buffer, 0, 1024);
        }
    }

    offset = lx->header->nonres_off;
    if (lx->header->nonres_off > 0) {
        memset(buffer, 0, 1024);
        count = 0;

        while (read_byte(offset) != '\0') {
            len = read_byte(offset);
            offset++;
            for (i = 0;i < len; i++) {
                buffer[i] = read_byte(offset);
                offset++;
            }
            ordinal = read_word(offset);
            offset = offset + 2;
            count++;
        }
        lx->nonresident_exports_count = count;
        lx->nonresident_exports_table = malloc(lx->resident_exports_count * sizeof(struct lx_exports));

        memset(buffer, 0, 1024);
        offset = lx->header->nonres_off;
        for (i = 0; i < lx->nonresident_exports_count; i++) {
            len = read_byte(offset);
            offset++;
            for (j = 0; j < len; j++) {
                buffer[j] = read_byte(offset);
                offset++;
            }
            ordinal = read_word(offset);
            offset = offset + 2;
            lx->nonresident_exports_table[i].name = strdup(buffer);
            lx->nonresident_exports_table[i].ordinal = ordinal;
            memset(buffer, 0, 1024);
        }
    }
}

static void get_lx_fixup_tables(off_t offset_lx, struct lx *lx) {
    off_t offset;
    int i;
    
    offset = lx->header->fixpage_off;
    //lx->fixups = malloc(lx->header->f* sizeof(struct lx_fixups));
    offset = lx->header->fixrec_off;
}

byte is_lxorle(struct lx *lx) { // EXETYPE 0 = LE, 1 = LX
    if (lx->header->signature == 0x454C) return 0x00; // LE
    else if (lx->header->signature == 0x584C) return 0x01; // LX
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
        lx->imports_table[i].name = strdup(tempname);
        lx->imports_table[i].ordinal = 0;
        //printf("%s, %d\n", lx->imports_table[i].name, lx->imports_table[i].ordinal);
        offset++;
    }
}

static void print_lx_objects(off_t offset_lx, struct lx *lx) {
    off_t offset;
    int i, j;

    printf("Objects:\n");
    for(i = 0; i < lx->object_tables_count; i++) {
        printf("Object 0x%04x (%u)\n", i, i);
        printf("Virtual size: 0x%08x (%u)\n", lx->object_tables[i].bytes_in_segment, lx->object_tables[i].bytes_in_segment);
        printf("Relocation base: 0x%08x (%u)\n", lx->object_tables[i].relocation_base_address, lx->object_tables[i].relocation_base_address);
        printf("Page table index 0x%08x (%u)\n", lx->object_tables[i].page_map_index, lx->object_tables[i].page_map_index);
        printf("Page table count: 0x%04x (%u)\n", lx->object_tables[i].page_map_entries, lx->object_tables[i].page_map_entries);
        print_lx_object_flags(lx->object_tables[i].flags, is_lxorle(lx));
        if (lx->object_tables[i].page_map_entries > 0) {
            for (j = 0; j < lx->object_tables[i].page_map_entries; j++) {
                printf("- Object page 0x%04x (%u)\n", lx->object_tables[i].page_map_index + j, lx->object_tables[i].page_map_index + j);
            }
        }
        putchar('\n');
    }
}

static void print_lx_objects_map(off_t offset_lx, struct lx *lx) {
    off_t offset, offset2;
    int i, j;

    printf("Object pages:\n");

    offset = lx->header->page_off;
    for(i = 0; i < lx->objects_page_count; i++) {
        printf("Page 0x%04x (%u)\n", i, i);
        //printf("moffset 0x%08x (%u)\n", lx->header->page_off, lx->header->page_off);
         if (lx->object_page_tables[i].flags & 0x0003 | lx->object_page_tables[i].flags & 0x0002) {
                // zero filled
        }
        if (lx->object_page_tables[i].flags == 0x0000) {
                 // Offset from preload
                if (i > 0) {
                    if (lx->object_page_tables[i - 1].flags & 0x0001) {
                        offset = offset + 0x1000;
                    }
                    else { 
                        offset = offset + lx->object_page_tables[i - 1].data_size;
                    }
                }
                else {
                    offset = offset + lx->object_page_tables[i].data_size;
                }
                //printf("marfset 0x%08x (%lu)\n", lx->object_page_tables[i].page_data_offset, lx->object_page_tables[i].page_data_offset);
        }
        if (lx->object_page_tables[i].flags & 0x0001) {
                // Offset from Iterated Data Section
            if (is_lxorle(lx) == 0) { //LE
                //offset = lx->object_page_tables[i].page_data_offset;
                //printf("møff\n");
            }
            else { //LX
                if (i > 0) { 
                    //offset = offset + lx->object_page_tables[i - 1].data_size;
                    offset = lx->header->page_off;
                }
                else {
                    offset = offset + lx->object_page_tables[i - 1].data_size;
                }
                //offset = lx->header->page_off << lx->object_page_tables[i].page_data_offset;
                //printf("møfset %lu\n", lx->object_page_tables[i].page_data_offset);
            }
        }
        if (lx->object_page_tables[i].flags & 0x04) {
                // Range of pages
        }
        printf("Page index: 0x%08x (%u)\n", lx->object_page_tables[i].page_data_offset, lx->object_page_tables[i].page_data_offset);
        /*if (lx->object_page_tables[i].flags & 0x0001) { 
            printf("Offset: 0x%08x (%lu)\n", lx->header->page_off, lx->header->page_off);
        }
        else {
            printf("Offset: 0x%08x (%lu)\n", offset, offset);
        }*/
        printf("Offset: 0x%08lx (%lu)\n", offset, offset);
        printf("Size: 0x%08x (%u)\n", lx->object_page_tables[i].data_size, lx->object_page_tables[i].data_size);
        print_lx_object_page_table_flags(lx->object_page_tables[i].flags);
        putchar('\n');
    }

}

static void readlx(off_t offset_lx, struct lx *lx) {
    off_t offset;

    lx->header = read_data(offset_lx);

    //printf("VXD DDK %04x\n", lx->header->r.vxd.DDK_version);
    printf("hest1\n");
    lx->object_tables_count = lx->header->num_objects;
    if (lx->object_tables_count > 0) {
        get_lx_objects(offset_lx, lx);
    }
    printf("hest2\n");
    lx->objects_page_count = lx->header->num_pages;
    if (lx->objects_page_count > 0) {
        printf("hest222\n");
        get_lx_object_pages(offset_lx, lx);
    }
    printf("hest3\n");
    lx->imports_count = lx->header->num_impmods;
    if (lx->imports_count > 0) {
        get_lx_import_table(offset_lx, lx);
    }
    printf("hest4\n");
    get_lx_export_tables(offset_lx, lx);

    printf("hest5\n");
    get_lx_fixup_tables(offset_lx, lx);

    
    if (lx->header->signature == 0x454C) {
        // LE
        lx->type = "LE";
    }
    else if (lx->header->signature == 0x584C) {
        // LX
        lx->type = "LX";
    }
    //printf("Magical girl %#04X\n", lx->header->signature);
    //printf("Byte order: %#2X\n", lx->header->byte_order);
    //printf("Object count %d\n", lx->object_tables_count);
}

void dumplx(off_t offset_lx) {
    struct lx lx = {0};
    int i;
    printf("hest\n");
    readlx(offset_lx, &lx);
    printf("hest2\n");
    if (mode == SPECFILE) {
        print_lx_specfile(&lx);
        freelx(&lx);
        return;
    }

    
    printf("Module type: %s (Linear Executable)\n", lx.type);

    if (lx.name)
        printf("Module name: %s\n", lx.name);
    
    if (mode & DUMPHEADER) {
        printf("MARFG\n");
        print_lx_header(lx.header);
    }

    if (mode & DUMPEXPORT) {
        if (lx.resident_exports_count > 0) {
            putchar('\n');
            printf("Resident exports:\n");
            for (i = 0; i < lx.resident_exports_count; i++) {
                printf("%s, %u\n", lx.resident_exports_table[i].name, lx.resident_exports_table->ordinal);
            }
        }
        if (lx.nonresident_exports_count > 0) {
            putchar('\n');
            printf("Non-resident exports:\n");
            for (i = 0; i < lx.nonresident_exports_count; i++) {
                printf("%s, %u\n", lx.nonresident_exports_table[i].name, lx.nonresident_exports_table[i].ordinal);
            }
        }
    }

    if (mode & DUMPIMPORT) {
        if (lx.imports_count >0) {
            putchar('\n');
            printf("Imported functions:\n");
            for(i = 0; i < lx.imports_count; i++) {
                printf("%s\n",lx.imports_table[i].name);
            }
        }
    }

    if (mode & DISASSEMBLE) {
        putchar('\n');
        print_lx_objects(offset_lx, &lx);
        print_lx_objects_map(offset_lx, &lx);
    }

     if (mode & DUMPRSRC) {

     }

    freelx(&lx);
}

