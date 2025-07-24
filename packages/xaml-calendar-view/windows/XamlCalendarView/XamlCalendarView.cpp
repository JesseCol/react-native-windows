#include "pch.h"

#include "XamlCalendarView.h"

#include <winrt/Microsoft.ReactNative.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace winrt::XamlCalendarView
{

struct CalendarView : public winrt::implements<CalendarView, winrt::Microsoft::ReactNative::Xaml::IXamlControl, winrt::IInspectable>, 
                    public Codegen::BaseCalendarView<CalendarView> {

  winrt::Microsoft::UI::Xaml::UIElement GetXamlElement() {
    // if (m_calendarView == nullptr) {
    //   CreateXamlCalendarView();
    // }
    // return m_calendarView;

    if (m_comboBox == nullptr) {
      CreateXamlDatePicker();
    }
    return m_stackPanel;

  }

  void UpdateProps(
    const winrt::Microsoft::ReactNative::ComponentView &view,
    const winrt::com_ptr<Codegen::CalendarViewProps> &newProps,
    const winrt::com_ptr<Codegen::CalendarViewProps> &oldProps) noexcept override {
    Codegen::BaseCalendarView<CalendarView>::UpdateProps(view, newProps, oldProps);
    
    if (m_calendarView && newProps) {
      m_calendarView.DisplayMode(static_cast<winrt::Microsoft::UI::Xaml::Controls::CalendarViewDisplayMode>(
          newProps->displayMode));
    }
  }

  void CreateXamlDatePicker() {
    m_comboBox = winrt::Microsoft::UI::Xaml::Controls::ComboBox();
    m_datePicker = winrt::Microsoft::UI::Xaml::Controls::DatePicker();

    // Add 10 items to the ComboBox
    for (int i = 0; i < 50; i++) {
      winrt::Microsoft::UI::Xaml::Controls::ComboBoxItem item;
      item.Content(winrt::box_value(L"Item " + winrt::to_hstring(i)));
      m_comboBox.Items().Append(item);
    }

    

    m_stackPanel = winrt::Microsoft::UI::Xaml::Controls::StackPanel();
    m_stackPanel.Orientation(winrt::Microsoft::UI::Xaml::Controls::Orientation::Horizontal);
    m_comboBox.Margin(winrt::Microsoft::UI::Xaml::Thickness(10, 10, 10, 10));
    m_stackPanel.Children().Append(m_comboBox);

    m_datePicker.Margin(winrt::Microsoft::UI::Xaml::Thickness(10, 10, 10, 10));
    m_stackPanel.Children().Append(m_datePicker);

    if (Props()) {
    }
    //m_comboBox.DateChanged([this](auto &&, auto &&) {
    //});
  }

  void CreateXamlCalendarView() {
    m_calendarView = winrt::Microsoft::UI::Xaml::Controls::CalendarView();
    if (Props()) {
      m_calendarView.DisplayMode(
        static_cast<winrt::Microsoft::UI::Xaml::Controls::CalendarViewDisplayMode>(
        Props()->displayMode));
    }
    m_calendarView.SelectedDatesChanged([this](auto &&, auto &&) {
      if (auto emitter = EventEmitter()) {
        Codegen::CalendarView_OnSelectedDatesChanged args;
        auto selectedDates = m_calendarView.SelectedDates();
        if (selectedDates.Size() == 0) {
          args.startDate = "(none)";
        } else {
          auto firstSelectedDate = selectedDates.GetAt(0);

          auto tt = winrt::clock::to_time_t(firstSelectedDate);
          tm local{};
          localtime_s(&local, &tt);
          auto timeStr = std::put_time(&local, "%F");

          std::stringstream ss;
          ss << (timeStr._Tptr->tm_year + 1900) << "-" << (timeStr._Tptr->tm_mon + 1) << "-" << timeStr._Tptr->tm_mday;

          args.startDate = ss.str();
        }
        emitter->onSelectedDatesChanged(args);
      }
    });
  }


  private:
    winrt::Microsoft::UI::Xaml::Controls::CalendarView m_calendarView{ nullptr };
    winrt::Microsoft::UI::Xaml::Controls::ComboBox m_comboBox{ nullptr };
    winrt::Microsoft::UI::Xaml::Controls::StackPanel m_stackPanel{ nullptr };
    winrt::Microsoft::UI::Xaml::Controls::DatePicker m_datePicker{ nullptr };
};


// See https://microsoft.github.io/react-native-windows/docs/native-modules for details on writing native modules

void XamlCalendarView::Initialize(React::ReactContext const &reactContext) noexcept {
  m_context = reactContext;
}

double XamlCalendarView::multiply(double a, double b) noexcept {
  return a * b;
}

void RegisterCalendarViewNativeComponent(
    winrt::Microsoft::ReactNative::IReactPackageBuilder const &packageBuilder) noexcept {
  Codegen::RegisterCalendarViewNativeComponent<winrt::XamlCalendarView::CalendarView>(packageBuilder, {});
}


} // namespace winrt::XamlCalendarView
