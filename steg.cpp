#include <vector>
#include <fstream>
#include <iostream>

using namespace std;

int StegCode(char* ContainerFileName, char* OutputFileName, char* InformationFileName);
 
int StegDecode(char* ContainerFName, char* OutputFileName); 

int GetBytesPerAmpl(unsigned char BitsPerAmpl);

vector<unsigned char> readFile(char* FileName);

int WriteOutFile(char* OutputFileName,vector<unsigned char> output);

int main(int argc, char* argv[])
{
	if (argc != 4 && argc != 3)
	{
		cout << "For code:" << "\n";
		cout << "steg <container.wav> <information file> <out.wav>" << "\n\n";
		cout << "For decode:" << "\n";
		cout << "steg <container.wav> <out information file>" <<"\n";
		return 0;
	}

	if (argc == 4)
		return StegCode(argv[1], argv[2], argv[3]);
	else
		return StegDecode(argv[1], argv[2]);
}

int StegCode(char* ContainerFileName, char* InformationFileName,char* OutputFileName)
{
	vector<unsigned char> inWav = readFile(ContainerFileName);
	vector<unsigned char> infFile = readFile(InformationFileName);
	if (infFile.empty())
	{	
		cout<<"Not exist information file!\n";
		return 0;
	}
	else
		if (inWav.size()<infFile.size()*8 + 20 +44)
		{
			cout << InformationFileName << " Information file is big" << "\n";
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

int GetBytesPerAmpl(unsigned char BitsPerAmpl)
{
	int BytesPerAmpl = 0;
	if (BitsPerAmpl==8)
		BytesPerAmpl=1;
	else
	if (BitsPerAmpl==16)
		BytesPerAmpl=2;
	else
	if (BitsPerAmpl==24)
		BytesPerAmpl=3;
	else
	if (BitsPerAmpl==32)
		BytesPerAmpl=4;
	else
		BytesPerAmpl=1;
	return BytesPerAmpl;
}
int WriteOutFile(char* OutputFileName,vector<unsigned char> output)
{
	ofstream outStream(OutputFileName, ios::binary);
	outStream.write((char*)&output.front(), output.size());
	outStream.close();
	return 1;
}
