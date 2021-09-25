#include <linux/syscalls.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/kallsyms.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <asm/unistd.h>
#include <linux/cred.h>

MODULE_LICENSE("GPL");

char ban_fname[128] = {0x0,};
int ban_uid = -1;
int protect_uid = -1;
void ** sctable ;
int count = 0 ;
int f = 0;

asmlinkage long (*orig_sys_kill)(pid_t pid, int sig);

asmlinkage long mousehole_sys_kill(pid_t pid, int sig)
{
	struct task_struct * t ;
	for_each_process(t){
		if(protect_uid == t->cred->uid.val){
			printk("Process %d is protect!!",pid);
			return -1;
		}
	}
	return orig_sys_kill(pid,sig);
}

asmlinkage int (*orig_sys_open)(const char __user * filename, int flags, umode_t mode) ; 

asmlinkage int mousehole_sys_open(const char __user * filename, int flags, umode_t mode)
{
	char fname[256] ;
	char *ptr;
	copy_from_user(fname, filename, 256) ;
	if(ban_uid == current->cred->uid.val){
		ptr = strstr(fname,ban_fname);
		if(ptr != 0x0){
			printk("%d-%s deny!!",ban_uid,ban_fname);
			return -1;
		}
	}

	return orig_sys_open(filename, flags, mode) ;
}


static 
int mousehole_proc_open(struct inode *inode, struct file *file) {
	return 0 ;
}

static 
int mousehole_proc_release(struct inode *inode, struct file *file) {
	return 0 ;
}

static
ssize_t mousehole_proc_read(struct file *file, char __user *ubuf, size_t size, loff_t *offset) 
{
	char buf[256] ;
	ssize_t toread ;

	sprintf(buf, "Ban UID : %d\nBan fname : %s\nProtect UID : %d\n", ban_uid, ban_fname,protect_uid) ;

	toread = strlen(buf) >= *offset + size ? size : strlen(buf) - *offset ;

	if (copy_to_user(ubuf, buf + *offset, toread))
		return -EFAULT ;	

	*offset = *offset + toread ;
	
	return toread;
}
static 
ssize_t mousehole_proc_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offset)
{	
		
	char buf[128] = {"0x0"};
	
	if (*offset != 0 || size > 128)
		return -EFAULT ;
	//if f = 0 -> this input is uid
	//if f = 1 -> this input is filename	
	if(f==0){
		if(copy_from_user(buf, ubuf, size))
			return -EFAULT ;
		int len = strlen(buf);
		int i;
		//check this input is ban_uid or protect_uid
		if(buf[0] == '1'){
			for(i=0;i<len-2;i++){
				buf[i] = buf[i+2];
			}
			buf[len-2] = 0;
			sscanf(buf,"%d",&ban_uid);
			printk(KERN_INFO "ban_uid : %d",ban_uid);
			f = 1;
		}
		else{
			for(i=0;i<len-2;i++){
				buf[i] = buf[i+2];
			}
			buf[len-2] = 0;
			sscanf(buf,"%d",&protect_uid);
			printk(KERN_INFO "protect_uid : %d",protect_uid);
		}
		
	
		*offset = strlen(buf);
		
		return *offset;
	}
	//filename input
	else{
		if(copy_from_user(buf, ubuf, size))
			return -EFAULT ;
		sscanf(buf,"%s",ban_fname);
                printk(KERN_INFO "ban_fname : %s",ban_fname);

		*offset = strlen(buf);
		f = 0;	
		return *offset;
	}	
}

static const struct file_operations mousehole_fops = {
	.owner = 	THIS_MODULE,
	.open = 	mousehole_proc_open,
	.read = 	mousehole_proc_read,
	.write = 	mousehole_proc_write,
	.llseek = 	seq_lseek,
	.release = 	mousehole_proc_release,
} ;

static 
int __init mousehole_init(void) {
	unsigned int level ; 
	pte_t * pte ;
	printk(KERN_INFO "NOW mousehole is working!!");
	proc_create("mousehole", S_IRUGO | S_IWUGO, NULL, &mousehole_fops) ;

	sctable = (void *) kallsyms_lookup_name("sys_call_table") ;

	orig_sys_open = sctable[__NR_open] ;
	orig_sys_kill = sctable[__NR_kill] ;

	pte = lookup_address((unsigned long) sctable, &level) ;
	if (pte->pte &~ _PAGE_RW) 
		pte->pte |= _PAGE_RW ;	

	sctable[__NR_open] = mousehole_sys_open ;
	sctable[__NR_kill] = mousehole_sys_kill ;
	
	return 0;
}

static 
void __exit mousehole_exit(void) {
	unsigned int level ;
	pte_t * pte ;
	printk(KERN_INFO "Bye mousehole!!");
	remove_proc_entry("mousehole", NULL) ;

	sctable[__NR_open] = orig_sys_open ;
	sctable[__NR_kill] = orig_sys_kill ;
	pte = lookup_address((unsigned long) sctable, &level) ;
	pte->pte = pte->pte &~ _PAGE_RW ;
}

module_init(mousehole_init);
module_exit(mousehole_exit);
