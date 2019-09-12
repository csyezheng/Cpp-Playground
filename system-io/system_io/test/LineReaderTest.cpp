#include "system_io/LineReader.h"

#include <string>
#include <glog/logging.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "system_io/File.h"
#include "system_io/FileUtil.h"


using namespace sysio;

namespace sysio
{
    namespace test
    {
        void writeAll(int fd, const char* str) {
            ssize_t n = strlen(str);
            CHECK_EQ(n, writeFull(fd, str, n));
        }

        void expect(LineReader& lr, const char* expected) {
            std::string line;
            size_t expectedLen = strlen(expected);
            EXPECT_EQ(
                    expectedLen != 0 ? LineReader::kReading : LineReader::kEof,
                    lr.readLine(line));
            EXPECT_EQ(expectedLen, line.size());
            EXPECT_EQ(std::string(expected, expectedLen), line);
        }

        TEST(LineReader, Simple) {
            File tmp = File::temporary();
            int fd = tmp.fd();
            writeAll(
                    fd,
                    "Meow\n"
                    "Hello world\n"
                    "This is a long line. It is longer than the other lines.\n"
                    "\n"
                    "Incomplete last line");

            {
                CHECK_ERR(lseek(fd, 0, SEEK_SET));
                char buf[10];
                LineReader lr(fd, buf, sizeof(buf));
                expect(lr, "Meow\n");
                expect(lr, "Hello worl");
                expect(lr, "d\n");
                expect(lr, "This is a ");
                expect(lr, "long line.");
                expect(lr, " It is lon");
                expect(lr, "ger than t");
                expect(lr, "he other l");
                expect(lr, "ines.\n");
                expect(lr, "\n");
                expect(lr, "Incomplete");
                expect(lr, " last line");
                expect(lr, "");
            }

            {
                CHECK_ERR(lseek(fd, 0, SEEK_SET));
                char buf[80];
                LineReader lr(fd, buf, sizeof(buf));
                expect(lr, "Meow\n");
                expect(lr, "Hello world\n");
                expect(lr, "This is a long line. It is longer than the other lines.\n");
                expect(lr, "\n");
                expect(lr, "Incomplete last line");
                expect(lr, "");
            }
        }
    }
}


