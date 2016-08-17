#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/hetfs.h>

SYSCALL_DEFINE0(hetfs)
{
    struct rb_node *node;
    struct data *entry;
    struct analyze_request *pos, *n;
    for (node = rb_first(&hetfstree); node; node = rb_next(node)) {
        entry = rb_entry(node, struct data, node);
        printk("{HETFS} file: %s with hash:%s\n", entry->file, entry->hash);
        printk("[HETFS] READ req:\n");
        list_for_each_entry_safe(pos, n, &entry->read_reqs->list, list) {
            printk("[HETFS] start: %lld - end:%lld\n", pos->start_offset
                                                    ,pos->end_offset);
        }
        printk("[HETFS] WRITE req:\n");
        list_for_each_entry_safe(pos, n, &entry->write_reqs->list, list) {
            printk("[HETFS] start: %lld - end:%lld\n", pos->start_offset
                                                    ,pos->end_offset);
        }
    }

    return 0;
}
