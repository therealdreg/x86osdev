// 
// task.c - Implements the functionality needed to multitask.
//          Written for JamesM's kernel development tutorials.
//

#include "task.h"
#include "paging.h"

// The currently running task.
volatile task_t *current_task;

// The start of the task linked list.
volatile task_t *ready_queue;

// Some externs are needed to access members in paging.c...
extern page_directory_t *kernel_directory;
extern page_directory_t *current_directory;
extern void alloc_frame(page_t*,int,int);
extern u32int initial_esp;
extern u32int read_eip();

// The next available process ID.
u32int next_pid = 1;

void initialise_tasking()
{
    // Rather important stuff happening, no interrupts please!
    asm volatile("cli");

    // Relocate the stack so we know where it is.
    move_stack((void*)0xE0000000, 0x2000);

    // Initialise the first task (kernel task)
    current_task = ready_queue = (task_t*)kmalloc(sizeof(task_t));
    current_task->id = next_pid++;
    current_task->esp = current_task->ebp = 0;
    current_task->eip = 0;
    current_task->page_directory = current_directory;
    current_task->next = 0;
    current_task->kernel_stack = kmalloc_a(KERNEL_STACK_SIZE);

    // Reenable interrupts.
    asm volatile("sti");
}

void move_stack(void *new_stack_start, u32int size)
{
  u32int i;
  // Allocate some space for the new stack.
  for( i = (u32int)new_stack_start;
       i >= ((u32int)new_stack_start-size);
       i -= 0x1000)
  {
    // General-purpose stack is in user-mode.
    alloc_frame( get_page(i, 1, current_directory), 0 /* User mode */, 1 /* Is writable */ );
  }
  
  // Flush the TLB by reading and writing the page directory address again.
  u32int pd_addr;
  asm volatile("mov %%cr3, %0" : "=r" (pd_addr));
  asm volatile("mov %0, %%cr3" : : "r" (pd_addr));

  // Old ESP and EBP, read from registers.
  u32int old_stack_pointer; asm volatile("mov %%esp, %0" : "=r" (old_stack_pointer));
  u32int old_base_pointer;  asm volatile("mov %%ebp, %0" : "=r" (old_base_pointer));

  // Offset to add to old stack addresses to get a new stack address.
  u32int offset            = (u32int)new_stack_start - initial_esp;

  // New ESP and EBP.
  u32int new_stack_pointer = old_stack_pointer + offset;
  u32int new_base_pointer  = old_base_pointer  + offset;

  // Copy the stack.
  memcpy((void*)new_stack_pointer, (void*)old_stack_pointer, initial_esp-old_stack_pointer);

  // Backtrace through the original stack, copying new values into
  // the new stack.  
  for(i = (u32int)new_stack_start; i > (u32int)new_stack_start-size; i -= 4)
  {
    u32int tmp = * (u32int*)i;
    // If the value of tmp is inside the range of the old stack, assume it is a base pointer
    // and remap it. This will unfortunately remap ANY value in this range, whether they are
    // base pointers or not.
    if (( old_stack_pointer < tmp) && (tmp < initial_esp))
    {
      tmp = tmp + offset;
      u32int *tmp2 = (u32int*)i;
      *tmp2 = tmp;
    }
  }

  // Change stacks.
  asm volatile("mov %0, %%esp" : : "r" (new_stack_pointer));
  asm volatile("mov %0, %%ebp" : : "r" (new_base_pointer));
}

void switch_task()
{
    // If we haven't initialised tasking yet, just return.
    if (!current_task)
        return;

    // Read esp, ebp now for saving later on.
    u32int esp, ebp, eip;
    asm volatile("mov %%esp, %0" : "=r"(esp));
    asm volatile("mov %%ebp, %0" : "=r"(ebp));

    // Read the instruction pointer. We do some cunning logic here:
    // One of two things could have happened when this function exits - 
    //   (a) We called the function and it returned the EIP as requested.
    //   (b) We have just switched tasks, and because the saved EIP is essentially
    //       the instruction after read_eip(), it will seem as if read_eip has just
    //       returned.
    // In the second case we need to return immediately. To detect it we put a dummy
    // value in EAX further down at the end of this function. As C returns values in EAX,
    // it will look like the return value is this dummy value! (0x12345).
    eip = read_eip();

    // Have we just switched tasks?
    if (eip == 0x12345)
        return;

    // No, we didn't switch tasks. Let's save some register values and switch.
    current_task->eip = eip;
    current_task->esp = esp;
    current_task->ebp = ebp;
    
    // Get the next task to run.
    current_task = current_task->next;
    // If we fell off the end of the linked list start again at the beginning.
    if (!current_task) current_task = ready_queue;

    eip = current_task->eip;
    esp = current_task->esp;
    ebp = current_task->ebp;

    // Make sure the memory manager knows we've changed page directory.
    current_directory = current_task->page_directory;

    // Change our kernel stack over.
    set_kernel_stack(current_task->kernel_stack+KERNEL_STACK_SIZE);
    // Here we:
    // * Stop interrupts so we don't get interrupted.
    // * Temporarily put the new EIP location in ECX.
    // * Load the stack and base pointers from the new task struct.
    // * Change page directory to the physical address (physicalAddr) of the new directory.
    // * Put a dummy value (0x12345) in EAX so that above we can recognise that we've just
    //   switched task.
    // * Restart interrupts. The STI instruction has a delay - it doesn't take effect until after
    //   the next instruction.
    // * Jump to the location in ECX (remember we put the new EIP in there).
    asm volatile("         \
      cli;                 \
      mov %0, %%ecx;       \
      mov %1, %%esp;       \
      mov %2, %%ebp;       \
      mov %3, %%cr3;       \
      mov $0x12345, %%eax; \
      sti;                 \
      jmp *%%ecx           "
                 : : "r"(eip), "r"(esp), "r"(ebp), "r"(current_directory->physicalAddr));
}

int fork()
{
    // We are modifying kernel structures, and so cannot be interrupted.
    asm volatile("cli");

    // Take a pointer to this process' task struct for later reference.
    task_t *parent_task = (task_t*)current_task;

    // Clone the address space.
    page_directory_t *directory = clone_directory(current_directory);

    // Create a new process.
    task_t *new_task = (task_t*)kmalloc(sizeof(task_t));
    new_task->id = next_pid++;
    new_task->esp = new_task->ebp = 0;
    new_task->eip = 0;
    new_task->page_directory = directory;
    current_task->kernel_stack = kmalloc_a(KERNEL_STACK_SIZE);
    new_task->next = 0;

    // Add it to the end of the ready queue.
    // Find the end of the ready queue...
    task_t *tmp_task = (task_t*)ready_queue;
    while (tmp_task->next)
        tmp_task = tmp_task->next;
    // ...And extend it.
    tmp_task->next = new_task;

    // This will be the entry point for the new process.
    u32int eip = read_eip();

    // We could be the parent or the child here - check.
    if (current_task == parent_task)
    {
        // We are the parent, so set up the esp/ebp/eip for our child.
        u32int esp; asm volatile("mov %%esp, %0" : "=r"(esp));
        u32int ebp; asm volatile("mov %%ebp, %0" : "=r"(ebp));
        new_task->esp = esp;
        new_task->ebp = ebp;
        new_task->eip = eip;
        // All finished: Reenable interrupts.
        asm volatile("sti");

        // And by convention return the PID of the child.
        return new_task->id;
    }
    else
    {
        // We are the child - by convention return 0.
        return 0;
    }

}

int getpid()
{
    return current_task->id;
}

void switch_to_user_mode()
{
    // Set up our kernel stack.
    set_kernel_stack(current_task->kernel_stack+KERNEL_STACK_SIZE);
    
    // Set up a stack structure for switching to user mode.
    asm volatile("  \
      cli; \
      mov $0x23, %ax; \
      mov %ax, %ds; \
      mov %ax, %es; \
      mov %ax, %fs; \
      mov %ax, %gs; \
                    \
       \
      mov %esp, %eax; \
      pushl $0x23; \
      pushl %esp; \
      pushf; \
      pushl $0x1B; \
      push $1f; \
      iret; \
    1: \
      "); 
      
}
