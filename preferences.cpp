#include "stdafx.h"
#include "disk_cache.h"
#include "config.h"

namespace unpack_7z
{
    namespace preferences
    {
        class dialog : public CDialogImpl<dialog>, public preferences_page_instance
        {
        public:
	        dialog (preferences_page_callback::ptr callback) : m_callback (callback) {}

	        //dialog resource ID
	        enum { IDD = IDD_7ZPREFERENCES };
	
	        t_uint32 get_state () override;
	        void apply () override;
	        void reset () override;

	        BEGIN_MSG_MAP (dialog)
		        MSG_WM_INITDIALOG (OnInitDialog)
                COMMAND_HANDLER_EX (IDC_CHECK_QUIET_MODE, BN_CLICKED, OnButtonClicked)
		        COMMAND_HANDLER_EX (IDC_CHECK_DEBUG_MESSAGES, BN_CLICKED, OnButtonClicked)
                COMMAND_HANDLER_EX (IDC_RADIO_DEFAULT_LOCATION, BN_CLICKED, OnButtonClicked)
                COMMAND_HANDLER_EX (IDC_RADIO_CUSTOM_LOCATION, BN_CLICKED, OnButtonClicked)
		        COMMAND_HANDLER_EX (IDC_BUTTON_BROWSE, BN_CLICKED, OnButtonClicked)
                COMMAND_HANDLER_EX (IDC_EDIT_SPIN_CTRL_BUDDY, EN_CHANGE, OnSpinChange)
	        END_MSG_MAP ()

        private:
	        BOOL OnInitDialog (CWindow, LPARAM);
            void OnButtonClicked (UINT, int, CWindow);
            void OnSpinChange (UINT, int, CWindow)
            {
                int i = 0;
            }

	        void OnChanged ();

	        t_uint32 GetSelectedDllMode () const { 
		        //return IsDlgButtonChecked (IDC_RADIO_DEFAULT_LOCATION) == BST_CHECKED ? cfg::loc_default : cfg::loc_custom;
                true;
	        }

	        void EnableBrowseButton (bool enable) {
		        GetDlgItem (IDC_BUTTON_BROWSE).EnableWindow (enable ? FALSE : TRUE);
	        }

	        const preferences_page_callback::ptr m_callback;

            CUpDownCtrl m_spin_cache_size;
        };

        BOOL dialog::OnInitDialog (CWindow, LPARAM)
        {
            /*CheckDlgButton (cfg::default_location () ? IDC_RADIO_DEFAULT_LOCATION : IDC_RADIO_CUSTOM_LOCATION, BST_CHECKED);

            EnableBrowseButton (cfg::default_location ());

            SetDlgItemText (IDC_STATIC_DLL_LOCATION, pfc::stringcvt::string_os_from_utf8 (cfg::dll_custom_path));

            CheckDlgButton (IDC_CHECK_QUIET_MODE, cfg::quiet_mode ? BST_CHECKED : BST_UNCHECKED);
	        CheckDlgButton (IDC_CHECK_DEBUG_MESSAGES, cfg::debug_messages ? BST_CHECKED : BST_UNCHECKED);

            m_spin_cache_size.Attach (GetDlgItem (IDC_SPIN_CACHE_SIZE));
            m_spin_cache_size.SetBuddy (GetDlgItem (IDC_EDIT_SPIN_CTRL));
            m_spin_cache_size.SetRange32 (0, unpack_7z::disk_cache::max_cache_size);
            m_spin_cache_size.SetPos32 (cfg::disk_cache_size);*/

	        return FALSE; 
        }

        void dialog::OnButtonClicked (UINT, int id, CWindow)
        {
	        /*EnableBrowseButton (GetSelectedDllMode () == cfg::loc_default);

	        if (id == IDC_BUTTON_BROWSE) {
		        pfc::string8 file_name;
		        if (uGetOpenFileName (*this, "Dlls|*.dll|All files|*.*", 0, "dll", "Locate 7z.dll", 
			        pfc::string_directory (cfg::dll_custom_path), file_name, FALSE))
			        SetDlgItemText (IDC_STATIC_DLL_LOCATION, pfc::stringcvt::string_wide_from_utf8 (file_name));
	        }*/ 

            OnChanged ();
        }

        t_uint32 dialog::get_state ()
        {
	        t_uint32 state = preferences_state::resettable;

            /*if ((IsDlgButtonChecked (IDC_CHECK_QUIET_MODE) == BST_CHECKED) != cfg::quiet_mode)
                state |= preferences_state::changed;

	        if (GetSelectedDllMode () != cfg::dll_location_mode)
		        state |= (preferences_state::needs_restart | preferences_state::changed); 

	        if (uGetDlgItemText (*this, IDC_STATIC_DLL_LOCATION) != cfg::dll_custom_path) {
		        state |= preferences_state::changed;
		        if (GetSelectedDllMode () == cfg::loc_custom)
			        state |= preferences_state::needs_restart;
	        }

            if ((IsDlgButtonChecked (IDC_CHECK_DEBUG_MESSAGES) == BST_CHECKED) != cfg::debug_messages)
                state |= preferences_state::changed;*/

	        return state;
        }

        void dialog::reset ()
        {
	        /*CheckDlgButton (IDC_RADIO_DEFAULT_LOCATION, BST_UNCHECKED);
	        CheckDlgButton (IDC_RADIO_CUSTOM_LOCATION, BST_UNCHECKED);

	        int id = cfg::dll_location_mode_def == cfg::loc_default ? IDC_RADIO_DEFAULT_LOCATION : IDC_RADIO_CUSTOM_LOCATION;
            CheckDlgButton (id, BST_CHECKED);
	
	        EnableBrowseButton (cfg::dll_location_mode_def == cfg::loc_default);
	        SetDlgItemText (IDC_STATIC_DLL_LOCATION, pfc::stringcvt::string_os_from_utf8 (""));

	        CheckDlgButton (IDC_CHECK_QUIET_MODE, cfg::quiet_mode_def ? BST_CHECKED : BST_UNCHECKED);
	        CheckDlgButton (IDC_CHECK_DEBUG_MESSAGES, cfg::debug_messages_def ? BST_CHECKED : BST_UNCHECKED);*/

	        OnChanged ();
        }

        void dialog::apply ()
        {	
	        /*cfg::dll_location_mode = IsDlgButtonChecked (IDC_RADIO_DEFAULT_LOCATION) == BST_CHECKED ? cfg::loc_default : cfg::loc_custom;
	        cfg::dll_custom_path = string_utf8_from_window (*this, IDC_STATIC_DLL_LOCATION);

	        cfg::quiet_mode = IsDlgButtonChecked (IDC_CHECK_QUIET_MODE) == BST_CHECKED;
	        cfg::debug_messages = IsDlgButtonChecked (IDC_CHECK_DEBUG_MESSAGES) == BST_CHECKED;*/

	        OnChanged ();
        }

        void dialog::OnChanged ()
        {
	        m_callback->on_state_changed ();
        }

        class preferences_page_myimpl : public preferences_page_impl<dialog>
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
    }
}