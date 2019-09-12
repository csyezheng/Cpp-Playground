#include <glog/logging.h>
#include <gtest/gtest.h>
#include <gflags/gflags.h>



int main(int argc, char** argv) {
    FLAGS_logtostderr = true;
    google::InitGoogleLogging(argv[0]);
    ::testing::InitGoogleTest(&argc, argv);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    return RUN_ALL_TESTS();
}