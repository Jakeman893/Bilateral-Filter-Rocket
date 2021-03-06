#include "stdafx.h"
#include <chrono>

using cv::Mat;
using cv::imread;
using std::cout;
using std::endl;

std::string path;

int main(int argc, char **argv)
{
	cout << "Please enter the path to the file without quotes or hit enter to use test image." << endl;
	std::getline(std::cin,path);

	if (path.empty())
		path = "../Test Images/14-2MP/test4.jpg";

	cout << "Choose what type of image to load:\n\t1: Color\n\t2: Grayscale" << endl;

#ifndef _DEBUG
	int selection, mode, width;
	float sigma_d, sigma_r;
	std::cin >> selection;
	cout << "Enter Kernel Width:" << endl;
	std::cin >> width;
	cout << "Enter Sigma d:" << endl;
	std::cin >> sigma_d;
	cout << "Enter Sigma r:" << endl;
	std::cin >> sigma_r;

	switch (selection)
	{
	case 2: mode = CV_LOAD_IMAGE_GRAYSCALE; break;
	case 1: 
	default: mode = CV_LOAD_IMAGE_ANYCOLOR | CV_LOAD_IMAGE_ANYDEPTH;
	}

	cout << "Creating Bilateral Filter...\n";
	BilateralFilter *bf = new BilateralFilter(width, sigma_d, sigma_r);
#else
//	int mode = CV_LOAD_IMAGE_GRAYSCALE;
	int mode = CV_LOAD_IMAGE_ANYCOLOR | CV_LOAD_IMAGE_ANYDEPTH;
	cout << "Creating Bilateral Filter...\n";
	BilateralFilter *bf = new BilateralFilter();
#endif

	cout << "Reading in image...\n";
	Mat input = imread(path, mode);
	if (input.empty())
		throw std::exception("The image could not be loaded successfully.");
	cout << "Displaying input image...\n";
	cv::namedWindow("Input", CV_WINDOW_NORMAL);
	cv::imshow("Input",input);
	cv::waitKey(0);

	cout << "Converting to float matrix...\n";
	// Converts the image to float with a domain of [0,1]
	input.convertTo(input, CV_32FC3, 1/255.0);

	cout << "Applying filter with CUDA...";
	auto start = std::chrono::high_resolution_clock::now();
	Mat cudaOut = bf->ApplyFilterCUDA(input.clone());
	auto total = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
	cout << "Finished in " << total.count() << " ms.\n";

	cout << "Displaying output image...\n";
	cv::namedWindow("Output CUDA", CV_WINDOW_AUTOSIZE);
	cv::imshow("Output CUDA", cudaOut);
	cv::waitKey(0);

	start = std::chrono::high_resolution_clock::now();
	cout << "Applying filter serially...";
	Mat out = bf->ApplyFilter(input.clone());
	total = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
	cout << "Finished in " << total.count() << " ms.\n";

	cout << "Displaying output image...\n";
	cv::namedWindow("Output Serial", CV_WINDOW_NORMAL);
	cv::imshow("Output Serial", out);
	cv::waitKey(0);

	delete bf;
	cudaOut.deallocate();
	//out.deallocate();
	input.deallocate();
}