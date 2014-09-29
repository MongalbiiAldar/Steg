#include <vector>
#include <fstream>
#include <iostream>

using namespace std;

int StegCode(char* ContainerFileName, char* OutputFileName, char* InformationFileName);
 
int StegDecode(char* ContainerFName, char* OutputFileName); 

bool CheckWav(vector<unsigned char> wav);

int GetBytesPerAmpl(unsigned char bitsPerAmpl);

vector<unsigned char> readFile(char* FileName);

int WriteOutFile(char* OutputFileName,vector<unsigned char> output);

int main(int argc, char* argv[])
{
	if (argc != 4 && argc != 3)
	{
		cout << "For code:" << "\n";
		cout << "steg <container.wav> <out.wav> <information file>" << "\n\n";
		cout << "For decode:" << "\n";
		cout << "steg <container.wav> <out information file>" <<"\n";
		return 0;
	}

	if (argc == 4)
		return StegCode(argv[1], argv[2], argv[3]);
	else
		return StegDecode(argv[1], argv[2]);
}

int StegCode(char* ContainerFileName, char* OutputFileName, char* InformationFileName)
{
	vector<unsigned char> inWav = readFile(ContainerFileName);
	if (!CheckWav(inWav))
	{
		cout << ContainerFileName << " It is not .wav file!" << "\n";
		return 1;
	}

	vector<unsigned char> infFile = readFile(InformationFileName);
	if (infFile.size() > 65536 || // если размер файла не умещается в 2 байта
		infFile.size() * 8 + 16 > (inWav.size() - 44) / GetBytesPerAmpl(inWav[34]) ) // или если его битовое представление не уместится в контейнер
	{
		cout << InformationFileName << " Information file is too big" << "\n";
		return 1;
	}

	
	vector<unsigned char> data; 
	
	int fileSize = infFile.size();
	for (int i = 0; i < 16; i++)
	{
		data.push_back(fileSize % 2);
		fileSize /= 2;
	}

	for (int i = 0; i < infFile.size(); i++)
	{
		unsigned char b = infFile[i];
		for (int j = 0; j < 8; j++)
		{
			data.push_back(b % 2);
			b /= 2;
		}
	}

	int bytesPerAmpl = GetBytesPerAmpl(inWav[34]);

	for (int i = 0, wavIndex = 44; i < data.size(); i++, wavIndex += bytesPerAmpl)
	{
		if (inWav[wavIndex] % 2 == 1) 
		{
			if (data[i] == 0) 
				inWav[wavIndex]++; 
		}
		else 
		{
			if (data[i] == 1) 
				inWav[wavIndex]++; 
		}
	}

	WriteOutFile(OutputFileName,inWav);
	return 0;
}

int StegDecode(char* ContainerFileName, char* OutputFileName)
{
	vector<unsigned char> inWav = readFile(ContainerFileName);
	if (!CheckWav(inWav))
	{
		cout << ContainerFileName << " It is not .wav file!" << "\n";
		return 1;
	}

	int wavIndex = 44; 
	int bytesPerAmpl = GetBytesPerAmpl(inWav[34]);

	int fileSize = 0;
	int pow2 = 1; 
	for (int i = 0; i < 16; i++)
	{
		fileSize += (inWav[wavIndex] % 2) * pow2;
		pow2 *= 2;
		wavIndex += bytesPerAmpl;
	}

	vector<unsigned char> output;
	for (int i = 0; i < fileSize; i++)
	{
		unsigned char b = 0;
		for (int j = 0, pow2 = 1; j < 8; j++, pow2 *= 2, wavIndex += bytesPerAmpl)
		{
			b += (inWav[wavIndex] % 2) * pow2;
		}
		output.push_back(b);
	}

	WriteOutFile(OutputFileName,output);
	return 0;
}

vector<unsigned char> readFile(char* fileName)
{
	vector<unsigned char> input;
	ifstream ifst(fileName, ios::binary);
	if (ifst.fail())
		return input;

	ifst.seekg(0, ios::end);
	int size = ifst.tellg();
	ifst.seekg(0, ios::beg);

	input.resize(size);
	ifst.read((char*)&input.front(), size);
	ifst.close();

	return input;
}

bool CheckWav(vector<unsigned char> wav)
{ 
	vector<unsigned char> fmtId; 
	
	fmtId.push_back('W');	 
	fmtId.push_back('A');	
	fmtId.push_back('V');	
	fmtId.push_back('E');	 

	vector<unsigned char> fileId;
	fileId.insert(fileId.begin(), wav.begin() + 8, wav.begin() + 12);

	return (fileId == fmtId);
}

int GetBytesPerAmpl(unsigned char bitsPerAmpl)
{
	int bytesPerAmpl = 0;
	switch (bitsPerAmpl)
	{
	case 8:
		bytesPerAmpl = 1;
		break;
	case 16:
		bytesPerAmpl = 2;
		break;
	case 24:
		bytesPerAmpl = 3;
		break;
	case 32:
		bytesPerAmpl = 4;
		break;
	default:
		bytesPerAmpl = 1;
	}
	return bytesPerAmpl;
}
int WriteOutFile(char* OutputFileName,vector<unsigned char> output)
{
	ofstream outStream(OutputFileName, ios::binary);
	outStream.write((char*)&output.front(), output.size());
	outStream.close();
	return 1;
}
