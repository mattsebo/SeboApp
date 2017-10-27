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
	{

	}
	/*Sheet()
		:
		filename(L""),
		batch(L""),
		isCut(false)
	{}*/
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