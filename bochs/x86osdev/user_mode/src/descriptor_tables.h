// 
// descriptor_tables.h - Defines the interface for initialising the GDT and IDT.
//                       Also defines needed structures.
//                       Based on code from Bran's kernel development tutorials.
//                       Rewritten for JamesM's kernel development tutorials.
//

#ifndef DESCRIPTOR_TABLES_H
#define DESCRIPTOR_TABLES_H

#include "common.h"

// Initialisation function is publicly accessible.
void init_descriptor_tables();

// Allows the kernel stack in the TSS to be changed.
void set_kernel_stack(u32int stack);

// This structure contains the value of one GDT entry.
// We use the attribute 'packed' to tell GCC not to change
// any of the alignment in the structure.
struct gdt_entry_struct
{
    u16int limit_low;           // The lower 16 bits of the limit.
    u16int base_low;            // The lower 16 bits of the base.
    u8int  base_middle;         // The next 8 bits of the base.
    u8int  access;              // Access flags, determine what ring this segment can be used in.
    u8int  granularity;
    u8int  base_high;           // The last 8 bits of the base.
} __attribute__((packed));

typedef struct gdt_entry_struct gdt_entry_t;

// This struct describes a GDT pointer. It points to the start of
// our array of GDT entries, and is in the format required by the
// lgdt instruction.
struct gdt_ptr_struct
{
    u16int limit;               // The upper 16 bits of all selector limits.
    u32int base;                // The address of the first gdt_entry_t struct.
} __attribute__((packed));

typedef struct gdt_ptr_struct gdt_ptr_t;

// A struct describing an interrupt gate.
struct idt_entry_struct
{
    u16int base_lo;             // The lower 16 bits of the address to jump to when this interrupt fires.
    u16int sel;                 // Kernel segment selector.
    u8int  always0;             // This must always be zero.
    u8int  flags;               // More flags. See documentation.
    u16int base_hi;             // The upper 16 bits of the address to jump to.
} __attribute__((packed));

typedef struct idt_entry_struct idt_entry_t;

// A struct describing a pointer to an array of interrupt handlers.
// This is in a format suitable for giving to 'lidt'.
struct idt_ptr_struct
{
    u16int limit;
    u32int base;                // The address of the first element in our idt_entry_t array.
} __attribute__((packed));

typedef struct idt_ptr_struct idt_ptr_t;

// A struct describing a Task State Segment.
struct tss_entry_struct
{
    u32int prev_tss;   // The previous TSS - if we used hardware task switching this would form a linked list.
    u32int esp0;       // The stack pointer to load when we change to kernel mode.
    u32int ss0;        // The stack segment to load when we change to kernel mode.
    u32int esp1;       // Unused...
    u32int ss1;
    u32int esp2;  
    u32int ss2;   
    u32int cr3;   
    u32int eip;   
    u32int eflags;
    u32int eax;
    u32int ecx;
    u32int edx;
    u32int ebx;
    u32int esp;
    u32int ebp;
    u32int esi;
    u32int edi;
    u32int es;         // The value to load into ES when we change to kernel mode.
    u32int cs;         // The value to load into CS when we change to kernel mode.
    u32int ss;         // The value to load into SS when we change to kernel mode.
    u32int ds;         // The value to load into DS when we change to kernel mode.
    u32int fs;         // The value to load into FS when we change to kernel mode.
    u32int gs;         // The value to load into GS when we change to kernel mode.
    u32int ldt;        // Unused...
    u16int trap;
    u16int iomap_base;

} __attribute__((packed));

typedef struct tss_entry_struct tss_entry_t;


// These extern directives let us access the addresses of our ASM ISR handlers.
extern void isr0 ();
extern void isr1 ();
extern void isr2 ();
extern void isr3 ();
extern void isr4 ();
extern void isr5 ();
extern void isr6 ();
extern void isr7 ();
extern void isr8 ();
extern void isr9 ();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void irq0 ();
extern void irq1 ();
extern void irq2 ();
extern void irq3 ();
extern void irq4 ();
extern void irq5 ();
extern void irq6 ();
extern void irq7 ();
extern void irq8 ();
extern void irq9 ();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();
extern void isr128();

#endif
