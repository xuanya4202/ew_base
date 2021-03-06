#include "evt_impl.h"

#include "ewc_base/evt/validator.h"
#include "ewc_base/app/res_manager.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/plugin/plugin_manager.h"
#include "ewc_base/wnd/wnd_model.h"

EW_ENTER


bool EvtCommandFunctor::CmdExecute(ICmdParam& cmd)
{
	if(!func)
	{
		return false;
	}

	return func(cmd);
}


String EvtCommand::MakeLabel(int hint) const
{
	String txt=m_sLabel;

	if(txt=="")
	{
		txt=m_sText;
	}

	if(txt=="")
	{
		txt=m_sId;
	}

	if(hint==LABEL_MENUBAR)
	{
		return txt;
	}

	if(hint==LABEL_MENU && m_sHotkey!="")
	{
		txt=txt+"(&"+m_sHotkey+")";
	}

	if(m_sExtra!="")
	{
		txt=txt+" "+m_sExtra;
	}

	if(flags.get(FLAG_DOTDOT))
	{
		txt+=(" ..");
	}

	if(hint==LABEL_MENU && m_sAccel!="")
	{
		txt+="\t"+m_sAccel;
	}

	return txt;
}

HeToolItemImpl* EvtCommand::CreateToolItem(HeTbarImpl* tb)
{
	if(flags.get(FLAG_SEPARATOR))
	{
		tb->AddSeparator();
		return NULL;
	}

	HeToolItemImpl* item=DoCreateToolImpl(tb,this);

	if(!m_bmpParam)	m_bmpParam=ResManager::current().icons.get(m_sId);
	m_bmpParam.update(item);

	tb->AddTool(item);
	UpdateToolItem(item);

	return item;
}

HeMenuItemImpl* EvtCommand::CreateMenuItem(HeMenuImpl* mu)
{
	if(flags.get(FLAG_SEPARATOR))
	{
		mu->AppendSeparator();
		return NULL;
	}

	HeMenuItemImpl* item=DoCreateMenuImpl(mu,this);

	if(!m_bmpParam)	m_bmpParam=ResManager::current().icons.get(m_sId);
	m_bmpParam.update(item);

	mu->Append(item);
	UpdateMenuItem(item);

	return item;

}

void EvtCommand::DoUpdateCtrl(IUpdParam& upd)
{
	if(flags.get(FLAG_HIDE_UI))
	{
		bst_set<HeMenuItemImpl*> tmp(m_setMenuImpls);
		for(bst_set<HeMenuItemImpl*>::iterator it=tmp.begin();it!=tmp.end();++it)
		{
			HeMenuImpl* mu=(*it)->GetMenu();
			if(mu) mu->Delete(*it);
		}
	}
	else
	{
		for(bst_set<HeMenuItemImpl*>::iterator it=m_setMenuImpls.begin();it!=m_setMenuImpls.end();++it)
		{
			UpdateMenuItem(*it);
		}
	}

	for(bst_set<HeToolItemImpl*>::iterator it=m_setToolImpls.begin();it!=m_setToolImpls.end();++it)
	{
		UpdateToolItem(*it);
	}
	
	EvtBase::DoUpdateCtrl(upd);
}

void EvtCommand::UpdateMenuItem(HeMenuItemImpl* item)
{
	HeMenuImpl* mu=item->GetMenu();
	if(!mu||&mu->wm!=&WndManager::current()) return;

	item->SetHelp(str2wx(m_sHelp));
	item->SetItemLabel(str2wx(MakeLabel(LABEL_MENU)));
	item->SetCheckable(flags.get(FLAG_CHECK));
	item->Enable(!flags.get(FLAG_DISABLE));

	if(flags.get(FLAG_CHECK))
	{
		item->Check(flags.get(FLAG_CHECKED));
	}

}

void EvtCommand::UpdateToolItem(HeToolItemImpl* item)
{
	if(m_nId<0)
	{
		return;
	}

	HeTbarImpl* tb=item->GetToolBar();
	if(!tb||&tb->wm!=&WndManager::current()) return;

	item->SetShortHelp(str2wx(MakeLabel()));
	item->SetLabel(str2wx(MakeLabel(LABEL_TOOL)));
	item->SetLongHelp(str2wx(m_sHelp));

	if(flags.get(FLAG_CHECK))
	{
		item->SetToggle(true);
		tb->ToggleTool(m_nId,flags.get(FLAG_CHECKED));
	}
	tb->EnableTool(m_nId,!flags.get(FLAG_DISABLE|FLAG_HIDE_UI));
}

HeMenuItemImpl* EvtCommand::DoCreateMenuImpl(HeMenuImpl* mu,EvtCommand* it)
{
	HeMenuItemImpl* item= new HeMenuItemImpl(mu,it);
	return item;
}

HeToolItemImpl* EvtCommand::DoCreateToolImpl(HeTbarImpl* tb,EvtCommand* it)
{
	HeToolItemImpl* tbitem=new HeToolItemImpl(tb,it);
	return tbitem;
}

IWindowPtr EvtCommand::CreateWndsItem(IWindowPtr pw)
{
	if(flags.get(FLAG_SEPARATOR)) return NULL;

	WndProperty wp;
	wp.id(m_nId);
	wp.label(m_sId);

	wxWindow* ctrl= WndInfoManger::current().Create("button",pw,wp);
	if(!ctrl) return NULL;

	CreateValidator(ctrl);
	return ctrl;

}

bool EvtCommandWindow::DoCmdExecute(ICmdParam& cmd)
{
	if(!m_pWindow) return true;

	if(cmd.param1>=+2)
	{
		flags.del(FLAG_DISABLE|FLAG_HIDE_UI);
	}
	else if(cmd.param1==-2)
	{
		flags.add(FLAG_DISABLE|FLAG_HIDE_UI);
	}
	else if(cmd.param1<-2)
	{
		flags.add(FLAG_DISABLE|FLAG_HIDE_UI);
	}
	else
	{
		flags.set(FLAG_CHECKED,cmd.param1!=0);
	}

	bool show=flags.get(FLAG_CHECKED)&&!flags.get(FLAG_DISABLE);

	OnWindow(show?IDefs::WND_SHOW:IDefs::WND_HIDE);

	UpdateCtrl();

	return true;
}

bool EvtCommandWindow::OnWindow(int flag)
{
	if(m_pWindow)
	{
		WndModel::current().OnChildWindow(m_pWindow,flag);
	}
	return true;
}

void EvtCommandWindow::SetWindow(wxWindow* w)
{
	if(m_pWindow)
	{
		OnWindow(IDefs::WND_DETACH);
		flags.del(FLAG_CHECK|FLAG_CHECKED);
		m_sText="";
	}
	m_pWindow=w;
	if(m_pWindow)
	{
		flags.add(FLAG_CHECK);	
		flags.set(FLAG_CHECKED,w->IsShown());
		m_sText=Translate(wx2str(w->GetName()));
		OnWindow(IDefs::WND_ATTACH);
	}
}


EvtCommandCtrl::EvtCommandCtrl(const String& id,const String& type,const WndProperty& w):basetype(id),m_sCtrlType(type),wp(w)
{

}

bool EvtCommandCtrl::DoStdExecute(IStdParam& cmd)
{
	if(m_pWindow) return false;

	wxWindow* pw=CreateWndsItem(NULL);

	if(!pw) return false;
	if(cmd.param1>0) flags.add(cmd.param1);
	if(flags.get(FLAG_HIDE_UI)||!flags.get(FLAG_CHECKED)) pw->Hide();

	SetWindow(pw);
	return true;
}

void EvtCommandCtrl::DoUpdateCtrl(IUpdParam& upd)
{
	basetype::DoUpdateCtrl(upd);
}

EvtCommandExtraWindow::EvtCommandExtraWindow(const String& id,const String& type,int h,const WndProperty& w):EvtCommandCtrl(id,type,w)
{
	StdExecuteEx(h);
}

HeToolItemImpl* EvtCommandCtrl::CreateToolItem(HeTbarImpl* tb)
{
	if(m_pWindow)
	{
		return NULL;
	}

	wxWindow* pwin=WndInfoManger::current().Create(m_sCtrlType,tb,wp);
	if(!pwin) return NULL;

	wxControl* ctrl=dynamic_cast<wxControl*>(pwin);
	if(!ctrl)
	{
		delete pwin;
		return NULL;
	}

	HeToolItemImpl* item=new HeToolItemImpl(tb,this,ctrl);
	tb->AddTool(item);

	CreateValidator(ctrl);

	m_pWindow=pwin;

	return item;
}

IWindowPtr EvtCommandCtrl::CreateWndsItem(IWindowPtr pw)
{
	if(m_pWindow)
	{
		return NULL;
	}

	if(!pw)
	{
		pw=WndModel::current().GetWindow();
	}

	
	wxWindow* pwin=WndInfoManger::current().Create(m_sCtrlType,pw,wp);
	if(!pwin) return NULL;

	pwin->SetName(str2wx(m_sId));
	CreateValidator(pwin);
	m_pWindow=pwin;
	return pwin;
}

Validator* EvtCommandCtrl::CreateValidator(wxWindow* w)
{
	WndInfo* wi=WndInfoManger::current().GetWndInfo(w);
	if(!wi) return NULL;
	return wi->CreateValidator(w,this);	
}

bool EvtCommandText::DoStdExecute(IStdParam& cmd)
{
	if(cmd.param1<=0)
	{
		value=cmd.extra;
		return WndExecuteEx(IDefs::ACTION_TRANSFER2WINDOW);
	}
	else if(WndExecuteEx(IDefs::ACTION_TRANSFER2MODEL))
	{
		cmd.extra=value;
		return true;
	}
	else
	{
		return false;
	}
}

bool EvtCommandText::DoWndExecute(IWndParam& cmd)
{

	Validator* pv=cmd.iwvptr;
	if(!pv)
	{
		if(m_aAttachedListeners.empty()) return true;
		pv=dynamic_cast<Validator*>(m_aAttachedListeners[0]);
		if(!pv) return false;
	}

	if(cmd.action==IDefs::ACTION_VALUE_CHANGED||cmd.action==IDefs::ACTION_TRANSFER2MODEL)
	{
		pv->DoGetValue(value);
	}
	else if(cmd.action==IDefs::ACTION_TRANSFER2WINDOW)
	{
		pv->DoSetValue(value);
	}

	return true;
}

EvtCommandWindow::EvtCommandWindow(wxWindow* pw):basetype(wx2str(pw->GetName()))
{
	SetWindow(pw);
}

EvtCommandWindow::EvtCommandWindow(const String& s,wxWindow* w):basetype(s)
{
	SetWindow(w);
}

IWindowPtr EvtCommandWindow::CreateWndsItem(IWindowPtr pw)
{
	return NULL;
}

void EvtCommandWindow::DoUpdateCtrl(IUpdParam& upd)
{
	EvtCommand::DoUpdateCtrl(upd);
}

EvtCommandWindowSharedBook::EvtCommandWindowSharedBook(const String& n):EvtCommandWindow(n)
{
	IWindowPtr p=WndModel::current().GetWindow();
	if(p)
	{
		WndProperty wp;
		m_pWindow=new IWnd_bookbase(p,wp);
		m_pWindow->SetName(str2wx(n));
		SetWindow(m_pWindow);
	}
}

bool EvtCommandWindowSharedBook::OnWindow(int show)
{
	return EvtCommandWindow::OnWindow(show);
}


bool EvtCommandWindowSharedBook::SelPage(const String& s,wxWindow* w)
{
	EvtBase* pevt=EvtManager::current().get(s);
	EvtCommandWindowSharedBook* pevt_book=NULL;

	if(!pevt)
	{
		pevt_book=new EvtCommandWindowSharedBook(s);
		EvtManager::current().gp_beg("OtherWindow");
		EvtManager::current().gp_add(pevt_book);
		EvtManager::current().gp_end();
	}
	else
	{
		pevt_book=dynamic_cast<EvtCommandWindowSharedBook*>(pevt);
		if(!pevt_book)
		{
			return false;
		}
	}

	if(w) w->Reparent(pevt_book->m_pWindow);

	pevt_book->m_pWindow->SelPage(w);
	return true;

}

Validator* EvtCommand::CreateValidator(wxWindow* w)
{
	WndInfo* wi=WndInfoManger::current().GetWndInfo(w);
	if(!wi) return NULL;
	return wi->CreateValidator(w,this);
}


EvtCommandShowModel::EvtCommandShowModel(const String& s,WndModel* p)
	:basetype(s)
	,m_pModel(p)
{
	flags.add(FLAG_CHECK);
	AttachEvent(p);
}

bool EvtCommandShowModel::OnWndEvent(IWndParam& cmd,int phase)
{
	if(phase>0 && cmd.action<=IDefs::ACTION_WINDOW_FINI)
	{
		UpdateCtrl();
	}
	return true;
}

void EvtCommandShowModel::DoUpdateCtrl(IUpdParam& upd)
{
	flags.set(FLAG_CHECKED,m_pModel && m_pModel->IsShown());
	basetype::DoUpdateCtrl(upd);
}

bool EvtCommandShowModel::DoCmdExecute(ICmdParam& cmd)
{
	if(m_pModel) m_pModel->Show(cmd.param1!=0);
	return true;
}

EW_LEAVE
