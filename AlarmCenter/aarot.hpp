#ifndef AAROT_HPP
#define AAROT_HPP

#include <windows.h>
#include <iostream>
#include <vector>
#include <math.h>

namespace gui {
namespace control {


#define aar_abs(a) (((a) < 0)?(-(a)):(a))

typedef bool(* aar_callback) (double);

struct aar_pnt
{
	double x,y;
	inline aar_pnt(){}
    inline aar_pnt(double x,double y):x(x),y(y){}
};

struct aar_dblrgbquad
{
	double red, green, blue, alpha;
};

struct aar_indll
{
    aar_indll * next;
    int ind;
};

class aarot
{
public:
    static HBITMAP rotate(HBITMAP, double, aar_callback, int, bool, int cntPt = 0, CPoint *pts = nullptr);
    static HBITMAP rotate(HBITMAP, double, aar_callback);
    static HBITMAP rotate(HBITMAP, double, aar_callback, int);
    static HBITMAP rotate(HBITMAP, double, aar_callback, bool);
private:
    static double coss;
    static double sins;
    static aar_pnt * polyoverlap;
    static int polyoverlapsize;
    static aar_pnt * polysorted;
    static int polysortedsize;
    static aar_pnt * corners;
    //562

    static inline int roundup(double a) {if (aar_abs(a - (int)(a + 5e-10)) < 1e-9) return (int)(a + 5e-10); else return (int)(a + 1);}
    static inline int round(double a) {return (int)(a + 0.5);}
    static inline BYTE byterange(double a) {int b = round(a); if (b <= 0) return 0; else if (b >= 255) return 255; else return (BYTE)b;}
    static inline double aar_min(double & a, double & b) {if (a < b) return a; else return b;}
    static inline double aar_max(double & a, double & b) {if (a > b) return a; else return b;}

    static inline double aar_cos(double);
    static inline double aar_sin(double);

    static inline double area();
    static inline void sortpoints();
    static inline bool isinsquare(aar_pnt, aar_pnt &);
    static inline double pixoverlap(aar_pnt *, aar_pnt);

    static HBITMAP dorotate(HBITMAP, double, aar_callback, int, bool, int cntPt = 0, CPoint *pts = nullptr);
};

NAMESPACE_END
NAMESPACE_END

#endif
