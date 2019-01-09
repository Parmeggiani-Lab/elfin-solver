#include "proto_tests.h"

#include "test_stat.h"
#include "input_manager.h"

namespace elfin {

namespace proto {

TestStat test() {
    TestStat ts;

    auto test_find_path = [&](std::string const & src_mod_name,
                              std::string const & src_chain_name,
                              std::string const & src_term_name,
                              std::string const & dst_mod_name,
                              std::string const & dst_chain_name,
                              std::string const & dst_term_name,
    size_t const exp_n_paths) {
        ts.tests++;

        auto const src_mod = XDB.get_mod(src_mod_name);
        size_t const src_chain_id = src_mod->get_chain_id(src_chain_name);
        auto const src_term = parse_term(src_term_name);
        FreeTerms const src_terms = { FreeTerm(nullptr, src_chain_id, src_term) };

        auto const dst_mod = XDB.get_mod(dst_mod_name);
        size_t const dst_chain_id = dst_mod->get_chain_id(dst_chain_name);
        auto const dst_term = parse_term(dst_term_name);
        FreeTerms const dst_terms = {FreeTerm(nullptr, dst_chain_id, dst_term) };

        PtPaths const& paths = src_mod->find_paths(src_terms, dst_mod, dst_terms);

        if (paths.size() != exp_n_paths) {
            JUtil.error("Expected %zu paths between %s.%s.%s and %s.%s.%s but got %zu\n",
                        exp_n_paths,
                        src_mod_name.c_str(), src_chain_name.c_str(), src_term_name.c_str(),
                        dst_mod_name.c_str(), dst_chain_name.c_str(), dst_term_name.c_str(),
                        paths.size());
            std::ostringstream oss;
            oss << "Proto-Paths:\n";
            for (auto const& path : paths) {
                oss << path << "\n\n";
            }

            JUtil.error(oss.str().c_str());
            ts.errors++;
        }
    };

    test_find_path("D8", "A", "C", "D8", "A", "N", 1);
    test_find_path("D81", "A", "C", "D81", "A", "N", 1);
    test_find_path("D81", "A", "N", "D81", "A", "C", 1);
    test_find_path("D49", "A", "N", "D49", "A", "C", 5);
    test_find_path("D49", "A", "C", "D49_aC2_ext", "C", "C", 0);
    test_find_path("D49", "A", "C", "D49_aC2_ext", "D", "N", 1);

    return ts;
}

}  /* proto */

}  /* elfin */