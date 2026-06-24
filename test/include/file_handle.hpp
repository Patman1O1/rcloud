#ifndef RCLOUD_TEST_FILE_HANDLE_HPP
#define RCLOUD_TEST_FILE_HANDLE_HPP

// ISO C++ Includes
#include <string>
#include <string_view>

// POSIX C Includes
#include <unistd.h>

namespace rcloud_testing {

    class file_handle {
    private:
        // ── Fields ───────────────────────────────────────────────────────────────────────────────────────────────────
        std::string path_;

    public:
        // ── Constructors ─────────────────────────────────────────────────────────────────────────────────────────────
        file_handle() noexcept = delete;

        inline explicit file_handle(std::string_view);

        file_handle(const file_handle&) = delete;

        inline file_handle(file_handle&& other) noexcept;

        // ── Destructor ───────────────────────────────────────────────────────────────────────────────────────────────
        ~file_handle() noexcept;

        // ── Overloaded Operators ─────────────────────────────────────────────────────────────────────────────────────
        file_handle& operator=(const file_handle&) = delete;

        inline file_handle& operator=(file_handle&& other) noexcept;

        [[nodiscard]] explicit inline operator const char*() const noexcept;
    };

    // ── Constructors ─────────────────────────────────────────────────────────────────────────────────────────────────
    inline file_handle::file_handle(file_handle&& other) noexcept : path_(std::move(other.path_)) {}

    inline file_handle::file_handle(const std::string_view path) : path_(path) {}

    // ── Destructor ───────────────────────────────────────────────────────────────────────────────────────────────────
    inline file_handle::~file_handle() noexcept {
        if (this->path_.empty()) {
            return;
        }
        ::unlink(this->path_.c_str());
    }

    // ── Overloaded Operators ─────────────────────────────────────────────────────────────────────────────────────────
    [[nodiscard]] inline file_handle::operator const char*() const noexcept { return this->path_.c_str(); }

    inline file_handle& file_handle::operator=(file_handle&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        if (!this->path_.empty()) {
            ::unlink(this->path_.c_str());
        }
        this->path_ = std::move(other.path_);

        return *this;
    }

} // namespace rcloud_testing

#endif // #ifndef RCLOUD_TEST_FILE_HANDLE_HPP