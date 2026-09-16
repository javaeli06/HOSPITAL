#include <windows.h>
#include <string>
#include <vector>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

const wchar_t CLASS_NAME[] = L"HondaClinicWindow";
HINSTANCE g_hInst;
HWND g_content;
HFONT g_font, g_titleFont, g_smallFont;
COLORREF BLUE = RGB(31,102,193), LIGHT = RGB(245,247,250), WHITE = RGB(255,255,255), TEXT = RGB(45,55,72), MUTED = RGB(110,120,135), BORDER = RGB(220,225,232);

struct ButtonInfo { HWND h; std::wstring page; };
std::vector<ButtonInfo> nav;

void Fill(HDC dc, RECT r, COLORREF c){ HBRUSH b=CreateSolidBrush(c); FillRect(dc,&r,b); DeleteObject(b); }
void Text(HDC dc, int x,int y,const std::wstring&s,COLORREF c,int size=15,bool bold=false){
    HFONT f=CreateFontW(size,0,0,0,bold?FW_BOLD:FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
    HFONT old=(HFONT)SelectObject(dc,f); SetTextColor(dc,c); SetBkMode(dc,TRANSPARENT); TextOutW(dc,x,y,s.c_str(),(int)s.size()); SelectObject(dc,old); DeleteObject(f);
}
void Card(HDC dc, RECT r){ Fill(dc,r,WHITE); FrameRect(dc,&r,CreateSolidBrush(BORDER)); }

void ClearContent(){ for(auto &b:nav){} }

void DrawDashboard(HWND w,HDC dc){
    Text(dc,250,35,L"Dashboard",TEXT,26,true); Text(dc,250,72,L"Welcome back, Admin!",MUTED,14);
    const wchar_t* labels[]={L"Total Patients",L"Doctors",L"Appointments",L"Revenue"}; const wchar_t* vals[]={L"69",L"15",L"32",L"₱89,000"};
    for(int i=0;i<4;i++){RECT r={250+i*190,115,425+i*190,205}; Card(dc,r); Text(dc,r.left+18,r.top+18,labels[i],MUTED,13); Text(dc,r.left+18,r.top+45,vals[i],TEXT,25,true);}
    RECT r={250,235,1010,500}; Card(dc,r); Text(dc,270,260,L"Today's Appointments",TEXT,19,true);
    Text(dc,270,305,L"Patient",MUTED,13); Text(dc,510,305,L"Doctor",MUTED,13); Text(dc,760,305,L"Time",MUTED,13); Text(dc,900,305,L"Status",MUTED,13);
    const wchar_t* p[]={L"Japeth Canonse",L"Mark Natural",L"Ishmael Catindig"}; const wchar_t* d[]={L"Dr. Jayson Lagera",L"Dr. Kent Gutierez",L"Dr. Eduard Javier"}; const wchar_t* t[]={L"9:00 AM",L"10:30 AM",L"1:00 PM"}; const wchar_t* s[]={L"Confirmed",L"Waiting",L"Confirmed"};
    for(int i=0;i<3;i++){int y=345+i*45; Text(dc,270,y,p[i],TEXT,14); Text(dc,510,y,d[i],TEXT,14); Text(dc,760,y,t[i],TEXT,14); Text(dc,900,y,s[i],i==1?RGB(190,125,20):RGB(35,145,85),13,true);}
}

void DrawPatients(HDC dc){
    Text(dc,250,35,L"Patients",TEXT,26,true); Text(dc,250,72,L"Manage patient records",MUTED,14);
    RECT r={250,110,1010,530}; Card(dc,r);
    Text(dc,275,135,L"Patient ID",MUTED,12); Text(dc,360,135,L"Patient Name",MUTED,12); Text(dc,530,135,L"Age",MUTED,12); Text(dc,585,135,L"Sex",MUTED,12); Text(dc,650,135,L"Contact",MUTED,12); Text(dc,820,135,L"Doctor",MUTED,12);
    const wchar_t* id[]={L"P001",L"P002",L"P003",L"P004",L"P005",L"P006"}; const wchar_t* n[]={L"John Doe",L"Maria Cruz",L"Michael Garcia",L"Angela Santos",L"Daniel Reyes",L"Sofia Mendoza"}; const wchar_t* a[]={L"22",L"25",L"30",L"28",L"35",L"24"}; const wchar_t* sx[]={L"Male",L"Female",L"Male",L"Female",L"Male",L"Female"}; const wchar_t* ph[]={L"0912-345-6789",L"0917-234-5678",L"0920-456-7890",L"0918-567-8901",L"0921-678-9012",L"0919-789-0123"}; const wchar_t* dr[]={L"Dr. Santos",L"Dr. Reyes",L"Dr. Garcia",L"Dr. Cruz",L"Dr. Mendoza",L"Dr. Santos"};
    for(int i=0;i<6;i++){int y=175+i*50; Text(dc,275,y,id[i],TEXT,13); Text(dc,360,y,n[i],TEXT,13); Text(dc,530,y,a[i],TEXT,13); Text(dc,585,y,sx[i],TEXT,13); Text(dc,650,y,ph[i],TEXT,13); Text(dc,820,y,dr[i],TEXT,13);}
}

void DrawAppointments(HDC dc){
    Text(dc,250,35,L"Appointments",TEXT,26,true); Text(dc,250,72,L"Schedule and manage appointments",MUTED,14);
    RECT r={250,110,1010,360}; Card(dc,r); Text(dc,275,135,L"ID",MUTED,12); Text(dc,325,135,L"Patient",MUTED,12); Text(dc,480,135,L"Doctor",MUTED,12); Text(dc,620,135,L"Department",MUTED,12); Text(dc,770,135,L"Date",MUTED,12); Text(dc,880,135,L"Status",MUTED,12);
    const wchar_t* id[]={L"A001",L"A002",L"A003"}; const wchar_t* p[]={L"Juan Dela Cruz",L"Maria Reyes",L"Pedro Santos"}; const wchar_t* d[]={L"Dr. Santos",L"Dr. Cruz",L"Dr. Garcia"}; const wchar_t* dep[]={L"General Medicine",L"Pediatrics",L"Cardiology"}; const wchar_t* date[]={L"May 25, 2024",L"May 26, 2024",L"May 27, 2024"}; const wchar_t* st[]={L"Confirmed",L"Pending",L"Completed"};
    for(int i=0;i<3;i++){int y=175+i*50; Text(dc,275,y,id[i],TEXT,13); Text(dc,325,y,p[i],TEXT,13); Text(dc,480,y,d[i],TEXT,13); Text(dc,620,y,dep[i],TEXT,13); Text(dc,770,y,date[i],TEXT,13); Text(dc,880,y,st[i],i==1?RGB(190,125,20):RGB(35,145,85),13,true);}
}

void DrawQueue(HDC dc){ Text(dc,250,35,L"Doctor Queue",TEXT,26,true); Text(dc,250,72,L"Today's patient queue",MUTED,14); RECT r={250,110,1010,360}; Card(dc,r); Text(dc,275,135,L"Queue",MUTED,12); Text(dc,340,135,L"Patient",MUTED,12); Text(dc,540,135,L"Doctor",MUTED,12); Text(dc,700,135,L"Department",MUTED,12); Text(dc,860,135,L"Status",MUTED,12);
 const wchar_t* q[]={L"001",L"002",L"003"}; const wchar_t* p[]={L"Osama Bin Laden",L"Maria Reyes",L"Pedro Santos"}; const wchar_t* d[]={L"Dr. Josiah Joshua Perez",L"Dr. Garcia",L"Dr. Cruz"}; const wchar_t* dep[]={L"Radiology",L"Cardiology",L"Pediatrics"}; const wchar_t* s[]={L"Waiting",L"Consulting",L"Completed"}; for(int i=0;i<3;i++){int y=175+i*50;Text(dc,275,y,q[i],TEXT,13);Text(dc,340,y,p[i],TEXT,13);Text(dc,540,y,d[i],TEXT,13);Text(dc,700,y,dep[i],TEXT,13);Text(dc,860,y,s[i],i==0?RGB(190,125,20):RGB(35,145,85),13,true);}}

void DrawSimple(HWND w,HDC dc,const std::wstring& title,const std::wstring& sub){ Text(dc,250,35,title,TEXT,26,true); Text(dc,250,72,sub,MUTED,14); RECT r={250,110,1010,500}; Card(dc,r); Text(dc,280,145,L"Honda Clinic",BLUE,20,true); Text(dc,280,185,L"This section is ready for the C++ GUI workflow.",TEXT,15);}

void DrawContent(HWND w,HDC dc,const std::wstring& page){
    Fill(dc,{0,0,1100,700},LIGHT);
    if(page==L"Dashboard")DrawDashboard(w,dc); else if(page==L"Patients")DrawPatients(dc); else if(page==L"Appointments")DrawAppointments(dc); else if(page==L"Doctor Queue")DrawQueue(dc); else if(page==L"Consultations")DrawSimple(w,dc,L"Consultations",L"Record patient consultation details"); else if(page==L"Prescriptions")DrawSimple(w,dc,L"Prescriptions",L"Manage prescriptions"); else if(page==L"Billing & Accounting")DrawSimple(w,dc,L"Billing & Accounting",L"Manage invoices and payments"); else DrawSimple(w,dc,page,L"Honda Clinic management system");
}

void AddNav(HWND parent,const wchar_t* name,int y){ HWND b=CreateWindowW(L"BUTTON",name,WS_CHILD|WS_VISIBLE|BS_FLAT,25,y,190,42,parent,(HMENU)(100+y),g_hInst,nullptr); SendMessageW(b,WM_SETFONT,(WPARAM)g_font,TRUE); nav.push_back({b,name}); }

LRESULT CALLBACK WndProc(HWND w,UINT msg,WPARAM wp,LPARAM lp){
    switch(msg){
    case WM_CREATE:{
        g_font=CreateFontW(16,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
        RECT cr; GetClientRect(w,&cr); g_content=CreateWindowW(L"STATIC",L"",WS_CHILD|WS_VISIBLE,225,0,cr.right-225,cr.bottom,g_hInst,nullptr);
        AddNav(w,L"Dashboard",75);AddNav(w,L"Patients",125);AddNav(w,L"Appointments",175);AddNav(w,L"Doctor Queue",225);AddNav(w,L"Consultations",275);AddNav(w,L"Prescriptions",325);AddNav(w,L"Billing & Accounting",375);AddNav(w,L"My Profile",425);AddNav(w,L"Logout",500);
        return 0;}
    case WM_COMMAND:{ int id=LOWORD(wp); if(id>=175 && id<=600){ for(auto &b:nav) if(GetDlgCtrlID(b.h)==id){ SetPropW(g_content,L"page",(HANDLE)new std::wstring(b.page)); InvalidateRect(g_content,nullptr,TRUE); } } return 0;}
    case WM_PAINT:{ PAINTSTRUCT ps; HDC dc=BeginPaint(w,&ps); RECT r;GetClientRect(w,&r); Fill(dc,r,WHITE); Fill(dc,{0,0,225,r.bottom},WHITE); Text(dc,28,25,L"HONDA",BLUE,25,true); Text(dc,29,53,L"CLINIC",TEXT,18,true); Text(dc,28,610,L"Hospital Management",MUTED,12); EndPaint(w,&ps); return 0;}
    case WM_ERASEBKGND:return 1;
    case WM_DESTROY:PostQuitMessage(0);return 0; }
    if(msg==WM_CTLCOLORSTATIC){HDC dc=(HDC)wp; SetBkColor(dc,LIGHT); SetTextColor(dc,TEXT); return (LRESULT)CreateSolidBrush(LIGHT);} return DefWindowProcW(w,msg,wp,lp);
}

int WINAPI wWinMain(HINSTANCE h,HINSTANCE,LPWSTR,int n){
    g_hInst=h; WNDCLASSW wc{}; wc.lpfnWndProc=WndProc; wc.hInstance=h; wc.lpszClassName=CLASS_NAME; wc.hCursor=LoadCursor(nullptr,IDC_ARROW); wc.hbrBackground=CreateSolidBrush(WHITE); RegisterClassW(&wc);
    HWND w=CreateWindowExW(0,CLASS_NAME,L"Honda Clinic - Hospital Management System",WS_OVERLAPPEDWINDOW|WS_VISIBLE,100,60,1120,700,nullptr,nullptr,h,nullptr); if(!w)return 0;
    MSG m{}; while(GetMessageW(&m,nullptr,0,0)){TranslateMessage(&m);DispatchMessageW(&m);} return 0;
}
