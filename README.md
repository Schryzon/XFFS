# XFFS — 西莱宗-复芯文件系统
## Xīláizōng Fùxīn Wénjiàn Xìtǒng

<p align="center">
  <img src="assets/xffs_logo.jpeg" width="420" alt="XFFS logo">
</p>

**XFFS (Xilaizong-Fuxin File System)** is a teaching-oriented user-space filesystem written in C++17.
This project focuses on demonstrating core filesystem concepts such as block management, inode structures, and directory handling using a virtual disk.

---

## ✨ Project Goals

* User-space virtual disk filesystem
* Clear educational design
* Cross-platform (Windows & Linux)
* Bitmap block allocation
* Inode-based file management
* Extensible for advanced features

---

## 🧱 Current Features

* ✔ Superblock layout
* ✔ Block bitmap initialization
* ✔ Reserved block marking
* ✔ Virtual disk formatter (`mkxffs`)
* ✔ Block allocator (`alloc_block` / `free_block`)
* ✔ Inode I/O (`read_inode` / `write_inode`) with XOR checksums
* ✔ Root inode creation on format
* ✔ Directory entry operations (`dir_init` / `dir_add` / `dir_lookup`)
* ✔ Path resolution (`/path/to/file`)
* ✔ Subdirectory support (`dir_create`)
* ✔ Interactive CLI Shell (`xffs_shell`)

---

## 🛠 Requirements

### Windows

* Visual Studio 2019+ (with C++ workload)
* CMake ≥ 3.16
* PowerShell

### Linux

* GCC or Clang with C++17 support
* CMake ≥ 3.16
* make or ninja

---

## 🚀 Build Instructions

From the project root:

```bash
cmake -S . -B build
cmake --build build
```

### Expected output

**Windows**

```
build/Debug/mkxffs.exe
```

**Linux**

```
build/mkxffs
```

---

## 💽 Create Virtual Disk

### Windows (PowerShell)

```powershell
fsutil file createnew xffs.img 67108864
```

### Linux

```bash
truncate -s 64M xffs.img
```

---

## 🧪 Format the Disk

### Windows

```powershell
.\build\Debug\mkxffs.exe .\xffs.img
```

### Linux

```bash
./build/mkxffs ./xffs.img
```

### Expected output

```
[xffs] reserved blocks: 0-65
[xffs] data blocks start at: 66
xffs formatted successfully
```

---

## 🔍 Verify Bitmap (Debug)

### Windows

```powershell
(Get-Content xffs.img -Encoding Byte -TotalCount (4096 + 64)) |
Select-Object -Skip 4096 |
Format-Hex
```

### Linux

```bash
hexdump -C xffs.img | head -n 40
```

You should see non-zero bytes at the start of block 1 after formatting.

---

## 📁 Project Structure

```
xffs/
├─ include/xffs/   # on-disk structures and interfaces
├─ src/            # implementation
├─ tools/          # future utilities
├─ tests/          # test programs
├─ docs/           # design notes
└─ assets/         # logo and media
```

---

## 🧭 Roadmap

### Phase 1 (current)

* [x] Disk abstraction
* [x] Superblock
* [x] Bitmap initialization

### Phase 3: File Operations & Path Resolution
* [x] Inode allocation/deallocation logic
* [x] File lifecycle: create, read, write, delete
* [x] Path resolution (absolute paths)

### Phase 4: Integrity & Shell
* [x] Inode XOR Checksums
* [x] Interactive REPL Shell (`xffs_shell`)
* [x] Recursive directory deletion (`rm -r`)

### Phase 5 (Future)
* [ ] **Indirect Blocks**: Support files larger than 48 KB.
* [ ] **WinFsp Integration**: Mount XFFS as a real drive on Windows!
* [ ] **Multiblock Directories**: Support directories with more than 64 entries.

---

## 📜 License

MIT License (recommended for academic projects).

---

## 👥 Authors

**XFFS — Xilaizong-Fuxin File System**

Developed as a filesystem learning project.

---

💙 *Built with curiosity, low-level curiosity, and a bit of chaos.*
