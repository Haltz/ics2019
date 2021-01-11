#include "proc.h"
#include <elf.h>
#include "fs.h"

#ifdef __ISA_AM_NATIVE__
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Phdr Elf64_Phdr
#else
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Phdr Elf32_Phdr
#endif

static uintptr_t loader(PCB *pcb, const char *filename) {
	Elf_Ehdr elf_header;
	// READ ELF HEADER
	ramdisk_read((void *)&elf_header, 0, sizeof(Elf_Ehdr));
	// Log("???");

	// define porgram header array to read program headers
	Elf_Phdr program_header[128];
	ramdisk_read((void *)program_header, elf_header.e_phoff, elf_header.e_phentsize * elf_header.e_phnum);
	// Log("???");

	// using the program header to load segment
	// such as data segment
	for (int i = 0; i < elf_header.e_phnum; i++) {
		// load type must be PT_LOAD
		if (program_header[i].p_type == PT_LOAD) {
			// copy the ramdisk's sengment to the NEMU's memory
			Log("???");
			ramdisk_read((void *)program_header[i].p_vaddr, program_header[i].p_offset, program_header[i].p_filesz);

			// assgin the uninitialized data segment to 0
			memset((void *)(program_header[i].p_vaddr + program_header[i].p_filesz), 0, program_header[i].p_memsz - program_header[i].p_filesz);
		}
	}
	return elf_header.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
	uintptr_t entry = loader(pcb, filename);
	Log("Jump to entry = %x", entry);
	((void (*)())entry)();
}

void context_kload(PCB *pcb, void *entry) {
	_Area stack;
	stack.start = pcb->stack;
	stack.end   = stack.start + sizeof(pcb->stack);

	pcb->cp = _kcontext(stack, entry, NULL);
}

void context_uload(PCB *pcb, const char *filename) {
	uintptr_t entry = loader(pcb, filename);

	_Area stack;
	stack.start = pcb->stack;
	stack.end   = stack.start + sizeof(pcb->stack);

	pcb->cp = _ucontext(&pcb->as, stack, stack, (void *)entry, NULL);
}
