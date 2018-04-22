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
//////////////////////////////////////////////////////////////////////////////////
struct Video
{
	Video()
	{
		title = "";
	}
	Video(std::string t, std::string f, std::string u)
	{
		title = t;
		url = u;
		filename = f;
	}
	std::string               shellFile;
	std::string               title;
	std::string               filename;
	std::string               url;
	std::set<std::pair<std::string, int>>  clipSecondMark;
	std::set<std::string>     clips;
	std::vector<std::string>  lines;

};

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

std::string sed = "sed -i 's/\\(<\\/c>\\)/ /g;s/\\(<\\)/ /g;s/\\(-->\\)/ /g;s/\\(align.*%\\)/ /g;s/\\(c\\..*>\\)/ /g;s/\\(c>\\)/ /g;s/ /\\n/g;s/\\n/ /g;s/0.*>/ /g' ";
std::vector<std::string> cleanFiles;
int  mention = 2;
char selection;
char response;
std::string  screen;
bool         sameVid = false;

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

void getTime(std::vector<std::string>&, std::vector<Video>&);
void launchUrls(std::vector<Video>&, std::vector<std::string>);
void getFileNames(std::vector<Video>&, std::string);
std::string getFileContents(std::string);
void saveToClip(std::set<std::string>&);
void cleanupFiles(std::vector<Video>&);
void getClips(Video&, int);
void redrawLoading();
void redraw(std::string);
void redraw();
void exportTimestamps(std::vector<Video>&);

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	std::vector<std::string>  keywords;
	std::vector<Video>        videos;

	//CreatePipe(read, write, NULL, sizeof(pipeBuff));

	std::string  firstUrl;//  = argv[argc - 1];
	std::string  pCmdA = "youtube-dl --yes-playlist --write-auto-sub --skip-download";
	std::string  vCmdA = "youtube-dl --write-auto-sub --skip-download ";
	std::string  vCmdE = "youtube-dl --skip-download --write-sub --sub-lang en ";
	std::string  pCmdE = "youtube-dl --yes-playlist --sub-lang en --skip-download";
	std::string  vid = "-video";
	std::string  play = "-playlist";
	std::string  log = " > logFile.txt";


	std::string word;
	char response = 'q';
	//push keywords into vector
	do
	{
		if (response != 'q')
			keywords.erase(keywords.begin(), keywords.end());

		redraw();

		if (sameVid == false)
		{
			if (response != 'q')
				videos.erase(videos.begin(), videos.end());

			screen = "";
			//ShellExecute(window, NULL, "\n\tSEARCH: \n\t\tP - playlist \n\t\tV - video", NULL, NULL, SW_MAXIMIZE);

			std::cout << "\n\tSEARCH: \n\t\tP - playlist \n\t\tV - video";
			selection = _getch();
		}

		redraw();

		if (sameVid == false)
		{
			std::cout << "\n\tENTER YOUTUBE URL: "; std::cin >> firstUrl;
			std::cout << "\n\tDOWNLOAD VIDEO? (Y/N): "; response = _getch();
			//video class are initialized in this function
			redrawLoading();
			getFileNames(videos, firstUrl);
			screen = "\n\tVIDEO: ";

			for (const Video& vid : videos)
				screen += vid.title + '\n';
		}
		redraw(screen);

		if (sameVid == false)
		{
			if (selection == 'v' || selection == 'V')
			{
				videos[0].url = firstUrl;
				std::string cmd = vCmdA + firstUrl;
				system((const char*)cmd.c_str());
				cmd = vCmdE + firstUrl;
				system((const char*)cmd.c_str());
				if (response == 'y' || response == 'Y')
				{
					cmd = "youtube-dl " + firstUrl;
					system((const char*)cmd.c_str());
				}
			}
			if (selection == 'p' || selection == 'P')
			{
				std::string cmd = pCmdA + firstUrl;
				system((const char*)cmd.c_str());
				cmd = pCmdE + firstUrl;
				system((const char*)cmd.c_str());
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

		if (sameVid == true)
		{
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

		if (response != 'y' && response != 'Y')
		{

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
//////////////////////////////////////////////////////////////////////////////////
void getFileNames(std::vector<Video>& vids, std::string firstUrl)
{
	std::ifstream  inFile;
	std::ifstream  urlFile;
	std::string    ttmp;
	std::string    utmp;
	std::string    tmp;
	std::string    cmd;


	//load in .vtt file titles
	if (selection == 'v' || selection == 'V')
		cmd = "youtube-dl --get-filename \"" + firstUrl + "\" > filenames.txt";
	if (selection == 'p' || selection == 'P')
		cmd = "youtube-dl --yes-playlist --get-filename \"" + firstUrl + "\" > filenames.txt";
	system((const char*)cmd.c_str());

	//cmd = firstUrl + "> urlnames.txt";
	//system((const char*)cmd.c_str());

	inFile.open("filenames.txt");
	//urlFile.open("urlnames.txt");

	//INITIALIZE VIDEO CLASSES
	int itmp = 0;

	do
	{
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
	} while (inFile.peek() != EOF);

	inFile.close();
}

// clean up original file formatting
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void cleanupFiles(std::vector<Video>& vids)
{
	std::string systemCall;
	std::string tmp;
	std::ofstream shell;
	int i = 1;
	for (Video& vid : vids)
	{
		vid.shellFile = "cleanFile" + std::to_string(i) + ".sh";
		shell.open(vid.shellFile);
		std::string tmp = "#!\\bin\\bash\n\n" + sed + "\"" + vid.filename + "\"";
		shell << tmp;
		shell.close();
		system((const char*)vid.shellFile.c_str());

		++i;
	}
	return;
}
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void getTime(std::vector<std::string>& keys, std::vector<Video>& vids)
{
	std::vector<std::string> lines;

	std::ifstream  inFile;
	std::string    temp;
	std::string    s;
	std::string    currentWord;

	//for (Video& vid : vids)
	//{
	if (sameVid == false)
	{
		s = '0';
		inFile.open(vids[0].filename);
		if (inFile.fail())
		{
			std::cout << "\n\tCouldn't open file in getTime function";
			system("PAUSE");
			exit(1);
		}
		while (inFile.good())
		{
			getline(inFile, temp);
			while (inFile.peek() != EOF)
			{
				std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
				vids[0].lines.push_back(temp);
				getline(inFile, temp);
			}
		} inFile.close();
	}
	//iterate through lines between '\n's
	short i = 0;
	for (std::string& line : vids[0].lines)
	{
		//iterate through keywords
		//parse look lines look for keywords
		std::istringstream iss(line);

		while (iss)
		{
			iss >> currentWord;

			if (currentWord == keys[0])
			{
				try
				{
					if (isdigit(vids[0].lines[i - 3][0]))
						getClips(vids[0], i - 3);
				}
				catch (std::exception& e) { continue; }

				try
				{
					if (isdigit(vids[0].lines[i - 2][0]))
						getClips(vids[0], i - 2);
				}
				catch (std::exception& e) { continue; }

				try
				{
					if (isdigit(vids[0].lines[i - 1][0]))
						getClips(vids[0], i - 1);
				}
				catch (std::exception& e) { continue; }
			}
		}
		++i;
	}

	return;
}
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void getClips(Video& vid, int line)
{
	short hr = 0;
	short min = 0;
	short sec = 0;

	//calculate timestamp
	try
	{
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

	//	std::cout << "\n\nhr: " << hr << "  min: " << min << "  sec: " << sec << std::endl;

	if ((sec - mention) < 0)
	{
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
//////////////////////////////////////////////////////////////////////////////////
void launchUrls(std::vector<Video>& vids, std::vector<std::string> key)
{
	for (Video& vid : vids)
	{
		if (vid.clips.size() == 0)
		{
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

		if (sameVid == false)
		{
			screen += "\n\n\t    'S' -- skip"
				"\n\t    'C' -- cancel"
				"\n\t    'A' -- save all to clipboard"
				"\n\t    'P' -- print timestamps (in seconds) to file"
				"\n\t'SPACE' -- next\n";
		}
		redraw(screen);

		for (std::string s : vid.clips)
		{
			if (i > 1) response = _getch();
			if (response == 'a' || response == 'A')
			{
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



			switch (switc)
			{
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
			}

			++i;
		}
	} return;
}
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
std::string getFileContents(std::string fileName)
{
	std::ifstream File;

	File.open(fileName);
	std::string   Lines = "";

	if (File.good())
	{
		while (File.good())
		{
			std::string TempLine;
			std::getline(File, TempLine);
			TempLine += "\n";
			Lines += TempLine;
		} return Lines;
	}
	else { return "File did not open\n"; }
}
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void redraw()
{
	system("cls");
	std::cout << getFileContents("title.txt");
	return;
}

void redrawLoading() {
	system("cls");
	std::cout << getFileContents("title.txt");

	std::cout << "\n\n\tLoading...";
	return;
}

void redraw(std::string s)
{
	system("cls");
	std::cout << getFileContents("title.txt");
	int count = 0;
	char e;

	if (s.size() > 40)
	{
		std::stringstream is(s);
		while (is)
		{
			std::cout << "\n\t";
			for (int i = 0; i < 80; ++i)
			{
				is.get(e);

				if (!is)
					return;
				if (e == '\n')
				{
					std::cout.put(e);
					i = 0;
					continue;
				}
				std::cout.put(e);
			}
		}
	}
	return;
}
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void saveToClip(std::set<std::string>& urls)
{
	std::string tmp;
	std::ofstream out;
	out.open("urls.txt");
	for (std::string s : urls)
	{
		out << s << std::endl;
	}
	out.close();

	tmp = "cat urls.txt | clip";

	system((const char*)tmp.c_str());
}



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