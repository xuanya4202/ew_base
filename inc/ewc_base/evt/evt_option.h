#ifndef __H_EW_UI_EVT_EVT_OPTION__
#define __H_EW_UI_EVT_EVT_OPTION__

#include "ewc_base/evt/evt_command.h"


EW_ENTER

class DLLIMPEXP_EWC_BASE EvtOptionPage : public EvtCommand
{
public:
	typedef EvtCommand basetype;

	WndManager& wm;

	EvtOptionPage(const String& s);

	DataPtrT<ValidatorGroup> m_pValdGroup;
	DataPtrT<Validator> m_pVald;

	virtual wxWindow* CreatePage(wxWindow* w);

protected:
	virtual void DoCreatePage(WndMaker&);

};


EW_LEAVE
#endif
