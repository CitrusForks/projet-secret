## Screen size constants

import gs

DEFAULT_DISP_WIDTH = 320

# ViewPort 1
# Logo area
WIDTH1 = 720//2
HEIGHT1 = 160
DISPL_WIDTH1 = DEFAULT_DISP_WIDTH
DISPL_HEIGHT1 = 79

# ViewPort 2
# Checkerboard & bobs area
ANIM_STRIPE = 10
WIDTH2 = 320
DISPL_WIDTH2 = DEFAULT_DISP_WIDTH
DISPL_HEIGHT2 = 150
HEIGHT2 = (DISPL_HEIGHT2 * ANIM_STRIPE)  

CHECKERBOARD_HEIGHT = 100                      

# ViewPort 2b
ANIM_STRIPEb = 8
WIDTH2b = 720//2
DISPL_WIDTH2b = DEFAULT_DISP_WIDTH
DISPL_HEIGHT2b = 150
HEIGHT2b = (DISPL_HEIGHT2b * ANIM_STRIPEb) 

# ViewPort 3
# Scrolltext area
WIDTH3 = 720//2
DISPL_HEIGHT3 = 13
HEIGHT3 = (DISPL_HEIGHT3 << 1)

COLOUR_PURPLE_DARK = gs.Color(0x2a / 255.0, 0x12 / 255.0, 0x21 / 255.0, 1.0)
COLOUR_PURPLE = gs.Color(0x4a / 255.0, 0x22 / 255.0, 0x51 / 255.0, 1.0)
COLOUR_PURPLE_LIGHT = gs.Color(0x5F / 255.0, 0x00 / 255.0, 0x4B / 255.0, 1.0)