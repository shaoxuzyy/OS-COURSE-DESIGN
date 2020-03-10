#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#define DRIVE_SIZE 0x4000
#define MESG_SIZE 1024

int my_open(struct inode *inode,struct file *file); //inode文件的各种属性
int my_release(struct inode *inode,struct file *file);
ssize_t my_read(struct file *file,char __user *user,size_t t,loff_t *f);//size_t是long/int类型
ssize_t my_write(struct file *file,const char __user *user,size_t t,loff_t *f);//loff_t 是一个long long类型

char informesg[MESG_SIZE] = "easy drive for IO";
int drive_num;
char *dev_name = "easy keyboard IO";

//函数入口绑定
struct file_operations pstruct = {
    open:my_open,
    release:my_release,
    read:my_read,
    write:my_write,
};
//注册该设备
int my_init_module(void){
    int ret;
    ret = register_chrdev(0,dev_name,&pstruct);//为0时，内核动态分配一个设备号；设备名称；文件系统的接口指针
    if(ret <0){
        printk("failed to register my drive.\n");
        return -1;
    }
    else {
        printk("my drive has been registered!\n");
        printk("id: %d\n",ret);
        drive_num = ret;
		printk("%d\n",ret);
        return 0;
    }
}

void clean_module(void){
    unregister_chrdev(drive_num,dev_name);
    printk("unregister successful.\n");
}

int my_open(struct inode *inode,struct file *file){
    printk("open my_drive OK!\n");
    try_module_get(THIS_MODULE);//判断模块是否处于活动状态，然后通过local_inc()宏将引用数加1
    return 0;
}

int my_release(struct inode *inode,struct file *file)
{
    printk("easy IO released!\n");
    module_put(THIS_MODULE);//模块引用数减一
    return 0;
}

ssize_t my_read(struct file *file, char __user *user,size_t t,loff_t *f){
    if(copy_to_user(user,informesg,sizeof(informesg))){
		//copy_to_user 成功返回0，失败返回没有拷贝成功的字节数
		//从用户空间到内核空间的拷贝
        return -2;
    }
    return sizeof(informesg);
}

ssize_t my_write(struct file *file,const char __user *user,size_t t,loff_t *f)
{
    if(copy_from_user(informesg,user,sizeof(informesg))){
        return -3;
    }
    return sizeof(informesg);
}

MODULE_LICENSE("GPL");
module_init(my_init_module);
module_exit(clean_module);