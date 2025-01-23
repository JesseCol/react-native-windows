
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "ContentIslandComponentView.h"

#include <Fabric/FabricUIManagerModule.h>
#include <IReactContext.h>
#include <UI.Xaml.Controls.h>
#include <Utils/ValueUtils.h>
#include <winrt/Microsoft.UI.Content.h>
#include <winrt/Microsoft.UI.Input.h>
#include <winrt/Windows.UI.Composition.h>
#include "CompositionContextHelper.h"
#include "RootComponentView.h"

#include "Composition.ContentIslandComponentView.g.cpp"

#include "CompositionDynamicAutomationProvider.h"

namespace winrt::Microsoft::ReactNative::Composition::implementation {

ContentIslandComponentView::ContentIslandComponentView(
    const winrt::Microsoft::ReactNative::Composition::Experimental::ICompositionContext &compContext,
    facebook::react::Tag tag,
    winrt::Microsoft::ReactNative::ReactContext const &reactContext,
    ReactCompositionViewComponentBuilder *builder)
    : base_type(
          ViewComponentView::defaultProps(),
          compContext,
          tag,
          reactContext,
          ComponentViewFeatures::Default,
          builder) {
  m_mountedToken = Mounted([](const winrt::IInspectable &, const winrt::Microsoft::ReactNative::ComponentView &view) {
    view.as<ContentIslandComponentView>()->OnMounted();
  });
  m_unmountedToken =
      Unmounted([](const winrt::IInspectable &, const winrt::Microsoft::ReactNative::ComponentView &view) {
        view.as<ContentIslandComponentView>()->OnUnmounted();
      });
}

void ContentIslandComponentView::OnMounted() noexcept {
#ifdef USE_EXPERIMENTAL_WINUI3
  m_childSiteLink = winrt::Microsoft::UI::Content::ChildSiteLink::Create(
      rootComponentView()->parentContentIsland(),
      winrt::Microsoft::ReactNative::Composition::Experimental::CompositionContextHelper::InnerVisual(Visual())
          .as<winrt::Microsoft::UI::Composition::ContainerVisual>());
  m_childSiteLink.ActualSize({m_layoutMetrics.frame.size.width, m_layoutMetrics.frame.size.height});

  // Setting this option before connecting the child content seems to be important to ensure the UIA tree
  // is correctly set up.
  m_childSiteLink.AutomationTreeOption(winrt::Microsoft::UI::Content::AutomationTreeOptions::FragmentBased);

  if (m_islandToConnect) {
    m_childSiteLink.Connect(m_islandToConnect);
    m_islandToConnect = nullptr;
  }

  ParentLayoutChanged();
  auto view = Parent();
  while (view) {
    m_layoutMetricChangedRevokers.push_back(view.LayoutMetricsChanged(
        winrt::auto_revoke,
        [wkThis = get_weak()](
            const winrt::IInspectable &sender, const winrt::Microsoft::ReactNative::LayoutMetricsChangedArgs &args) {
          if (auto strongThis = wkThis.get()) {
            strongThis->ParentLayoutChanged();
          }
        }));
    view = view.Parent();
  }
#endif
}

void ContentIslandComponentView::OnUnmounted() noexcept {
  m_layoutMetricChangedRevokers.clear();
  m_uiaProvider = nullptr;
}

void ContentIslandComponentView::ParentLayoutChanged() noexcept {
#ifdef USE_EXPERIMENTAL_WINUI3
  if (m_layoutChangePosted)
    return;

  m_layoutChangePosted = true;
  ReactContext().UIDispatcher().Post([wkThis = get_weak()]() {
    if (auto strongThis = wkThis.get()) {
      auto clientRect = strongThis->getClientRect();
      strongThis->m_childSiteLink
          .LocalToParentTransformMatrix(
              winrt::Windows::Foundation::Numerics::make_float3x2_translation(static_cast<float>(clientRect.left), static_cast<float>(clientRect.top)));

      strongThis->m_layoutChangePosted = false;
    }
  });
#endif
}

ContentIslandComponentView::~ContentIslandComponentView() noexcept {
  if (m_islandToConnect) {
    m_islandToConnect.Close();
  }
}

void ContentIslandComponentView::MountChildComponentView(
    const winrt::Microsoft::ReactNative::ComponentView &childComponentView,
    uint32_t index) noexcept {
  assert(false);
  base_type::MountChildComponentView(childComponentView, index);
}

void ContentIslandComponentView::UnmountChildComponentView(
    const winrt::Microsoft::ReactNative::ComponentView &childComponentView,
    uint32_t index) noexcept {
  assert(false);
  base_type::UnmountChildComponentView(childComponentView, index);
}

void ContentIslandComponentView::updateLayoutMetrics(
    facebook::react::LayoutMetrics const &layoutMetrics,
    facebook::react::LayoutMetrics const &oldLayoutMetrics) noexcept {
#ifdef USE_EXPERIMENTAL_WINUI3
  if (m_childSiteLink) {
    m_childSiteLink.ActualSize({layoutMetrics.frame.size.width, layoutMetrics.frame.size.height});
    ParentLayoutChanged();
  }
#endif
  base_type::updateLayoutMetrics(layoutMetrics, oldLayoutMetrics);
}

void ContentIslandComponentView::Connect(const winrt::Microsoft::UI::Content::ContentIsland &contentIsland) noexcept {
#ifdef USE_EXPERIMENTAL_WINUI3
  if (m_childSiteLink) {
    m_islandToConnect = nullptr;
    m_childSiteLink.Connect(contentIsland);
  } else {
    m_islandToConnect = contentIsland;
  }
#endif
}

void ContentIslandComponentView::prepareForRecycle() noexcept {
  Super::prepareForRecycle();
}

winrt::IInspectable ContentIslandComponentView::EnsureUiaProvider() noexcept {
  if (m_uiaProvider == nullptr) {
    m_uiaProvider =
        winrt::make<winrt::Microsoft::ReactNative::implementation::CompositionDynamicAutomationProvider>(
          *get_strong(), m_childContentLink);
  }
  return m_uiaProvider;
}

bool ContentIslandComponentView::focusable() const noexcept {
  /* TODO: Check to see if we actually have focusable content in the ContentIsland.
  if (m_childContentLink) {
    auto navigationHost = winrt::Microsoft::UI::Input::InputFocusNavigationHost::GetForSiteBridge(m_childContentLink);
    auto request = winrt::Microsoft::UI::Input::FocusNavigationRequest::Create(
        winrt::Microsoft::UI::Input::FocusNavigationReason::First);
    navigationHost.NavigateFocus(request);
  }
  */
  return true;
}

} // namespace winrt::Microsoft::ReactNative::Composition::implementation
