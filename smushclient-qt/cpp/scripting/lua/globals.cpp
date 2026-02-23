#include "globals.h"
#include "../miniwindow/miniwindow.h"
#include "../qlua.h"
#include "smushclient_qt/src/ffi/send_request.cxx.h"
#include "sqlite3.h"
#include <string>
#include <utility>
extern "C"
{
#include "lua.h"
}

using std::pair;
using std::string;

#define I(val) static_cast<lua_Integer>(val)

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

static const pair<string, lua_Integer> miniwin[] = {
  { "pos_stretch_to_view", I(MiniWindow::Position::OutputStretch) },
  { "pos_stretch_to_view_with_aspect", I(MiniWindow::Position::OutputScale) },
  { "pos_stretch_to_owner", I(MiniWindow::Position::OwnerStretch) },
  { "pos_stretch_to_owner_with_aspect", I(MiniWindow::Position::OwnerScale) },
  { "pos_top_left", I(MiniWindow::Position::TopLeft) },
  { "pos_top_center", I(MiniWindow::Position::TopCenter) },
  { "pos_top_right", I(MiniWindow::Position::TopRight) },
  { "pos_center_right", I(MiniWindow::Position::CenterRight) },
  { "pos_bottom_right", I(MiniWindow::Position::BottomRight) },
  { "pos_bottom_center", I(MiniWindow::Position::BottomCenter) },
  { "pos_bottom_left", I(MiniWindow::Position::BottomLeft) },
  { "pos_center_left", I(MiniWindow::Position::CenterLeft) },
  { "pos_center_all", I(MiniWindow::Position::Center) },
  { "pos_tile", I(MiniWindow::Position::Tile) },

  { "create_underneath", MiniWindow::Flag::DrawUnderneath },
  { "create_absolute_location", MiniWindow::Flag::Absolute },
  { "create_transparent", MiniWindow::Flag::Transparent },
  { "create_ignore_mouse", MiniWindow::Flag::IgnoreMouse },
  { "create_keep_hotspots", MiniWindow::Flag::KeepHotspots },

  { "pen_solid", I(ScriptPen::Style::SolidLine) },
  { "pen_dash", I(ScriptPen::Style::DashLine) },
  { "pen_dot", I(ScriptPen::Style::DotLine) },
  { "pen_dash_dot", I(ScriptPen::Style::DashDotLine) },
  { "pen_dash_dot_dot", I(ScriptPen::Style::DashDotDotLine) },
  { "pen_null", I(ScriptPen::Style::NoPen) },
  { "pen_inside_frame", I(ScriptPen::Style::InsideFrame) },

  { "pen_endcap_round", I(ScriptPen::Cap::RoundCap) },
  { "pen_endcap_square", I(ScriptPen::Cap::SquareCap) },
  { "pen_endcap_flat", I(ScriptPen::Cap::FlatCap) },

  { "pen_join_round", I(ScriptPen::Join::RoundJoin) },
  { "pen_join_bevel", I(ScriptPen::Join::BevelJoin) },
  { "pen_join_miter", I(ScriptPen::Join::MiterJoin) },

  { "brush_solid", I(ScriptBrush::SolidPattern) },
  { "brush_null", I(ScriptBrush::NoBrush) },
  { "brush_hatch_horizontal", I(ScriptBrush::HorPattern) },
  { "brush_hatch_vertical", I(ScriptBrush::VerPattern) },
  { "brush_hatch_forwards_diagonal", I(ScriptBrush::FDiagPattern) },
  { "brush_hatch_backwards_diagonal", I(ScriptBrush::BDiagPattern) },
  { "brush_hatch_cross", I(ScriptBrush::CrossPattern) },
  { "brush_hatch_cross_diagonal", I(ScriptBrush::DiagCrossPattern) },
  { "brush_fine_pattern", I(ScriptBrush::Dense4Pattern) },
  { "brush_medium_pattern", I(ScriptBrush::Dense2Pattern) },
  { "brush_coarse_pattern", I(ScriptBrush::Dense1Pattern) },
  { "brush_waves_horizontal", I(ScriptBrush::HorWaves) },
  { "brush_waves_vertical", I(ScriptBrush::VerWaves) },

  { "rect_frame", I(RectOp::Frame) },
  { "rect_fill", I(RectOp::Fill) },
  { "rect_invert", I(RectOp::Invert) },
  { "rect_3d_rect", I(RectOp::Frame3D) },
  { "rect_draw_edge", I(RectOp::Edge3D) },
  { "rect_flood_fill_border", I(RectOp::FloodFillBorder) },
  { "rect_flood_fill_surface", I(RectOp::FloodFillSurface) },
  { "rect_edge_raised", I(ButtonFrame::Raised) },
  { "rect_edge_etched", I(ButtonFrame::Etched) },
  { "rect_edge_bump", I(ButtonFrame::Bump) },
  { "rect_edge_sunken", I(ButtonFrame::Sunken) },
  { "rect_edge_at_top_left", 0x3 },
  { "rect_edge_at_top_right", 0x6 },
  { "rect_edge_at_bottom_left", 0x9 },
  { "rect_edge_at_bottom_right", 0xC },
  { "rect_edge_at_all", 0xF },
  { "rect_diagonal_end_top_left", 0x13 },
  { "rect_diagonal_end_top_right", 0x16 },
  { "rect_diagonal_end_bottom_left", 0x19 },
  { "rect_diagonal_end_bottom_right", 0x1C },
  { "rect_option_fill_middle", MiniWindow::ButtonFlag::Fill },
  { "rect_option_softer_buttons", MiniWindow::ButtonFlag::Soft },
  { "rect_option_flat_borders", MiniWindow::ButtonFlag::Flat },
  { "rect_option_monochrom_borders", MiniWindow::ButtonFlag::Monochrome },

  { "circle_ellipse", I(CircleOp::Ellipse) },
  { "circle_rectangle", I(CircleOp::Rectangle) },
  { "circle_round_rectangle", I(CircleOp::RoundedRectangle) },
  { "circle_chord", I(CircleOp::Chord) },
  { "circle_pie", I(CircleOp::Pie) },

  { "gradient_horizontal", Qt::Orientation::Horizontal },
  { "gradient_vertical", Qt::Orientation::Vertical },
  { "gradient_texture", 3 },

  { "font_charset_ansi", 0 },
  { "font_charset_default", 1 },
  { "font_charset_symbol", 2 },

  { "font_family_any", I(ScriptFont::Family::AnyFamily) },
  { "font_family_roman", I(ScriptFont::Family::Roman) },
  { "font_family_swiss", I(ScriptFont::Family::Swiss) },
  { "font_family_modern", I(ScriptFont::Family::Modern) },
  { "font_family_script", I(ScriptFont::Family::Script) },
  { "font_family_decorative", I(ScriptFont::Family::Decorative) },

  { "font_pitch_default", I(ScriptFont::Pitch::Default) },
  { "font_pitch_fixed", I(ScriptFont::Pitch::Fixed) },
  { "font_pitch_variable", I(ScriptFont::Pitch::Variable) },
  { "font_pitch_monospaced", I(ScriptFont::Pitch::Monospace) },

  { "font_truetype", 4 },

  { "image_copy", I(DrawImageMode::Copy) },
  { "image_stretch", I(DrawImageMode::Stretch) },
  { "image_transparent_copy", I(DrawImageMode::CopyTransparent) },

  { "image_fill_ellipse", I(ImageOp::Ellipse) },
  { "image_fill_rectangle", I(ImageOp::Rectangle) },
  { "image_fill_round_fill_rectangle", I(ImageOp::RoundedRectangle) },

  { "filter_noise", I(FilterOp::Noise) },
  { "filter_monochrome_noise", I(FilterOp::MonoNoise) },
  { "filter_blur", I(FilterOp::Blur) },
  { "filter_sharpen", I(FilterOp::Sharpen) },
  { "filter_find_edges", I(FilterOp::EdgeDetect) },
  { "filter_emboss", I(FilterOp::Emboss) },
  { "filter_brightness", I(FilterOp::BrightnessAdd) },
  { "filter_contrast", I(FilterOp::Contrast) },
  { "filter_gamma", I(FilterOp::Gamma) },
  { "filter_red_brightness", I(FilterOp::RedBrightnessAdd) },
  { "filter_red_contrast", I(FilterOp::RedContrast) },
  { "filter_red_gamma", I(FilterOp::RedGamma) },
  { "filter_green_brightness", I(FilterOp::GreenBrightnessAdd) },
  { "filter_green_contrast", I(FilterOp::GreenContrast) },
  { "filter_green_gamma", I(FilterOp::GreenGamma) },
  { "filter_blue_brightness", I(FilterOp::BlueBrightnessAdd) },
  { "filter_blue_contrast", I(FilterOp::BlueContrast) },
  { "filter_blue_gamma", I(FilterOp::BlueGamma) },
  { "filter_grayscale", I(FilterOp::GrayscaleLinear) },
  { "filter_normal_grayscale", I(FilterOp::GrayscalePerceptual) },
  { "filter_brightness_multiply", I(FilterOp::BrightnessMult) },
  { "filter_red_brightness_multiply", I(FilterOp::RedBrightnessMult) },
  { "filter_green_brightness_multiply", I(FilterOp::GreenBrightnessMult) },
  { "filter_blue_brightness_multiply", I(FilterOp::BlueBrightnessMult) },
  { "filter_lesser_blur", I(FilterOp::LesserBlur) },
  { "filter_minor_blur", I(FilterOp::MinorBlur) },
  { "filter_average", I(FilterOp::Average) },

  { "blend_normal", I(BlendMode::Normal) },
  { "blend_average", I(BlendMode::Average) },
  { "blend_interpolate", I(BlendMode::Interpolate) },
  { "blend_dissolve", I(BlendMode::Dissolve) },
  { "blend_darken", I(BlendMode::Darken) },
  { "blend_multiply", I(BlendMode::Multiply) },
  { "blend_colour_burn", I(BlendMode::ColorBurn) },
  { "blend_linear_burn", I(BlendMode::LinearBurn) },
  { "blend_inverse_colour_burn", I(BlendMode::InverseColorBurn) },
  { "blend_subtract", I(BlendMode::Subtract) },
  { "blend_lighten", I(BlendMode::Lighten) },
  { "blend_screen", I(BlendMode::Screen) },
  { "blend_colour_dodge", I(BlendMode::ColorDodge) },
  { "blend_linear_dodge", I(BlendMode::LinearDodge) },
  { "blend_inverse_colour_dodge", I(BlendMode::InverseColorDodge) },
  { "blend_add", I(BlendMode::Add) },
  { "blend_overlay", I(BlendMode::Overlay) },
  { "blend_soft_light", I(BlendMode::SoftLight) },
  { "blend_hard_light", I(BlendMode::HardLight) },
  { "blend_vivid_light", I(BlendMode::VividLight) },
  { "blend_linear_light", I(BlendMode::LinearLight) },
  { "blend_pin_light", I(BlendMode::PinLight) },
  { "blend_hard_mix", I(BlendMode::HardMix) },
  { "blend_difference", I(BlendMode::Difference) },
  { "blend_exclusion", I(BlendMode::Exclusion) },
  { "blend_reflect", I(BlendMode::Reflect) },
  { "blend_glow", I(BlendMode::Glow) },
  { "blend_freeze", I(BlendMode::Freeze) },
  { "blend_heat", I(BlendMode::Heat) },
  { "blend_negation", I(BlendMode::Negation) },
  { "blend_phoenix", I(BlendMode::Phoenix) },
  { "blend_stamp", I(BlendMode::Stamp) },
  { "blend_xor", I(BlendMode::Xor) },
  { "blend_and", I(BlendMode::And) },
  { "blend_or", I(BlendMode::Or) },
  { "blend_red", I(BlendMode::Red) },
  { "blend_green", I(BlendMode::Green) },
  { "blend_blue", I(BlendMode::Blue) },
  { "blend_yellow", I(BlendMode::Yellow) },
  { "blend_cyan", I(BlendMode::Cyan) },
  { "blend_magenta", I(BlendMode::Magenta) },
  { "blend_green_limited_by_red", I(BlendMode::GreenLimitedByRed) },
  { "blend_green_limited_by_blue", I(BlendMode::GreenLimitedByBlue) },
  { "blend_green_limited_by_average_of_red_and_blue",
    I(BlendMode::GreenLimitedByRedAndBlue) },
  { "blend_blue_limited_by_red", I(BlendMode::BlueLimitedByRed) },
  { "blend_blue_limited_by_green", I(BlendMode::BlueLimitedByGreen) },
  { "blend_blue_limited_by_average_of_red_and_green",
    I(BlendMode::BlueLimitedByRedAndGreen) },
  { "blend_red_limited_by_green", I(BlendMode::RedLimitedByGreen) },
  { "blend_red_limited_by_blue", I(BlendMode::RedLimitedByBlue) },
  { "blend_red_limited_by_average_of_green_and_blue",
    I(BlendMode::RedLimitedByGreenAndBlue) },
  { "blend_red_only", I(BlendMode::RedOnly) },
  { "blend_green_only", I(BlendMode::GreenOnly) },
  { "blend_blue_only", I(BlendMode::BlueOnly) },
  { "blend_discard_red", I(BlendMode::DiscardRed) },
  { "blend_discard_green", I(BlendMode::DiscardGreen) },
  { "blend_discard_blue", I(BlendMode::DiscardBlue) },
  { "blend_all_red", I(BlendMode::AllRed) },
  { "blend_all_green", I(BlendMode::AllGreen) },
  { "blend_all_blue", I(BlendMode::AllBlue) },
  { "blend_hue_mode", I(BlendMode::Hue) },
  { "blend_saturation_mode", I(BlendMode::Saturation) },
  { "blend_colour_mode", I(BlendMode::Color) },
  { "blend_luminance_mode", I(BlendMode::Luminance) },
  { "blend_hsl", I(BlendMode::Hsl) },

  { "cursor_none", I(ScriptCursor::BlankCursor) },
  { "cursor_arrow", I(ScriptCursor::ArrowCursor) },
  { "cursor_hand", I(ScriptCursor::OpenHandCursor) },
  { "cursor_ibeam", I(ScriptCursor::IBeamCursor) },
  { "cursor_plus", I(ScriptCursor::CrossCursor) },
  { "cursor_wait", I(ScriptCursor::WaitCursor) },
  { "cursor_up", I(ScriptCursor::UpArrowCursor) },
  { "cursor_nw_se_arrow", I(ScriptCursor::SizeFDiagCursor) },
  { "cursor_ne_sw_arrow", I(ScriptCursor::SizeBDiagCursor) },
  { "cursor_ew_arrow", I(ScriptCursor::SizeHorCursor) },
  { "cursor_ns_arrow", I(ScriptCursor::SizeVerCursor) },
  { "cursor_both_arrow", I(ScriptCursor::SizeAllCursor) },
  { "cursor_x", I(ScriptCursor::ForbiddenCursor) },
  { "cursor_help", I(ScriptCursor::WhatsThisCursor) },

  { "hotspot_report_all_mouseovers", Hotspot::Flag::ReportAllMouseovers },

  { "hotspot_got_shift", Hotspot::EventFlag::Shift },
  { "hotspot_got_control", Hotspot::EventFlag::Control },
  { "hotspot_got_alt", Hotspot::EventFlag::Alt },
  { "hotspot_got_lh_mouse", Hotspot::EventFlag::MouseLeft },
  { "hotspot_got_rh_mouse", Hotspot::EventFlag::MouseRight },
  { "hotspot_got_dbl_click", Hotspot::EventFlag::DoubleClick },
  { "hotspot_got_not_first", Hotspot::EventFlag::Hover },
  { "hotspot_got_middle_mouse", Hotspot::EventFlag::MouseMiddle },

  { "merge_straight", I(MergeMode::Straight) },
  { "merge_transparent", I(MergeMode::Transparent) },

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

namespace {
template<typename K, typename V, size_t N>
void
registerTable(lua_State* L, const char* name, const pair<K, V> (&entries)[N])
{
  const bool isNew = lua_getglobal(L, name) == LUA_TNIL;
  if (isNew) {
    lua_createtable(L, 0, N);
  }
  for (const pair<K, V>& entry : entries) {
    qlua::pushEntry(L, entry.first, entry.second);
  }
  if (isNew) {
    lua_setglobal(L, name);
  } else {
    lua_pop(L, 1);
  }
}
} // namespace

int
registerLuaGlobals(lua_State* L)
{
  qlua::push(L, QCoreApplication::applicationVersion());
  lua_setglobal(L, "SMUSHCLIENT_VERSION");
  registerTable(L, "alias_flag", alias_flag);
  registerTable(L, "custom_colour", custom_colour);
  registerTable(L, "error_code", error_code);
  registerTable(L, "error_desc", error_desc);
  registerTable(L, "miniwin", miniwin);
  registerTable(L, "operating_system", operating_system);
  registerTable(L, "sendto", sendto);
  registerTable(L, "timer_flag", timer_flag);
  registerTable(L, "trigger_flag", trigger_flag);
  return 0;
}
