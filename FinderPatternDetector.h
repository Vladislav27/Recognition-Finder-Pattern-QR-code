#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;


class FinderPatternDetector {
public:
    void ReadImage(string path);
    bool Detector();
    void CreateResultImage();

private:
    bool Find();
    void DrawFinderPatterns();
    bool CheckRatio(int stateCount[]);
    bool HandlePossibleCenter(int stateCount[], int row, int col);
    bool CheckDiagonal(float centerRow, float centerCol, int maxCount, int stateCountTotal);
    float CheckVertical(int startRow, int centerCol, int centralCount, int stateCountTotal);
    float CheckHorizontal(int centerRow, int startCol, int centerCount, int stateCountTotal);
    float GetCenter(int stateCount[], int end);

    vector<Point2f> centers;
    vector<float> moduleSize;

    string pathInputImage;
    Mat inputImage;
    Mat imageBW;
};