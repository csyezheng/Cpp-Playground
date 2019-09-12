# posix api

Each function manually restarts the read or write function if it is interrupted by the return from an application signal handler. **To be as portable as possible, we allow for interrupted system calls and restart them when necessary.**


### Open Close
**`<unistd.h>`**
```
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
```
```
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int open(char *filename, int flags, mode_t mode)
int close (int fd)
```
flags in `<fcntl.h>`: `O_RDONLY`, `O_WRONLY`, `O_RDWR`, `O_CREAT`, `O_TRUNC`, `O_APPEND`

mode in `<sys/stat.h>`: `S_IRUSR`, `S_IWUSR`, `S_IXUSR`

### read write
```
#include <unistd.h>

ssize_t read (int fd, void *buf, size_t nbytes);
ssize_t write (int fd, const void *buf, size_t n);
```

### file attributes
```
#include <unistd.h>
#include <sys/stat.h>

int stat (const char *filename, struct stat *buf);
int fstat (int fd, struct stat *buf);
```

```
int ::dup (int fd)
```
```
int ::dup2 (int fd, int fd2)
```
`<sys/types.h>`
```
ssize_t
```
`<sys/file.h>`
```
int flock(int fd, int operation);
```
`<fcntl.h>`
```
LOCK_SH (Macros)  /* Shared lock. */
LOCK_EX (Macros)  /* Exclusive lock. */
LOCK_NB (Macros)  /* Or'd with one of the above to prevent blocking. */
LOCK_UN (Macros)  /* Remove lock. */
```

# C

`<cstdio>` `<stdio.h>`
```
std::FILE* tmpf = std::tmpfile();
```
```
int fclose( std::FILE* stream );
```

`<cerrno>`
```
EINTR (Macros) 
```

`#include <cassert>`
```
assert(2+2==4);
```

`cstring`
```
char source[] = "once upon a midnight dreary...", dest[4];
std::memcpy(dest, source, sizeof dest);
```


# STL
## `<system_error>`
```
std::generic_category()
```
```
std::system_error(int err, const std::error_category(), const std::string& __what)
```
## `<exception>`
```
void terminate();
```
## `<utility>`
```
template<class T>
void wrapper(T&& arg) 
{
    // arg is always lvalue
    foo(std::forward<T>(arg)); // Forward as lvalue or as rvalue, depending on T
}
```

`#include <limits>`
```
size_t num_bytes = std::numeric_limits<size_t>::max()
```

## Scope Guard

* To ensure that resources are always released in face of an exception but not while returning normally
* To provide basic exception safety guarantee

Resource Acquisition is Initialization (RAII) idiom allows us to acquire resources in the constructor and release them in the destructor when scope ends successfully or due to an exception. It will always release resources. This is not very flexible. Sometime we don't want to release resources if no exception is thrown but we do want to release them if an exception is thrown.

# errno

* `<sys/file.h>`
** flock

* <cerrno>
** errno
** EINTR

## glog

### usage
```
#include <glog/logging.h>

int main(int argc, char* argv[]) {
    // Initialize Google's logging library.
    google::InitGoogleLogging(argv[0]);
    
    // ...
    LOG(INFO) << "Found " << num_cookies << " cookies";
}
```

### CHECK Macros
```
CHECK_NE(1, 2) << ": The is the error message appended!";
DCHECK_NE   // debug-only checking
```


## gtest
### Writing the main() Function
Write your own main() function, which should return the value of `RUN_ALL_TESTS()`
### Simple Tests
```
TEST(TestSuiteName, TestName) {
  ... test body ...
}
```
### Test Fixtures: Using the Same Data Configuration for Multiple Tests 
```
TEST_F(TestFixtureName, TestName) {
  ... test body ...
}
```
### Reference
https://github.com/google/googletest/blob/master/googletest/docs/primer.md

## gmock

#### String Matchers
`HasSubstr(string)`


### Reference
https://github.com/google/googletest/blob/master/googlemock/docs/cheat_sheet.md

## CMake
