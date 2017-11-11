#pragma once
#include <map>
#include <collection.h>
#include "WorkOrder.h"

using namespace std;
using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Storage::AccessCache;

ref class MachineCutLog sealed
{
public:
	MachineCutLog(Platform::String^ FALtoken)
		:
		workOrders(ref new Map<String^, WorkOrder^>()),
		workOrdersShort(ref new Map<String^, Windows::Foundation::Collections::IVector<String^>^>()),
		FALtoken(FALtoken)
	{}
	void Init(Windows::UI::Xaml::Controls::TextBox^ textbox, Windows::UI::Xaml::Controls::AutoSuggestBox^ searchbox
		, Windows::UI::Core::CoreDispatcher^ dispatcher)
	{
		workOrders = ref new Map<String^, WorkOrder^>();
		workOrdersShort = ref new Map<String^, Windows::Foundation::Collections::IVector<String^>^>();
		try
		{
			if (StorageApplicationPermissions::FutureAccessList->ContainsItem(FALtoken))
			{
				create_task(StorageApplicationPermissions::FutureAccessList->GetFolderAsync(FALtoken))
					.then([=](StorageFolder^ folder)
				{ // Task to query base path
					if (folder != nullptr)
					{
						// Query the folder.
						auto query2 = folder->CreateFolderQuery();
						create_task(query2->GetFoldersAsync()).then([=](IVectorView<StorageFolder^>^ subfolders)
						{
							// Query the subfolders.
							for (auto subfolder : subfolders)
							{
								dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High,
									ref new Windows::UI::Core::DispatchedHandler([textbox, searchbox]()
								{
									textbox->Text = L"Loading...";
									searchbox->IsEnabled = false;
								}));
								auto query = subfolder->CreateFileQuery();
								String^ name = subfolder->Name;
								WorkOrder^ workOrder = ref new WorkOrder(name, query, textbox);
								workOrders->Insert(name, workOrder);
								String^ shortName = ref new String(name->Begin());
								RemoveBatchFromString(shortName);
								if (!workOrdersShort->HasKey(shortName))
								{
									workOrdersShort->Insert(shortName, ref new Vector<String^>(1,name));
								} 
								else
								{
									workOrdersShort->Lookup(shortName)->Append(name);
								}
							}
						}).then([=](void)
						{
							dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
								ref new Windows::UI::Core::DispatchedHandler([this, textbox, searchbox]()
							{
								textbox->Text = L"Ready to search!";
								searchbox->IsEnabled = true;
							}));
						});
					}
				});
			}
		}
		catch (const COMException^ e)
		{

		}
	}

	void InitSheets(Platform::String^ search, MachineCutLog^ othermachine)
	{
		for (auto it : workOrdersShort->Lookup(search))
		{
			workOrders->Lookup(it)->Init(GetBatchFromString(it));
		}
		if (othermachine->GetWorkOrdersShort()->HasKey(search))
		{
			for (auto it : othermachine->GetWorkOrdersShort()->Lookup(search))
			{
				othermachine->GetWorkOrders()->Lookup(it)->Init(GetBatchFromString(it));
			}
		}
	}
	bool KeyExists(Platform::String^ search)
	{
		return workOrdersShort->HasKey(search);
	}
	void EchoSheetsToCut(Windows::UI::Xaml::Controls::TextBox^ textbox,Platform::String^ search, Windows::UI::Xaml::Controls::ProgressBar^ progressbar, MachineCutLog^ othermachine)
	{
		textbox->Text = L"";
		int numSheets = 0;
		int numSheetsCut = 0;
		for (auto workorder : workOrdersShort->Lookup(search)) // iterate through vector of string keys for full workorder names
		{
			if (workOrders->HasKey(workorder) && othermachine->GetWorkOrders()->HasKey(workorder)) // scenario where both machines has current workorder.
			{

				for (auto sheet : workOrders->Lookup(workorder)->GetSheets())
				{
					numSheets++;
					auto otherMachineSheetMap = othermachine->GetWorkOrders()->Lookup(workorder)->GetSheets();
					auto otherMachineSheet = ref new Sheet();
					if (otherMachineSheetMap->HasKey(sheet->Key))
					{
						otherMachineSheet = otherMachineSheetMap->Lookup(sheet->Key);
					}
					if (sheet->Value->GetIsCut() || otherMachineSheet->GetIsCut())
					{
						numSheetsCut++;
					}
					else
					{
						textbox->Text += sheet->Key + L"\n";
					}
				}
			}
			// TO DO: Implement scenario where only main machine has key found.
		}
		int result = int((float(numSheetsCut) / float(numSheets)) * 100.0f);
		progressbar->Value = result;
		textbox->Text = L"Job " + search + L" is " + result.ToString() + L"% cut.\nSheets left to cut:\n" + textbox->Text;
	}
	void EchoDebug(Platform::String^ search, Windows::UI::Xaml::Controls::TextBox^ textbox, MachineCutLog^ othermachine)
	{
		textbox->Text = L"";
		for (auto it : workOrdersShort->Lookup(search))
		{
			//workOrders->Lookup(it)->Init(GetBatchFromString(it));
			for (auto sheet : workOrders->Lookup(it)->GetSheets())
			{
				textbox->Text += sheet->Key + L"/" + sheet->Value->GetIsCut() + L"\n";
			}
			//textbox->Text += it + L" Size: " + L"\n";
		}
		for (auto it : othermachine->GetWorkOrdersShort()->Lookup(search))
		{
			for (auto sheet : othermachine->GetWorkOrders()->Lookup(it)->GetSheets())
			{
				textbox->Text += sheet->Key + L"/" + sheet->Value->GetIsCut() + L"\n";
			}
			//textbox->Text += it + L"\n";
		}
	}
	Windows::Foundation::Collections::IMap<String^, WorkOrder^>^ GetWorkOrders()
	{
		return workOrders;
	}
	Windows::Foundation::Collections::IMap<String^, Windows::Foundation::Collections::IVector<String^>^>^ GetWorkOrdersShort()
	{
		return workOrdersShort;
	}
private:
	Platform::String^ GetBatchFromString(Platform::String^ s)
	{
		std::wstring name = s->Begin();
		for (auto i = name.length(); i > 0; i--)
		{
			if (name[i] == L'_')
			{
				return ref new Platform::String(name.substr(i + 1, name.length()).c_str());
			}
		}
		return ref new Platform::String(L"BATCH NOT FOUND!");
	}
	void RemoveBatchFromString(Platform::String^& name)
	{
		std::wstring s = name->Begin();
		for (auto i = s.length(); i > 0; i--)
		{
			if (s[i] == L'_')
			{
				name = ref new Platform::String(s.substr(0, i).c_str());
				break;
			}
		}
	}
private:
	Map<String^, WorkOrder^>^ workOrders;
	Map<String^, Windows::Foundation::Collections::IVector<String^>^>^ workOrdersShort;
	String^ FALtoken;
};