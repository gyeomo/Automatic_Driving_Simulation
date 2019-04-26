# Automatic_Driving_Simulation


This is a simulated autonomous vehicle.

It imitates controlling the movement after openCV recognizes the lane and calculates in what direction to move it.

The car was implemented as Unity. You can control the movement of the car with the arrow keys on the keyboard.

Autonomous navigation was created using openCV in C ++. It works in Windows environment.

First, create a project that includes the opencv library in visual studio. Then type UnitySimulation_Event.cpp.

This code captures a quarter of the entire window. It is 1/4 in the upper left.

And controls the keyboard with virtual key codes.

Recognize the yellowish line on the screen to calculate the slope and determine whether it is left or right. The direction key is controlled through the discriminated value.

Run the unity car project and place it at 1/4 of the top left of the full screen.

Run the project and run the openCV project. You should then click on the unity program immediately.

You should be careful because you recognize it and control the keyboard.

You can quit the console of visual studio.



"`
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






