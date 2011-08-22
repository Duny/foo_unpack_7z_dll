#include "stdafx.h"

static const GUID guid_cfg_quiet_mode = { 0x47419c83, 0xf297, 0x4052, { 0x9a, 0x92, 0x43, 0x55, 0x43, 0x1e, 0x8e, 0xf8 } };
static const GUID guid_cfg_debug_messages = { 0x4bbcc43d, 0xb10, 0x4bd4, { 0x98, 0xba, 0x58, 0x80, 0x6a, 0x9d, 0x61, 0x5e } };
static const GUID guid_cfg_dll_location_mode = { 0x52b27d56, 0xa744, 0x4d67, { 0xb4, 0x8f, 0x65, 0xac, 0x8a, 0xeb, 0x60, 0x65 } };
static const GUID guid_cfg_dll_custom_path = { 0x292200c6, 0xfe9a, 0x4174, { 0xaa, 0xf7, 0xb8, 0xe3, 0x9f, 0x41, 0x92, 0x9f } };

enum 
{
    default_cfg_quiet_mode = false,
	default_cfg_debug_messages = false,
    default_cfg_dll_location_mode = dll_location_default
};

cfg_bool   cfg_quiet_mode (guid_cfg_quiet_mode, default_cfg_quiet_mode);
cfg_bool   cfg_debug_messages (guid_cfg_debug_messages, default_cfg_debug_messages);
cfg_uint   cfg_dll_location_mode (guid_cfg_dll_location_mode, default_cfg_dll_location_mode);
cfg_string cfg_dll_custom_path (guid_cfg_dll_custom_path, "");


class C7zPreferences : public CDialogImpl<C7zPreferences>, public preferences_page_instance
{
public:
	C7zPreferences (preferences_page_callback::ptr callback) : m_callback (callback) {}

	//dialog resource ID
	enum { IDD = IDD_7ZPREFERENCES };
	
	t_uint32 get_state ();
	void apply ();
	void reset ();

	BEGIN_MSG_MAP (C7zPreferences)
		MSG_WM_INITDIALOG (OnInitDialog)
        COMMAND_HANDLER_EX (IDC_CHECK_QUIET_MODE, BN_CLICKED, OnButtonClicked)
		COMMAND_HANDLER_EX (IDC_CHECK_DEBUG_MESSAGES, BN_CLICKED, OnButtonClicked)
        COMMAND_HANDLER_EX (IDC_RADIO_DEFAULT_LOCATION, BN_CLICKED, OnButtonClicked)
        COMMAND_HANDLER_EX (IDC_RADIO_CUSTOM_LOCATION, BN_CLICKED, OnButtonClicked)
		COMMAND_HANDLER_EX (IDC_BUTTON_BROWSE, BN_CLICKED, OnButtonClicked)
	END_MSG_MAP ()

private:
	BOOL OnInitDialog (CWindow, LPARAM);
    void OnButtonClicked (UINT, int, CWindow);

	void OnChanged ();

	inline t_uint32 GetSelectedDllMode () const { 
		return IsDlgButtonChecked (IDC_RADIO_DEFAULT_LOCATION) == BST_CHECKED ? dll_location_default : dll_location_custom;
	}

	inline void EnableBrowseButton (bool enable) {
		GetDlgItem (IDC_BUTTON_BROWSE).EnableWindow (enable ? FALSE : TRUE);
	}

	const preferences_page_callback::ptr m_callback;
};

BOOL C7zPreferences::OnInitDialog (CWindow, LPARAM)
{
	bool is_default_dll_mode = cfg_dll_location_mode == dll_location_default;

	int id = is_default_dll_mode ? IDC_RADIO_DEFAULT_LOCATION : IDC_RADIO_CUSTOM_LOCATION;
    CheckDlgButton (id, BST_CHECKED);

    EnableBrowseButton (is_default_dll_mode);

    SetDlgItemText (IDC_STATIC_DLL_LOCATION, pfc::stringcvt::string_wide_from_utf8 (cfg_dll_custom_path));

    CheckDlgButton (IDC_CHECK_QUIET_MODE, cfg_quiet_mode ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton (IDC_CHECK_DEBUG_MESSAGES, cfg_debug_messages ? BST_CHECKED : BST_UNCHECKED);

	return FALSE; 
}

void C7zPreferences::OnButtonClicked (UINT, int id, CWindow)
{
	EnableBrowseButton (GetSelectedDllMode () == dll_location_default);

	if (id == IDC_BUTTON_BROWSE) {
		pfc::string8 file_name;
		if (uGetOpenFileName (*this, "Dlls|*.dll|All files|*.*", 0, "dll", "Locate 7z.dll", 
			pfc::string_directory (cfg_dll_custom_path), file_name, FALSE))
			SetDlgItemText (IDC_STATIC_DLL_LOCATION, pfc::stringcvt::string_wide_from_utf8 (file_name));
	} 

    OnChanged ();
}

t_uint32 C7zPreferences::get_state ()
{
	t_uint32 state = preferences_state::resettable;

    if ((IsDlgButtonChecked (IDC_CHECK_QUIET_MODE) == BST_CHECKED) != cfg_quiet_mode)
        state |= preferences_state::changed;

	if (GetSelectedDllMode () != cfg_dll_location_mode)
		state |= (preferences_state::needs_restart | preferences_state::changed); 

	if (uGetDlgItemText (*this, IDC_STATIC_DLL_LOCATION) != cfg_dll_custom_path) {
		state |= preferences_state::changed;
		if (GetSelectedDllMode () == dll_location_custom)
			state |= preferences_state::needs_restart;
	}

    if ((IsDlgButtonChecked (IDC_CHECK_DEBUG_MESSAGES) == BST_CHECKED) != cfg_debug_messages)
        state |= preferences_state::changed;

	return state;
}

void C7zPreferences::reset ()
{
	CheckDlgButton (IDC_RADIO_DEFAULT_LOCATION, BST_UNCHECKED);
	CheckDlgButton (IDC_RADIO_CUSTOM_LOCATION, BST_UNCHECKED);

	int id = default_cfg_dll_location_mode == dll_location_default ? IDC_RADIO_DEFAULT_LOCATION : IDC_RADIO_CUSTOM_LOCATION;
    CheckDlgButton (id, BST_CHECKED);
	
	EnableBrowseButton (default_cfg_dll_location_mode == dll_location_default);
	SetDlgItemText (IDC_STATIC_DLL_LOCATION, pfc::stringcvt::string_wide_from_utf8 (""));

	CheckDlgButton (IDC_CHECK_QUIET_MODE, default_cfg_quiet_mode ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton (IDC_CHECK_DEBUG_MESSAGES, default_cfg_debug_messages ? BST_CHECKED : BST_UNCHECKED);

	OnChanged ();
}

void C7zPreferences::apply ()
{	
	cfg_dll_location_mode = IsDlgButtonChecked (IDC_RADIO_DEFAULT_LOCATION) == BST_CHECKED ? dll_location_default : dll_location_custom;
	cfg_dll_custom_path = string_utf8_from_window (*this, IDC_STATIC_DLL_LOCATION);

	cfg_quiet_mode = IsDlgButtonChecked (IDC_CHECK_QUIET_MODE) == BST_CHECKED;
	cfg_debug_messages = IsDlgButtonChecked (IDC_CHECK_DEBUG_MESSAGES) == BST_CHECKED;

	OnChanged ();
}

void C7zPreferences::OnChanged ()
{
	m_callback->on_state_changed ();
}

class preferences_page_myimpl : public preferences_page_impl<C7zPreferences>
{
public:
	const char * get_name () { return "7z.dll Unpacker"; }
	GUID get_guid () {
		static const GUID guid = { 0xc25cb13f, 0x2a77, 0x451d, { 0xb5, 0xa5, 0x1c, 0x29, 0x5d, 0x67, 0x2c, 0xe6 } };
		return guid;
	}
	GUID get_parent_guid () { return guid_tools; }
};

static preferences_page_factory_t<preferences_page_myimpl> g_preferences_page_myimpl_factory;