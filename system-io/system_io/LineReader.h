#ifndef SYSTEM_IO_LINEREADER_H
#define SYSTEM_IO_LINEREADER_H

#include <cstddef>
#include <string>

namespace sysio {
    /*
     * Async-signal-safe line reader.
     */
    class LineReader {
    public:
        /*
         * Create a line reader that reads into a user-provided buffer (of size
         * bufSize).
         */
        LineReader(int fd, char* buf, size_t bufSize);

        LineReader(const LineReader&) = delete;
        LineReader& operator=(const LineReader&) = delete;

        enum State {
            kReading,
            kEof,
            kError,
        };
        /**
         * Read the next line from the file.
         *
         * If the line is at most bufSize characters long, including the trailing
         * newline, it will be returned (including the trailing newline).
         *
         * If the line is longer than bufSize, we return the first bufSize bytes
         * (which won't include a trailing newline) and then continue from that
         * point onwards.
         *
         * The lines returned are not null-terminated.
         *
         * Returns kReading with a valid line, kEof if at end of file, or kError
         * if a read error was encountered.
         *
         * Example:
         *   bufSize = 10
         *   input has "hello world\n"
         *   The first call returns "hello worl"
         *   The second call returns "d\n"
         */
        State readLine(std::string& line);

    private:
        int const fd_;
        char* const buf_;
        char* const bufEnd_;

        // buf_ <= bol_ <= eol_ <= end_ <= bufEnd_
        //
        // [buf_, end_): current buffer contents (read from file)
        //
        // [buf_, bol_): free (already processed, can be discarded)
        // [bol_, eol_): current line, including \n if it exists, eol_ points
        //               1 character past the \n
        // [eol_, end_): read, unprocessed
        // [end_, bufEnd_): free

        char* bol_;
        char* eol_;
        char* end_;
        State state_;
    };
}
#endif //SYSTEM_IO_LINEREADER_H
