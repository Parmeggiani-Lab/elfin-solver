#ifndef NODE_PATH_H
#define NODE_PATH_H

##include <memory>

namespace elfin {

/* fwd dcl */
class NodePath;
typedef std::unique_ptr<NodePath> NodePathUP;

class NodePath {
public:
	/* accessors */
	NodePathUP clone() const;
	NodeSPVMap const& nodes() const { return nodes_; }
	FreeChainList const& free_chains() const { return free_chains_; }
	size_t size() const { return nodes_.size(); }
	float score() const { return score_; }
	Crc32 checksum() const { return checksum_; }
	static bool ScoreCompareSP(
	    NodePathSP const& lhs,
	    NodePathSP const& rhs) {
		return lhs->score_ < rhs->score_;
	}

	/* modifiers */
	NodePath& operator=(NodePath const& other);
	NodePath& operator=(NodePath && other);

	virtual mutation::Mode mutate_and_score(
	    NodePath const& mother,
	    NodePath const& father) = 0;
	virtual void randomize() = 0;

	/* printers */
	virtual std::string to_string() const = 0;
	virtual JSON gen_nodes_json() const = 0;

	/* tests */
	// static BasicNodePath build_team(StepList const& steps);
	// static TestStat test();
};

}  /* elfin */

#endif  /* end of include guard: NODE_PATH_H */