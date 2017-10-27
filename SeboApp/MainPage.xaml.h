//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include <vector>
#include "MachineCutLog.h"

namespace SeboApp
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();
	private:
		Platform::String^ CS40token;
		Platform::String^ CS45token;
		MachineCutLog^ CS40;
		MachineCutLog^ CS45;
		std::vector<std::wstring> ConfigSettings;
		void InitConfig();
		void InitMachineLogs();
		void ExplodeFile(Platform::String^ file);
		void RemoveBatchFromString(std::wstring& s);
		bool Contains(Platform::String^ s, Platform::Collections::Vector<String^>^ vec);
		void ProcessConfigString(Platform::String^ file);
		void SerializeConfig();
		void DoOnSearchEvent();
		void Page_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void CS40Path_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void CS45Path_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void AutoSuggestBox_TextChanged(Windows::UI::Xaml::Controls::AutoSuggestBox^ sender, Windows::UI::Xaml::Controls::AutoSuggestBoxTextChangedEventArgs^ args);
		void AutoSuggestBox_QuerySubmitted(Windows::UI::Xaml::Controls::AutoSuggestBox^ sender, Windows::UI::Xaml::Controls::AutoSuggestBoxQuerySubmittedEventArgs^ args);
		void AutoSuggestBox_SuggestionChosen(Windows::UI::Xaml::Controls::AutoSuggestBox^ sender, Windows::UI::Xaml::Controls::AutoSuggestBoxSuggestionChosenEventArgs^ args);
	};
}
