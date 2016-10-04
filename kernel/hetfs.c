#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/hetfs.h>
#include <linux/init_task.h>

int print_tree(void) {
    struct rb_node *node;
    struct data *entry;
    struct analyze_request *posh, *nh;

    printk(KERN_EMERG "[ERROR]print hetfstree %p\n", init_task.hetfstree);
    if (RB_EMPTY_ROOT(init_task.hetfstree)) {
        exact = -4;
        printk(KERN_EMERG "[ERROR] exact empty root\n");
        return 1;
    }
    for (node = rb_first(init_task.hetfstree); node; node = rb_next(node)) {
        entry = rb_entry(node, struct data, node);
        printk(KERN_EMERG "[HETFS] file: %s,entry %p, node %p", entry->file, entry, node);
        //sha512print(entry->hash, 1);
        if (!list_empty(&entry->read_reqs.list))
            printk(KERN_EMERG "[HETFS] READ req:\n");
        list_for_each_entry_safe(posh, nh, &entry->read_reqs.list, list) {
            printk(KERN_EMERG "[HETFS] start: %lld - end:%lld\n", posh->start_offset
                                                    ,posh->end_offset);
        }
        if (!list_empty(&entry->write_reqs.list))
            printk(KERN_EMERG "[HETFS] WRITE req:\n");
        list_for_each_entry_safe(posh, nh, &entry->write_reqs.list, list) {
           printk(KERN_EMERG "[HETFS] start: %lld - end:%lld\n", posh->start_offset
                                                    ,posh->end_offset);
        }
    }
    return 0;
}
EXPORT_SYMBOL(print_tree);

SYSCALL_DEFINE0(hetfs)
{

    printk(KERN_EMERG "[HETFS]Start of hetfs\n");
    printk(KERN_EMERG "[HETFS] Start of hetfs\n");
    down_read(&tree_sem);
    print_tree();
    up_read(&tree_sem);
    printk(KERN_EMERG "[HETFS] End of hetfs\n");

    return 0;
}
