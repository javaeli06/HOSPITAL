#define UNICODE
#define _UNICODE
#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>

#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using std::wstring;

// Honda Clinic - native Win32 C++ GUI. No database required.
static const COLORREF BLUE       = RGB(58, 101, 190);   // screenshot blue
static const COLORREF BLUE_DARK  = RGB(45, 84, 165);
static const COLORREF BG         = RGB(244, 247, 252);
static const COLORREF CARD       = RGB(255, 255, 255);
static const COLORREF TEXT       = RGB(35, 48, 70);
static const COLORREF MUTED      = RGB(105, 119, 142);
static const COLORREF BORDER     = RGB(222, 227, 235);
static const COLORREF WHITE      = RGB(255, 255, 255);

HWND g_main = nullptr;
HFONT g_font = nullptr, g_bold = nullptr, g_title = nullptr, g_small = nullptr;
std::vector<HWND> g_controls;
int g_role = 0; // 1 patient, 2 admin

// Page identifiers
enum Page { LOGIN, ROLE, ADMIN_DASH, PATIENTS, APPOINTMENTS, QUEUE, CONSULTATION, PRESCRIPTIONS, BILLING, PATIENT_DASH, PROFILE, MEDICAL, LOGOUT_PAGE };
Page g_page = LOGIN;

void DeleteFonts() {
    if (g_font) DeleteObject(g_font); if (g_bold) DeleteObject(g_bold);
    if (g_title) DeleteObject(g_title); if (g_small) DeleteObject(g_small);
    g_font = g_bold = g_title = g_small = nullptr;
}

void ClearControls() {
    for (HWND h : g_controls) if (IsWindow(h)) DestroyWindow(h);
    g_controls.clear();
}

HWND Add(const wchar_t* cls, const wchar_t* text, DWORD style, int x, int y, int w, int h, int id = 0) {
    HWND hWnd = CreateWindowExW(0, cls, text, style, x, y, w, h, g_main,
        (HMENU)(INT_PTR)id, GetModuleHandleW(nullptr), nullptr);
    if (hWnd) {
        SendMessageW(hWnd, WM_SETFONT, (WPARAM)g_font, TRUE);
        g_controls.push_back(hWnd);
    }
    return hWnd;
}

HWND Label(const wchar_t* text, int x, int y, int w, int h, int size = 14, bool bold = false) {
    HWND h = Add(L"STATIC", text, WS_CHILD | WS_VISIBLE, x, y, w, h);
    SendMessageW(h, WM_SETFONT, (WPARAM)(bold ? (size >= 18 ? g_title : g_bold) : (size <= 12 ? g_small : g_font)), TRUE);
    return h;
}

HWND Button(const wchar_t* text, int x, int y, int w, int h, int id) {
    return Add(L"BUTTON", text, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, x, y, w, h, id);
}

HWND EditBox(const wchar_t* cue, int x, int y, int w, int h, int id, bool password = false) {
    DWORD style = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL;
    if (password) style |= ES_PASSWORD;
    HWND e = Add(L"EDIT", L"", style, x, y, w, h, id);
    SendMessageW(e, EM_SETCUEBANNER, FALSE, (LPARAM)cue);
    return e;
}

HWND Check(const wchar_t* text, int x, int y, int w, int h, int id) {
    return Add(L"BUTTON", text, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, x, y, w, h, id);
}

void Fill(HDC dc, RECT r, COLORREF color) {
    HBRUSH b = CreateSolidBrush(color); FillRect(dc, &r, b); DeleteObject(b);
}

void DrawCard(HDC dc, int x, int y, int w, int h, bool shadow = false) {
    if (shadow) { RECT s{x+4,y+6,x+w+4,y+h+6}; Fill(dc,s,RGB(228,232,239)); }
    RECT r{x,y,x+w,y+h}; Fill(dc,r,CARD);
    HPEN p=CreatePen(PS_SOLID,1,BORDER); HGDIOBJ old=SelectObject(dc,p); SelectObject(dc,GetStockObject(HOLLOW_BRUSH)); Rectangle(dc,x,y,x+w,y+h); SelectObject(dc,old); DeleteObject(p);
}

void DrawHospitalIcon(HDC dc, int cx, int cy, int scale=1) {
    // Small medical icon similar to the original page; intentionally simple and native.
    HBRUSH pink=CreateSolidBrush(RGB(244,95,175)); HBRUSH light=CreateSolidBrush(RGB(218,218,225));
    RECT body{cx-22*scale,cy-15*scale,cx+22*scale,cy+25*scale}; Fill(dc,body,light);
    RECT top{cx-18*scale,cy-35*scale,cx+18*scale,cy-12*scale}; Fill(dc,top,light);
    RECT crossV{cx-3*scale,cy-31*scale,cx+3*scale,cy-17*scale}; Fill(dc,crossV,pink);
    RECT crossH{cx-9*scale,cy-27*scale,cx+9*scale,cy-21*scale}; Fill(dc,crossH,pink);
    RECT stripe{cx-18*scale,cy+4*scale,cx+18*scale,cy+9*scale}; Fill(dc,stripe,pink);
    DeleteObject(pink); DeleteObject(light);
}

void Text(HDC dc, const wchar_t* s, int x, int y, COLORREF color, HFONT font) {
    SetBkMode(dc, TRANSPARENT); SetTextColor(dc,color); HFONT old=(HFONT)SelectObject(dc,font);
    TextOutW(dc,x,y,s,(int)wcslen(s)); SelectObject(dc,old);
}

void CenterText(HDC dc, const wchar_t* s, int y, COLORREF color, HFONT font, int left, int right) {
    RECT r{left,y,right,y+50}; SetBkMode(dc,TRANSPARENT); SetTextColor(dc,color); HFONT old=(HFONT)SelectObject(dc,font);
    DrawTextW(dc,s,-1,&r,DT_CENTER|DT_VCENTER|DT_SINGLELINE); SelectObject(dc,old);
}

void DrawTable(HDC dc, int x, int y, int w, int rowH, const std::vector<std::vector<wstring>>& rows) {
    if (rows.empty()) return; int cols=(int)rows[0].size(); if (!cols) return; int cw=w/cols;
    for (size_t r=0;r<rows.size();++r) {
        RECT rr{x,y+(int)r*rowH,x+w,y+(int)(r+1)*rowH};
        Fill(dc,rr,r==0?RGB(239,244,251):CARD);
        for (int c=0;c<cols;++c) {
            RECT cell{x+c*cw+8,y+(int)r*rowH+5,x+(c+1)*cw-5,y+(int)(r+1)*rowH-5};
            SetBkMode(dc,TRANSPARENT); SetTextColor(dc,r==0?TEXT:MUTED); HFONT old=(HFONT)SelectObject(dc,g_small);
            DrawTextW(dc,rows[r][c].c_str(),-1,&cell,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS); SelectObject(dc,old);
        }
    }
    HPEN p=CreatePen(PS_SOLID,1,BORDER); HGDIOBJ old=SelectObject(dc,p); SelectObject(dc,GetStockObject(HOLLOW_BRUSH)); Rectangle(dc,x,y,x+w,y+(int)rows.size()*rowH); SelectObject(dc,old); DeleteObject(p);
}

void DrawSidebar(HDC dc, const wchar_t* active, bool patient=false) {
    RECT r{0,0,230,1000}; Fill(dc,r,BLUE_DARK);
    Text(dc,L"HONDA CLINIC",25,24,WHITE,g_bold);
    Text(dc,patient?L"Patient Portal":L"Admin Management",25,48,RGB(220,230,250),g_small);
    const wchar_t* adminItems[]={L"Dashboard",L"Patients",L"Appointments",L"Doctor Queue",L"Consultations",L"Prescriptions",L"Billing & Accounting",L"Logout"};
    const wchar_t* patientItems[]={L"Dashboard",L"My Profile",L"Medical Information",L"My History",L"Appointments",L"Consultations",L"Prescriptions",L"Billing",L"Logout"};
    const wchar_t** items=patient?patientItems:adminItems; int count=patient?9:8;
    for(int i=0;i<count;++i){ int yy=105+i*45; bool a=wcscmp(active,items[i])==0;
        if(a){RECT ar{12,yy-7,218,yy+31};Fill(dc,ar,BLUE);}
        Text(dc,items[i],38,yy,WHITE,g_small);
    }
}

void Header(HDC dc,const wchar_t* title,const wchar_t* sub=L"") {
    Text(dc,title,255,28,TEXT,g_title); if(*sub) Text(dc,sub,257,60,MUTED,g_small);
}

void Navigate(Page p) {
    g_page=p; ClearControls(); InvalidateRect(g_main,nullptr,TRUE);
    // Controls are created in response to the new page below.
    RECT cr{}; GetClientRect(g_main,&cr); int split=cr.right/2; int cx=split+220;
    if(p==LOGIN){
        EditBox(L"Enter your email",cx,cr.bottom/2-35,420,45,101);
        EditBox(L"Enter your password",cx,cr.bottom/2+45,420,45,102,true);
        Button(L"Login",cx,cr.bottom/2+125,420,48,103);
    } else if(p==ROLE){
        Check(L"I am a Patient",cx,cr.bottom/2+10,210,35,201);
        Check(L"I am an Admin",cx,cr.bottom/2+55,210,35,202);
        Button(L"Submit",cx,cr.bottom/2+105,100,40,203);
    } else if(p==PATIENTS){ Button(L"+ Add Patient",1050,32,125,36,401); }
    else if(p==APPOINTMENTS){ Button(L"+ Book Appointment",1010,32,165,36,402); }
    else if(p==QUEUE){ Button(L"Refresh Queue",1040,32,135,36,403); }
    else if(p==CONSULTATION){ Button(L"Save Consultation",965,600,200,42,404); EditBox(L"Enter diagnosis...",280,315,850,55,501); EditBox(L"Enter treatment plan...",280,410,850,75,502); }
    else if(p==PRESCRIPTIONS){ Button(L"+ Add Prescription",1020,32,155,36,405); }
    else if(p==BILLING){ Button(L"+ Create Invoice",1020,32,155,36,406); }
    else if(p==PROFILE){ Button(L"Edit Profile",1050,32,125,36,407); }
    else if(p==MEDICAL){ Button(L"Edit",1050,32,125,36,408); }
    InvalidateRect(g_main,nullptr,TRUE);
}

void DrawLogin(HDC dc, RECT rc) {
    Fill(dc,rc,BG); int split=rc.right/2;
    RECT left{0,0,split,rc.bottom}; Fill(dc,left,BLUE);
    DrawHospitalIcon(dc,split/2,rc.bottom/2-95,1);
    CenterText(dc,L"Honda Clinic Admin Management System",rc.bottom/2-45,WHITE,g_title,25,split-25);
    DrawCard(dc,split+170,rc.bottom/2-210,540,390,true);
    Text(dc,L"Welcome Back",split+220,rc.bottom/2-155,BLUE,g_title);
    Text(dc,L"Please login to continue.",split+220,rc.bottom/2-110,MUTED,g_font);
    Text(dc,L"Email",split+220,rc.bottom/2-55,TEXT,g_bold);
    Text(dc,L"Password",split+220,rc.bottom/2+25,TEXT,g_bold);
}

void DrawRole(HDC dc, RECT rc) {
    Fill(dc,rc,BG); int split=rc.right/2;
    RECT left{0,0,split,rc.bottom}; Fill(dc,left,BLUE);
    DrawHospitalIcon(dc,split/2,rc.bottom/2-95,1);
    CenterText(dc,L"Honda Clinic Admin Management System",rc.bottom/2-45,WHITE,g_title,25,split-25);
    DrawCard(dc,split+170,rc.bottom/2-210,540,390,true);
    Text(dc,L"Welcome Back",split+220,rc.bottom/2-155,BLUE,g_title);
    Text(dc,L"Are you a Patient or Admin?",split+220,rc.bottom/2-110,MUTED,g_font);
}

void DrawAdminDashboard(HDC dc) {
    DrawSidebar(dc,L"Dashboard"); Header(dc,L"Admin Dashboard",L"Welcome back, Admin Perez!");
    const wchar_t* names[]={L"Total Patients",L"Doctors",L"Appointments",L"Revenue"}; const wchar_t* vals[]={L"69",L"15",L"32",L"₱89,000"};
    for(int i=0;i<4;++i){int x=255+i*225;DrawCard(dc,x,100,i==3?245:205,92);Text(dc,names[i],x+20,120,MUTED,g_small);Text(dc,vals[i],x+20,148,(i==3?RGB(25,150,95):BLUE),g_title);}
    DrawCard(dc,255,220,650,380); DrawCard(dc,930,220,245,380);
    Text(dc,L"Today's Appointments",275,245,TEXT,g_bold);
    DrawTable(dc,275,280,610,190,{{L"Patient",L"Doctor",L"Time",L"Status"},{L"Japeth Canonse",L"Dr. Jayson Lagera",L"9:00 AM",L"Confirmed"},{L"Mark Natural",L"Dr. Kent Gutierez",L"10:30 AM",L"Waiting"},{L"Ishmael Catindig",L"Dr. Eduard Javier",L"1:00 PM",L"Confirmed"}});
    Text(dc,L"Doctor Queue",950,245,TEXT,g_bold); Text(dc,L"Dr. Eduard Javier",950,300,TEXT,g_bold); Text(dc,L"5 Patients Waiting",950,325,MUTED,g_small); Text(dc,L"Dr. Kent Gutierez",950,375,TEXT,g_bold); Text(dc,L"3 Patients Waiting",950,400,MUTED,g_small); Text(dc,L"Dr. Jayson Lagera",950,450,TEXT,g_bold); Text(dc,L"3 Patients Waiting",950,475,MUTED,g_small);
}

void DrawPatients(HDC dc){ DrawSidebar(dc,L"Patients"); Header(dc,L"Patients",L"Manage patient records"); DrawCard(dc,255,100,920,520); DrawTable(dc,275,130,880,58,{{L"ID",L"Name",L"Age",L"Sex",L"Contact",L"Actions"},{L"P001",L"John Doe",L"22",L"Male",L"0912-345-6789",L"View   Delete"},{L"P002",L"Maria Cruz",L"25",L"Female",L"0917-234-5678",L"View   Delete"},{L"P003",L"Michael Garcia",L"30",L"Male",L"0920-456-7890",L"View   Delete"},{L"P004",L"Angela Santos",L"28",L"Female",L"0918-567-8901",L"View   Delete"},{L"P005",L"Daniel Reyes",L"35",L"Male",L"0921-678-9012",L"View   Delete"},{L"P006",L"Sofia Mendoza",L"24",L"Female",L"0919-789-0123",L"View   Delete"}}); }
void DrawAppointments(HDC dc){ DrawSidebar(dc,L"Appointments"); Header(dc,L"Appointments"); DrawCard(dc,255,100,920,400); DrawTable(dc,275,130,880,68,{{L"ID",L"Patient",L"Doctor",L"Department",L"Date & Time",L"Status"},{L"A001",L"Juan Dela Cruz",L"Dr. Santos",L"General Medicine",L"May 25, 2024  9:00 AM",L"Confirmed"},{L"A002",L"Maria Reyes",L"Dr. Cruz",L"Pediatrics",L"May 26, 2024  10:30 AM",L"Pending"},{L"A003",L"Pedro Santos",L"Dr. Garcia",L"Cardiology",L"May 27, 2024  1:00 PM",L"Completed"}}); }
void DrawQueue(HDC dc){ DrawSidebar(dc,L"Doctor Queue"); Header(dc,L"Doctor Queue"); DrawCard(dc,255,100,920,350); DrawTable(dc,275,130,880,68,{{L"Queue No.",L"Patient",L"Doctor",L"Department",L"Time",L"Status"},{L"001",L"Osama Bin Laden",L"Dr. Josiah Joshua Perez",L"Radiology",L"9:00 AM",L"Waiting"},{L"002",L"Maria Reyes",L"Dr. Garcia",L"Cardiology",L"9:15 AM",L"Consulting"},{L"003",L"Pedro Santos",L"Dr. Cruz",L"Pediatrics",L"9:30 AM",L"Completed"}}); }
void DrawConsultation(HDC dc){ DrawSidebar(dc,L"Consultations"); Header(dc,L"Consultation"); DrawCard(dc,255,100,920,550); Text(dc,L"Patient Name",280,130,MUTED,g_small); Text(dc,L"Juan Dela Cruz",280,155,TEXT,g_font); Text(dc,L"Doctor",650,130,MUTED,g_small); Text(dc,L"Dr. Santos",650,155,TEXT,g_font); Text(dc,L"Date",280,210,MUTED,g_small); Text(dc,L"May 25, 2024",280,235,TEXT,g_font); Text(dc,L"Diagnosis",280,285,TEXT,g_bold); Text(dc,L"Treatment Plan",280,380,TEXT,g_bold); }
void DrawPrescriptions(HDC dc){ DrawSidebar(dc,L"Prescriptions"); Header(dc,L"Prescriptions"); DrawCard(dc,255,100,920,430); DrawTable(dc,275,130,880,68,{{L"ID",L"Patient",L"Medicine",L"Dosage",L"Date"},{L"PR001",L"Juan Dela Cruz",L"Amoxicillin",L"500mg",L"May 25, 2024"},{L"PR002",L"Maria Reyes",L"Paracetamol",L"500mg",L"May 26, 2024"},{L"PR003",L"Pedro Santos",L"Ibuprofen",L"400mg",L"May 27, 2024"}}); }
void DrawBilling(HDC dc){ DrawSidebar(dc,L"Billing & Accounting"); Header(dc,L"Billing & Accounting"); DrawCard(dc,255,100,920,500); DrawTable(dc,275,130,880,65,{{L"Invoice",L"Patient",L"Service",L"Amount",L"Date",L"Status"},{L"INV001",L"Juan Dela Cruz",L"Consultation",L"₱500.00",L"May 25, 2024",L"Paid"},{L"INV002",L"Maria Reyes",L"Laboratory",L"₱1,200.00",L"May 26, 2024",L"Pending"},{L"INV003",L"Pedro Santos",L"X-Ray",L"₱900.00",L"May 27, 2024",L"Paid"}}); Text(dc,L"Today's Revenue",280,375,MUTED,g_small); Text(dc,L"₱18,450",280,400,RGB(25,150,95),g_title); Text(dc,L"Pending Payments",500,375,MUTED,g_small); Text(dc,L"₱5,200",500,400,RGB(230,130,20),g_title); Text(dc,L"Total Transactions",720,375,MUTED,g_small); Text(dc,L"34",720,400,BLUE,g_title); }
void DrawPatientDash(HDC dc){ DrawSidebar(dc,L"Dashboard",true); Header(dc,L"Patient Dashboard",L"Welcome back, Juan Dela Cruz!"); const wchar_t* n[]={L"My Appointments",L"My Consultations",L"My Prescriptions",L"My Billing"}; const wchar_t* v[]={L"1",L"4",L"2",L"₱2,500"}; for(int i=0;i<4;++i){int x=255+i*225;DrawCard(dc,x,100,i==3?245:205,92);Text(dc,n[i],x+20,120,MUTED,g_small);Text(dc,v[i],x+20,150,i==3?RGB(25,150,95):BLUE,g_title);} DrawCard(dc,255,220,920,270); Text(dc,L"Upcoming Appointment",280,245,TEXT,g_bold); DrawTable(dc,280,280,850,90,{{L"Date",L"Doctor",L"Time",L"Status"},{L"May 25, 2024",L"Dr. Santos",L"9:00 AM",L"Confirmed"}}); }
void DrawProfile(HDC dc){ DrawSidebar(dc,L"My Profile",true); Header(dc,L"My Profile"); DrawCard(dc,255,100,920,350); Text(dc,L"Juan Dela Cruz",300,145,TEXT,g_title); Text(dc,L"Patient  •  P001",300,180,MUTED,g_small); Text(dc,L"Age",300,230,MUTED,g_small); Text(dc,L"22",300,255,TEXT,g_font); Text(dc,L"Sex",480,230,MUTED,g_small); Text(dc,L"Male",480,255,TEXT,g_font); Text(dc,L"Contact",650,230,MUTED,g_small); Text(dc,L"0912-345-6789",650,255,TEXT,g_font); Text(dc,L"Address",300,300,MUTED,g_small); Text(dc,L"123 Rizal St., Manila",300,325,TEXT,g_font); }
void DrawMedical(HDC dc){ DrawSidebar(dc,L"Medical Information",true); Header(dc,L"Medical Information"); DrawCard(dc,255,100,920,430); Text(dc,L"Patient ID",285,140,MUTED,g_small); Text(dc,L"P001",285,165,TEXT,g_font); Text(dc,L"Blood Type",500,140,MUTED,g_small); Text(dc,L"O+",500,165,TEXT,g_font); Text(dc,L"Allergies",715,140,MUTED,g_small); Text(dc,L"None",715,165,TEXT,g_font); Text(dc,L"Medical History",285,235,TEXT,g_bold); Text(dc,L"No known medical history.",285,275,MUTED,g_font); }
void DrawLogout(HDC dc,RECT rc){ Fill(dc,rc,BG); int split=rc.right/2; RECT left{0,0,split,rc.bottom}; Fill(dc,left,BLUE); DrawHospitalIcon(dc,split/2,rc.bottom/2-70,1); CenterText(dc,L"Honda Clinic Admin Management System",rc.bottom/2-20,WHITE,g_bold,20,split-20); DrawCard(dc,split+170,rc.bottom/2-150,540,300,true); CenterText(dc,L"You have been logged out.",rc.bottom/2-70,TEXT,g_bold,split+190,rc.right-30); CenterText(dc,L"Thank you for using Honda Clinic!",rc.bottom/2-25,MUTED,g_small,split+190,rc.right-30); }

void DrawPage(HDC dc, RECT rc) {
    if(g_page==LOGIN){DrawLogin(dc,rc);return;} if(g_page==ROLE){DrawRole(dc,rc);return;} if(g_page==LOGOUT_PAGE){DrawLogout(dc,rc);return;}
    switch(g_page){case ADMIN_DASH:DrawAdminDashboard(dc);break;case PATIENTS:DrawPatients(dc);break;case APPOINTMENTS:DrawAppointments(dc);break;case QUEUE:DrawQueue(dc);break;case CONSULTATION:DrawConsultation(dc);break;case PRESCRIPTIONS:DrawPrescriptions(dc);break;case BILLING:DrawBilling(dc);break;case PATIENT_DASH:DrawPatientDash(dc);break;case PROFILE:DrawProfile(dc);break;case MEDICAL:DrawMedical(dc);break;default:break;}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch(msg){
    case WM_CREATE:
        g_main=hwnd;
        g_font=CreateFontW(-16,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH|FF_SWISS,L"Segoe UI");
        g_bold=CreateFontW(-16,0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH|FF_SWISS,L"Segoe UI");
        g_title=CreateFontW(-23,0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH|FF_SWISS,L"Segoe UI");
        g_small=CreateFontW(-13,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH|FF_SWISS,L"Segoe UI");
        Navigate(LOGIN); return 0;
    case WM_SIZE: InvalidateRect(hwnd,nullptr,TRUE); return 0;
    case WM_PAINT:{PAINTSTRUCT ps;HDC dc=BeginPaint(hwnd,&ps);RECT rc;GetClientRect(hwnd,&rc);DrawPage(dc,rc);EndPaint(hwnd,&ps);return 0;}
    case WM_COMMAND:{
        int id=LOWORD(wp);
        if(id==103){Navigate(ROLE);return 0;}
        if(id==201){g_role=1;return 0;}
        if(id==202){g_role=2;return 0;}
        if(id==203){
            HWND p=GetDlgItem(hwnd,201), a=GetDlgItem(hwnd,202);
            bool patient=p && (SendMessageW(p,BM_GETCHECK,0,0)==BST_CHECKED); bool admin=a && (SendMessageW(a,BM_GETCHECK,0,0)==BST_CHECKED);
            if(patient==admin){MessageBoxW(hwnd,L"Please select Patient or Admin.",L"Honda Clinic",MB_OK|MB_ICONWARNING);return 0;}
            Navigate(patient?PATIENT_DASH:ADMIN_DASH); return 0;
        }
        if(id>=301&&id<=309){
            // reserved for future sidebar hit areas
            return 0;
        }
        if(id==401){MessageBoxW(hwnd,L"Add Patient form is ready. Database connection can be added later.",L"Honda Clinic",MB_OK|MB_ICONINFORMATION);return 0;}
        if(id==402){MessageBoxW(hwnd,L"Book Appointment form is ready. Database connection can be added later.",L"Honda Clinic",MB_OK|MB_ICONINFORMATION);return 0;}
        if(id==403||id==405||id==406||id==407||id==408){MessageBoxW(hwnd,L"This function is ready for the next database-enabled version.",L"Honda Clinic",MB_OK|MB_ICONINFORMATION);return 0;}
        if(id==404){MessageBoxW(hwnd,L"Consultation saved successfully.\n\nData is temporary because no database is connected yet.",L"Honda Clinic",MB_OK|MB_ICONINFORMATION);return 0;}
        return 0;
    }
    case WM_LBUTTONDOWN:{
        if(g_page!=LOGIN && g_page!=ROLE && g_page!=LOGOUT_PAGE){
            int x=LOWORD(lp), y=HIWORD(lp);
            if(x<230 && y>=90){
                int idx=(y-90)/45;
                if(g_role==1){
                    switch(idx){case 0:Navigate(PATIENT_DASH);break;case 1:Navigate(PROFILE);break;case 2:Navigate(MEDICAL);break;case 3:MessageBoxW(hwnd,L"History page is ready for the next version.",L"Honda Clinic",MB_OK|MB_ICONINFORMATION);break;case 4:Navigate(APPOINTMENTS);break;case 5:Navigate(CONSULTATION);break;case 6:Navigate(PRESCRIPTIONS);break;case 7:Navigate(BILLING);break;case 8:Navigate(LOGOUT_PAGE);break;}
                } else {
                    switch(idx){case 0:Navigate(ADMIN_DASH);break;case 1:Navigate(PATIENTS);break;case 2:Navigate(APPOINTMENTS);break;case 3:Navigate(QUEUE);break;case 4:Navigate(CONSULTATION);break;case 5:Navigate(PRESCRIPTIONS);break;case 6:Navigate(BILLING);break;case 7:Navigate(LOGOUT_PAGE);break;}
                }
                return 0;
            }
        }
        return 0;
    }
    case WM_DESTROY: ClearControls(); DeleteFonts(); PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(hwnd,msg,wp,lp);
}

int WINAPI wWinMain(HINSTANCE hInst,HINSTANCE,LPWSTR,int nCmdShow){
    WNDCLASSW wc{}; wc.hInstance=hInst; wc.lpfnWndProc=WndProc; wc.lpszClassName=L"HondaClinicWindow"; wc.hCursor=LoadCursor(nullptr,IDC_ARROW); wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
    RegisterClassW(&wc);
    HWND hwnd=CreateWindowExW(0,wc.lpszClassName,L"Honda Clinic - Hospital Clinic System",WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,CW_USEDEFAULT,CW_USEDEFAULT,1400,850,nullptr,nullptr,hInst,nullptr);
    if(!hwnd)return 0; ShowWindow(hwnd,nCmdShow); UpdateWindow(hwnd);
    MSG msg; while(GetMessageW(&msg,nullptr,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);} return (int)msg.wParam;
}
