#include "proto_tests.h"

#include "test_stat.h"
#include "input_manager.h"

namespace elfin {

namespace proto {

TestStat test() {
    TestStat ts;

    auto test_ptterm_profile =
        [&ts](std::string const & src_mod_name,
              std::string const & src_chain_name,
              std::string const & src_term_name,
              std::string const & dst_mod_name,
              std::string const & dst_chain_name,
              std::string const & dst_term_name,
              bool const exp_reacheable
    ) {
        ts.tests++;

        auto const src_mod = XDB.get_mod(src_mod_name);
        size_t const src_chain_id = src_mod->get_chain_id(src_chain_name);
        auto const src_term = parse_term(src_term_name);
        FreeTerms const src_terms = { FreeTerm(nullptr, src_chain_id, src_term) };

        auto const dst_mod = XDB.get_mod(dst_mod_name);
        size_t const dst_chain_id = dst_mod->get_chain_id(dst_chain_name);
        auto const dst_term = parse_term(dst_term_name);
        FreeTerms const dst_terms = {FreeTerm(nullptr, dst_chain_id, dst_term) };


        auto const& profile = src_mod->get_reachable_ptterms(src_terms);

        bool const reachable = any_of(begin(dst_terms), end(dst_terms),
        [&profile, dst_mod](auto const & dst_ft) {
            auto const itr = profile.find(
            PtTermFinder{
                nullptr,
                0,
                TermType::NONE,
                const_cast<ProtoTerm*>(&dst_mod->get_term(dst_ft))
            });
            return itr != end(profile);
        });

        if (reachable != exp_reacheable) {
            ts.errors++;

            std::ostringstream oss;
            auto const print_free_terms = [&oss](PtModKey const mod, FreeTerms const & fts) {
                oss << mod->name << " free terms:\n";
                for (auto const ft : fts) {
                    oss << "  " << mod->chains().at(ft.chain_id).name;
                    oss << "(id=" << ft.chain_id << ")";
                    oss << ":" << TermTypeToCStr(ft.term) << "\n";
                }
            };

            print_free_terms(src_mod, src_terms);
            print_free_terms(dst_mod, dst_terms);
            JUtil.error("Expected %spath between modules %s and %s.\n%s",
                        exp_reacheable ? "" : "no ",
                        src_mod->name.c_str(),
                        dst_mod->name.c_str(),
                        oss.str().c_str());
        }
    };

    // Note!
    //
    // The following might change with updated XDB that has a richer set of connections.

    test_ptterm_profile("D8", "A", "C", "D8", "A", "N", true);
    test_ptterm_profile("D8", "A", "C", "D49", "A", "N", false);  // Simply no path.

    test_ptterm_profile("D81", "A", "C", "D81", "A", "N", true);
    test_ptterm_profile("D81", "A", "N", "D81", "A", "C", true);
    test_ptterm_profile("D81", "A", "N", "D81_aC2_05", "B", "C", true);
    test_ptterm_profile("D81_aC2_05", "A", "N", "D81", "A", "C", false);  // Src term non existent.
    test_ptterm_profile("D81_aC2_05", "A", "C", "D81", "A", "N", true);
    test_ptterm_profile("D81_aC2_05", "A", "C", "D81_aC2_05", "A", "N", false);  // Dst term non existent.
    test_ptterm_profile("D81_aC2_05", "A", "C", "D81_aC2_05", "A", "C", false);  // D81 has no N term back link.
    test_ptterm_profile("D81_aC2_05", "B", "C", "D14_j1_D81", "A", "C", false);  // No resuse of same ProtoTerm (D81.A.N).

    test_ptterm_profile("D49", "A", "N", "D49", "A", "C", true);
    test_ptterm_profile("D49", "A", "C", "D49", "A", "C", true);
    test_ptterm_profile("D49", "A", "N", "D81", "A", "C", false);  // No way to reach D81.A.C as an inward ProtoTerm.
    test_ptterm_profile("D49", "A", "N", "D49_aC2_24", "A", "C", true);
    test_ptterm_profile("D49", "A", "N", "D49_aC2_24", "B", "C", true);
    test_ptterm_profile("D49", "A", "N", "D49_aC2_24", "B", "N", false);  // Dst term non existent.
    test_ptterm_profile("D49", "A", "C", "D49_aC2_24", "A", "C", true);
    test_ptterm_profile("D49", "A", "C", "D49_aC2_24", "B", "C", true);
    test_ptterm_profile("D49", "A", "C", "D49_aC2_24", "B", "N", false);  // Dst term non existent.
    test_ptterm_profile("D53", "A", "N", "D64", "A", "C", false);  // No way to reach D64.A.C as an inward ProtoTerm.

    test_ptterm_profile("D49", "A", "C", "D49_aC2_ext", "C", "C", true);
    test_ptterm_profile("D49", "A", "C", "D49_aC2_ext", "D", "N", true);
    test_ptterm_profile("D49", "A", "C", "D71", "A", "N", true);
    test_ptterm_profile("D49", "A", "C", "D71", "A", "C", false);  // D49-...D71-D71

    test_ptterm_profile("D49_aC2_ext", "C", "N", "D49_aC2_ext", "D", "N", true);  // hub-D49-hub-D49-hub
    test_ptterm_profile("D49_aC2_ext", "C", "N", "D49_aC2_ext", "C", "N", true);  // hub-D49-hub
    test_ptterm_profile("D49_aC2_ext", "C", "N", "D79", "A", "N", true);
    test_ptterm_profile("D49_aC2_ext", "C", "N", "D8", "A", "N", false);
    test_ptterm_profile("D49_aC2_ext", "C", "N", "D8", "A", "C", false);

    return ts;
}

}  /* proto */

}  /* elfin */