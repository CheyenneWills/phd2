/*
 *  cam_qhy.h
 *  Open PHD Guiding
 *
 *  Created by Andy Galasso
 *  Copyright (c) 2015 Andy Galasso.
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
 *    Neither the name of openphdguiding.org nor the names of its
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

#ifndef CAM_QHY_INCLUDED
#define CAM_QHY_INCLUDED

#include "camera.h"
#include "qhyccd.h"

class Camera_QHY : public GuideCamera
{
    qhyccd_handle *m_camhandle;
    double m_gainMin;
    double m_gainMax;
    double m_gainStep;
    double m_devicePixelSize;
    unsigned char *RawBuffer;
    wxSize m_maxSize;
    int m_curGain;
    int m_curExposure;
    unsigned short m_curBin;
    wxRect m_roi;
    bool Color;

public:

    Camera_QHY();
    ~Camera_QHY();

    bool    Capture(int duration, usImage& img, int options, const wxRect& subframe);
    bool    Connect(const wxString& camId);
    bool    Disconnect();

    bool    ST4PulseGuideScope(int direction, int duration);

    bool HasNonGuiCapture() { return true; }
    bool ST4HasNonGuiMove() { return true; }
    wxByte BitsPerPixel();
    bool GetDevicePixelSize(double *devPixelSize);
};

#endif // QHY5IIBASE_H_INCLUDED
