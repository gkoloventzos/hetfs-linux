#ifndef HETFS_H
#define HETFS_H

#include <linux/rbtree.h>
#include <linux/list.h>
#include <linux/types.h>

struct analyze_request {
    long long start_offset;
    long long end_offset;
    unsigned long long int start_time;
    unsigned long long int end_time;
    int times;
    struct list_head list;
};

struct data {
	char *hash;
    char *file;
	loff_t size;
    int read_all_file;
    int write_all_file;
    int read_seq;
    int write_seq;
    unsigned long long int deleted;
    struct list_head *read_reqs;
    struct list_head *write_reqs;
    struct dentry *dentry;
    struct rb_node node;
};

struct kdata {
    struct dentry *dentry;
    loff_t offset;
    long length;
    int type;
    unsigned long long int time;
};

struct data *rb_search(struct rb_root *, char *);
int rb_insert(struct rb_root *, struct data *);
int print_tree(void);

#endif
