/*
 *  image_math.cpp
 *  PHD Guiding
 *
 *  Created by Craig Stark.
 *  Copyright (c) 2006-2010 Craig Stark.
 *  All rights reserved.
 *
 *  This source code is distributed under the following "BSD" license
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *    Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *    Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *    Neither the name of Craig Stark, Stark Labs nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "phd.h"
#include "image_math.h"

#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/tokenzr.h>

#include <algorithm>

int dbl_sort_func (double *first, double *second)
{
    if (*first < *second)
        return -1;
    else if (*first > *second)
        return 1;
    return 0;
}

float CalcSlope(const ArrayOfDbl& y)
{
    // Does a linear regression to calculate the slope

    int x, size;
    double s_xy, s_x, s_y, s_xx, nvalid;
    double retval;
    size = (int) y.GetCount();
    nvalid = 0.0;
    s_xy = 0.0;
    s_xx = 0.0;
    s_x = 0.0;
    s_y = 0.0;

    for (x=1; x<=size; x++) {
        if (1) {
            nvalid = nvalid + 1;
            s_xy = s_xy + (double) x * y[x-1];
            s_x = s_x + (double) x;
            s_y = s_y + y[x-1];
            s_xx = s_xx + (double) x * (double) x;
        }
    }

    retval = (nvalid * s_xy - (s_x * s_y)) / (nvalid * s_xx - (s_x * s_x));
    return (float) retval;
}

bool QuickLRecon(usImage& img)
{
    // Does a simple debayer of luminance data only -- sliding 2x2 window
    usImage Limg;
    if (Limg.Init(img.Size)) {
        pFrame->Alert(_("Memory allocation error"));
        return true;
    }

    int xsize = img.Size.GetWidth();
    int ysize = img.Size.GetHeight();

    for (int y = 0; y < ysize - 1; y++)
    {
        int x;
        for (x = 0; x < xsize - 1; x++)
        {
            Limg.ImageData[x+y*xsize] = (img.ImageData[x+y*xsize] + img.ImageData[x+1+y*xsize] + img.ImageData[x+(y+1)*xsize] + img.ImageData[x+1+(y+1)*xsize]) / 4;
        }
        Limg.ImageData[x+y*xsize]=Limg.ImageData[(x-1)+y*xsize];  // Last one in this row -- just duplicate
    }
    for (int x=0; x<xsize; x++)
        Limg.ImageData[x+(ysize-1)*xsize]=Limg.ImageData[x+(ysize-2)*xsize];  // Last row -- just duplicate

    img.SwapImageData(Limg);
    return false;
}

bool Median3(usImage& img)
{
    usImage tmp;
    tmp.Init(img.Size);
    bool err = Median3(tmp.ImageData, img.ImageData, img.Size.GetWidth(), img.Size.GetHeight());
    img.SwapImageData(tmp);
    return err;
}

inline static void swap(unsigned short& a, unsigned short& b)
{
    unsigned short const t = a;
    a = b;
    b = t;
}

inline static unsigned short median9(const unsigned short l[9])
{
    unsigned short l0 = l[0], l1 = l[1], l2 = l[2], l3 = l[3], l4 = l[4];
    unsigned short x;
    x = l[5];
    if (x < l0) swap(x, l0);
    if (x < l1) swap(x, l1);
    if (x < l2) swap(x, l2);
    if (x < l3) swap(x, l3);
    if (x < l4) swap(x, l4);
    x = l[6];
    if (x < l0) swap(x, l0);
    if (x < l1) swap(x, l1);
    if (x < l2) swap(x, l2);
    if (x < l3) swap(x, l3);
    if (x < l4) swap(x, l4);
    x = l[7];
    if (x < l0) swap(x, l0);
    if (x < l1) swap(x, l1);
    if (x < l2) swap(x, l2);
    if (x < l3) swap(x, l3);
    if (x < l4) swap(x, l4);
    x = l[8];
    if (x < l0) swap(x, l0);
    if (x < l1) swap(x, l1);
    if (x < l2) swap(x, l2);
    if (x < l3) swap(x, l3);
    if (x < l4) swap(x, l4);

    if (l1 > l0) l0 = l1;
    if (l2 > l0) l0 = l2;
    if (l3 > l0) l0 = l3;
    if (l4 > l0) l0 = l4;

    return l0;
}

inline static unsigned short median8(const unsigned short l[8])
{
    unsigned short l0 = l[0], l1 = l[1], l2 = l[2], l3 = l[3], l4 = l[4];
    unsigned short x;

    x = l[5];
    if (x < l0) swap(x, l0);
    if (x < l1) swap(x, l1);
    if (x < l2) swap(x, l2);
    if (x < l3) swap(x, l3);
    if (x < l4) swap(x, l4);
    x = l[6];
    if (x < l0) swap(x, l0);
    if (x < l1) swap(x, l1);
    if (x < l2) swap(x, l2);
    if (x < l3) swap(x, l3);
    if (x < l4) swap(x, l4);
    x = l[7];
    if (x < l0) swap(x, l0);
    if (x < l1) swap(x, l1);
    if (x < l2) swap(x, l2);
    if (x < l3) swap(x, l3);
    if (x < l4) swap(x, l4);

    if (l2 > l0) swap(l2, l0);
    if (l2 > l1) swap(l2, l1);

    if (l3 > l0) swap(l3, l0);
    if (l3 > l1) swap(l3, l1);

    if (l4 > l0) swap(l4, l0);
    if (l4 > l1) swap(l4, l1);

    return (unsigned short)(((unsigned int) l0 + (unsigned int) l1) / 2);
}

inline static unsigned short median5(const unsigned short l[5])
{
    unsigned short l0 = l[0], l1 = l[1], l2 = l[2];
    unsigned short x;
    x = l[3];
    if (x < l0) swap(x, l0);
    if (x < l1) swap(x, l1);
    if (x < l2) swap(x, l2);
    x = l[4];
    if (x < l0) swap(x, l0);
    if (x < l1) swap(x, l1);
    if (x < l2) swap(x, l2);

    if (l1 > l0) l0 = l1;
    if (l2 > l0) l0 = l2;

    return l0;
}

inline static unsigned short median3(const unsigned short l[3])
{
    unsigned short l0 = l[0], l1 = l[1], l2 = l[2];
    if (l2 < l0) swap(l2, l0);
    if (l2 < l1) swap(l2, l1);
    if (l1 > l0) l0 = l1;
    return l0;
}

bool Median3(unsigned short *dst, const unsigned short *src, int xsize, int ysize)
{
    for (int y = 1; y < ysize - 1; y++)
    {
        for (int x = 1; x < xsize - 1; x++)
        {
            unsigned short array[9];
            array[0] = src[(x - 1) + (y - 1)*xsize];
            array[1] = src[(x)+(y - 1)*xsize];
            array[2] = src[(x + 1) + (y - 1)*xsize];
            array[3] = src[(x - 1) + (y)*xsize];
            array[4] = src[(x)+(y)*xsize];
            array[5] = src[(x + 1) + (y)*xsize];
            array[6] = src[(x - 1) + (y + 1)*xsize];
            array[7] = src[(x)+(y + 1)*xsize];
            array[8] = src[(x + 1) + (y + 1)*xsize];
            dst[x + y * xsize] = median9(array);
        }
        dst[(xsize - 1) + y * xsize] = src[(xsize - 1) + y * xsize];  // 1st & Last one in this row -- just grab from orig
        dst[y * xsize] = src[y * xsize];
    }
    for (int x = 0; x < xsize; x++)
    {
        dst[x + (ysize - 1) * xsize] = src[x + (ysize - 1) * xsize];  // Last row -- just duplicate
        dst[x] = src[x];  // First row
    }

    return false;
}

static unsigned short MedianBorderingPixels(const usImage& img, int x, int y)
{
    unsigned short array[8];
    int const xsize = img.Size.GetWidth();
    int const ysize = img.Size.GetHeight();

    if (x > 0 && y > 0 && x < xsize - 1 && y < ysize - 1)
    {
        array[0] = img.ImageData[(x-1) + (y-1) * xsize];
        array[1] = img.ImageData[(x)   + (y-1) * xsize];
        array[2] = img.ImageData[(x+1) + (y-1) * xsize];
        array[3] = img.ImageData[(x-1) + (y)   * xsize];
        array[4] = img.ImageData[(x+1) + (y)   * xsize];
        array[5] = img.ImageData[(x-1) + (y+1) * xsize];
        array[6] = img.ImageData[(x)   + (y+1) * xsize];
        array[7] = img.ImageData[(x+1) + (y+1) * xsize];
        return median8(array);
    }

    if (x == 0 && y > 0 && y < ysize - 1)
    {
        // On left edge
        array[0] = img.ImageData[(x)     + (y - 1) * xsize];
        array[1] = img.ImageData[(x)     + (y + 1) * xsize];
        array[2] = img.ImageData[(x + 1) + (y - 1) * xsize];
        array[3] = img.ImageData[(x + 1) + (y)     * xsize];
        array[4] = img.ImageData[(x + 1) + (y + 1) * xsize];
        return median5(array);
    }

    if (x == xsize - 1 && y > 0 && y < ysize - 1)
    {
        // On right edge
        array[0] = img.ImageData[(x)     + (y - 1) * xsize];
        array[1] = img.ImageData[(x)     + (y + 1) * xsize];
        array[2] = img.ImageData[(x - 1) + (y - 1) * xsize];
        array[3] = img.ImageData[(x - 1) + (y)     * xsize];
        array[4] = img.ImageData[(x - 1) + (y + 1) * xsize];
        return median5(array);
    }

    if (y == 0 && x > 0 && x < xsize - 1)
    {
        // On bottom edge
        array[0] = img.ImageData[(x - 1) + (y)     * xsize];
        array[1] = img.ImageData[(x - 1) + (y + 1) * xsize];
        array[2] = img.ImageData[(x)     + (y + 1) * xsize];
        array[3] = img.ImageData[(x + 1) + (y)     * xsize];
        array[4] = img.ImageData[(x + 1) + (y + 1) * xsize];
        return median5(array);
    }

    if (y == ysize - 1 && x > 0 && x < xsize - 1)
    {
        // On top edge
        array[0] = img.ImageData[(x - 1) + (y)     * xsize];
        array[1] = img.ImageData[(x - 1) + (y - 1) * xsize];
        array[2] = img.ImageData[(x)     + (y - 1) * xsize];
        array[3] = img.ImageData[(x + 1) + (y)     * xsize];
        array[4] = img.ImageData[(x + 1) + (y - 1) * xsize];
        return median5(array);
    }

    if (x == 0 && y == 0)
    {
        // At lower left corner
        array[0] = img.ImageData[(x + 1) + (y)     * xsize];
        array[1] = img.ImageData[(x)     + (y + 1) * xsize];
        array[2] = img.ImageData[(x + 1) + (y + 1) * xsize];
    }
    else if (x == 0 && y == ysize - 1)
    {
        // At upper left corner
        array[0] = img.ImageData[(x + 1) + (y)     * xsize];
        array[1] = img.ImageData[(x)     + (y - 1) * xsize];
        array[2] = img.ImageData[(x + 1) + (y - 1) * xsize];
    }
    else if (x == xsize - 1 && y == ysize - 1)
    {
        // At upper right corner
        array[0] = img.ImageData[(x - 1) + (y)     * xsize];
        array[1] = img.ImageData[(x)     + (y - 1) * xsize];
        array[2] = img.ImageData[(x - 1) + (y - 1) * xsize];
    }
    else if (x == xsize - 1 && y == 0)
    {
        // At lower right corner
        array[0] = img.ImageData[(x - 1) + (y)     * xsize];
        array[1] = img.ImageData[(x)     + (y + 1) * xsize];
        array[2] = img.ImageData[(x - 1) + (y + 1) * xsize];
    }
    else
    {
        // unreachable
        return 0;
    }

    return median3(array);
}

bool SquarePixels(usImage& img, float xsize, float ysize)
{
    // Stretches one dimension to square up pixels
    int x,y;
    int newsize;
    usImage tempimg;
    unsigned short *ptr;
    unsigned short *optr;
    double ratio, oldposition;

    if (!img.ImageData)
        return true;
    if (xsize == ysize) return false;  // nothing to do

    // Copy the existing data
    if (tempimg.Init(img.Size)) {
        pFrame->Alert(_("Memory allocation error"));
        return true;
    }
    ptr = tempimg.ImageData;
    optr = img.ImageData;
    for (x=0; x<img.NPixels; x++, ptr++, optr++) {
        *ptr=*optr;
    }

    float weight;
    int ind1, ind2, linesize;
    // if X > Y, when viewing stock, Y is unnaturally stretched, so stretch X to match
    if (xsize > ysize) {
        ratio = ysize / xsize;
        newsize = ROUND((float) tempimg.Size.GetWidth() * (1.0/ratio));  // make new image correct size
        img.Init(newsize,tempimg.Size.GetHeight());
        optr=img.ImageData;
        linesize = tempimg.Size.GetWidth();  // size of an original line
        for (y=0; y<img.Size.GetHeight(); y++) {
            for (x=0; x<newsize; x++, optr++) {
                oldposition = x * ratio;
                ind1 = (unsigned int) floor(oldposition);
                ind2 = (unsigned int) ceil(oldposition);
                if (ind2 > (tempimg.Size.GetWidth() - 1)) ind2 = tempimg.Size.GetWidth() - 1;
                weight = ceil(oldposition) - oldposition;
                *optr = (unsigned short) (((float) *(tempimg.ImageData + y*linesize + ind1) * weight) + ((float) *(tempimg.ImageData + y*linesize + ind1) * (1.0 - weight)));
            }
        }
    }
    return false;
}

bool Subtract(usImage& light, const usImage& dark)
{
    if ((!light.ImageData) || (!dark.ImageData))
        return true;
    if (light.NPixels != dark.NPixels)
        return true;

    unsigned int left, top, width, height;
    if (light.Subframe.GetWidth() > 0 && light.Subframe.GetHeight() > 0)
    {
        left = light.Subframe.GetLeft();
        width = light.Subframe.GetWidth();
        top = light.Subframe.GetTop();
        height = light.Subframe.GetHeight();
    }
    else
    {
        left = top = 0;
        width = light.Size.GetWidth();
        height = light.Size.GetHeight();
    }

    int mindiff = 65535;

    unsigned short *pl0 = &light.Pixel(left, top);
    const unsigned short *pd0 = &dark.Pixel(left, top);
    for (unsigned int r = 0; r < height;
         r++, pl0 += light.Size.GetWidth(), pd0 += light.Size.GetWidth())
    {
        unsigned short *const endl = pl0 + width;
        unsigned short *pl;
        const unsigned short *pd;
        for (pl = pl0, pd = pd0; pl < endl; pl++, pd++)
        {
            int diff = (int) *pl - (int) *pd;
            if (diff < mindiff)
                mindiff = diff;
        }
    }

    int offset = 0;
    if (mindiff < 0) // dark was lighter than light
        offset = -mindiff;

    pl0 = &light.Pixel(left, top);
    pd0 = &dark.Pixel(left, top);
    for (unsigned int r = 0; r < height;
         r++, pl0 += light.Size.GetWidth(), pd0 += light.Size.GetWidth())
    {
        unsigned short *const endl = pl0 + width;
        unsigned short *pl;
        const unsigned short *pd;
        for (pl = pl0, pd = pd0; pl < endl; pl++, pd++)
        {
            int newval = (int) *pl - (int) *pd + offset;
            if (newval < 0) newval = 0; // shouldn't hit this...
            else if (newval > 65535) newval = 65535;
            *pl = (unsigned short) newval;
        }
    }

    return false;
}

inline static unsigned short histo_median(unsigned short histo1[256], unsigned short histo2[65536], int n)
{
    n /= 2;
    unsigned int i;
    for (i = 0; i < 256; i++)
    {
        if (histo1[i] > n)
            break;
        n -= histo1[i];
    }
    for (i <<= 8; i < 65536; i++)
    {
        if (histo2[i] > n)
            break;
        n -= histo2[i];
    }
    return i;
}

static void MedianFilter(usImage& dst, const usImage& src, int halfWidth)
{
    dst.Init(src.Size);
    unsigned short *d = &dst.ImageData[0];

    int const width = src.Size.GetWidth();
    int const height = src.Size.GetHeight();

    for (int y = 0; y < height; y++)
    {
        int top = std::max(0, y - halfWidth);
        int bot = std::min(y + halfWidth, height - 1);
        int left = 0;
        int right = halfWidth;

        // TODO: we initialize the histogram at the start of each row, but we could make this faster
        // if we scan left to right, move down, scan right to left, move down so we never need to
        // reinitialize the histogram

        // initialize 2-level histogram
        unsigned short histo1[256];
        unsigned short histo2[65536];
        memset(&histo1[0], 0, sizeof(histo1));
        memset(&histo2[0], 0, sizeof(histo2));

        for (int j = top; j <= bot; j++)
        {
            const unsigned short *p = &src.Pixel(left, j);
            for (int i = left; i <= right; i++, p++)
            {
                ++histo1[*p >> 8];
                ++histo2[*p];
            }
        }
        unsigned int n = (right - left + 1) * (bot - top + 1);

        // read off first value for this row
        *d++ = histo_median(histo1, histo2, n);

        // loop across remaining columns for this row
        for (int i = 1; i < width; i++)
        {
            left = std::max(0, i - halfWidth);
            right = std::min(i + halfWidth, width - 1);

            // remove leftmost column
            if (left > 0)
            {
                const unsigned short *p = &src.Pixel(left - 1, top);
                for (int j = top; j <= bot; j++, p += width)
                {
                    --histo1[*p >> 8];
                    --histo2[*p];
                }
                n -= (bot - top + 1);
            }

            // add new column on right
            if (i + halfWidth <= width - 1)
            {
                const unsigned short *p = &src.Pixel(right, top);
                for (int j = top; j <= bot; j++, p += width)
                {
                    ++histo1[*p >> 8];
                    ++histo2[*p];
                }
                n += (bot - top + 1);
            }

            *d++ = histo_median(histo1, histo2, n);
        }
    }
}

struct ImageStatsWork
{
    ImageStats stats;
    usImage temp;
};

static void GetImageStats(ImageStatsWork& w, const usImage& img, const wxRect& win)
{
    w.temp.Init(img.Size);

    // Determine the mean and standard deviation
    double sum = 0.0;
    double a = 0.0;
    double q = 0.0;
    double k = 1.0;
    double km1 = 0.0;

    const unsigned short *p0 = &img.Pixel(win.GetLeft(), win.GetTop());
    unsigned short *dst = &w.temp.ImageData[0];
    for (int y = 0; y < win.GetHeight(); y++)
    {
        const unsigned short *end = p0 + win.GetWidth();
        for (const unsigned short *p = p0; p < end; p++)
        {
            *dst++ = *p;
            double const x = (double) *p;
            sum += x;
            double const a0 = a;
            a += (x - a) / k;
            q += (x - a0) * (x - a);
            km1 = k;
            k += 1.0;
        }
        p0 += img.Size.GetWidth();
    }

    w.stats.mean = sum / km1;
    w.stats.stdev = sqrt(q / km1);

    int winPixels = win.GetWidth() * win.GetHeight();
    unsigned short *tmp = &w.temp.ImageData[0];
    std::nth_element(tmp, tmp + winPixels / 2, tmp + winPixels);

    w.stats.median = tmp[winPixels / 2];

    // replace each pixel with the absolute deviation from the median
    unsigned short *p = tmp;
    for (int i = 0; i < winPixels; i++)
    {
        unsigned short ad = (unsigned short) std::abs((int) *p - (int) w.stats.median);
        *p++ = ad;
    }
    std::nth_element(tmp, tmp + winPixels / 2, tmp + winPixels);
    w.stats.mad = tmp[winPixels / 2];
}

void DefectMapDarks::BuildFilteredDark()
{
    enum { WINDOW = 15 };
    filteredDark.Init(masterDark.Size);
    MedianFilter(filteredDark, masterDark, WINDOW);
}

static wxString DefectMapMasterPath()
{
    return MyFrame::GetDarksDir() + PATHSEPSTR + wxString::Format("PHD2_defect_map_master_%d.fit", pConfig->GetCurrentProfileId());
}

static wxString DefectMapFilterPath()
{
    return MyFrame::GetDarksDir() + PATHSEPSTR + wxString::Format("PHD2_defect_map_master_filt_%d.fit", pConfig->GetCurrentProfileId());
}

void DefectMapDarks::SaveDarks(const wxString& notes)
{
    masterDark.Save(DefectMapMasterPath(), notes);
    filteredDark.Save(DefectMapFilterPath());
}

void DefectMapDarks::LoadDarks()
{
    masterDark.Load(DefectMapMasterPath());
    filteredDark.Load(DefectMapFilterPath());
}

struct BadPx
{
    unsigned short x;
    unsigned short y;
    int v;

    BadPx();
    BadPx(int x_, int y_, int v_) : x(x_), y(y_), v(v_) { }
    bool operator<(const BadPx& rhs) const { return v < rhs.v; }
};

typedef std::set<BadPx> BadPxSet;

struct DefectMapBuilderImpl
{
    DefectMapDarks *darks;
    ImageStatsWork w;
    wxArrayString mapInfo;
    int aggrCold;
    int aggrHot;
    BadPxSet coldPx;
    BadPxSet hotPx;
    BadPxSet::const_iterator coldPxThresh;
    BadPxSet::const_iterator hotPxThresh;
    unsigned int coldPxSelected;
    unsigned int hotPxSelected;
    bool threshValid;

    DefectMapBuilderImpl()
        :
        darks(0),
        aggrCold(100),
        aggrHot(100),
        threshValid(false)
    { }
};

DefectMapBuilder::DefectMapBuilder()
    : m_impl(new DefectMapBuilderImpl())
{
}

DefectMapBuilder::~DefectMapBuilder()
{
    delete m_impl;
}

inline static double AggrToSigma(int val)
{
    // Aggressiveness of 0 to 100 maps to signma factor from 8.0 to 0.125
    return exp2(3.0 - (6.0 / 100.0) * (double)val);
}

void DefectMapBuilder::Init(DefectMapDarks& darks)
{
    m_impl->darks = &darks;

    Debug.AddLine("DefectMapBuilder: Init");

    ::GetImageStats(m_impl->w, darks.masterDark,
        wxRect(0, 0, darks.masterDark.Size.GetWidth(), darks.masterDark.Size.GetHeight()));

    const ImageStats& stats = m_impl->w.stats;

    Debug.AddLine("DefectMapBuilder: Dark N = %d Mean = %.f Median = %d Standard Deviation = %.f MAD=%d",
        darks.masterDark.NPixels, stats.mean, stats.median, stats.stdev, stats.mad);

    // load potential defects

    int thresh = (int)(AggrToSigma(100) * stats.stdev);

    Debug.AddLine("DefectMapBuilder: load potential defects thresh = %d", thresh);

    usImage& dark = m_impl->darks->masterDark;
    usImage& medianFilt = m_impl->darks->filteredDark;

    m_impl->coldPx.clear();
    m_impl->hotPx.clear();

    for (int y = 0; y < dark.Size.GetHeight(); y++)
    {
        for (int x = 0; x < dark.Size.GetWidth(); x++)
        {
            int filt = (int) medianFilt.Pixel(x, y);
            int val = (int) dark.Pixel(x, y);
            int v = val - filt;
            if (v > thresh)
            {
                m_impl->hotPx.insert(BadPx(x, y, v));
            }
            else if (-v > thresh)
            {
                m_impl->coldPx.insert(BadPx(x, y, -v));
            }
        }
    }

    Debug.AddLine("DefectMapBuilder: Loaded %d cold %d hot", m_impl->coldPx.size(), m_impl->hotPx.size());
}

const ImageStats& DefectMapBuilder::GetImageStats() const
{
    return m_impl->w.stats;
}

void DefectMapBuilder::SetAggressiveness(int aggrCold, int aggrHot)
{
    m_impl->aggrCold = std::max(0, std::min(100, aggrCold));
    m_impl->aggrHot = std::max(0, std::min(100, aggrHot));
    m_impl->threshValid = false;
}

static void FindThresh(DefectMapBuilderImpl *impl)
{
    if (impl->threshValid)
        return;

    double multCold = AggrToSigma(impl->aggrCold);
    double multHot = AggrToSigma(impl->aggrHot);

    int coldThresh = (int) (multCold * impl->w.stats.stdev);
    int hotThresh = (int) (multHot * impl->w.stats.stdev);

    Debug.AddLine("DefectMap: find thresholds aggr:(%d,%d) sigma:(%.1f,%.1f) px:(%+d,%+d)",
        impl->aggrCold, impl->aggrHot, multCold, multHot, -coldThresh, hotThresh);

    impl->coldPxThresh = impl->coldPx.lower_bound(BadPx(0, 0, coldThresh));
    impl->hotPxThresh = impl->hotPx.lower_bound(BadPx(0, 0, hotThresh));

    impl->coldPxSelected = std::distance(impl->coldPxThresh, impl->coldPx.end());
    impl->hotPxSelected = std::distance(impl->hotPxThresh, impl->hotPx.end());

    Debug.AddLine("DefectMap: find thresholds found (%d,%d)", impl->coldPxSelected, impl->hotPxSelected);

    impl->threshValid = true;
}

int DefectMapBuilder::GetColdPixelCnt() const
{
    FindThresh(m_impl);
    return m_impl->coldPxSelected;
}

int DefectMapBuilder::GetHotPixelCnt() const
{
    FindThresh(m_impl);
    return m_impl->hotPxSelected;
}

inline static unsigned int emit_defects(DefectMap& defectMap, BadPxSet::const_iterator p0, BadPxSet::const_iterator p1, double stdev, int sign)
{
    unsigned int cnt = 0;
    for (BadPxSet::const_iterator it = p0; it != p1; ++it, ++cnt)
    {
        int v = sign * it->v;
        Debug.AddLine("DefectMap: defect @ (%d, %d) val = %d (%+.1f sigma)", it->x, it->y, v, stdev > 0.1 ? (double)v / stdev : 0.0);
        defectMap.push_back(wxPoint(it->x, it->y));
    }
    return cnt;
}

void DefectMapBuilder::BuildDefectMap(DefectMap& defectMap) const
{
    wxArrayString& info = m_impl->mapInfo;

    double multCold = AggrToSigma(m_impl->aggrCold);
    double multHot = AggrToSigma(m_impl->aggrHot);
    const ImageStats& stats = m_impl->w.stats;

    info.Clear();
    info.push_back(wxString::Format("Generated: %s", wxDateTime::UNow().FormatISOCombined(' ')));
    info.push_back(wxString::Format("Camera: %s", pCamera->Name));
    info.push_back(wxString::Format("Dark Exposure Time: %d ms", m_impl->darks->masterDark.ImgExpDur));
    info.push_back(wxString::Format("Dark Frame Count: %d", m_impl->darks->masterDark.ImgStackCnt));
    info.push_back(wxString::Format("Aggressiveness, cold: %d", m_impl->aggrCold));
    info.push_back(wxString::Format("Aggressiveness, hot: %d", m_impl->aggrHot));
    info.push_back(wxString::Format("Sigma Thresh, cold: %.2f", multCold));
    info.push_back(wxString::Format("Sigma Thresh, hot: %.2f", multHot));
    info.push_back(wxString::Format("Mean: %.f", stats.mean));
    info.push_back(wxString::Format("Stdev: %.f", stats.stdev));
    info.push_back(wxString::Format("Median: %d", stats.median));
    info.push_back(wxString::Format("MAD: %d", stats.mad));

    int deltaCold = (int)(multCold * stats.stdev);
    int deltaHot = (int)(multHot * stats.stdev);

    info.push_back(wxString::Format("DeltaCold: %+d", -deltaCold));
    info.push_back(wxString::Format("DeltaHot: %+d", deltaHot));

    Debug.AddLine("DefectMap: deltaCold = %+d deltaHot = %+d", -deltaCold, deltaHot);

    FindThresh(m_impl);

    defectMap.clear();
    unsigned int nr_cold = emit_defects(defectMap, m_impl->coldPxThresh, m_impl->coldPx.end(), stats.stdev, -1);
    unsigned int nr_hot = emit_defects(defectMap, m_impl->hotPxThresh, m_impl->hotPx.end(), stats.stdev, +1);

    Debug.AddLine("New defect map created, count=%d (cold=%d, hot=%d)", defectMap.size(), nr_cold, nr_hot);
}

const wxArrayString& DefectMapBuilder::GetMapInfo() const
{
    return m_impl->mapInfo;
}

bool RemoveDefects(usImage& light, const DefectMap& defectMap)
{
    // Check to make sure the light frame is valid
    if (!light.ImageData)
        return true;

    if (light.Subframe.GetWidth() > 0 && light.Subframe.GetHeight() > 0)
    {
        // Step over each defect and replace the light value
        // with the median of the surrounding pixels
        for (DefectMap::const_iterator it = defectMap.begin(); it != defectMap.end(); ++it)
        {
            const wxPoint& pt = *it;
            // Check to see if we are within the subframe before correcting the defect
            if (light.Subframe.Contains(pt))
            {
                light.Pixel(pt.x, pt.y) = MedianBorderingPixels(light, pt.x, pt.y);
            }
        }
    }
    else
    {
        // Step over each defect and replace the light value
        // with the median of the surrounding pixels
        for (DefectMap::const_iterator it = defectMap.begin(); it != defectMap.end(); ++it)
        {
            int const x = it->x;
            int const y = it->y;

            if (x >= 0 && x < light.Size.GetWidth() && y >= 0 && y < light.Size.GetHeight())
            {
                light.Pixel(x, y) = MedianBorderingPixels(light, x, y);
            }
        }
    }

    return false;
}

wxString DefectMap::DefectMapFileName(int profileId)
{
    return MyFrame::GetDarksDir() + PATHSEPSTR + wxString::Format("PHD2_defect_map_%d.txt", profileId);
}

bool DefectMap::DefectMapExists(int profileId)
{
    return wxFileExists(DefectMapFileName(profileId));
}

void DefectMap::Save(const wxArrayString& info) const
{
    wxString filename = DefectMapFileName(m_profileId);
    wxFileOutputStream oStream(filename);
    wxTextOutputStream outText(oStream);

    if (oStream.GetLastError() != wxSTREAM_NO_ERROR)
    {
        Debug.AddLine(wxString::Format("Failed to save defect map to %s", filename));
        return;
    }

    outText << "# PHD2 Defect Map v1\n";

    for (wxArrayString::const_iterator it = info.begin(); it != info.end(); ++it)
    {
        outText << "# " << *it << "\n";
    }
    outText << "# Defect count: " << size() << "\n";

    for (const_iterator it = begin(); it != end(); ++it)
    {
        outText << it->x << " " << it->y << "\n";
    }

    oStream.Close();
    Debug.AddLine(wxString::Format("Saved defect map to %s", filename));
}

DefectMap::DefectMap()
    : m_profileId(pConfig->GetCurrentProfileId())
{
}

DefectMap::DefectMap(int profileId)
    : m_profileId(profileId)
{
}

void DefectMap::AddDefect(const wxPoint& pt)
{
    // first add the point
    push_back(pt);

    wxString filename = DefectMapFileName(m_profileId);
    wxFile file(filename, wxFile::write_append);
    wxFileOutputStream oStream(file);
    wxTextOutputStream outText(oStream);

    if (oStream.GetLastError() != wxSTREAM_NO_ERROR)
    {
        Debug.AddLine(wxString::Format("Failed to save defect map to %s", filename));
        return;
    }

    outText << pt.x << " " << pt.y << "\n";

    oStream.Close();
    Debug.AddLine(wxString::Format("Saved defect map to %s", filename));
}

DefectMap *DefectMap::LoadDefectMap(int profileId)
{
    wxString filename = DefectMapFileName(profileId);
    Debug.AddLine(wxString::Format("Loading defect map file %s", filename));

    if (!wxFileExists(filename))
    {
        Debug.AddLine(wxString::Format("Defect map file not found: %s", filename));
        return 0;
    }

    wxFileInputStream iStream(filename);
    wxTextInputStream inText(iStream);

    // Re-initialize the defect map and parse the defect map file
    if (iStream.GetLastError() != wxSTREAM_NO_ERROR)
    {
        Debug.AddLine(wxString::Format("Unexpected eof on defect map file %s", filename));
        return 0;
    }

    DefectMap *defectMap = new DefectMap(profileId);

    int linenum = 0;
    while (!inText.GetInputStream().Eof())
    {
        wxString line = inText.ReadLine();
        ++linenum;
        line.Trim(false); // trim leading whitespace
        if (line.IsEmpty())
            continue;
        if (line.StartsWith("#"))
            continue;

        wxStringTokenizer tok(line);
        wxString s1 = tok.GetNextToken();
        wxString s2 = tok.GetNextToken();
        long x, y;
        if (s1.ToLong(&x) && s2.ToLong(&y))
        {
            defectMap->push_back(wxPoint(x, y));
        }
        else
        {
            Debug.AddLine(wxString::Format("DefectMap: ignore junk on line %d: %s", linenum, line));
        }
    }

    Debug.AddLine(wxString::Format("Loaded %d defects", defectMap->size()));
    return defectMap;
}

void DefectMap::DeleteDefectMap(int profileId)
{
    wxString filename = DefectMapFileName(profileId);
    if (wxFileExists(filename))
    {
        Debug.AddLine("Removing defect map file: " + filename);
        wxRemoveFile(filename);
    }
}
