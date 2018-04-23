#include<algorithm>
#include<Windows.h>
#include<stdlib.h>
#include<iostream>
#include<iterator>
#include<fstream>
#include<sstream>
#include<conio.h>
#include<stdio.h>
#include<cctype>
#include<string>
#include<vector>
#include<set>
#include<utility>
#include<map>
#include<cmath>

//////////////////////////////////////////////////////////////////////////////////
// VIDEO
//////////////////////////////////////////////////////////////////////////////////
struct Video
{
	Video()
	{
		title = "";
	}
	Video(std::string t, std::string f, std::string u)
	{
		title    = t;
		url      = u;
		filename = f;
	}
	std::set<std::pair<std::string, int>>  clipSecondMark;
	std::vector<std::string>               lines;
	std::set<std::string>                  clips;
	std::string                            shellFile;
	std::string                            filename;
	std::string                            title;
	std::string                            url;
	std::string                            id;

};
//////////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////////

std::vector<std::string> cleanFiles;
std::string              sed = "sed -i 's/\\(<\\/c>\\)/ /g;s/\\(<\\)/ /g;s/\\(-->\\)/ /g;s/\\(align.*%\\)/ /g;s/\\(c\\..*>\\)/ /g;s/\\(c>\\)/ /g;s/ /\\n/g;s/\\n/ /g;s/0.*>/ /g' ";
std::string              screen;
int                      mention = 2;
char                     selection;
char                     response;
bool                     sameVid = false;
std::string              search;

//////////////////////////////////////////////////////////////////////////////////
// FUNCTION DECLARATIONS
//////////////////////////////////////////////////////////////////////////////////

std::string getFileContents  (std::string                                   );
void exportTimestamps        (std::vector<Video>&                           );
void redrawLoading           (                                              );
void cleanupFiles            (std::vector<Video>&                           );
void getFileNames            (std::vector<Video>&, std::string              );
void launchUrls              (std::vector<Video>&, std::vector<std::string> );
void saveToClip              (std::set<std::string>&                        );
void getClips                (Video&, int                                   );
void getTime                 (std::vector<std::string>&, std::vector<Video>&);
void redraw                  (std::string                                   );
void redraw                  (                                              );

//////////////////////////////////////////////////////////////////////////////////
// MAIN
//////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
	std::vector<std::string>  keywords;
	std::vector<Video>        videos;
	std::string               firstUrl;
	std::string               word;
	std::string               sCmdA    = "youtube-dl --write-auto-sub --skip-download ytsearch";
	std::string               sCmdF    = "youtube-dl --skip-download --get-filename --get-title --get-id ytsearch";
	std::string               vCmdA    = "youtube-dl --write-auto-sub --skip-download ";
	std::string               vCmdE    = "youtube-dl --skip-download --write-sub --sub-lang en ";
	std::string               sCmdE    = "youtube-dl --sub-lang en --skip-download ytsearch";
	std::string               vid      = "-video";
	std::string               play     = "-playlist";
	std::string               log      = " > logFile.txt";
	char                      response = 'q';
	char                      check;

	//push keywords into vector
	do {
		if (response != 'q')
			keywords.erase(keywords.begin(), keywords.end());

		redraw();

		if (sameVid == false) {
			screen = "";
			if (response != 'q')
				videos.erase(videos.begin(), videos.end());
			do {
				std::cout << "\n\tSEARCH:";
				std::cout << "\n\t\tS - search";
				std::cout << "\n\t\tV - video";
				selection = _getch();
			} while (selection != 's' && selection != 'S' && selection != 'v' && selection != 'V');
		}
		if (selection == 's' || selection == 'S') {
			std::cout << "\nSearch how many videos?: ";
			std::cin >> search;
			search += ":\"";
		}

		redraw();

		if (sameVid == false && (selection == 'v' || selection == 'V')) {
			std::cout << "\n\tENTER YOUTUBE URL: "; std::cin >> firstUrl;
			std::cout << "\n\tDOWNLOAD VIDEO? (Y/N): "; response = _getch();
			//video class are initialized in this function
			redrawLoading();
			getFileNames(videos, firstUrl);
			screen = "\n\tVIDEO: ";

			for (const Video& vid : videos)
				screen += vid.title + '\n';
		}
		// add search term to command argument
		if (sameVid == false && (selection == 's' || selection == 'S')) {
			std::string tmpSearch;
			std::cout << "\nEnter search term: ";
			std::cin.ignore();
			std::getline(std::cin, tmpSearch);
			redrawLoading();
			sCmdA  += search + tmpSearch + "\"";
			sCmdE  += search + tmpSearch + "\"";
			sCmdF  += search + tmpSearch + "\"";
			getFileNames(videos, sCmdF);
			redraw(screen);
		}
		redraw(screen);

		if (sameVid == false) {
			if (selection == 'v' || selection == 'V') {
				videos[0].url   = firstUrl;
				std::string cmd = vCmdA + firstUrl;
				system((const char*)cmd.c_str());
				cmd = vCmdE + firstUrl;
				system((const char*)cmd.c_str());
				if (response == 'y' || response == 'Y') {
					cmd = "youtube-dl " + firstUrl;
					system((const char*)cmd.c_str());
				}
			}
			if (selection == 'S' || selection == 's') {				
				system((const char*)sCmdA.c_str());
				system((const char*)sCmdE.c_str());
			}
			if (sameVid == false)
				cleanupFiles(videos);
		}

		redraw();

		if (sameVid == false) {
			std::cout << "\n\tWhat word do you want to search?: ";
			std::cin >> word;
			keywords.push_back(word);
			redraw(screen); //
			std::cout << "\n\n";
		}

		if (sameVid == true) {
			redraw(screen);
			for (Video& v : videos)
				v.clips.erase(v.clips.begin(), v.clips.end());
			std::cout << "\n\tWhat word do you want to search?: "; std::cin >> word;
			keywords.push_back(word);

		}

		getTime(keywords, videos);
		launchUrls(videos, keywords);

		std::cout << "\n\tDo you want to search another video? (Y/N)";
		response = _getch();

		if (response != 'y' && response != 'Y') {
			std::cout << "\n\tDo you want to search the same video for a different word? (Y/N): ";
			response = _getch();
			if (response == 'y' || response == 'Y')
				sameVid = true;
		}
		else sameVid = false;
	} while (response == 'y' || response == 'Y');

	system("PAUSE");
	return 0;
}
//////////////////////////////////////////////////////////////////////////////////
void getFileNames(std::vector<Video>& vids, std::string fileCommand)
{
	std::ifstream  inFile;
	std::ifstream  urlFile;
	std::string    ttmp;
	std::string    utmp;
	std::string    tmp;
	std::string    cmd;


	//load in .vtt file titles
	if (selection == 'v' || selection == 'V')
		cmd = "youtube-dl --get-filename \"" + fileCommand + "\" > filenames.txt";
	if (selection == 's' || selection == 'S')
		cmd = fileCommand + " > filenames.txt";
	system((const char*)cmd.c_str());

	//cmd = firstUrl + "> urlnames.txt";
	//system((const char*)cmd.c_str());

	inFile.open("filenames.txt");
	//urlFile.open("urlnames.txt");

	//INITIALIZE VIDEO CLASSES
	int itmp = 0;
	do {
		getline(inFile, tmp);

		//getline(urlFile, utmp);
		std::istringstream iss(tmp);
		char c;
		int i = 0;
		int dotSpot;
		while (iss) {
			iss.get(c);
			if (c == '.')
				dotSpot = i;
			++i;
		}

		if (tmp[dotSpot + 1] != 'e' && tmp[dotSpot + 2] != 'n')
			tmp.erase(tmp.begin() + dotSpot, tmp.end());

		ttmp = tmp;
		tmp += ".en.vtt";
		std::cout << "tmp: " << tmp;
		Video v(ttmp, tmp, utmp);
		vids.push_back(v);
		getline(inFile, tmp);
		v.title = tmp;
		getline(inFile, tmp);
		v.id = tmp;

	} while (inFile.peek() != EOF);

	inFile.close();
}

// clean up original file formatting
//////////////////////////////////////////////////////////////////////////////////
void cleanupFiles(std::vector<Video>& vids)
{
	std::string    systemCall;
	std::string    tmp;
	std::ofstream  shell;

	for (Video& vid : vids)
	{
		vid.shellFile = "cleanFile.sh";
		shell.open(vid.shellFile);
		tmp = "#!\\bin\\bash\n\n" + sed + "\"" + vid.filename + "\"";
		shell << tmp;
		shell.close();
		system((const char*)vid.shellFile.c_str());
		tmp = "rm cleanFile.sh";
		system((const char*)tmp.c_str());
	}
	return;
}
//////////////////////////////////////////////////////////////////////////////////
void getTime(std::vector<std::string>& keys, std::vector<Video>& vids) {

	std::vector<std::string> lines;

	std::ifstream  inFile;
	std::string    temp;
	std::string    s;
	std::string    currentWord;

	if (sameVid == false) {
		s = '0';
		inFile.open(vids[0].filename);
		if (inFile.fail()) {
			std::cout << "\n\tCouldn't open file in getTime function";
			system("PAUSE");
			exit(1);
		}
		while (inFile.good()) {
			getline(inFile, temp);
			while (inFile.peek() != EOF) {
				std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
				vids[0].lines.push_back(temp);
				getline(inFile, temp);
			}
		} inFile.close();
	}
	//iterate through lines between '\n's
	short i = 0;
	for(auto& vid : vids){
		for (std::string& line : vid.lines) {
			//iterate through keywords
			//parse look lines look for keywords
			std::istringstream iss(line);

			while (iss) {
				iss >> currentWord;

				if (currentWord == keys[0]) {
					try {
						if (isdigit(vid.lines[i - 3][0]))
							getClips(vid, i - 3);
					}
					catch (std::exception& e) { continue; }

					try {
						if (isdigit(vid.lines[i - 2][0]))
							getClips(vid, i - 2);
					}
					catch (std::exception& e) { continue; }

					try {
						if (isdigit(vid.lines[i - 1][0]))
							getClips(vid, i - 1);
					}
					catch (std::exception& e) { continue; }
				}
			} ++i;
		}
	} return;
}
//////////////////////////////////////////////////////////////////////////////////
void getClips(Video& vid, int line) {
	short hr = 0;
	short min = 0;
	short sec = 0;

	//calculate timestamp
	try {
		if (vid.lines[line][0] != '0')
			hr += atoi(&vid.lines[line][0]);
		if (vid.lines[line][0] == '0' && vid.lines[line][1] != '0')
			hr += atoi(&vid.lines[line][1]);
		if (vid.lines[line][3] != '0')
			min += atoi(&vid.lines[line][3]);
		if (vid.lines[line][3] == '0' && vid.lines[line][4] != '0')
			min += atoi(&vid.lines[line][4]);
		if (vid.lines[line][6] != '0')
			sec += atoi(&vid.lines[line][6]);
		if (vid.lines[line][6] == '0' && vid.lines[line][7] != '0')
			sec += atoi(&vid.lines[line][7]);
	}
	catch (std::exception& e) {}

	if ((sec - mention) < 0) {
		--min;
		sec = 60 + (sec - mention);
	}
	std::string name = vid.url + "&feature=youtu.be&t=" + std::to_string(hr) + 'h'
		+ std::to_string(min) + 'm' + std::to_string(sec - mention) + 's';

	// skip if new file is within 5 seconds of another clip
	int seconds = (min * 60) + (sec - mention);
	for (auto &c : vid.clipSecondMark) {
		if (fabs(c.second - seconds) <= 5 && fabs(c.second - seconds) != 0)
			return;
	}
	vid.clipSecondMark.insert(std::make_pair(name, seconds));
	vid.clips.insert(name);

	return;
}
//////////////////////////////////////////////////////////////////////////////////
void launchUrls(std::vector<Video>& vids, std::vector<std::string> key) {
	
	int matchSize = 0;
	for (auto& vid : vids) {
		matchSize += vid.clips.size();
	}
	std::cout << "\n\n\"" << key[0] << "\" mentioned " << matchSize << " across all videos";
	Sleep(3000);

	for (Video& vid : vids) {
		if (vid.clips.size() == 0) {
			std::cout << "\n\n\tCouldn't find any mentions of "
				<< key[0] << " in this video...";
			return;
		}
		std::string newFile;
		short  switc = 1;
		short i = 1;
		char  response = 'q';
		std::cout << "\n\n";
		screen += "\t\"" + key[0] + "\" is mentioned: "
			+ std::to_string(vid.clips.size()) + " times.";
		if (sameVid == true) screen += "\n";
		redraw(screen);
		std::cout << "\n\n\tPress ENTER to launch first video...";
		_getch();

		if (sameVid == false) {
			screen += "\n\n\t    'S' -- skip"
					    "\n\t    'C' -- cancel"
				        "\n\t    'A' -- save all to clipboard"
				        "\n\t    'P' -- print timestamps (in seconds) to file"
				        "\n\t'SPACE' -- next\n";
		}
		redraw(screen);

		for (std::string s : vid.clips) {
			if (i > 1) response = _getch();
			if (response == 'a' || response == 'A') {
				saveToClip(vid.clips);
				screen += "\n\tURL List saved to clipboard";
				redraw(screen);
				response = _getch();
			}

			if (response == 's' || response == 'S')
				switc = 0;
			if (response == 'c' || response == 'C')
				return;
			if (response == 'a' || response == 'A')
				continue;
			if (response == 'p' || response == 'P')
				exportTimestamps(vids);

			switch (switc) {
				case 0:
					screen += "\n\tSKIPPING VIDEO " + std::to_string(i) + "/" + std::to_string(vid.clips.size());
					redraw(screen);
					switc = 1;
					break;
				case 1:
					screen += "\n\tPLAYING VIDEO " + std::to_string(i) + "/" + std::to_string(vid.clips.size());
					redraw(screen);
					newFile = "start chrome \"" + s + "\"";
					system((const char*)newFile.c_str());
					break;
				default:
					screen += "\n\tINVALID ENTRY!";
					switc = 1;
					--i;
					break;
			} ++i;
		}
	} return;
}
//////////////////////////////////////////////////////////////////////////////////
std::string getFileContents(std::string fileName) {
	
	std::ifstream File;

	File.open(fileName);
	std::string Lines = "";

	if (File.good()) {
		while (File.good()) {
			std::string TempLine;
			std::getline(File, TempLine);
			TempLine += "\n";
			Lines += TempLine;
		} return Lines;
	} else { return "File did not open\n"; }
}
//////////////////////////////////////////////////////////////////////////////////
void redraw() {
	system("cls");
	std::cout << getFileContents("title.txt");
	return;
}
//////////////////////////////////////////////////////////////////////////////////
void redrawLoading() {
	system("cls");
	std::cout << getFileContents("title.txt");

	std::cout << "\n\n\tLoading...";
	return;
}
//////////////////////////////////////////////////////////////////////////////////
void redraw(std::string s) {
	system("cls");
	std::cout << getFileContents("title.txt");
	int count = 0;
	char e;

	if (s.size() > 40) {
		std::stringstream is(s);
		while (is) {
			std::cout << "\n\t";
			for (int i = 0; i < 80; ++i) {
				is.get(e);
				if (!is)
					return;
				if (e == '\n') {
					std::cout.put(e);
					i = 0;
					continue;
				}
				std::cout.put(e);
			}
		}
	} return;
}
//////////////////////////////////////////////////////////////////////////////////
void saveToClip(std::set<std::string>& urls) {
	
	std::string tmp;
	std::ofstream out;
	out.open("urls.txt");
	for (std::string s : urls) {
		out << s << std::endl;
	}
	out.close();
	tmp = "cat urls.txt | clip";
	system((const char*)tmp.c_str());
}
//////////////////////////////////////////////////////////////////////////////////
void exportTimestamps(std::vector<Video>& vids) {

	std::ofstream timeFile;
	timeFile.open("timestamps.txt");
	if (timeFile.good()) {
		for (auto& v : vids) {
			for (auto& t : v.clipSecondMark) {
				timeFile << t.second << '\n';
			}
		}
	}
	else {
		std::cout << "\n\nCould not export timestamps";
		exit(3);
	}
	return;
}