---@meta

---@class Rectangle
---@field top integer
---@field height integer
---@field left integer
---@field width integer

---OR combination of the following flags:
---
---Pen style:\
---`miniwin.pen_solid` (0)\
---`miniwin.pen_dash` (1)\
---`miniwin.pen_dot` (2)\
---`miniwin.pen_dash_dot` (3)\
---`miniwin.pen_dash_dot_dot` (4)\
---`miniwin.pen_null` (5)\
---`miniwin.pen_inside_frame` (6)
---
---Endcap:\
---`miniwin.pen_endcap_round` (0)\
---`miniwin.pen_endcap_square` (256)\
---`miniwin.pen_endcap_flat` (512)
---
---Join:\
---`miniwin.pen_join_round` (0)\
---`miniwin.pen_join_bevel` (4096)\
---`miniwin.pen_join_miter` (8192)
---@alias WindowPenStyle integer

---This draws an arc from `x1`,`y1` to `x2`,`y2` with the designated pen.
---
---`left`, `top`, `right`, `bottom` — describes the rectangle into which the arc must fit.
---
---`x1`, `y1` — the starting point for the arc - this point does not have to lie exactly on the arc.
---
---`x2`, `y2` — the ending point for the arc - this point does not have to lie exactly on the arc.
---@param windowName string The name of an existing miniwindow.
---@param left number
---@param top number
---@param right number
---@param bottom number
---@param x1 number
---@param y1 number
---@param x2 number
---@param y2 number
---@param penColour integer|string Integer BBGGRR colour code, string hex code, or string colour name.
---@param penStyle WindowPenStyle See [`WindowPenStyle`](lua://WindowPenStyle).
---@param penWidth number Brush width in pixels.
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.ePenStyleNotValid`: Invalid pen style.\
---`error_code.eOK`: Completed OK.
function WindowArc(windowName, left, top, right, bottom, x1, y1, x2, y2, penColour, penStyle, penWidth) end


---This blends the specified image into the miniwindow, on top of the specified rectangle. This function uses the entire image as its source rectangle.
---
---`left`, `top`, `right`, `bottom` — describes the destination rectangle. The blend image is merged with the contents of this rectangle to produce the blended result.
---@param windowName string The name of an existing miniwindow.
---@param imageID string ID of an image loaded into the miniwindow with [`WindowLoadImage`](lua://WindowLoadImage), [`WindowLoadImageMemory`](lua://WindowLoadImageMemory), or [`WindowImageFromWindow`](lua://WindowImageFromWindow).
---@param left number
---@param top number
---@param right number
---@param bottom number
---@param mode miniwin.blend See [`miniwin.blend`](lua://miniwin.blend).
---@param opacity number Between 0 and 1. An opacity of 0 means you will not see the blend image. An opacity of 1 means the blend is fully applied. An opacity of 0.5 means the blended image is merged 50% with the original contents of the miniwindow.
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.eImageNotInstalled`: That image was not loaded.\
---`error_code.eBadParameter`: Opacity less than zero or greater than 1.\
---`error_code.eOK`: Completed OK.
function WindowBlendImage(windowName, imageID, left, top, right, bottom, mode, opacity, srcLeft, srcTop, srcRight, srcBottom) end

---This blends the specified image into the miniwindow, on top of the specified rectangle.
---
---`left`, `top`, `right`, `bottom` — describes the destination rectangle. The blend image is merged with the contents of this rectangle to produce the blended result.
---
---`srcLeft`, `srcTop`, `srcRight`, `srcBottom` — the source rectangle in the original image (use 0,0,0,0 to get the whole image, or use the overload that omits all four parameters). Negative numbers for the *srcRight* and *srcBottom* parameters represent an offset from the bottom or right edge.
---@param windowName string The name of an existing miniwindow.
---@param imageID string ID of an image loaded into the miniwindow with [`WindowLoadImage`](lua://WindowLoadImage), [`WindowLoadImageMemory`](lua://WindowLoadImageMemory), or [`WindowImageFromWindow`](lua://WindowImageFromWindow).
---@param left number
---@param top number
---@param right number
---@param bottom number
---@param mode miniwin.blend See [`miniwin.blend`](lua://miniwin.blend).
---@param opacity number Between 0 and 1. An opacity of 0 means you will not see the blend image. An opacity of 1 means the blend is fully applied. An opacity of 0.5 means the blended image is merged 50% with the original contents of the miniwindow.
---@param srcLeft number
---@param srcTop number
---@param srcRight number
---@param srcBottom number
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.eImageNotInstalled`: That image was not loaded.\
---`error_code.eBadParameter`: Opacity less than zero or greater than 1.\
---`error_code.eOK`: Completed OK.
function WindowBlendImage(windowName, imageID, left, top, right, bottom, mode, opacity, srcLeft, srcTop, srcRight, srcBottom) end


---Draws an ellipse or rectangle, controlled by the *action* parameter.
---
---Unlike [`WindowRectOp`](lua://WindowRectOp), the rectangles drawn here can be filled with a brush, thus allowing you to have patterned interiors if desired.
---
---`left`, `top`, `right`, `bottom` — describes the rectangle into which the shape must fit.
---@param windowName string The name of an existing miniwindow.
---@param action
---| 1 # (miniwin.circle_ellipse) Ellipse
---| 2 # (miniwin.circle_rectangle) Rectangle
---@param left number
---@param top number
---@param right number
---@param bottom number
---@param penColour integer|string Integer BBGGRR colour code, string hex code, or string colour name.
---@param penStyle WindowPenStyle See [`WindowPenStyle`](lua://WindowPenStyle).
---@param penWidth number
---@param brushColour integer|string Integer BBGGRR colour code, string hex code, or string colour name.
---@param brushStyle? miniwin.brush See [`miniwin.brush`](lua://miniwin.brush). Default: 0 (solid).
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.ePenStyleNotValid`: Invalid pen style.\
---`error_code.eBrushStyleNotValid`: Invalid brush style.\
---`error_code.eUnknownOption`: Action not in list above.\
---`error_code.eOK`: Completed OK.
---
---@see WindowRectOp
function WindowCircleOp(windowName, action, left, top, right, bottom, penColour, penStyle, penWidth, brushColour,
                        brushStyle) end

---This draws a rounded rectangle.
---
---Unlike [`WindowRectOp`](lua://WindowRectOp), the rectangles drawn here can be filled with a brush, thus allowing you to have patterned interiors if desired.
---
---`left`, `top`, `right`, `bottom` — describes the rectangle into which the shape must fit.
---@param windowName string The name of an existing miniwindow.
---@param action
---| 3 #(miniwin.circle_round_rectangle) Rounded rectangle
---@param left number
---@param top number
---@param right number
---@param bottom number
---@param penColour integer|string Integer BBGGRR colour code, string hex code, or string colour name.
---@param penStyle WindowPenStyle See [`WindowPenStyle`](lua://WindowPenStyle).
---@param penWidth number Pixel width of the pen.
---@param brushColour integer|string Integer BBGGRR colour code, string hex code, or string colour name.
---@param brushStyle miniwin.brush See [`miniwin.brush`](lua://miniwin.brush).
---@param roundedWidth number Width of the ellipse used to draw the rounded corner.
---@param roundedHeight number Height of the ellipse used to draw the rounded corner.
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.ePenStyleNotValid`: Invalid pen style.\
---`error_code.eBrushStyleNotValid`: Invalid brush style.\
---`error_code.eUnknownOption`: Action not in list above.\
---`error_code.eOK`: Completed OK.
---
---@see WindowRectOp
function WindowCircleOp(windowName, action, left, top, right, bottom, penColour, penStyle, penWidth, brushColour,
                        brushStyle, roundedWidth, roundedHeight) end


---This creates (or re-creates) a miniwindow of the given name. It is not initially shown so it will be invisible until you call [`WindowShow`](lua://WindowShow). If there is an existing, visible, window of that name, then it will be marked invisible, so you will need to call `WindowShow` to make it visible again.
---
---If you want to change the size of a miniwindow you don't need to delete it, simply call `WindowCreate` again, which causes it to be recreated in the new location / size. However you can call [`WindowDelete`](lua://WindowDelete) to completely delete an existing miniwindow.
---
---Alternatively, call [`WindowResize`](lua://WindowResize) which resizes the window, keeping all fonts, images and hotspots.
---
---It is reasonable to use `WindowCreate` to create a very small window (eg. 0 x 0 pixels) simply to load images or fonts into it. Thus you can find out font sizes, in order to calculate what size window needs to be re-created, so it is the correct size to hold a particular piece of text.
---
---If you call `WindowCreate` on an existing window then:
---
---* The existing offscreen image will be discarded, and the memory it used freed.
---* A new offscreen image will be created, using the new dimensions. Thus you can use `WindowCreate` to resize a window. For example, you might do this for an inventory window, if you get more inventory.
---* The image will be cleared to the background colour you specify.
---* Any hotspots you added are removed - unless flag `miniwin.create_keep_hotspots` (0x10) is set, see below. The assumption is that if you are starting drawing from scratch, any hotspot items will probably appear in a different place.\
---* The window's "show" flag is set to `false`. Thus you need to call [`WindowShow`](lua://WindowShow) before it will become visible.
---@param windowName string The name we are calling it - needed for all subsequent operations on this miniwindow. We suggest you use [`GetPluginID()`](lua://GetPluginID) to get a unique name for this window. If you plan to use more than one window in a plugin, simply append "A", "B", etc. to the plugin ID. The name may not be the empty string.
---@param left integer Offset in pixels from the left edge of the output window.
---@param top integer Offset in pixels from the top edge of the output window.
---@param width integer Width of the window, in pixels.
---@param height integer Height of the window, in pixels.
---@param position miniwin.pos Indicates where you want the window positioned on the screen. Whenever the screen is resized or redrawn the contents of the window are drawn in the requested position. See [`miniwin.pos`](lua://miniwin.pos).
---@param flags integer OR combination of the following flags: \
---`miniwin.create_underneath` (1): Draw underneath. If set, the miniwindow is drawn beneath the scrolling text in the output window. WARNING: If you set the "draw underneath" flag then you cannot use hotspots, as the hotspots are underneath the text and will not be detected.\
---`miniwin.create_absolute_location` (2): Absolute location. If set, the miniwindow is not subject to auto positioning (so the *position* argument is ignored), and it is located exactly at the *left*, *top* position designated in the function call. By setting this bit you have absolute control over where the window will appear.\
---`miniwin.create_transparent` (4): Transparent. If set, whenever a pixel in the contents of the window matches the *backgroundColour*, it is not drawn, and the text underneath shows through. This lets you make odd-shape windows like stars or circles, by filling the outside (the part you don't want to see) with the background colour.\
---`miniwin.create_ignore_mouse` (8): Ignore mouse. If set, this miniwindow is not considered for mouse-over, mouse-down, mouse-up events. WARNING: If you set the "ignore mouse" flag then you cannot use hotspots, as mouse clicks and movement will not be detected.\
---`miniwin.create_keep_hotspots` (16): Keep existing hotspots. If set, hotspots are not deleted if you are recreating an existing miniwindow.
---@param backgroundColour integer|string Integer BBGGRR colour code, string hex code, or string colour name for the colour that the window is initially filled with, and used when doing transparent drawing.
---@return error_code code #
---`error_code.eNoNameSpecified`: Miniwindow name must be specified.\
---`error_code.eBadParameter`: Width or height less than zero.\
---`error_code.eOK`: Success.
---
---@see WindowDelete
---@see WindowPosition
---@see WindowResize
---@see WindowSetZOrder
---@see WindowShow
function WindowCreate(windowName, left, top, width, height, position, flags, backgroundColour) end


---This creates a small image (bitmap) by specifying the bit pattern for an 8 x 8 bit image. This is primarily intended for making patterns for use with [`WindowImageOp`](lua://WindowImageOp), described elsewhere. By designing suitable patterns you could make grass, waves, or other landscape effects for use in maps.
---
---The easiest way of making your own pattern is to use Lua's [`tonumber`](lua://tonumber) function and work in binary. When doing that, each 1 represents a bit that is set (and is thus in the PenColour) and each '0' represents a bit that is clear (and is thus in the BrushColour).
---
---`row1`, `row2`, `row3`, `row4`, `row5`, `row6`, `row7`, `row8` — the bit pattern for each row.
---@param windowName string The name of an existing miniwindow.
---@param imageID string The image id to store this image under.
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.
---`error_code.eOK`: Completed OK.
---
---@see WindowImageOp
function WindowCreateImage(windowName, imageID, row8, row7, row6, row5, row4, row3, row2, row1) end




---This deletes an existing miniwindow, freeing all the resources it used.
---@param windowName string The name of the existing window.
---@return error_code code #
---`error_code.eNoSuchWindow`: There is no such miniwindow.\
---`error_code.eOK`: Success.
---
---@see WindowCreate
---@see WindowShow
function WindowDelete(windowName) end


---This copies an image to the miniwindow. This function uses the entire image as its source rectangle.
---
---`left`, `top`, `right`, `bottom` — describes the rectangle to be drawn into.
---@param windowName string The name of an existing miniwindow.
---@param imageID string ID of an image loaded into the miniwindow with [`WindowLoadImage`](lua://WindowLoadImage), [`WindowLoadImageMemory`](lua://WindowLoadImageMemory), or [`WindowImageFromWindow`](lua://WindowImageFromWindow).
---@param left number
---@param top number
---@param right number
---@param bottom number
---@param mode
---| 1 # (miniwin.image_copy) Copy without stretching to the destination position. The image is not clipped, so only the left and top parameters are used - the full image is copied to that position. (miniwin.image_copy)
---| 2 # (miniwin.image_stretch) Stretch or shrink the image appropriately to fit into the rectangle: left, top, right, bottom.
---| 3 # (miniwin.image_transparent_copy) Copy without stretching to the position left, top. However this is a transparent copy, where the pixel at the left,top corner (pixel position 0,0) is considered the transparent colour. Any pixels that exactly match that colour are not copied.
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.eImageNotInstalled`: That image was not loaded.\
---`error_code.eBadParameter`: Drawing mode not in list above.\
---`error_code.eOK`: Completed OK.
function WindowDrawImage(windowName, imageID, left, top, right, bottom, mode) end

---This copies an image to the miniwindow.
---
---`left`, `top`, `right`, `bottom` — describes the rectangle to be drawn into.
---
---`srcLeft`, `srcTop`, `srcRight`, `srcBottom` — the source rectangle in the original image (use 0,0,0,0 to get the whole image, or use the overload that omits all four parameters). Negative numbers for the *srcRight* and *srcBottom* parameters represent an offset from the bottom or right edge.
---@param windowName string The name of an existing miniwindow.
---@param imageID string ID of an image loaded into the miniwindow with [`WindowLoadImage`](lua://WindowLoadImage), [`WindowLoadImageMemory`](lua://WindowLoadImageMemory), or [`WindowImageFromWindow`](lua://WindowImageFromWindow).
---@param left number
---@param top number
---@param right number
---@param bottom number
---@param mode
---| 1 # (miniwin.image_copy) Copy without stretching to the destination position. The image is not clipped, so only the left and top parameters are used - the full image is copied to that position.
---| 2 # (miniwin.image_stretch) Stretch or shrink the image appropriately to fit into the rectangle: left, top, right, bottom.
---| 3 # (miniwin.image_transparent_copy) Copy without stretching to the position left, top. However this is a transparent copy, where the pixel at the left,top corner (pixel position 0,0) is considered the transparent colour. Any pixels that exactly match that colour are not copied.
---@param srcLeft number
---@param srcTop number
---@param srcRight number
---@param srcBottom number
---@return error_code code #
---`error_code.eNoSuchWindow`: no such miniwindow\
---`error_code.eImageNotInstalled`: that image was not loaded\
---`error_code.eBadParameter`: drawing mode not in list above\
---`error_code.eOK`: completed OK
function WindowDrawImage(windowName, imageID, left, top, right, bottom, mode, srcLeft, srcTop, srcRight, srcBottom) end


---This copies an image to the miniwindow, using the alpha channel to show transparent areas.
---
---`left`, `top`, `right`, `bottom` — describes the rectangle to be drawn to.
---@param windowName string The name of an existing miniwindow.
---@param imageID string ID of an image loaded into the miniwindow with [`WindowLoadImage`](lua://WindowLoadImage), [`WindowLoadImageMemory`](lua://WindowLoadImageMemory), or [`WindowImageFromWindow`](lua://WindowImageFromWindow).
---@param left number
---@param top number
---@param right number
---@param bottom number
---@param opacity number The amount of opacity, between 0 and 1 (floating-point number). An opacity of 0 means you will not see the drawn image. An opacity of 1 means the image is fully drawn (apart from where the alpha channel makes part of it opaque). An opacity of 0.5 means the image is merged 50% with the original contents of the miniwindow.
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.eImageNotInstalled`: That image was not loaded, or the image does not have an alpha channel.\
---`error_code.eBadParameter`: Opacity not in range 0 to 1.\
---`error_code.eOK`: Completed OK.
function WindowDrawImageAlpha(windowName, imageID, left, top, right, bottom, opacity, srcLeft, srcTop) end

---This copies an image to the miniwindow, using the alpha channel to show transparent areas.
---
---`left`, `top`, `right`, `bottom` — describes the rectangle to be drawn to.
---
---`srcLeft`, `srcTop` — the source offset in the original image (use 0,0 to get the whole image, or use the overload that omits both parameters).
---@param windowName string The name of an existing miniwindow.
---@param imageID string ID of an image loaded into the miniwindow with [`WindowLoadImage`](lua://WindowLoadImage), [`WindowLoadImageMemory`](lua://WindowLoadImageMemory), or [`WindowImageFromWindow`](lua://WindowImageFromWindow).
---@param left number
---@param top number
---@param right number
---@param bottom number
---@param opacity number The amount of opacity, between 0 and 1 (floating-point number). An opacity of 0 means you will not see the drawn image. An opacity of 1 means the image is fully drawn (apart from where the alpha channel makes part of it opaque). An opacity of 0.5 means the image is merged 50% with the original contents of the miniwindow.
---@param srcLeft number
---@param srcTop number
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.eImageNotInstalled`: That image was not loaded, or the image does not have an alpha channel.\
---`error_code.eBadParameter`: Opacity not in range 0 to 1.\
---`error_code.eOK`: Completed OK.
function WindowDrawImageAlpha(windowName, imageID, left, top, right, bottom, opacity, srcLeft, srcTop) end


---This takes a copy of the specified rectangle in a miniwindow, filters it according to the operation specified with an *integer* option parameter, and replaces the filtered version back in place.
---
---`left`, `top`, `right`, `bottom` — describes the rectangle to be filtered.
---@param windowName string The name of an existing window.
---@param left number
---@param top number
---@param right number
---@param bottom number
---@param filter
---| 7 # (miniwin.filter_brightness) Adjust Brightness - by adding Options to each pixel
---| 10 # (miniwin.filter_red_brightness) Adjust Brightness for red channel only - by adding Options to each pixel
---| 13 # (miniwin.filter_green_brightness) Adjust Brightness for green channel only - by adding Options to each pixel
---| 16 # (miniwin.filter_blue_brightness) Adjust Brightness for blue channel only - by adding Options to each pixel
---@param option integer
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.eUnknownOption`: Filter mode not in the list above.\
---`error_code.eOK`: Completed OK.
function WindowFilter(windowName, left, top, right, bottom, filter, option) end

---This takes a copy of the specified rectangle in a miniwindow, filters it according to the operation specified with a *number* option parameter, and replaces the filtered version back in place.
---
---`left`, `top`, `right`, `bottom` — describes the rectangle to be filtered.
---@param windowName string The name of an existing window.
---@param left number
---@param top number
---@param right number
---@param bottom number
---@param filter
---| 1 # (miniwin.filter_noise) Apply colour noise
---| 2 # (miniwin.filter_monochrome_noise) Apply monochrome noise
---| 8 # (miniwin.filter_contrast) Adjust Contrast
---| 9 # (miniwin.filter_gamma) Adjust Gamma
---| 11 # (miniwin.filter_red_contrast) Adjust Contrast for red channel only
---| 12 # (miniwin.filter_red_gamma) Adjust Gamma for red channel only
---| 14 # (miniwin.filter_green_contrast) Adjust Contrast for green channel only
---| 15 # (miniwin.filter_green_gamma) Adjust Gamma for green channel only
---| 17 # (miniwin.filter_blue_contrast) Adjust Contrast for blue channel only
---| 18 # (miniwin.filter_blue_gamma) Adjust Gamma for blue channel only
---| 21 # (miniwin.filter_brightness_multiply) Adjust Brightness - by multiplying Options by each pixel
---| 22 # (miniwin.filter_red_brightness_multiply) Adjust Brightness for red channel only - by multiplying Options by each pixel
---| 23 # (miniwin.filter_green_brightness_multiply) Adjust Brightness for green channel only - by multiplying Options by each pixel
---| 24 # (miniwin.filter_blue_brightness_multiply) Adjust Brightness for blue channel only - by multiplying Options by each pixel
---@param option number
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.eUnknownOption`: Filter mode not in the list above.\
---`error_code.eOK`: Completed OK.
function WindowFilter(windowName, left, top, right, bottom, filter, option) end

---This takes a copy of the specified rectangle in a miniwindow, filters it according to the operation specified without any parameter, and replaces the filtered version back in place.
---
---`left`, `top`, `right`, `bottom` — describes the rectangle to be filtered.
---@param windowName string The name of an existing window.
---@param left number
---@param top number
---@param right number
---@param bottom number
---@param filter
---| 3 # (miniwin.filter_blur) Blur - by comparing 2 pixels on each side
---| 4 # (miniwin.filter_sharpen) Sharpen
---| 5 # (miniwin.filter_find_edges) Find edges
---| 6 # (miniwin.filter_emboss) Emboss
---| 19 # (miniwin.filter_grayscale) Convert to grayscale - mix red/green/blue equally
---| 20 # (miniwin.filter_normal_grayscale) Convert to grayscale - mix 30% red + 59% green + 11% blue for normal perception
---| 25 # (miniwin.filter_lesser_blur) Lesser Blur - by comparing 1 pixel on each side
---| 26 # (miniwin.filter_minor_blur) Minor Blur - by comparing 1 pixel on each side, taking a smaller amount
---| 27 # (miniwin.filter_average) Average - the entire rectangle is set to its average colour
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.eUnknownOption`: Filter mode not in the list above.\
---`error_code.eOK`: Completed OK.
function WindowFilter(windowName, left, top, right, bottom, filter, option) end


---This loads the specified font into the miniwindow, and remembers it by the nominated "font id". The font id is later used for drawing in this font. You do not need to specify a colour, that is done when the text is actually drawn.
---@param windowName string The name of an existing miniwindow.
---@param fontID string The font id to be associated with this particular font and size. If you are only planning to use one or two fonts you might call them "f1" and "f2". For more complex situations you might call them something like "heading", "body", "boldbody", "footer" etc. If you use an existing fontID then the existing font is deleted and replaced by the one specified now.
---@param fontName string The name of the font (for example, "Lucida Console", "Comic Sans MS", "Fixedsys"). If the *fontName* is an empty string (ie. "") and the *pointSize* is zero, then any existing font of the same *fontID* is deleted (and no new font is added). You can use the [`utils.getfontfamilies`](lua://utils.getfontfamilies) function to find which font names are actually installed.
---@param pointSize number The size of the font in points.
---@param bold? boolean Use the bold version of the font. Default: `false`.
---@param italic? boolean Use the italic version of the font. Default: `false`.
---@param underline? boolean Use the underlined version of the font. Default: `false`.
---@param strikeout? boolean Use the struck-out version of the font. Default: `false`.
---@param _charset? integer Unused.
---@param pitchAndFamily? integer OR combination of the following flags:
---Family\
---`miniwin.font_family_any` (0): Don't care\
---`miniwin.font_family_roman` (16): Roman (variable width, serif, eg. Times Roman)\
---`miniwin.font_family_swiss` (32): Swiss (variable width, sans-serif, eg. Helvetica)\
---`miniwin.font_family_modern` (48): Modern (fixed width, serif or sans-serif - eg. Courier)\
---`miniwin.font_family_script` (64): Script (cursive, etc.)\
---`miniwin.font_family_decorative` (80): Decorative (Old English, etc.)
---
---Pitch\
---`miniwin.font_pitch_default` (0): Default\
---`miniwin.font_pitch_fixed` (1): Fixed pitch\
---`miniwin.font_pitch_variable` (2): Variable pitch\
---`miniwin.font_pitch_monospaced` (8): Mono font
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.eCannotAddFont`: Cannot add it (perhaps font does not exist).\
---`error_code.eOK`: Completed OK.
---
---@see AddFont - import a font file.
---@see SetInputFont - set font of command input.
---@see SetOutputFont - set font of MUD output.
function WindowFont(windowName, fontID, fontName, pointSize, bold, italic, underline, strikeout, _charset, pitchAndFamily) end


---This returns a list of all fonts loaded into this miniwindow. You could use this to find which fonts have been loaded, and then use [`WindowFontInfo`](lua://WindowFontInfo) to find information about each one.
---@param windowName string The name of the miniwindow to check for fonts.
---@return string[] fontIDs
---
---@see WindowFont
---@see WindowFontInfo
function WindowFontList(windowName) end


---This copies the alpha channel of an image to the miniwindow, showing the alpha information as grayscale.
---
---The alpha channel could be used when copying an image onto the miniwindow using the [`WindowMergeImageAlpha`](lua://WindowMergeImageAlpha) function.
---
---Thus you could use `WindowGetImageAlpha` to create a scratch window (one not visible to the player) which holds the alpha information. This could then be turned into another image by using []`WindowImageFromWindow`](lua://WindowImageFromWindow). This image is now available for use with `WindowMergeImageAlpha` to selectively copy the image to another miniwindow (this image would be used as the *maskId*).
---
---`left`, `top`, `right`, `bottom` — describes the rectangle to be drawn to.
---@param windowName string The name of an existing miniwindow.
---@param imageID string ID of an image loaded into the miniwindow with [`WindowLoadImage`](lua://WindowLoadImage), [`WindowLoadImageMemory`](lua://WindowLoadImageMemory), or [`WindowImageFromWindow`](lua://WindowImageFromWindow).
---@param left number
---@param top number
---@param right number
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.eImageNotInstalled`: That image was not loaded, or the image does not have an alpha channel.\
---`error_code.eOK`: Completed OK.
---
---@see WindowMergeAlpha
---@see WindowImageFromWindow
function WindowGetImageAlpha(windowName, imageID, left, top, right, srcLeft, srcTop) end

---This copies the alpha channel of an image to the miniwindow, showing the alpha information as grayscale.
---
---The alpha channel could be used when copying an image onto the miniwindow using the [`WindowMergeImageAlpha`](lua://WindowMergeImageAlpha) function.
---
---Thus you could use `WindowGetImageAlpha` to create a scratch window (one not visible to the player) which holds the alpha information. This could then be turned into another image by using [`WindowImageFromWindow`](lua://WindowImageFromWindow). This image is now available for use with `WindowMergeImageAlpha` to selectively copy the image to another miniwindow (this image would be used as the *maskId*).
---
---`left`, `top`, `right`, `bottom` — describes the rectangle to be drawn to.
---
---`srcLeft`, `srcTop` — the source offset in the original image (use 0,0 to get the whole image, or use the overload that omits both parameters).
---@param windowName string The name of an existing miniwindow.
---@param imageID string ID of an image loaded into the miniwindow with [`WindowLoadImage`](lua://WindowLoadImage), [`WindowLoadImageMemory`](lua://WindowLoadImageMemory), or [`WindowImageFromWindow`](lua://WindowImageFromWindow).
---@param left number
---@param top number
---@param right number
---@param srcLeft number
---@param srcTop number
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.eImageNotInstalled`: That image was not loaded, or the image does not have an alpha channel.\
---`error_code.eOK`: Completed OK.
---
---@see WindowMergeAlpha
---@see WindowImageFromWindow
function WindowGetImageAlpha(windowName, imageID, left, top, right, srcLeft, srcTop) end


---Gets a single pixel colour from the miniwindow.
---@param windowName string The name of an existing miniwindow.
---@param x integer X coordinate of the pixel.
---@param y integer Y coordinate of the pixel.
---@return integer rgb -1 if the pixel is out of the range of the window size, -2 if the miniwindow does not exist, otherwise BBGGRR color code of the colour at that location.
function WindowGetPixel(windowName, x, y) end


---Draws a gradient - that is a rectangle that gradually changes from the start colour to the end colour.
---
---`left`, `top`, `right`, `bottom` — describes the rectangle to be drawn to.
---@param windowName string The name of an existing miniwindow.
---@param left number
---@param top number
---@param right number
---@param bottom number
---@param startColour integer|string Integer BBGGRR colour code, string hex code, or string colour name.
---@param endColour integer|string Integer BBGGRR colour code, string hex code, or string colour name.
---@param mode
---| 1 # (miniwin.gradient_horizontal) Horizontal
---| 2 # (miniwin.gradient_vertical) Vertical
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow (either source or destination).\
---`error_code.eUnknownOption`: Mode not one of the modes specified above.\
---`error_code.eOK`: Completed OK.
function WindowGradient(windowName, left, top, right, bottom, startColour, endColour, mode) end


---This uses an existing miniwindow's offscreen bitmap, to make an image that can be loaded into another miniwindow. This lets you set up offscreen windows (that you may never plan to actually show with [`WindowShow`](lua://WindowShow)) as "work areas" for creating text or images, that can be loaded or blended into other windows.
---@param windowName string The name of an existing miniwindow into which the image should be loaded.
---@param imageID string The image ID to store its image under.
---@param sourceWindowName string The name of an existing window to use as the source of the image.
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow (either source or destination).\
---`error_code.eOK`: Completed OK.
function WindowImageFromWindow(windowName, imageID, sourceWindowName) end


---This returns a list of all images loaded into this miniwindow with [`WindowLoadImage`](lua://WindowLoadImage), [`WindowLoadImageMemory`](lua://WindowLoadImageMemory), or [`WindowImageFromWindow`](lua://WindowImageFromWindow). You could use this to find which images have been loaded, and then use [`WindowImageInfo`](lua://WindowImageInfo) to find information about each one.
---@param windowName string The name of an existing miniwindow.
---@return string[] imageIDs
---
---@see WindowImageInfo
function WindowImageList(windowName) end


---This draws an ellipse, rectangle, round rectangle, controlled by the *action* parameter. The specified shape is filled with a previously-loaded image.
---
---`left`, `top`, `right`, `bottom` — describes the rectangle to be drawn to.
---@param windowName string The name of an existing miniwindow.
---@param action
---| 1 # (miniwin.image_fill_ellipse) Ellipse
---| 2 # (miniwin.image_fill_rectangle) Rectangle
---@param left number
---@param top number
---@param right number
---@param bottom number
---@param penColour integer|string Integer BBGGRR colour code, string hex code, or string colour name.
---@param penStyle WindowPenStyle See [`WindowPenStyle`](lua://WindowPenStyle).
---@param penWidth number
---@param brushColour integer|string Integer BBGGRR colour code, string hex code, or string colour name.
---@param imageID string ID of an image loaded into the miniwindow with [`WindowCreateImage`](lua://WindowCreateImage), [`WindowLoadImage`](lua://WindowLoadImage), [`WindowLoadImageMemory`](lua://WindowLoadImageMemory), or [`WindowImageFromWindow`](lua://WindowImageFromWindow).
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.eImageNotInstalled`: That image was not loaded.\
---`error_code.ePenStyleNotValid`: Invalid pen style.\
---`error_code.eUnknownOption`: Drawing mode not in list above.\
---`error_code.eOK`: Completed OK.
---
---@see WindowCreateImage
function WindowImageOp(windowName, action, left, top, right, bottom, penColour, penStyle, penWidth, brushColour, imageID) end

---This draws an ellipse, rectangle, round rectangle, controlled by the *action* parameter. The specified shape is filled with a previously-loaded image.
---
---`left`, `top`, `right`, `bottom` — describes the rectangle to be drawn to.
---@param windowName string The name of an existing miniwindow.
---@param action
---| 3 # (miniwin.image_fill_round_fill_rectangle) Rounded rectangle
---@param left number
---@param top number
---@param right number
---@param bottom number
---@param penColour integer|string Integer BBGGRR colour code, string hex code, or string colour name.
---@param penStyle WindowPenStyle See [`WindowPenStyle`](lua://WindowPenStyle).
---@param penWidth number
---@param brushColour integer|string Integer BBGGRR colour code, string hex code, or string colour name.
---@param imageID string ID of an image loaded into the miniwindow with [`WindowCreateImage`](lua://WindowCreateImage), [`WindowLoadImage`](lua://WindowLoadImage), [`WindowLoadImageMemory`](lua://WindowLoadImageMemory), or [`WindowImageFromWindow`](lua://WindowImageFromWindow).
---@param roundedWidth number Width of the ellipse used to draw the rounded corner.
---@param roundedHeight number Height of the ellipse used to draw the rounded corner.
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.eImageNotInstalled`: That image was not loaded.\
---`error_code.ePenStyleNotValid`: Invalid pen style.\
---`error_code.eUnknownOption`: Drawing mode not in list above.\
---`error_code.eOK`: Completed OK.
---
---@see WindowCreateImage
function WindowImageOp(windowName, action, left, top, right, bottom, penColour, penStyle, penWidth, brushColour, imageID, roundedWidth, roundedHeight) end


---This draws a line from *x1*,*y1* to *x2*,*y2* with the designated pen.
---@param windowName string
---@param x1 number Starting X coordinate for the line.
---@param y1 number Starting Y coordinate for the line.
---@param x2 number Ending X coordinate for the line.
---@param y2 number Ending Y coordinate for the line.
---@param penColour integer|string Integer BBGGRR colour code, string hex code, or string colour name.
---@param penStyle WindowPenStyle See [`WindowPenStyle`](lua://WindowPenStyle).
---@param penWidth number
---@return error_code code #
---`error_code.eNoSuchWindow`: Mo such miniwindow.\
---`error_code.ePenStyleNotValid`: Invalid pen style.\
---`error_code.eOK`: Completed OK.
function WindowLine(windowName, x1, y1, x2, y2, penColour, penStyle, penWidth) end


---This returns a list of all miniwindows created for this world. You could use this to find which windows have been created, and then use [`WindowInfo`](lua://WindowInfo) to find information about each one.
---@return string[] windowNames
---
---@see WindowInfo
function WindowList() end


---This loads the specified image into the miniwindow, and remembers it by the nominated *imageID*. The image ID is later used for drawing this image. Loading does not draw it, it simply reads it from disk ready for drawing later.
---@param windowName string The name of an existing miniwindow.
---@param imageID string The image ID to be associated with this particular image.
---@param fileName string The disk file to load the image from.
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.eFileNotFound`: Specified file was not found.\
---`error_code.eUnableToLoadImage`: Can't load image - perhaps it is not a recognized format.\
---`error_code.eOK`: Completed OK.
---
---@see WindowLoadImageMemory
function WindowLoadImage(windowName, imageID, fileName) end


---This loads the specified image into the miniwindow from image data in memory, and remembers it by the nominated *imageID*. The image ID is later used for drawing this image.
---@param windowName string The name of an existing miniwindow.
---@param imageID string The image ID to be associated with this particular image.
---@param data string Image file data, preferably in PNG format.
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.eUnableToLoadImage`: Can't load image - perhaps it is not a recognized format.\
---`error_code.eOK`: Completed OK.
function WindowLoadImageMemory(windowName, imageID, data) end


---This creates a pop-up menu inside a miniwindow. This is intended to let you click on an item (for example, a piece of inventory), and select "take/drop/equip/wield" and so on.
---
---The *x* and *y* position must be inside the miniwindow (ie. not negative, and not exceeding the miniwindow's defined width and height). Otherwise, an empty string is returned.
---
---You can use [`WindowInfo(windowName, 14)`](lua://WindowInfo) and [`WindowInfo(windowName, 15)`](lua://WindowInfo) to find where the mouse was last clicked inside this miniwindow.
---
---SUGGESTION - we strongly recommend that, if you are creating a menu in response to a mouse-click, you do it on the "mouse up" rather than "mouse down".
---
---The reason for this is that SmushClient tracks mouse-down events and their corresponding mouse-up events, in order to call the appropriate hotspot routines (mouse-down, mouse-up, mouse-down cancel).
---
---However if you call `WindowMenu` in response to a mouse-down, the next mouse-up is "swallowed" up by the menu handler, and thus MUSHclient doesn't "see" it. The end result is that SmushClient thinks the mouse is down, when it is in fact up. This gets the hotspot events out of synchronization, and then subsequent things like mouse-overs are not processed correctly.
---
---So, for best results, draw a menu after a mouse-up. This if in fact what most programs do, if you look carefully. In general, programs don't respond when you click down on the mouse, the action response happens when you let go (the mouse-up event).
---@param windowName string The name of an existing miniwindow.
---@param left integer X position inside the miniwindow.
---@param top integer Y position inside the miniwindow.
---@param items string A string containing the menu items to be displayed, separated by the "|" character. Leading and trailing spaces are removed from each item.
---
---For example:
---
---```text
---`"^Sword | - | Wield | Equip | - | Take | Drop "`
---```
---
---There must be at least one item, or an empty string is returned.
---
---An item which is empty, or consisting of the character `-` is displayed as a menu separator (horizontal line).
---
---An item which commences with the `^` character is displayed disabled (grayed out, and it cannot be selected). The `^` character is removed.
---
---An item which commences with the `+` character is displayed checked (with a tick mark next to it). The `+` character is removed.
---
---Items may have both a `+` and `^` in front of them, in either order.
---
---An item which commences with the `>` character represents a nested menu. Subsequent items until a `<` character are in the nested menu.
---
---An item which commences with the `<` character ends a nested menu.
---
---In the above example, a menu would be shown like this:
---
---```text
---Sword (grayed out)
--------
---Wield
---Equip
--------
---Take
---Drop
---```
---
---Thus the return value would be one of:
---
---* `""` - menu dismissed
---* `"Wield"`, `"Equip"`, `"Take"`, or `"Drop"`
---
---Note that the leading and trailing spaces around the items have been removed in the return value.
---
---Example of nested menus:
---
---```text
---"Take | Drop | >Eat | With Fork | With Hands | < | >Emote | Smile | Laugh "
---```
---
---This would display:
---
---```text
---Take
---Drop
---Eat ->
---..With Fork
---..With Hands
---Emote ->
---..Smile
---..Laugh
---```
---
---(Where Eat and Emote are nested menus)
---
---
---If the first character of the entire *items* string is the `!` character, then the function returns the selected item number rather than the selected string. It still returns an empty string if nothing is selected. The `!` is then removed and the rest of the string treated in the usual way (so you could then start a submenu, for example).
---
---This lets you distinguish between multiple identical items in submenus.
---
---For example:
---
---```text
---"!Take |-| ^Drop | >Eat | Fork | Hands | < | >Emote | Smile | Laugh "
---```
---
---In this case we would get:
---
---```text
---Take: 1
---Divider line: (can't be selected)
---Drop: (can't be selected)
---Eat -> Fork: 2
---Eat -> Hands: 3
---Emote -> Smile: 4
---Emote -> Laugh: 5
---
---Cancelled: empty string
---```
---
---If the first character of the entire Items string is the `~` character, then the menu alignment is specified by the next two characters. The `~xx` characters are then removed and the rest of the string treated in the usual way (so you could then start a submenu, for example).
---
---The two characters following the `~` are the horizontal and vertical alignments, as follows:\
---Horizontal: `L`, `C` or `R` (left, centre, right)\
---Vertical: `T`, `C` or `B` (top, centre, bottom)
---
---Default is top left (so the menu aligns to the specified point and appears on the right and underneath). That is, top left is where the menu starts. The letters are not case-sensitive. Anything other than those letters will be ignored.
---
---For example:
---
---```text
---"~RBTake | Drop" -- align cursor on right and bottom of point.
---```
---
---If you want to use both `!` and `~` then use `!` first as that is checked for and then removed from the string. Then it checks for `~` next. After that the normal menu processing takes place.
---
---
---Summary of special characters in the first column:
---
---`!` : return numeric result (only in first position of entire string)\
---`~` : specify menu alignment (only in first position of entire string, or after `!`)\
---`-` : separator line\
---`^` : grayed-out item\
---`+` : checked (ticked) item\
---`>` : start of nested menu (rest of line is nested menu title)\
---`<` : end of nested menu (rest of line is ignored)
---
---Nested menus can be further nested.
---@return string selection
---If there was an error condition as described above, the empty string.
---
---If the menu was dismissed, or no item selected, the empty string.
---
---Otherwise, the text of the selected item, with leading and trailing spaces removed.
---
---However if the first character of the Items string is the `!` character then, if the menu was not cancelled, a number is returned, starting at 1, being the item number of the selected item. Only active items (ones which can be selected) are counted.
function WindowMenu(windowName, left, top, items) end


---This merges the previously-loaded image identified by *imageID* onto the miniwindow identified by *windowName*, based on the opacity of the mask identified by *maskID*. The mask must be large enough to cover the requested area in the blended image. This function uses the entire image as the source rectangle.
---
---One method of setting up a mask would be to use the [`WindowGetImageAlpha`](lua://WindowGetImageAlpha) function to obtain the alpha information from an image. Alternatively you might have the alpha information in a separate image file.
---
---Each colour (red/green/blue) in the mask is individually considered when merging each pixel. If the mask is white (ie. 255) for that colour, then the blended image is used. If the mask is black (ie. 0) then the base image is preserved. In between, the blended image is used to the extent of the value (for example, 128 would be a 50% blend).
---
---You can use a mask with a soft border (eg. by blurring it) to make a soft transition between the loaded image and the existing contents of the miniwindow.
---
---`left`, `top`, `right`, `bottom` — describes the destination rectangle. The blend image is merged with the contents of this rectangle to produce the blended result.
---
---When blending, whichever rectangle is smaller is the one used for the width and height of the operation (the destination rectangle or the source rectangle).
---@param windowName string The name of an existing miniwindow.
---@param imageID string ID of an image loaded into the miniwindow with [`WindowLoadImage`](lua://WindowLoadImage), [`WindowLoadImageMemory`](lua://WindowLoadImageMemory), or [`WindowImageFromWindow`](lua://WindowImageFromWindow).
---@param maskID string ID of an image to use as the mask, which was loaded into the miniwindow with [`WindowLoadImage`](lua://WindowLoadImage), [`WindowLoadImageMemory`](lua://WindowLoadImageMemory), or [`WindowImageFromWindow`](lua://WindowImageFromWindow).
---@param left integer
---@param top integer
---@param right integer
---@param bottom integer
---@param mode miniwin.blend See [`miniwin.blend`](lua://miniwin.blend).
---@param opacity number The amount of opacity, between 0 and 1 (floating-point number). An opacity of 0 means you will not see the merged image. An opacity of 1 means the merge is fully applied. An opacity of 0.5 means the masked, merged image is merged 50% with the original contents of the miniwindow.
---@return error_code code #
---`error_code.eBadParameter`: Opacity not in range 0 to 1, or mask not large enough.\
---`error_code.eImageNotInstalled`: Either the blend image or the mask image is not installed.\
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.eUnknownOption`: Mode not in list above.\
---`error_code.eOK`: Success.
function WindowMergeImageAlpha(windowName, imageID, maskID, left, top, right, bottom, mode, opacity, srcLeft, srcTop, srcRight, srcBottom) end

---This merges the previously-loaded image identified by *imageID* onto the miniwindow identified by *windowName*, based on the opacity of the mask identified by *maskID*. The mask must be large enough to cover the requested area in the blended image.
---
---One method of setting up a mask would be to use the [`WindowGetImageAlpha`](lua://WindowGetImageAlpha) function to obtain the alpha information from an image. Alternatively you might have the alpha information in a separate image file.
---
---Each colour (red/green/blue) in the mask is individually considered when merging each pixel. If the mask is white (ie. 255) for that colour, then the blended image is used. If the mask is black (ie. 0) then the base image is preserved. In between, the blended image is used to the extent of the value (for example, 128 would be a 50% blend).
---
---You can use a mask with a soft border (eg. by blurring it) to make a soft transition between the loaded image and the existing contents of the miniwindow.
---
---`left`, `top`, `right`, `bottom` — describes the destination rectangle. The blend image is merged with the contents of this rectangle to produce the blended result.
---
---`srcLeft`, `srcTop`, `srcRight`, `srcBottom` — the source rectangle in the original image (use 0,0,0,0 to get the whole image, or use the overload that omits all four parameters). Negative numbers for the *srcRight* and *srcBottom* parameters represent an offset from the bottom or right edge.
---
---When blending, whichever rectangle is smaller is the one used for the width and height of the operation (the destination rectangle or the source rectangle).
---@param windowName string The name of an existing miniwindow.
---@param imageID string ID of an image loaded into the miniwindow with [`WindowLoadImage`](lua://WindowLoadImage), [`WindowLoadImageMemory`](lua://WindowLoadImageMemory), or [`WindowImageFromWindow`](lua://WindowImageFromWindow).
---@param maskID string ID of an image to use as the mask, which was loaded into the miniwindow with [`WindowLoadImage`](lua://WindowLoadImage), [`WindowLoadImageMemory`](lua://WindowLoadImageMemory), or [`WindowImageFromWindow`](lua://WindowImageFromWindow).
---@param left integer
---@param top integer
---@param right integer
---@param bottom integer
---@param mode miniwin.blend See [`miniwin.blend`](lua://miniwin.blend).
---@param opacity number The amount of opacity, between 0 and 1 (floating-point number). An opacity of 0 means you will not see the merged image. An opacity of 1 means the merge is fully applied. An opacity of 0.5 means the masked, merged image is merged 50% with the original contents of the miniwindow.
---@param srcLeft integer
---@param srcTop integer
---@param srcRight integer
---@param srcBottom integer
---@return error_code code #
---`error_code.eBadParameter`: Opacity not in range 0 to 1, or mask not large enough.\
---`error_code.eImageNotInstalled`: Either the blend image or the mask image is not installed.\
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.eUnknownOption`: Mode not in list above.\
---`error_code.eOK`: Success.
function WindowMergeImageAlpha(windowName, imageID, maskID, left, top, right, bottom, mode, opacity, srcLeft, srcTop, srcRight, srcBottom) end


---This draws a polygon (series of straight lines) connecting the specified points, with the designated pen and filled with the designated brush.
---@param windowName string The name of an existing miniwindow.
---@param points string|[number,number][] Either an array of points or a string consisting of pairs of numbers (integers or floating-point), one for each point, in the format x1,y1,x2,y2 ...
---
---For example: `"20,50,180,50"` would specify two points, one at 20,50 and one at 180,50.
---
---You must have at least 2 points (the start and end), which is 4 numbers. The number of numbers in the string must be even (divisible by two), as it takes a pair of numbers to specify one point.
---@param penColour integer|string Integer BBGGRR colour code, string hex code, or string colour name for the pen.
---@param penStyle WindowPenStyle See [`WindowPenStyle`](lua://WindowPenStyle).
---@param penWidth number Width of the pen in pixels.
---@param brushColour integer|string Integer BBGGRR colour code, string hex code, or string colour name for the brush.
---@param brushStyle? miniwin.brush See [`miniwin.brush`](lua://miniwin.brush). Default: 0 (solid).
---@param close? boolean If `true`, the polygon is closed by connecting the last point to the first one, if necessary. Default: `false`.
---@param winding? boolean If `true` the polygon is filled using the "winding" algorithm, if false it is filled using the "alternate" algorithm. Default: `false`.
---
---When the polygon-filling mode is "alternate", the system fills the area between odd-numbered and even-numbered polygon sides on each scan line. That is, the system fills the area between the first and second side, between the third and fourth side, and so on. This mode is the default.
---
---When the polygon-filling mode is "winding", the system uses the direction in which a figure was drawn to determine whether to fill an area. Each line segment in a polygon is drawn in either a clockwise or a counterclockwise direction. Whenever an imaginary line drawn from an enclosed area to the outside of a figure passes through a clockwise line segment, a count is incremented. When the line passes through a counterclockwise line segment, the count is decremented. The area is filled if the count is nonzero when the line reaches the outside of the figure.
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.ePenStyleNotValid`: Invalid pen style.\
---`error_code.eBrushStyleNotValid`: Invalid brush style.\
---`error_code.eInvalidNumberOfPoints`: Must have at least 2 points (4 numbers) and the number of numbers must be divisible by 2.\
---`error_code.eOK`: Completed OK.
function WindowPolygon(windowName, points, penColour, penStyle, penWidth, brushColour, brushStyle, close, winding) end


---This lets you move a miniwindow. This is only really useful if you are setting the "Absolute location" flag when creating it. In this case, if you notice the output window has been resized, you may want to resposition the window, without having to re-create it, and redraw all its contents.
---@param windowName string The name of an existing miniwindow.
---@param left integer Pixel offset from the left of the output window.
---@param top integer Pixel offset from the top of the output window.
---@param position miniwin.pos See [`miniwin.pos`](lua://miniwin.pos).
---@param flags integer OR combination of the following flags:
---`miniwin.create_underneath` (1): Draw underneath. If set, the miniwindow is drawn beneath the scrolling text in the output window.\
---`miniwin.create_absolute_location` (2): Absolute location. If set, the miniwindow is not subject to auto positioning (so the Position argument is ignored), and it is located exactly at the Left, Top position designated in the function call. By setting this bit you have absolute control over where the window will appear.\
---`miniwin.create_transparent` (4): Transparent. If set, whenever a pixel in the contents of the window matches the BackgroundColour, it is not drawn, and the text underneath shows through. This lets you make odd-shape windows like stars or circles, by filling the outside (the part you don't want to see) with the background colour.\
---`miniwin.create_ignore_mouse` (8): Ignore mouse. If set, this miniwindow is not considered for mouse-over, mouse-down, mouse-up events.\
---`miniwin.create_keep_hotspots` (16): Keep existing hotspots. If set, hotspots are not deleted if you are recreating an existing miniwindow (with [`WindowCreate`](lua://WindowCreate)).
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.eOK`: Success.
---
---@see WindowCreate
---@see WindowResize
---@see WindowSetZOrder
---@see WindowShow
function WindowPosition(windowName, left, top, position, flags) end


---This draws a rectangle in various styles, controlled by the *action* parameter.
---
---`left`, `top`, `right`, `bottom` — describes the rectangle to be drawn.
---@param windowName string The name of an existing miniwindow.
---@param action
---| 1 # (miniwin.rect_frame) Frame by a single pixel wide line
---| 2 # (miniwin.rect_fill) Fill the entire rectangle
---@param left number
---@param top number
---@param right number
---@param bottom number
---@param colour integer|string Integer BBGGRR colour code, string hex code, or string colour name for the rectangle.
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.eUnknownOption`: Action not in list above.\
---`error_code.eBadParameter`: Edge parameter invalid.\
---`error_code.eOK`: Completed OK.
function WindowRectOp(windowName, action, left, top, right, bottom, colour) end

---This draws a rectangle in various styles, controlled by the *action* parameter.
---
---`left`, `top`, `right`, `bottom` — describes the rectangle to be drawn.
---@param windowName string The name of an existing miniwindow.
---@param action
---| 3 # (miniwin.rect_invert) InvertRect - the colour on the miniwindow inside that rectangle is inverted
---@param left number
---@param top number
---@param right number
---@param bottom number
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.eUnknownOption`: Action not in list above.\
---`error_code.eBadParameter`: Edge parameter invalid.\
---`error_code.eOK`: Completed OK.
function WindowRectOp(windowName, action, left, top, right, bottom) end

---This draws a rectangle in various styles, controlled by the *action* parameter.
---
---`left`, `top`, `right`, `bottom` — describes the rectangle to be drawn.
---@param windowName string The name of an existing miniwindow.
---@param action
---| 4 # (miniwin.rect_3d_rect) Draw a "3D-style" rectangle in two colors, a single pixel wide
---@param left number
---@param top number
---@param right number
---@param bottom number
---@param colour1 integer|string Integer BBGGRR colour code, string hex code, or string colour name for the top and left edge.
---@param colour2 integer|string Integer BBGGRR colour code, string hex code, or string colour name for the bottom and right edge.
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.eUnknownOption`: Action not in list above.\
---`error_code.eBadParameter`: Edge parameter invalid.\
---`error_code.eOK`: Completed OK.
function WindowRectOp(windowName, action, left, top, right, bottom, colour1, colour2) end

---This draws a rectangle in various styles, controlled by the *action* parameter.
---
---`left`, `top`, `right`, `bottom` — describes the rectangle to be drawn.
---@param windowName string The name of an existing miniwindow.
---@param action
---| 5 # (miniwin.rect_draw_edge) Draw Edge - draws a 3d-style edge with optional fill
---@param left number
---@param top number
---@param right number
---@param bottom number
---@param frame
---| 5 # (miniwin.rect_edge_raised) Raised
---| 6 # (miniwin.rect_edge_etched) Etched
---| 9 # (miniwin.rect_edge_bump) Bump
---| 10 # (miniwin.rect_edge_sunken) Sunken
---@param flags integer OR combination of the following flags:
---`miniwin.rect_edge_at_top_left` (3): Top left.\
---`miniwin.rect_edge_at_top_right` (6): Top right.\
---`miniwin.rect_edge_at_bottom_left` (9): Bottom left.\
---`miniwin.rect_edge_at_bottom_right` (12): Bottom right.\
---`miniwin.rect_edge_at_all` (15): Full rect.
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.eUnknownOption`: Action not in list above.\
---`error_code.eBadParameter`: Edge parameter invalid.\
---`error_code.eOK`: Completed OK.
function WindowRectOp(windowName, action, left, top, right, bottom, frame, flags) end


---Resizes a miniwindow.
---
---If the new size is larger than the existing size the BackgroundColour is used to fill any newly exposed area.
---
---Any existing contents of the miniwindow are copied to the top-left corner of the new window.
---
---Any existing hotspots, images, and fonts are preserved. All other window parameters for this miniwindow are unchanged.
---@param windowName string The name of an existing miniwindow.
---@param width integer Pixel width of the window.
---@param height integer Pixel height of the window.
---@param background integer|string Integer BBGGRR colour code, string hex code, or string colour name for the background fill.
---@return error_code code #
---`error_code.eNoNameSpecified`: Miniwindow name must be specified.\
---`error_code.eBadParameter`: Width or height less than zero.\
---`error_code.eOK`: Sucess.
---
---@see WindowCreate
---@see WindowPosition
---@see WindowSetZOrder
---@see WindowShow
function WindowResize(windowName, width, height, background) end


---Sets a single pixel in the miniwindow.
---
---Warning - although setting individual pixels is reasonably fast, if you need to set a lot (for example, to draw a line or a box), it would be much faster to use the appropriate dedicated function (such as [`WindowLine`](lua://WindowLine)). This is because to draw boxes and lines, especially large ones, or filled ones, would take many, many calls to `WindowSetPixel` to achieve the same result.
---@param windowName string The name of an existing miniwindow.
---@param x integer Horizontal coordinate of the pixel.
---@param y integer Vertical coordinate of the pixel.
---@param colour integer|string Integer BBGGRR colour code, string hex code, or string colour name for the pixel.
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.eOK`: Success.
function WindowSetPixel(windowName, x, y, colour) end


---Sets a Z-Order (drawing order) for the miniwindow.
---@param windowName string The name of an existing miniwindow.
---@param zOrder integer The order to draw the window. Lower is drawn sooner. Windows with the same Z-Order are drawn in window name order. So for example, the default case of a zero Z-Order results in windows being drawn in name order.
---
---More specifically:
---
---* Windows with a negative Z-order are drawn first (in order, so -2 would be drawn before -1).
---* Windows with a zero Z-order (the default) are drawn next, in name order (lower name first).
---* Windows with a positive Z-order are drawn last, in Z-order sequence.
---* If there are two (or more) windows with the same Z-order number, they are drawn in name order.
---
---Note that being "drawn first" means that a window appears underneath a window which is drawn later.
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.eOK`: Success.
---
---@see WindowCreate
---@see WindowPosition
---@see WindowResize
---@see WindowShow
function WindowSetZOrder(windowName, zOrder) end


---This shows or hides the designated window.
---
---Creating and showing windows are separate operations. This is because you may want to maintain something like an inventory list, or quest objectives, but not necessarily have space on the screen for them at a particular moment. By creating the window early on, you can draw to it (offscreen), add or remove items, and generally keep it up-to-date as events occur on the game. However when the player actually wants to see the window s/he may hit a hotkey or type an alias, which simply causes the window to be shown (or hidden).
---@param windowName string The name of an existing miniwindow.
---@param show? boolean `true` to display the window, `false` to hide it. Default: `true`.
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.eOK`: Success.
---
---@see WindowCreate
---@see WindowDelete
---@see WindowPosition
---@see WindowResize
---@see WindowSetZOrder
---@see WindowShow
function WindowShow(windowName, show) end


---This draws some text in miniwindow.
---
---This function does not do anything special for newlines, carriage-returns, tabs etc. Probably a box or graphical character will be drawn if you attempt to use them. To handle changing lines you will need to do a second `WindowText` with the Y (*top*) parameter increased. To handle tabbing you would need to increase the X (*left*) parameter.
---
---`left`, `top`, `right`, `bottom` — describes the rectangle to be drawn to.
---@param windowName string The name of an existing miniwindow.
---@param fontID string ID of a font loaded into the miniwindow with [`WindowFont`](lua://WindowFont).
---@param text string The text to be drawn.
---@param left number
---@param top number
---@param right number
---@param bottom number
---@param colour integer|string Integer BBGGRR colour code, string hex code, or string colour name for the text.
---@param unicode? boolean If `true`, the text is Unicode text in UTF-8 format. Default: `false`.
---@return number width If successful, returns the pixel width the text took horizontally. Thus, by adding that to the *left* parameter, you can draw some more text to the right of what was just drawn.
---
---If unsuccessful, returns a negative number as follows:\
----1: That window name does not exist\
----2: That font was not loaded\
----3: For Unicode, invalid UTF-8 sequence
---
---@see WindowTextWidth - calculate width without drawing text.
function WindowText(windowName, fontID, text, left, top, right, bottom, colour, unicode) end


---This calculates how many pixels a particular piece of text will take up, which can help in calculating how big a window to create, or where to put other things. Note that [`WindowText`](lua://WindowText) returns the number of pixels when it draws text, so you don't need to call `WindowTextWidth` simply to work out where to draw the very next item.
---@param windowName string The name of an existing miniwindow.
---@param fontID string ID of a font loaded into the miniwindow with [`WindowFont`](lua://WindowFont).
---@param text string The text to be drawn.
---@param unicode? boolean If `true`, the text is Unicode text in UTF-8 format. Default: `false`.
---@return number width If successful, the pixel width the text would take horizontally.
---
---If unsuccessful, returns a negative number as follows:\
----1: That window name does not exist\
----2: That font was not loaded\
----3: For Unicode, invalid UTF-8 sequence
---
---@see WindowText - draw text.
function WindowTextWidth(windowName, fontID, text, unicode) end


---This copies an image to the miniwindow. You specify effectively a "matrix" which is applied to each pixel position, so that the image can be rotated, scaled, reflected, sheared and translated.
---
---The position of each destination pixel (x' and y') is given by:
---
---x' = (x * mxx) + (y * mxx) + left\
---y' = (x * myx) + (y * myy) + top
---
---Note that if you draw a monochrome image, such as one set up by [`WindowCreateImage`](lua://WindowCreateImage) then the pen colour from the most recent drawing operation is used as the foreground colour, and the brush colour for the background colour. Thus you may want to draw a small rectangle (eg. 1 x 1 pixel) with [`WindowCircleOp`](lua://WindowCircleOp) to establish those colours first.
---@param windowName string The name of an existing miniwindow.
---@param imageID string ID of an image loaded into the miniwindow with [`WindowLoadImage`](lua://WindowLoadImage), [`WindowLoadImageMemory`](lua://WindowLoadImageMemory), or [`WindowImageFromWindow`](lua://WindowImageFromWindow).
---@param left number Pixel offset from the left edge of the destination miniwindow.
---@param top number Pixel offset from the top edge of the destination miniwindow.
---@param mode
---| 1 # (miniwin.image_copy) Copy non-transparently to the destination position.
---| 3 # (miniwin.image_transparent_copy) Do a transparent copy, where the pixel at the left,top corner (pixel position 0,0) is considered the transparent colour. Any pixels that exactly match that colour are not copied. WARNING - do not choose black or white as the transparent colour as that throws out the calculations. Choose some other colour (eg. purple) - you won't see that colour anyway.
---@param mxx number
---@param mxy number
---@param myx number
---@param myy number
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.eImageNotInstalled`: That image was not loaded.\
---`error_code.eBadParameter`: Drawing mode not in list above or not available.\
---`error_code.eOK`: Completed OK.
function WindowTransformImage(windowName, imageID, left, top, mode, mxx, mxy, myx, myy) end


---The entire contents of the named miniwindow are written to disk as file name *fileName*. The output format is determined by the filename extension.
---@param windowName string The name of an existing miniwindow.
---@param fileName string Output file name.
---@return error_code code #
---`error_code.eNoSuchWindow`: No such miniwindow.\
---`error_code.eNoNameSpecified`: No file name specified.\
---`error_code.eBadParameter`: File name too short or has an unrecognized extension.\
---`error_code.eCouldNotOpenFile`: Cannot open file (eg. pathname does not exist, no write permissions).\
---`error_code.eLogFileBadWrite`: Problem writing (eg. disk full).\
---`error_code.eOK`: Success.
function WindowWrite(windowName, fileName) end
