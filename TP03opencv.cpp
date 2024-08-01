#include <windows.h>
#include <iostream>
#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>
#include <opencv2/opencv.hpp>
#include <sapi.h>

using namespace cv;
using namespace std;
using namespace tesseract;

// UTF-8 ���ڿ��� ANSI ���ڿ��� ��ȯ�ϴ� �Լ� ����
char* UTF8ToANSI(const char* pszCode);

Mat src;  // ���� �̹����� �����ϴ� ����
Mat dst;  // ������ �̹����� �����ϴ� ����

int brightnessValue = 50;  // ��� Ʈ������ �ʱ� ��
int contrastValue = 50;    // ������ Ʈ������ �ʱ� ��

// ��� ���� Ʈ���� �ݹ� �Լ�
void onBrightnessChange(int, void*) {
    double alpha = 1.0;
    int beta = brightnessValue - 50;
    src.convertTo(dst, -1, alpha, beta);
    imshow("������ �̹���", dst);
}

// ������ ���� Ʈ���� �ݹ� �Լ�
void onContrastChange(int, void*) {
    double alpha = contrastValue / 50.0;
    int beta = 0;
    src.convertTo(dst, -1, alpha, beta);
    imshow("������ �̹���", dst);
}

// �ؽ�Ʈ�� �������� ��ȯ�ϴ� �Լ�
void textToSpeech(const char* text) {
    CoInitialize(NULL);

    ISpVoice* pVoice = NULL;

    if (FAILED(CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice))) {
        wcout << L"SAPI �ʱ�ȭ�� �����߽��ϴ�." << endl;
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
    // �̹��� �ε�
    src = imread("C:\\Users\\82103\\Desktop\\OpenCV �⸻ TP03OCV\\�ȳ��ϼ���.jpg");

    if (src.empty()) {
        cout << "�̹����� �ҷ����� �� �����߽��ϴ�." << endl;
        return -1;
    }

    namedWindow("������ �̹���");

    // ��� ���� Ʈ���� ���� �� �ʱ�ȭ
    createTrackbar("���", "������ �̹���", &brightnessValue, 100, onBrightnessChange);
    onBrightnessChange(brightnessValue, 0);

    // ������ ���� Ʈ���� ���� �� �ʱ�ȭ
    createTrackbar("������", "������ �̹���", &contrastValue, 100, onContrastChange);
    onContrastChange(contrastValue, 0);

    waitKey(0);

    // ������ �̹����� ���Ϸ� ����
    imwrite("C:\\Users\\82103\\Desktop\\OpenCV �⸻ TP03OCV\\�ȳ��ϼ���.jpg", dst);

    // ��� �̹����� ��ȯ
    Mat gray;
    cvtColor(dst, gray, COLOR_BGR2GRAY);

    imshow("�׷��̽�����", gray);
    waitKey(0);

    // ����ȭ ����
    Mat bw;
    threshold(gray, bw, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);

    imshow("���� �̹���", bw);
    waitKey(0);

    // ������ ����
    vector<vector<Point>> contours;
    findContours(bw.clone(), contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

    // ���� �̹����� ������ �׸���
    Mat result = dst.clone();
    for (size_t i = 0; i < contours.size(); ++i) {
        drawContours(result, contours, i, Scalar(0, 255, 0), 0.5);
    }

    imshow("������", result);
    waitKey(0);

    // �������� �׷��� �̹����� ���Ϸ� ����
    imwrite("C:\\Users\\82103\\Desktop\\OpenCV �⸻ TP03OCV\\�ȳ��ϼ���.jpg", result);

    // Tesseract OCR �ʱ�ȭ �� �̹��� ����
    TessBaseAPI* api = new TessBaseAPI();
    if (api->Init("C:/Program Files/Tesseract-OCR/tessdata", "kor")) {
        return -1;
    }

    api->SetImage(result.data, result.cols, result.rows, result.channels(), result.step1());

    // OCR ����
    char* outText = api->GetUTF8Text();
    string utf_text(outText);
    string text = UTF8ToANSI(utf_text.c_str());
    cout << text << endl;

    delete[] outText;

    api->End();
    delete api;

    // �ؽ�Ʈ�� �������� ��ȯ
    textToSpeech(utf_text.c_str());

    return 0;
}

// UTF-8 ���ڿ��� ANSI ���ڿ��� ��ȯ�ϴ� �Լ� ����
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
