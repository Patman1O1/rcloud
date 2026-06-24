#ifndef RCLOUD_TEST_DIR_HANDLE_HPP
#define RCLOUD_TEST_DIR_HANDLE_HPP

// ISO C++ Includes
#include <filesystem>
#include <stdexcept>
#include <string_view>
#include <system_error>

// POSIX C Includes
#include <cerrno>
#include <dirent.h>

namespace rcloud_testing {
    class dir_handle {
    private:
        // ── Fields ──────────────────────────────────────────────────────────────────────────────────────────────────
        std::filesystem::path path_;
        ::DIR*                dir_ = nullptr;

    public:
        // ── Constructors ─────────────────────────────────────────────────────────────────────────────────────────────
        dir_handle() noexcept = delete;

        inline explicit dir_handle(std::string_view path) /* throws std::runtime_error, std::system_error */;

        dir_handle(const dir_handle&) = delete;

        inline dir_handle(dir_handle&& other) noexcept;

        // ── Destructor ───────────────────────────────────────────────────────────────────────────────────────────────
        inline ~dir_handle() noexcept;

        // ── Overloaded Operators ─────────────────────────────────────────────────────────────────────────────────────
        dir_handle& operator=(const dir_handle&) = delete;

        inline dir_handle& operator=(dir_handle&& other) noexcept;

        [[nodiscard]] inline explicit operator ::DIR*() const noexcept;

        [[nodiscard]] inline explicit operator const char*() const noexcept;

        [[nodiscard]] inline explicit operator std::filesystem::path() const noexcept;

        // ── Accessors ────────────────────────────────────────────────────────────────────────────────────────────────
        [[nodiscard]] inline ::DIR* get_dir() const noexcept;
    };

    // ── Constructors ─────────────────────────────────────────────────────────────────────────────────────────────────
    inline dir_handle::dir_handle(const std::string_view path) /* throws std::runtime_error, std::system_error */ {
        if (path.empty()) {
            throw std::runtime_error("\"path\" is empty");
        }
        this->path_ = path;
        this->dir_ = ::opendir(this->path_.c_str());
        if (this->dir_ == nullptr) {
            throw std::system_error(std::error_code(errno, std::system_category()), "opendir failed");
        }
    }

    inline dir_handle::dir_handle(dir_handle&& other) noexcept
        : path_(std::move(other.path_)), dir_(other.dir_) {
        other.dir_ = nullptr;
    }

    // ── Destructor ───────────────────────────────────────────────────────────────────────────────────────────────────
    inline dir_handle::~dir_handle() noexcept {
        if (this->dir_ == nullptr) {
            return;
        }
        ::closedir(this->dir_);
        this->dir_ = nullptr;
    }

    // ── Overloaded Operators ─────────────────────────────────────────────────────────────────────────────────────────
    inline dir_handle& dir_handle::operator=(dir_handle&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        if (this->dir_ != nullptr) {
            ::closedir(this->dir_);
        }

        this->path_ = std::move(other.path_);
        this->dir_  = other.dir_;
        other.dir_  = nullptr;

        return *this;
    }

    [[nodiscard]] inline dir_handle::operator ::DIR*() const noexcept { return this->dir_; }

    [[nodiscard]] inline dir_handle::operator const char*() const noexcept { return this->path_.c_str(); }

    [[nodiscard]] inline dir_handle::operator std::filesystem::path() const noexcept { return this->path_; }

    // ── Accessors ────────────────────────────────────────────────────────────────────────────────────────────────────
    [[nodiscard]] inline ::DIR* dir_handle::get_dir() const noexcept { return this->dir_; }

} // namespace rcloud_testing

#endif // #ifndef RCLOUD_TEST_DIR_HANDLE_HPP