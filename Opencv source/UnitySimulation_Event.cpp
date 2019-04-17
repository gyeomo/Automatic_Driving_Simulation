#include <opencv2/opencv.hpp>
#include <iostream>
#include <Windows.h>
#include <thread>
#include <atomic>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using namespace std;
Mat hwnd2mat(HWND hwnd);

Mat frame, treatImage, original;
Mutex mutex_In, mutex_Treat;
atomic<int> flag = 0;

thread myThread1, myThread2, myThread3;
HWND hwndDesktop;

#define RHO   0
#define THETA 1
const float DEGREE1 = CV_PI * 1 / 180;
atomic<int> counter = 0;
atomic<double> slopeSum = 0;
atomic<double> angleSum = 0;
atomic<int> straightFlag = 0;
atomic<double> straightAngle = 0;
atomic<int> interrupt = 0;
void Detect_Draw_Line(Mat* originImg, Mat* ROIimg, float* resultLine, Point* lineP);
void Draw_Line(Mat* originImg, float* resultLine, Point* lineP, vector<Vec2f> lines);
void setColorScalar(int &low_hue1, int &high_hue1, int &low_hue2, int &high_hue2, int &range_count, int flag);
Scalar meanV, stdDev;
int main(int, char**)
{
	Sleep(3000);
	hwndDesktop = GetDesktopWindow();
	myThread1 = thread([]() {
		Mat temp;
		for (;;)
		{
			temp = hwnd2mat(hwndDesktop);
			if (temp.empty())
				continue;
			if (flag == 1) {
				break;
			}
			resize(temp, temp, Size(320, 240));
			mutex_In.lock();
			frame = temp;
			mutex_In.unlock();
		}

	});
	myThread2 = thread([]() {
		Mat grayImage, currentFrame, edgeImage;
		Mat image_ROI;
		Mat image_Yellow, image_Red;
		Mat image_Dump;
		Mat image_HSV;
		int low_hue1 = 0, low_hue2 = 0;
		int high_hue1 = 0, high_hue2 = 0;
		int range_count = 0;
		int interruptFlag1 = 0, interruptFlag2 = 0;
		Scalar meanValues, stdDevValues;
		for (;;)
		{
			if (flag == 1) {
				break;
			}
			mutex_In.lock();
			currentFrame = frame;
			mutex_In.unlock();
			if (currentFrame.empty())
				continue;
			//////////////////////////////////////////	
			cvtColor(currentFrame, image_HSV, CV_BGR2HSV);
			low_hue1 = 0; low_hue2 = 0;
			high_hue1 = 0; high_hue2 = 0;
			range_count = 0;
			setColorScalar(low_hue1, high_hue1, low_hue2, high_hue2, range_count, 2);
			inRange(image_HSV, Scalar(low_hue1, 50, 127), Scalar(high_hue1, 255, 255), image_Yellow);
			if (range_count == 2) {
				inRange(image_HSV, Scalar(low_hue2, 50, 127), Scalar(high_hue2, 255, 255), image_Dump);
				image_Yellow |= image_Dump;
			}
			///////////////////////////////////////////우선정지장애물
			if (interruptFlag2 == 0) {
				low_hue1 = 0; low_hue2 = 0;
				high_hue1 = 0; high_hue2 = 0;
				range_count = 0;
				setColorScalar(low_hue1, high_hue1, low_hue2, high_hue2, range_count, 0);
				inRange(image_HSV, Scalar(low_hue1, 50, 127), Scalar(high_hue1, 255, 255), image_Red);
				if (range_count == 2) {
					inRange(image_HSV, Scalar(low_hue2, 50, 127), Scalar(high_hue2, 255, 255), image_Dump);
					image_Red |= image_Dump;
				}
				meanStdDev(image_Red, meanValues, stdDevValues);
				if (meanValues[0] > 5 && interruptFlag1 == 0) {
					interrupt = 1;
					interruptFlag1 = 1;
				}
				else if (meanValues[0] < 5 && interruptFlag1 == 1) {
					keybd_event(VK_DOWN, MapVirtualKey(VK_DOWN, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
					interrupt = 0;
					meanValues = 0;
					stdDevValues = 0;
					interruptFlag2 = 1;
				}
			}
			////////////////////////////////////////////터널
			if (interruptFlag2 == 1) {
				cvtColor(currentFrame, grayImage, COLOR_BGR2GRAY);
				meanStdDev(grayImage, meanValues, stdDevValues);
				if (meanValues[0] < 100 && stdDevValues[0] < 100) {
					interrupt = 1;
				}
				else if(interrupt == 1){
					interrupt = 0;
					keybd_event(VK_DOWN, MapVirtualKey(VK_DOWN, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
					interruptFlag2 = 2;
				}
			}
			///////////////////////////////////////////
			Canny(image_Yellow, edgeImage, 130, 210, 3);
			image_ROI = edgeImage(Rect(0, edgeImage.rows / 2, edgeImage.cols, edgeImage.rows / 2));
			if (mutex_Treat.trylock()) {
				treatImage = image_ROI;
				original = currentFrame;
				mutex_Treat.unlock();
			}
		}
	});
	myThread3 = thread([]() {
		clock_t start, end;
		start = clock();
		vector<Vec2f> lines;
		float resultLine[2];
		Point lineP[2];
		int windowFlag1 = 0;
		Mat temp, houghImage, inputImage;
		int timer1 = 0, timer2 = 0;
		int curveFlag1 = 0, curveFlag2 = 0;
		double averageS = 0, averageA = 0;
		for (;;)
		{
			if (mutex_Treat.trylock()) {
				temp = treatImage;
				inputImage = original;
				mutex_Treat.unlock();
			}
			if (temp.empty())
				continue;
			houghImage = temp;
			Detect_Draw_Line(&inputImage, &houghImage, resultLine, lineP);
			averageS = 0; averageA = 0;
			if (counter > 0) {
				averageS = slopeSum / (double)counter;
				averageA = angleSum / (double)counter;
			}
			slopeSum = 0; angleSum = 0; counter = 0;
			imshow("output", inputImage);
			system("cls");
		//	cout << "평균 각도 : " << averageA << endl;
		//	cout << "평균 기울기 : " << averageS << endl;
			//////////////////////////////////////////////////
			if (interrupt == 0) {
				if (windowFlag1 == 0)
					keybd_event(VK_UP, MapVirtualKey(VK_UP, 0), KEYEVENTF_EXTENDEDKEY, 0);
				if (straightFlag == 1) {
					if ((straightAngle > -2 && straightAngle < 2) && averageA > -11 && averageA < 11) {
						keybd_event(VK_RIGHT, MapVirtualKey(VK_RIGHT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
						keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
					}
					else if (straightAngle < -6 || straightAngle > 6) {
						if (averageA < -15) {
							keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
							keybd_event(VK_RIGHT, MapVirtualKey(VK_RIGHT, 0), KEYEVENTF_EXTENDEDKEY, 0);
						}
						else if (averageA > 15) {
							keybd_event(VK_RIGHT, MapVirtualKey(VK_RIGHT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
							keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, 0), KEYEVENTF_EXTENDEDKEY, 0);
						}
						else if (straightAngle > 0) {
							if (curveFlag2 == 0) {
								keybd_event(VK_RIGHT, MapVirtualKey(VK_RIGHT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
								keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, 0), KEYEVENTF_EXTENDEDKEY, 0);
								curveFlag2 = 1;
							}
							else {
								keybd_event(VK_RIGHT, MapVirtualKey(VK_RIGHT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
								keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
								curveFlag2 = 0;
							}
						}
						else {
							if (curveFlag2 == 0) {
								keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
								keybd_event(VK_RIGHT, MapVirtualKey(VK_RIGHT, 0), KEYEVENTF_EXTENDEDKEY, 0);
								curveFlag2 = 1;
							}
							else {
								keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
								keybd_event(VK_RIGHT, MapVirtualKey(VK_RIGHT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
								curveFlag2 = 0;
							}
						}
					}
					else if (averageA < -11) {
						if (timer1 == 1) {
							timer1 = 0;
							keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
							keybd_event(VK_RIGHT, MapVirtualKey(VK_RIGHT, 0), KEYEVENTF_EXTENDEDKEY, 0);
						}
						else {
							timer1 = 1;
							keybd_event(VK_RIGHT, MapVirtualKey(VK_RIGHT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
							keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
						}
					}
					else if (averageA > 11) {
						if (timer2 == 1) {
							timer2 = 0;
							keybd_event(VK_RIGHT, MapVirtualKey(VK_RIGHT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
							keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, 0), KEYEVENTF_EXTENDEDKEY, 0);
						}
						else {
							timer2 = 1;
							keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
							keybd_event(VK_RIGHT, MapVirtualKey(VK_RIGHT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
						}
					}
					else if (averageA < -4) {
						timer1 = timer1 + 1;
						if (timer1 == 4)
							timer1 = 0;
						if (timer1 <= 1) {
							timer2 = 0;
							keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
							keybd_event(VK_RIGHT, MapVirtualKey(VK_RIGHT, 0), KEYEVENTF_EXTENDEDKEY, 0);
						}
						else {
							keybd_event(VK_RIGHT, MapVirtualKey(VK_RIGHT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
							keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
						}
					}
					else if (averageA > 4) {
						timer2 = timer2 + 1;
						if (timer2 == 4)
							timer2 = 0;
						if (timer2 <= 1) {
							timer1 = 0;
							keybd_event(VK_RIGHT, MapVirtualKey(VK_RIGHT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
							keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, 0), KEYEVENTF_EXTENDEDKEY, 0);
						}
						else {
							keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
							keybd_event(VK_RIGHT, MapVirtualKey(VK_RIGHT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
						}
					}
					else {
						keybd_event(VK_RIGHT, MapVirtualKey(VK_RIGHT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
						keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
					}
				}
				else {
					if (averageA < -50) {
						keybd_event(VK_RIGHT, MapVirtualKey(VK_RIGHT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
						keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, 0), KEYEVENTF_EXTENDEDKEY, 0);
					}
					else if (averageA > 50) {
						keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
						keybd_event(VK_RIGHT, MapVirtualKey(VK_RIGHT, 0), KEYEVENTF_EXTENDEDKEY, 0);
					}
					else {
						if (averageA < -30) {
							if (curveFlag1 == 0) {
								keybd_event(VK_RIGHT, MapVirtualKey(VK_RIGHT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
								keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, 0), KEYEVENTF_EXTENDEDKEY, 0);
								curveFlag1 = 1;
							}
							else {
								keybd_event(VK_RIGHT, MapVirtualKey(VK_RIGHT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
								keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
								curveFlag1 = 0;
							}
						}
						else if (averageA > 30) {
							if (curveFlag1 == 0) {
								keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
								keybd_event(VK_RIGHT, MapVirtualKey(VK_RIGHT, 0), KEYEVENTF_EXTENDEDKEY, 0);
								curveFlag1 = 1;
							}
							else {
								keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
								keybd_event(VK_RIGHT, MapVirtualKey(VK_RIGHT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
								curveFlag1 = 0;
							}
						}
						else {
							keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
							keybd_event(VK_RIGHT, MapVirtualKey(VK_RIGHT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
						}

					}
				}
			}
			else {
				keybd_event(VK_RIGHT, MapVirtualKey(VK_RIGHT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
				keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
				keybd_event(VK_UP, MapVirtualKey(VK_UP, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
				keybd_event(VK_DOWN, MapVirtualKey(VK_DOWN, 0), KEYEVENTF_EXTENDEDKEY, 0);
			}
			//////////////////////////////////////////////////
			end = clock();
			cout << "Frame : " << 1.0 / ((float)(end - start) / 1000.0) << endl;
			start = clock();
			if (waitKey(20) == 27) {
				flag = 1;
				break;
			}
		}
	});

	myThread2.join();
	myThread1.join();
	myThread3.join();
	ReleaseCapture();
	keybd_event(VK_RIGHT, MapVirtualKey(VK_RIGHT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	keybd_event(VK_UP, MapVirtualKey(VK_UP, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	return 0;
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

void Detect_Draw_Line(Mat* originImg, Mat* ROIimg, float* resultLine, Point* lineP)
{
	vector<Vec2f> lines;
	int leftFlag = 0, rightFlag = 0;
	Point leftP[2], rightP[2], banishP;
	HoughLines(*ROIimg, lines, 1, CV_PI / 180, 50, 0, 0, 80.0*CV_PI / 180.0, 90.0*CV_PI / 180.0);//왼쪽커브
	if (lines.size() == 0)
		HoughLines(*ROIimg, lines, 1, CV_PI / 180, 50, 0, 0, 90.0*CV_PI / 180.0, 100.0*CV_PI / 180.0);//오른쪽커브
	if (lines.size() == 0) {
		HoughLines(*ROIimg, lines, 1, CV_PI / 180, 50, 0, 0, 0, 80.0*CV_PI / 180.0);//왼쪽직진
		if (lines.size() > 0)
		{
			Draw_Line(originImg, resultLine, lineP, lines);
			leftFlag = 1;
			if (leftFlag == 1) {
				leftP[0] = lineP[0];
				leftP[1] = lineP[1];
			}
			lines.clear();

		}
		HoughLines(*ROIimg, lines, 1, CV_PI / 180, 50, 0, 0, 100.0*CV_PI / 180.0, CV_PI);//오른쪽직진
		if (lines.size() > 0)
		{
			Draw_Line(originImg, resultLine, lineP, lines);
			rightFlag = 1;
			if (rightFlag == 1) {
				rightP[0] = lineP[0];
				rightP[1] = lineP[1];
			}
			lines.clear();
		}
	}
	if (lines.size() > 0)
	{
		Draw_Line(originImg, resultLine, lineP, lines);
		lines.clear();
	}
	//////////////////////////////////////////
	if (rightFlag == 1 && leftFlag == 1) {
		straightFlag = 1;
		float leftLineA = (float)(leftP[1].y - leftP[0].y) / (float)(leftP[1].x - leftP[0].x);
		float leftLineB = leftP[1].y - leftLineA * leftP[1].x;

		// 오른쪽 차선의 1차 방정식
		float rightLineA = (float)(rightP[1].y - rightP[0].y) / (float)(rightP[1].x - rightP[0].x);
		float rightLineB = rightP[1].y - rightLineA * rightP[1].x;
		float rows = originImg->rows / 2;
		// 1차 소실점: 좌우 1차선의 교점
		banishP.x = (int)((rightLineB - leftLineB) / (leftLineA - rightLineA));
		banishP.y = (int)(leftLineA * banishP.x + leftLineB) + rows;

		straightAngle = 90 - cvFastArctan(((double)banishP.y - (double)originImg->rows), ((double)banishP.x - (double)originImg->cols / 2));
		if (straightAngle < 0)
			straightAngle = straightAngle + 180;
		//	cout << "소실점 기울기 : "<<straightAngle<<endl;
		line(*originImg, Point(originImg->cols / 2, originImg->rows), Point(banishP.x, banishP.y), Scalar(0, 0, 255), 2);
		line(*originImg, Point(originImg->cols / 2, originImg->rows), Point(originImg->cols / 2, 0), Scalar(255, 0, 0), 1);
	}
	else
		straightFlag = 0;
	//////////////////////////////////////////
}

void Draw_Line(Mat* originImg, float* resultLine, Point* lineP, vector<Vec2f> lines) {
	// Theta가 가장 큰 선 1개만 검출
	resultLine[RHO] = lines[0][0];
	resultLine[THETA] = lines[0][1];

	float rho = resultLine[RHO];
	float theta = resultLine[THETA];
	double a = cos(theta), b = sin(theta);
	double x0 = a * rho, y0 = b * rho;
	float rows = originImg->rows;
	float cols = originImg->cols;
	lineP[0].x = cvRound(x0 + (rows + cols) * (-b));
	lineP[0].y = cvRound(y0 + (rows + cols) * (a));
	lineP[1].x = cvRound(x0 - (rows + cols) * (-b));
	lineP[1].y = cvRound(y0 - (rows + cols) * (a));
	Point pt1(lineP[0].x, lineP[0].y + rows / 2);
	Point pt2(lineP[1].x, lineP[1].y + rows / 2);
	line(*originImg, pt1, pt2, Scalar(0, 255, 0), 2);
	double slope = (((double)lineP[0].y - (double)lineP[1].y) / ((double)lineP[1].x - (double)lineP[0].x));
	double angle = 90 - cvFastArctan(((double)lineP[0].y - (double)lineP[1].y), ((double)lineP[1].x - (double)lineP[0].x));
	if (angle < 0)
		angle += 180;
	counter = counter + 1;
	slopeSum = slopeSum + slope;
	angleSum = angleSum + angle;
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