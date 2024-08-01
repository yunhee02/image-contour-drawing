#include <windows.h>
#include <iostream>
#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>
#include <opencv2/opencv.hpp>
#include <sapi.h>

using namespace cv;
using namespace std;
using namespace tesseract;

// UTF-8 문자열을 ANSI 문자열로 변환하는 함수 선언
char* UTF8ToANSI(const char* pszCode);

Mat src;  // 원본 이미지를 저장하는 변수
Mat dst;  // 조절된 이미지를 저장하는 변수

int brightnessValue = 50;  // 밝기 트랙바의 초기 값
int contrastValue = 50;    // 대조도 트랙바의 초기 값

// 밝기 조절 트랙바 콜백 함수
void onBrightnessChange(int, void*) {
    double alpha = 1.0;
    int beta = brightnessValue - 50;
    src.convertTo(dst, -1, alpha, beta);
    imshow("조절된 이미지", dst);
}

// 대조도 조절 트랙바 콜백 함수
void onContrastChange(int, void*) {
    double alpha = contrastValue / 50.0;
    int beta = 0;
    src.convertTo(dst, -1, alpha, beta);
    imshow("조절된 이미지", dst);
}

// 텍스트를 음성으로 변환하는 함수
void textToSpeech(const char* text) {
    CoInitialize(NULL);

    ISpVoice* pVoice = NULL;

    if (FAILED(CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice))) {
        wcout << L"SAPI 초기화에 실패했습니다." << endl;
        CoUninitialize();
        return;
    }

    int len = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
    wchar_t* wideText = new wchar_t[len];
    MultiByteToWideChar(CP_UTF8, 0, text, -1, wideText, len);

    pVoice->Speak(wideText, 0, NULL);

    delete[] wideText;
    pVoice->Release();
    CoUninitialize();
}

int main() {
    // 이미지 로드
    src = imread("C:\\Users\\82103\\Desktop\\OpenCV 기말 TP03OCV\\안녕하세요.jpg");

    if (src.empty()) {
        cout << "이미지를 불러오는 데 실패했습니다." << endl;
        return -1;
    }

    namedWindow("조절된 이미지");

    // 밝기 조절 트랙바 생성 및 초기화
    createTrackbar("밝기", "조절된 이미지", &brightnessValue, 100, onBrightnessChange);
    onBrightnessChange(brightnessValue, 0);

    // 대조도 조절 트랙바 생성 및 초기화
    createTrackbar("대조도", "조절된 이미지", &contrastValue, 100, onContrastChange);
    onContrastChange(contrastValue, 0);

    waitKey(0);

    // 조절된 이미지를 파일로 저장
    imwrite("C:\\Users\\82103\\Desktop\\OpenCV 기말 TP03OCV\\안녕하세요.jpg", dst);

    // 흑백 이미지로 변환
    Mat gray;
    cvtColor(dst, gray, COLOR_BGR2GRAY);

    imshow("그레이스케일", gray);
    waitKey(0);

    // 이진화 수행
    Mat bw;
    threshold(gray, bw, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);

    imshow("이진 이미지", bw);
    waitKey(0);

    // 윤곽선 검출
    vector<vector<Point>> contours;
    findContours(bw.clone(), contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

    // 원본 이미지에 윤곽선 그리기
    Mat result = dst.clone();
    for (size_t i = 0; i < contours.size(); ++i) {
        drawContours(result, contours, i, Scalar(0, 255, 0), 0.5);
    }

    imshow("윤곽선", result);
    waitKey(0);

    // 윤곽선이 그려진 이미지를 파일로 저장
    imwrite("C:\\Users\\82103\\Desktop\\OpenCV 기말 TP03OCV\\안녕하세요.jpg", result);

    // Tesseract OCR 초기화 및 이미지 설정
    TessBaseAPI* api = new TessBaseAPI();
    if (api->Init("C:/Program Files/Tesseract-OCR/tessdata", "kor")) {
        return -1;
    }

    api->SetImage(result.data, result.cols, result.rows, result.channels(), result.step1());

    // OCR 수행
    char* outText = api->GetUTF8Text();
    string utf_text(outText);
    string text = UTF8ToANSI(utf_text.c_str());
    cout << text << endl;

    delete[] outText;

    api->End();
    delete api;

    // 텍스트를 음성으로 변환
    textToSpeech(utf_text.c_str());

    return 0;
}

// UTF-8 문자열을 ANSI 문자열로 변환하는 함수 정의
char* UTF8ToANSI(const char* pszCode) {
    BSTR bstrWide;
    char* pszAnsi;
    int nLength;

    nLength = MultiByteToWideChar(CP_UTF8, 0, pszCode, strlen(pszCode) + 1, NULL, NULL);
    bstrWide = SysAllocStringLen(NULL, nLength);

    MultiByteToWideChar(CP_UTF8, 0, pszCode, strlen(pszCode) + 1, bstrWide, nLength);

    nLength = WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, NULL, 0, NULL, NULL);
    pszAnsi = new char[nLength];

    WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, pszAnsi, nLength, NULL, NULL);
    SysFreeString(bstrWide);

    return pszAnsi;
}
