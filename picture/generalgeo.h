#pragma once

//RECT
#define WIDTHOF(R)					((R).right - (R).left)
#define HEIGHTOF(R)					((R).bottom - (R).top)

#define MIDDLEXOF(R)				(((R).left + (R).right)/2)
#define MIDDLEYOF(R)				(((R).top + (R).bottom)/2)

#define RECT(X, Y, W, H)			{X, Y, X + W, Y + H}
#define RECT_LB(X, Y, W, H)			{X, Y - H, X + W, Y}

#define ZERORECT(R)					{(R).left = 0; (R).top = 0; (R).right = 0; (R).bottom = 0;}


//POINT
#define ADDPOINT(P1, P2)			{(P1).x + (P2).x, (P1).y + (P2).y}
#define MINUSPOINT(P1, P2)			{(P1).x - (P2).x, (P1).y - (P2).y}

#define INSIDE(P, R)				((P).x >= (R).left && (P).x <= (R).right && (P).y >= (R).top && (P).y <= (R).bottom)
#define OUTSIDE(P, R)				((P).x < (R).left || (P).x > (R).right || (P).y < (R).top || (P).y > (R).bottom)

#define INSIDE2(P, W, H)			((P).x >= 0 && (P).y >= 0 && (P).x <= (W) && (P).y <= (H))
#define OUTSIDE2(P, W, H)			((P).x < 0 || (P).y < 0 || (P).x > (W) || (P).y > (H))

#define GET_X_LPARAM(lp)			((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)			((int)(short)HIWORD(lp)) 

#define ZEROPOINT(P)				{(P).x = 0; (P).y = 0;}
#define SETPOINT(P, X, Y)			{(P).x = X; (P).y = Y;}