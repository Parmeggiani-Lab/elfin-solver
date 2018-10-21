#include <sstream>

#include "Gene.h"
#include "jutil.h"

namespace elfin
{

// Static vars
bool Gene::setupDone = false;
const IdNameMap * Gene::inm = NULL;

// Constructors
Gene::Gene(const uint _nodeId) :
    myNodeId(_nodeId),
    myCom(Point3f(0, 0, 0))
{
    panic_if(!setupDone,
             "Gene::setup() must be callsed first!\n");
}

Gene::Gene(const uint _nodeId,
           const Point3f _com) :
    myNodeId(_nodeId),
    myCom(_com)
{
    panic_if(!setupDone,
             "Gene::setup() must be callsed first!\n");
}

Gene::Gene(const uint _nodeId,
           const float x,
           const float y,
           const float z) :
    myNodeId(_nodeId),
    myCom(x, y, z)
{
    panic_if(!setupDone,
             "Gene::setup() must be callsed first!\n");
}

uint &
Gene::nodeId()
{
    return myNodeId;
}

const uint &
Gene::nodeId() const
{
    return myNodeId;
}

Point3f &
Gene::com()
{
    return myCom;
}

const Point3f &
Gene::com() const
{
    return myCom;
}

std::string
Gene::to_string() const
{
    std::stringstream ss;
    ss << "ID: " << myNodeId << " ";
    ss << "Name: " << inm->at(myNodeId) << " ";
    ss << "CoM: " << myCom.to_string();

    return ss.str();
}

std::string
Gene::to_csv_string() const
{
    return myCom.to_csv_string();
}

void
Gene::setup(const IdNameMap * _inm)
{
    inm = _inm;
    setupDone = true;
}

std::string
genes_to_string(const Genes & genes)
{
    std::stringstream ss;

    const int N = genes.size();
    for (int i = 0; i < N; i++)
    {
        ss << "Node #" << i << " / " << N << ": "
           << genes.at(i).to_string() << std::endl;
    }

    return ss.str();
}

std::string
genes_to_csv_string(const Genes & genes)
{
    std::stringstream ss;

    const int N = genes.size();
    for (int i = 0; i < N; i++)
        ss << genes.at(i).to_csv_string() << std::endl;

    return ss.str();
}

} // namespace elfin