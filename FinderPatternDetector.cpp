#include "FinderPatternDetector.h"

void FinderPatternDetector::ReadImage(string path) {
    pathInputImage = path;
    inputImage = imread(pathInputImage, CV_LOAD_IMAGE_COLOR);
    if(inputImage.empty()) {
        cout <<  "Could not open or find the image" << std::endl;
        exit(-1);
    }
    cvtColor(inputImage, imageBW, CV_BGR2GRAY);
    adaptiveThreshold(imageBW, imageBW, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 51, 0);
}

void FinderPatternDetector::CreateResultImage() {
    string pathResultImage;
    for(size_t i = 0; i < pathInputImage.size() - 4; i++) {
        pathResultImage += pathInputImage[i];
    }
    pathResultImage += "_result.jpg";
    imwrite(pathResultImage, inputImage);
}

bool FinderPatternDetector::Detector() {
    bool found = Find();
    if(found) {
        DrawFinderPatterns();
    }
    return found;
}

float FinderPatternDetector::GetCenter(int counterState[], int end) {
    return end - counterState[4] - counterState[3] - counterState[2] / 2.0f;
}

bool FinderPatternDetector::Find() {
    centers.clear();
    moduleSize.clear();

    int counterState[5] = {0};
    int currentState = 0;
    for(int row = 0; row < imageBW.rows; row++) {
        counterState[0] = 0;
        counterState[1] = 0;
        counterState[2] = 0;
        counterState[3] = 0;
        counterState[4] = 0;
        currentState = 0;
        const uchar* ptr = imageBW.ptr<uchar>(row);
        for(int col = 0; col < imageBW.cols; col++) {
            if(ptr[col] < 128) { //черный пиксель
                if((currentState & 0x1)==1) {
                    currentState++;
                }
                counterState[currentState]++;
            } else { //белый пиксель
                if((currentState & 0x1)==1) {
                    counterState[currentState]++;
                }
                else {
                    if(currentState==4) {
                        if(!(CheckRatio(counterState) and HandlePossibleCenter(counterState, row, col))) {
                            currentState = 3;
                            counterState[0] = counterState[2];
                            counterState[1] = counterState[3];
                            counterState[2] = counterState[4];
                            counterState[3] = 1;
                            counterState[4] = 0;
                            continue;
                        }
                        currentState = 0;
                        counterState[0] = 0;
                        counterState[1] = 0;
                        counterState[2] = 0;
                        counterState[3] = 0;
                        counterState[4] = 0;
                    }
                    else {
                        currentState++;
                        counterState[currentState]++;
                    }
                }
            }
        }
    }
    return !centers.empty();
}

bool FinderPatternDetector::CheckRatio(int stateCount[]) {
    int totalWidth = 0;
    for(int i = 0; i < 5; i++) {
        if (!stateCount[i]) {
            return false;
        }
        totalWidth += stateCount[i];
    }

    if(totalWidth < 7) {
        return false;
    }

    int width = ceil(totalWidth / 7.0);
    int dispersion = width / 2;

    return ((abs(width - (stateCount[0])) < dispersion) && (abs(width - (stateCount[1])) < dispersion) &&
            (abs(3 * width - (stateCount[2])) < 3 * dispersion) && (abs(width - (stateCount[3])) < dispersion) &&
            (abs(width - (stateCount[4])) < dispersion));
}

bool FinderPatternDetector::HandlePossibleCenter(int stateCount[], int row, int col) {
    int totalState = 0;
    for(int i = 0; i < 5; i++) {
        totalState += stateCount[i];
    }

    int centerCol = (int)GetCenter(stateCount, col);

    int centerRow = (int)CheckVertical(row, centerCol, stateCount[2], totalState);
    if(centerRow == -1.0f) {
        return false;
    }

    centerCol = (int)CheckHorizontal(centerRow, centerCol, stateCount[2], totalState);
    if(centerCol == -1.0f) {
        return false;
    }

    if(!CheckDiagonal(centerRow, centerCol, stateCount[2], totalState)) {
        return false;
    }

    Point2f newCenter(centerCol, centerRow);
    float newModuleSize = totalState / 7.0f;

    bool found = false;
    int index = 0;
    for(Point2f point : centers) {
        Point2f diff = point - newCenter;
        float distance = sqrt(diff.dot(diff));

        //Если между точками меньше 10 пикселей, то они равны
        if(distance < 10) {
            point = point + newCenter;
            point.x /= 2.0f;
            point.y /= 2.0f;
            moduleSize[index] = (moduleSize[index] + newModuleSize)/2.0f;
            found = true;
            break;
        }
        index++;
    }

    if(!found) {
        centers.push_back(newCenter);
        moduleSize.push_back(newModuleSize);
    }

    return false;
}

float FinderPatternDetector::CheckVertical(int startRow, int centerCol, int centralCount, int stateCountTotal) {
    int counterState[5] = {0};
    int row = startRow;

    while(row >= 0 && imageBW.at<uchar>(row, centerCol) < 128) {
        counterState[2]++;
        row--;
    }

    if(row < 0) {
        return -1.0f;
    }

    while(row >= 0 && imageBW.at<uchar>(row, centerCol) >= 128 && counterState[1] < centralCount) {
        counterState[1]++;
        row--;
    }

    if(row < 0 || counterState[1] >= centralCount) {
        return -1.0f;
    }

    while(row >= 0 && imageBW.at<uchar>(row, centerCol) < 128 && counterState[0] < centralCount) {
        counterState[0]++;
        row--;
    }

    if(row < 0 || counterState[0] >= centralCount) {
        return -1.0f;
    }

    row = startRow + 1;
    while(row < imageBW.rows && imageBW.at<uchar>(row, centerCol) < 128) {
        counterState[2]++;
        row++;
    }

    if(row == imageBW.rows) {
        return -1.0f;
    }

    while(row < imageBW.rows && imageBW.at<uchar>(row, centerCol) >= 128 && counterState[3] < centralCount) {
        counterState[3]++;
        row++;
    }

    if(row == imageBW.rows || counterState[3] >= stateCountTotal) {
        return -1.0f;
    }

    while(row < imageBW.rows && imageBW.at<uchar>(row, centerCol) < 128 && counterState[4] < centralCount) {
        counterState[4]++;
        row++;
    }

    if(row == imageBW.rows || counterState[4] >= centralCount) {
        return -1.0f;
    }

    int counterStateTotal = 0;
    for(int i = 0; i < 5; i++) {
        counterStateTotal += counterState[i];
    }

    if(5 * abs(counterStateTotal - stateCountTotal) >= 2 * stateCountTotal) {
        return -1.0f;
    }

    float center = GetCenter(counterState, row);
    return CheckRatio(counterState) ? center: -1.0f;
}

float FinderPatternDetector::CheckHorizontal(int centerRow, int startCol, int centerCount, int stateCountTotal) {
    int counterState[5] = {0};

    int col = startCol;
    const uchar* ptr = imageBW.ptr<uchar>(centerRow);
    while(col >= 0 && ptr[col] < 128) {
        counterState[2]++;
        col--;
    }
    if(col < 0) {
        return -1.0f;
    }

    while(col >= 0 && ptr[col] >= 128 && counterState[1] < centerCount) {
        counterState[1]++;
        col--;
    }
    if(col < 0 || counterState[1] == centerCount) {
        return -1.0f;
    }

    while(col >= 0 && ptr[col] < 128 && counterState[0] < centerCount) {
        counterState[0]++;
        col--;
    }
    if(col < 0 || counterState[0] == centerCount) {
        return -1.0f;
    }

    col = startCol + 1;
    while(col < imageBW.cols && ptr[col] < 128) {
        counterState[2]++;
        col++;
    }
    if(col == imageBW.cols) {
        return -1.0f;
    }

    while(col < imageBW.cols && ptr[col] >= 128 && counterState[3] < centerCount) {
        counterState[3]++;
        col++;
    }
    if(col == imageBW.cols || counterState[3] == centerCount) {
        return -1.0f;
    }

    while(col < imageBW.cols && ptr[col] < 128 && counterState[4] < centerCount) {
        counterState[4]++;
        col++;
    }
    if(col == imageBW.cols || counterState[4] == centerCount) {
        return -1.0f;
    }

    int counterStateTotal = 0;
    for(int i = 0;i < 5; i++) {
        counterStateTotal += counterState[i];
    }

    if(5 * abs(stateCountTotal - counterStateTotal) >= stateCountTotal) {
        return -1.0f;
    }

    if (CheckRatio(counterState)) {
        return GetCenter(counterState, col);
    } else {
        return -1.0f;
    }
}

bool FinderPatternDetector::CheckDiagonal(float centerRow, float centerCol, int maxCount, int stateCountTotal) {
    int stateCount[5] = {0};

    int i = 0;
    while(centerRow >= i && centerCol >= i && imageBW.at<uchar>(centerRow - i, centerCol - i) < 128) {
        stateCount[2]++;
        i++;
    }
    if(centerRow < i || centerCol < i) {
        return false;
    }

    while(centerRow >= i && centerCol >= i && imageBW.at<uchar>(centerRow - i, centerCol - i) >= 128 && stateCount[1] <= maxCount) {
        stateCount[1]++;
        i++;
    }
    if(centerRow < i || centerCol < i || stateCount[1] > maxCount) {
        return false;
    }

    while(centerRow >= i && centerCol >= i && imageBW.at<uchar>(centerRow - i, centerCol - i) < 128 && stateCount[0] <= maxCount) {
        stateCount[0]++;
        i++;
    }
    if(stateCount[0] > maxCount) {
        return false;
    }

    i = 1;
    while((centerRow + i) < imageBW.rows && (centerCol + i) < imageBW.cols && imageBW.at<uchar>(centerRow + i, centerCol + i)<128) {
        stateCount[2]++;
        i++;
    }
    if((centerRow + i) >= imageBW.rows || (centerCol + i) >= imageBW.cols) {
        return false;
    }

    while((centerRow + i) < imageBW.rows && (centerCol + i) < imageBW.cols && imageBW.at<uchar>(centerRow + i, centerCol + i)>=128 && stateCount[3] < maxCount) {
        stateCount[3]++;
        i++;
    }
    if((centerRow + i) >= imageBW.rows || (centerCol + i) >= imageBW.cols || stateCount[3] > maxCount) {
        return false;
    }

    while((centerRow + i) < imageBW.rows && (centerCol + i) < imageBW.cols && imageBW.at<uchar>(centerRow + i, centerCol + i)<128 && stateCount[4] < maxCount) {
        stateCount[4]++;
        i++;
    }
    if((centerRow+i) >= imageBW.rows || (centerCol+i) >= imageBW.cols || stateCount[4] > maxCount) {
        return false;
    }

    int newStateCountTotal = 0;

    for(int j = 0; j < 5; j++) {
        newStateCountTotal += stateCount[j];
    }

    return (abs(stateCountTotal - newStateCountTotal) < 2 * stateCountTotal) && CheckRatio(stateCount);
}

void FinderPatternDetector::DrawFinderPatterns() {
    if(centers.size()==0) {
        return;
    }

    for(int i = 0;i < centers.size(); i++) {
        Point2f pt = centers[i];
        float diff = moduleSize[i] * 3.5f;

        Point2f point1(pt.x - diff, pt.y - diff);
        Point2f point2(pt.x + diff, pt.y + diff);
        rectangle(inputImage, point1, point2, CV_RGB(255, 0, 0), 1);
    }
}