#include <iostream>
#include <assert.h>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
using namespace std;


bool parseGps(ifstream &gpsFile, string &rmcLine, int &rmcTime){
	std::string line;
	while (getline(gpsFile, line))
	{
		unsigned time, h, mn, s, ms;
		if(sscanf(line.c_str(), "$GPRMC,%u.%u", &time, &ms) != 2) continue;
		ms *= 10;
		s = time % 100; time /= 100;
		mn = time % 100; time /= 100;
		h = time;
		rmcLine = line;
		rmcTime = ms + 1000*(s + 60*(mn + 60*(h)));
		return true;

	}
	return false;
}

int main(int argc, char** argv)
{
    assert(argc == 2);
    string imageDir = string(argv[1]);

    ifstream gpsFile(imageDir + "/gps.txt");
    string gpsRmc;
    int gpsTime;
    assert(parseGps(gpsFile, gpsRmc, gpsTime));

    int i = 1;
    while(true){
    	std::ifstream image(imageDir + "/img_0_" + to_string(i) + ".txt");
    	if(!image.good()) break;
    	int imageTime;
    	while(true){
    		string line;
			getline(image, line);
			istringstream iss(line);

			string date(100, ' '), time(100, ' ');
			unsigned h, mn, s, ms;
			if(sscanf (line.c_str(), "Capture Time CPU: %s %u:%u:%u:%u",&date[0], &h, &mn, &s, &ms) != 5) continue;
			imageTime = ms + 1000*(s + 60*(mn + 60*(h))) - 3600*1000;
			break;
    	}
    	while(gpsTime < imageTime){
    	    if(!parseGps(gpsFile, gpsRmc, gpsTime)){
    	    	cout << "Stoped at image " << i << endl;
    	    	return 0;
    	    }
    	}
    	cout << gpsTime << " " << imageTime << endl;


    	for(int cameraId = 0;cameraId < 2;cameraId++){
    		string path = imageDir + "/img_" + to_string(cameraId) + "_" + to_string(i) + ".txt";
        	std::ifstream ifs(path);
        	string str;
        	while(!ifs.eof()){
        		string line;
    			getline(ifs, line);
    			if(line.rfind("RMC:", 0) != 0 && line.length() != 0){
    				str += line + "\n";
    			}
        	}
        	str += "RMC: " + gpsRmc + "\n";
        	ifs.close();
        	std::ofstream ofs (path, std::ofstream::out);
			ofs << str;
			ofs.close();
    	}

    	i++;
    }

    return 0;
}
