#ifndef SYSTEM_IO_FILE_H
#define SYSTEM_IO_FILE_H


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string>
#include <system_error>

namespace sysio
{
    /*
     * A File represents an open file.
     */
    class File
    {
    public:
        /*
         * Creates an empty File object, for late initialization.
         */
        File() noexcept;

        /*
         * Create a File object from an existing file descriptor.
         * Takes ownership of the file descriptor if ownsFd is true.
         */
        explicit File(int fd, bool ownsFd = false) noexcept;

        /*
         * Open and create a file object.  Throws on error.
         * Owns the file descriptor implicitly.
         */
        explicit File(const char *name, int flags = O_RDONLY, mode_t mode = 0666);

        explicit File(
                const std::string &name,
                int flags = O_RDONLY,
                mode_t mode = 0666);

        // unique
        File(const File &) = delete;

        File &operator=(const File &) = delete;

        // movable
        File(File &&) noexcept;

        File &operator=(File &&) noexcept;

        ~File();

        /*
         * Create and return a temporary, owned file (uses tmpfile()).
         */
        static File temporary();

        /*
         * Return the file descriptor, or -1 if the file was closed.
         */
        int fd() const;

        /*
         * Returns 'true' if the file was successfully opened.
         */
        //
        explicit operator bool() const;

        /*
         * Duplicate file descriptor and return File that owns it.
         */
        File dup() const;

        /*
         * If we own the file descriptor, close the file and throw on error.
         * Otherwise, do nothing.
         */
        void close();

        /*
         * Closes the file (if owned).  Returns true on success, false (and sets
         * errno) on error.
         */
        bool closeNoThrow();

        /*
         * Returns and releases the file descriptor; no longer owned by this File.
         * Returns -1 if the File object didn't wrap a file.
         */
        int release() noexcept;

        /*
         * Swap this File with another.
         */
        void swap(File &other) noexcept;

        /*
         * FLOCK (INTERPROCESS) LOCKS
         *
         * NOTE THAT THESE LOCKS ARE flock() LOCKS.
         * That is, they may only be used for inter-process synchronization.
         *  an attempt to acquire a second lock on the same file descriptor from the same process may succeed.
         *  Attempting to acquire a second lock on a different file descriptor for the same file should fail,
         *  but some systems might implement flock() using fcntl() locks, in which case it will succeed.
         */
        void lock();

        bool try_lock();

        void unlock();

        void lock_shared();

        bool try_lock_shared();

        void unlock_shared();

    private:
        void doLock(int op);

        bool doTryLock(int op);

        int fd_;
        bool ownsFd_;
    };

    void swap(File &a, File &b) noexcept;
}

#endif //SYSTEM_IO_FILE_H
