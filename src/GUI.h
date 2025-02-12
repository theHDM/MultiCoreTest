#pragma once

// high-level code
// presumes this is included at the end of other declarations

const uint8_t hex_0_0_at_X = 108;
const uint8_t hex_0_0_at_Y = 101;

void draw_GUI(int context) {
  if (context & _show_HUD) {
    for (auto& b : hexBoard.btn) {
      if (!b.isBtn) continue;
      int atX = hex_0_0_at_X + 2 * b.coord.x 
                                - (b.coord.x <= -10 ? 1 : 0);
      int atY = hex_0_0_at_Y + 3 * b.coord.y;
      u8g2.drawPixel(atX,atY);
      if (b.pressure) {
                                u8g2.drawPixel(atX  ,atY-1);   // off low mid hi
                                u8g2.drawPixel(atX  ,atY+1);   //      *   *  ***
        if (b.pressure >  64) { u8g2.drawPixel(atX-1,atY  );   //  *   *  *** ***
                                u8g2.drawPixel(atX+1,atY  ); } //      *   *  ***
        if (b.pressure >  96) { u8g2.drawPixel(atX-1,atY-1);   //
                                u8g2.drawPixel(atX-1,atY+1);   //
                                u8g2.drawPixel(atX+1,atY-1);   //
                                u8g2.drawPixel(atX+1,atY+1); } //          
      }
    }
  }
  if (context & _show_pixel_ID) {
    u8g2.drawStr(_LEFT_MARGIN, 8, "Press anchor hex >>");
  }
  if (context & _show_custom_msg) {
    u8g2.drawStr(_LEFT_MARGIN, 18, "Success or fail");
    u8g2.drawStr(_LEFT_MARGIN, 28, "message here.");
  }
}