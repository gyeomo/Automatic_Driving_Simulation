#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <Windows.h>
#include <iostream>
#include <stdio.h>

using namespace cv;
using namespace std;
void setColorScalar(int &low_hue1, int &high_hue1, int &low_hue2, int &high_hue2, int &range_count, int flag);

Mat hwnd2mat(HWND hwnd);
int main() {
	clock_t start, end;
	start = clock();
	HWND hwndDesktop = GetDesktopWindow();
	Mat src;
	Mat image_HSV, image_Dump, image_Red, image_Green, image_Yellow;
	int low_hue1 = 0, low_hue2 = 0;
	int high_hue1 = 0, high_hue2 = 0;
	int range_count = 0;
	int redFlag = 0, yellowFlag = 0, greenFlag = 0;
	Point redCenter, yellowCenter, greenCenter;
	vector<Vec3f> circles;
	for (;;) {
		src = hwnd2mat(hwndDesktop);
		if (src.empty())
			continue;
		resize(src, src, Size(320, 240));
		cvtColor(src, image_HSV, CV_BGR2HSV);
		low_hue1 = 0; low_hue2 = 0;
		high_hue1 = 0; high_hue2 = 0;
		range_count = 0;

		////////////////////////////////////
		if (redFlag == 0) {
			setColorScalar(low_hue1, high_hue1, low_hue2, high_hue2, range_count, 0);
			inRange(image_HSV, Scalar(low_hue1, 50, 127), Scalar(high_hue1, 255, 255), image_Red);
			if (range_count == 2) {
				inRange(image_HSV, Scalar(low_hue2, 50, 127), Scalar(high_hue2, 255, 255), image_Dump);
				image_Red |= image_Dump;
			}
			medianBlur(image_Red, image_Red, 5);
			Canny(image_Red, image_Red, 130, 210, 3);
			circles.clear();
			HoughCircles(image_Red, circles, HOUGH_GRADIENT, 1, 10,
				150, 15, 7, 50);
			if (circles.size() > 0) {
				redCenter = Point(circles[0][0], circles[0][1]);
				redFlag = 1;
			}
			else
				redFlag = 0;
		}
		////////////////////////////////////
		if (redFlag == 1 && yellowFlag == 0) {
			low_hue1 = 0; low_hue2 = 0;
			high_hue1 = 0; high_hue2 = 0;
			range_count = 0;
			setColorScalar(low_hue1, high_hue1, low_hue2, high_hue2, range_count, 2);
			inRange(image_HSV, Scalar(low_hue1, 50, 127), Scalar(high_hue1, 255, 255), image_Yellow);
			if (range_count == 2) {
				inRange(image_HSV, Scalar(low_hue2, 50, 127), Scalar(high_hue2, 255, 255), image_Dump);
				image_Yellow |= image_Dump;
			}
			medianBlur(image_Yellow, image_Yellow, 5);
			Canny(image_Yellow, image_Yellow, 130, 210, 3);
			circles.clear();
			HoughCircles(image_Yellow, circles, HOUGH_GRADIENT, 1, 10,
				150, 15, 7, 50);
			for (int i = 0; i < circles.size(); i++) {
				Vec3i c = circles[i];
				circle(src, Point(c[0], c[1]), c[2], Scalar(255, 0, 0), 2);
			}
			if (circles.size() > 0) {
				yellowFlag = 1;
				yellowCenter = Point(circles[0][0], circles[0][1]);
			}
			else
				yellowFlag = 0;
		}
		/////////////////////////////////////
		if (yellowFlag == 1) {
			low_hue1 = 0; low_hue2 = 0;
			high_hue1 = 0; high_hue2 = 0;
			range_count = 0;
			setColorScalar(low_hue1, high_hue1, low_hue2, high_hue2, range_count, 1);
			inRange(image_HSV, Scalar(low_hue1, 50, 127), Scalar(high_hue1, 255, 255), image_Green);
			if (range_count == 2) {
				inRange(image_HSV, Scalar(low_hue2, 50, 127), Scalar(high_hue2, 255, 255), image_Dump);
				image_Green |= image_Dump;
			}
			medianBlur(image_Green, image_Green, 5);
			Canny(image_Green, image_Green, 130, 210, 3);
			circles.clear();
			HoughCircles(image_Green, circles, HOUGH_GRADIENT, 1, 10,
				150, 15, 7, 50);
			if (circles.size() > 0) {
				greenFlag = 1;
				greenCenter = Point(circles[0][0], circles[0][1]);
			}
			else {
				if (redFlag == 1 && yellowFlag == 1 && greenFlag == 1)
				{
					redFlag = 0; yellowFlag = 0; greenFlag = 0;
				}
				greenFlag = 0;
			}
		}
		//////////////////////////////////////
		
		imshow("output", src);
		end = clock();
		system("cls");
		waitKey(1);
		if (redFlag == 1) {
			cout << "정지 x: "<<redCenter.x<<" y: "<< redCenter.y << endl;
		}
		if (yellowFlag == 1)
			cout << "서행 x: " << yellowCenter.x << " y: " << yellowCenter.y << endl;
		if (greenFlag == 1) {
			if((yellowCenter.x - redCenter.x)*1.5 < (greenCenter.x- yellowCenter.x))
				cout << "우회전 x: " << greenCenter.x << " y: " << greenCenter.y << endl;
			else
				cout << "좌회전 x: " << greenCenter.x << " y: " << greenCenter.y << endl;
		}
		cout << "Frame : " << 1.0 / ((float)(end - start) / 1000.0) << endl;
		start = clock();
	}
	ReleaseCapture();
	return 0;
}
void setColorScalar(int &low_hue1, int &high_hue1, int &low_hue2, int &high_hue2, int &range_count, int flag) {
	Scalar red(0, 0, 255);
	Scalar greenCustom(170, 225, 65);
	Scalar yellow(0, 255, 255);
	Mat rgb_color;
	switch (flag)
	{
	case 0:
		rgb_color = Mat(1, 1, CV_8UC3, red);
		break;
	case 1:
		rgb_color = Mat(1, 1, CV_8UC3, greenCustom);
		break;
	case 2:
		rgb_color = Mat(1, 1, CV_8UC3, yellow);
		break;
	default:
		break;
	}
	Mat hsv_color;

	cvtColor(rgb_color, hsv_color, COLOR_BGR2HSV);


	int hue = (int)hsv_color.at<Vec3b>(0, 0)[0];
	int saturation = (int)hsv_color.at<Vec3b>(0, 0)[1];
	int value = (int)hsv_color.at<Vec3b>(0, 0)[2];

	int low_hue = hue - 10;
	int high_hue = hue + 10;


	if (low_hue < 10) {
		range_count = 2;

		high_hue1 = 180;
		low_hue1 = low_hue + 180;
		high_hue2 = high_hue;
		low_hue2 = 0;
	}
	else if (high_hue > 170) {
		range_count = 2;

		high_hue1 = low_hue;
		low_hue1 = 180;
		high_hue2 = high_hue - 180;
		low_hue2 = 0;
	}
	else {
		range_count = 1;

		low_hue1 = low_hue;
		high_hue1 = high_hue;
	}
}
Mat hwnd2mat(HWND hwnd) {

	HDC hwindowDC, hwindowCompatibleDC;

	int height, width, srcheight, srcwidth;
	HBITMAP hbwindow;
	Mat src;
	BITMAPINFOHEADER bi;
	hwindowDC = GetDC(hwnd);
	hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
	SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

	RECT windowsize;
	GetClientRect(hwnd, &windowsize);

	srcheight = windowsize.bottom / 2;
	srcwidth = windowsize.right / 2;
	height = windowsize.bottom / 2;
	width = windowsize.right / 2;

	src.create(height, width, CV_8UC4);

	hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = width;
	bi.biHeight = -height;
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	SelectObject(hwindowCompatibleDC, hbwindow);
	StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, 0, 0, srcwidth, srcheight, SRCCOPY);
	GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO *)&bi, DIB_RGB_COLORS);

	DeleteObject(hbwindow);
	DeleteDC(hwindowCompatibleDC);
	ReleaseDC(hwnd, hwindowDC);

	return src;
}