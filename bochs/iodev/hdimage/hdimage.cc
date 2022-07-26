/////////////////////////////////////////////////////////////////////////
// $Id: hdimage.cc 14229 2021-04-18 17:20:41Z vruppert $
/////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2002-2021  The Bochs Project
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
/////////////////////////////////////////////////////////////////////////

#ifdef BXIMAGE
#include "config.h"
#include "misc/bxcompat.h"
#include "osdep.h"
#include "misc/bswap.h"
#else
#include "bochs.h"
#include "gui/siminterface.h"
#include "param_names.h"
#include "plugin.h"
#include "cdrom.h"
#include "cdrom_amigaos.h"
#include "cdrom_misc.h"
#include "cdrom_osx.h"
#include "cdrom_win32.h"
#endif
#include "hdimage.h"

#if BX_HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif
#ifdef linux
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#endif

#ifndef O_ACCMODE
#define O_ACCMODE (O_WRONLY | O_RDWR)
#endif

#define LOG_THIS bx_hdimage_ctl.

#ifndef BXIMAGE

bx_hdimage_ctl_c bx_hdimage_ctl;

const Bit8u n_hdimage_builtin_modes = 7;

const char *builtin_mode_names[n_hdimage_builtin_modes] = {
  "flat",
  "concat",
  "sparse",
  "dll",
  "growing",
  "undoable",
  "volatile"
};

const char **hdimage_mode_names;

bx_hdimage_ctl_c::bx_hdimage_ctl_c()
{
  put("hdimage", "IMG");
}

void bx_hdimage_ctl_c::init(void)
{
  Bit8u count = n_hdimage_builtin_modes;

  count += PLUG_get_plugins_count(PLUGTYPE_IMG);
  hdimage_mode_names = (const char**) malloc((count + 1) * sizeof(char*));
  for (Bit8u i = 0; i < n_hdimage_builtin_modes; i++) {
    hdimage_mode_names[i] = builtin_mode_names[i];
  }
  Bit8u n = 0;
  for (Bit8u i = n_hdimage_builtin_modes; i < count; i++) {
    hdimage_mode_names[i] = PLUG_get_plugin_name(PLUGTYPE_IMG, n);
    n++;
  }
  hdimage_mode_names[count] = NULL;
}

const char **bx_hdimage_ctl_c::get_mode_names(void)
{
  return hdimage_mode_names;
}

int bx_hdimage_ctl_c::get_mode_id(const char *mode)
{
  int i = 0;

  while (hdimage_mode_names[i] != NULL) {
    if (!strcmp(mode, hdimage_mode_names[i])) return i;
    i++;
  }
  return -1;
}

void bx_hdimage_ctl_c::list_modules(void)
{
  char list[60];
  Bit8u i = 0;
  size_t len = 0, len1;

  BX_INFO(("Disk image modules"));
  list[0] = 0;
  while (hdimage_mode_names[i] != NULL) {
    len1 = strlen(hdimage_mode_names[i]);
    if ((len + len1 + 1) > 58) {
      BX_INFO((" %s", list));
      list[0] = 0;
      len = 0;
    }
    strcat(list, " ");
    strcat(list, hdimage_mode_names[i]);
    len = strlen(list);
    i++;
  }
  if (len > 0) {
    BX_INFO((" %s", list));
  }
}

void bx_hdimage_ctl_c::exit(void)
{
  free(hdimage_mode_names);
  hdimage_locator_c::cleanup();
}

device_image_t* bx_hdimage_ctl_c::init_image(const char *image_mode, Bit64u disk_size, const char *journal)
{
  device_image_t *hdimage = NULL;

  // instantiate the right class
  if (!strcmp(image_mode, "flat")) {
    hdimage = new flat_image_t();
  } else if (!strcmp(image_mode, "concat")) {
    hdimage = new concat_image_t();
#ifdef WIN32
  } else if (!strcmp(image_mode, "dll")) {
    hdimage = new dll_image_t();
#endif //DLL_HD_SUPPORT
  } else if (!strcmp(image_mode, "sparse")) {
    hdimage = new sparse_image_t();
  } else if (!strcmp(image_mode, "undoable")) {
    hdimage = new undoable_image_t(journal);
  } else if (!strcmp(image_mode, "growing")) {
    hdimage = new growing_image_t();
  } else if (!strcmp(image_mode, "volatile")) {
    hdimage = new volatile_image_t(journal);
  } else {
    if (!hdimage_locator_c::module_present(image_mode)) {
#if BX_PLUGINS
      PLUG_load_plugin_var(image_mode, PLUGTYPE_IMG);
#else
      BX_PANIC(("Disk image mode '%s' not available", image_mode));
#endif
    }
    hdimage = hdimage_locator_c::create(image_mode, disk_size, journal);
  }
  return hdimage;
}

cdrom_base_c* bx_hdimage_ctl_c::init_cdrom(const char *dev)
{
#if BX_SUPPORT_CDROM
  return new LOWLEVEL_CDROM(dev);
#else
  return new cdrom_base_c(dev);
#endif
}

#endif // ifndef BXIMAGE

hdimage_locator_c *hdimage_locator_c::all = NULL;

//
// Each disk image module has a static locator class that registers
// here
//
hdimage_locator_c::hdimage_locator_c(const char *mode)
{
  hdimage_locator_c *ptr;

  this->mode = mode;
  this->next = NULL;
  if (all == NULL) {
    all = this;
  } else {
    ptr = all;
    while (ptr->next) ptr = ptr->next;
    ptr->next = this;
  }
}

hdimage_locator_c::~hdimage_locator_c()
{
  hdimage_locator_c *ptr = 0;

  if (this == all) {
    all = all->next;
  } else {
    ptr = all;
    while (ptr != NULL) {
      if (ptr->next != this) {
        ptr = ptr->next;
      } else {
        break;
      }
    }
  }
  if (ptr) {
    ptr->next = this->next;
  }
}

bool hdimage_locator_c::module_present(const char *mode)
{
  hdimage_locator_c *ptr;

  for (ptr = all; ptr != NULL; ptr = ptr->next) {
    if (strcmp(mode, ptr->mode) == 0)
      return 1;
  }
  return 0;
}

void hdimage_locator_c::cleanup()
{
#if BX_PLUGINS && !defined(BXIMAGE)
  while (all != NULL) {
    PLUG_unload_plugin_type(all->mode, PLUGTYPE_IMG);
  }
#endif
}

//
// Called by common hdimage code to locate and create a device_image_t
// object
//
device_image_t*
hdimage_locator_c::create(const char *mode, Bit64u disk_size, const char *journal)
{
  hdimage_locator_c *ptr = 0;

  for (ptr = all; ptr != NULL; ptr = ptr->next) {
    if (strcmp(mode, ptr->mode) == 0)
      return (ptr->allocate(disk_size, journal));
  }
  return NULL;
}

bool hdimage_locator_c::detect_image_mode(int fd, Bit64u disk_size,
                                          const char **image_mode)
{
  hdimage_locator_c *ptr = 0;

  for (ptr = all; ptr != NULL; ptr = ptr->next) {
    if (ptr->check_format(fd, disk_size) == HDIMAGE_FORMAT_OK) {
      *image_mode = ptr->mode;
      return 1;
    }
  }
  return 0;
}

// helper functions

#ifdef BXIMAGE
int bx_create_image_file(const char *filename)
{
  int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC
#ifdef O_BINARY
                | O_BINARY
#endif
                , S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP
                );
  return fd;
}
#endif

int bx_read_image(int fd, Bit64s offset, void *buf, int count)
{
  if (lseek(fd, offset, SEEK_SET) == -1) {
    return -1;
  }
  return read(fd, buf, count);
}

int bx_write_image(int fd, Bit64s offset, void *buf, int count)
{
  if (lseek(fd, offset, SEEK_SET) == -1) {
    return -1;
  }
  return write(fd, buf, count);
}

int bx_close_image(int fd, const char *pathname)
{
#ifndef BXIMAGE
  char lockfn[BX_PATHNAME_LEN];

  sprintf(lockfn, "%s.lock", pathname);
  if (access(lockfn, F_OK) == 0) {
    unlink(lockfn);
  }
#endif
  return ::close(fd);
}

#ifndef WIN32
int hdimage_open_file(const char *pathname, int flags, Bit64u *fsize, time_t *mtime)
#else
int hdimage_open_file(const char *pathname, int flags, Bit64u *fsize, FILETIME *mtime)
#endif
{
#ifndef BXIMAGE
  char lockfn[BX_PATHNAME_LEN];
  int lockfd;
#endif

#ifdef WIN32
  if (fsize != NULL) {
    HANDLE hFile = CreateFile(pathname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
      ULARGE_INTEGER FileSize;
      FileSize.LowPart = GetFileSize(hFile, &FileSize.HighPart);
      if (mtime != NULL) {
        GetFileTime(hFile, NULL, NULL, mtime);
      }
      CloseHandle(hFile);
      if ((FileSize.LowPart != INVALID_FILE_SIZE) || (GetLastError() == NO_ERROR)) {
        *fsize = FileSize.QuadPart;
      } else {
        return -1;
      }
    } else {
      return -1;
    }
  }
#endif

#ifndef BXIMAGE
  sprintf(lockfn, "%s.lock", pathname);
  lockfd = ::open(lockfn, O_RDONLY);
  if (lockfd >= 0) {
    ::close(lockfd);
    if (SIM->get_param_bool(BXPN_UNLOCK_IMAGES)->get()) {
      // Remove lock file if requested
      if (access(lockfn, F_OK) == 0) {
        unlink(lockfn);
      }
    } else {
      // Opening image must fail if lock file exists.
      BX_ERROR(("image locked: '%s'", pathname));
      return -1;
    }
  }
#endif

  int fd = ::open(pathname, flags
#ifdef O_BINARY
              | O_BINARY
#endif
              );

  if (fd < 0) {
    return fd;
  }

#ifndef WIN32
  if (fsize != NULL) {
    struct stat stat_buf;
    if (fstat(fd, &stat_buf)) {
      BX_PANIC(("fstat() returns error!"));
      return -1;
    }
#ifdef linux
    if (S_ISBLK(stat_buf.st_mode)) { // Is this a special device file (e.g. /dev/sde) ?
      ioctl(fd, BLKGETSIZE64, fsize); // yes it's!
    }
    else
#endif
    {
      *fsize = (Bit64u)stat_buf.st_size; // standard unix procedure to get size of regular files
    }
    if (mtime != NULL) {
      *mtime = stat_buf.st_mtime;
    }
  }
#endif
#ifndef BXIMAGE
  if ((flags & O_ACCMODE) != O_RDONLY) {
    lockfd = ::open(lockfn, O_CREAT | O_RDWR
#ifdef O_BINARY
                | O_BINARY
#endif
                , S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP);
    if (lockfd >= 0) {
      // lock this image
      ::close(lockfd);
    }
  }
#endif
  return fd;
}

bool hdimage_detect_image_mode(const char *pathname, const char **image_mode)
{
  bool result = false;
  Bit64u image_size = 0;

#if BX_PLUGINS && !defined(BXIMAGE)
  PLUG_load_plugin_var("*", PLUGTYPE_IMG);
#endif
  int fd = hdimage_open_file(pathname, O_RDONLY, &image_size, NULL);
  if (fd < 0) {
    return result;
  }

  if (sparse_image_t::check_format(fd, image_size) == HDIMAGE_FORMAT_OK) {
    *image_mode = "sparse";
    result = true;
  } else if (growing_image_t::check_format(fd, image_size) == HDIMAGE_FORMAT_OK) {
    *image_mode = "growing";
    result = true;
  } else if (hdimage_locator_c::detect_image_mode(fd, image_size, image_mode)) {
    result = true;
  } else if (flat_image_t::check_format(fd, image_size) == HDIMAGE_FORMAT_OK) {
    *image_mode = "flat";
    result = true;
  }
  ::close(fd);

  return result;
}

// if return_time==0, this returns the fat_date, else the fat_time
#ifndef WIN32
Bit16u fat_datetime(time_t time, int return_time)
{
  struct tm* t;
  struct tm t1;

  t = &t1;
  localtime_r(&time, t);
  if (return_time)
    return htod16((t->tm_sec/2) | (t->tm_min<<5) | (t->tm_hour<<11));
  return htod16((t->tm_mday) | ((t->tm_mon+1)<<5) | ((t->tm_year-80)<<9));
}
#else
Bit16u fat_datetime(FILETIME time, int return_time)
{
  SYSTEMTIME gmtsystime, systime;
  TIME_ZONE_INFORMATION tzi;

  FileTimeToSystemTime(&time, &gmtsystime);
  GetTimeZoneInformation(&tzi);
  SystemTimeToTzSpecificLocalTime(&tzi, &gmtsystime, &systime);
  if (return_time)
    return htod16((systime.wSecond/2) | (systime.wMinute<<5) | (systime.wHour<<11));
  return htod16((systime.wDay) | (systime.wMonth<<5) | ((systime.wYear-1980)<<9));
}
#endif

#ifndef BXIMAGE
// generic save/restore functions
Bit64s hdimage_save_handler(void *class_ptr, bx_param_c *param)
{
  char imgname[BX_PATHNAME_LEN];
  char path[BX_PATHNAME_LEN+1];

  param->get_param_path(imgname, BX_PATHNAME_LEN);
  if (!strncmp(imgname, "bochs.", 6)) {
    strcpy(imgname, imgname+6);
  }
  if (SIM->get_param_string(BXPN_RESTORE_PATH)->isempty()) {
    return 0;
  }
  sprintf(path, "%s/%s", SIM->get_param_string(BXPN_RESTORE_PATH)->getptr(), imgname);
  return ((device_image_t*)class_ptr)->save_state(path);
}

void hdimage_restore_handler(void *class_ptr, bx_param_c *param, Bit64s value)
{
  char imgname[BX_PATHNAME_LEN];
  char path[BX_PATHNAME_LEN+1];

  if (value != 0) {
    param->get_param_path(imgname, BX_PATHNAME_LEN);
    if (!strncmp(imgname, "bochs.", 6)) {
      strcpy(imgname, imgname+6);
    }
    sprintf(path, "%s/%s", SIM->get_param_string(BXPN_RESTORE_PATH)->getptr(), imgname);
    ((device_image_t*)class_ptr)->restore_state(path);
  }
}

bool hdimage_backup_file(int fd, const char *backup_fname)
{
  char *buf;
  off_t offset;
  int nread, size;
  bool ret = 1;

  int backup_fd = ::open(backup_fname, O_RDWR | O_CREAT | O_TRUNC
#ifdef O_BINARY
    | O_BINARY
#endif
    , S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP);
  if (backup_fd >= 0) {
    offset = 0;
    size = 0x20000;
    buf = new char[size];
    if (buf == NULL) {
      ::close(backup_fd);
      return 0;
    }
    while ((nread = bx_read_image(fd, offset, buf, size)) > 0) {
      if (bx_write_image(backup_fd, offset, buf, nread) < 0) {
        ret = 0;
        break;
      }
      if (nread < size) {
        break;
      }
      offset += size;
    };
    if (nread < 0) {
      ret = 0;
    }
    delete [] buf;
    ::close(backup_fd);
    return ret;
  }
  return 0;
}
#endif

bool hdimage_copy_file(const char *src, const char *dst)
{
#ifdef WIN32
  return (bool)CopyFile(src, dst, FALSE);
#elif defined(linux)
  pid_t pid, ws;

  if ((src == NULL) || (dst == NULL)) {
    return 0;
  }

  if (!(pid = fork())) {
    execl("/bin/cp", "/bin/cp", src, dst, (char *)0);
    return 0;
  }
  wait(&ws);
  if (!WIFEXITED(ws)) {
    return -1;
  }
  return (WEXITSTATUS(ws) == 0);
#else
  int fd1, fd2;
  char *buf;
  off_t offset;
  int nread, size;
  bool ret = 1;

  fd1 = ::open(src, O_RDONLY
#ifdef O_BINARY
    | O_BINARY
#endif
    );
  if (fd1 < 0) return 0;
  fd2 = ::open(dst, O_RDWR | O_CREAT | O_TRUNC
#ifdef O_BINARY
    | O_BINARY
#endif
    , S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP);
  if (fd2 < 0) {
    ::close(fd1);
    return 0;
  }
  offset = 0;
  size = 0x20000;
  buf = new char[size];
  if (buf == NULL) {
    ::close(fd1);
    ::close(fd2);
    return 0;
  }
  while ((nread = bx_read_image(fd1, offset, buf, size)) > 0) {
    if (bx_write_image(fd2, offset, buf, nread) < 0) {
      ret = 0;
      break;
    }
    if (nread < size) {
      break;
    }
    offset += size;
  };
  if (nread < 0) {
    ret = 0;
  }
  delete [] buf;
  ::close(fd1);
  ::close(fd2);
  return ret;
#endif
}

/*** base class device_image_t ***/

device_image_t::device_image_t()
{
  cylinders = 0;
  hd_size = 0;
  sect_size = 512;
}

int device_image_t::open(const char* _pathname)
{
  return open(_pathname, O_RDWR);
}

Bit32u device_image_t::get_capabilities()
{
  return (cylinders == 0) ? HDIMAGE_AUTO_GEOMETRY : 0;
}

Bit32u device_image_t::get_timestamp()
{
  return (fat_datetime(mtime, 1) | (fat_datetime(mtime, 0) << 16));
}

#ifndef BXIMAGE
void device_image_t::register_state(bx_list_c *parent)
{
  bx_param_bool_c *image = new bx_param_bool_c(parent, "image", NULL, NULL, 0);
  image->set_sr_handlers(this, hdimage_save_handler, hdimage_restore_handler);
}
#endif

/*** flat_image_t function definitions ***/

int flat_image_t::open(const char* _pathname, int flags)
{
  pathname = _pathname;
  if ((fd = hdimage_open_file(pathname, flags, &hd_size, &mtime)) < 0) {
    return -1;
  }
  BX_INFO(("hd_size: " FMT_LL "u", hd_size));
  if (hd_size <= 0) BX_PANIC(("size of disk image not detected / invalid"));
  if ((hd_size % sect_size) != 0) {
    BX_PANIC(("size of disk image must be multiple of %d bytes", sect_size));
  }
  return fd;
}

void flat_image_t::close()
{
  if (fd > -1) {
    bx_close_image(fd, pathname);
  }
}

Bit64s flat_image_t::lseek(Bit64s offset, int whence)
{
  return (Bit64s)::lseek(fd, (off_t)offset, whence);
}

ssize_t flat_image_t::read(void* buf, size_t count)
{
  return ::read(fd, (char*) buf, count);
}

ssize_t flat_image_t::write(const void* buf, size_t count)
{
  return ::write(fd, (char*) buf, count);
}

int flat_image_t::check_format(int fd, Bit64u imgsize)
{
  char buffer[512];

  if ((imgsize <= 0) || ((imgsize % 512) != 0)) {
    return HDIMAGE_SIZE_ERROR;
  } else if (bx_read_image(fd, 0, buffer, 512) < 0) {
    return HDIMAGE_READ_ERROR;
  } else {
    return HDIMAGE_FORMAT_OK;
  }
}

#ifndef BXIMAGE
bool flat_image_t::save_state(const char *backup_fname)
{
  return hdimage_backup_file(fd, backup_fname);
}

void flat_image_t::restore_state(const char *backup_fname)
{
  close();
  if (!hdimage_copy_file(backup_fname, pathname)) {
    BX_PANIC(("Failed to restore image '%s'", pathname));
    return;
  }
  if (device_image_t::open(pathname) < 0) {
    BX_PANIC(("Failed to open restored image '%s'", pathname));
  }
}
#endif

// helper function for concat and sparse mode images

char increment_string(char *str, int diff)
{
  // find the last character of the string, and increment it.
  char *p = str;
  while (*p != 0) p++;
  BX_ASSERT(p>str);  // choke on zero length strings
  p--;  // point to last character of the string
  (*p) += diff;  // increment to next/previous ascii code.
  BX_DEBUG(("increment string returning '%s'", str));
  return (*p);
}

/*** concat_image_t function definitions ***/

concat_image_t::concat_image_t()
{
  curr_fd = -1;
}

void concat_image_t::increment_string(char *str)
{
  ::increment_string(str, +1);
}

int concat_image_t::open(const char* _pathname0, int flags)
{
  UNUSED(flags);
  pathname0 = _pathname0;
  char *pathname1 = new char[strlen(pathname0) + 1];
  strcpy(pathname1, pathname0);
  BX_DEBUG(("concat_image_t::open"));
  Bit64s start_offset = 0;
  for (int i=0; i<BX_CONCAT_MAX_IMAGES; i++) {
    fd_table[i] = hdimage_open_file(pathname1, flags, &length_table[i], NULL);
    if (fd_table[i] < 0) {
      // open failed.
      // if no FD was opened successfully, return -1 (fail).
      if (i==0) return -1;
      // otherwise, it only means that all images in the series have
      // been opened.  Record the number of fds opened successfully.
      maxfd = i;
      break;
    }
    BX_INFO(("concat_image: open image #%d: '%s', (" FMT_LL "u bytes)", i, pathname1, length_table[i]));
    struct stat stat_buf;
    int ret = fstat(fd_table[i], &stat_buf);
    if (ret) {
      BX_PANIC(("fstat() returns error!"));
    }
#ifdef S_ISBLK
    if (S_ISBLK(stat_buf.st_mode)) {
      BX_PANIC(("block devices should REALLY NOT be used as concat images"));
    }
#endif
    if ((stat_buf.st_size % sect_size) != 0) {
      BX_PANIC(("size of disk image must be multiple of %d bytes", sect_size));
    }
    start_offset_table[i] = start_offset;
    start_offset += length_table[i];
    increment_string(pathname1);
  }
  delete [] pathname1;
  // start up with first image selected
  total_offset = 0;
  index = 0;
  curr_fd = fd_table[0];
  curr_min = 0;
  curr_max = length_table[0]-1;
  hd_size = start_offset;
  BX_INFO(("hd_size: " FMT_LL "u", hd_size));
  return 0; // success.
}

void concat_image_t::close()
{
  BX_DEBUG(("concat_image_t.close"));
  char *pathname1 = new char[strlen(pathname0) + 1];
  strcpy(pathname1, pathname0);
  for (int index = 0; index < maxfd; index++) {
    if (fd_table[index] > -1) {
      bx_close_image(fd_table[index], pathname1);
    }
    increment_string(pathname1);
  }
  delete [] pathname1;
}

Bit64s concat_image_t::lseek(Bit64s offset, int whence)
{
  if ((offset % sect_size) != 0)
    BX_PANIC(("lseek HD with offset not multiple of %d", sect_size));
  BX_DEBUG(("concat_image_t.lseek(%d)", whence));
  switch (whence) {
    case SEEK_SET:
      total_offset = offset;
      break;
    case SEEK_CUR:
      total_offset += offset;
      break;
    case SEEK_END:
      total_offset = hd_size - offset;
      break;
    default:
      return -1;
  }
  // is this offset in this disk image?
  if (total_offset < curr_min) {
    // no, look at previous images
    for (int i=index-1; i>=0; i--) {
      if (total_offset >= start_offset_table[i]) {
        index = i;
        curr_fd = fd_table[i];
        curr_min = start_offset_table[i];
        curr_max = curr_min + length_table[i] - 1;
        BX_DEBUG(("concat_image_t.lseek to earlier image, index=%d", index));
        break;
      }
    }
  } else if (total_offset > curr_max) {
    // no, look at later images
    for (int i=index+1; i<maxfd; i++) {
      if (total_offset < (start_offset_table[i] + length_table[i])) {
        index = i;
        curr_fd = fd_table[i];
        curr_min = start_offset_table[i];
        curr_max = curr_min + length_table[i] - 1;
        BX_DEBUG(("concat_image_t.lseek to earlier image, index=%d", index));
        break;
      }
    }
  }
  // now offset should be within the current image.
  offset = total_offset - start_offset_table[index];
  if ((offset < 0) || (offset >= (Bit64s)length_table[index])) {
    BX_PANIC(("concat_image_t.lseek to byte %ld failed", (long)offset));
    return -1;
  }
  return (Bit64s)::lseek(curr_fd, (off_t)offset, SEEK_SET);
}

ssize_t concat_image_t::read(void* buf, size_t count)
{
  size_t readmax, count1 = count;
  ssize_t ret = -1;
  char *buf1 = (char*)buf;

  BX_DEBUG(("concat_image_t.read %ld bytes", (long)count));
  do {
    readmax = (size_t)(curr_max - total_offset + 1);
    if (count1 > readmax) {
      ret = ::read(curr_fd, buf1, readmax);
      if (ret >= 0) {
        buf1 += readmax;
        count1 -= readmax;
        ret = lseek(curr_max + 1, SEEK_SET);
      }
    } else {
      ret = ::read(curr_fd, buf1, count1);
      if (ret >= 0) {
        ret = lseek(count1, SEEK_CUR);
      }
      break;
    }
  } while (ret > 0);
  return (ret < 0) ? ret : count;
}

ssize_t concat_image_t::write(const void* buf, size_t count)
{
  size_t writemax, count1 = count;
  ssize_t ret = -1;
  char *buf1 = (char*)buf;

  BX_DEBUG(("concat_image_t.write %ld bytes", (long)count));
  do {
    writemax = (size_t)(curr_max - total_offset + 1);
    if (count1 > writemax) {
      ret = ::write(curr_fd, buf1, writemax);
      if (ret >= 0) {
        buf1 += writemax;
        count1 -= writemax;
        ret = lseek(curr_max + 1, SEEK_SET);
      }
    } else {
      ret = ::write(curr_fd, buf1, count1);
      if (ret >= 0) {
        ret = lseek(count1, SEEK_CUR);
      }
      break;
    }
  } while (ret > 0);
  return (ret < 0) ? ret : count;
}

#ifndef BXIMAGE
bool concat_image_t::save_state(const char *backup_fname)
{
  bool ret = 1;
  char tempfn[BX_PATHNAME_LEN];

  for (int index = 0; index < maxfd; index++) {
    sprintf(tempfn, "%s%d", backup_fname, index);
    ret &= hdimage_backup_file(fd_table[index], tempfn);
    if (ret == 0) break;
  }
  return ret;
}

void concat_image_t::restore_state(const char *backup_fname)
{
  char tempfn[BX_PATHNAME_LEN];

  close();
  char *image_name = new char[strlen(pathname0) + 1];
  strcpy(image_name, pathname0);
  for (int index = 0; index < maxfd; index++) {
    sprintf(tempfn, "%s%d", backup_fname, index);
    if (!hdimage_copy_file(tempfn, image_name)) {
      BX_PANIC(("Failed to restore concat image '%s'", image_name));
      delete [] image_name;
      return;
    }
    increment_string(image_name);
  }
  delete [] image_name;
  device_image_t::open(pathname0);
}
#endif

/*** sparse_image_t function definitions ***/

sparse_image_t::sparse_image_t()
{
  fd = -1;
  pathname = NULL;
#ifdef _POSIX_MAPPED_FILES
  mmap_header = NULL;
#endif
  pagetable = NULL;
  parent_image = NULL;
}

int sparse_image_t::read_header()
{
  BX_ASSERT(sizeof(header) == SPARSE_HEADER_SIZE);

  int ret = check_format(fd, underlying_filesize);
  if (ret != HDIMAGE_FORMAT_OK) {
    switch (ret) {
      case HDIMAGE_READ_ERROR:
        BX_PANIC(("sparse: could not read entire header"));
        break;
      case HDIMAGE_NO_SIGNATURE:
        BX_PANIC(("sparse: failed header magic check"));
        break;
      case HDIMAGE_VERSION_ERROR:
        BX_PANIC(("sparse: unknown version in header"));
        break;
    }
    return -1;
  }

  ret = bx_read_image(fd, 0, &header, sizeof(header));
  if (ret < 0) {
    return -1;
  }

  pagesize = dtoh32(header.pagesize);
  Bit32u numpages = dtoh32(header.numpages);

  total_size = pagesize;
  total_size *= numpages;

  pagesize_shift = 0;
  while ((pagesize >> pagesize_shift) > 1) pagesize_shift++;

  if ((Bit32u)(1 << pagesize_shift) != pagesize) {
    panic("failed block size header check");
  }

  pagesize_mask = pagesize - 1;

  size_t  preamble_size = (sizeof(Bit32u) * numpages) + sizeof(header);
  data_start = 0;
  while ((size_t)data_start < preamble_size) data_start += pagesize;

  bool did_mmap = 0;

#ifdef _POSIX_MAPPED_FILES
  // Try to memory map from the beginning of the file (0 is trivially a page multiple)
  void *mmap_header = mmap(NULL, preamble_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (mmap_header == MAP_FAILED) {
    BX_INFO(("failed to mmap sparse disk file - using conventional file access"));
    mmap_header = NULL;
  }
  else
  {
    mmap_length = preamble_size;
    did_mmap = 1;
    pagetable = ((Bit32u *) (((Bit8u *) mmap_header) + sizeof(header)));
    system_pagesize_mask = getpagesize() - 1;
  }
#endif

  if (!did_mmap) {
    pagetable = new Bit32u[numpages];

    if (pagetable == NULL) {
      panic("could not allocate memory for sparse disk block table");
    }

    ret = ::read(fd, pagetable, sizeof(Bit32u) * numpages);

    if (ret < 0) {
      panic(strerror(errno));
    }

    if ((int)(sizeof(Bit32u) * numpages) != ret) {
      panic("could not read entire block table");
    }
  }
  return 0;
}

int sparse_image_t::open(const char* pathname0, int flags)
{
  pathname = strdup(pathname0);
  BX_DEBUG(("sparse_image_t::open"));

  if ((fd = hdimage_open_file(pathname, flags, &underlying_filesize, &mtime)) < 0) {
    return -1;
  }
  BX_DEBUG(("sparse_image: open image %s", pathname));

  if (read_header() < 0) {
    return -1;
  }

  if ((underlying_filesize % pagesize) != 0)
    panic("size of sparse disk image is not multiple of page size");

  if ((pagesize % sect_size) != 0)
    panic("page size of sparse disk image is not multiple of sector size");

  underlying_current_filepos = 0;
  if (-1 == ::lseek(fd, 0, SEEK_SET))
    panic("error while seeking to start of file");

  lseek(0, SEEK_SET);

  char * parentpathname = strdup(pathname);
  char lastchar = ::increment_string(parentpathname, -1);

  if ((lastchar >= '0') && (lastchar <= '9'))
  {
    struct stat stat_buf;
    if (0 == stat(parentpathname, &stat_buf))
    {
      parent_image = new sparse_image_t();
      int ret = parent_image->open(parentpathname, flags);
      if (ret != 0) return ret;
      if (    (parent_image->pagesize != pagesize)
          ||  (parent_image->total_size != total_size))
      {
        panic("child drive image does not have same page count/page size configuration");
      }
    }
  }

  if (parentpathname != NULL) free(parentpathname);

  if (dtoh32(header.version) == SPARSE_HEADER_VERSION) {
    hd_size = dtoh64(header.disk);
    BX_INFO(("sparse: pagesize = 0x%x, data_start = 0x" FMT_LL "x", pagesize, data_start));
  }

  return 0; // success
}

void sparse_image_t::close()
{
  BX_DEBUG(("concat_image_t.close"));
#ifdef _POSIX_MAPPED_FILES
  if (mmap_header != NULL) {
    int ret = munmap(mmap_header, mmap_length);
    if (ret != 0)
      BX_INFO(("failed to un-memory map sparse disk file"));
  }
  pagetable = NULL; // We didn't malloc it
#endif
  if (fd > -1) {
    bx_close_image(fd, pathname);
  }
  if (pathname != NULL) {
    free(pathname);
  }
  if (pagetable != NULL) {
    delete [] pagetable;
  }
  if (parent_image != NULL) {
    delete parent_image;
  }
}

Bit64s sparse_image_t::lseek(Bit64s offset, int whence)
{
  if ((offset % sect_size) != 0)
    BX_PANIC(("lseek HD with offset not multiple of %d", sect_size));
  if (whence != SEEK_SET)
    BX_PANIC(("lseek HD with whence not SEEK_SET"));

  BX_DEBUG(("sparse_image_t::lseek(%d)", whence));

  if (offset > total_size)
  {
    BX_PANIC(("sparse_image_t.lseek to byte %ld failed", (long)offset));
    return -1;
  }

  set_virtual_page((Bit32u)(offset >> pagesize_shift));
  position_page_offset = (Bit32u)(offset & pagesize_mask);

  return 0;
}

inline Bit64s sparse_image_t::get_physical_offset()
{
  Bit64s physical_offset = data_start;
  physical_offset += ((Bit64s)position_physical_page << pagesize_shift);
  physical_offset += position_page_offset;
  return physical_offset;
}

void sparse_image_t::set_virtual_page(Bit32u new_virtual_page)
{
  position_virtual_page = new_virtual_page;
  position_physical_page = dtoh32(pagetable[position_virtual_page]);
}

ssize_t sparse_image_t::read_page_fragment(Bit32u read_virtual_page, Bit32u read_page_offset, size_t read_size, void * buf)
{
  if (read_virtual_page != position_virtual_page)
  {
    set_virtual_page(read_virtual_page);
  }

  position_page_offset = read_page_offset;

  if (position_physical_page == SPARSE_PAGE_NOT_ALLOCATED)
  {
    if (parent_image != NULL)
    {
      return parent_image->read_page_fragment(read_virtual_page, read_page_offset, read_size, buf);
    }
    else
    {
      memset(buf, 0, read_size);
    }
  }
  else
  {
    Bit64s physical_offset = get_physical_offset();

    if (physical_offset != underlying_current_filepos)
    {
      off_t ret = ::lseek(fd, (off_t)physical_offset, SEEK_SET);
      // underlying_current_filepos update deferred
      if (ret == -1)
        panic(strerror(errno));
    }

    ssize_t readret = ::read(fd, buf, read_size);

    if (readret == -1)
    {
      panic(strerror(errno));
    }

    if ((size_t)readret != read_size)
    {
      panic("could not read block contents from file");
    }

    underlying_current_filepos = physical_offset + read_size;
  }

  return read_size;
}

ssize_t sparse_image_t::read(void* buf, size_t count)
{
  ssize_t total_read = 0;

  BX_DEBUG(("sparse_image_t.read %ld bytes", (long)count));

  while (count != 0)
  {
    size_t can_read = pagesize - position_page_offset;
    if (count < can_read) can_read = count;

    BX_ASSERT (can_read != 0);

    size_t was_read = (size_t)read_page_fragment(position_virtual_page, position_page_offset, can_read, buf);

    if (was_read != can_read) {
      BX_PANIC(("could not read from sparse disk"));
    }

    total_read += can_read;

    position_page_offset += can_read;
    if (position_page_offset == pagesize)
    {
      position_page_offset = 0;
      set_virtual_page(position_virtual_page + 1);
    }

    BX_ASSERT(position_page_offset < pagesize);

    buf = (((Bit8u *) buf) + can_read);
    count -= can_read;
  }

  return total_read;
}

void sparse_image_t::panic(const char * message)
{
  char buffer[1024];
  if (message == NULL)
  {
    snprintf(buffer, sizeof(buffer), "error with sparse disk image %s", pathname);
  }
  else
  {
    snprintf(buffer, sizeof(buffer), "error with sparse disk image %s - %s", pathname, message);
  }
  BX_PANIC(("%s", buffer));
}

ssize_t sparse_image_t::write(const void* buf, size_t count)
{
  ssize_t total_written = 0;

  Bit32u update_pagetable_start = position_virtual_page;
  Bit32u update_pagetable_count = 0;

  BX_DEBUG(("sparse_image_t.write %ld bytes", (long)count));

  while (count != 0)
  {
    size_t can_write = pagesize - position_page_offset;
    if (count < can_write) can_write = count;

    BX_ASSERT (can_write != 0);

    if (position_physical_page == SPARSE_PAGE_NOT_ALLOCATED)
    {
      // We just add on another page at the end of the file
      // Reclamation, compaction etc should currently be done off-line

      Bit64s data_size = underlying_filesize - data_start;
      BX_ASSERT((data_size % pagesize) == 0);

      Bit32u data_size_pages = (Bit32u)(data_size / pagesize);
      Bit32u next_data_page = data_size_pages;

      pagetable[position_virtual_page] = htod32(next_data_page);
      position_physical_page = next_data_page;

      Bit64s page_file_start = data_start + ((Bit64s)position_physical_page << pagesize_shift);

      if (parent_image != NULL)
      {
        // If we have a parent, we must merge our portion with the parent
        void *writebuffer = NULL;

        if (can_write == pagesize)
        {
          writebuffer = (void *) buf;
        }
        else
        {
          writebuffer = malloc(pagesize);
          if (writebuffer == NULL)
            panic("Cannot allocate sufficient memory for page-merge in write");

          // Read entire page - could optimize, but simple for now
          parent_image->read_page_fragment(position_virtual_page, 0, pagesize, writebuffer);

          void *dest_start = ((Bit8u *) writebuffer) + position_page_offset;
          memcpy(dest_start, buf, can_write);
        }

        int ret = (int)::lseek(fd, page_file_start, SEEK_SET);
        // underlying_current_filepos update deferred
        if (ret == -1) panic(strerror(errno));

        ret = ::write(fd, writebuffer, pagesize);
        if (ret == -1) panic(strerror(errno));

        if (pagesize != (Bit32u)ret) panic("failed to write entire merged page to disk");

        if (can_write != pagesize)
        {
          free(writebuffer);
        }
      }
      else
      {
        // We need to write a zero page because read has been returning zeroes
        // We seek as close to the page end as possible, and then write a little
        // This produces a sparse file which has blanks
        // Also very quick, even when pagesize is massive
        int ret = (int)::lseek(fd, page_file_start + pagesize - 4, SEEK_SET);
        // underlying_current_filepos update deferred
        if (ret == -1) panic(strerror(errno));

        Bit32u zero = 0;
        ret = ::write(fd, &zero, 4);
        if (ret == -1) panic(strerror(errno));

        if (ret != 4) panic("failed to write entire blank page to disk");
      }

      update_pagetable_count = (position_virtual_page - update_pagetable_start) + 1;
      underlying_filesize = underlying_current_filepos = page_file_start + pagesize;
    }

    BX_ASSERT(position_physical_page != SPARSE_PAGE_NOT_ALLOCATED);

    Bit64s physical_offset = get_physical_offset();

    if (physical_offset != underlying_current_filepos)
    {
      off_t ret = ::lseek(fd, (off_t)physical_offset, SEEK_SET);
      // underlying_current_filepos update deferred
      if (ret == -1)
        panic(strerror(errno));
    }

    ssize_t writeret = ::write(fd, buf, can_write);

    if (writeret == -1)
    {
      panic(strerror(errno));
    }

    if ((size_t)writeret != can_write)
    {
      panic("could not write block contents to file");
    }

    underlying_current_filepos = physical_offset + can_write;

    total_written += can_write;

    position_page_offset += can_write;
    if (position_page_offset == pagesize)
    {
      position_page_offset = 0;
      set_virtual_page(position_virtual_page + 1);
    }

    BX_ASSERT(position_page_offset < pagesize);

    buf = (((Bit8u *) buf) + can_write);
    count -= can_write;
  }

  if (update_pagetable_count != 0)
  {
    bool done = 0;
    off_t pagetable_write_from = sizeof(header) + (sizeof(Bit32u) * update_pagetable_start);
    size_t write_bytecount = update_pagetable_count * sizeof(Bit32u);

#ifdef _POSIX_MAPPED_FILES
    if (mmap_header != NULL)
    {
      // Sync from the beginning of the page
      size_t system_page_offset = pagetable_write_from & system_pagesize_mask;
      void *start = ((Bit8u *) mmap_header + pagetable_write_from - system_page_offset);

      int ret = msync(start, system_page_offset + write_bytecount, MS_ASYNC);

      if (ret != 0)
        panic(strerror(errno));

      done = 1;
    }
#endif

    if (!done)
    {
      int ret = (int)::lseek(fd, pagetable_write_from, SEEK_SET);
      // underlying_current_filepos update deferred
      if (ret == -1) panic(strerror(errno));

      ret = ::write(fd, &pagetable[update_pagetable_start], write_bytecount);
      if (ret == -1) panic(strerror(errno));
      if ((size_t)ret != write_bytecount) panic("could not write entire updated block header");

      underlying_current_filepos = pagetable_write_from + write_bytecount;
    }
  }

  return total_written;
}

int sparse_image_t::check_format(int fd, Bit64u imgsize)
{
  sparse_header_t temp_header;

  int ret = ::read(fd, &temp_header, sizeof(temp_header));
  if (ret < 0) {
    return HDIMAGE_READ_ERROR;
  }
  if (ret != sizeof(temp_header)) {
    return HDIMAGE_READ_ERROR;
  }

  if (dtoh32(temp_header.magic) != SPARSE_HEADER_MAGIC) {
    return HDIMAGE_NO_SIGNATURE;
  }

  if ((dtoh32(temp_header.version) != SPARSE_HEADER_VERSION) &&
      (dtoh32(temp_header.version) != SPARSE_HEADER_V1)) {
    return HDIMAGE_VERSION_ERROR;
  }

  return HDIMAGE_FORMAT_OK;
}

#ifdef BXIMAGE
int sparse_image_t::create_image(const char *pathname, Bit64u size)
{
  Bit64u numpages;
  sparse_header_t header;
  size_t sizesofar;
  int padtopagesize;

  memset(&header, 0, sizeof(header));
  header.magic = htod32(SPARSE_HEADER_MAGIC);
  header.version = htod32(SPARSE_HEADER_VERSION);

  header.pagesize = htod32((1 << 10) * 32); // Use 32 KB Pages - could be configurable
  numpages = (size / dtoh32(header.pagesize)) + 1;

  header.numpages = htod32((Bit32u)numpages);
  header.disk = htod64(size);

  if (numpages != dtoh32(header.numpages)) {
    BX_FATAL(("ERROR: The disk image is too large for a sparse image!"));
    // Could increase page size here.
    // But note this only happens at 128 Terabytes!
  }

  int fd = bx_create_image_file(pathname);
  if (fd < 0)
    BX_FATAL(("ERROR: failed to create sparse image file"));
  if (bx_write_image(fd, 0, &header, sizeof(header)) != sizeof(header)) {
    ::close(fd);
    BX_FATAL(("ERROR: The disk image is not complete - could not write header!"));
  }

  Bit32u *pagetable = new Bit32u[dtoh32(header.numpages)];
  if (pagetable == NULL)
    BX_FATAL(("ERROR: The disk image is not complete - could not create pagetable!"));
  for (Bit32u i=0; i<dtoh32(header.numpages); i++)
    pagetable[i] = htod32(SPARSE_PAGE_NOT_ALLOCATED);

  if (bx_write_image(fd, sizeof(header), pagetable, 4 * dtoh32(header.numpages)) != (int)(4 * dtoh32(header.numpages))) {
    ::close(fd);
    BX_FATAL(("ERROR: The disk image is not complete - could not write pagetable!"));
  }
  delete [] pagetable;

  sizesofar = SPARSE_HEADER_SIZE + (4 * dtoh32(header.numpages));
  padtopagesize = dtoh32(header.pagesize) - (sizesofar & (dtoh32(header.pagesize) - 1));

  Bit8u *padding = new Bit8u[padtopagesize];
  memset(padding, 0, padtopagesize);

  if (bx_write_image(fd, sizesofar, padding, padtopagesize) != padtopagesize) {
    ::close(fd);
    BX_FATAL(("ERROR: The disk image is not complete - could not write padding!"));
  }
  delete [] padding;
  ::close(fd);
  return 0;
}
#else
bool sparse_image_t::save_state(const char *backup_fname)
{
  return hdimage_backup_file(fd, backup_fname);
}

void sparse_image_t::restore_state(const char *backup_fname)
{
  int backup_fd;
  Bit64u imgsize = 0;
  char *temp_pathname;

  if ((backup_fd = hdimage_open_file(backup_fname, O_RDONLY, &imgsize, NULL)) < 0) {
    BX_PANIC(("Could not open sparse image backup"));
    return;
  }
  if (check_format(backup_fd, imgsize) != HDIMAGE_FORMAT_OK) {
    ::close(backup_fd);
    BX_PANIC(("Could not detect sparse image header"));
    return;
  }
  ::close(backup_fd);
  temp_pathname = strdup(pathname);
  close();
  if (!hdimage_copy_file(backup_fname, temp_pathname)) {
    BX_PANIC(("Failed to restore sparse image '%s'", temp_pathname));
    free(temp_pathname);
    return;
  }
  if (device_image_t::open(temp_pathname) < 0) {
    BX_PANIC(("Failed to open restored image '%s'", temp_pathname));
  }
  free(temp_pathname);
}
#endif

#ifdef WIN32

/*** dll_image_t function definitions ***/

typedef int  (CDECL *vdisk_open_t)  (const char *path, int flags);
typedef BOOL (CDECL *vdisk_read_t)  (int vunit, LONGLONG blk, void *buf);
typedef BOOL (CDECL *vdisk_write_t) (int vunit, LONGLONG blk, const void *buf);
typedef void (CDECL *vdisk_close_t) (int vunit);
typedef LONGLONG (CDECL *vdisk_get_size_t) (int vunit);

HINSTANCE hlib_vdisk = NULL;
vdisk_open_t vdisk_open = NULL;
vdisk_read_t vdisk_read = NULL;
vdisk_write_t vdisk_write = NULL;
vdisk_close_t vdisk_close = NULL;
vdisk_get_size_t vdisk_get_size = NULL;

dll_image_t::dll_image_t()
{
  if (hlib_vdisk == NULL) {
    hlib_vdisk = LoadLibrary("vdisk.dll");
    if (hlib_vdisk != NULL) {
      vdisk_open =  (vdisk_open_t)        GetProcAddress(hlib_vdisk,"vdisk_open");
      vdisk_read =  (vdisk_read_t)        GetProcAddress(hlib_vdisk,"vdisk_read");
      vdisk_write = (vdisk_write_t)       GetProcAddress(hlib_vdisk,"vdisk_write");
      vdisk_close = (vdisk_close_t)       GetProcAddress(hlib_vdisk,"vdisk_close");
      vdisk_get_size = (vdisk_get_size_t) GetProcAddress(hlib_vdisk,"vdisk_get_size");
      if ((vdisk_open == NULL) || (vdisk_read == NULL) || (vdisk_write == NULL) ||
          (vdisk_close == NULL) || (vdisk_get_size == NULL)) {
        FreeLibrary(hlib_vdisk);
        hlib_vdisk = NULL;
      }
    }
  }
}

int dll_image_t::open(const char* pathname, int flags)
{
  if (hlib_vdisk != NULL) {
    vunit = vdisk_open(pathname, flags);
    if (vunit >= 0) {
      hd_size = (Bit64u)vdisk_get_size(vunit) << 9;
      sect_size = 512;
      vblk = 0;
    }
  } else {
    vunit = -2;
  }
  return vunit;
}

void dll_image_t::close()
{
  if ((vunit >= 0) && (hlib_vdisk != NULL)) {
    vdisk_close(vunit);
  }
}

Bit64s dll_image_t::lseek(Bit64s offset, int whence)
{
  if (whence == SEEK_SET) {
    vblk = offset >> 9;
  } else if (whence == SEEK_CUR) {
    vblk += offset >> 9;
  } else {
    BX_ERROR(("lseek: mode not supported yet"));
    return -1;
  }
  if (vblk >= (Bit64s)(hd_size >> 9))
    return -1;
  return 0;
}

ssize_t dll_image_t::read(void* buf, size_t count)
{
  if ((vunit >= 0) && (hlib_vdisk != NULL)) {
    if (vdisk_read(vunit, vblk, buf)) {
      vblk++;
      return count;
    }
  }
  return -1;
}

ssize_t dll_image_t::write(const void* buf, size_t count)
{
  if ((vunit >= 0) && (hlib_vdisk != 0)) {
    if (vdisk_write(vunit, vblk, buf)) {
      vblk++;
      return count;
    }
  }
  return -1;
}
#endif // DLL_HD_SUPPORT

// redolog implementation
redolog_t::redolog_t()
{
  fd = -1;
  pathname = NULL;
  catalog = NULL;
  bitmap = NULL;
  extent_index = (Bit32u)0;
  extent_offset = (Bit32u)0;
  extent_next = (Bit32u)0;
}

void redolog_t::print_header()
{
  BX_INFO(("redolog : Standard Header : magic='%s', type='%s', subtype='%s', version = %d.%d",
           header.standard.magic, header.standard.type, header.standard.subtype,
           dtoh32(header.standard.version)/0x10000,
           dtoh32(header.standard.version)%0x10000));
  if (dtoh32(header.standard.version) == STANDARD_HEADER_VERSION) {
    BX_INFO(("redolog : Specific Header : #entries=%d, bitmap size=%d, exent size = %d disk size = " FMT_LL "d",
             dtoh32(header.specific.catalog),
             dtoh32(header.specific.bitmap),
             dtoh32(header.specific.extent),
             dtoh64(header.specific.disk)));
  } else if (dtoh32(header.standard.version) == STANDARD_HEADER_V1) {
    redolog_header_v1_t header_v1;
    memcpy(&header_v1, &header, STANDARD_HEADER_SIZE);
    BX_INFO(("redolog : Specific Header : #entries=%d, bitmap size=%d, exent size = %d disk size = " FMT_LL "d",
             dtoh32(header_v1.specific.catalog),
             dtoh32(header_v1.specific.bitmap),
             dtoh32(header_v1.specific.extent),
             dtoh64(header_v1.specific.disk)));
  }
}

int redolog_t::make_header(const char* type, Bit64u size)
{
  Bit32u entries, extent_size, bitmap_size;
  Bit64u maxsize;
  Bit32u flip=0;

  // Set standard header values
  memset(&header, 0, sizeof(redolog_header_t));
  strcpy((char*)header.standard.magic, STANDARD_HEADER_MAGIC);
  strcpy((char*)header.standard.type, REDOLOG_TYPE);
  strcpy((char*)header.standard.subtype, type);
  header.standard.version = htod32(STANDARD_HEADER_VERSION);
  header.standard.header = htod32(STANDARD_HEADER_SIZE);

  entries = 512;
  bitmap_size = 1;

  // Compute #entries and extent size values
  do {
    extent_size = 8 * bitmap_size * 512;

    header.specific.catalog = htod32(entries);
    header.specific.bitmap = htod32(bitmap_size);
    header.specific.extent = htod32(extent_size);

    maxsize = (Bit64u)entries * (Bit64u)extent_size;

    flip++;

    if(flip&0x01) bitmap_size *= 2;
    else entries *= 2;
  } while (maxsize < size);

  header.specific.timestamp = 0;
  header.specific.disk = htod64(size);

  print_header();

  catalog = new Bit32u[dtoh32(header.specific.catalog)];
  bitmap =  new Bit8u[dtoh32(header.specific.bitmap)];

  if ((catalog == NULL) || (bitmap==NULL))
    BX_PANIC(("redolog : could not malloc catalog or bitmap"));

  for (Bit32u i=0; i<dtoh32(header.specific.catalog); i++)
    catalog[i] = htod32(REDOLOG_PAGE_NOT_ALLOCATED);

  bitmap_blocks = 1 + (dtoh32(header.specific.bitmap) - 1) / 512;
  extent_blocks = 1 + (dtoh32(header.specific.extent) - 1) / 512;

  BX_DEBUG(("redolog : each bitmap is %d blocks", bitmap_blocks));
  BX_DEBUG(("redolog : each extent is %d blocks", extent_blocks));

  return 0;
}

int redolog_t::create(const char* filename, const char* type, Bit64u size)
{
#ifndef BXIMAGE
  char lockfn[BX_PATHNAME_LEN];

  sprintf(lockfn, "%s.lock", filename);
  if (access(lockfn, F_OK) == 0) {
    return -1;
  }
#endif

  BX_INFO(("redolog : creating redolog %s", filename));

  int filedes = ::open(filename, O_RDWR | O_CREAT | O_TRUNC
#ifdef O_BINARY
            | O_BINARY
#endif
            , S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP);

  return create(filedes, type, size);
}

int redolog_t::create(int filedes, const char* type, Bit64u size)
{
  fd = filedes;

  if (fd < 0)
  {
    return -1; // open failed
  }

  if (make_header(type, size) < 0)
  {
    return -1;
  }

  // Write header
  ::write(fd, &header, dtoh32(header.standard.header));

  // Write catalog
  // FIXME could mmap
  ::write(fd, catalog, dtoh32(header.specific.catalog) * sizeof (Bit32u));

  return 0;
}

int redolog_t::open(const char* filename, const char *type)
{
  return open(filename, type, O_RDWR);
}

int redolog_t::open(const char* filename, const char *type, int flags)
{
  Bit64u imgsize = 0;
#ifndef WIN32
  time_t mtime;
#else
  FILETIME mtime;
#endif

  pathname = new char[strlen(filename) + 1];
  strcpy(pathname, filename);
  fd = hdimage_open_file(filename, flags, &imgsize, &mtime);
  if (fd < 0) {
    BX_INFO(("redolog : could not open image %s", filename));
    // open failed.
    return -1;
  }
  BX_INFO(("redolog : open image %s", filename));

  int res = check_format(fd, type);
  if (res != HDIMAGE_FORMAT_OK) {
    switch (res) {
      case HDIMAGE_READ_ERROR:
        BX_PANIC(("redolog : could not read header"));
        break;
      case HDIMAGE_NO_SIGNATURE:
        BX_PANIC(("redolog : Bad header magic"));
        break;
      case HDIMAGE_TYPE_ERROR:
        BX_PANIC(("redolog : Bad header type or subtype"));
        break;
      case HDIMAGE_VERSION_ERROR:
        BX_PANIC(("redolog : Bad header version"));
        break;
    }
    return -1;
  }

  if (bx_read_image(fd, 0, &header, sizeof(header)) < 0) {
    return -1;
  }
  print_header();

  if (dtoh32(header.standard.version) == STANDARD_HEADER_V1) {
    redolog_header_v1_t header_v1;

    memcpy(&header_v1, &header, STANDARD_HEADER_SIZE);
    header.specific.disk = header_v1.specific.disk;
  }
  if (!strcmp(type, REDOLOG_SUBTYPE_GROWING)) {
    set_timestamp(fat_datetime(mtime, 1) | (fat_datetime(mtime, 0) << 16));
  }

  catalog = new Bit32u[dtoh32(header.specific.catalog)];

  // FIXME could mmap
  res = bx_read_image(fd, dtoh32(header.standard.header), catalog, dtoh32(header.specific.catalog) * sizeof(Bit32u));

  if (res !=  (ssize_t)(dtoh32(header.specific.catalog) * sizeof(Bit32u)))
  {
    BX_PANIC(("redolog : could not read catalog %d=%d",res, dtoh32(header.specific.catalog)));
    return -1;
  }

  // check last used extent
  extent_next = 0;
  for (Bit32u i=0; i < dtoh32(header.specific.catalog); i++)
  {
    if (dtoh32(catalog[i]) != REDOLOG_PAGE_NOT_ALLOCATED)
    {
      if (dtoh32(catalog[i]) >= extent_next)
        extent_next = dtoh32(catalog[i]) + 1;
    }
  }
  BX_INFO(("redolog : next extent will be at index %d",extent_next));

  // memory used for storing bitmaps
  bitmap = new Bit8u[dtoh32(header.specific.bitmap)];

  bitmap_blocks = 1 + (dtoh32(header.specific.bitmap) - 1) / 512;
  extent_blocks = 1 + (dtoh32(header.specific.extent) - 1) / 512;

  BX_DEBUG(("redolog : each bitmap is %d blocks", bitmap_blocks));
  BX_DEBUG(("redolog : each extent is %d blocks", extent_blocks));

  imagepos = 0;
  bitmap_update = 1;

  return 0;
}

void redolog_t::close()
{
  if (fd >= 0)
    bx_close_image(fd, pathname);

  if (pathname != NULL)
    delete [] pathname;

  if (catalog != NULL)
    delete [] catalog;

  if (bitmap != NULL)
    delete [] bitmap;
}

Bit64u redolog_t::get_size()
{
  return dtoh64(header.specific.disk);
}

Bit32u redolog_t::get_timestamp()
{
  return dtoh32(header.specific.timestamp);
}

bool redolog_t::set_timestamp(Bit32u timestamp)
{
  header.specific.timestamp = htod32(timestamp);
  // Update header
  bx_write_image(fd, 0, &header, dtoh32(header.standard.header));
  return 1;
}

Bit64s redolog_t::lseek(Bit64s offset, int whence)
{
  if ((offset % 512) != 0) {
    BX_PANIC(("redolog : lseek() offset not multiple of 512"));
    return -1;
  }
  if (whence == SEEK_SET) {
    imagepos = offset;
  } else if (whence == SEEK_CUR) {
    imagepos += offset;
  } else {
    BX_PANIC(("redolog: lseek() mode not supported yet"));
    return -1;
  }
  if (imagepos > (Bit64s)dtoh64(header.specific.disk)) {
    BX_PANIC(("redolog : lseek() to byte %ld failed", (long)offset));
    return -1;
  }

  Bit32u old_extent_index = extent_index;
  extent_index = (Bit32u)(imagepos / dtoh32(header.specific.extent));
  if (extent_index != old_extent_index) {
    bitmap_update = 1;
  }
  extent_offset = (Bit32u)((imagepos % dtoh32(header.specific.extent)) / 512);

  BX_DEBUG(("redolog : lseeking extent index %d, offset %d",extent_index, extent_offset));

  return imagepos;
}

ssize_t redolog_t::read(void* buf, size_t count)
{
  Bit64s block_offset, bitmap_offset;
  ssize_t ret;

  if (count != 512) {
    BX_PANIC(("redolog : read() with count not 512"));
    return -1;
  }

  BX_DEBUG(("redolog : reading index %d, mapping to %d", extent_index, dtoh32(catalog[extent_index])));

  if (dtoh32(catalog[extent_index]) == REDOLOG_PAGE_NOT_ALLOCATED) {
    // page not allocated
    return 0;
  }

  bitmap_offset  = (Bit64s)STANDARD_HEADER_SIZE + (dtoh32(header.specific.catalog) * sizeof(Bit32u));
  bitmap_offset += (Bit64s)512 * dtoh32(catalog[extent_index]) * (extent_blocks + bitmap_blocks);
  block_offset    = bitmap_offset + ((Bit64s)512 * (bitmap_blocks + extent_offset));

  BX_DEBUG(("redolog : bitmap offset is %x", (Bit32u)bitmap_offset));
  BX_DEBUG(("redolog : block offset is %x", (Bit32u)block_offset));

  if (bitmap_update) {
    if (bx_read_image(fd, (off_t)bitmap_offset, bitmap,  dtoh32(header.specific.bitmap)) != (ssize_t)dtoh32(header.specific.bitmap)) {
      BX_PANIC(("redolog : failed to read bitmap for extent %d", extent_index));
      return -1;
    }
    bitmap_update = 0;
  }

  if (((bitmap[extent_offset/8] >> (extent_offset%8)) & 0x01) == 0x00) {
    BX_DEBUG(("read not in redolog"));

    // bitmap says block not in redolog
    return 0;
  }

  ret = bx_read_image(fd, (off_t)block_offset, buf, count);
  if (ret >= 0) lseek(512, SEEK_CUR);

  return ret;
}

ssize_t redolog_t::write(const void* buf, size_t count)
{
  Bit32u i;
  Bit64s block_offset, bitmap_offset, catalog_offset;
  ssize_t written;
  bool update_catalog = 0;

  if (count != 512) {
    BX_PANIC(("redolog : write() with count not 512"));
    return -1;
  }

  BX_DEBUG(("redolog : writing index %d, mapping to %d", extent_index, dtoh32(catalog[extent_index])));

  if (dtoh32(catalog[extent_index]) == REDOLOG_PAGE_NOT_ALLOCATED) {
    if (extent_next >= dtoh32(header.specific.catalog)) {
      BX_PANIC(("redolog : can't allocate new extent... catalog is full"));
      return -1;
    }

    BX_DEBUG(("redolog : allocating new extent at %d", extent_next));

    // Extent not allocated, allocate new
    catalog[extent_index] = htod32(extent_next);

    extent_next += 1;

    char *zerobuffer = new char[512];
    memset(zerobuffer, 0, 512);

    // Write bitmap
    bitmap_offset  = (Bit64s)STANDARD_HEADER_SIZE + (dtoh32(header.specific.catalog) * sizeof(Bit32u));
    bitmap_offset += (Bit64s)512 * dtoh32(catalog[extent_index]) * (extent_blocks + bitmap_blocks);
    ::lseek(fd, (off_t)bitmap_offset, SEEK_SET);
    for (i=0; i<bitmap_blocks; i++) {
      ::write(fd, zerobuffer, 512);
    }
    // Write extent
    for (i=0; i<extent_blocks; i++) {
      ::write(fd, zerobuffer, 512);
    }

    delete [] zerobuffer;

    update_catalog = 1;
  }

  bitmap_offset  = (Bit64s)STANDARD_HEADER_SIZE + (dtoh32(header.specific.catalog) * sizeof(Bit32u));
  bitmap_offset += (Bit64s)512 * dtoh32(catalog[extent_index]) * (extent_blocks + bitmap_blocks);
  block_offset    = bitmap_offset + ((Bit64s)512 * (bitmap_blocks + extent_offset));

  BX_DEBUG(("redolog : bitmap offset is %x", (Bit32u)bitmap_offset));
  BX_DEBUG(("redolog : block offset is %x", (Bit32u)block_offset));

  // Write block
  written = bx_write_image(fd, (off_t)block_offset, (void*)buf, count);

  // Write bitmap
  if (bitmap_update) {
    if (bx_read_image(fd, (off_t)bitmap_offset, bitmap,  dtoh32(header.specific.bitmap)) != (ssize_t)dtoh32(header.specific.bitmap)) {
      BX_PANIC(("redolog : failed to read bitmap for extent %d", extent_index));
      return 0;
    }
    bitmap_update = 0;
  }

  // If bloc does not belong to extent yet
  if (((bitmap[extent_offset/8] >> (extent_offset%8)) & 0x01) == 0x00) {
    bitmap[extent_offset/8] |= 1 << (extent_offset%8);
    bx_write_image(fd, (off_t)bitmap_offset, bitmap,  dtoh32(header.specific.bitmap));
  }

  // Write catalog
  if (update_catalog) {
    // FIXME if mmap
    catalog_offset  = (Bit64s)STANDARD_HEADER_SIZE + (extent_index * sizeof(Bit32u));

    BX_DEBUG(("redolog : writing catalog at offset %x", (Bit32u)catalog_offset));

    bx_write_image(fd, (off_t)catalog_offset, &catalog[extent_index], sizeof(Bit32u));
  }

  if (written >= 0) lseek(512, SEEK_CUR);

  return written;
}

int redolog_t::check_format(int fd, const char *subtype)
{
  redolog_header_t temp_header;

  int res = bx_read_image(fd, 0, &temp_header, sizeof(redolog_header_t));
  if (res != STANDARD_HEADER_SIZE) {
    return HDIMAGE_READ_ERROR;
  }

  if (strcmp((char*)temp_header.standard.magic, STANDARD_HEADER_MAGIC) != 0) {
    return HDIMAGE_NO_SIGNATURE;
  }

  if (strcmp((char*)temp_header.standard.type, REDOLOG_TYPE) != 0) {
    return HDIMAGE_TYPE_ERROR;
  }
  if (strcmp((char*)temp_header.standard.subtype, subtype) != 0) {
    return HDIMAGE_TYPE_ERROR;
  }

  if ((dtoh32(temp_header.standard.version) != STANDARD_HEADER_VERSION) &&
      (dtoh32(temp_header.standard.version) != STANDARD_HEADER_V1)) {
    return HDIMAGE_VERSION_ERROR;
  }
  return HDIMAGE_FORMAT_OK;
}

#ifdef BXIMAGE
int redolog_t::commit(device_image_t *base_image)
{
  int ret = 0;
  Bit32u i;
  Bit8u buffer[512];

  printf("\nCommitting changes to base image file: [  0%%]");

  for (i = 0; i < dtoh32(header.specific.catalog); i++) {
    printf("\x8\x8\x8\x8\x8%3d%%]", (i+1)*100/dtoh32(header.specific.catalog));
    fflush(stdout);

    if (dtoh32(catalog[i]) != REDOLOG_PAGE_NOT_ALLOCATED) {
      Bit64s bitmap_offset;
      Bit32u bitmap_size, j;

      bitmap_offset  = (Bit64s)STANDARD_HEADER_SIZE + (dtoh32(header.specific.catalog) * sizeof(Bit32u));
      bitmap_offset += (Bit64s)512 * dtoh32(catalog[i]) * (extent_blocks + bitmap_blocks);

      // Read bitmap
      bitmap_size = dtoh32(header.specific.bitmap);
      if ((Bit32u)bx_read_image(fd, (off_t)bitmap_offset, bitmap, bitmap_size) != bitmap_size) {
        ret = -1;
        break;
      }

      for (j = 0; j < dtoh32(header.specific.bitmap); j++) {
        Bit32u bit;

        for (bit = 0; bit < 8; bit++) {
          if ( (bitmap[j] & (1 << bit)) != 0) {
            Bit64s base_offset, block_offset;

            block_offset = bitmap_offset + ((Bit64s)512 * (bitmap_blocks + ((j * 8) + bit)));

            if (bx_read_image(fd, (off_t)block_offset, buffer, 512) != 512) {
              ret = -1;
              break;
            }

            base_offset  = (Bit64s)i * (dtoh32(header.specific.extent));
            base_offset += (Bit64s)512 * ((j * 8) + bit);

            if (base_image->lseek(base_offset, SEEK_SET) < 0) {
              ret = -1;
              break;
            }
            if (base_image->write(buffer, 512) < 0) {
              ret = -1;
              break;
            }
          }
        }
      }
    }
  }
  return ret;
}
#endif

#ifndef BXIMAGE
bool redolog_t::save_state(const char *backup_fname)
{
  return hdimage_backup_file(fd, backup_fname);
}
#endif

/*** growing_image_t function definitions ***/

growing_image_t::growing_image_t()
{
  redolog = new redolog_t();
}

growing_image_t::~growing_image_t()
{
  delete redolog;
}

int growing_image_t::open(const char* _pathname, int flags)
{
  pathname = _pathname;
  int filedes = redolog->open(pathname, REDOLOG_SUBTYPE_GROWING, flags);
  hd_size = redolog->get_size();
  BX_INFO(("'growing' disk opened, growing file is '%s'", pathname));
  return filedes;
}

void growing_image_t::close()
{
  redolog->close();
}

Bit64s growing_image_t::lseek(Bit64s offset, int whence)
{
  return redolog->lseek(offset, whence);
}

ssize_t growing_image_t::read(void* buf, size_t count)
{
  char *cbuf = (char*)buf;
  size_t n = 0;
  ssize_t ret = 0;

  memset(buf, 0, count);
  while (n < count) {
    ret = redolog->read(cbuf, 512);
    if (ret < 0) break;
    cbuf += 512;
    n += 512;
  }
  return (ret < 0) ? ret : count;
}

ssize_t growing_image_t::write(const void* buf, size_t count)
{
  char *cbuf = (char*)buf;
  size_t n = 0;
  ssize_t ret = 0;

  while (n < count) {
    ret = redolog->write(cbuf, 512);
    if (ret < 0) break;
    cbuf += 512;
    n += 512;
  }
  return (ret < 0) ? ret : count;
}

Bit32u growing_image_t::get_timestamp()
{
  return redolog->get_timestamp();
}

int growing_image_t::check_format(int fd, Bit64u imgsize)
{
  return redolog_t::check_format(fd, REDOLOG_SUBTYPE_GROWING);
}

#ifdef BXIMAGE
int growing_image_t::create_image(const char *pathname, Bit64u size)
{
  redolog = new redolog_t;
  if (redolog->create(pathname, REDOLOG_SUBTYPE_GROWING, size) < 0)
    BX_FATAL(("Can't create growing mode image"));
  redolog->close();
  return 0;
}
#else
bool growing_image_t::save_state(const char *backup_fname)
{
  return redolog->save_state(backup_fname);
}

void growing_image_t::restore_state(const char *backup_fname)
{
  redolog_t *temp_redolog = new redolog_t();
  if (temp_redolog->open(backup_fname, REDOLOG_SUBTYPE_GROWING, O_RDONLY) < 0) {
    delete temp_redolog;
    BX_PANIC(("Can't open growing image backup '%s'", backup_fname));
    return;
  } else {
    bool okay = (temp_redolog->get_size() == redolog->get_size());
    temp_redolog->close();
    delete temp_redolog;
    if (!okay) {
      BX_PANIC(("size reported by backup doesn't match growing disk size"));
      return;
    }
  }
  redolog->close();
  if (!hdimage_copy_file(backup_fname, pathname)) {
    BX_PANIC(("Failed to restore growing image '%s'", pathname));
    return;
  }
  if (device_image_t::open(pathname) < 0) {
    BX_PANIC(("Failed to open restored growing image '%s'", pathname));
  }
}
#endif

// compare hd_size and modification time of r/o disk and journal

bool coherency_check(device_image_t *ro_disk, redolog_t *redolog)
{
  Bit32u timestamp1, timestamp2;
  char buffer[24];

  if (ro_disk->hd_size != redolog->get_size()) {
    BX_PANIC(("size reported by redolog doesn't match r/o disk size"));
    return 0;
  }
  timestamp1 = ro_disk->get_timestamp();
  timestamp2 = redolog->get_timestamp();
  if (timestamp2 != 0) {
    if (timestamp1 != timestamp2) {
      sprintf(buffer, "%02d.%02d.%04d %02d:%02d:%02d", (timestamp2 >> 16) & 0x001f,
              (timestamp2 >> 21) & 0x000f, ((timestamp2 >> 25) & 0x007f) + 1980,
              (timestamp2 & 0xf800) >> 11, (timestamp2 & 0x07e0) >> 5,
              (timestamp2 & 0x001f) << 1);
      BX_PANIC(("unexpected modification time of the r/o disk (should be %s)", buffer));
      return 0;
    }
  } else if (timestamp1 != 0) {
    redolog->set_timestamp(timestamp1);
  }
  return 1;
}

/*** undoable_image_t function definitions ***/

undoable_image_t::undoable_image_t(const char* _redolog_name)
{
  redolog = new redolog_t();
  redolog_name = NULL;
  if (_redolog_name != NULL) {
    if ((strlen(_redolog_name) > 0) && (strcmp(_redolog_name,"none") != 0)) {
      redolog_name = new char[strlen(_redolog_name) + 1];
      strcpy(redolog_name, _redolog_name);
    }
  }
}

undoable_image_t::~undoable_image_t()
{
  delete redolog;
  delete ro_disk;
}

int undoable_image_t::open(const char* pathname, int flags)
{
  const char* image_mode = NULL;

  UNUSED(flags);
  if (access(pathname, F_OK) < 0) {
    BX_PANIC(("r/o disk image doesn't exist"));
  }
  if (!hdimage_detect_image_mode(pathname, &image_mode)) {
    BX_PANIC(("r/o disk image mode not detected"));
    return -1;
  } else {
    BX_INFO(("base image mode = '%s'", image_mode));
  }
  ro_disk = DEV_hdimage_init_image(image_mode, 0, NULL);
  if (ro_disk == NULL) {
    return -1;
  }
  if (ro_disk->open(pathname, O_RDONLY) < 0)
    return -1;

  hd_size = ro_disk->hd_size;
  if (ro_disk->get_capabilities() & HDIMAGE_HAS_GEOMETRY) {
    cylinders = ro_disk->cylinders;
    heads = ro_disk->heads;
    spt = ro_disk->spt;
    caps = HDIMAGE_HAS_GEOMETRY;
  } else if (cylinders == 0) {
    caps = HDIMAGE_AUTO_GEOMETRY;
  }
  sect_size = ro_disk->sect_size;

  // If not set, we make up the redolog filename from the pathname
  if (redolog_name == NULL) {
    redolog_name = new char[strlen(pathname) + UNDOABLE_REDOLOG_EXTENSION_LENGTH + 1];
    sprintf(redolog_name, "%s%s", pathname, UNDOABLE_REDOLOG_EXTENSION);
  }

  if (redolog->open(redolog_name, REDOLOG_SUBTYPE_UNDOABLE) < 0) {
    if (redolog->create(redolog_name, REDOLOG_SUBTYPE_UNDOABLE, hd_size) < 0) {
      BX_PANIC(("Can't open or create redolog '%s'",redolog_name));
      return -1;
    }
  }
  if (!coherency_check(ro_disk, redolog)) {
    close();
    return -1;
  }

  BX_INFO(("'undoable' disk opened: ro-file is '%s', redolog is '%s'", pathname, redolog_name));

  return 0;
}

void undoable_image_t::close()
{
  redolog->close();
  ro_disk->close();

  if (redolog_name != NULL)
    delete [] redolog_name;
}

Bit64s undoable_image_t::lseek(Bit64s offset, int whence)
{
  redolog->lseek(offset, whence);
  return ro_disk->lseek(offset, whence);
}

ssize_t undoable_image_t::read(void* buf, size_t count)
{
  char *cbuf = (char*)buf;
  size_t n = 0;
  ssize_t ret = 0;

  while (n < count) {
    if ((size_t)redolog->read(cbuf, 512) != 512) {
      ret = ro_disk->read(cbuf, 512);
      if (ret < 0) break;
    }
    cbuf += 512;
    n += 512;
  }
  return (ret < 0) ? ret : count;
}

ssize_t undoable_image_t::write(const void* buf, size_t count)
{
  char *cbuf = (char*)buf;
  size_t n = 0;
  ssize_t ret = 0;

  while (n < count) {
    ret = redolog->write(cbuf, 512);
    if (ret < 0) break;
    cbuf += 512;
    n += 512;
  }
  return (ret < 0) ? ret : count;
}

#ifndef BXIMAGE
bool undoable_image_t::save_state(const char *backup_fname)
{
  return redolog->save_state(backup_fname);
}

void undoable_image_t::restore_state(const char *backup_fname)
{
  redolog_t *temp_redolog = new redolog_t();
  if (temp_redolog->open(backup_fname, REDOLOG_SUBTYPE_UNDOABLE, O_RDONLY) < 0) {
    delete temp_redolog;
    BX_PANIC(("Can't open undoable redolog backup '%s'", backup_fname));
    return;
  } else {
    bool okay = coherency_check(ro_disk, temp_redolog);
    temp_redolog->close();
    delete temp_redolog;
    if (!okay) return;
  }
  redolog->close();
  if (!hdimage_copy_file(backup_fname, redolog_name)) {
    BX_PANIC(("Failed to restore undoable redolog '%s'", redolog_name));
    return;
  } else {
    if (redolog->open(redolog_name, REDOLOG_SUBTYPE_UNDOABLE) < 0) {
      BX_PANIC(("Can't open restored undoable redolog '%s'", redolog_name));
    }
  }
}
#endif

/*** volatile_image_t function definitions ***/

volatile_image_t::volatile_image_t(const char* _redolog_name)
{
  redolog = new redolog_t();
  redolog_temp = NULL;
  redolog_name = NULL;
  if (_redolog_name != NULL) {
    if ((strlen(_redolog_name) > 0) && (strcmp(_redolog_name,"none") != 0)) {
      redolog_name = new char[strlen(_redolog_name) + 1];
      strcpy(redolog_name, _redolog_name);
    }
  }
}

volatile_image_t::~volatile_image_t()
{
  delete redolog;
  delete ro_disk;
}

int volatile_image_t::open(const char* pathname, int flags)
{
  int filedes;
  Bit32u timestamp;
  const char* image_mode = NULL;

  UNUSED(flags);
  if (access(pathname, F_OK) < 0) {
    BX_PANIC(("r/o disk image doesn't exist"));
  }
  if (!hdimage_detect_image_mode(pathname, &image_mode)) {
    BX_PANIC(("r/o disk image mode not detected"));
    return -1;
  } else {
    BX_INFO(("base image mode = '%s'", image_mode));
  }
  ro_disk = DEV_hdimage_init_image(image_mode, 0, NULL);
  if (ro_disk == NULL) {
    return -1;
  }
  if (ro_disk->open(pathname, O_RDONLY)<0)
    return -1;

  hd_size = ro_disk->hd_size;
  if (ro_disk->get_capabilities() & HDIMAGE_HAS_GEOMETRY) {
    cylinders = ro_disk->cylinders;
    heads = ro_disk->heads;
    spt = ro_disk->spt;
    caps = HDIMAGE_HAS_GEOMETRY;
  } else if (cylinders == 0) {
    caps = HDIMAGE_AUTO_GEOMETRY;
  }
  sect_size = ro_disk->sect_size;

  // If not set, use pathname as template
  if (redolog_name == NULL) {
    redolog_name = new char[strlen(pathname) + 1];
    strcpy(redolog_name, pathname);
  }

  redolog_temp = new char[strlen(redolog_name) + VOLATILE_REDOLOG_EXTENSION_LENGTH + 1];
  sprintf(redolog_temp, "%s%s", redolog_name, VOLATILE_REDOLOG_EXTENSION);

  filedes = mkstemp(redolog_temp);

  if (filedes < 0) {
    BX_PANIC(("Can't create volatile redolog '%s'", redolog_temp));
    return -1;
  }
  if (redolog->create(filedes, REDOLOG_SUBTYPE_VOLATILE, hd_size) < 0) {
    BX_PANIC(("Can't create volatile redolog '%s'", redolog_temp));
    return -1;
  }

#if (!defined(WIN32)) && !BX_WITH_MACOS
  // on unix it is legal to delete an open file
  unlink(redolog_temp);
#endif

  // timestamp required for save/restore support
  timestamp = ro_disk->get_timestamp();
  redolog->set_timestamp(timestamp);

  BX_INFO(("'volatile' disk opened: ro-file is '%s', redolog is '%s'", pathname, redolog_temp));

  return 0;
}

void volatile_image_t::close()
{
  redolog->close();
  ro_disk->close();

#if defined(WIN32) || BX_WITH_MACOS
  // on non-unix we have to wait till the file is closed to delete it
  unlink(redolog_temp);
#endif
  if (redolog_temp!=NULL)
    delete [] redolog_temp;

  if (redolog_name!=NULL)
    delete [] redolog_name;
}

Bit64s volatile_image_t::lseek(Bit64s offset, int whence)
{
  redolog->lseek(offset, whence);
  return ro_disk->lseek(offset, whence);
}

ssize_t volatile_image_t::read(void* buf, size_t count)
{
  char *cbuf = (char*)buf;
  size_t n = 0;
  ssize_t ret = 0;

  while (n < count) {
    if ((size_t)redolog->read(cbuf, 512) != 512) {
      ret = ro_disk->read(cbuf, 512);
      if (ret < 0) break;
    }
    cbuf += 512;
    n += 512;
  }
  return (ret < 0) ? ret : count;
}

ssize_t volatile_image_t::write(const void* buf, size_t count)
{
  char *cbuf = (char*)buf;
  size_t n = 0;
  ssize_t ret = 0;

  while (n < count) {
    ret = redolog->write(cbuf, 512);
    if (ret < 0) break;
    cbuf += 512;
    n += 512;
  }
  return (ret < 0) ? ret : count;
}

#ifndef BXIMAGE
bool volatile_image_t::save_state(const char *backup_fname)
{
  return redolog->save_state(backup_fname);
}

void volatile_image_t::restore_state(const char *backup_fname)
{
  redolog_t *temp_redolog = new redolog_t();
  if (temp_redolog->open(backup_fname, REDOLOG_SUBTYPE_VOLATILE, O_RDONLY) < 0) {
    delete temp_redolog;
    BX_PANIC(("Can't open volatile redolog backup '%s'", backup_fname));
    return;
  } else {
    bool okay = coherency_check(ro_disk, temp_redolog);
    temp_redolog->close();
    delete temp_redolog;
    if (!okay) return;
  }
  redolog->close();
  if (!hdimage_copy_file(backup_fname, redolog_temp)) {
    BX_PANIC(("Failed to restore volatile redolog '%s'", redolog_temp));
    return;
  } else {
    if (redolog->open(redolog_temp, REDOLOG_SUBTYPE_VOLATILE) < 0) {
      BX_PANIC(("Can't open restored volatile redolog '%s'", redolog_temp));
      return;
    }
  }
#if (!defined(WIN32)) && !BX_WITH_MACOS
  // on unix it is legal to delete an open file
  unlink(redolog_temp);
#endif
}
#endif
