#pragma once
#include <collection.h>
#include "pch.h"
#include "Sheet.h"

using namespace std;
using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage;
using namespace Concurrency;

ref class WorkOrder sealed
{
public:
	WorkOrder(String^ name, Windows::Storage::Search::StorageFileQueryResult^ query, Windows::UI::Xaml::Controls::TextBox^ textbox)
		:
		sheets(ref new Map<String^, Sheet^>()),
		name(name),
		query(query)
	{}
	void Init(Platform::String^ batch)
	{
		sheets = ref new Map<String^, Sheet^>();
		create_task(query->GetFilesAsync()).then([=](IVectorView<StorageFile^>^ files)
		{
			// check each file within the subfolder
			for (auto file : files)
			{
				std::wstring name = file->Name->Begin();
				// cut off .cnc from filename:
				name = name.substr(0, name.length() - 4);
				// check if the file starts with batch name or CUT and batch name
				if (name.substr(0, batch->Length()) == batch->Begin() ||
					name.substr(0, batch->Length() + 3) == (L"CUT" + batch)->Begin())
				{
					// create sheet and insert it to sheets map.
					Sheet^ sheet = ref new Sheet(ref new String(name.c_str()));
					sheets->Insert(sheet->GetFilename(), sheet);
				}
			}
		});
	}
	Windows::Foundation::Collections::IMap<String^, Sheet^>^ GetSheets()
	{
		return sheets;
	}
private:
	Map<String^, Sheet^>^ sheets;
	Windows::Storage::Search::StorageFileQueryResult^ query;
	String^ name;
};