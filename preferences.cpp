#include "stdafx.h"

namespace unpack_7z
{
    namespace preferences
    {
        class page : public CDialogImpl<page>, public preferences_page_instance
        {
        public:
	        page (preferences_page_callback::ptr callback) : m_callback (callback) {}

	        //dialog resource ID
	        enum { IDD = IDD_PREFERENCES };

        private: 
            //
            // preferences_page_instance methods overrides
            //
            t_uint32 get_state () override;
            void apply () override;
            void reset () override;


            // CDialogImpl
            BEGIN_MSG_MAP (page)
                MSG_WM_INITDIALOG (on_init_dialog)

                COMMAND_ID_HANDLER_SIMPLE (IDC_CHECK_DEBUG_LOG, on_state_changed)

                COMMAND_ID_HANDLER_SIMPLE (IDC_RADIO_DLL_LOCATION_DEFAULT, on_dll_and_cache_mode_change)
                COMMAND_ID_HANDLER_SIMPLE (IDC_RADIO_DLL_LOCATION_CUSTOM, on_dll_and_cache_mode_change)

                COMMAND_ID_HANDLER_SIMPLE (IDC_RADIO_CACHE_LOCATION_SYSTEM_TEMP, on_dll_and_cache_mode_change)
                COMMAND_ID_HANDLER_SIMPLE (IDC_RADIO_CACHE_LOCATION_CUSTOM, on_dll_and_cache_mode_change)

                COMMAND_ID_HANDLER_SIMPLE (IDC_BUTTON_BROWSE_FOR_DLL_LOCATION, on_browse_for_dll)
                COMMAND_ID_HANDLER_SIMPLE (IDC_BUTTON_BROWSE_FOR_CACHE_LOCATION, on_browse_for_cache_location)

                COMMAND_HANDLER_SIMPLE (IDC_EDIT_SPIN_CTRL_BUDDY, EN_CHANGE, on_state_changed)
            END_MSG_MAP ()


            // messages
	        BOOL on_init_dialog (CWindow, LPARAM)
            {
                // attach windows to members
                m_disc_cache_size.Attach (GetDlgItem (IDC_SPIN_CACHE_SIZE));
                m_disc_cache_size.SetBuddy (GetDlgItem (IDC_EDIT_SPIN_CTRL_BUDDY));
                m_disc_cache_size.SetRange32 (0, unpack_7z::disk_cache::max_cache_size);

                // initialize controls
                uButton_SetCheck (*this, IDC_CHECK_DEBUG_LOG, cfg::debug_log);

                set_dll_custom_mode (cfg::dll_path_custom, cfg::dll_path);

                set_cache_custom_mode (cfg::cache_location_custom, cfg::cache_location);
                
                m_disc_cache_size.SetPos32 (cfg::cache_size);

                return FALSE;
            }

            inline void on_state_changed () { m_callback->on_state_changed (); }

            inline void on_browse_for_dll ()
            {
                pfc::string8 tmp;
                if (uGetOpenFileName (*this, "Dlls|*.dll|All files|*.*", 0, "dll", "Locate 7z.dll", pfc::string_directory (cfg::dll_path), tmp, FALSE)) {
                    uSetDlgItemText (*this, IDC_STATIC_DLL_LOCATION, tmp);
                    m_callback->on_state_changed ();
                }
            }

            inline void on_browse_for_cache_location ()
            {
                if (auto result = uBrowseForFolderEx (*this, "Select cache location", cfg::cache_location)) {
                    pfc::string8_fast tmp = result->GetFileName (0);
                    if (!tmp.ends_with ('\\')) tmp.add_char ('\\');
                    uSetDlgItemText (*this, IDC_STATIC_CACHE_LOCATION, tmp);
                    m_callback->on_state_changed ();
                }
            }

            inline void on_dll_and_cache_mode_change ()
            {
                GetDlgItem (IDC_BUTTON_BROWSE_FOR_DLL_LOCATION).EnableWindow (dll_custom_mode ());
                GetDlgItem (IDC_BUTTON_BROWSE_FOR_CACHE_LOCATION).EnableWindow (cache_custom_mode ());
                m_callback->on_state_changed ();
            }

            // helpers
            inline bool dll_custom_mode () const { return uButton_GetCheck (*this, IDC_RADIO_DLL_LOCATION_CUSTOM); }

            inline void set_dll_custom_mode (bool custom, const pfc::string8 &path)
            {
                uButton_SetCheck (*this, custom ? IDC_RADIO_DLL_LOCATION_CUSTOM : IDC_RADIO_DLL_LOCATION_DEFAULT, true);
                uSetDlgItemText (*this, IDC_STATIC_DLL_LOCATION, path);
                GetDlgItem (IDC_BUTTON_BROWSE_FOR_DLL_LOCATION).EnableWindow (custom);
            }

            inline bool cache_custom_mode () const { return uButton_GetCheck (*this, IDC_RADIO_CACHE_LOCATION_CUSTOM); }

            inline void set_cache_custom_mode (bool custom, const pfc::string8 &location)
            {
                uButton_SetCheck (*this, custom ? IDC_RADIO_CACHE_LOCATION_CUSTOM : IDC_RADIO_CACHE_LOCATION_SYSTEM_TEMP, true);
                uSetDlgItemText (*this, IDC_STATIC_CACHE_LOCATION, location);
                GetDlgItem (IDC_BUTTON_BROWSE_FOR_CACHE_LOCATION).EnableWindow (custom);
            }

            // member variables
	        const preferences_page_callback::ptr m_callback;
            CUpDownCtrl m_disc_cache_size;
        };

        t_uint32 page::get_state ()
        {
	        t_uint32 state = preferences_state::resettable;

            if (uButton_GetCheck (*this, IDC_CHECK_DEBUG_LOG) != cfg::debug_log)
                state |= preferences_state::changed;

            if (dll_custom_mode () != cfg::dll_path_custom)
                state |= (preferences_state::changed | preferences_state::needs_restart);

            pfc::string8_fast path;
            uGetDlgItemText (*this, IDC_STATIC_DLL_LOCATION, path);
            if (cfg::dll_path != path) {
                state |= preferences_state::changed;
                if (dll_custom_mode ())
                    state |= preferences_state::needs_restart;
            }

            if (cache_custom_mode () != cfg::cache_location_custom)
                state |= (preferences_state::changed | preferences_state::needs_restart);

            uGetDlgItemText (*this, IDC_STATIC_CACHE_LOCATION, path);
            if (cfg::cache_location != path) {
                state |= preferences_state::changed;
                if (cache_custom_mode ())
                    state |= preferences_state::needs_restart;
            }

            if (m_disc_cache_size.GetPos32 () != cfg::cache_size)
                state |= (preferences_state::changed | preferences_state::needs_restart);

	        return state;
        }

        void page::apply ()
        {	
            cfg::debug_log = uButton_GetCheck (*this, IDC_CHECK_DEBUG_LOG);

            cfg::dll_path_custom = dll_custom_mode ();
            uGetDlgItemText (*this, IDC_STATIC_DLL_LOCATION, cfg::dll_path);

            cfg::cache_location_custom = cache_custom_mode ();
            uGetDlgItemText (*this, IDC_STATIC_CACHE_LOCATION, cfg::cache_location);

            cfg::cache_size = m_disc_cache_size.GetPos32 ();

            m_callback->on_state_changed ();
        }

        void page::reset ()
        {
            uButton_SetCheck (*this, IDC_CHECK_DEBUG_LOG, cfg::defaults::debug_log);

            set_dll_custom_mode (cfg::defaults::dll_path_custom, "");

            set_cache_custom_mode (cfg::defaults::cache_location_custom, "");
            m_disc_cache_size.SetPos32 (cfg::defaults::cache_size);

	        m_callback->on_state_changed ();
        }

        class preferences_page_unpack_7z : public preferences_page_impl<page>
        {
	        const char * get_name () { return "7z.dll Unpacker"; }
	        GUID get_guid () { return guid_inline<0xc25cb13f, 0x2a77, 0x451d, 0xb5, 0xa5, 0x1c, 0x29, 0x5d, 0x67, 0x2c, 0xe6>::guid; }
	        GUID get_parent_guid () { return guid_tools; }
        };
        static preferences_page_factory_t<preferences_page_unpack_7z> g_factory;
    }
}