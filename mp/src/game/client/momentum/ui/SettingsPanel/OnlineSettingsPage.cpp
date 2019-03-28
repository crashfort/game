#include "cbase.h"

#include "OnlineSettingsPage.h"
#include "vgui_controls/CvarSlider.h"
#include "vgui_controls/TextEntry.h"
#include <vgui_controls/CvarToggleCheckButton.h>

#include "tier0/memdbgon.h"

using namespace vgui;

OnlineSettingsPage::OnlineSettingsPage(Panel* pParent) : BaseClass(pParent, "OnlineSettings")
{
    m_pEnableColorAlphaOverride = new CvarToggleCheckButton(this, "EnableAlphaOverride", "#MOM_Settings_Override_Alpha_Enable", "mom_ghost_online_alpha_override_enable");
    m_pEnableColorAlphaOverride->AddActionSignalTarget(this);
    m_pAlphaOverrideSlider = new CvarSlider(this, "AlphaOverrideSlider", nullptr, 0.0f, 255.0f, "mom_ghost_online_alpha_override");
    m_pAlphaOverrideSlider->AddActionSignalTarget(this);
    m_pAlphaOverrideInput = new TextEntry(this, "AlphaOverrideEntry");
    m_pAlphaOverrideInput->SetAllowNumericInputOnly(true);
    m_pAlphaOverrideInput->AddActionSignalTarget(this);

    LoadControlSettings("resource/ui/SettingsPanel_OnlineSettings.res");
}

void OnlineSettingsPage::OnApplyChanges()
{
    BaseClass::OnApplyChanges();

    
}

void OnlineSettingsPage::LoadSettings()
{

    UpdateSliderSettings();
}

void OnlineSettingsPage::OnCheckboxChecked(Panel* p)
{
}

void OnlineSettingsPage::OnTextChanged(vgui::Panel* panel)
{
    BaseClass::OnTextChanged(panel);

    if (panel == m_pAlphaOverrideInput)
    {
        char buf[64];
        m_pAlphaOverrideInput->GetText(buf, 64);

        float fValue = static_cast<float>(Q_atof(buf));
        if (fValue >= 0.0f && fValue <= 255.0f)
        {
            m_pAlphaOverrideSlider->SetSliderValue(fValue);
        }
    }
}

void OnlineSettingsPage::OnControlModified(vgui::Panel* p)
{
    BaseClass::OnControlModified(p);

    if (p == m_pAlphaOverrideSlider)
    {
        UpdateSliderSettings();
    }
}

void OnlineSettingsPage::UpdateSliderSettings()
{
    char buf[64];
    Q_snprintf(buf, sizeof(buf), "%i", static_cast<int>(m_pAlphaOverrideSlider->GetSliderValue()));
    m_pAlphaOverrideInput->SetText(buf);
}
