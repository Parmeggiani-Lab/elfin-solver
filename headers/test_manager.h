#ifndef TEST_MANAGER_H_
#define TEST_MANAGER_H_

#include <cstddef>

namespace elfin {

class TestManager {
private:
    /* accessors */
    size_t test_units() const;
    size_t test_integration() const;
public:
    /* accessors */
    void run() const;
};  /* class TestManager*/

}  /* elfin */

#endif  /* end of include guard: TEST_MANAGER_H_ */