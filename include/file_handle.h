#ifndef VIRTOO_FILEHANDLE_H
#define VIRTOO_FILEHANDLE_H

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

class FileHandle {
 public:
    FileHandle(int fd) : fd(fd) {}

    FileHandle(const char *name, const int flags) {
        this->fd = open(name, flags);
    }

    FileHandle(FileHandle &&other) {
        this->fd = other.fd;
        other.MarkInvalid();
    }

    ~FileHandle() {
        if (this->fd > 0) {
            close(this->fd);
        }
    }

    FileHandle(const FileHandle &) = delete;
    FileHandle &operator=(const FileHandle &) = delete;
    FileHandle &operator=(FileHandle &&) = delete;

    int GetRaw() const {
        return fd;
    }

    bool IsValid() const {
        return fd >= 0;
    }

    int ioctl(const unsigned long request, void *arg) const {
        return ::ioctl(this->fd, request, arg);
    }

 private:
    int fd;

    void MarkInvalid() {
        this->fd = -1;
    }
};


#endif
