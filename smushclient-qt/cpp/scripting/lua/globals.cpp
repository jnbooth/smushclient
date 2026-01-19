#include "globals.h"
#include "../miniwindow.h"
#include "../qlua.h"
#include "../scriptapi.h"
#include "../scriptenums.h"
#include "sqlite3.h"
#include <string>
#include <utility>
extern "C"
{
#include "lua.h"
}

using std::pair;
using std::string;

static const pair<string, AliasFlag> alias_flag[] = {
  { "Enabled", AliasFlag::Enabled },
  { "KeepEvaluating", AliasFlag::KeepEvaluating },
  { "IgnoreAliasCase", AliasFlag::IgnoreAliasCase },
  { "OmitFromLogFile", AliasFlag::OmitFromLogFile },
  { "RegularExpression", AliasFlag::RegularExpression },
  { "ExpandVariables", AliasFlag::ExpandVariables },
  { "Replace", AliasFlag::Replace },
  { "AliasSpeedWalk", AliasFlag::AliasSpeedWalk },
  { "AliasQueue", AliasFlag::AliasQueue },
  { "AliasMenu", AliasFlag::AliasMenu },
  { "Temporary", AliasFlag::Temporary },
  { "OneShot", AliasFlag::OneShot }
};

static const pair<string, int> custom_colour[] = {
  { "NoChange", -1 },       { "Custom1", 0x8080FF },  { "Custom2", 0x80FFFF },
  { "Custom3", 0x80FF80 },  { "Custom4", 0xFFFF80 },  { "Custom5", 0xFF8000 },
  { "Custom6", 0xC080FF },  { "Custom7", 0x0000FF },  { "Custom8", 0xC08000 },
  { "Custom9", 0xFF00FF },  { "Custom10", 0x404080 }, { "Custom11", 0x4080FF },
  { "Custom12", 0x808000 }, { "Custom13", 0x804000 }, { "Custom14", 0x8000FF },
  { "Custom15", 0x008000 }, { "Custom16", 0xFF0000 }, { "CustomOther", -1 }
};

static const pair<string, ApiCode> error_code[] = {
  { "eOK", ApiCode::OK },
  { "eWorldOpen", ApiCode::WorldOpen },
  { "eWorldClosed", ApiCode::WorldClosed },
  { "eNoNameSpecified", ApiCode::NoNameSpecified },
  { "eCannotPlaySound", ApiCode::CannotPlaySound },
  { "eTriggerNotFound", ApiCode::TriggerNotFound },
  { "eTriggerAlreadyExists", ApiCode::TriggerAlreadyExists },
  { "eTriggerCannotBeEmpty", ApiCode::TriggerCannotBeEmpty },
  { "eInvalidObjectLabel", ApiCode::InvalidObjectLabel },
  { "eScriptNameNotLocated", ApiCode::ScriptNameNotLocated },
  { "eAliasNotFound", ApiCode::AliasNotFound },
  { "eAliasAlreadyExists", ApiCode::AliasAlreadyExists },
  { "eAliasCannotBeEmpty", ApiCode::AliasCannotBeEmpty },
  { "eCouldNotOpenFile", ApiCode::CouldNotOpenFile },
  { "eLogFileNotOpen", ApiCode::LogFileNotOpen },
  { "eLogFileAlreadyOpen", ApiCode::LogFileAlreadyOpen },
  { "eLogFileBadWrite", ApiCode::LogFileBadWrite },
  { "eTimerNotFound", ApiCode::TimerNotFound },
  { "eTimerAlreadyExists", ApiCode::TimerAlreadyExists },
  { "eVariableNotFound", ApiCode::VariableNotFound },
  { "eCommandNotEmpty", ApiCode::CommandNotEmpty },
  { "eBadRegularExpression", ApiCode::BadRegularExpression },
  { "eTimeInvalid", ApiCode::TimeInvalid },
  { "eBadMapItem", ApiCode::BadMapItem },
  { "eNoMapItems", ApiCode::NoMapItems },
  { "eUnknownOption", ApiCode::UnknownOption },
  { "eOptionOutOfRange", ApiCode::OptionOutOfRange },
  { "eTriggerSequenceOutOfRange", ApiCode::TriggerSequenceOutOfRange },
  { "eTriggerSendToInvalid", ApiCode::TriggerSendToInvalid },
  { "eTriggerLabelNotSpecified", ApiCode::TriggerLabelNotSpecified },
  { "ePluginFileNotFound", ApiCode::PluginFileNotFound },
  { "eProblemsLoadingPlugin", ApiCode::ProblemsLoadingPlugin },
  { "ePluginCannotSetOption", ApiCode::PluginCannotSetOption },
  { "ePluginCannotGetOption", ApiCode::PluginCannotGetOption },
  { "eNoSuchPlugin", ApiCode::NoSuchPlugin },
  { "eNotAPlugin", ApiCode::NotAPlugin },
  { "eNoSuchRoutine", ApiCode::NoSuchRoutine },
  { "ePluginDoesNotSaveState", ApiCode::PluginDoesNotSaveState },
  { "ePluginCouldNotSaveState", ApiCode::PluginCouldNotSaveState },
  { "ePluginDisabled", ApiCode::PluginDisabled },
  { "eErrorCallingPluginRoutine", ApiCode::ErrorCallingPluginRoutine },
  { "eCommandsNestedTooDeeply", ApiCode::CommandsNestedTooDeeply },
  { "eCannotCreateChatSocket", ApiCode::CannotCreateChatSocket },
  { "eCannotLookupDomainName", ApiCode::CannotLookupDomainName },
  { "eNoChatConnections", ApiCode::NoChatConnections },
  { "eChatPersonNotFound", ApiCode::ChatPersonNotFound },
  { "eBadParameter", ApiCode::BadParameter },
  { "eChatAlreadyListening", ApiCode::ChatAlreadyListening },
  { "eChatIDNotFound", ApiCode::ChatIDNotFound },
  { "eChatAlreadyConnected", ApiCode::ChatAlreadyConnected },
  { "eClipboardEmpty", ApiCode::ClipboardEmpty },
  { "eFileNotFound", ApiCode::FileNotFound },
  { "eAlreadyTransferringFile", ApiCode::AlreadyTransferringFile },
  { "eNotTransferringFile", ApiCode::NotTransferringFile },
  { "eNoSuchCommand", ApiCode::NoSuchCommand },
  { "eArrayAlreadyExists", ApiCode::ArrayAlreadyExists },
  { "eArrayDoesNotExist", ApiCode::ArrayDoesNotExist },
  { "eArrayNotEvenNumberOfValues", ApiCode::ArrayNotEvenNumberOfValues },
  { "eImportedWithDuplicates", ApiCode::ImportedWithDuplicates },
  { "eBadDelimiter", ApiCode::BadDelimiter },
  { "eSetReplacingExistingValue", ApiCode::SetReplacingExistingValue },
  { "eKeyDoesNotExist", ApiCode::KeyDoesNotExist },
  { "eCannotImport", ApiCode::CannotImport },
  { "eItemInUse", ApiCode::ItemInUse },
  { "eSpellCheckNotActive", ApiCode::SpellCheckNotActive },
  { "eCannotAddFont", ApiCode::CannotAddFont },
  { "ePenStyleNotValid", ApiCode::PenStyleNotValid },
  { "eUnableToLoadImage", ApiCode::UnableToLoadImage },
  { "eImageNotInstalled", ApiCode::ImageNotInstalled },
  { "eInvalidNumberOfPoints", ApiCode::InvalidNumberOfPoints },
  { "eInvalidPoint", ApiCode::InvalidPoint },
  { "eHotspotPluginChanged", ApiCode::HotspotPluginChanged },
  { "eHotspotNotInstalled", ApiCode::HotspotNotInstalled },
  { "eNoSuchWindow", ApiCode::NoSuchWindow },
  { "eBrushStyleNotValid", ApiCode::BrushStyleNotValid }
};

static const pair<ApiCode, string> error_desc[] = {
  { ApiCode::OK, "No error" },
  { ApiCode::WorldOpen, "The world is already open" },
  { ApiCode::WorldClosed,
    "The world is closed, this action cannot be performed" },
  { ApiCode::NoNameSpecified,
    "No name has been specified where one is required" },
  { ApiCode::CannotPlaySound, "The sound file could not be played" },
  { ApiCode::TriggerNotFound, "The specified trigger name does not exist" },
  { ApiCode::TriggerAlreadyExists,
    "Attempt to add a trigger that already exists" },
  { ApiCode::TriggerCannotBeEmpty,
    "The trigger \"match\" string cannot be empty" },
  { ApiCode::InvalidObjectLabel, "The name of this object is invalid" },
  { ApiCode::ScriptNameNotLocated, "Script name is not in the script file" },
  { ApiCode::AliasNotFound, "The specified alias name does not exist" },
  { ApiCode::AliasAlreadyExists, "Attempt to add a alias that already exists" },
  { ApiCode::AliasCannotBeEmpty, "The alias \"match\" string cannot be empty" },
  { ApiCode::CouldNotOpenFile, "Unable to open requested file" },
  { ApiCode::LogFileNotOpen, "Log file was not open" },
  { ApiCode::LogFileAlreadyOpen, "Log file was already open" },
  { ApiCode::LogFileBadWrite, "Bad write to log file" },
  { ApiCode::TimerNotFound, "The specified timer name does not exist" },
  { ApiCode::TimerAlreadyExists, "Attempt to add a timer that already exists" },
  { ApiCode::VariableNotFound,
    "Attempt to delete a variable that does not exist" },
  { ApiCode::CommandNotEmpty,
    "Attempt to use SetCommand with a non-empty command window" },
  { ApiCode::BadRegularExpression, "Bad regular expression syntax" },
  { ApiCode::TimeInvalid, "Time given to AddTimer is invalid" },
  { ApiCode::BadMapItem, "Direction given to AddToMapper is invalid" },
  { ApiCode::NoMapItems, "No items in mapper" },
  { ApiCode::UnknownOption, "Option name not found" },
  { ApiCode::OptionOutOfRange, "New value for option is out of range" },
  { ApiCode::TriggerSequenceOutOfRange, "Trigger sequence value invalid" },
  { ApiCode::TriggerSendToInvalid, "Where to send trigger text to is invalid" },
  { ApiCode::TriggerLabelNotSpecified,
    "Trigger label not specified/invalid for 'send to variable'" },
  { ApiCode::PluginFileNotFound, "File name specified for plugin not found" },
  { ApiCode::ProblemsLoadingPlugin,
    "There was a parsing or other problem loading the plugin" },
  { ApiCode::PluginCannotSetOption,
    "Plugin is not allowed to set this option" },
  { ApiCode::PluginCannotGetOption,
    "Plugin is not allowed to get this option" },
  { ApiCode::NoSuchPlugin, "Requested plugin is not installed" },
  { ApiCode::NotAPlugin, "Only a plugin can do this" },
  { ApiCode::NoSuchRoutine,
    "Plugin does not support that subroutine (subroutine not in script)" },
  { ApiCode::PluginDoesNotSaveState, "Plugin does not support saving state" },
  { ApiCode::PluginCouldNotSaveState,
    "Plugin could not save state (eg. no state directory)" },
  { ApiCode::PluginDisabled, "Plugin is currently disabled" },
  { ApiCode::ErrorCallingPluginRoutine, "Could not call plugin routine" },
  { ApiCode::CommandsNestedTooDeeply,
    "Calls to \"Execute\" nested too deeply" },
  { ApiCode::CannotCreateChatSocket,
    "Unable to create socket for chat connection" },
  { ApiCode::CannotLookupDomainName,
    "Unable to do DNS (domain name) lookup for chat connection" },
  { ApiCode::NoChatConnections, "No chat connections open" },
  { ApiCode::ChatPersonNotFound, "Requested chat person not connected" },
  { ApiCode::BadParameter,
    "General problem with a parameter to a script call" },
  { ApiCode::ChatAlreadyListening, "Already listening for incoming chats" },
  { ApiCode::ChatIDNotFound, "Chat session with that ID not found" },
  { ApiCode::ChatAlreadyConnected, "Already connected to that server/port" },
  { ApiCode::ClipboardEmpty, "Cannot get (text from the) clipboard" },
  { ApiCode::FileNotFound, "Cannot open the specified file" },
  { ApiCode::AlreadyTransferringFile, "Already transferring a file" },
  { ApiCode::NotTransferringFile, "Not transferring a file" },
  { ApiCode::NoSuchCommand, "There is not a command of that name" },
  { ApiCode::ArrayAlreadyExists, "That array already exists" },
  { ApiCode::BadKeyName, "That name is not permitted for a key" },
  { ApiCode::ArrayDoesNotExist, "That array does not exist" },
  { ApiCode::ArrayNotEvenNumberOfValues,
    "Values to be imported into array are not in pairs" },
  { ApiCode::ImportedWithDuplicates,
    "Import succeeded, however some values were overwritten" },
  { ApiCode::BadDelimiter,
    "Import/export delimiter must be a single "
    "character, other than backslash" },
  { ApiCode::SetReplacingExistingValue,
    "Array element set, existing value overwritten" },
  { ApiCode::KeyDoesNotExist, "Array key does not exist" },
  { ApiCode::CannotImport,
    "Cannot import because cannot find unused temporary character" },
  { ApiCode::ItemInUse,
    "Cannot delete trigger/alias/timer because it is executing a script" },
  { ApiCode::SpellCheckNotActive, "Spell checker is not active" },
  { ApiCode::CannotAddFont, "Cannot create requested font" },
  { ApiCode::PenStyleNotValid, "Invalid settings for pen parameter" },
  { ApiCode::UnableToLoadImage, "Bitmap image could not be loaded" },
  { ApiCode::ImageNotInstalled, "Image has not been loaded into window" },
  { ApiCode::InvalidNumberOfPoints, "Number of points supplied is incorrect" },
  { ApiCode::InvalidPoint, "Point is not numeric" },
  { ApiCode::HotspotPluginChanged,
    "Hotspot processing must all be in same plugin" },
  { ApiCode::HotspotNotInstalled,
    "Hotspot has not been defined for this window" },
  { ApiCode::NoSuchWindow, "Requested miniwindow does not exist" },
  { ApiCode::BrushStyleNotValid, "Invalid settings for brush parameter" }
};

static const pair<string, int> miniwin[] = {
  { "pos_stretch_to_view", (int)MiniWindow::Position::OutputStretch },
  { "pos_stretch_to_view_with_aspect", (int)MiniWindow::Position::OutputScale },
  { "pos_stretch_to_owner", (int)MiniWindow::Position::OwnerStretch },
  { "pos_stretch_to_owner_with_aspect", (int)MiniWindow::Position::OwnerScale },
  { "pos_top_left", (int)MiniWindow::Position::TopLeft },
  { "pos_top_center", (int)MiniWindow::Position::TopCenter },
  { "pos_top_right", (int)MiniWindow::Position::TopRight },
  { "pos_center_right", (int)MiniWindow::Position::CenterRight },
  { "pos_bottom_right", (int)MiniWindow::Position::BottomRight },
  { "pos_bottom_center", (int)MiniWindow::Position::BottomCenter },
  { "pos_bottom_left", (int)MiniWindow::Position::BottomLeft },
  { "pos_center_left", (int)MiniWindow::Position::CenterLeft },
  { "pos_center_all", (int)MiniWindow::Position::Center },
  { "pos_tile", (int)MiniWindow::Position::Tile },

  { "create_underneath", MiniWindow::Flag::DrawUnderneath },
  { "create_absolute_location", MiniWindow::Flag::Absolute },
  { "create_transparent", MiniWindow::Flag::Transparent },
  { "create_ignore_mouse", MiniWindow::Flag::IgnoreMouse },
  { "create_keep_hotspots", MiniWindow::Flag::KeepHotspots },

  { "pen_solid", PenStyleFlag::SolidLine },
  { "pen_dash", PenStyleFlag::DashLine },
  { "pen_dot", PenStyleFlag::DotLine },
  { "pen_dash_dot", PenStyleFlag::DashDotLine },
  { "pen_dash_dot_dot", PenStyleFlag::DashDotDotLine },
  { "pen_null", PenStyleFlag::NoPen },
  { "pen_inside_frame", PenStyleFlag::InsideFrame },

  { "pen_endcap_round", PenCapFlag::RoundCap },
  { "pen_endcap_square", PenCapFlag::SquareCap },
  { "pen_endcap_flat", PenCapFlag::FlatCap },

  { "pen_join_round", PenJoinFlag::RoundJoin },
  { "pen_join_bevel", PenJoinFlag::BevelJoin },
  { "pen_join_miter", PenJoinFlag::MiterJoin },

  { "brush_solid", (int)ScriptBrush::SolidPattern },
  { "brush_null", (int)ScriptBrush::NoBrush },
  { "brush_hatch_horizontal", (int)ScriptBrush::HorPattern },
  { "brush_hatch_vertical", (int)ScriptBrush::VerPattern },
  { "brush_hatch_forwards_diagonal", (int)ScriptBrush::FDiagPattern },
  { "brush_hatch_backwards_diagonal", (int)ScriptBrush::BDiagPattern },
  { "brush_hatch_cross", (int)ScriptBrush::CrossPattern },
  { "brush_hatch_cross_diagonal", (int)ScriptBrush::DiagCrossPattern },
  { "brush_fine_pattern", (int)ScriptBrush::Dense4Pattern },
  { "brush_medium_pattern", (int)ScriptBrush::Dense2Pattern },
  { "brush_coarse_pattern", (int)ScriptBrush::Dense1Pattern },
  { "brush_waves_horizontal", (int)ScriptBrush::HorWaves },
  { "brush_waves_vertical", (int)ScriptBrush::VerWaves },

  { "rect_frame", (int)RectOp::Frame },
  { "rect_fill", (int)RectOp::Fill },
  { "rect_invert", (int)RectOp::Invert },
  { "rect_3d_rect", (int)RectOp::Frame3D },
  { "rect_draw_edge", (int)RectOp::Edge3D },
  { "rect_flood_fill_border", (int)RectOp::FloodFillBorder },
  { "rect_flood_fill_surface", (int)RectOp::FloodFillSurface },
  { "rect_edge_raised", 5 },
  { "rect_edge_etched", 6 },
  { "rect_edge_bump", 9 },
  { "rect_edge_sunken", 10 },
  { "rect_edge_at_top_left", 3 },
  { "rect_edge_at_top_right", 6 },
  { "rect_edge_at_bottom_left", 9 },
  { "rect_edge_at_bottom_right", 12 },
  { "rect_edge_at_all", 15 },
  { "rect_diagonal_end_top_left", 19 },
  { "rect_diagonal_end_top_right", 22 },
  { "rect_diagonal_end_bottom_left", 25 },
  { "rect_diagonal_end_bottom_right", 28 },
  { "rect_option_fill_middle", 0x0800 },
  { "rect_option_softer_buttons", 0x1000 },
  { "rect_option_flat_borders", 0x4000 },
  { "rect_option_monochrom_borders", 0x8000 },

  { "circle_ellipse", (int)CircleOp::Ellipse },
  { "circle_rectangle", (int)CircleOp::Rectangle },
  { "circle_round_rectangle", (int)CircleOp::RoundedRectangle },
  { "circle_chord", (int)CircleOp::Chord },
  { "circle_pie", (int)CircleOp::Pie },

  { "gradient_horizontal", Qt::Orientation::Horizontal },
  { "gradient_vertical", Qt::Orientation::Vertical },
  { "gradient_texture", 3 },

  { "font_charset_ansi", 0 },
  { "font_charset_default", 1 },
  { "font_charset_symbol", 2 },

  { "font_family_any", FontFamilyFlag::AnyFamily },
  { "font_family_roman", FontFamilyFlag::Roman },
  { "font_family_swiss", FontFamilyFlag::Swiss },
  { "font_family_modern", FontFamilyFlag::Modern },
  { "font_family_script", FontFamilyFlag::Script },
  { "font_family_decorative", FontFamilyFlag::Decorative },

  { "font_pitch_default", FontPitchFlag::Default },
  { "font_pitch_fixed", FontPitchFlag::Fixed },
  { "font_pitch_variable", FontPitchFlag::Variable },
  { "font_pitch_monospaced", FontPitchFlag::Monospace },

  { "font_truetype", 4 },

  { "image_copy", (int)MiniWindow::DrawImageMode::Copy },
  { "image_stretch", (int)MiniWindow::DrawImageMode::Stretch },
  { "image_transparent_copy", (int)MiniWindow::DrawImageMode::CopyTransparent },

  { "image_fill_ellipse", (int)ImageOp::Ellipse },
  { "image_fill_rectangle", (int)ImageOp::Rectangle },
  { "image_fill_round_fill_rectangle", (int)ImageOp::RoundedRectangle },

  { "filter_noise", 1 },
  { "filter_monochrome_noise", 2 },
  { "filter_blur", 3 },
  { "filter_sharpen", 4 },
  { "filter_find_edges", 5 },
  { "filter_emboss", 6 },
  { "filter_brightness", 7 },
  { "filter_contrast", 8 },
  { "filter_gamma", 9 },
  { "filter_red_brightness", 10 },
  { "filter_red_contrast", 11 },
  { "filter_red_gamma", 12 },
  { "filter_green_brightness", 13 },
  { "filter_green_contrast", 14 },
  { "filter_green_gamma", 15 },
  { "filter_blue_brightness", 16 },
  { "filter_blue_contrast", 17 },
  { "filter_blue_gamma", 18 },
  { "filter_grayscale", 19 },
  { "filter_normal_grayscale", 20 },
  { "filter_brightness_multiply", 21 },
  { "filter_red_brightness_multiply", 22 },
  { "filter_green_brightness_multiply", 23 },
  { "filter_blue_brightness_multiply", 24 },
  { "filter_lesser_blur", 25 },
  { "filter_minor_blur", 26 },
  { "filter_average", 27 },

  { "blend_normal", 1 },
  { "blend_average", 2 },
  { "blend_interpolate", 3 },
  { "blend_dissolve", 4 },
  { "blend_darken", 5 },
  { "blend_multiply", 6 },
  { "blend_colour_burn", 7 },
  { "blend_linear_burn", 8 },
  { "blend_inverse_colour_burn", 9 },
  { "blend_subtract", 10 },
  { "blend_lighten", 11 },
  { "blend_screen", 12 },
  { "blend_colour_dodge", 13 },
  { "blend_linear_dodge", 14 },
  { "blend_inverse_colour_dodge", 15 },
  { "blend_add", 16 },
  { "blend_overlay", 17 },
  { "blend_soft_light", 18 },
  { "blend_hard_light", 19 },
  { "blend_vivid_light", 20 },
  { "blend_linear_light", 21 },
  { "blend_pin_light", 22 },
  { "blend_hard_mix", 23 },
  { "blend_difference", 24 },
  { "blend_exclusion", 25 },
  { "blend_reflect", 26 },
  { "blend_glow", 27 },
  { "blend_freeze", 28 },
  { "blend_heat", 29 },
  { "blend_negation", 30 },
  { "blend_phoenix", 31 },
  { "blend_stamp", 32 },
  { "blend_xor", 33 },
  { "blend_and", 34 },
  { "blend_or", 35 },
  { "blend_red", 36 },
  { "blend_green", 37 },
  { "blend_blue", 38 },
  { "blend_yellow", 39 },
  { "blend_cyan", 40 },
  { "blend_magenta", 41 },
  { "blend_green_limited_by_red", 42 },
  { "blend_green_limited_by_blue", 43 },
  { "blend_green_limited_by_average_of_red_and_blue", 44 },
  { "blend_blue_limited_by_red", 45 },
  { "blend_blue_limited_by_green", 46 },
  { "blend_blue_limited_by_average_of_red_and_green", 47 },
  { "blend_red_limited_by_green", 48 },
  { "blend_red_limited_by_blue", 49 },
  { "blend_red_limited_by_average_of_green_and_blue", 50 },
  { "blend_red_only", 51 },
  { "blend_green_only", 52 },
  { "blend_blue_only", 53 },
  { "blend_discard_red", 54 },
  { "blend_discard_green", 55 },
  { "blend_discard_blue", 56 },
  { "blend_all_red", 57 },
  { "blend_all_green", 58 },
  { "blend_all_blue", 59 },
  { "blend_hue_mode", 60 },
  { "blend_saturation_mode", 61 },
  { "blend_colour_mode", 62 },
  { "blend_luminance_mode", 63 },
  { "blend_hsl", 64 },

  { "cursor_none", (int)ScriptCursor::BlankCursor },
  { "cursor_arrow", (int)ScriptCursor::ArrowCursor },
  { "cursor_hand", (int)ScriptCursor::OpenHandCursor },
  { "cursor_ibeam", (int)ScriptCursor::IBeamCursor },
  { "cursor_plus", (int)ScriptCursor::CrossCursor },
  { "cursor_wait", (int)ScriptCursor::WaitCursor },
  { "cursor_up", (int)ScriptCursor::UpArrowCursor },
  { "cursor_nw_se_arrow", (int)ScriptCursor::SizeFDiagCursor },
  { "cursor_ne_sw_arrow", (int)ScriptCursor::SizeBDiagCursor },
  { "cursor_ew_arrow", (int)ScriptCursor::SizeHorCursor },
  { "cursor_ns_arrow", (int)ScriptCursor::SizeVerCursor },
  { "cursor_both_arrow", (int)ScriptCursor::SizeAllCursor },
  { "cursor_x", (int)ScriptCursor::ForbiddenCursor },
  { "cursor_help", (int)ScriptCursor::WhatsThisCursor },

  { "hotspot_report_all_mouseovers", Hotspot::Flag::ReportAllMouseovers },

  { "hotspot_got_shift", Hotspot::EventFlag::Shift },
  { "hotspot_got_control", Hotspot::EventFlag::Control },
  { "hotspot_got_alt", Hotspot::EventFlag::Alt },
  { "hotspot_got_lh_mouse", Hotspot::EventFlag::MouseLeft },
  { "hotspot_got_rh_mouse", Hotspot::EventFlag::MouseRight },
  { "hotspot_got_dbl_click", Hotspot::EventFlag::DoubleClick },
  { "hotspot_got_not_first", Hotspot::EventFlag::Hover },
  { "hotspot_got_middle_mouse", Hotspot::EventFlag::MouseMiddle },

  { "merge_straight", 0 },
  { "merge_transparent", 1 },

  { "drag_got_shift", Hotspot::EventFlag::Shift },
  { "drag_got_control", Hotspot::EventFlag::Control },
  { "drag_got_alt", Hotspot::EventFlag::Alt },

  { "wheel_got_shift", Hotspot::EventFlag::Shift },
  { "wheel_got_control", Hotspot::EventFlag::Control },
  { "wheel_got_alt", Hotspot::EventFlag::Alt },
  { "wheel_scroll_back", Hotspot::EventFlag::ScrollDown }
};

static const pair<string, OperatingSystem> operating_system[] = {
  { "Windows", OperatingSystem::Windows },
  { "MacOS", OperatingSystem::MacOS },
  { "Linux", OperatingSystem::Linux }
};

static const pair<string, SendTarget> sendto[] = {
  { "world", SendTarget::World },
  { "command", SendTarget::Command },
  { "output", SendTarget::Output },
  { "status", SendTarget::Status },
  { "notepad", SendTarget::NotepadNew },
  { "notepadappend", SendTarget::NotepadAppend },
  { "logfile", SendTarget::Log },
  { "notepadreplace", SendTarget::NotepadReplace },
  { "commandqueue", SendTarget::WorldDelay },
  { "variable", SendTarget::Variable },
  { "execute", SendTarget::Execute },
  { "speedwalk", SendTarget::Speedwalk },
  { "script", SendTarget::Script },
  { "immediate", SendTarget::WorldImmediate },
  { "scriptafteromit", SendTarget::ScriptAfterOmit }
};

static const pair<string, int> sqlite3_[] = { { "OK", SQLITE_OK },
                                              { "ERROR", SQLITE_ERROR },
                                              { "INTERNAL", SQLITE_INTERNAL },
                                              { "PERM", SQLITE_PERM },
                                              { "ABORT", SQLITE_ABORT },
                                              { "BUSY", SQLITE_BUSY },
                                              { "LOCKED", SQLITE_LOCKED },
                                              { "NOMEM", SQLITE_NOMEM },
                                              { "READONLY", SQLITE_READONLY },
                                              { "INTERRUPT", SQLITE_INTERRUPT },
                                              { "IOERR", SQLITE_IOERR },
                                              { "CORRUPT", SQLITE_CORRUPT },
                                              { "NOTFOUND", SQLITE_NOTFOUND },
                                              { "FULL", SQLITE_FULL },
                                              { "CANTOPEN", SQLITE_CANTOPEN },
                                              { "PROTOCOL", SQLITE_PROTOCOL },
                                              { "EMPTY", SQLITE_EMPTY },
                                              { "SCHEMA", SQLITE_SCHEMA },
                                              { "TOOBIG", SQLITE_TOOBIG },
                                              { "CONSTRAINT",
                                                SQLITE_CONSTRAINT },
                                              { "MISMATCH", SQLITE_MISMATCH },
                                              { "MISUSE", SQLITE_MISUSE },
                                              { "NOLFS", SQLITE_NOLFS },
                                              { "AUTH", SQLITE_AUTH },
                                              { "FORMAT", SQLITE_FORMAT },
                                              { "RANGE", SQLITE_RANGE },
                                              { "NOTADB", SQLITE_NOTADB },
                                              { "NOTICE", SQLITE_NOTICE },
                                              { "WARNING", SQLITE_WARNING },
                                              { "ROW", SQLITE_ROW },
                                              { "DONE", SQLITE_DONE },
                                              { "INTEGER", SQLITE_INTEGER },
                                              { "FLOAT", SQLITE_FLOAT },
                                              { "BLOB", SQLITE_BLOB },
                                              { "TEXT", SQLITE_TEXT } };

static const pair<string, TimerFlag> timer_flag[] = {
  { "Enabled", TimerFlag::Enabled },
  { "AtTime", TimerFlag::AtTime },
  { "OneShot", TimerFlag::OneShot },
  { "TimerSpeedwalk", TimerFlag::TimerSpeedWalk },
  { "TimerNote", TimerFlag::TimerNote },
  { "ActiveWhenClosed", TimerFlag::ActiveWhenClosed },
  { "Replace", TimerFlag::Replace },
  { "Temporary", TimerFlag::Temporary }
};

static const pair<string, TriggerFlag> trigger_flag[] = {
  { "Enabled", TriggerFlag::Enabled },
  { "OmitFromLog", TriggerFlag::OmitFromLog },
  { "OmitFromOutput", TriggerFlag::OmitFromOutput },
  { "KeepEvaluating", TriggerFlag::KeepEvaluating },
  { "IgnoreCase", TriggerFlag::IgnoreCase },
  { "RegularExpression", TriggerFlag::RegularExpression },
  { "ExpandVariables", TriggerFlag::ExpandVariables },
  { "Replace", TriggerFlag::Replace },
  { "LowercaseWildcard", TriggerFlag::LowercaseWildcard },
  { "Temporary", TriggerFlag::Temporary },
  { "OneShot", TriggerFlag::OneShot }
};

void
pushValue(lua_State* L, int value)
{
  lua_pushinteger(L, value);
}
void
pushValue(lua_State* L, const string& value)
{
  qlua::pushString(L, value);
}
#define IMPL_PUSH_ENUM(T)                                                      \
  void pushValue(lua_State* L, T value)                                        \
  {                                                                            \
    lua_pushinteger(L, (int)value);                                            \
  }

IMPL_PUSH_ENUM(AliasFlag)
IMPL_PUSH_ENUM(ApiCode)
IMPL_PUSH_ENUM(OperatingSystem)
IMPL_PUSH_ENUM(SendTarget)
IMPL_PUSH_ENUM(TimerFlag)
IMPL_PUSH_ENUM(TriggerFlag)

template<typename K, typename V, size_t N>
void
registerTable(lua_State* L, const char* name, const pair<K, V> (&entries)[N])
{
  const bool isNew = lua_getglobal(L, name) == LUA_TNIL;
  if (isNew) {
    lua_pop(L, 1);
    lua_createtable(L, 0, N);
  }
  for (const pair<K, V>& entry : entries) {
    pushValue(L, entry.first);
    pushValue(L, entry.second);
    lua_rawset(L, -3);
  }
  if (isNew) {
    lua_setglobal(L, name);
  }
}

int
registerLuaGlobals(lua_State* L)
{
  qlua::pushQString(L, QCoreApplication::applicationVersion());
  lua_setglobal(L, "SMUSHCLIENT_VERSION");
  registerTable(L, "alias_flag", alias_flag);
  registerTable(L, "custom_colour", custom_colour);
  registerTable(L, "error_code", error_code);
  registerTable(L, "error_desc", error_desc);
  registerTable(L, "miniwin", miniwin);
  registerTable(L, "operating_system", operating_system);
  registerTable(L, "sendto", sendto);
  registerTable(L, "sqlite3", sqlite3_);
  registerTable(L, "timer_flag", timer_flag);
  registerTable(L, "trigger_flag", trigger_flag);
  return 0;
}
