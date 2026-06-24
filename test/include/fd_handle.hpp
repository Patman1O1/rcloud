#ifndef RCLOUD_TEST_FD_HANDLE_HPP
#define RCLOUD_TEST_FD_HANDLE_HPP

// ISO C++ Includes
#include <concepts>
#include <system_error>

// POSIX C Includes
#include <unistd.h>

namespace rcloud_testing {
    class fd_handle {
    private:
        // ── Fields ───────────────────────────────────────────────────────────────────────────────────────────────────
        int fd_ = -1;

    public:
        // ── Constructors ─────────────────────────────────────────────────────────────────────────────────────────────
        fd_handle() noexcept = delete;

        template<std::signed_integral T>
        inline explicit fd_handle(T fd) /* throws std::system_error */;

        fd_handle(const fd_handle&) = delete;

        inline fd_handle(fd_handle&& other) noexcept;

        // ── Destructor ───────────────────────────────────────────────────────────────────────────────────────────────
        inline ~fd_handle() noexcept;

        // ── Overloaded Operators ─────────────────────────────────────────────────────────────────────────────────────
        fd_handle& operator=(const fd_handle&) noexcept = delete;

        inline fd_handle& operator=(fd_handle&& other) noexcept;

        [[nodiscard]] inline explicit operator int() const noexcept;
    };

    // ── Constructors ─────────────────────────────────────────────────────────────────────────────────────────────────
    template<std::signed_integral T>
    inline fd_handle::fd_handle(const T fd) /* throws std::system_error */ {
        if (fd < 0) {
            throw std::system_error(std::make_error_code(std::errc::bad_file_descriptor));
        }
        this->fd_ = static_cast<int>(fd);
    }

    inline fd_handle::fd_handle(fd_handle&& other) noexcept : fd_(other.fd_) { other.fd_ = -1; }

    // ── Destructor ───────────────────────────────────────────────────────────────────────────────────────────────────
    inline fd_handle::~fd_handle() noexcept {
        if (this->fd_ < 0) {
            return;
        }

        ::close(this->fd_);
        this->fd_ = -1;
    }

    // ── Overloaded Operators ─────────────────────────────────────────────────────────────────────────────────────────
    inline fd_handle& fd_handle::operator=(fd_handle&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        if (this->fd_ >= 0) {
            ::close(this->fd_);
        }

        this->fd_ = other.fd_;
        other.fd_ = -1;

        return *this;
    }

    [[nodiscard]] inline fd_handle::operator int() const noexcept { return this->fd_; }

} // namespace rcloud_testing

#endif // #ifndef RCLOUD_TEST_FD_HANDLE_HPP