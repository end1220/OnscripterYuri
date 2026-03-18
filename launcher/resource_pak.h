#ifndef LAUNCHER_RESOURCE_PAK_H
#define LAUNCHER_RESOURCE_PAK_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdio>

// 简单的只读资源包格式：
// 文件结构（小端）：
// [char magic[4] = "PAK1"]
// [uint32_t fileCount]
//   重复 fileCount 次：
//     [uint16_t nameLen]
//     [nameLen 字节的 UTF-8 文件名，不含 '\0']
//     [uint64_t offset]  // 相对于整个 pak 文件起始
//     [uint64_t size]
//
// 之后紧跟各个文件的数据块（顺序任意，由 offset/size 决定）。
//
// 约定：
// - 文件名使用类似 "bg.jpg"、"fonts/default.ttf" 这类逻辑名；
// - 本实现为只读，不支持在运行时修改 pak。

class ResourcePak {
public:
    ResourcePak() = default;
    ~ResourcePak();

    ResourcePak(const ResourcePak &) = delete;
    ResourcePak &operator=(const ResourcePak &) = delete;

    // 打开指定的 pak 文件。成功返回 true。
    bool open(const std::string &pakPath);
    void close();

    bool isOpen() const { return file_ != nullptr; }

    // 判断资源名是否存在于索引中。
    bool hasEntry(const std::string &name) const;

    // 将指定资源完整读入内存。
    // 返回值：成功 true；如果找不到资源或读取失败则返回 false。
    bool readAll(const std::string &name, std::vector<std::uint8_t> &outData) const;

    // 列出所有资源名，主要用于调试。
    std::vector<std::string> listEntries() const;

private:
    struct Entry {
        std::uint64_t offset = 0;
        std::uint64_t size = 0;
    };

    bool loadIndex();

    std::string pakPath_;
    mutable std::FILE *file_ = nullptr;
    std::unordered_map<std::string, Entry> entries_;
};

#endif // LAUNCHER_RESOURCE_PAK_H

