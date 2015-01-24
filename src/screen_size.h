#ifndef SCREEN_SIZE
#define SCREEN_SIZE

#define DEFAULT_DISP_WIDTH 320
/* ViewPort 1 */
#define WIDTH1   384
#define DISPL_WIDTH1   DEFAULT_DISP_WIDTH
#define HEIGHT1  80
#define DEPTH1     5
#define COLOURS1  (2 << DEPTH1)

/* ViewPort 2 */
#define ANIM_STRIPE 10
#define WIDTH2   320
#define DISPL_WIDTH2   DEFAULT_DISP_WIDTH
#define DISPL_HEIGHT2   150
#define HEIGHT2 (DISPL_HEIGHT2 * ANIM_STRIPE)                        
#define DEPTH2     3
#define COLOURS2   (2 << DEPTH2)

/* ViewPort 2b */
#define ANIM_STRIPEb 8
#define WIDTH2b   384
#define DISPL_WIDTH2b   DEFAULT_DISP_WIDTH
#define DISPL_HEIGHT2b   150
#define HEIGHT2b (DISPL_HEIGHT2b * ANIM_STRIPEb)                        
#define DEPTH2b     3
#define COLOURS2b   (2 << DEPTH2b)

/* ViewPort 3 */
#define WIDTH3   DEFAULT_DISP_WIDTH
#define HEIGHT3  12
#define DEPTH3     2
#define COLOURS3  (2 << DEPTH3)

#endif // #ifndef SCREEN_SIZE