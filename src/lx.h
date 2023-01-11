#ifndef __LX_H
#define __LX_H

#include "semblance.h"

#define OSF_FLAT_RESERVED 20

#pragma pack(1)

struct header_lx {
    word            signature;
    byte            byte_order;     /* the byte ordering of the .exe */
    byte            word_order;     /* the word ordering of the .exe */
    dword           level;          /* the exe format level */
    word            cpu_type;       /* the cpu type */
    word            os_type;        /* the operating system type */
    dword           version;        /* .exe version */
    dword           flags;          /* .exe flags */
    dword           num_pages;      /* # of pages in .exe */
    dword           start_obj;      /* starting object number */
    dword           eip;            /* starting value of eip */
    dword           stack_obj;      /* object # for stack pointer */
    dword           esp;            /* starting value of esp */
    dword           page_size;      /* .exe page size */
    union {
        dword           last_page;      /* size of last page - LE */
        dword           page_shift;     /* left shift for page offsets - LX */
    } l;
    dword           fixup_size;     /* fixup section size */
    dword           fixup_cksum;    /* fixup section checksum */
    dword           loader_size;    /* loader section size */
    dword           loader_cksum;   /* loader section checksum */
    dword           objtab_off;     /* object table offset */
    dword           num_objects;    /* number of objects in .exe */
    dword           objmap_off;     /* object page map offset */
    dword           idmap_off;      /* iterated data map offset */
    dword           rsrc_off;       /* offset of resource table */
    dword           num_rsrcs;      /* number of resource entries */
    dword           resname_off;    /* offset of resident names table */
    dword           entry_off;      /* offset of entry table */
    dword           moddir_off;     /* offset of module directives table */
    dword           num_moddirs;    /* number of module directives */
    dword           fixpage_off;    /* offset of fixup page table */
    dword           fixrec_off;     /* offset of fixup record table */
    dword           impmod_off;     /* offset of import module name table */
    dword           num_impmods;    /* # of entries in import mod name tbl */
    dword           impproc_off;    /* offset of import procedure name table */
    dword           cksum_off;      /* offset of per-page checksum table */
    dword           page_off;       /* offset of enumerated data pages */
    dword           num_preload;    /* number of preload pages */
    dword           nonres_off;     /* offset of non-resident names table */
    dword           nonres_size;    /* size of non-resident names table */
    dword           nonres_cksum;   /* non-resident name table checksum */
    dword           autodata_obj;   /* object # of autodata segment */
    dword           debug_off;      /* offset of the debugging information */
    dword           debug_len;      /* length of the debugging info */
    dword           num_inst_preload;   /* # of instance pages in preload sect*/
    dword           num_inst_demand;    /*# instance pages in demand load sect*/
    dword           heapsize;       /* size of heap - for 16-bit apps */
    dword           stacksize;      /* size of stack OS/2 only */
    union {                         /* pad to 196 bytes. */
        byte            reserved[ OSF_FLAT_RESERVED ];
        struct {
            byte            reserved1[8];   /* +0xB0 */
            dword           winresoff;        /* +0xB8 Windows VxD version info resource offset */
            dword           winreslen;        /* +0xBC Windows VxD version info resource lenght */
            word            device_ID;        /* +0xC0 Windows VxD device ID */
            word            DDK_version;      /* +0xC2 Windows VxD DDK version (0x030A) */
        } vxd;
    } r;
};

//STATIC_ASSERT(sizeof(struct header_lx) == 0xC4);

#pragma pack()

struct le_map_entry {
    byte page_num[3];  /* 24-bit page number in .exe file */
    byte flags;
};

struct lx_map_entry {
    dword page_offset; /* offset from Preload page start shifted by page_shift in hdr */
    word data_size; /* # bytes in this page */
    word flags;
};

typedef union {
    struct le_map_entry le;
    struct lx_map_entry lx;
} lx_mapentry;

struct lx_object_table {
    dword bytes_in_segment;
    dword relocation_base_address;
    dword flags;
    dword page_map_index;
    dword page_map_entries;
    dword reserved;
};

struct lx_object_page_table {
    dword page_data_offset;
    word data_size;
    word flags;
};

struct lx_resource_table {
    word type;
    word name_id;
    dword resource_size;
    word object_count;
    word offset;
};

struct lx_exports {
    char *name;
    word ordinal;
};

struct lx_imports {
    char *name;
    word ordinal;
};

struct lx_module {
    word direct;
    word length;
    word offset;
};

struct lx {
    const struct header_lx *header;
    char *name;
    char *description;

    char *type;

    struct lx_object_table *object_tables;
    unsigned int object_tables_count;

    struct lx_object_page_table *object_page_tables;
    unsigned int object_page_table_count;
    
    struct lx_resource_table *resource_tables;
    unsigned int resources_count;

    struct lx_exports *exports_table;
    unsigned int exports_count;

    struct lx_imports *imports_table;
    unsigned int imports_count;
};

#endif /* __LX_H */