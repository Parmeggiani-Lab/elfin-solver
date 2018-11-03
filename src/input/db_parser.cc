#include "db_parser.h"

#include "json.h"
#include "jutil.h"

namespace elfin {

void DBParser::parse(
    const JSON & j,
    NameIdMap & name_map_out,
    IdNameMap & inm_out,
    RelationshipMatrix & rel_mat_out,
    RadiiList & radii_list_out)
{
    rel_mat_out.clear();
    JSON double_data = j["double_data"];
    const size_t dim = double_data.size();
    rel_mat_out.resize(dim);

    // First we need the name-to-ID map
    unsigned int id = 0;
    for (JSON::iterator it = double_data.begin();
            it != double_data.end();
            ++it)
    {
        // JSON::iterator does not support offsetting,
        // but we need the key() which is only available
        // from JSON::iterator
        name_map_out[it.key()] = id;
        inm_out[id] = it.key();

        RelaRow & row = rel_mat_out.at(id);
        row.resize(dim, NULL);

        dbg("Name-ID Pair: %s<->%d, neighbours: %d\n",
            it.key().c_str(), id, (*it).size());
        id++;
    }

    // With the mame-to-ID map we can construct the
    // relationship matrix without using string names
    //
    // This is thinking ahead for the data to be operated
    // on by a more C-like kernel. In that case strings
    // strings are definitely bad for performance
    for (JSON::iterator it = double_data.begin();
            it != double_data.end();
            ++it) {
        id = name_map_out[it.key()];
        RelaRow & row = rel_mat_out.at(id);

        // Add neighbouring nodes
        for (JSON::iterator innerIt = (*it).begin();
                innerIt != (*it).end();
                ++innerIt) {
            const unsigned int innerId = name_map_out[innerIt.key()];
            prf("innerIt key: %s, id: %u, data: %s\n\n",
                innerIt.key().c_str(),
                innerId,
                innerIt->dump().c_str());

            panic_if(
                row.at(innerId) != NULL,
                "Initial PairRelationship must be NULL - implementation error?\n");

            std::vector<float> combBv;
            for (JSON::iterator comBf = (*innerIt)["com_b"].begin();
                    comBf != (*innerIt)["com_b"].end();
                    ++comBf) {
                combBv.push_back((*comBf).get<float>());
            }
            std::vector<float> rotv;
            for (JSON::iterator rotRow = (*innerIt)["rot"].begin();
                    rotRow != (*innerIt)["rot"].end();
                    ++rotRow) {
                for (JSON::iterator rotF = (*rotRow).begin();
                        rotF != (*rotRow).end();
                        ++rotF) {
                    rotv.push_back((*rotF).get<float>());
                }
            }
            std::vector<float> tranv;
            for (JSON::iterator tranRow = (*innerIt)["tran"].begin();
                    tranRow != (*innerIt)["tran"].end();
                    ++tranRow) {
                for (JSON::iterator tranF = (*tranRow).begin();
                        tranF != (*tranRow).end();
                        ++tranF) {
                    tranv.push_back((*tranF).get<float>());
                }
            }
            row.at(innerId) = new PairRelationship(
                combBv, rotv, tranv);
        }

        id++;
    }

    // Verify parsed pair relationship
    for (int i = 0; i < dim; i++)
    {
        for (int j = 0; j < dim; j++)
        {
            PairRelationship * pr = rel_mat_out.at(i).at(j);
            if (pr != NULL)
            {
                prf("rel_mat_out[%2d][%2d] is:\n%s\n",
                    i, j, pr->to_string().c_str());
            }
        }
    }

    // Parse radii for collision checking
    JSON single_data = j["single_data"];
    for (JSON::iterator it = single_data.begin();
            it != single_data.end();
            ++it)
    {
        JSON & radii = (*it)["radii"];
        radii_list_out.emplace_back(radii["average_all"],
                                  radii["max_ca_dist"],
                                  radii["max_heavy_dist"]);
    }
}

}  /* elfin */