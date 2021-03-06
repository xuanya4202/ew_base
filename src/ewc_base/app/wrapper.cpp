
#include "ewc_base/app/app.h"
#include "ewc_base/app/res_manager.h"
#include "ewc_base/plugin/plugin_manager.h"
#include "ewc_base/wnd/impl_wx/window.h"
#include "ewc_base/app/wrapper.h"

#include "wx/clipbrd.h"
#include "wx/filedlg.h"


EW_ENTER

 int Wrapper::FileDialog(String& file,int type,const String& title,const String& exts)
{
	arr_1t<String> files;
	int ret=FileDialog(files,type,title,exts);
	if(!files.empty())
	{
		file=files[0];
	}
	return ret;
}

int Wrapper::FileDialog(arr_1t<String>& files,int type,const String& title,const String& exts)
{

	wxString wxtitle(str2wx(title));
	if(wxtitle=="")
	{
		wxtitle="Please select a file";
	}

	int flag=(type&IDefs::FD_SAVE)==0?wxFD_OPEN:wxFD_SAVE;
	if(type&IDefs::FD_MULTI) flag|=wxFD_MULTIPLE;
	if(type&IDefs::FD_MUST_EXIST) flag|=wxFD_FILE_MUST_EXIST;

	wxString wxexts;
	if(!exts.empty())
	{
		if(exts.find('|')<0)
		{
			wxexts+=str2wx(exts+"|"+exts+"|"+_hT("AllFiles")+"(*.*)|*.*");
		}
		else
		{
			wxexts+=str2wx(exts+"|"+_hT("AllFiles")+"(*.*)|*.*");
		}
	}


	wxWindow* ptopwindow=WndModel::current().GetWindow();

	::wxFileDialog dlg(ptopwindow,wxtitle,"","",wxexts,flag);
	if(files.size()==1)
	{
		dlg.SetPath(str2wx(files[0]));
	}

	if(dlg.ShowModal()==wxID_OK)
	{
		files.clear();
		wxArrayString wxfiles;
		dlg.GetPaths(wxfiles);
		for(size_t i=0;i<wxfiles.size();i++)
		{
			files.append(wx2str(wxfiles[i]));
		}
		return IDefs::BTN_OK;
	}
	else
	{
		return IDefs::BTN_CANCEL;
	}
}

int Wrapper::MsgsDialog(const String& cont,int type,const String& title)
{
	wxWindow* ptopwindow=WndModel::current().GetWindow();
	int fg=wxCENTER;
	if(type&IDefs::BTN_YES) fg|=wxYES;
	if(type&IDefs::BTN_NO) fg|=wxNO;
	if(type&IDefs::BTN_CANCEL) fg|=wxCANCEL;
	if(type&IDefs::BTN_OK) fg|=wxOK;

	::wxMessageDialog dlg(NULL,str2wx(cont),str2wx(title),fg);
	int ret=dlg.ShowModal();

	if(ret==wxID_YES)
	{
		return IDefs::BTN_YES;
	}
	else if(ret==wxID_NO)
	{
		return IDefs::BTN_NO;
	}
	else if(ret==wxID_OK)
	{
		return IDefs::BTN_OK;
	}
	else
	{
		return IDefs::BTN_CANCEL;
	}

}

bool Wrapper::SetClipboardText(const String& s)
{
	wxClipboard* pcb=wxClipboard::Get();
	if(!pcb||!pcb->Open()) return false;
	pcb->SetData(new wxTextDataObject(str2wx(s)));
	pcb->Close();
	return true;
}

bool Wrapper::GetClipboardText(String& s)
{
	wxClipboard* pcb=wxClipboard::Get();
	if(!pcb||!pcb->Open()) return false;
	wxTextDataObject data;
	pcb->GetData(data);
	s=wx2str(data.GetText());
	pcb->Close();
	return true;
}

EW_LEAVE
