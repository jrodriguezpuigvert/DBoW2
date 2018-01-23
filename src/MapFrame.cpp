//
// Created by Javier Rodriguez Puigvert on 17.01.18.
//

#include "MapFrame.h"

namespace LS{
    MapFrame::MapFrame() {
    }
    MapFrame::MapFrame(std::string imageName, vector<KeyPoint> keyPoints) {
        this->imageName = imageName;
        this->keyPoints = keyPoints;
    }
    MapFrame::~MapFrame() {}
    MapFrame::MapFrame(const MapFrame& mapFrame): imageName(mapFrame.imageName),keyPoints(mapFrame.keyPoints){}


}