#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched/signal.h>
#include <linux/mm.h>
#include <linux/mm_types.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Process Memory Map Logger for Kernel 6.9+");

// Function to log memory map of a process
static void log_memory_map(struct task_struct *task, int depth) {
    struct mm_struct *mm = task->mm;
    struct vma_iterator vmi;
    struct vm_area_struct *vma;
    struct task_struct *child;
    struct list_head *list;

    if (!mm) return;

    // Print indentation based on depth level
    printk(KERN_INFO "%*s[PID %d] %s\n", depth * 4, "", task->pid, task->comm);

    mmap_read_lock(mm);
    vma_iter_init(&vmi, mm, 0);

    for (vma = vma_next(&vmi); vma; vma = vma_next(&vmi)) {
        printk(KERN_INFO "%*s ├── Start: %lx - End: %lx, Flags: %lx",
               (depth + 1) * 4, "", vma->vm_start, vma->vm_end, vma->vm_flags);
    }

    mmap_read_unlock(mm);

    // Recursively log child processes
    list_for_each(list, &task->children) {
        child = list_entry(list, struct task_struct, sibling);
        log_memory_map(child, depth + 1);
    }
}


// Kernel module init function
static int __init mem_logger_init(void) {
    struct task_struct *task;

    printk(KERN_INFO "Process Memory Logger Module: Initialization\n");

    // Start logging memory maps from top-level (init/systemd) processes
    for_each_process(task) {
        if (task->parent == &init_task) {  // Only consider top-level parent processes
            log_memory_map(task, 0);  // Start from depth 0
        }
    }

    return 0;
}


// Kernel module exit function
static void __exit mem_logger_exit(void) {
    printk(KERN_INFO "Process Memory Logger Module: Cleanup\n");
}

module_init(mem_logger_init);
module_exit(mem_logger_exit);
