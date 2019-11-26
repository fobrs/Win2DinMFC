
// Win2DinMFCDoc.cpp : implementation of the CWin2DinMFCDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "Win2DinMFC.h"
#endif

#include "Win2DinMFCDoc.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CWin2DinMFCDoc

IMPLEMENT_DYNCREATE(CWin2DinMFCDoc, CDocument)

BEGIN_MESSAGE_MAP(CWin2DinMFCDoc, CDocument)
END_MESSAGE_MAP()


// CWin2DinMFCDoc construction/destruction

CWin2DinMFCDoc::CWin2DinMFCDoc() noexcept
{
	// TODO: add one-time construction code here

}

CWin2DinMFCDoc::~CWin2DinMFCDoc()
{
}

BOOL CWin2DinMFCDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	m_svg_xml.clear();

	return TRUE;
}




// CWin2DinMFCDoc serialization

void CWin2DinMFCDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		CFile * file = ar.GetFile();
		if (file)
		{
			auto len = file->GetLength();

			m_svg_xml.resize(len + 2, 0);
			if (ar.Read(&m_svg_xml[0], len) != len)
			{
				ar.Abort();
			}
		}
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CWin2DinMFCDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CWin2DinMFCDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data.
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CWin2DinMFCDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = nullptr;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != nullptr)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CWin2DinMFCDoc diagnostics

#ifdef _DEBUG
void CWin2DinMFCDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CWin2DinMFCDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CWin2DinMFCDoc commands
