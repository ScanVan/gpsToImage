#include <iostream>
#include <assert.h>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <algorithm>
using namespace std;


class Gga{
public:
	unsigned time;
	double latitude , longitude, altitude;
};

bool parseGps(ifstream &gpsFile, string &ggaLine, Gga &gga){
	std::string line;
	while (getline(gpsFile, line))
	{
		unsigned time, h, mn, s, ms;
		double lat, lon, altitude;
		char latSign, lonSign;
		double dummyD;
		unsigned dummyU;
		char dummy[100];
//		cout << "matches " << sscanf(line.c_str(), "$GPGGA,%u.%u,%lf,%c,%lf,%c,%s,%s,%s,%lf", &time, &ms, &lat, &latSign, &lon, &lonSign, dummy, dummy, dummy, &altitude) << endl;
		if(sscanf(line.c_str(), "$GPGGA,%u.%u,%lf,%c,%lf,%c,%u,%u,%lf,%lf", &time, &ms, &lat, &latSign, &lon, &lonSign, &dummyU, &dummyU, &dummyD, &altitude) != 10) continue;
		ms *= 10;
		s = time % 100; time /= 100;
		mn = time % 100; time /= 100;
		h = time;
		ggaLine = line;

		if(latSign != 'N') lat *= -1;
		if(lonSign != 'E') lon *= -1;
		lat /= 100;
		lon /= 100;

		gga.altitude = altitude;
		gga.longitude = lon;
		gga.latitude = lat;
		gga.time = ms + 1000*(s + 60*(mn + 60*(h)));

		return true;

	}
	return false;
}


string zeroPad(string in, int finalLength){
	string ret = in;
	while(ret.length() < finalLength) ret = string("0") + ret;
	return ret;
}

int main(int argc, char** argv)
{
    assert(argc == 3);
    string imageDir = string(argv[1]);
    string targetDir = string(argv[2]);

    ifstream gpsFile(imageDir + "/gps.txt");
    string gpsGga;
    Gga gga;
    assert(parseGps(gpsFile, gpsGga, gga));

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
    	while(gga.time < imageTime){
    	    if(!parseGps(gpsFile, gpsGga, gga)){
    	    	cout << "Stoped at image " << i << endl;
    	    	return 0;
    	    }
    	}
    	cout << gga.time << " " << imageTime << endl;


    	for(int cameraId = 0;cameraId < 2;cameraId++){
    		string path = imageDir + "/img_" + to_string(cameraId) + "_" + to_string(i) + ".txt";
        	std::ifstream ifs(path);
        	string str;
        	string ofsFileName;
			string date(100, ' ');
			string time(100, ' ');
        	while(!ifs.eof()){
        		string line;
    			getline(ifs, line);
    			if(line.length() != 0){
    				str += line + "\n";
    			}
				if(sscanf(line.c_str(), "Capture Time CPU: %s %s", &date[0], &time[0]) == 2){
					date.erase(remove(date.begin(), date.end(), '-'), date.end());


//					time.erase(remove(time.begin(), time.end(), ':'), time.end());
					ofsFileName = string(date.c_str()) + "-";
					istringstream iss(time);
					string tmp;
					getline(iss, tmp, ':'); ofsFileName += zeroPad(tmp, 2);
					getline(iss, tmp, ':'); ofsFileName += zeroPad(tmp, 2);
					getline(iss, tmp, ':'); ofsFileName += zeroPad(tmp, 2);
					getline(iss, tmp, ':'); ofsFileName += "-" + zeroPad(tmp, 3);
					getline(iss, tmp, ':'); ofsFileName += zeroPad(string(tmp.c_str()),3);
					ofsFileName += ".txt";
				}
        	}
        	str += "GGA: " + gpsGga + "\n";
        	str += "latitude: " + to_string(gga.latitude) + "\n";
        	str += "longitude: " + to_string(gga.longitude) + "\n";
        	str += "altitude: " + to_string(gga.altitude) + "\n";


        	ifs.close();
        	std::ofstream ofs (targetDir + "/" + ofsFileName, std::ofstream::out);
			ofs << str;
			ofs.close();
    	}

    	i++;
    }

    return 0;
}
