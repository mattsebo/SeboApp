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
		Platform::String^ timeLogToken;
		Platform::String^ requestedDate;
		int numSheetsCutOnDate;
		MachineCutLog^ CS40;
		MachineCutLog^ CS45;
		std::vector<std::wstring> ConfigSettings;
		void InitConfig();
		void InitMachineLogs(Windows::UI::Xaml::Controls::TextBox^ textbox, Windows::UI::Xaml::Controls::AutoSuggestBox^ searchbox);
		void ReadTimeLog();
		Platform::String^ GetRecentSheets(Platform::String^ file);
		void CountDatesFromFile(Platform::String^ file);
		void ExplodeFile(Platform::String^ file);
		void ProcessConfigString(Platform::String^ file);
		void SerializeConfig();
		void DoOnSearchEvent(Platform::String^ search, Windows::UI::Xaml::Controls::TextBox^ textbox);
		void Page_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void GetFilePickerToken(Platform::String^& token);
		void CS40Path_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void CS45Path_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void AutoSuggestBox_TextChanged(Windows::UI::Xaml::Controls::AutoSuggestBox^ sender, Windows::UI::Xaml::Controls::AutoSuggestBoxTextChangedEventArgs^ args);
		void AutoSuggestBox_QuerySubmitted(Windows::UI::Xaml::Controls::AutoSuggestBox^ sender, Windows::UI::Xaml::Controls::AutoSuggestBoxQuerySubmittedEventArgs^ args);
		void AutoSuggestBox_SuggestionChosen(Windows::UI::Xaml::Controls::AutoSuggestBox^ sender, Windows::UI::Xaml::Controls::AutoSuggestBoxSuggestionChosenEventArgs^ args);
		void SetTimeLog_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void MyCalendarPicker_SelectedDatesChanged(Windows::UI::Xaml::Controls::CalendarView^ sender, Windows::UI::Xaml::Controls::CalendarViewSelectedDatesChangedEventArgs^ args);
		void MyCalendarPicker_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
