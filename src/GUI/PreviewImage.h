#pragma once

#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <wx/panel.h>

class PreviewImage : public wxPanel
{
public:
    PreviewImage(wxWindow* parent): wxPanel(parent)
    {
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        SetMinSize(wxSize(200, 200));
        Bind(wxEVT_SIZE, &PreviewImage::OnResize, this);
        Bind(wxEVT_PAINT, &PreviewImage::OnPaint, this);
    }

    void SetImage(const wxBitmap& theBitmap)
    {
        mOriginal = theBitmap; 
        Refresh();
    }

    void SetGridSize(int row, int col)
    {
        mRowCount = row;
        mColCount = col;
        Refresh();
    }

private:
    wxBitmap mOriginal;
    wxBitmap mScaled;
    int mRowCount;
    int mColCount;

    void OnPaint(wxPaintEvent& event)
    {
        wxAutoBufferedPaintDC dc(this);
        dc.Clear();
        if (mScaled.IsOk())
            dc.DrawBitmap(mScaled, 0, 0, true);

        if (mRowCount > 1 || mColCount > 1)
        {
            wxSize sz = wxSize(mScaled.GetWidth(), mScaled.GetHeight());
            dc.SetPen(wxPen(*wxRED, 1, wxPENSTYLE_SOLID));

            for (int c = 1; c < mColCount; ++c)
            {
                int x = c * sz.x / mColCount;
                dc.DrawLine(x, 0, x, sz.y);
            }

            for (int r = 1; r < mRowCount; ++r)
            {
                int y = r * sz.y / mRowCount;
                dc.DrawLine(0, y, sz.x, y);
            }
        }
    }

    void OnResize(wxSizeEvent& event)
    {
        if (mOriginal.IsOk())
        {
            wxSize sz = GetClientSize();
            wxImage img = mOriginal.ConvertToImage();
            float scale = std::min((float)sz.x / mOriginal.GetWidth(), (float)sz.y / mOriginal.GetHeight());
            if (scale == 0)
                scale = 0.01;
            img.Rescale(mOriginal.GetWidth() * scale, mOriginal.GetHeight() * scale, wxIMAGE_QUALITY_HIGH);
            mScaled = wxBitmap(img);
        }
        Refresh();
        event.Skip();
    }
};
