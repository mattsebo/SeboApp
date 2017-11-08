//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "MachineCutLog.h"
#include <ppltasks.h>
#include <threadpoolapiset.h>

using namespace SeboApp;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Platform::Collections;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage::AccessCache;
using namespace Windows::System::Threading;
using namespace Windows::Storage::Search;
using namespace Windows::Globalization::DateTimeFormatting;
using namespace concurrency;
using namespace std;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409


MainPage::MainPage()
	:
	CS40token(L""),
	CS45token(L""),
	timeLogToken(L""),
	requestedDate(L""),
	numSheetsCutOnDate(0)
{
	InitializeComponent();
	InitConfig();
}

void SeboApp::MainPage::InitConfig()
{
	// Reads contents of config file.
	StorageFolder^ storageFolder = ApplicationData::Current->LocalFolder;
	auto getFileTask = concurrency::create_task(storageFolder->GetFileAsync("config.txt"));
	getFileTask.then([this, storageFolder](StorageFile^ file)
	{
		// Process file
		auto readFileTask = create_task(FileIO::ReadTextAsync(file));
		readFileTask.then([this, file](task<String^> task)
		{
			String^ fileContent = task.get();
			ProcessConfigString(fileContent);
		}).then([this](void)
		{
			// If tokens are not saved in the file, show buttons to set paths.
			if (CS40token->IsEmpty())
			{
				CS40Path->Visibility = Windows::UI::Xaml::Visibility::Visible;
			}
			if (CS45token->IsEmpty())
			{
				CS45Path->Visibility = Windows::UI::Xaml::Visibility::Visible;
			}
			if (timeLogToken->IsEmpty())
			{
				SetTimeLog->Visibility = Windows::UI::Xaml::Visibility::Visible;
			}
		});
	}).then([this, storageFolder](task<void> t)
	{
		// Error handling
		try
		{
			t.get();
			// .get() didn' t throw, so we succeeded.
		}
		catch (Platform::COMException^ e)
		{
			// CREATE config.txt in localfolder.
			try
			{
				concurrency::create_task(storageFolder->CreateFileAsync("config.txt", CreationCollisionOption::GenerateUniqueName));
				CS40Path->Visibility = Windows::UI::Xaml::Visibility::Visible;
				CS45Path->Visibility = Windows::UI::Xaml::Visibility::Visible;
				SetTimeLog->Visibility = Windows::UI::Xaml::Visibility::Visible;
			}
			catch (COMException ^ex)
			{

			}
		}
	});
}

void SeboApp::MainPage::InitMachineLogs(Windows::UI::Xaml::Controls::TextBox^ textbox, Windows::UI::Xaml::Controls::AutoSuggestBox^ searchbox)
{
	CS40 = ref new MachineCutLog(CS40token);
	CS45 = ref new MachineCutLog(CS45token);
	create_task([this,searchbox, textbox](void){
		if (!CS45token->IsEmpty())
		{
			CS45->Init(textbox, searchbox, this->Dispatcher);
		}
	}).then([this, searchbox, textbox](void) {
		create_task([this, searchbox, textbox](void) {
			if (!CS40token->IsEmpty())
			{
				CS40->Init(textbox, searchbox, this->Dispatcher);
			}
		});
	});
}

void SeboApp::MainPage::ReadTimeLog()
{
	numSheetsCutOnDate = 0;
	try
	{
		if (StorageApplicationPermissions::FutureAccessList->ContainsItem(timeLogToken))
		{
			create_task(StorageApplicationPermissions::FutureAccessList->GetFolderAsync(timeLogToken))
				.then([=](StorageFolder^ folder)
			{ // Task to query base path
				create_task(folder->GetFilesAsync()).then([=](IVectorView<StorageFile^>^ files)
				{
					// check each file within the subfolder
					for (auto file : files)
					{
						create_task(FileIO::ReadTextAsync(file)).then([this, file](task<String^> task)
						{
							try
							{
								String^ fileContent = task.get();
								CountDatesFromFile(fileContent);
							}
							catch (COMException^ ex)
							{
								
							}
						}).then([=](void)
						{
							NumCutSheets->Text = numSheetsCutOnDate.ToString();
						});
					}
				});
			});
		}
		else
		{
			resultTextBox->Text += L"CS40 Path not found!\n";
		}
	}
	catch (const exception& e)
	{

	}
}

void SeboApp::MainPage::CountDatesFromFile(Platform::String^ file)
{
	std::wstring stdFile = file->Begin();
	for (int i = 0; i < file->Length() - requestedDate->Length() + 4; ++i)
	{
		std::wstring s = stdFile.substr(i, requestedDate->Length() - 4).c_str();
		std::wstring date = requestedDate->Begin();
		std::wstring dt;
		for (int j = 0; j < requestedDate->Length(); ++j)
		{
			if (date.at(j) < 200)
			{
				dt += date.at(j);
			}
		}
		int compare = s.compare(dt);
		if (compare == 0)
		{
			numSheetsCutOnDate++;
		}
	}
}

void SeboApp::MainPage::ExplodeFile(Platform::String^ file)
{
	// Separates the contents of the config file by line.
	// Each line will be stored into ConfigSettings vector.
	std::wstring line;
	for (auto it = file->Begin(); it != file->End(); it++)
	{
		String^ delimiter;
		for (int i = 0; i < 2; i++)
		{
			delimiter += (*(it + i)).ToString();
		}
		if (delimiter == L"\r\n")
		{
			ConfigSettings.push_back(line);
			line = L"";
			it = it + 1;
		}
		else
		{
			line += (*it);
		}
		if ((it + 1) == file->End())
		{
			ConfigSettings.push_back(line);
		}
	}
}

void SeboApp::MainPage::ProcessConfigString(Platform::String ^ file)
{
	// Loop through ConfigSettings and do stuff with the contents.
	ExplodeFile(file);
	for (auto it = ConfigSettings.begin(); it != ConfigSettings.end(); it++)
	{
		if (it->substr(0, 11) == L"CS40 Key = ")
		{
			CS40token = ref new Platform::String((it->substr(11, it->length() - 1)).c_str());
		} 
		else if (it->substr(0, 11) == L"CS45 Key = ")
		{
			CS45token = ref new Platform::String((it->substr(11, it->length() - 1)).c_str());
		}
		else if (it->substr(0, 14) == L"Timelog Key = ")
		{
			timeLogToken = ref new Platform::String((it->substr(14, it->length() - 1)).c_str());
		}
	}
	if (CS40token == L"")
	{
		CS40Path->Visibility = Windows::UI::Xaml::Visibility::Visible;
	}
	if (CS45token == L"")
	{
		CS45Path->Visibility = Windows::UI::Xaml::Visibility::Visible;
	}
	if (timeLogToken == L"")
	{
		SetTimeLog->Visibility = Windows::UI::Xaml::Visibility::Visible;
	}
}

void SeboApp::MainPage::SerializeConfig()
{
	// Save config file
	StorageFolder^ storageFolder = ApplicationData::Current->LocalFolder;
	create_task(storageFolder->GetFileAsync("config.txt")).then([=](StorageFile^ sampleFile)
	{
		//Write text to a file
		create_task(FileIO::WriteTextAsync(sampleFile, "CS40 Key = " + CS40token + "\r\n"))
			.then([sampleFile, this](void) {
			create_task(FileIO::AppendTextAsync(sampleFile, "CS45 Key = " + CS45token + "\r\n"));
		})
			.then([sampleFile, this](void) {
			create_task(FileIO::AppendTextAsync(sampleFile, "Timelog Key = " + timeLogToken + "\r\n"));
		});
	});
}

void SeboApp::MainPage::DoOnSearchEvent(Platform::String^ search, Windows::UI::Xaml::Controls::TextBox^ textbox)
{
	if (CS40token == L"" || CS45token == L"")
	{
		resultTextBox->Text = L"Cannot find machine path.";
	}
	else if (!CS45->KeyExists(search))
	{
		resultTextBox->Text = L"Cannot find that job, try again.";
	}
	else
	{
		CS45->InitSheets(search, CS40);
		TimeSpan period;
		period.Duration = (1 * 10000000); // 10 million tickss per second
		try
		{
			ThreadPoolTimer^ PeriodicTimer = ThreadPoolTimer::CreateTimer(
				ref new TimerElapsedHandler([this](ThreadPoolTimer^ source)
			{
				// Do stuff
				Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High,
					ref new Windows::UI::Core::DispatchedHandler([this]()
				{
					// Access UI elements
					CS45->EchoSheetsToCut(resultTextBox, searchTextBox->Text, progressBar, CS40);
				}));
			}), period);
		}
		catch (const exception& e)
		{

		}
	}
}

void SeboApp::MainPage::Page_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	TimeSpan period;
	TimeSpan refreshDataPeriod;
	period.Duration = 1 * 10000000; // 10 million ticks per second
	refreshDataPeriod.Duration = 3 * (60 * 10000000);
	try // delay start of reading machine logs, to allow config to be read.
	{
		ThreadPoolTimer::CreateTimer(
			ref new TimerElapsedHandler([this](ThreadPoolTimer^ source)
		{
			// Do stuff
			Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High,
				ref new Windows::UI::Core::DispatchedHandler([this]()
			{
				// Access UI elements
				InitMachineLogs(resultTextBox, searchTextBox);
			}));
		}), period);
	}
	catch (const exception& e)
	{

	}
	try // periodic timer to update data.
	{
		ThreadPoolTimer::CreatePeriodicTimer(
			ref new TimerElapsedHandler([this](ThreadPoolTimer^ source)
		{
			// Do stuff
			Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High,
				ref new Windows::UI::Core::DispatchedHandler([this]()
			{
				// Access UI elements
				if (timeLogToken != nullptr)
				{
					ReadTimeLog();
				}
				InitMachineLogs(resultTextBox, searchTextBox);
			}));
		}), refreshDataPeriod);
	}
	catch (const exception& e)
	{

	}
}

void SeboApp::MainPage::GetFilePickerToken(Platform::String^& token)
{
	this->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
		ref new Windows::UI::Core::DispatchedHandler([&]()
	{
		FolderPicker^ picker = ref new FolderPicker();
		picker->FileTypeFilter->Append("*");
		picker->ViewMode = PickerViewMode::List;
		picker->SuggestedStartLocation = PickerLocationId::ComputerFolder;
		create_task(picker->PickSingleFolderAsync()).then([=, &token](StorageFolder^ folder)
		{
			if (folder != nullptr)
			{
				// Query the folder.
				try
				{
					token = StorageApplicationPermissions::FutureAccessList->Add(folder);
					SerializeConfig();
				}
				catch (COMException^ ex)
				{
					if (ex->HResult == FA_E_MAX_PERSISTED_ITEMS_REACHED)
					{
						// A real program would call Remove() to create room in the FAL.
						token = "The folder '" + folder->Name + "' was not added to the FAL list because the FAL list is full.";
					}
					else
					{
						throw;
					}
				}
			}
		});
	}));
}

void SeboApp::MainPage::CS40Path_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	GetFilePickerToken(CS40token);
	CS40Path->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
}

void SeboApp::MainPage::CS45Path_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	GetFilePickerToken(CS45token);
	CS45Path->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
}

void SeboApp::MainPage::AutoSuggestBox_TextChanged(Windows::UI::Xaml::Controls::AutoSuggestBox^ sender, Windows::UI::Xaml::Controls::AutoSuggestBoxTextChangedEventArgs^ args)
{
	Vector<String^>^ vec = ref new Vector<String^>();
	for (auto it : CS45->GetWorkOrdersShort())
	{
		wstring name = it->Key->Begin();
		if (sender->Text->Begin() == name.substr(0, sender->Text->Length()))
		{
			vec->Append(it->Key);
		}
	}
	if (args->Reason == AutoSuggestionBoxTextChangeReason::UserInput)
	{
		if (vec->Size == 0)
		{
			vec->Append(L"No Results");
		}
		sender->ItemsSource = vec;
	}
}

void SeboApp::MainPage::AutoSuggestBox_QuerySubmitted(Windows::UI::Xaml::Controls::AutoSuggestBox^ sender, Windows::UI::Xaml::Controls::AutoSuggestBoxQuerySubmittedEventArgs^ args)
{
	if (args->QueryText == L"No Results")
	{
		searchTextBox->Text = L"";
	}
	else
	{
		searchTextBox->Text = args->QueryText;
	}
	DoOnSearchEvent(searchTextBox->Text, resultTextBox);
}

void SeboApp::MainPage::AutoSuggestBox_SuggestionChosen(Windows::UI::Xaml::Controls::AutoSuggestBox^ sender, Windows::UI::Xaml::Controls::AutoSuggestBoxSuggestionChosenEventArgs^ args)
{
	if (args->SelectedItem->ToString() == L"No Results")
	{
		searchTextBox->Text = L"";
	}
	else
	{
		searchTextBox->Text = args->SelectedItem->ToString();
	}
	DoOnSearchEvent(searchTextBox->Text, resultTextBox);
}

void SeboApp::MainPage::SetTimeLog_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	GetFilePickerToken(timeLogToken);
	SetTimeLog->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
}


void SeboApp::MainPage::MyCalendarPicker_SelectedDatesChanged(Windows::UI::Xaml::Controls::CalendarView^ sender, Windows::UI::Xaml::Controls::CalendarViewSelectedDatesChangedEventArgs^ args)
{
	DateTimeFormatter^ dateFormatter = ref new DateTimeFormatter(L"‎‎{month.integer(2)}/{day.integer(2)}/‎‎{year.abbreviated}");
	Windows::Foundation::DateTime dateToFormat = Windows::Foundation::DateTime(args->AddedDates->First()->Current);
	requestedDate = dateFormatter->Format(dateToFormat);
	if (timeLogToken != nullptr)
	{
		ReadTimeLog();
	}
}


void SeboApp::MainPage::MyCalendarPicker_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	TimeSpan period;
	period.Duration = 1 * 10000000; // 10 million tickss per second
	try
	{
		ThreadPoolTimer^ PeriodicTimer = ThreadPoolTimer::CreateTimer(
			ref new TimerElapsedHandler([this](ThreadPoolTimer^ source)
		{
			// Do stuff
			Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High,
				ref new Windows::UI::Core::DispatchedHandler([this]()
			{
				// Access UI elements
				Windows::Globalization::Calendar^ c = ref new Windows::Globalization::Calendar;
				c->SetToNow();
				DateTimeFormatter^ dateFormatter = ref new DateTimeFormatter(L"‎‎{month.integer(2)}/{day.integer(2)}/‎‎{year.abbreviated}");
				DateTime dateToFormat = c->GetDateTime();
				requestedDate = dateFormatter->Format(dateToFormat);
				if (timeLogToken != nullptr)
				{
					ReadTimeLog();
				}
			}));
		}), period);
	}
	catch (const exception& e)
	{

	}
}
