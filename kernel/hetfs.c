#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/hetfs.h>
#include <linux/het.h>
#include <linux/inhet.h>
#include <linux/init_task.h>

int print_tree(void) {
    struct rb_node *node;
    struct data *entry;
    struct analyze_request *posh, *nh;
    int all_nodes, all_requests, requests;

    all_nodes = all_requests = requests = 0;

    if (RB_EMPTY_ROOT(init_task.hetfstree)) {
        __exact = -4;
        printk(KERN_EMERG "[ERROR] __exact empty root\n");
        return 1;
    }
    for (node = rb_first(init_task.hetfstree); node; node = rb_next(node)) {
        ++all_nodes;
        entry = rb_entry(node, struct data, node);
        printk(KERN_EMERG "[HETFS] file: %s\n", entry->file);
        //sha512print(entry->hash, 1);
        if (!list_empty(entry->read_reqs))
            printk(KERN_EMERG "[HETFS] READ req:\n");
        list_for_each_entry_safe(posh, nh, entry->read_reqs, list) {
            all_requests += posh->times;
            printk(KERN_EMERG "[HETFS] start: %lld - end:%lld times:%d\n",
                            posh->start_offset, posh->end_offset, posh->times);
        }
        if (!list_empty(entry->write_reqs))
            printk(KERN_EMERG "[HETFS] WRITE req:\n");
        list_for_each_entry_safe(posh, nh, entry->write_reqs, list) {
            all_requests += posh->times;
            printk(KERN_EMERG "[HETFS] start: %lld - end:%lld times:%d\n",
                            posh->start_offset, posh->end_offset, posh->times);
        }
    }
    printk(KERN_EMERG "[HETFS]Tree Nodes:%d, requests:%d\n", all_nodes, all_requests);
    return 0;
}
EXPORT_SYMBOL(print_tree);

struct list_head *zip_list(struct list_head *general)
{
    struct list_head *pos, *n, *pos1, *new;
    struct analyze_request *areq, *areq1;
    int found;

    new = kzalloc(sizeof(struct list_head), GFP_KERNEL);
    if (new == NULL)
        return NULL;
    INIT_LIST_HEAD(new);

    list_for_each_safe(pos, n, general) {
        found = 0;
        areq = list_entry(pos, struct analyze_request, list);
        list_for_each(pos1, new){
            areq1 = list_entry(pos1, struct analyze_request, list);
            if (areq->start_offset == areq1->start_offset &&
                areq->end_offset == areq1->end_offset) {
                areq1->times += areq->times;
                found = 1;
                break;
            }
        }
        if (!found)
            list_move_tail(pos,new);
    }
    list_for_each_safe(pos, n, general) {
        areq = list_entry(pos, struct analyze_request, list);
        list_del(pos);
        kfree(areq);
    }
    kfree(general);
    return new;
}

void analyze(struct data* InsNode)
{
    struct list_head *pos, *n;
    struct analyze_request *areq;
    loff_t part, half;
    int mid, all = 0;
    half = InsNode->size >> 1;
    if (!list_empty(InsNode->read_reqs)) {
        InsNode->to_rot = 0;
        InsNode->read_reqs = zip_list(InsNode->read_reqs);
        printk(KERN_EMERG "[HETFS]File %s\n", InsNode->file);
        list_for_each_safe(pos, n, InsNode->read_reqs) {
            areq = list_entry(pos, struct analyze_request, list);
            part = areq->end_offset - areq->start_offset;
            InsNode->read_all_file += areq->times;
            if (part == InsNode->size) {
                all += areq->times;
            }
            else if (part >= half) {
                printk(KERN_EMERG "[HETFS] This part is a big read start %lld end %lld accessed %d times\n",
                        areq->start_offset, areq->end_offset, areq->times);
            }
        }
        mid = InsNode->read_all_file >> 1;
        if (all > 0 && (((all & 1) && all > mid) || (!(all & 1) && all >= mid))) {
            InsNode->to_rot |= METASLAB_ROTOR_VDEV_TYPE_HDD;
            printk(KERN_EMERG "[HETFS] It was read sequentially\n");
        }
        else {
            InsNode->to_rot |= METASLAB_ROTOR_VDEV_TYPE_SSD;
        }
    }
    if (!list_empty(InsNode->write_reqs)) {
        InsNode->write_reqs = zip_list(InsNode->write_reqs);
        all = 0;
        list_for_each_safe(pos, n, InsNode->write_reqs) {
            areq = list_entry(pos, struct analyze_request, list);
            part = areq->end_offset - areq->start_offset;
            InsNode->write_all_file += areq->times;
            if (part == InsNode->size)
                all++;
            else if (part >= half) {
                printk(KERN_EMERG "[HETFS] This part is a big write start %lld end %lld accessed %d times\n",
                        areq->start_offset, areq->end_offset, areq->times);
            }
        }
        mid = InsNode->write_all_file >> 1;
        if (all > 0 && (((all & 1) && all > mid) || (!(all & 1) && all >= mid)))
            printk(KERN_EMERG "[HETFS] It was write sequentially\n");
    }
}


SYSCALL_DEFINE0(chprint)
{
    _myprint = (_myprint)?0:1;
    return 0;
}

SYSCALL_DEFINE0(pprint)
{
    printk(KERN_EMERG "_myprint is %s\n", _myprint ? "true" : "false");
    return 0;
}

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

SYSCALL_DEFINE0(analyze){

    struct rb_node *node;
    struct data *entry;
    printk(KERN_EMERG "[HETFS]Start of analyze\n");
    down_read(&tree_sem);
    /*We actually write to nodes inthe tree but no insert or delete*/
    for (node = rb_first(init_task.hetfstree); node; node = rb_next(node)) {
        entry = rb_entry(node, struct data, node);
        analyze(entry);
    }
    up_read(&tree_sem);
    printk(KERN_EMERG "[HETFS] End of analyze\n");

    return 0;
}
