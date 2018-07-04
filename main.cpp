#include "FinderPatternDetector.h"

int main() {
    FinderPatternDetector object;

    for (int i = 1; i <= 47; i++) {
        if (i < 10) {
            object.ReadImage("./img/TestSet1/000" + std::to_string(i) + ".jpg");
        } else {
            object.ReadImage("./img/TestSet1/00" + std::to_string(i) + ".jpg");
        }
        object.Detector();
        object.CreateResultImage();
    }

    for (int i = 1; i <= 48; i++) {
        if (i < 10) {
            object.ReadImage("./img/TestSet2/000" + std::to_string(i) + ".jpg");
        } else {
            object.ReadImage("./img/TestSet2/00" + std::to_string(i) + ".jpg");
        }
        object.Detector();
        object.CreateResultImage();
    }

    for (int i = 1; i <= 150; i++) {
        if (i < 10) {
            object.ReadImage("./img/TestSet3/000" + std::to_string(i) + ".jpg");
        } else if (i < 100) {
            object.ReadImage("./img/TestSet3/00" + std::to_string(i) + ".jpg");
        } else {
            object.ReadImage("./img/TestSet3/0" + std::to_string(i) + ".jpg");
        }
        object.Detector();
        object.CreateResultImage();
    }

    return 0;
}