SYSCALL_DEFINE2(mycopy,const char*,source_file,const char*, outputFile)
{
	struct kstat k_buf;
	char buf[1024];
	char sfile[256],ofile[256];
	int read_fd, write_fd;
	long filesize;
	
	mm_segment_t fs = get_fs();//获得当前的线程
	set_fs(get_ds());//将能访问的空间扩大 KERNEL_DS
	//系统调用的时候并不会直接操作用户输入的字符串，而是先拷贝到内核的缓存区中，调用_do_strncoy_from_users
	filesize = strncpy_from_users(sfile,source_file,sizeof(sfile));
	if(filesize < 0 || filesize == sizeof(sfile))
	{
		set_fs(fs);
		return -EFAULT;
	}
	
	filesize = strncpy_from_users(ofile,outputFile,sizeof(ofile));
	if(filesize < 0 || filesize == sizeof(ofile))
	{
		set_fs(fs);
		return -EFAULT;
	}
	//vfs_stat 查找sfile的文件属性，可以读文件大小
	if(vfs_stat(sfile, &k_buf) != 0)
	{
		set_fs(fs);
		return -EFAULT;
	}
	
	if((read_fd = ksys_open(sfile,O_RDONLY,S_IRUSR)) == -1)
	{
		set_fs(fs);
		return -EFAULT;
	}
	
	if((write_fd = ksys_open(ofile,O_WRONLY | O_CREAT | O_TRUNC,k_buf.mode))==-1)
	{
		set_fs(fs);
		return -EAFAULT;
	}
	
	for(;;)
	{
		filesize = ksys_read(read_fd, buf, sizeof(buf));
		if(filesize < 0)
		{
			set_fs(fs);
			return -EFAULT;
		}
		else if(filesize == 0)
			break;
		ksys_write(write_fd, buf, filesize);
	}
	
	ksys_close(read_fd);
	ksys_close(write_fd);
	
	set_fs(fs);
	
	return 0;
}