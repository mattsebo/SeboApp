#pragma once
#include <sstream>

using namespace std;

ref class Sheet sealed
{
public:
	Sheet(Platform::String^ filename, Platform::String^ batch, Platform::Boolean isCut)
		:
		filename(filename),
		batch(batch),
		isCut(isCut)
	{}
	Sheet(Platform::String^ rawFilename)
	{
		std::wstring rawFile = rawFilename->Begin();
		if (rawFile.substr(0, 3) == L"CUT")
		{
			isCut = true;
			rawFile = rawFile.substr(3, rawFile.length());
		}
		else
		{
			isCut = false;
		}
		batch = ref new Platform::String(rawFile.substr(0, rawFile.length() - 4).c_str());
		filename = ref new Platform::String(rawFile.c_str());
	}
	Platform::String^ GetFilename()
	{
		return filename;
	}
	Platform::String^ GetBatch()
	{
		return batch;
	}
	Platform::Boolean GetIsCut()
	{
		return isCut;
	}
private:
	Platform::String^ filename;
	Platform::String^ batch;
	Platform::Boolean isCut;
};