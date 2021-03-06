#ifndef HETFS_H
#define HETFS_H

#include <linux/rbtree.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/rwsem.h>

#define MAX_DIFF 200000000
#define MAX_NAME 255

/*
 * Five categories, from faster to slower:
 *
 * nonrot (SSD)      disk or mirror
 * nonrot (SSD)      raidz
 * mixed nonrot+rot  anything (raidz makes little sense)
 * rot (HDD)         disk or mirror
 * rot (HDD)         raidz
 */

#define	METASLAB_ROTOR_VDEV_TYPE_SSD		0x01
#define	METASLAB_ROTOR_VDEV_TYPE_SSD_RAIDZ	0x02
#define	METASLAB_ROTOR_VDEV_TYPE_MIXED		0x04
#define	METASLAB_ROTOR_VDEV_TYPE_HDD		0x08
#define	METASLAB_ROTOR_VDEV_TYPE_HDD_RAIDZ	0x10

#define filp2name(filp) file_dentry(filp)->d_name.name

int _myprint = 0;
EXPORT_SYMBOL(_myprint);
static DECLARE_RWSEM(tree_sem);

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
    int to_rot;
    unsigned long long int deleted;
    struct list_head *read_reqs;
    struct list_head *write_reqs;
    struct rw_semaphore read_sem;
    struct rw_semaphore write_sem;
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
int print_tree(int, int);

#endif
