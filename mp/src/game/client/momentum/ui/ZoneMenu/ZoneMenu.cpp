#include "cbase.h"
#include "ZoneMenu.h"
#include "clientmode.h"
#include "icliententitylist.h"
#include "mom_player_shared.h"
#include "util\mom_util.h"
#include <usermessages.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/CvarSlider.h>
#include <vgui_controls/CvarTextEntry.h>

#include "tier0/memdbgon.h"

using namespace vgui;

C_MomZoneMenu *g_pZoneMenu = nullptr;

CON_COMMAND(mom_show_zonemenu, "Shows zoning menu")
{
    if (!g_pZoneMenu)
    {
        g_pZoneMenu = new C_MomZoneMenu(g_pClientMode->GetViewport());
    }
    g_pZoneMenu->Activate();
}

C_MomZoneMenu::C_MomZoneMenu(Panel *pParentPanel) : Frame(pParentPanel, "ZoneMenu")
{
    usermessages->HookMessage("ZoneInfo", OnZoneInfoThunk);

    m_bBindKeys = false;
    m_eZoneAction = ZONEACTION_NONE;

    m_pEditorTitleLabel = new Label(this, "ZoneMenuEditorLabel", "");

    m_pCreateNewZoneButton = new Button(this, "CreateNewZoneButton", "");
    m_pDeleteZoneButton    = new Button(this, "DeleteZoneButton", "");
    m_pEditZoneButton      = new Button(this, "EditZoneButton", "");
    m_pCancelZoneButton    = new Button(this, "CancelZoneButton", "");
    m_pSaveZonesButton     = new Button(this, "SaveZonesButton", "");

    m_pCreateNewZoneButton->SetCommand(new KeyValues("CreateNewZone"));
    m_pDeleteZoneButton->SetCommand(new KeyValues("DeleteZone"));
    m_pEditZoneButton->SetCommand(new KeyValues("EditZone"));
    m_pCancelZoneButton->SetCommand(new KeyValues("CancelZone"));
    m_pSaveZonesButton->SetCommand(new KeyValues("SaveZones"));

    m_pZoneTypeLabel = new Label(this, "ZoneTypeLabel", "");
    m_pZoneTypeCombo = new ComboBox(this, "ZoneTypeCombo", 7, false);
    m_pZoneTypeCombo->AddItem("Auto",        new KeyValues("vals", "zone_type", "auto",  "bonus", "0"));
    m_pZoneTypeCombo->AddItem("Start",       new KeyValues("vals", "zone_type", "start", "bonus", "0"));
    m_pZoneTypeCombo->AddItem("End",         new KeyValues("vals", "zone_type", "end",   "bonus", "0"));
    m_pZoneTypeCombo->AddItem("Bonus Start", new KeyValues("vals", "zone_type", "start", "bonus", "1"));
    m_pZoneTypeCombo->AddItem("Bonus End",   new KeyValues("vals", "zone_type", "end",   "bonus", "1"));
    m_pZoneTypeCombo->AddItem("Stage",       new KeyValues("vals", "zone_type", "stage", "bonus", "0"));
    m_pZoneTypeCombo->AddItem("Checkpoint",  new KeyValues("vals", "zone_type", "cp",    "bonus", "0"));
    m_pZoneTypeCombo->GetMenu()->SetTypeAheadMode(Menu::NO_TYPE_AHEAD_MODE); // Disable the annoying type ahead
    m_pZoneTypeCombo->SetNumberOfEditLines(7); // Make sure they're all visible at once
    m_pZoneTypeCombo->ActivateItemByRow(0);

    m_pGridSizeLabel        = new Label(this, "GridSizeLabel", "");
    m_pGridSizeSlider       = new CvarSlider(this, "GridSizeSlider");
    m_pGridSizeTextEntry    = new CvarTextEntry(this, "GridSizeTextEntry", "");
    m_bUpdateGridSizeSlider = false;

    LoadControlSettingsAndUserConfig("resource/ui/ZoneMenu.res");
}

int C_MomZoneMenu::HandleKeyInput(int down, ButtonCode_t keynum)
{
    if (keynum == MOUSE_RIGHT)
    {
        SetMouseInputEnabled(true);
        SetKeyBoardInputEnabled(true);
        return true;
    }
    else if (ShouldBindKeys() && down)
    {
        if (keynum == MOUSE_LEFT)
        {
            engine->ExecuteClientCmd("mom_zone_mark");
            return true;
        }
        else if (keynum == KEY_DELETE)
        {
            engine->ExecuteClientCmd("mom_zone_back");
            return true;
        }
        else if (keynum == KEY_ENTER)
        {
            engine->ExecuteClientCmd("mom_zone_create");
            return true;
        }
    }

    return false;
}

void C_MomZoneMenu::OnZoneInfoThunk(bf_read &msg) { g_pZoneMenu->OnZoneInfo(msg); }

void C_MomZoneMenu::OnZoneInfo(bf_read &msg)
{
    if (m_eZoneAction == ZONEACTION_NONE)
    {
        // Nothing to do :/
        return;
    }

    static ConVarRef mom_zone_edit("mom_zone_edit");
    int zoneidx = msg.ReadLong();
    int zonetype = msg.ReadLong();
    (void)zonetype;

    if (zoneidx == -1)
    {
        // No zone found
        Warning("No zones found\n");
        return;
    }

    switch (m_eZoneAction)
    {
    case ZONEACTION_DELETE:
    {
        bool old = mom_zone_edit.GetBool();
        mom_zone_edit.SetValue(true);

        char buf[64];
        Q_snprintf(buf, sizeof(buf), "mom_zone_delete %i", zoneidx);
        engine->ExecuteClientCmd(buf);

        mom_zone_edit.SetValue(old);

        break;
    }
    case ZONEACTION_EDIT:
    {
        mom_zone_edit.SetValue(true);

        char buf[64];
        Q_snprintf(buf, sizeof(buf), "mom_zone_edit_existing %i", zoneidx);
        engine->ExecuteClientCmd(buf);

        break;
    }
    default:
    {
        AssertMsg(false, "Unhandled zone action (%i)", m_eZoneAction);
        break;
    }
    }

    // Clear the command
    m_eZoneAction = ZONEACTION_NONE;
}

void C_MomZoneMenu::CancelZoning()
{
    static ConVarRef mom_zone_edit("mom_zone_edit");

    engine->ExecuteClientCmd("mom_zone_cancel");
    mom_zone_edit.SetValue(false);
    m_bBindKeys = false;
}

void C_MomZoneMenu::OnMousePressed(MouseCode code)
{
    if (code == MOUSE_RIGHT)
    {
        SetMouseInputEnabled(false);
        SetKeyBoardInputEnabled(false);
    }

    BaseClass::OnMousePressed(code);
}

void C_MomZoneMenu::OnClose()
{
    CancelZoning();

    BaseClass::OnClose();
}

void C_MomZoneMenu::OnControlModified(Panel* pPanel)
{
    if (pPanel == m_pGridSizeSlider)
    {
        // Don't retrigger the cvar change if it was already updated by textentry
        if (!m_bUpdateGridSizeSlider)
        {
            m_bUpdateGridSizeSlider = true;
            return;
        }

        // Round val to whole number, because no one wants to align to 6.1238765426
        float flVal = roundf(m_pGridSizeSlider->GetSliderValue());
        m_pGridSizeSlider->SetSliderValue(flVal);
        m_pGridSizeSlider->ApplyChanges();
        // update textentry control
        char szVal[32];
        Q_snprintf(szVal, sizeof(szVal), "%.0f", flVal);
        m_pGridSizeTextEntry->SetText(szVal);
    }
}

void C_MomZoneMenu::OnTextChanged(Panel *pPanel)
{
    if (pPanel == m_pGridSizeTextEntry)
    {
        m_bUpdateGridSizeSlider = false;
        m_pGridSizeTextEntry->ApplyChanges();
    }
    else if (pPanel == m_pZoneTypeCombo)
    {
        static ConVarRef mom_zone_type("mom_zone_type");
        static ConVarRef mom_zone_bonus("mom_zone_bonus");

        KeyValues *pData = m_pZoneTypeCombo->GetActiveItemUserData();
        mom_zone_type.SetValue(pData->GetString("zone_type"));
        mom_zone_bonus.SetValue(pData->GetInt("bonus"));
    }
}

void C_MomZoneMenu::OnCreateNewZone()
{
    ConVarRef mom_zone_edit("mom_zone_edit");
    ConVarRef mom_zone_usenewmoethod("mom_zone_usenewmethod");

    mom_zone_usenewmoethod.SetValue(true);
    mom_zone_edit.SetValue(true);

    // return control to game so they can start zoning immediately
    m_bBindKeys = true;
    SetMouseInputEnabled(false);
    SetKeyBoardInputEnabled(false);
}

void C_MomZoneMenu::OnDeleteZone()
{
    m_eZoneAction = ZONEACTION_DELETE;
    engine->ExecuteClientCmd("mom_zone_info");
}

void C_MomZoneMenu::OnEditZone()
{
    m_eZoneAction = ZONEACTION_EDIT;
    engine->ExecuteClientCmd("mom_zone_info");
}

void C_MomZoneMenu::OnCancelZone() { CancelZoning(); }

void C_MomZoneMenu::OnSaveZones() { engine->ExecuteClientCmd("mom_zone_generate"); }