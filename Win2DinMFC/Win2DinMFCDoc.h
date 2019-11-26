
// Win2DinMFCDoc.h : interface of the CWin2DinMFCDoc class
//


#pragma once

#include <string>

class CWin2DinMFCDoc : public CDocument
{
protected: // create from serialization only
	CWin2DinMFCDoc() noexcept;
	DECLARE_DYNCREATE(CWin2DinMFCDoc)

	// Attributes
public:
	std::string m_svg_xml;

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CWin2DinMFCDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
