#ifndef TEST_STAT_H_
#define TEST_STAT_H_

namespace elfin {

struct TestStat {
    size_t tests = 0;
    size_t errors = 0;
    TestStat& operator+=(TestStat const& rhs) {
        tests += rhs.tests;
        errors += rhs.errors;
        return *this;
    }
};

}  /* elfin */

#endif  /* end of include guard: TEST_STAT_H_ */