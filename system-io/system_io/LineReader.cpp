#include "LineReader.h"
#include <cstring>
#include "FileUtil.h"

namespace sysio
{
    LineReader::LineReader(int fd, char *buf, size_t bufSize)
            : fd_(fd),
              buf_(buf),
              bufEnd_(buf_ + bufSize),
              bol_(buf),
              eol_(buf),
              end_(buf),
              state_(kReading)
    {}

    LineReader::State LineReader::readLine(std::string &line)
    {
        bol_ = eol_; // Start past what we already returned
        for (;;)
        {
            // Search for newline
            char *newline = static_cast<char *>(memchr(eol_, '\n', end_ - eol_));
            if (newline)
            {
                eol_ = newline + 1;
                break;
            } else if (state_ != kReading || (bol_ == buf_ && end_ == bufEnd_))
            {
                // If the buffer is full with one line (line too long), or we're
                // at the end of the file, return what we have.
                eol_ = end_;
                break;
            }

            // We don't have a full line in the buffer, but we have room to read.
            // Move to the beginning of the buffer.
            memmove(buf_, eol_, end_ - eol_);
            end_ -= (eol_ - buf_);
            bol_ = buf_;
            eol_ = end_;

            // Refill
            ssize_t available = bufEnd_ - end_;
            ssize_t n = sysio::readFull(fd_, end_, available);
            if (n < 0)
            {
                state_ = kError;
                n = 0;
            } else if (n < available)
            {
                state_ = kEof;
            }
            end_ += n;
        }

        line.assign(bol_, eol_);
        return eol_ != bol_ ? kReading : state_;
    }
}


