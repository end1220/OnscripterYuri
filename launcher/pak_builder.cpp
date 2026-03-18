#include "resource_pak.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <algorithm>

// 本工具用于离线打包 launcher/assets 目录下的资源文件，生成 data.pak。
// 规则：
// - 扫描给定的 assets 目录；
// - 忽略子目录；
// - 忽略扩展名为 ".pak" 的文件；
// - 其余所有普通文件都打进一个 data.pak；
// - pak 内的逻辑名为相对文件名，例如 "bg.jpg"、"foo.png"。

namespace {

static constexpr const char PAK_MAGIC[4] = {'P', 'A', 'K', '1'};

struct SourceFile {
    std::string name;   // 逻辑名（如 "bg.jpg"）
    std::string path;   // 物理路径（如 "assets/bg.jpg"）
    std::uint64_t size; // 字节数
};

template <typename T>
bool fwrite_pod(std::FILE *f, const T &v) {
    return std::fwrite(&v, sizeof(T), 1, f) == 1;
}

bool fwrite_bytes(std::FILE *f, const void *buf, std::size_t len) {
    return std::fwrite(buf, 1, len, f) == len;
}

bool is_regular_file(const std::string &path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return false;
    }
    return S_ISREG(st.st_mode);
}

std::uint64_t file_size(const std::string &path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return 0;
    }
    return static_cast<std::uint64_t>(st.st_size);
}

bool has_pak_extension(const std::string &name) {
    auto pos = name.rfind('.');
    if (pos == std::string::npos) return false;
    std::string ext = name.substr(pos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext == "pak";
}

// 扫描 assets 目录，收集待打包的文件。
bool collect_source_files(const std::string &assetsDir, std::vector<SourceFile> &outFiles) {
    DIR *dir = opendir(assetsDir.c_str());
    if (!dir) {
        std::perror("opendir assets failed");
        return false;
    }

    outFiles.clear();

    for (;;) {
        dirent *ent = readdir(dir);
        if (!ent) break;

        const char *name = ent->d_name;
        if (std::strcmp(name, ".") == 0 || std::strcmp(name, "..") == 0) {
            continue;
        }

        std::string relName = name;  // 仅支持一层目录，直接用文件名作为逻辑名
        std::string fullPath = assetsDir + "/" + relName;

        if (!is_regular_file(fullPath)) {
            continue;
        }
        if (has_pak_extension(relName)) {
            continue;
        }

        std::uint64_t sz = file_size(fullPath);
        if (sz == 0) {
            // 允许 0 字节文件，但可视情况跳过
            // 这里仍然加入索引，方便占位。
        }

        SourceFile sf;
        sf.name = relName;
        sf.path = fullPath;
        sf.size = sz;
        outFiles.push_back(sf);
    }

    closedir(dir);
    return true;
}

bool build_pak(const std::string &assetsDir, const std::string &pakPath) {
    std::vector<SourceFile> files;
    if (!collect_source_files(assetsDir, files)) {
        return false;
    }

    if (files.empty()) {
        std::fprintf(stderr, "No source files found in %s\n", assetsDir.c_str());
        return false;
    }

    // 先计算索引区大小，以便确定每个文件数据块的 offset。
    std::uint32_t count = static_cast<std::uint32_t>(files.size());

    std::uint64_t indexSize = 0;
    for (const auto &f : files) {
        if (f.name.size() > 0xFFFFu) {
            std::fprintf(stderr, "File name too long: %s\n", f.name.c_str());
            return false;
        }
        indexSize += sizeof(std::uint16_t);          // nameLen
        indexSize += static_cast<std::uint64_t>(f.name.size()); // name bytes
        indexSize += sizeof(std::uint64_t);          // offset
        indexSize += sizeof(std::uint64_t);          // size
    }

    const std::uint64_t headerSize = 4 /*magic*/ + sizeof(std::uint32_t) /*fileCount*/;
    std::uint64_t dataOffsetBase = headerSize + indexSize;

    // 为每个源文件计算 offset。
    std::uint64_t curOffset = dataOffsetBase;
    for (auto &f : files) {
        f.size = file_size(f.path); // 再取一次，保证准确
        // 为了简单，零长度文件也分配 offset，但不会实际读取数据。
        f.size = f.size;
        f.size = f.size; // 保持编译器不报警告（此处仅强调逻辑）
        // 记录 offset 到临时字段：这里不再额外建结构体，直接算时使用。
        // curOffset 在写索引时再使用。
        curOffset += f.size;
    }

    std::FILE *out = std::fopen(pakPath.c_str(), "wb");
    if (!out) {
        std::perror("fopen pak for write failed");
        return false;
    }

    // 写 header
    if (!fwrite_bytes(out, PAK_MAGIC, sizeof(PAK_MAGIC))) {
        std::fprintf(stderr, "Write magic failed\n");
        std::fclose(out);
        return false;
    }
    if (!fwrite_pod(out, count)) {
        std::fprintf(stderr, "Write fileCount failed\n");
        std::fclose(out);
        return false;
    }

    // 写索引区，同时记录每个文件的 offset。
    std::uint64_t runningOffset = dataOffsetBase;
    for (const auto &f : files) {
        std::uint16_t nameLen = static_cast<std::uint16_t>(f.name.size());
        if (!fwrite_pod(out, nameLen)) {
            std::fprintf(stderr, "Write nameLen failed\n");
            std::fclose(out);
            return false;
        }
        if (nameLen > 0) {
            if (!fwrite_bytes(out, f.name.data(), nameLen)) {
                std::fprintf(stderr, "Write name bytes failed\n");
                std::fclose(out);
                return false;
            }
        }

        std::uint64_t offset = runningOffset;
        std::uint64_t size = file_size(f.path);

        if (!fwrite_pod(out, offset)) {
            std::fprintf(stderr, "Write offset failed\n");
            std::fclose(out);
            return false;
        }
        if (!fwrite_pod(out, size)) {
            std::fprintf(stderr, "Write size failed\n");
            std::fclose(out);
            return false;
        }

        runningOffset += size;
    }

    // 写数据区
    const std::size_t bufSize = 64 * 1024;
    std::vector<unsigned char> buffer(bufSize);

    for (const auto &f : files) {
        std::FILE *in = std::fopen(f.path.c_str(), "rb");
        if (!in) {
            std::fprintf(stderr, "Open source file failed: %s\n", f.path.c_str());
            std::fclose(out);
            return false;
        }

        for (;;) {
            std::size_t n = std::fread(buffer.data(), 1, buffer.size(), in);
            if (n > 0) {
                if (!fwrite_bytes(out, buffer.data(), n)) {
                    std::fprintf(stderr, "Write data failed for %s\n", f.path.c_str());
                    std::fclose(in);
                    std::fclose(out);
                    return false;
                }
            }
            if (n < buffer.size()) {
                if (std::feof(in)) {
                    break;
                } else {
                    std::fprintf(stderr, "Read error for %s\n", f.path.c_str());
                    std::fclose(in);
                    std::fclose(out);
                    return false;
                }
            }
        }

        std::fclose(in);
    }

    std::fclose(out);

    std::fprintf(stderr, "Build pak success: %s (files: %u)\n",
                 pakPath.c_str(), count);
    return true;
}

} // namespace

int main(int argc, char **argv) {
    const char *assetsDir = "assets";
    const char *pakName = "data.pak";

    if (argc >= 2) {
        assetsDir = argv[1];
    }
    if (argc >= 3) {
        pakName = argv[2];
    }

    if (!build_pak(assetsDir, pakName)) {
        std::fprintf(stderr, "Failed to build pak from '%s' to '%s'\n",
                     assetsDir, pakName);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

