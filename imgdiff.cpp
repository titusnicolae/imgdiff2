#include "lodepng.h"
#include <iostream>
#include <cstdio>
#include <iostream>
#include <cstring>
#include <string>
#include <fstream>
#include <utility>
#include <unordered_map>
#include <vector>
const int mul = 359;
const int prime = 10083087720779LL;
using namespace std;


void decodeOneStep(const char* filename)
{
  std::vector<unsigned char> image; //the raw pixels
  unsigned width, height;

  //decode
  unsigned error = lodepng::decode(image, width, height, filename);

  //if there's an error, display it
  if(error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

  //the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...
}

void encodeOneStep(const char* filename, std::vector<unsigned char>& image, unsigned width, unsigned height)
{
  //Encode the image
  unsigned error = lodepng::encode(filename, image, width, height);

  //if there's an error, display it
  if(error) std::cout << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
}

char blob[]="P6\n%d %d\n255\n";

struct Point
{
    int x,y;
    Point(int x,int y):x(x),y(y){}
    Point(){}
};

class PNG {

    public:
    string filename;
    unsigned w, h;
    std::vector<unsigned char> data; //the raw pixels
    unordered_map<long long int, vector<Point>> asd;

    PNG(int w, int h, std::vector<unsigned char> data, string filename): w(w), h(h), filename(filename) {
        this->data = data;
    }

    PNG(string filename) {
        this->filename = filename;
        unsigned error = lodepng::decode(data, this->w, this->h, filename.c_str());
        if(error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    }

    /*
    private:
    void getsize()
    {
        FILE *fin=fopen(file, "rb");
        char crap[10];
        int crap255;
        fscanf(fin, "%s%d%d%d", crap, &w, &h, &crap255);
        fseek(fin, 1, SEEK_CUR); 

        data = new char[bytes()];
        fread(data, 1, bytes(), fin);
        fclose(fin);
    }
    */

    public:
    int bytes()
    {
        return w * h *4;
    }

    void aa(int x, int y)
    {
        int pos = (w * y + x) * 4;

        int red =   (unsigned short)(data[pos+0]&0xFF);
        int green = (unsigned short)(data[pos+1]&0xFF);
        int blue =  (unsigned short)(data[pos+2]&0xFF);
        cout<<red<<" "<<green<<" "<<blue<<endl;
    }
    unsigned char& red(int x, int y)
    {
        return data[(y*w+x)*4+0];
    }
    unsigned char& green(int x, int y)
    {
        return data[(y*w+x)*4+1];
    }
    unsigned char& blue(int x, int y)
    {
        return data[(y*w+x)*4+2];
    }
    int rgb(int x, int y)
    {
        return data[(y*w+x)*4]<<16 + data[(y*w+x)*4+1]<<8 + data[(y*w+x)*4+2];
    }

//    GBR

    void save()
    {
        encodeOneStep(filename.c_str(), data, w, h);
    }
    void hash()
    {
        int block_size = 8;

        for(int x=0;x<w-block_size;x++)
        {
            for(int y=0;y<h-block_size;y++)
            {
                long long int crc = 0;
                for(int i=0;i<block_size;i++)
                {
                    for(int j=0;j<block_size;j++)
                    {
                        crc = ((crc*mul) + rgb(x+i, y+j)) % prime;
                    }
                }
                asd[crc].push_back(Point(x,y));
            }
        }
        cout<<(w-block_size)*(h-block_size)<<" "<<asd.size()<<endl;

    }
};

int main (int argc, char *argv[])
{
    if(argc<3)
    {
        cout<<"missing argument";
        return 0;
    }

    string file1(argv[1]);
    string file2(argv[2]);

    if(file1.size()>4 and file1.substr(file1.size()-4)!=".png" and
       file2.size()>4 and file2.substr(file2.size()-4)!=".png")
    {
        cout<<"File type not png";
        return 0;
    }

    PNG png1 = PNG(argv[1]);
    png1.hash();
    PNG png2 = PNG(argv[2]);
    png2.hash();
    PNG diff = PNG(png1.w, png1.h, png1.data, "diff.png");
    vector<Point> missing;
    for(auto& e: png1.asd)
    {
        if(png2.asd.find(e.first) == png2.asd.end())
        {
            for(auto& f: e.second)
            {
                missing.push_back(f);
            }
        }
    }
    for(auto& e: missing)
    {
        for(int i=0;i<8;i++)
        {
            for(int j=0;j<8;j++)
            {
                diff.red(e.x+i,e.y+j) = 255;
            }
        }
    }
    diff.save();

    return 0;
}
