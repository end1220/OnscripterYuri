#include "resource_pak.h"

#include <cerrno>
#include <cstring>

namespace {

static constexpr const char PAK_MAGIC[4] = {'P', 'A', 'K', '1'};

// 从文件中读取一个 POD 类型，返回是否成功。
template <typename T>
bool fread_pod(std::FILE *f, T &out) {
    return std::fread(&out, sizeof(T), 1, f) == 1;
}

bool fread_bytes(std::FILE *f, void *buf, std::size_t len) {
    return std::fread(buf, 1, len, f) == len;
}

} // namespace

ResourcePak::~ResourcePak() {
    close();
}

void ResourcePak::close() {
    if (file_) {
        std::fclose(file_);
        file_ = nullptr;
    }
    entries_.clear();
    pakPath_.clear();
}

bool ResourcePak::open(const std::string &pakPath) {
    close();

    file_ = std::fopen(pakPath.c_str(), "rb");
    if (!file_) {
        return false;
    }
    pakPath_ = pakPath;

    if (!loadIndex()) {
        close();
        return false;
    }

    return true;
}

bool ResourcePak::loadIndex() {
    // 读取 magic
    char magic[4] = {0, 0, 0, 0};
    if (!fread_bytes(file_, magic, sizeof(magic))) {
        return false;
    }
    if (std::memcmp(magic, PAK_MAGIC, sizeof(magic)) != 0) {
        return false;
    }

    // 读取文件数量
    std::uint32_t count = 0;
    if (!fread_pod(file_, count)) {
        return false;
    }

    entries_.clear();
    entries_.reserve(count);

    for (std::uint32_t i = 0; i < count; ++i) {
        std::uint16_t nameLen = 0;
        if (!fread_pod(file_, nameLen)) {
            return false;
        }
        if (nameLen == 0) {
            return false;
        }

        std::string name;
        name.resize(nameLen);
        if (!fread_bytes(file_, &name[0], nameLen)) {
            return false;
        }

        Entry e;
        if (!fread_pod(file_, e.offset)) {
            return false;
        }
        if (!fread_pod(file_, e.size)) {
            return false;
        }

        entries_[name] = e;
    }

    return true;
}

bool ResourcePak::hasEntry(const std::string &name) const {
    return entries_.find(name) != entries_.end();
}

bool ResourcePak::readAll(const std::string &name, std::vector<std::uint8_t> &outData) const {
    if (!file_) {
        return false;
    }
    auto it = entries_.find(name);
    if (it == entries_.end()) {
        return false;
    }

    const Entry &e = it->second;
    if (e.size == 0) {
        outData.clear();
        return true;
    }

    if (std::fseek(file_, static_cast<long>(e.offset), SEEK_SET) != 0) {
        return false;
    }

    outData.resize(static_cast<std::size_t>(e.size));
    if (!fread_bytes(file_, outData.data(), outData.size())) {
        outData.clear();
        return false;
    }

    return true;
}

std::vector<std::string> ResourcePak::listEntries() const {
    std::vector<std::string> names;
    names.reserve(entries_.size());
    for (const auto &kv : entries_) {
        names.push_back(kv.first);
    }
    return names;
}

