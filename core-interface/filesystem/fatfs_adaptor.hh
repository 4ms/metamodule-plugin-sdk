#pragma once
#include "ff_host.hh"
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

namespace MetaModule
{

// forward declare (this is defined by host: cortex-a7, vcv, or simulator)
struct FsProxy;

struct FatFS {

	FatFS(std::string_view root = "");
	~FatFS();

	// Read-only:
	FRESULT f_open(File *fp, const char *path, uint8_t mode);
	FRESULT f_close(File *fp);
	FRESULT f_read(File *fp, void *buff, unsigned btr, unsigned *br);
	FRESULT f_lseek(File *fp, uint64_t ofs);
	FRESULT f_stat(const char *path, Fileinfo *fno);
	char *f_gets(char *buff, int len, File *fp);

	// Dirs
	FRESULT f_opendir(Dir *dp, const char *path);
	FRESULT f_closedir(Dir *dp);
	FRESULT f_readdir(Dir *dp, Fileinfo *fno);
	FRESULT f_findfirst(Dir *dp, Fileinfo *fno, const char *path, const char *pattern);
	FRESULT f_findnext(Dir *dp, Fileinfo *fno);

	// Create dir
	FRESULT f_mkdir(const char *path);

	// Working Dir
	FRESULT f_getcwd(char *buff, unsigned len);
	FRESULT f_chdir(const char *path);

	// Writing:
	FRESULT f_write(File *fp, const void *buff, unsigned btw, unsigned *bw);
	FRESULT f_sync(File *fp);
	FRESULT f_truncate(File *fp);
	int f_putc(char c, File *fp);
	int f_puts(const char *str, File *cp);
	int f_printf(File *fp, const char *str, ...);

	FRESULT f_unlink(const char *path);
	FRESULT f_rename(const char *path_old, const char *path_new);
	FRESULT f_utime(const char *path, const Fileinfo *fno);
	FRESULT f_expand(File *fp, uint64_t fsz, uint8_t opt);

	void reset_file(File *fp);
	void reset_dir(Dir *dp);
	bool is_file_reset(File *fp);
	bool is_dir_reset(Dir *fp);

	bool find_valid_root(std::string_view path);

#undef f_eof
	static bool f_eof(File *fp);

#undef f_error
	static uint8_t f_error(File *fp);

#undef f_tell
	static uint64_t f_tell(File *fp);

#undef f_size
	static uint64_t f_size(File *fp);

#undef f_rewind
	FRESULT f_rewind(File *fp);

#undef f_rewinddir
	FRESULT f_rewinddir(Dir *dp);

#undef f_rmdir
	FRESULT f_rmdir(const char *path);

private:
	std::unique_ptr<FsProxy> impl;
	std::string root;
	std::string cwd;

	std::string full_path(const char *path);
};
} // namespace MetaModule
