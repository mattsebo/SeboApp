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
using namespace concurrency;
using namespace std;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409


MainPage::MainPage()
	:
	CS40token(""),
	CS45token("")
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
			//Example output: The system cannot find the specified file.
			// CREATE config.txt in localfolder.
			try
			{
				concurrency::create_task(storageFolder->CreateFileAsync("config.txt", CreationCollisionOption::GenerateUniqueName));
				CS40Path->Visibility = Windows::UI::Xaml::Visibility::Visible;
				CS45Path->Visibility = Windows::UI::Xaml::Visibility::Visible;
			}
			catch (COMException ^ex)
			{

			}
		}
	});
}

void SeboApp::MainPage::InitMachineLogs()
{
	CS40 = ref new MachineCutLog(searchTextBox->Text);
	CS45 = ref new MachineCutLog(searchTextBox->Text);

	/*StorageFolder^ picturesFolder = KnownFolders::PicturesLibrary;

	StorageFolderQueryResult^ queryResult =
		picturesFolder->CreateFolderQuery(CommonFolderQuery::GroupByMonth);

	// Use shared_ptr so that outputString remains in memory
	// until the task completes, which is after the function goes out of scope.
	auto outputString = std::make_shared<wstring>();

	create_task(queryResult->GetFoldersAsync()).then([this, outputString](IVectorView<StorageFolder^>^ view)
	{
		for (unsigned int i = 0; i < view->Size; i++)
		{
			create_task(view->GetAt(i)->GetFilesAsync()).then([this, i, view, outputString](IVectorView<StorageFile^>^ files)
			{
				*outputString += view->GetAt(i)->Name->Data();
				*outputString += L"(";
				*outputString += to_wstring(files->Size);
				*outputString += L")\r\n";
				for (unsigned int j = 0; j < files->Size; j++)
				{
					*outputString += L"     ";
					*outputString += files->GetAt(j)->Name->Data();
					*outputString += L"\r\n";
				}
			}).then([this, outputString]()
			{
				resultTextBox->Text = ref new String((*outputString).c_str());
			});
		}
	});*/

	/*// Be sure to specify the Pictures Folder capability in the appxmanifext file.
	StorageFolder^ picturesFolder = KnownFolders::PicturesLibrary;

	// Use a shared_ptr so that the string stays in memory
	// until the last task is complete.
	auto outputString = make_shared<wstring>();
	*outputString += L"Files:\n";

	// Get a read-only vector of the file objects
	// and pass it to the continuation.
	create_task(picturesFolder->GetFilesAsync())
		// outputString is captured by value, which creates a copy
		// of the shared_ptr and increments its reference count.
		.then([outputString](IVectorView<StorageFile^>^ files)
	{
		for (unsigned int i = 0; i < files->Size; i++)
		{
			*outputString += files->GetAt(i)->Name->Data();
			*outputString += L"\n";
		}
	})
		// We need to explicitly state the return type
		// here: -> IAsyncOperation<...>
		.then([picturesFolder]()->IAsyncOperation<IVectorView<StorageFolder^>^>^
	{
		return picturesFolder->GetFoldersAsync();
	})
		// Capture "this" to access m_OutputTextBlock from within the lambda.
		.then([this, outputString](IVectorView<StorageFolder^>^ folders)
	{
		*outputString += L"Folders:\n";

		for (unsigned int i = 0; i < folders->Size; i++)
		{
			*outputString += folders->GetAt(i)->Name->Data();
			*outputString += L"\n";
		}

		// Assume m_OutputTextBlock is a TextBlock defined in the XAML.
		resultTextBox->Text = ref new String((*outputString).c_str());
	});*/
	try
	{
		if (StorageApplicationPermissions::FutureAccessList->ContainsItem(CS40token))
		{
			create_task(StorageApplicationPermissions::FutureAccessList->GetFolderAsync(CS40token))
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
							auto query = subfolder->CreateFileQuery();
							wstring name = subfolder->Name->Begin();
							if (CS40->GetWorkOrder()->Begin() == name.substr(0, CS40->GetWorkOrder()->Length()))
							{
								create_task(query->GetFilesAsync()).then([=](IVectorView<StorageFile^>^ files)
								{
									// check each file within the subfolder
									for (auto file : files)
									{
										//String^ s = ref new String(file->Name->Begin());
										CS40->AddSheet(file->Name);
									}
								})
									.then([=](void)
								{
									if (CS40 != nullptr && CS45 != nullptr)
									{
										CS40->UpdateTextBox(resultTextBox, CS45);
										CS40->UpdateProgressBar(progressBar, CS45);
									}
								});
							}
						}
					});
				}
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
	try
	{
		if (StorageApplicationPermissions::FutureAccessList->ContainsItem(CS45token))
		{
			create_task(StorageApplicationPermissions::FutureAccessList->GetFolderAsync(CS45token))
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
							auto query = subfolder->CreateFileQuery();
							wstring name = subfolder->Name->Begin();
							if (CS45->GetWorkOrder()->Begin() == name.substr(0, CS45->GetWorkOrder()->Length()))
							{
								create_task(query->GetFilesAsync()).then([=](IVectorView<StorageFile^>^ files)
								{
									// check each file within the subfolder
									for (auto file : files)
									{
										String^ s = ref new String(file->Name->Begin());
										CS45->AddSheet(file->Name);
									}
								})
									.then([=](void)
								{
									if (CS40 != nullptr && CS45 != nullptr)
									{
										CS40->UpdateTextBox(resultTextBox, CS45);
										CS40->UpdateProgressBar(progressBar, CS45);
									}
								});
							}
						}
					});
				}
			});
		}
		else
		{
			resultTextBox->Text += L"CS45 Path not found!\n";
		}
	}
	catch (const exception& e)
	{
		
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
		if (delimiter == "\r\n")
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

void SeboApp::MainPage::RemoveBatchFromString(std::wstring & s)
{
	for (int i = s.length(); i > 0; i--)
	{
		if (s[i] == L'_')
		{
			s = s.substr(0, i);
			break;
		}
	}
}

bool SeboApp::MainPage::Contains(Platform::String ^ s, Platform::Collections::Vector<String^>^ vec)
{
	for (auto it : vec)
	{
		if (it == s)
		{
			return true;
		}
	}
	return false;
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
	}
	if (CS40token == "")
	{
		CS40Path->Visibility = Windows::UI::Xaml::Visibility::Visible;
	}
	if (CS45token == "")
	{
		CS45Path->Visibility = Windows::UI::Xaml::Visibility::Visible;
	}
}

void SeboApp::MainPage::SerializeConfig()
{
	// Save config file
	StorageFolder^ storageFolder = ApplicationData::Current->LocalFolder;
	create_task(storageFolder->GetFileAsync("config.txt")).then([=](StorageFile^ sampleFile)
	{
		//Write text to a file
		create_task(FileIO::WriteTextAsync(sampleFile, "CS40 Key = " + CS40token + "\r\n"));
		create_task(FileIO::AppendTextAsync(sampleFile, "CS45 Key = " + CS45token + "\r\n"));
	});
}

void SeboApp::MainPage::DoOnSearchEvent()
{
	if (CS40token == L"" || CS45token == L"")
	{
		resultTextBox->Text = L"Cannot find machine path.";
	}
	else
	{
		InitMachineLogs();
	}
}

void SeboApp::MainPage::Page_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	/*TimeSpan period;
	period.Duration = 4 * 10000000;

	try
	{
		ThreadPoolTimer^ PeriodicTimer = ThreadPoolTimer::CreatePeriodicTimer(
			ref new TimerElapsedHandler([this](ThreadPoolTimer^ source)
		{
			// Do stuff
			Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High,
				ref new Windows::UI::Core::DispatchedHandler([this]()
			{
				// Access UI elements
				if (CS40 != nullptr && CS45 != nullptr)
				{
					CS40->UpdateTextBox(resultTextBox, CS45);
					CS40->UpdateProgressBar(progressBar, CS45);
				}
			}));
		}), period);
	}
	catch (const exception& e)
	{

	}*/
}

void SeboApp::MainPage::CS40Path_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	FolderPicker^ picker = ref new FolderPicker();
	picker->FileTypeFilter->Append("*");
	picker->ViewMode = PickerViewMode::List;
	picker->SuggestedStartLocation = PickerLocationId::ComputerFolder;
	create_task(picker->PickSingleFolderAsync()).then([=](StorageFolder^ folder)
	{
		if (folder != nullptr)
		{
			// Query the folder.
			try
			{
				CS40token = StorageApplicationPermissions::FutureAccessList->Add(folder);
				SerializeConfig();
			}
			catch (COMException^ ex)
			{
				if (ex->HResult == FA_E_MAX_PERSISTED_ITEMS_REACHED)
				{
					// A real program would call Remove() to create room in the FAL.
					CS40token = "The folder '" + folder->Name + "' was not added to the FAL list because the FAL list is full.";
				}
				else
				{
					throw;
				}
			}
		}
	});
	CS40Path->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
}

void SeboApp::MainPage::CS45Path_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	FolderPicker^ picker = ref new FolderPicker();
	picker->FileTypeFilter->Append("*");
	picker->ViewMode = PickerViewMode::List;
	picker->SuggestedStartLocation = PickerLocationId::ComputerFolder;
	create_task(picker->PickSingleFolderAsync()).then([=](StorageFolder^ folder)
	{
		if (folder != nullptr)
		{
			// Query the folder.
			try
			{
				CS45token = StorageApplicationPermissions::FutureAccessList->Add(folder);
				SerializeConfig();
			}
			catch (COMException^ ex)
			{
				if (ex->HResult == FA_E_MAX_PERSISTED_ITEMS_REACHED)
				{
					// A real program would call Remove() to create room in the FAL.
					CS45token = "The folder '" + folder->Name + "' was not added to the FAL list because the FAL list is full.";
				}
				else
				{
					throw;
				}
			}
		}
	});
	CS45Path->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
}

void SeboApp::MainPage::AutoSuggestBox_TextChanged(Windows::UI::Xaml::Controls::AutoSuggestBox^ sender, Windows::UI::Xaml::Controls::AutoSuggestBoxTextChangedEventArgs^ args)
{
	if (args->Reason == AutoSuggestionBoxTextChangeReason::UserInput)
	{
		Vector<String^>^ match = ref new Vector<String^>();
		if (!CS45token->IsEmpty() && StorageApplicationPermissions::FutureAccessList->ContainsItem(CS45token))
			{
				create_task(StorageApplicationPermissions::FutureAccessList->GetFolderAsync(CS45token)).then([=](StorageFolder^ folder)
				{ // Task to query base path
					if (folder != nullptr)
					{
						// Query the folder.
						auto query2 = folder->CreateFolderQuery();
						create_task(query2->GetFoldersAsync()).then([=](IVectorView<StorageFolder^>^ subfolders)-> Vector<String^>^
						 {
							Vector<String^>^ vec = ref new Vector<String^>();
							// Query the subfolders.
							for (auto subfolder : subfolders)
							{
								wstring name = subfolder->Name->Begin();
								RemoveBatchFromString(name);
								if (sender->Text->Begin() == name.substr(0, sender->Text->Length()))
								{
									Platform::String^ s = ref new String(name.c_str());
									if (!Contains(s, vec))
									{
										vec->Append(s);
									}
								}
							}
							return vec;
						}).then([=](Vector<String^>^ newVec) {
							//match->Clear();
							for (auto it : newVec)
							{
								bool exists = false;
								for (auto it2 : match)
								{
									if (it2 == it)
									{
										exists = true;
									}
								}
								if (!exists)
								{
									match->Append(it);
								}
							}
						});
					}
				});
			}
			else
			{
				resultTextBox->Text = L"Machine path not found!\n";
			}
		if (!CS40token->IsEmpty() && StorageApplicationPermissions::FutureAccessList->ContainsItem(CS40token))
		{
			create_task(StorageApplicationPermissions::FutureAccessList->GetFolderAsync(CS40token)).then([=](StorageFolder^ folder)
			{ // Task to query base path
				if (folder != nullptr)
				{
					// Query the folder.
					auto query2 = folder->CreateFolderQuery();
					create_task(query2->GetFoldersAsync()).then([=](IVectorView<StorageFolder^>^ subfolders)-> Vector<String^>^
					{
						Vector<String^>^ vec = ref new Vector<String^>();
						// Query the subfolders.
						for (auto subfolder : subfolders)
						{
							wstring name = subfolder->Name->Begin();
							RemoveBatchFromString(name);
							if (sender->Text->Begin() == name.substr(0, sender->Text->Length()))
							{
								Platform::String^ s = ref new String(name.c_str());
								if (!Contains(s, vec))
								{
									vec->Append(s);
								}
							}
						}
						return vec;
					}).then([=](Vector<String^>^ newVec) {
						//match->Clear();
						for (auto it : newVec)
						{
							bool exists = false;
							for (auto it2 : match)
							{
								if (it2 == it)
								{
									exists = true; 
								}
							}
							if (!exists)
							{
								match->Append(it);
							}
						}
					});
				}
			});
		}
		else
		{
			resultTextBox->Text = L"Machine path not found!\n";
		}
		sender->ItemsSource = match;
	}
}

void SeboApp::MainPage::AutoSuggestBox_QuerySubmitted(Windows::UI::Xaml::Controls::AutoSuggestBox^ sender, Windows::UI::Xaml::Controls::AutoSuggestBoxQuerySubmittedEventArgs^ args)
{
	searchTextBox->Text = args->ChosenSuggestion->ToString();
	//searchTextBox->Text = sender->Text;
	DoOnSearchEvent();
}

void SeboApp::MainPage::AutoSuggestBox_SuggestionChosen(Windows::UI::Xaml::Controls::AutoSuggestBox^ sender, Windows::UI::Xaml::Controls::AutoSuggestBoxSuggestionChosenEventArgs^ args)
{
	searchTextBox->Text = args->SelectedItem->ToString();
	//searchTextBox->Text = sender->Text;
	DoOnSearchEvent();
}
