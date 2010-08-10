(:~
 : This module provides functions to handle basic painting operations on images.
 : For most functions there are different versions as it is possible to either paint with the standard color specified for the image (change using set-color etc.) or by explicitly passing a color argument to the functions.
 : Color types are defined in the schema at 'http://www.zorba-xquery.com/modules/image/image'.
 : 
 : @author Daniel Thomas
 : @version 0.1
 : @see http://www.zorba-xquery.com/modules/image/image
 : @see http://www.imagemagick.org/Magick++/
 :)
module namespace paint = 'http://www.zorba-xquery.com/modules/image/paint';
import module namespace error = 'http://www.zorba-xquery.com/modules/image/error';

(:~
 : Contains the definitions of the possible image types.
 :)
import schema namespace image = 'http://www.zorba-xquery.com/modules/image/image';


 (:~
  : Draws a line on the passed image from $start-x, start-y to $end-x, $end-y.
  : If no color is passed, the default color for this image will be taken. However, a passed color does not change the default color for this image.
  : Coordinates that in any way are not inside the image will generate a line to the hypothetical point beyond the border of the image that discontinues at the border.
  : 
  : 
  : @param $image is the image to paint a line to.
  : @param $start-x is the horizontal starting point of the line.
  : @param $start-y is the vertical starting point of the line.
  : @param $end-x is the horizontal end point of the line.
  : @param $end-y is the vertical end point of the line.
  : @param $stroke-color is a color as hex triplet of 3 bytes indicating the rgb values with leading '#' as xs:string (default is '#0000000', black)
  : @param $stroke-width is the width of the line that should be painted (default is 1).
  : @param $anti-aliasing defines if anti-aliasing should be used (default is false).
  : @return A new image with the line painted as specified.
  : @error If the passed xs:base64Binary is not a valid image type.
  :)
declare function paint:draw-line($image as xs:base64Binary, $start-x as xs:double, $start-y as xs:double, $end-x as xs:double, $end-y as xs:double, $stroke-color as image:colorType?, $stroke-width as xs:double?, $anti-aliasing as xs:boolean?) as xs:base64Binary {
    paint:draw-poly-line($image, ($start-x, $end-x), ($start-y, $end-y), $stroke-color, (), $stroke-width, $anti-aliasing)
};




(:~
 : Draws a poly-line (different adjascent line segments) to the passed image with a specified color.
 : Make sure that both $x-values and $y-values have the same count, only coordinates that have an x and a y value will be included in the poly-line. 
 : A line segment is painted always from $x-value[n], $y-value[n] to $x-value[n+1], $y-value[n+1], starting at $x-value[0], $y-value[0].
 : Passing only one x-value or y-value can lead to strange and rather unpredictable lines, whereas passing two x-values an y-values will just paint a simple line with the specified coordinates.
 :
 : @param $image is the image to paint a poly-line to.
 : @param $x-values are the horizontal values of the different line segment starting/ending points. 
 : @param $y-values are the vertical values of the different line segment starting/ending points.
 : @param $stroke-color is a color as hex triplet of 3 bytes indicating the rgb values with leading '#' as xs:string (default is '#0000000', black)
 : @param $fill-color is the color with which to fill the poly-line. If no value is passed then the poly-line is not filled.
 : @param $stroke-width is the width of the line that should be painted (default is 1, equal or smaller than 1 will result in the thinnest possible line drawn).
 : @param $anti-aliasing defines if anti-aliasing should be used (default is false).
 : @return A new image with the specified poly-line painted.
 : @error If the passed xs:base64Binary is not a valid image type.
 :)
declare function paint:draw-poly-line($image as xs:base64Binary, $x-values as xs:double+, $y-values as xs:double+, $stroke-color as image:colorType?, $fill-color as image:colorType?, $stroke-width as xs:double?, $anti-aliasing as xs:boolean?)  as xs:base64Binary external;



  
  
(:~
 : Draws a stroked poly-line (different adjascent line segments which are painted as short strokes of a specified width) with the specified color.
 : Make sure that both $x-values and $y-values have the same count. 
 : A line segment is painted always from $x-value[n], $y-value[n] to $x-value[n+1], $y-value[n+1], starting at $x-value[0], $y-value[0].
 : If no color is passed, the default color for this image will be taken.
 :
 : @param $image is the image to paint a stroked poly-line to.
 : @param $x-values are the horizontal values of the different line segment starting/ending points. 
 : @param $y-values are the vertical values of the different line segment starting/ending points.
 : @param $stroke-length is the length of the strokes with which the line segments are painted.
 : @param $gap-length is the length of the gaps between the strokes.
 : @param $stroke-color is a color as hex triplet of 3 bytes indicating the rgb values with leading '#' as xs:string (default is '#0000000', black)
 : @param $stroke-width is the width of the line that should be painted (default is 1, equal or smaller than 1 will result in the thinnest possible line drawn).
 : @param $anti-aliasing defines if anti-aliasing should be used (default is false).
 : @return A new image with the specified stroked poly-kube painted.
 : @error If the passed xs:base64Binary is not a valid image type.
 :)
declare function paint:draw-stroked-poly-line($image as xs:base64Binary, $x-values as xs:double+, $y-values as xs:double+, $stroke-length as xs:double, $gap-length as xs:double, $stroke-color as image:colorType?, $stroke-width as xs:double?, $anti-aliasing as xs:boolean?) as xs:base64Binary external; 




(:~
 : Draws a rectangle to the passed image.
 : If no color is passed, the default color for this image will be taken.
 :
 : @param $image is the image to paint a rectangle to.
 : @param $upper-left-x is the horizontal value of the upper left corner of the rectangle.
 : @param $upper-left-y is the vertical value of the upper left corner of the rectangle.
 : @param $lower-right-x is the horizontal value of the lower right corner of the rectangle.
 : @param $lower-right-y is the vertical value of the lower right corner of the rectangle.
 : @param $stroke-color is a color as hex triplet of 3 bytes indicating the rgb values with leading '#' as xs:string (default is '#0000000', black)
 : @param $fill-color is the color with which to fill the rectangle, if no color is passed then the rectangle isn't filled.
 : @param $stroke-width is the width of the outer lines of the rectangle that should be painted (default is 1, equal or smaller than 1 will result in the thinnest possible line drawn).
 : @param $anti-aliasing defines if anti-aliasing should be used (default is false).
 : @return A new image with the specified rectangle painted.
 : @error If the passed xs:base64Binary is not a valid image type.
 :)
declare function paint:draw-rectangle($image as xs:base64Binary, $upper-left-x as xs:double, $upper-left-y as xs:double, $lower-right-x as xs:double, $lower-right-y as xs:double, $stroke-color as image:colorType?, $fill-color as image:colorType?, $stroke-width as xs:double?, $anti-aliasing as xs:boolean?) as xs:base64Binary external;


 
 
 (:~
 : Draws a rounded rectangle to the passed image.
 :
 : @param $image is the image to paint a rectangle to.
 : @param $upper-left-x is the horizontal value of the upper left corner of the rectangle.
 : @param $upper-left-y is the vertical value of the upper left corner of the rectangle.
 : @param $lower-right-x is the horizontal value of the lower right corner of the rectangle.
 : @param $lower-right-y is the vertical value of the lower right corner of the rectangle.
 : @param $corner-with is the width a corner should have for the rounded corners.
 : @param $corner-height is the height a corner should have for the rounded corners. 
 : @param $stroke-color is a color as hex triplet of 3 bytes indicating the rgb values with leading '#' as xs:string (default is '#0000000', black)
 : @param $fill-color is the color with which to fill the rectangle, if no color is passed then the rectangle isn't filled.
 : @param $stroke-width is the width of the outer lines of the rectangle that should be painted (default is 1, equal or smaller than 1 will result in the thinnest possible line drawn).
 : @param $anti-aliasing defines if anti-aliasing should be used (default is false).
 : @return A new image with the specified rectangle painted.
 : @error If the passed xs:base64Binary is not a valid image type.
 :)
declare function paint:draw-rounded-rectangle($image as xs:base64Binary, $upper-left-x as xs:double, $upper-left-y as xs:double, $lower-right-x as xs:double, $lower-right-y as xs:double, $corner-width as xs:double, $corner-height as xs:double, $stroke-color as image:colorType?, $fill-color as image:colorType?, $stroke-width as xs:double?, $anti-aliasing as xs:boolean?) as xs:base64Binary external;
 



(:~
 : Draw a circle to the passed image.
 : 
 : @param $image is the image to paint a circle to.
 : @param $origin-x is the horizontal value of the center of this circle.
 : @param $origin-y is the vertical value of the center of this circle.
 : @param $perimeter is the perimeter of this circle.
 : @param $stroke-color is a color as hex triplet of 3 bytes indicating the rgb values with leading '#' as xs:string (default is '#0000000', black)
 : @param $fill-color is the color with which to fill the rectangle, if no color is passed then the circle isn't filled.
 : @param $stroke-width is the width of the outer lines of the circle that should be painted (default is 1, equal or smaller than 1 will result in the thinnest possible line drawn).
 : @param $anti-aliasing defines if anti-aliasing should be used (default is false). 
 : @return A new image with the specified circle painted.
 : @error If the passed xs:base64Binary is not a valid image type.
 :)
declare function paint:draw-circle($image as xs:base64Binary, $origin-x as xs:double, $origin-y as xs:double, $perimeter as xs:double,  $stroke-color as image:colorType?, $fill-color as image:colorType?, $stroke-width as xs:double?, $anti-aliasing as xs:boolean?) as xs:base64Binary {
    paint:draw-ellipse($image, $origin-x, $origin-y, $perimeter, $perimeter, $stroke-color, $fill-color, $stroke-width, $anti-aliasing)
};


 



(:~
 : Draw an ellipse to the passed image.
 : 
 : @param $image is the image to paint a ellipse to.
 : @param $origin-x is the horizontal value of the center of this ellipse.
 : @param $origin-y is the vertical value of the center of this ellipse.
 : @param $perimeter-x is the horizontal perimeter of this ellipse.
 : @param $perimeter-y is the vertical perimeter of this ellipse.
 : @param $stroke-color is a color as hex triplet of 3 bytes indicating the rgb values with leading '#' as xs:string (default is '#0000000', black)
 : @param $fill-color is the color with which to fill the rectangle, if no color is passed then the ellipse isn't filled.
 : @param $stroke-width is the width of the outer lines of the ellipse that should be painted (default is 1, equal or smaller than 1 will result in the thinnest possible line drawn).
 : @param $anti-aliasing defines if anti-aliasing should be used (default is false). 
 : @return A new image with the specified circle painted.
 : @error If the passed xs:base64Binary is not a valid image type.
 :)
declare function paint:draw-ellipse($image as xs:base64Binary, $origin-x as xs:double, $origin-y as xs:double, $perimeter-x as xs:double, $perimeter-y as xs:double,  $stroke-color as image:colorType?, $fill-color as image:colorType?, $stroke-width as xs:double?, $anti-aliasing as xs:boolean?) as xs:base64Binary {
    paint:draw-arc($image, $origin-x, $origin-y, $perimeter-x, $perimeter-y, 0, 360, $stroke-color, $fill-color, $stroke-width, $anti-aliasing)
};

(:~
 : Draw an arc to the passed image.
 : 
 : @param $image is the image to paint a ellipse to.
 : @param $origin-x is the horizontal value of the center of this ellipse.
 : @param $origin-y is the vertical value of the center of this ellipse.
 : @param $perimeter-x is the horizontal perimeter of this ellipse.
 : @param $perimeter-y is the vertical perimeter of this ellipse.
 : @param $start-degrees is the start of the arc in degrees.
 : @param $end-degrees is the end of the arc in degrees.
 : @param $stroke-color is a color as hex triplet of 3 bytes indicating the rgb values with leading '#' as xs:string (default is '#0000000', black)
 : @param $fill-color is the color with which to fill the arc, if no color is passed then the arc isn't filled.
 : @param $stroke-width is the width of the outer lines of the arc that should be painted (default is 1, equal or smaller than 1 will result in the thinnest possible line drawn).
 : @param $anti-aliasing defines if anti-aliasing should be used (default is false). 
 : @return A new image with the specified circle painted.
 : @error If the passed xs:base64Binary is not a valid image type.
 :)
declare function paint:draw-arc($image as xs:base64Binary, $origin-x as xs:double, $origin-y as xs:double, $perimeter-x as xs:double, $perimeter-y as xs:double, $start-degrees as xs:double, $end-degrees as xs:double,  $stroke-color as image:colorType?, $fill-color as image:colorType?, $stroke-width as xs:double?, $anti-aliasing as xs:boolean?) as xs:base64Binary external;




(:~
 : Draws a polygon to the passed image. This is similar to a poly-line, however the end-points are connected with the starting points.
 : Make sure that both $x-values and $y-values have the same count, only coordinates that have an x and a y value will be included in the poly-line. 
 : A line segment is painted always from $x-value[n], $y-value[n] to $x-value[n+1], $y-value[n+1], starting at $x-value[0], $y-value[0].
 : Passing only one x-value or y-value can lead to strange and rather unpredictable lines, whereas passing two x-values an y-values will just paint a simple line with the specified coordinates.
 :
 : @param $image is the image to which a polygon should be painted.
 : @param $x-values are the horizontal values of the different line segment starting/ending points. 
 : @param $y-values are the vertical values of the different line segment starting/ending points.
 : @param $stroke-color is a color as hex triplet of 3 bytes indicating the rgb values with leading '#' as xs:string (default is '#0000000', black)
 : @param $fill-color is the color with which to fill the polygon, if no color is passed then the polygon isn't filled.
 : @param $stroke-width is the width of the outer lines of the polygon that should be painted (default is 1, equal or smaller than 1 will result in the thinnest possible line drawn).
 : @param $anti-aliasing defines if anti-aliasing should be used (default is false). 
 : @return A new image with the specified polygon painted.
 : @error If the passed xs:base64Binary is not a valid image type.
 :)
declare function paint:draw-polygon($image as xs:base64Binary, $x-values as xs:double+, $y-values as xs:double+, $stroke-color as image:colorType?, $fill-color as image:colorType?, $stroke-width as xs:double?, $anti-aliasing as xs:boolean?) as xs:base64Binary external;



(:~ 
 : Draws the specified text to the passed image.
 :
 : @param $image is the image to paint the text to.
 : @param $text it the text to paint. 
 : @param $x is the horizontal location of the text.
 : @param $y is the vertical location of the text.
 : @param $font-family is the font with which to paint the text (default is Arial, if font family is passed that is not known to the system, then Arial is used).
 : @param $font-size is the size of the text to be painted (default is 12).  
 : @param $font-weight is the weight with which to paint the text (default is 400).
 : @param $font-style is the style for the font (default is 'Normal').
 : @param $text-color is a color with which the text should be painted (default is '#000000', black).
 : @return A new image with the specified text.
 : @error If the passed xs:base64Binary is not a valid image type.
 :)
declare function paint:draw-text($image as xs:base64Binary, $text as xs:string, $x as xs:double, $y as xs:double, $font-family as xs:string, $font-size as xs:double?, $font-weight as image:fontWeightType?, $font-style as image:fontStyleType?,  $text-color as image:colorType?) as xs:base64Binary external;


