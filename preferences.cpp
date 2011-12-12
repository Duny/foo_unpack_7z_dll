#include "stdafx.h"
#include "resource.h"

#include "cache_system.h"

namespace unpack_7z
{
    namespace preferences
    {
        const t_uint32 archive_history_sizes[] = { 1, 100, 500, 1000, 10000, 50000, 100000, pfc_infinite };
        const t_uint32 num_sizes = sizeof (archive_history_sizes) / sizeof (archive_history_sizes[0]);


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

                COMMAND_ID_HANDLER_SIMPLE (IDC_BUTTON_BROWSE_FOR_CACHE_LOCATION, on_browse_for_cache_location)
                COMMAND_ID_HANDLER_SIMPLE (IDC_BUTTON_HISTORY_REMOVE_DEAD_ITEMS, on_remove_dead_items)
                COMMAND_ID_HANDLER_SIMPLE (IDC_BUTTON_CACHE_CLEAR, on_cache_clear)
                COMMAND_ID_HANDLER_SIMPLE (IDC_BUTTON_HISTORY_CLEAR, on_history_clear)
                COMMAND_ID_HANDLER_SIMPLE (IDC_BUTTON_PRINT_CACHE_STATS, on_print_cache_stats)

                COMMAND_HANDLER_SIMPLE (IDC_EDIT_SPIN_CTRL_BUDDY, EN_CHANGE, on_state_changed)
                COMMAND_HANDLER_SIMPLE (IDC_COMBO_ARCHIVE_HISTORY_SIZE, CBN_SELCHANGE, on_state_changed)
                COMMAND_ID_HANDLER_SIMPLE (IDC_RADIO_USE_DEFAULT_CACHE_LOCATION, on_state_changed)
                COMMAND_ID_HANDLER_SIMPLE (IDC_RADIO_USE_CUSTOM_CACHE_LOCATION, on_state_changed)
                COMMAND_ID_HANDLER_SIMPLE (IDC_CHECK_CACHE_EMPTY_AT_EXIT, on_state_changed)
            END_MSG_MAP ()


            // messages
	        BOOL on_init_dialog (CWindow, LPARAM)
            {
                // attach windows to members
                m_cache_size.Attach (GetDlgItem (IDC_SPIN_CACHE_SIZE));
                m_cache_size.SetBuddy (GetDlgItem (IDC_EDIT_SPIN_CTRL_BUDDY));
                m_cache_size.SetRange32 (0, INT_MAX);

                // initialize controls
                uButton_SetCheck (*this, IDC_CHECK_DEBUG_LOG, cfg::debug_log);

                set_cache_mode (cfg::use_sys_tmp_for_cache, cfg::custom_cache_path);
                m_cache_size.SetPos32 (cfg::file_cache_max);
                uButton_SetCheck (*this, IDC_CHECK_CACHE_EMPTY_AT_EXIT, cfg::cache_clear_at_exit);

                init_archive_history_sizes (cfg::archive_history_max);
                
                return FALSE;
            }

            inline void on_state_changed ()
            { 
                GetDlgItem (IDC_BUTTON_BROWSE_FOR_CACHE_LOCATION).EnableWindow (!is_default_cache_mode ());
                GetDlgItem (IDC_CHECK_CACHE_EMPTY_AT_EXIT).EnableWindow (!is_default_cache_mode ());
                GetDlgItem (IDC_BUTTON_PRINT_CACHE_STATS).EnableWindow (uButton_GetCheck (*this, IDC_CHECK_DEBUG_LOG));
                m_callback->on_state_changed ();
            }

            inline void on_browse_for_cache_location ()
            {
                if (auto result = uBrowseForFolderEx (*this, "Select cache location", cfg::custom_cache_path)) {
                    pfc::string8_fast tmp = result->GetFileName (0);
                    if (!tmp.ends_with ('\\')) tmp.add_char ('\\');
                    uSetDlgItemText (*this, IDC_STATIC_CUSTOM_CACHE_LOCATION, tmp);
                    m_callback->on_state_changed ();
                }
            }

            inline void on_remove_dead_items () { static_api_ptr_t<cache_system>()->remove_dead_history_items (); }

            inline void on_cache_clear ()
            {
                auto res = uMessageBox (*this, "Clear disk cache?", COMPONENT_NAME, MB_YESNO | MB_ICONQUESTION);
                if (res == IDYES) static_api_ptr_t<cache_system>()->cache_free ();
            }

            inline void on_history_clear ()
            {
                auto res = uMessageBox (*this, "Clear archive history?", COMPONENT_NAME, MB_YESNO | MB_ICONQUESTION);
                if (res == IDYES) static_api_ptr_t<cache_system>()->history_clear ();
            }

            inline void on_print_cache_stats () { static_api_ptr_t<cache_system>()->print_stats (); }

            // helpers

            inline bool is_default_cache_mode () const { return uButton_GetCheck (*this, IDC_RADIO_USE_DEFAULT_CACHE_LOCATION); }

            inline void set_cache_mode (bool mode_default, const pfc::string8 &custom_path)
            {
                uButton_SetCheck (*this, mode_default ? IDC_RADIO_USE_DEFAULT_CACHE_LOCATION : IDC_RADIO_USE_CUSTOM_CACHE_LOCATION, true);
                uButton_SetCheck (*this, !mode_default ? IDC_RADIO_USE_DEFAULT_CACHE_LOCATION : IDC_RADIO_USE_CUSTOM_CACHE_LOCATION, false);
                uSetDlgItemText (*this, IDC_STATIC_CUSTOM_CACHE_LOCATION, custom_path);
            }

            inline void init_archive_history_sizes (t_uint32 history_max)
            {
                uSendDlgItemMessage (IDC_COMBO_ARCHIVE_HISTORY_SIZE, CB_RESETCONTENT);
                int selected = CB_ERR;
                for (t_uint32 i = 0; i < num_sizes; i++) {
                    uSendDlgItemMessageText (*this, IDC_COMBO_ARCHIVE_HISTORY_SIZE, CB_ADDSTRING, 0, 
                        (archive_history_sizes[i] == pfc_infinite ? "Infinite" : pfc::toString (archive_history_sizes[i]).get_ptr ()));
                    if (archive_history_sizes[i] <= history_max)
                        selected = i;
                }
                uSendDlgItemMessage (IDC_COMBO_ARCHIVE_HISTORY_SIZE, CB_SETCURSEL, selected);
            }

            inline t_uint32 get_sel_archive_history_size ()
            {
                int i = uSendDlgItemMessage (IDC_COMBO_ARCHIVE_HISTORY_SIZE, CB_GETCURSEL);
                return i == CB_ERR ? 0 : archive_history_sizes[i];
            }

            // member variables
	        const preferences_page_callback::ptr m_callback;
            CUpDownCtrl m_cache_size;
        };

        t_uint32 page::get_state ()
        {
	        t_uint32 state = preferences_state::resettable;

            if (uButton_GetCheck (*this, IDC_CHECK_DEBUG_LOG) != cfg::debug_log)
                state |= preferences_state::changed;

            if (is_default_cache_mode () != cfg::use_sys_tmp_for_cache)
                state |= (preferences_state::changed | preferences_state::needs_restart);

            if (!(state & preferences_state::needs_restart)) {
                pfc::string8_fast path;
                uGetDlgItemText (*this, IDC_STATIC_CUSTOM_CACHE_LOCATION, path);
                if (cfg::custom_cache_path != path) {
                    state |= preferences_state::changed;
                    if (!is_default_cache_mode ())
                        state |= preferences_state::needs_restart;
                }
            }

            if (!(state & preferences_state::changed) && m_cache_size.GetPos32 () != cfg::file_cache_max)
                state |= preferences_state::changed;

            if (!(state & preferences_state::changed) && uButton_GetCheck (*this, IDC_CHECK_CACHE_EMPTY_AT_EXIT) != cfg::cache_clear_at_exit)
                state |= preferences_state::changed;

            if (!(state & preferences_state::changed) && get_sel_archive_history_size () != cfg::archive_history_max)
                state |= preferences_state::changed;

	        return state;
        }

        void page::apply ()
        {	
            cfg::debug_log = uButton_GetCheck (*this, IDC_CHECK_DEBUG_LOG);

            cfg::use_sys_tmp_for_cache = is_default_cache_mode ();
            uGetDlgItemText (*this, IDC_STATIC_CUSTOM_CACHE_LOCATION, cfg::custom_cache_path);
            static_api_ptr_t<cache_system>()->set_cache_size_max (m_cache_size.GetPos32 ());
            cfg::cache_clear_at_exit = uButton_GetCheck (*this, IDC_CHECK_CACHE_EMPTY_AT_EXIT);

            static_api_ptr_t<cache_system>()->set_history_size_max (get_sel_archive_history_size ());

            m_callback->on_state_changed ();
        }

        void page::reset ()
        {
            uButton_SetCheck (*this, IDC_CHECK_DEBUG_LOG, cfg::defaults::debug_log);

            set_cache_mode (cfg::defaults::use_sys_tmp_for_cache, "");
            m_cache_size.SetPos32 (cfg::defaults::file_cache_max);
            uButton_SetCheck (*this, IDC_CHECK_CACHE_EMPTY_AT_EXIT, cfg::defaults::cache_clear_at_exit);

            init_archive_history_sizes (cfg::defaults::archive_history_max);

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