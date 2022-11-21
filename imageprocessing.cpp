#include <iostream>
#include <sstream>
#include <vector>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <chrono>
#include <omp.h>


using namespace std;

/*This programs opens a pgm image, reads it, processes it as desired, and writes back its processed pixel values as a pgm image.



/* 3 by 3 edge detection convolution filter(kernel); can change its values for other various convolution purposes such as blurring. sharpening, etc.*/

double edgeDetection[3][3]={-0.0625,-0.0625,-0.0625,-0.0625,0.5,-0.0625,-0.0625,-0.0625,-0.0625};

// Function that implements the application of the filter on every pixel of the image
double applyConvolution(std::vector<std::vector<int>>& matrix, size_t& x, size_t& y) {
  double result = 0;

  for (int i=0; i<3; i++) {
    for (int j=0; j<3; j++) {
      result += edgeDetection[i][j] * matrix[x - 1 + i][y - 1 + j];
    }
  }

  if (result < 0) return 0;
  return result;
}

// Function to write out the final image pixels after processing to a pgm file
bool writeResults(std::string& header, std::string& result) {
  ofstream writeFile;
  writeFile.open ("./imageprocessed.pgm");

  writeFile << header.c_str() << "\n";
  writeFile << result.c_str() << "\n";

  writeFile.close();}


//Function to read the pixels of the original image and apply zero padding in preparation for convolution
  std::vector<std::vector<int>> fillPixelMatrix(int& numColumns, std::ifstream& image) {
  int i = 0;
  int j = 0;

  std::vector<std::vector<int>> pixels;
  std::vector<int> row;
  std::vector<int> padding(numColumns + 2, 0);

  row.push_back(0);
  pixels.push_back(padding);

  std::string line;
  while (getline(image, line)) {
    std::stringstream linestream(line);
    string pixel;
    linestream >> pixel;

    try
    {
      row.push_back(stoi(pixel));
    }
    catch(const std::exception& e)
    {
      std::cerr << e.what() << '\n';
      continue;
    }
  
    j++;
    if (j >= numColumns) {
      j = 0;
      i++;

      row.push_back(0);
      pixels.push_back(row);
      row.clear();
      row.push_back(0);
    }
  }

   pixels.push_back(padding);

  return pixels;
}

int main(int argc, char *argv[]) {
    string imagepath;
    std::cout << "Enter image path" << std::endl;
    cin >> imagepath;

  std::ifstream image;
  image.open(imagepath);

  if(!image.is_open())
  {
    cout<<"Unable to open file."<<endl;
    return 0;
  }

  string header;

  string magic, comment, dimensions, maximumgrey, intensity;
  getline(image, magic);
  getline(image, comment);
  getline(image, dimensions);
  getline(image, maximumgrey);

  header += magic + "\n" + comment + "\n" + dimensions + "\n" + maximumgrey + "\n";

  int space = dimensions.find_first_of(" ");

  string width = dimensions.substr(0, space);
  string height = dimensions.substr(space + 1);

  int columns = stoi(width);
  int rows = stoi(height);

  std::cout <<"Image Dimensions: " << dimensions <<std::endl;
  
  std::cout <<"Dimensions rows "<< rows <<std::endl;
  std::cout <<"Dimensions columns "<< columns <<std::endl;

  auto startReading = std::chrono::high_resolution_clock::now();

  std::vector<std::vector<int>> pixels = fillPixelMatrix(columns, image);
 
  image.close();

  auto stopReading = std::chrono::high_resolution_clock::now();
  auto totalTimeReading = std::chrono::duration_cast<std::chrono::nanoseconds>(stopReading - startReading);
  double totalReadingms = totalTimeReading.count()/1e+6;

	cout << "Total time reading image:  " << totalTimeReading.count() << " -> " <<totalReadingms << " ms" << endl;
	
// after opening image and reading pixels, ask user for what they'd like to do with it
 	
  string iprocessing_type;
  cout << "Please enter the desired type of image processing:" << endl;
  cin >> iprocessing_type;
  
  for (int i = 0; i < iprocessing_type.length(); i++)
  	{
  		iprocessing_type[i] = tolower(iprocessing_type[i]);
  	}
  	

  string results;

  vector<vector<int>> result(rows, vector<int>(columns));
  
  
  auto startProcessing = std::chrono::high_resolution_clock::now();

  for(std::vector<std::vector<int>>::size_type x = 1; x != rows + 1; x++) {
    std::vector<int> row;
    
    #pragma omp parallel for
    for(std::vector<std::vector<int>>::size_type y = 1; y != columns + 1; y++) {
        if (iprocessing_type == "edgedetection"){

            // Convolution; the same can be duplicated for other types of convolutions
            int r = round(applyConvolution(pixels, x, y));

        result[x-1][y-1] = r;
        }
       // Brightening; darkening can be similarly applied by multiplying the pixes by a number less than 1
          if (iprocessing_type == "brighten"){
             int r = round((pixels[x-1][y-1])*1.5);
             
              // Max pixel values in pgm files is 255
               if (r > 255 ){
         result[x-1][y-1] = 255;
         }
                   else{ result[x-1][y-1] = r;}
            }
            
   //Rotating by 180 degrees; other rotation angles can be achieved in a similar way by switching around the pixel values appropriately
         if (iprocessing_type == "rotate"){
           int r =  pixels[rows-x][columns-y];
            result[x-1][y-1] = r;
           }
           
           
           //Vertical reflection; horizontal can be achieved similarly by subtracting from the rows pixels
         if (iprocessing_type == "reflect"){
          int r =  pixels[x][columns-y];
            result[x-1][y-1] = r;
           }
         }
    }
  
  
  
    
  
       
  cout << "Image Processed!"<< endl;
  auto stopProcessing = std::chrono::high_resolution_clock::now();
  auto totalTimeProcessing = std::chrono::duration_cast<std::chrono::nanoseconds>(stopProcessing - startProcessing);

  double totalms = totalTimeProcessing.count()/1e+6;
        
	cout << "Total time processing image:  " << totalTimeProcessing.count() << " -> " << totalms << " ms" << endl;
 
 
//preprocessing the values appropriately before applying the write function defined above	
  int maxGreyValue = 0;  
  for(std::vector<int>r : result) {
    for(int pixel: r) {
      if (pixel > maxGreyValue) {
         maxGreyValue= pixel;
      }
      results += to_string(pixel) + "\n";
    }
  }
  

  stringstream ss;

  ss << maxGreyValue;

  string strMGV = ss.str();
  header = magic + "\n" + comment + "\n" + dimensions + "\n" + strMGV + "\n";

  writeResults(header, results);
} 



