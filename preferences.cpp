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
	        enum { IDD = IDD_PREFERENCES };

        private:
            // preferences_page_instance methods overrides
            t_uint32 get_state () override;
            void apply () override;
            void reset () override;

            // CDialogImpl
            BEGIN_MSG_MAP (dialog)
                MSG_WM_INITDIALOG (on_init_dialog)
                COMMAND_HANDLER_EX (IDC_CHECK_DEBUG_LOG, BN_CLICKED, on_state_changed)
                COMMAND_HANDLER_EX (IDC_RADIO_LOC_DEFAULT, BN_CLICKED, on_dll_mode)
                COMMAND_HANDLER_EX (IDC_RADIO_LOC_CUSTOM, BN_CLICKED, on_dll_mode)
                COMMAND_HANDLER_EX (IDC_BUTTON_BROWSE, BN_CLICKED, on_browse_for_dll)
                COMMAND_HANDLER_EX (IDC_EDIT_SPIN_CTRL_BUDDY, EN_CHANGE, on_state_changed)
            END_MSG_MAP ()

            // messages
	        BOOL on_init_dialog (CWindow, LPARAM)
            {
                // attach windows to members
                m_disc_cache_size.Attach (GetDlgItem (IDC_SPIN_CACHE_SIZE));
                m_location_default.Attach (GetDlgItem (IDC_RADIO_LOC_DEFAULT));
                m_location_custom.Attach (GetDlgItem (IDC_RADIO_LOC_CUSTOM));
                m_browse.Attach (GetDlgItem (IDC_BUTTON_BROWSE));
                m_dll_path.Attach (GetDlgItem (IDC_STATIC_DLL_LOCATION));
                m_debug_log.Attach (GetDlgItem (IDC_CHECK_DEBUG_LOG));

                // initialize controls
                set_dll_custom_mode (cfg::using_custom_dll, cfg::dll_path);
                m_debug_log.SetCheck (cfg::debug_log ? BST_CHECKED : BST_UNCHECKED);
                m_disc_cache_size.SetBuddy (GetDlgItem (IDC_EDIT_SPIN_CTRL_BUDDY));
                m_disc_cache_size.SetRange32 (0, unpack_7z::disk_cache::max_cache_size);
                m_disc_cache_size.SetPos32 (cfg::disk_cache_size);

                return FALSE;
            }

            void on_state_changed (UINT, int, CWindow)
            {
                m_callback->on_state_changed ();
            }

            void on_browse_for_dll (UINT, int, CWindow)
            {
                pfc::string8 tmp;
                if (uGetOpenFileName (*this, "Dlls|*.dll|All files|*.*", 0, "dll", "Locate 7z.dll", pfc::string_directory (cfg::dll_path), tmp, FALSE)) {
                    m_dll_path.SetWindowText (pfc::stringcvt::string_os_from_utf8 (tmp));
                    m_callback->on_state_changed ();
                }
            }

            void on_dll_mode (UINT, int, CWindow)
            {
                m_browse.EnableWindow (m_location_custom.IsChecked ());
                m_callback->on_state_changed ();
            }

            // helpers
            void set_dll_custom_mode (bool custom, const pfc::string8 &path)
            {
                (custom ? m_location_custom : m_location_default).SetCheck (BST_CHECKED);
                m_dll_path.SetWindowText (pfc::stringcvt::string_os_from_utf8 (path));
                m_browse.EnableWindow (custom);
            }

            // member variables
	        const preferences_page_callback::ptr m_callback;

            CCheckBox m_debug_log, m_location_default, m_location_custom;
            CButton m_browse;
            CStatic m_dll_path;
            CUpDownCtrl m_disc_cache_size;
        };

        t_uint32 dialog::get_state ()
        {
	        t_uint32 state = preferences_state::resettable;

            if (m_debug_log.IsChecked () != cfg::debug_log)
                state |= preferences_state::changed;

            if (m_location_custom.IsChecked () != cfg::using_custom_dll)
                state |= (preferences_state::changed | preferences_state::needs_restart);

            if (cfg::dll_path != pfc::string8 (string_utf8_from_window (m_dll_path))) {
                state |= preferences_state::changed;
                if (m_location_custom.IsChecked ())
                    state |= preferences_state::needs_restart;
            }

            if (m_disc_cache_size.GetPos32 () != cfg::disk_cache_size)
                state |= preferences_state::changed;

	        return state;
        }

        void dialog::reset ()
        {
            m_debug_log.SetCheck (cfg::defaults::debug_log ? BST_CHECKED : BST_UNCHECKED);
            set_dll_custom_mode (cfg::defaults::using_custom_dll, "");
            m_disc_cache_size.SetPos32 (cfg::defaults::disk_cache_size);

	        m_callback->on_state_changed ();
        }

        void dialog::apply ()
        {	
            cfg::debug_log = m_debug_log.IsChecked ();
            cfg::using_custom_dll = m_location_custom.IsChecked ();
            cfg::dll_path = string_utf8_from_window (m_dll_path);

            cfg::disk_cache_size = m_disc_cache_size.GetPos32 ();
            static_api_ptr_t<disk_cache::manager>()->restart ();

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