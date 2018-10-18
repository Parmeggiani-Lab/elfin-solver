#include "work_area.h"

#include "../../jutil/src/jutil.h"

namespace elfin {

std::shared_ptr<WorkArea> WorkArea::from_json(const JSON & pgn, const JSON & networks) {
    std::shared_ptr<WorkArea> wa = std::make_shared<WorkArea>();

    for(auto it = pgn.begin(); it != pgn.end(); ++it) {
        std::shared_ptr<Point3f> p = Point3f::from_json((*it)["trans"]);
        wa->push_back(*p);
    }

    return wa;
}

}  /* elfin */