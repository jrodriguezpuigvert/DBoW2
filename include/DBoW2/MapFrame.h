//
// Created by Javier Rodriguez Puigvert on 17.01.18.
//

#include <string>
#include <vector>
#include <opencv2/core.hpp>

using namespace cv;

using namespace std;

namespace LS {
class MapFrame{
    public:
        std::string imageName;
        vector<KeyPoint> keyPoints;
    MapFrame(void);
    MapFrame(const MapFrame& mapFrame);
    MapFrame(std::string imageName, vector<KeyPoint> keyPoints);
    ~MapFrame(void);
};

}
