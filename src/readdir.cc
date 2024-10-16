#include "tmfs.hh"

int tmfs_readdir(const char * path, void * buf, fuse_fill_dir_t filler_cb, off_t offset,
                 struct fuse_file_info * fi FUSE3_ONLY(, enum fuse_readdir_flags))
{
  // get the real path
  std::string real_path = get_real_path(path);

  // checks if it's really a directory
  if (!fs::is_directory(real_path))
    return -ENOTDIR;

  struct stat stbuf;

  // report ./ and ../
  stbuf.st_mode = S_IFDIR | 0755;
  stbuf.st_nlink = 2;
  filler_cb(buf, ".", &stbuf, 0 FUSE3_ONLY(, FUSE_FILL_DIR_PLUS));
  filler_cb(buf, "..", &stbuf, 0 FUSE3_ONLY(, FUSE_FILL_DIR_PLUS));

  // now iterate over the real directory
  DIR * dir = opendir(real_path.c_str());
  if (!dir)
    return 0;

  struct dirent * entry;
  while ((entry = readdir(dir)))
  {
    // stat the file pointed by entry
    auto file_path = fs::path(path) / entry->d_name;
    if (tmfs_getattr(file_path.string().c_str(), &stbuf FUSE3_ONLY(, nullptr)))
      continue;
    stbuf.st_mode |= 0755;
    // report the entry
    filler_cb(buf, entry->d_name, &stbuf, 0 FUSE3_ONLY(, FUSE_FILL_DIR_PLUS));
  }
  closedir(dir);

  return 0;
}
