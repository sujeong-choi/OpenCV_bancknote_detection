#include <opencv2\opencv.hpp>
#include <Windows.h>
#include<math.h>
#include <string>
#include <windows.h>
#include <sapi.h>
#include <iostream>
#include "Pixel.h"

using namespace cv;
using namespace std;

int num1 = 0, num10 = 0;//1000원 10000원
int num5 = 0;//5000원 50000원 개수
int ind = 0;// 돈이 아닌 객체 개수 
double num50 = 0;


class Picture {
public:
    int r = 0; int  g = 0; int b = 0;
    double area = 0;
    int color;
};

void classfication(Picture* a, int n)// 면적에 따른 지폐 분류
{
    int i = 0;
    while (i < n - 1) {

        if (48 <= a->b && a->b <= 67 && 81 <= a->g && a->g <= 100 && 93 <= a->r && a->r <= 121) {
            if (9950 < a->area) num50++;
            else if (a->b == 67 && a->area < 3000) num5++;
            else num50 += 0.5;
        }
        else if (67 <= a->b && a->b <= 88 && 76 <= a->g && a->g <= 101 && 100 <= a->r && a->r <= 114) {
            if (a->area < 10000) num5++;
            else num50++;
        }
        else if (65 <= a->b && a->b <= 86 && 86 <= a->g && a->g <= 103 && 78 <= a->r && a->r <= 91) {
            if (3000 < a->area && a->area < 15000) num10++;
        }
        else if (88 <= a->b && a->b <= 121 && 66 <= a->g && a->g <= 97 && 64 <= a->r && a->r <= 86) {
            if (2800 < a->area && a->area < 13000) num1++;

        }
        else ind++;
        i++;
        a++;
    }
}
int object_detection(Mat& a, Mat& dst, double& ratio, int num)//객체 검출 (원본 threshold, 출력) --> 빨간색 검출 (머리도 나올수 0)
{
    int count = 0;

    if (a.empty()) {// 이미지 없는 경우 error
        cerr << "image load error" << endl;
        return 0;
    }

    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;

    findContours(a, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);//윤곽선 찾기 (객체화)
    cvtColor(a, dst, COLOR_GRAY2BGR);// gray scale이미지 → BGR 이미지 (라벨링)

    if (hierarchy.empty()) return 0;//아무것도 없는 경우  --> 종료

    for (int idx = 0; idx >= 0; idx = hierarchy[idx][0]) {// 객체 검사
        double tmp_area = contourArea(contours[idx]);
        if (num == 1) {
            double standard = 12000; // 기준이 되는 표식 넓이
            if (tmp_area > 1000 && tmp_area < 200000 && ratio < standard / tmp_area) {
                cout << tmp_area << endl;
                ratio = standard / contourArea(contours[idx]);
            }
        }
        else {
            tmp_area = tmp_area * ratio;
            if (tmp_area > 2000 && tmp_area < 1000000) {//지폐 or 표식 넓이에 포합되는 경우
                count++;
                //cout << "면적" << tmp_area << endl;// 돈 면적 출력
                Scalar color(255, 255, 255);
                drawContours(dst, contours, idx, color, -1, LINE_8, hierarchy);// 지폐 일 경우에만 255 흰색으로 객체 내부 채워 출력 
            }
            else {//지폐나 표식이 아닌 경우
                Scalar color(0, 0, 0);
                drawContours(dst, contours, idx, color, -1, LINE_8, hierarchy);// 해당 객체 검정색으로 출력 
            }
        }
    }
    return count;
}

int thresh_value_decision(double ratio) {
    if (ratio < 0.5) {
        cout << "높이가 낮아 정확한 측정이 어렵습니다" << endl;
        return 120;
    }
    else if (ratio < 0.9) return 60;
    else if (ratio < 1.06) return 60;
    else if (ratio < 1.155)  return 60;
    else return 60;
    return 60;
}

int voice_annaounce(int num) {
    ISpVoice* pVoice = NULL;

    if (FAILED(::CoInitialize(NULL)))
        return FALSE;

    HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);
    if (SUCCEEDED(hr))
    {
        cout << "Succeeded" << endl;
        // 1개일 때 
        if (num == 1) {
            hr = pVoice->Speak(L"1개", 0, NULL);
        }
        // 2개일 때 
        else if (num == 2) {
            hr = pVoice->Speak(L"2개", 0, NULL);
        }
        // 3개일 때 
        else if (num == 3) {
            hr = pVoice->Speak(L"3개", 0, NULL);
        }
        // 4개일 때 
        else if (num == 4) {
            hr = pVoice->Speak(L"4개", 0, NULL);
        }
        // 4개 이상일 때 
        else {
            hr = pVoice->Speak(L"여러개", 0, NULL);
        }
        pVoice->Release();
        pVoice = NULL;
    }
    else
    {
        cout << "Not succeeded" << endl;
    }

    ::CoUninitialize();
    return TRUE;

}

// 각 객체의 rgb color 평균 반환
void getEachColor(Mat input, Mat original, Picture* result, int num, double& ratio) {
    Mat tmp = original.clone();
    Mat mult = input.clone();
    Mat r, dst2;


    resize(mult, mult, Size(input.cols, input.rows), 0, 0, CV_INTER_LINEAR);// 사이즈 변경 
    int nBlue = 0, nGreen = 0, nRed = 0; // 두 영상의 곱 변수 
    for (int i = 0; i < tmp.size().height; i++) {
        for (int j = 0; j < tmp.size().width; j++) {
            nBlue = tmp.at<Vec3b>(i, j)[0] * input.at<Vec3b>(i, j)[0];
            nGreen = tmp.at<Vec3b>(i, j)[1] * input.at<Vec3b>(i, j)[1];
            nRed = tmp.at<Vec3b>(i, j)[2] * input.at<Vec3b>(i, j)[2];//rgb 각각의 곱 결과
            // cramping 고려해 mult에 값 할당 
            if (nBlue > 255) {
                mult.at<Vec3b>(i, j)[0] = 255;
            }
            else mult.at<Vec3b>(i, j)[0] = nBlue;

            if (nGreen > 255) {
                mult.at<Vec3b>(i, j)[1] = 255;
            }
            else mult.at<Vec3b>(i, j)[1] = nGreen;

            if (nRed > 255) {
                mult.at<Vec3b>(i, j)[2] = 255;
            }
            else mult.at<Vec3b>(i, j)[2] = nRed;

        }
    }
    //imshow("mult", mult);

    int pix = 0;
    int m_b = 0, m_g = 0, m_r = 0;

    for (int i = 0; i < mult.size().height; i++) {
        for (int j = 0; j < mult.size().width; j++) {
            if (mult.at<Vec3b>(i, j)[0] == 0 && mult.at<Vec3b>(i, j)[1] == 0 && mult.at<Vec3b>(i, j)[2] == 0)
                continue;
            m_b += mult.at<Vec3b>(i, j)[0];
            m_g += mult.at<Vec3b>(i, j)[1];
            m_r += mult.at<Vec3b>(i, j)[2];
            pix++;

        }
    }
    result[num].r = (m_r / pix);
    result[num].g = (m_g / pix);
    result[num].b = (m_b / pix);

    cout << "b = " << (m_b / pix) << endl;
    cout << "g = " << (m_g / pix) << endl;
    cout << "r = " << (m_r / pix) << endl;

    //int r_ratio = m_r * ratio;
    //int g_ratio = m_g * ratio;
    //int b_ratio = m_b * ratio;
    //int pix_ratio = pix * ratio;


    //cout << "b = " << b_ratio/pix_ratio << endl;
    //cout << "g = " << g_ratio/pix_ratio << endl;
    //cout << "r = " << r_ratio/pix_ratio << endl;
}

// 객체 넓이 변환
double getArea(Mat& a, Picture* result, int num, double ratio) {
    if (a.empty()) {// 이미지 없는 경우 error
        cerr << "image load error" << endl;
        return 0;
    }

    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    cvtColor(a, a, COLOR_BGR2GRAY);
    findContours(a, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);//윤곽선 찾기 (객체화)

    if (hierarchy.empty()) {
        cout << "객체가 없습니다. " << endl;
        return 0;//아무것도 없는 경우  --> 종료
    }

    result[num].area = contourArea(contours[0]) * ratio;
    cout << " area = " << result[num].area << endl;

}

// labeling 객체 반환
void getEachObject(double& ratio, int object_count, Mat img_labels, Mat img_color_tmp, Mat original, Picture* result) {
    Mat img_color;
    //Mat img_binary = input.clone();
    //cvtColor(img_binary, img_color_tmp, CV_GRAY2BGR);
    double tmp;


    int num = 1; // 0은 배경이므로 제외
    ////imshow("img_labels", img_labels);

    //cout << "numOfLables = " << numOfLabels << endl;
    //result = new Picture[numOfLabels];
    //라벨링된 이미지중 특정 라벨을 컬러로 표현해주기  
    while (num < object_count) {
        // output을 생성할 img_color 영상을 초기화
        img_color = img_color_tmp.clone();
        // num번째 객체를 (0,255,0)으로 색칠
        for (int y = 0; y < img_labels.rows; ++y) {
            // cout << "for문 " << endl;
            int* label = img_labels.ptr<int>(y);
            Vec3b* pixel = img_color.ptr<Vec3b>(y);
            for (int x = 0; x < img_labels.cols; ++x) {
                if (label[x] == num) {
                    pixel[x][2] = 1;
                    pixel[x][1] = 1;
                    pixel[x][0] = 1;
                }
                else
                {
                    pixel[x][2] = 0;
                    pixel[x][1] = 0;
                    pixel[x][0] = 0;

                }
            }
        }


        // mult이미지 구하는 부분 
        //cout << num << "번째 객체의 bgr color" << endl;
        getEachColor(img_color, original, result, num - 1, ratio);
        getArea(img_color, result, num - 1, ratio);
        if (num == 1) imwrite("45/out1.png", img_color);
        else if (num == 2) imwrite("45/out2.png", img_color);
        else if (num == 3) imwrite("45/out3.png", img_color);
        else imwrite("45/out4.png", img_color);
        num++;
    }

}

// 배경 차분 함수
Mat remove_back(Mat& imageOriginal, Mat& backOriginal) { //rgb
    Mat image = imageOriginal;
    Mat back = backOriginal;
    //cvtColor(image, image, COLOR_BGR2HSV);
    //cvtColor(back, back, COLOR_BGR2HSV); 

    for (int i = 0; i < image.rows; i++)
    {
        for (int j = 0; j < image.cols; j++)
        {


            //back.at<Vec3b>(i, j)[0] = 0;
            //back.at<Vec3b>(i, j)[1] = 0;
            //back.at<Vec3b>(i, j)[2] = 100;
            //if (abs(imageH - backH) < 20 && abs(imageS - backS) < 20) {
            //    image.at<Vec3b>(i, j)[0] = 0;
            //    image.at<Vec3b>(i, j)[1] = 0;
            //    image.at<Vec3b>(i, j)[2] = 0;
            //    back.at<Vec3b>(i, j)[2] = 0;

            //}
            Vec3b image_hsv = image.at<Vec3b>(i, j);
            Vec3b back_hsv = back.at<Vec3b>(i, j);
            int imageB = image_hsv.val[0]; //hue
            int imageG = image_hsv.val[1]; //saturation
            int imageR = image_hsv.val[2]; //saturation
            int backB = back_hsv.val[0]; //hue
            int backG = back_hsv.val[1]; //saturation
            int backR = back_hsv.val[2]; //saturation

            back.at<Vec3b>(i, j)[0] = 215;
            back.at<Vec3b>(i, j)[1] = 215;
            back.at<Vec3b>(i, j)[2] = 215;
            if (abs(imageB - backB) < 25 && abs(imageG - backG) < 25 && abs(imageR - backR) < 25) {
                image.at<Vec3b>(i, j)[0] = 0;
                image.at<Vec3b>(i, j)[1] = 0;
                image.at<Vec3b>(i, j)[2] = 0;
                back.at<Vec3b>(i, j)[0] = 0;
                back.at<Vec3b>(i, j)[1] = 0;
                back.at<Vec3b>(i, j)[2] = 0;

            }
        }
    }

    //cvtColor(image, image, COLOR_HSV2BGR);
    //cvtColor(back, back, COLOR_HSV2BGR);
    //back = back * 2.3;  // 흰색으로 바꾸는 부분
    //Mat sub = (back - image);

   /* cvtColor(image, imageOriginal, COLOR_HSV2BGR);
    cvtColor(back, backOriginal, COLOR_HSV2BGR);*/
    //imshow("back", backOriginal);
    //backOriginal = backOriginal * 2.2;  // 흰색으로 바꾸는 부분
    //imshow("back_after", backOriginal);
    Mat sub = (backOriginal - imageOriginal);
    return sub;
}

// 빨간 부분만 추출하기 위한 filter 구하는 함수
void red_detection(Mat& image, Mat& red_only) {
    Mat image_hsv;
    cvtColor(image, image_hsv, COLOR_BGR2HSV);
    Scalar redBottom(-20, 0, 0), redUpper(60, 40, 255);
    inRange(image, redBottom, redUpper, red_only);
}

void thresholding_bgr(Mat& image, Mat& dst, int thresh_value) {
    CPixel photo;
    Mat gray_image;
    cvtColor(image, gray_image, CV_RGB2GRAY);//color -> grayscale
    imwrite("45/gray2.png", gray_image);
    dst = photo.GS_threshold(gray_image, thresh_value, 255, 0);
}


int vmain() {

    CPixel photo;
    int numPPP;
    int num_money;
    cout << "지폐의 수를 입력해주세요:";
    cin >> numPPP;
    string s;

    for (int i = 2; i <= 2; i++) {
        num1 = num5 = num10 = num50 = ind = 0;
        string tmp = "45/" + to_string(i) + ".png";
        //Mat image = imread("45/KakaoTalk_20210615_194917496_0" + to_string(i) + ".jpg", IMREAD_REDUCED_COLOR_4);//1/4 줄인 color 영상으로 읽기
        Mat image = imread("45/102.png", IMREAD_REDUCED_COLOR_4);//1/4 줄인 color 영상으로 읽기
        Mat back = imread("45/KakaoTalk_20210615_210703062_01.jpg", IMREAD_REDUCED_COLOR_4);
        //Mat back_white = imread("45/KakaoTalk_20210225_193606598_15.jpg", IMREAD_REDUCED_COLOR_4);
        Mat back_white = imread("white/45.jpg", IMREAD_REDUCED_COLOR_4);

        Mat original = image.clone();

        // ---- only red area detection part ------------------------------------------------------------------------------------------------------------------------------

        double ratio = 0; // 비율값
        Mat onlyRedFilter, onlyRed;
        red_detection(image, onlyRedFilter);
        //imshow("image", image);
        cvtColor(onlyRedFilter, onlyRedFilter, COLOR_GRAY2BGR);
        //imshow("red_only_filter", onlyRedFilter);
        onlyRed = image - (~onlyRedFilter);
        imwrite("red.png", onlyRed);

        //------------------------------------------------------------------------------------------------------------------- only red area detection part end -----------------------

        // get ratio value  part --------------------------------------------------------------------------------------------------------------------------------------

        Mat thresh_onlyRed, thresh_onlyRed_dst;
        thresholding_bgr(onlyRed, thresh_onlyRed, 10);
        //imshow("thresh_onlyRed", thresh_onlyRed);
        object_detection(thresh_onlyRed, thresh_onlyRed_dst, ratio, 1);
        cout << "ratio = " << ratio << endl;

        // -------------------------------------------------------------------------------------------------------------------- get ratio value part end----------------------

        //----- 배경 차분 part --------------------------------------------------------------------------------------------------------------------------------------------

        Mat sub_image;
        //sub_image = remove_back(image, back);
        Mat sub1 = image - back;
        Mat sub2 = back - image;
        //imwrite("sub1.png", sub1);
        //imwrite("sub2.png", sub2);
        Mat tmp_sub = image.clone();

        for (int i = 0; i < image.rows; i++)
        {
            for (int j = 0; j < image.cols; j++)
            {

                int i_b = image.at<Vec3b>(i, j)[0];
                int i_g = image.at<Vec3b>(i, j)[1];
                int i_r = image.at<Vec3b>(i, j)[2];
                int b_b = back.at<Vec3b>(i, j)[0];
                int b_g = back.at<Vec3b>(i, j)[1];
                int b_r = back.at<Vec3b>(i, j)[2];

                int w_b = back_white.at<Vec3b>(i, j)[0];
                int w_g = back_white.at<Vec3b>(i, j)[1];
                int w_r = back_white.at<Vec3b>(i, j)[2];

                if (abs(i_b - b_b) < 40 && abs(i_g - b_g) < 40 && abs(i_r - b_r) < 40) {
                    tmp_sub.at<Vec3b>(i, j)[0] = w_b;
                    tmp_sub.at<Vec3b>(i, j)[1] = w_g;
                    tmp_sub.at<Vec3b>(i, j)[2] = w_r;
                    /*tmp_sub.at<Vec3b>(i, j)[0] = 0;
                    tmp_sub.at<Vec3b>(i, j)[1] = 0;
                    tmp_sub.at<Vec3b>(i, j)[2] = 0; */
                }
                else {
                    tmp_sub.at<Vec3b>(i, j)[0] = i_b;
                    tmp_sub.at<Vec3b>(i, j)[1] = i_g;
                    tmp_sub.at<Vec3b>(i, j)[2] = i_r;
                }
            }
        }
        imwrite("45/sub_image_tmp.png", tmp_sub);

        sub_image = (tmp_sub - back_white) + (back_white - tmp_sub); // subimage- whight back 
        //sub_image = (image -back) + (back-image);
        imwrite("45/sub_image2.png", sub_image);

        //-------------------------------------------------------------------------------------------------------------------- 배경 차분 part end -----------------------------

        // ---- using thresholding for removing noise part ----------------------------------------------------------------------------------------------------------------------------

        Mat thresh_image;
        int thresh_value = thresh_value_decision(ratio);
        thresholding_bgr(sub_image, thresh_image, thresh_value);
        imwrite("45/thresholing2.png", thresh_image);
        dilate(thresh_image, thresh_image, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
        imwrite("45/dilate2.png", thresh_image);

        //---------------------------------------------------------------------------------------------------------------using thresholding for removing noise part end-----------------------

        //----- 지폐 or 표식 부분만 색 표시 part ---------------------------------------------------------------------------------------------------------------------------------------------

        Mat thresh_dst;
        int object_count = object_detection(thresh_image, thresh_dst, ratio, 0);// 라벨링(객체화) & 지폐 검출
        imwrite("지폐.png", thresh_dst);
        Mat color_thresh_dst = image - (~thresh_dst);
        imwrite("45/머리 색칠2.png", color_thresh_dst);

        // --------------------------------------------------------------------------------------------지폐 or 표식 부분만 색 표시 part end ------------------------------------

        // ---- 각 객체의 넓이와 색상 정보 얻는 part --------------------------------------------------------------------------------------------------------------------------------

        Mat gray_thresh_dst, labels, stats, centroids;
        cvtColor(thresh_dst, gray_thresh_dst, COLOR_BGR2GRAY);
        int numOfLabels = connectedComponentsWithStats(gray_thresh_dst, labels, stats, centroids, 8, CV_32S);
        int num = 1; // 0은 배경이므로 제외

        Picture* result;
        result = new Picture[numOfLabels];

        getEachObject(ratio, numOfLabels, labels, thresh_dst, original, result);

        // ---------------------------------------------------------------------------------------------------------- 각 객체의 넓이와 색상 정보 얻는 part end-------------------------

        // ---- 지폐 판단 부분 ----------------------------------------------------------------------------------------------------------------------------------------------------

        classfication(result, numOfLabels);

        if (num50 - (int)(num50 / 1) == 0.5) num5++;

        /* cout << "1000원:  " << num1 << endl;
         cout << "5000원:  " << num5 << endl;
         cout << "10000원:  " << num10 << endl;
         cout << "50000원:  " << int(num50) << endl;
         cout << "나머지 : " << ind << endl;*/

        if (numPPP != (num1 + num5 + num10 + num50)) {
            cout << "입력된 지폐의 개수와 일치하지 않습니다." << endl;
            cout << "1000원:  " << num1 << endl;
            cout << "5000원:  " << num5 << endl;
            cout << "10000원:  " << num10 << endl;
            cout << "50000원:  " << num50 << endl;
        }
        else {
            cout << "1000원:  " << num1 << endl;
            cout << "5000원:  " << num5 << endl;
            cout << "10000원:  " << num10 << endl;
            cout << "50000원:  " << num50 << endl;
        }

        // -------------------------------------------------------------------------------------------------------- 지폐 판단 부분 --------------------------------------------------

        waitKey(0);

    }
    return 0;
}


int Num(int num) {
    ISpVoice* pVoice = NULL;

    if (FAILED(::CoInitialize(NULL)))
        return FALSE;

    HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);
    if (SUCCEEDED(hr))
    {
        //cout << "Succeeded" << endl;
        // 1개일 때 
        if (num == 1) {
            hr = pVoice->Speak(L"1개", 0, NULL);
        }
        // 2개일 때 
        else if (num == 2) {
            hr = pVoice->Speak(L"2개", 0, NULL);
        }
        // 3개일 때 
        else if (num == 3) {
            hr = pVoice->Speak(L"3개", 0, NULL);
        }
        // 4개일 때 
        else if (num == 4) {
            hr = pVoice->Speak(L"4개", 0, NULL);
        }
        // 4개 이상일 때 
        else {
            hr = pVoice->Speak(L"여러개", 0, NULL);
        }
        pVoice->Release();
        pVoice = NULL;
    }
    else
    {
        cout << "Not succeeded" << endl;
    }

    ::CoUninitialize();
    return TRUE;

}
int main(int argc, char* argv[])
{
    // 지폐 개수 알아내는 함수
    vmain();
    //cout << "main" << endl;

    ISpVoice* pVoice = NULL;

    if (FAILED(::CoInitialize(NULL)))
        return FALSE;

    HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);
    if (SUCCEEDED(hr))
    {
        //cout << "Succeeded" << endl;
        // 지폐 개수가 모두 0인 경우 
        // 지페가 없다고 출력
        if (num1 == 0 && num10 == 0 && num5 == 0 && num50 == 0)
            hr = pVoice->Speak(L"지폐가 없습니다", 0, NULL);
        else {
            // 1000원 지폐가 있는 경우
            if (num1 != 0) {
                hr = pVoice->Speak(L"1000원", 0, NULL);
                //  num1 개수에 해당하는 음성 안내 출력
                Num(num1);
            }
            // 5000원 지폐가 있는 경우
            if (num5 != 0) {
                hr = pVoice->Speak(L"5000원", 0, NULL);
                // num5 개수에 해당하는 음성 안내 출력
                Num(num5);
            }
            // 10000원 지폐가 있는 경우
            if (num10 != 0) {
                hr = pVoice->Speak(L"10000원", 0, NULL);
                // num10 개수에 해당하는 음성 안내 출력
                Num(num10);
            }
            // 50000원 지폐가 있는 경우
            if (num50 != 0) {
                hr = pVoice->Speak(L"50000원", 0, NULL);
                // num50 개수에 해당하는 음성 안내 출력
                Num(num50);
            }
            hr = pVoice->Speak(L"입니다", 0, NULL);

        }
        pVoice->Release();
        pVoice = NULL;
    }
    else
    {
        cout << "Not succeeded" << endl;
    }

    ::CoUninitialize();
    return TRUE;
}