#include "xffs/allocator.hpp"
#include "xffs/dir.hpp"
#include "xffs/disk.hpp"
#include "xffs/file_ops.hpp"
#include "xffs/inode_io.hpp"
#include "xffs/path.hpp"
#include "xffs/superblock.hpp"


#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


using namespace xffs;

// Helper to format permissions string from inode mode
std::string format_perms(uint16_t mode) {
  if (mode == INODE_DIR)
    return "drwxr-xr-x";
  if (mode == INODE_FILE)
    return "-rw-r--r--";
  return "----------";
}

// Split string by space and strip Windows carriage returns / BOM
std::vector<std::string> split_args(const std::string &cmd) {
  std::vector<std::string> args;
  std::istringstream iss(cmd);
  std::string s;
  while (iss >> s) {
    // Strip carriage returns just in case
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n')) {
      s.pop_back();
    }
    // Strip UTF-8 BOM from the very first token if present
    if (args.empty() && s.size() >= 3 &&
        static_cast<unsigned char>(s[0]) == 0xEF &&
        static_cast<unsigned char>(s[1]) == 0xBB &&
        static_cast<unsigned char>(s[2]) == 0xBF) {
      s = s.substr(3);
    }
    if (!s.empty()) {
      args.push_back(s);
    }
  }
  return args;
}

void print_help() {
  std::cout << "XFFS Interactive Shell Commands:\n"
            << "  ls [-la] <path>  - List directory contents\n"
            << "  mkdir <path>     - Create a directory\n"
            << "  touch <path>     - Create an empty file\n"
            << "  write <path>     - Write text data to a file\n"
            << "  cat <path>       - Read file contents\n"
            << "  rm [-r] <path>   - Remove file or directory\n"
            << "  stat <path>      - Show inode details\n"
            << "  help             - Show this help\n"
            << "  exit             - Exit shell\n";
}

void do_ls(disk &dsk, const superblock &sb,
           const std::vector<std::string> &args) {
  bool show_all = false;
  std::string path = "/";

  for (size_t i = 1; i < args.size(); i++) {
    if (args[i] == "-la" || args[i] == "-al" || args[i] == "-l" ||
        args[i] == "-a") {
      show_all = true;
    } else {
      path = args[i];
    }
  }

  uint64_t dir_ino_idx = path_resolve(dsk, sb, path.c_str());
  if (dir_ino_idx == UINT64_MAX) {
    std::cerr << "ls: cannot access '" << path
              << "': No such file or directory\n";
    return;
  }

  inode dir_ino;
  if (!read_inode(dsk, sb, dir_ino_idx, dir_ino) || dir_ino.mode != INODE_DIR) {
    std::cerr << "ls: '" << path << "' is not a directory\n";
    return;
  }

  auto entries = dir_list(dsk, sb, dir_ino_idx);
  for (const auto &entry : entries) {
    std::string name(entry.name);
    if (!show_all && name[0] == '.')
      continue; // hide hidden files unless -a

    if (show_all) {
      inode child_ino;
      if (read_inode(dsk, sb, entry.inode, child_ino)) {
        std::cout << format_perms(child_ino.mode) << " " << std::setw(4)
                  << child_ino.size << " " << name << "\n";
      }
    } else {
      std::cout << name << "  ";
    }
  }
  if (!show_all && !entries.empty())
    std::cout << "\n";
}

void do_mkdir(disk &dsk, const superblock &sb,
              const std::vector<std::string> &args) {
  if (args.size() < 2) {
    std::cerr << "mkdir: missing operand\n";
    return;
  }
  std::string path = args[1];

  // Simplistic split for parent/child (Assume UNIX style /parent/child)
  size_t last_slash = path.find_last_of('/');
  std::string parent_path =
      (last_slash == 0) ? "/" : path.substr(0, last_slash);
  if (parent_path.empty())
    parent_path = "/";
  std::string name = path.substr(last_slash + 1);

  uint64_t parent_ino = path_resolve(dsk, sb, parent_path.c_str());
  if (parent_ino == UINT64_MAX) {
    std::cerr << "mkdir: cannot create directory '" << path
              << "': No such file or directory\n";
    return;
  }

  if (dir_create(dsk, sb, parent_ino, name.c_str()) == UINT64_MAX) {
    std::cerr << "mkdir: cannot create directory '" << path
              << "': Failed (might already exist)\n";
  }
}

void do_touch(disk &dsk, const superblock &sb,
              const std::vector<std::string> &args) {
  if (args.size() < 2) {
    std::cerr << "touch: missing file operand\n";
    return;
  }
  std::string path = args[1];
  size_t last_slash = path.find_last_of('/');
  std::string parent_path =
      (last_slash == 0) ? "/" : path.substr(0, last_slash);
  if (parent_path.empty())
    parent_path = "/";
  std::string name = path.substr(last_slash + 1);

  uint64_t parent_ino = path_resolve(dsk, sb, parent_path.c_str());
  if (parent_ino == UINT64_MAX) {
    std::cerr << "touch: cannot touch '" << path
              << "': No such file or directory\n";
    return;
  }

  if (file_create(dsk, sb, parent_ino, name.c_str()) == UINT64_MAX) {
    std::cerr << "touch: cannot touch '" << path << "': Failed\n";
  }
}

void do_write(disk &dsk, const superblock &sb,
              const std::vector<std::string> &args) {
  if (args.size() < 3) {
    std::cerr << "write: usage: write <path> <text>\n";
    return;
  }
  std::string path = args[1];
  std::string data = args[2];
  for (size_t i = 3; i < args.size(); i++) {
    data += " " + args[i]; // re-join string
  }

  uint64_t file_ino = path_resolve(dsk, sb, path.c_str());
  if (file_ino == UINT64_MAX) {
    std::cerr << "write: " << path << ": No such file\n";
    return;
  }

  if (!file_write(dsk, sb, file_ino, 0,
                  reinterpret_cast<const uint8_t *>(data.c_str()),
                  data.size())) {
    std::cerr << "write: failed to write data\n";
  }
}

void do_cat(disk &dsk, const superblock &sb,
            const std::vector<std::string> &args) {
  if (args.size() < 2) {
    std::cerr << "cat: missing file operand\n";
    return;
  }
  std::string path = args[1];
  uint64_t file_ino = path_resolve(dsk, sb, path.c_str());
  if (file_ino == UINT64_MAX) {
    std::cerr << "cat: " << path << ": No such file\n";
    return;
  }

  inode ino;
  if (!read_inode(dsk, sb, file_ino, ino) || ino.mode != INODE_FILE) {
    std::cerr << "cat: " << path << ": Is a directory or invalid\n";
    return;
  }

  std::vector<uint8_t> buffer(ino.size + 1, 0);
  if (file_read(dsk, sb, file_ino, 0, buffer.data(), ino.size)) {
    std::cout << reinterpret_cast<char *>(buffer.data()) << "\n";
  } else {
    std::cerr << "cat: failed to read data\n";
  }
}

bool recursive_rm(disk &dsk, const superblock &sb, uint64_t parent_ino,
                  uint64_t target_ino, const std::string &name) {
  inode target;
  if (!read_inode(dsk, sb, target_ino, target))
    return false;

  if (target.mode == INODE_DIR) {
    auto entries = dir_list(dsk, sb, target_ino);
    for (const auto &entry : entries) {
      std::string child_name(entry.name);
      if (child_name == "." || child_name == "..")
        continue;
      recursive_rm(dsk, sb, target_ino, entry.inode, child_name);
    }
  }
  // Delete target
  return file_delete(dsk, sb, parent_ino, name.c_str());
}

void do_rm(disk &dsk, const superblock &sb,
           const std::vector<std::string> &args) {
  if (args.size() < 2) {
    std::cerr << "rm: missing operand\n";
    return;
  }

  bool recursive = false;
  std::string path;

  for (size_t i = 1; i < args.size(); i++) {
    if (args[i] == "-r" || args[i] == "-rf") {
      recursive = true;
    } else {
      path = args[i];
    }
  }

  if (path.empty() || path == "/") {
    std::cerr << "rm: cannot remove root directory\n";
    return;
  }

  size_t last_slash = path.find_last_of('/');
  std::string parent_path =
      (last_slash == 0) ? "/" : path.substr(0, last_slash);
  if (parent_path.empty())
    parent_path = "/";
  std::string name = path.substr(last_slash + 1);

  uint64_t parent_ino = path_resolve(dsk, sb, parent_path.c_str());
  uint64_t target_ino = path_resolve(dsk, sb, path.c_str());

  if (target_ino == UINT64_MAX || parent_ino == UINT64_MAX) {
    std::cerr << "rm: cannot remove '" << path
              << "': No such file or directory\n";
    return;
  }

  inode target;
  read_inode(dsk, sb, target_ino, target);

  if (target.mode == INODE_DIR && !recursive) {
    std::cerr << "rm: cannot remove '" << path << "': Is a directory\n";
    return;
  }

  if (recursive) {
    recursive_rm(dsk, sb, parent_ino, target_ino, name);
  } else {
    if (!file_delete(dsk, sb, parent_ino, name.c_str())) {
      std::cerr << "rm: failed to remove '" << path << "'\n";
    }
  }
}

void do_stat(disk &dsk, const superblock &sb,
             const std::vector<std::string> &args) {
  if (args.size() < 2) {
    std::cerr << "stat: missing operand\n";
    return;
  }
  std::string path = args[1];
  uint64_t file_ino = path_resolve(dsk, sb, path.c_str());
  if (file_ino == UINT64_MAX) {
    std::cerr << "stat: cannot stat '" << path
              << "': No such file or directory\n";
    return;
  }

  inode ino;
  if (read_inode(dsk, sb, file_ino, ino)) {
    std::cout << "  File: " << path << "\n"
              << "  Size: " << ino.size
              << "\tBlocks: " << (ino.size > 0 ? 1 : 0) << "\n"
              << "  Mode: " << format_perms(ino.mode) << "\tInode: " << file_ino
              << "\n"
              << "  Checksum: " << std::hex << ino.checksum << std::dec << "\n";
  }
}

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <disk_image.img>\n";
    return 1;
  }

  disk dsk;
  if (!dsk.open(argv[1])) {
    std::cerr << "Failed to open disk image: " << argv[1] << "\n";
    return 1;
  }

  superblock sb;
  std::vector<uint8_t> sb_block(XFFS_BLOCK_SIZE);
  if (!dsk.read_block(0, sb_block.data())) {
    std::cerr << "Failed to read superblock.\n";
    return 1;
  }
  std::memcpy(&sb, sb_block.data(), sizeof(superblock));

  if (sb.magic != XFFS_MAGIC) {
    std::cerr << "Not a valid XFFS filesystem.\n";
    return 1;
  }

  std::cout << "XFFS Interactive Shell. Type 'help' for commands.\n";

  std::string line;
  while (true) {
    std::cout << "xffs> ";
    if (!std::getline(std::cin, line))
      break;

    auto args = split_args(line);
    if (args.empty())
      continue;

    const std::string &cmd = args[0];
    if (cmd == "exit" || cmd == "quit") {
      break;
    } else if (cmd == "help") {
      print_help();
    } else if (cmd == "ls") {
      do_ls(dsk, sb, args);
    } else if (cmd == "mkdir") {
      do_mkdir(dsk, sb, args);
    } else if (cmd == "touch") {
      do_touch(dsk, sb, args);
    } else if (cmd == "write") {
      do_write(dsk, sb, args);
    } else if (cmd == "cat") {
      do_cat(dsk, sb, args);
    } else if (cmd == "rm") {
      do_rm(dsk, sb, args);
    } else if (cmd == "stat") {
      do_stat(dsk, sb, args);
    } else {
      std::cerr << "xffs: command not found: " << cmd << "\n";
    }
  }

  dsk.close();
  return 0;
}
