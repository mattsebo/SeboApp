#pragma once
#include <map>
#include "Sheet.h"
#include <collection.h>


using namespace std;
using namespace Platform;
using namespace Platform::Collections;

ref class MachineCutLog sealed
{
public:
	MachineCutLog(Platform::String^ workOrder)
		:
		workOrder(workOrder)
	{
		workOrder->Concat(workOrder->ToString(), L"_");
	}
	void EchoSheets()
	{}
	void AddSheet(Platform::String^ s)
	{
		wstring filename = s->Begin();
		if (filename.substr(filename.length() - 4), filename.length())
		{
			filename = filename.substr(0, filename.length() - 4);
		}
		try {
			stoi(filename.substr(filename.length() - 4, 4), nullptr, 10); // convert last 4 characters to int, if it's int it's a full sheet.
			wstring batch = filename.substr(0, filename.length() - 4);
			bool sheetCut = false;
			if (batch.substr(0, 3) == L"CUT")
			{
				sheetCut = true;
				batch = batch.substr(3, batch.length());
				filename = filename.substr(3, filename.length() - 3);
			}
			String^ sFilename = ref new String(filename.c_str());
			String^ sBatch = ref new String(batch.c_str());
			Sheet^ sheet = ref new Sheet(sFilename, sBatch, sheetCut);
			sheets->Insert(sFilename, sheet);
		}
		catch (const std::invalid_argument& ia) {}
	}
	int GetPercentageCut(MachineCutLog^ othermachine)
	{
		int numSheets = sheets->Size;
		int numCutSheets = 0;
		if (sheets->Size == 0)
		{
			numSheets = othermachine->GetSheets()->Size;
			for (auto pair : othermachine->GetSheets())
			{
				auto key = pair->Key;
				auto value = pair->Value;
				if (othermachine->GetSheets()->Lookup(key)->GetIsCut())
				{
					numCutSheets++;
				}
			}
		}
		else if (othermachine->GetSheets()->Size == 0)
		{
			numSheets = sheets->Size;
			for (auto pair : sheets)
			{
				auto key = pair->Key;
				auto value = pair->Value;
				if (sheets->Lookup(key)->GetIsCut())
				{
					numCutSheets++;
				}
			}
		}
		else
		{
			for (auto pair : sheets)
			{
				auto key = pair->Key;
				auto value = pair->Value;
				if (value->GetIsCut() || othermachine->GetSheets()->Lookup(key)->GetIsCut())
				{
					numCutSheets++;
				}
			}
		}
		int result = int((float(numCutSheets) / float(numSheets)) * 100.0f);
		return result;
	}
	Platform::String^ GetWorkOrder()
	{
		return workOrder;
	}
	void EchoSheetsToCut(Windows::UI::Xaml::Controls::TextBox^ textbox, MachineCutLog^ othermachine)
	{
		if (sheets->Size == 0)
		{
			for (auto pair : othermachine->GetSheets())
			{
				auto key = pair->Key;
				auto value = pair->Value;
				bool m1 = value->GetIsCut();
				if (!m1)
				{
					textbox->Text += value->GetFilename() + L"\n";
				}
			}
		}
		else if (othermachine->GetSheets()->Size == 0)
		{
			for (auto pair : sheets)
			{
				auto key = pair->Key;
				auto value = pair->Value;
				bool m1 = value->GetIsCut();
				if (!m1)
				{
					textbox->Text += value->GetFilename() + L"\n";
				}
			}
		}
		else
		{
			for (auto pair : sheets)
			{
				auto key = pair->Key;
				auto value = pair->Value;
				bool m1 = value->GetIsCut();
				bool m2 = othermachine->GetSheets()->Lookup(key)->GetIsCut();
				if (!m1 && !m2)
				{
					textbox->Text += value->GetFilename() + L"\n";
				}
			}
		}
	}
	void UpdateTextBox(Windows::UI::Xaml::Controls::TextBox^ textbox, MachineCutLog^ othermachine)
	{
		if (workOrder->IsEmpty())
		{
			textbox->Text = L"Cannot find that job, try again.";
		}
		else
		{
			textbox->Text = L"Job " + workOrder + L" is ";
			textbox->Text += GetPercentageCut(othermachine) + L"% cut.\n\n";
			textbox->Text += L"Sheets left to be cut:\n";
			EchoSheetsToCut(textbox, othermachine);
		}
	}
	void UpdateProgressBar(Windows::UI::Xaml::Controls::ProgressBar^ progressBar, MachineCutLog^ othermachine)
	{
		if ((sheets->Size == 0 && othermachine->GetSheets()->Size == 0)
			|| workOrder->IsEmpty())
		{
			progressBar->Value = 0;
		}
		else
		{
			progressBar->Value = GetPercentageCut(othermachine);
		}
	}
	Windows::Foundation::Collections::IMap<String^, Sheet^>^ GetSheets()
	{
		return sheets;
	}
private:
	Platform::String^ workOrder;
private:
	Map<String^, Sheet^>^ sheets = ref new Map<String^, Sheet^>();
};