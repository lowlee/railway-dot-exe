// InterfaceUnit.cpp
/*
   BEWARE OF COMMENTS in .cpp files:  they were accurate when written but have
   sometimes been overtaken by changes and not updated
   Comments in .h files are believed to be accurate and up to date

   This is a source code file for "railway.exe", a railway operation
   simulator, written originally in Borland C++ Builder 4 Professional with
   later updates in Embarcadero C++Builder 10.2.
   Copyright (C) 2010 Albert Ball [original development]

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
// ---------------------------------------------------------------------------

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
#include <Graphics.hpp>
#include <ComCtrls.hpp>
#include <fstream>
#include <vector>
#include <vcl.h>
#include <stdio.h>
#include <algorithm>  //for sort

#pragma hdrstop
// The above batch of include files above #pragma hdrstop appear in all .cpp files.
// They aren't all needed in each case but being together and identical they speed
// up compilation considerably (without ~14 min, with ~1 min!). They are used in
// conjunction with 'use pre-compiled headers' in the project compiler options.

#include "InterfaceUnit.h"
#include "GraphicUnit.h"
#include "DisplayUnit.h"
#include "TextUnit.h"
#include "TrainUnit.h"
#include "Utilities.h"
#include "TrackUnit.h"
#include "AboutUnit.h"
#include <fstream>
#include <dirent.h>
#include <Filectrl.hpp> //to check whether directories exist

// ---------------------------------------------------------------------------
#include <Vcl.HTMLHelpViewer.hpp>   //added at v2.0.0 for access to the .chm help file
#pragma package(smart_init)
#pragma link "Vcl.HTMLHelpViewer"   //added at v2.0.0 for access to the .chm help file
#pragma resource "*.dfm"

TInterface *Interface;

// Folder Names
const UnicodeString TInterface::RAILWAY_DIR_NAME = "Railways";
const UnicodeString TInterface::TIMETABLE_DIR_NAME = "Program timetables";
const UnicodeString TInterface::PERFLOG_DIR_NAME = "Performance logs";
const UnicodeString TInterface::SESSION_DIR_NAME = "Sessions";
const UnicodeString TInterface::IMAGE_DIR_NAME = "Images";
const UnicodeString TInterface::FORMATTEDTT_DIR_NAME = "Formatted timetables";
const UnicodeString TInterface::USERGRAPHICS_DIR_NAME = "Graphics";

// ---------------------------------------------------------------------------

__fastcall TInterface::TInterface(TComponent* Owner): TForm(Owner)
{ // constructor
    try
    {
        Screen->Cursor = TCursor(-11); // Hourglass;
        DirOpenError = false;
        AllSetUpFlag = false; // flag to prevent MasterClock from being enabled when application activates if there has been an error during
                                     // initial setup
        // MasterClock->Enabled = false;//keep this stopped until all set up (no effect here as form not yet created, made false in object insp)
        // Visible = false; //keep the Interface form invisible until all set up (no effect here as form not yet created, made false in object insp)
        ProgramVersion = GetVersion();
        // use GNU Major/Minor/Patch version numbering system, change for each published modification, Dev x = interim internal
        // development stages (don't show on published versions)

        // check for presence of directories, creation failure probably indicates that the
        // working folder is read-only
        CurDir = GetCurrentDir();
        if(!DirectoryExists(RAILWAY_DIR_NAME))
        {
            if(!CreateDir(RAILWAY_DIR_NAME))
            {
                DirOpenError = true;
            }
        }
        if(!DirectoryExists(TIMETABLE_DIR_NAME))
        {
            if(!CreateDir(TIMETABLE_DIR_NAME))
            {
                DirOpenError = true;
            }
        }
        if(!DirectoryExists(PERFLOG_DIR_NAME))
        {
            if(!CreateDir(PERFLOG_DIR_NAME))
            {
                DirOpenError = true;
            }
        }
        if(!DirectoryExists(SESSION_DIR_NAME))
        {
            if(!CreateDir(SESSION_DIR_NAME))
            {
                DirOpenError = true;
            }
        }
        if(!DirectoryExists(IMAGE_DIR_NAME))
        {
            if(!CreateDir(IMAGE_DIR_NAME))
            {
                DirOpenError = true;
            }
        }
        if(!DirectoryExists(FORMATTEDTT_DIR_NAME))
        {
            if(!CreateDir(FORMATTEDTT_DIR_NAME))
            {
                DirOpenError = true;
            }
        }
        if(!DirectoryExists(USERGRAPHICS_DIR_NAME))
        {
            if(!CreateDir(USERGRAPHICS_DIR_NAME))
            {
                DirOpenError = true;
            }
        }
        if(DirOpenError)
        {
            ShowMessage("Failed to create one or more of folders: " + RAILWAY_DIR_NAME + ", " + TIMETABLE_DIR_NAME + ", " + PERFLOG_DIR_NAME + ", " +
                SESSION_DIR_NAME + ", " + IMAGE_DIR_NAME + ", " + FORMATTEDTT_DIR_NAME + ", " + USERGRAPHICS_DIR_NAME + ", " +
                "program operation may be restricted");
        }
// ShowMessage("NOTE: APPDEACTIVATE etc Disabled in FormCreate");
        SaveRailwayDialog->InitialDir = CurDir + "\\" + RAILWAY_DIR_NAME;
        LoadRailwayDialog->InitialDir = CurDir + "\\" + RAILWAY_DIR_NAME;
        TimetableDialog->InitialDir = CurDir + "\\" + TIMETABLE_DIR_NAME;
        SaveTTDialog->InitialDir = CurDir + "\\" + TIMETABLE_DIR_NAME;
        LoadSessionDialog->InitialDir = CurDir + "\\" + SESSION_DIR_NAME;
        LoadUserGraphicDialog->InitialDir = CurDir + "\\" + USERGRAPHICS_DIR_NAME;

        Application->HelpFile = AnsiString(CurDir + "\\Help.chm"); // added at v2.0.0 for .chm help file

        MainMenu1->AutoHotkeys = maManual; // Embarcadero mod: to suppress '&' inclusion for underlined characters in menu items
        PopupMenu->AutoHotkeys = maManual; // as above

        Utilities = new TUtilities;
        RailGraphics = new TRailGraphics();

        int DispW = (Screen->Width - 64) / 16; // will truncate down to a multiple of 16 OK here as screen dimensions are accurate
        int DispH = (Screen->Height - 192) / 16; // Interface dimensions are 16 too wide & 14 short in height
        MainScreen->Width = DispW * 16;
        MainScreen->Height = DispH * 16;

        Display = new TDisplay(MainScreen, PerformanceLogBox, OutputLog1, OutputLog2, OutputLog3, OutputLog4, OutputLog5, OutputLog6, OutputLog7, OutputLog8,
            OutputLog9, OutputLog10);
        Utilities->ScreenElementWidth = DispW;
        Utilities->ScreenElementHeight = DispH;
        HiddenScreen = new TImage(Interface);
        HiddenScreen->Width = MainScreen->Width;
        HiddenScreen->Height = MainScreen->Height;
        HiddenDisplay = new TDisplay(HiddenScreen, PerformanceLogBox, OutputLog1, OutputLog2, OutputLog3, OutputLog4, OutputLog5, OutputLog6, OutputLog7,
            OutputLog8, OutputLog9, OutputLog10);
        TextHandler = new TTextHandler;
        Track = new TTrack;
        AllRoutes = new TAllRoutes;
        ConstructPrefDir = new TOnePrefDir;
        ConstructRoute = new TOneRoute;
        EveryPrefDir = new TOnePrefDir;
        TrainController = new TTrainController;
        SelectBitmap = new Graphics::TBitmap;
        SelectBitmap->PixelFormat = pf8bit;
        SelectBitmap->Transparent = true;
        PointFlash = new TGraphicElement;
        AutoRouteStartMarker = new TGraphicElement;
        SigRouteStartMarker = new TGraphicElement;
        NonSigRouteStartMarker = new TGraphicElement;

        TrackInfoOnOffMenuItem->Caption = "Show"; // added here at v1.2.0 because dropped from ResetAll()
        TrainStatusInfoOnOffMenuItem->Caption = "Hide Status"; // changed at v2.0.0 so normally visible
        TrainTTInfoOnOffMenuItem->Caption = "Hide Timetable"; // as above
        ResetAll(0);

        TempTTFileName = "";

        PointFlash->LoadOverlayGraphic(3, RailGraphics->bmGreenRect);
        AutoRouteStartMarker->LoadOverlayGraphic(4, RailGraphics->bmLightBlueRect);
        SigRouteStartMarker->LoadOverlayGraphic(5, RailGraphics->bmGreenRect);
        NonSigRouteStartMarker->LoadOverlayGraphic(6, RailGraphics->bmRedRect);

        RouteFlashDuration = 0.0;
        PointsFlashDuration = 0.0;

        FloatingLabel->Color = clB4G5R5;
        TrackElementPanel->Color = clB5G5R4;
        InfoPanel->Color = clB4G5R5;

        SpeedButton1->Glyph->LoadFromResourceName(0, "gl1");
        SpeedButton2->Glyph->LoadFromResourceName(0, "gl2");
        SpeedButton3->Glyph->LoadFromResourceName(0, "gl3");
        SpeedButton4->Glyph->LoadFromResourceName(0, "gl4");
        SpeedButton5->Glyph->LoadFromResourceName(0, "gl5");
        SpeedButton6->Glyph->LoadFromResourceName(0, "gl6");
        SpeedButton7->Glyph->LoadFromResourceName(0, "gl7");
        SpeedButton8->Glyph->LoadFromResourceName(0, "gl8");
        SpeedButton9->Glyph->LoadFromResourceName(0, "gl9");
        SpeedButton10->Glyph->LoadFromResourceName(0, "gl10");
        SpeedButton11->Glyph->LoadFromResourceName(0, "gl11");
        SpeedButton12->Glyph->LoadFromResourceName(0, "gl12");
        SpeedButton13->Glyph->LoadFromResourceName(0, "gl13");
        SpeedButton14->Glyph->LoadFromResourceName(0, "gl14");
        SpeedButton15->Glyph->LoadFromResourceName(0, "gl15");
        SpeedButton16->Glyph->LoadFromResourceName(0, "gl16");
        SpeedButton18->Glyph->LoadFromResourceName(0, "gl18");
        SpeedButton19->Glyph->LoadFromResourceName(0, "gl19");
        SpeedButton20->Glyph->LoadFromResourceName(0, "gl20");
        SpeedButton21->Glyph->LoadFromResourceName(0, "gl21");
        SpeedButton22->Glyph->LoadFromResourceName(0, "gl22");
        SpeedButton23->Glyph->LoadFromResourceName(0, "gl23");
        SpeedButton24->Glyph->LoadFromResourceName(0, "gl24");
        SpeedButton25->Glyph->LoadFromResourceName(0, "gl25");
        SpeedButton26->Glyph->LoadFromResourceName(0, "gl26");
        SpeedButton27->Glyph->LoadFromResourceName(0, "gl27");
        SpeedButton28->Glyph->LoadFromResourceName(0, "gl28");
        SpeedButton29->Glyph->LoadFromResourceName(0, "gl29");
        SpeedButton30->Glyph->LoadFromResourceName(0, "gl30");
        SpeedButton31->Glyph->LoadFromResourceName(0, "gl31");
        SpeedButton32->Glyph->LoadFromResourceName(0, "gl32");
        SpeedButton33->Glyph->LoadFromResourceName(0, "gl33");
        SpeedButton34->Glyph->LoadFromResourceName(0, "gl34");
        SpeedButton35->Glyph->LoadFromResourceName(0, "gl35");
        SpeedButton36->Glyph->LoadFromResourceName(0, "gl36");
        SpeedButton37->Glyph->LoadFromResourceName(0, "gl37");
        SpeedButton38->Glyph->LoadFromResourceName(0, "gl38");
        SpeedButton39->Glyph->LoadFromResourceName(0, "gl39");
        SpeedButton40->Glyph->LoadFromResourceName(0, "gl40");
        SpeedButton41->Glyph->LoadFromResourceName(0, "gl41");
        SpeedButton42->Glyph->LoadFromResourceName(0, "gl42");
        SpeedButton43->Glyph->LoadFromResourceName(0, "gl43");
        SpeedButton44->Glyph->LoadFromResourceName(0, "gl44");
        SpeedButton45->Glyph->LoadFromResourceName(0, "gl45");
        SpeedButton46->Glyph->LoadFromResourceName(0, "gl46");
        SpeedButton47->Glyph->LoadFromResourceName(0, "gl47");
        SpeedButton48->Glyph->LoadFromResourceName(0, "gl48");
        SpeedButton49->Glyph->LoadFromResourceName(0, "gl49");
        SpeedButton50->Glyph->LoadFromResourceName(0, "gl50");
        SpeedButton51->Glyph->LoadFromResourceName(0, "gl51");
        SpeedButton52->Glyph->LoadFromResourceName(0, "gl52");
        SpeedButton53->Glyph->LoadFromResourceName(0, "gl53");
        SpeedButton54->Glyph->LoadFromResourceName(0, "gl54");
        SpeedButton55->Glyph->LoadFromResourceName(0, "gl55");
        SpeedButton56->Glyph->LoadFromResourceName(0, "gl56");
        SpeedButton57->Glyph->LoadFromResourceName(0, "gl57");
        SpeedButton58->Glyph->LoadFromResourceName(0, "gl58");
        SpeedButton59->Glyph->LoadFromResourceName(0, "gl59");
        SpeedButton60->Glyph->LoadFromResourceName(0, "gl60");
        SpeedButton61->Glyph->LoadFromResourceName(0, "gl61");
        SpeedButton62->Glyph->LoadFromResourceName(0, "gl62");
        SpeedButton63->Glyph->LoadFromResourceName(0, "gl63");
        SpeedButton64->Glyph->LoadFromResourceName(0, "gl64");
        SpeedButton65->Glyph->LoadFromResourceName(0, "gl65");
        SpeedButton66->Glyph->LoadFromResourceName(0, "gl66");
        SpeedButton67->Glyph->LoadFromResourceName(0, "gl67");
        SpeedButton68->Glyph->LoadFromResourceName(0, "gl68");
        SpeedButton69->Glyph->LoadFromResourceName(0, "gl69");
        SpeedButton70->Glyph->LoadFromResourceName(0, "gl70");
        SpeedButton71->Glyph->LoadFromResourceName(0, "gl71");
        SpeedButton72->Glyph->LoadFromResourceName(0, "gl72");
        SpeedButton73->Glyph->LoadFromResourceName(0, "gl73");
        SpeedButton74->Glyph->LoadFromResourceName(0, "gl74");
        SpeedButton75->Glyph->LoadFromResourceName(0, "gl75");
        SpeedButton76->Glyph->LoadFromResourceName(0, "gl76");
        SpeedButton77->Glyph->LoadFromResourceName(0, "gl77");
        SpeedButton78->Glyph->LoadFromResourceName(0, "gl78");
        SpeedButton79->Glyph->LoadFromResourceName(0, "gl79");
        SpeedButton80->Glyph->LoadFromResourceName(0, "gl80");
        SpeedButton81->Glyph->LoadFromResourceName(0, "gl81");
        SpeedButton82->Glyph->LoadFromResourceName(0, "gl82");
        SpeedButton83->Glyph->LoadFromResourceName(0, "gl83");
        SpeedButton84->Glyph->LoadFromResourceName(0, "gl84");
        SpeedButton85->Glyph->LoadFromResourceName(0, "gl85");
        SpeedButton86->Glyph->LoadFromResourceName(0, "gl86");
        SpeedButton87->Glyph->LoadFromResourceName(0, "gl87");
        SpeedButton88->Glyph->LoadFromResourceName(0, "gl88set");
        SpeedButton89->Glyph->LoadFromResourceName(0, "gl89set");
        SpeedButton90->Glyph->LoadFromResourceName(0, "gl90set");
        SpeedButton91->Glyph->LoadFromResourceName(0, "gl91set");
        SpeedButton92->Glyph->LoadFromResourceName(0, "gl92set");
        SpeedButton93->Glyph->LoadFromResourceName(0, "gl93set");
        SpeedButton94->Glyph->LoadFromResourceName(0, "gl94set");
        SpeedButton95->Glyph->LoadFromResourceName(0, "gl95set");
        SpeedButton96->Glyph->LoadFromResourceName(0, "ConcourseGlyph");
        SpeedButton97->Glyph->LoadFromResourceName(0, "gl97");
        SpeedButton98->Glyph->LoadFromResourceName(0, "gl98");
        SpeedButton99->Glyph->LoadFromResourceName(0, "gl99");
        SpeedButton100->Glyph->LoadFromResourceName(0, "gl100");
        SpeedButton101->Glyph->LoadFromResourceName(0, "gl101");
        SpeedButton102->Glyph->LoadFromResourceName(0, "gl102");
        SpeedButton103->Glyph->LoadFromResourceName(0, "gl103");
        SpeedButton104->Glyph->LoadFromResourceName(0, "gl104");
        SpeedButton105->Glyph->LoadFromResourceName(0, "gl105");
        SpeedButton106->Glyph->LoadFromResourceName(0, "gl106");
        SpeedButton107->Glyph->LoadFromResourceName(0, "gl107");
        SpeedButton108->Glyph->LoadFromResourceName(0, "gl108");
        SpeedButton109->Glyph->LoadFromResourceName(0, "gl109");
        SpeedButton110->Glyph->LoadFromResourceName(0, "gl110");
        SpeedButton111->Glyph->LoadFromResourceName(0, "gl111");
        SpeedButton112->Glyph->LoadFromResourceName(0, "gl112");
        SpeedButton113->Glyph->LoadFromResourceName(0, "gl113");
        SpeedButton114->Glyph->LoadFromResourceName(0, "gl114");
        SpeedButton115->Glyph->LoadFromResourceName(0, "gl115");
        SpeedButton116->Glyph->LoadFromResourceName(0, "gl116");
        SpeedButton117->Glyph->LoadFromResourceName(0, "gl117");
        SpeedButton118->Glyph->LoadFromResourceName(0, "gl118");
        SpeedButton119->Glyph->LoadFromResourceName(0, "gl119");
        SpeedButton120->Glyph->LoadFromResourceName(0, "gl120");
        SpeedButton121->Glyph->LoadFromResourceName(0, "gl121");
        SpeedButton122->Glyph->LoadFromResourceName(0, "gl122");
        SpeedButton123->Glyph->LoadFromResourceName(0, "gl123");
        SpeedButton124->Glyph->LoadFromResourceName(0, "gl124");
        SpeedButton125->Glyph->LoadFromResourceName(0, "gl125");
        SpeedButton126->Glyph->LoadFromResourceName(0, "gl126");
        SpeedButton127->Glyph->LoadFromResourceName(0, "gl127");
        SpeedButton128->Glyph->LoadFromResourceName(0, "gl128");
        SpeedButton129->Glyph->LoadFromResourceName(0, "gl129");
        SpeedButton130->Glyph->LoadFromResourceName(0, "gl130");
        SpeedButton131->Glyph->LoadFromResourceName(0, "gl131");
        SpeedButton132->Glyph->LoadFromResourceName(0, "gl132");
        SpeedButton133->Glyph->LoadFromResourceName(0, "gl133");
        SpeedButton134->Glyph->LoadFromResourceName(0, "gl134");
        SpeedButton135->Glyph->LoadFromResourceName(0, "gl135");
        SpeedButton136->Glyph->LoadFromResourceName(0, "gl136");
        SpeedButton137->Glyph->LoadFromResourceName(0, "gl137");
        SpeedButton138->Glyph->LoadFromResourceName(0, "gl138");
        SpeedButton139->Glyph->LoadFromResourceName(0, "gl139");
        SpeedButton140->Glyph->LoadFromResourceName(0, "gl140");
        SpeedButton141->Glyph->LoadFromResourceName(0, "gl141");
        SpeedButton142->Glyph->LoadFromResourceName(0, "gl142");
        SpeedButton143->Glyph->LoadFromResourceName(0, "gl143");
        SpeedButton145->Glyph->LoadFromResourceName(0, "gl145");
        SpeedButton146->Glyph->LoadFromResourceName(0, "gl146");
        // below not in RailGraphics
        SpeedButton144->Glyph->LoadFromResourceName(0, "LCGlyph");

        AddPrefDirButton->Glyph->LoadFromResourceName(0, "AddPrefDir");
        AddTextButton->Glyph->LoadFromResourceName(0, "AddText");
        AddTrackButton->Glyph->LoadFromResourceName(0, "AddTrack");
        AutoSigsButton->Glyph->LoadFromResourceName(0, "AutoSig");
        CallingOnButton->Glyph->LoadFromResourceName(0, "CallingOn");
        DeleteAllPrefDirButton->Glyph->LoadFromResourceName(0, "ClearAllPrefDir");
        DeleteOnePrefDirButton->Glyph->LoadFromResourceName(0, "ClearOnePrefDir");
        ExitOperationButton->Glyph->LoadFromResourceName(0, "Exit");
        ExitPrefDirButton->Glyph->LoadFromResourceName(0, "Exit");
        ExitTrackButton->Glyph->LoadFromResourceName(0, "Exit");
        ExitTTModeButton->Glyph->LoadFromResourceName(0, "Exit");
        FontButton->Glyph->LoadFromResourceName(0, "FontGraphic");
        HomeButton->Glyph->LoadFromResourceName(0, "Home");
        LocationNameButton->Glyph->LoadFromResourceName(0, "NameLocs");
        MoveTextOrGraphicButton->Glyph->LoadFromResourceName(0, "MoveTextOrGraphic");
        NewHomeButton->Glyph->LoadFromResourceName(0, "NewHome");
        UnrestrictedButton->Glyph->LoadFromResourceName(0, "NonSig");
        OperateButton->Glyph->LoadFromResourceName(0, "RunGraphic");
        OperatorActionButton->Glyph->LoadFromResourceName(0, "ShowOpActionPanel");
        PerformanceLogButton->Glyph->LoadFromResourceName(0, "ShowLog");
        PresetAutoSigRoutesButton->Glyph->LoadFromResourceName(0, "PresetAutoSigRoutes");
        RouteCancelButton->Glyph->LoadFromResourceName(0, "RouteCancel");
        SaveRailwayPDPButton->Glyph->LoadFromResourceName(0, "SaveRailway"); // PrefDirPanel
        SaveRailwayBaseModeButton->Glyph->LoadFromResourceName(0, "SaveRailway"); // OperatingPanel
        SaveRailwayTBPButton->Glyph->LoadFromResourceName(0, "SaveRailway"); // TrackBuildPanel
        SaveSessionButton->Glyph->LoadFromResourceName(0, "SaveSession");
        ScreenDownButton->Glyph->LoadFromResourceName(0, "BlackArrowDown");
        ScreenGridButton->Glyph->LoadFromResourceName(0, "ScreenGrid");
        ScreenLeftButton->Glyph->LoadFromResourceName(0, "BlackArrowLeft");
        ScreenRightButton->Glyph->LoadFromResourceName(0, "BlackArrowRight");
        ScreenUpButton->Glyph->LoadFromResourceName(0, "BlackArrowUp");
        SetGapsButton->Glyph->LoadFromResourceName(0, "ConnectGaps");
        SetLengthsButton->Glyph->LoadFromResourceName(0, "SetDists");
        ShowHideTTButton->Glyph->LoadFromResourceName(0, "Hide");
        SigAspectButton->Glyph->LoadFromResourceName(0, "FourAspect"); // new at version 0.6
        SigPrefButton->Glyph->LoadFromResourceName(0, "PrefSig");
        TextOrUserGraphicGridButton->Glyph->LoadFromResourceName(0, "PixelPrecision1");
        TrackOKButton->Glyph->LoadFromResourceName(0, "Validate");
        TTClockAdjButton->Glyph->LoadFromResourceName(0, "TTClock");
        UserGraphicButton->Glyph->LoadFromResourceName(0, "PictureImage");

        BufferAttentionImage->Picture->Bitmap->LoadFromResourceName(0, "BufferWarning");
        CallOnImage->Picture->Bitmap->LoadFromResourceName(0, "CallingOn");
        CrashImage->Picture->Bitmap->LoadFromResourceName(0, "CrashWarning");
        DerailImage->Picture->Bitmap->LoadFromResourceName(0, "DerailWarning");
        SignalStopImage->Picture->Bitmap->LoadFromResourceName(0, "SignalStopWarning");
        SPADImage->Picture->Bitmap->LoadFromResourceName(0, "SPADWarning");
        TrainFailedImage->Picture->Bitmap->LoadFromResourceName(0, "TrainFailedWarning"); // new at v2.4.0

        DistanceKey->Picture->Bitmap->LoadFromResourceName(0, "DistanceKey");
        PrefDirKey->Picture->Bitmap->LoadFromResourceName(0, "PrefDirKey");

        TrackLinkedImage->Picture->Bitmap->LoadFromResourceName(0, "TrackLinkedGraphic");
        TrackNotLinkedImage->Picture->Bitmap->LoadFromResourceName(0, "TrackNotLinkedGraphic");
        GapsNotSetImage->Picture->Bitmap->LoadFromResourceName(0, "GapsNotSetGraphic");
        GapsSetImage->Picture->Bitmap->LoadFromResourceName(0, "GapsSetGraphic");
        LocationNamesNotSetImage->Picture->Bitmap->LoadFromResourceName(0, "LocNamesNotSetGraphic");
        LocationNamesSetImage->Picture->Bitmap->LoadFromResourceName(0, "LocNamesSetGraphic");

        SigsOnLeftImage1->Picture->Bitmap->LoadFromResourceName(0, "SigsOnLeft");
        SigsOnLeftImage2->Picture->Bitmap->LoadFromResourceName(0, "SigsOnLeft");
        SigsOnLeftImage1->Transparent = true;
        SigsOnLeftImage2->Transparent = true;
        SigsOnLeftImage1->Picture->Bitmap->TransparentColor = clB5G5R5;
        SigsOnLeftImage2->Picture->Bitmap->TransparentColor = clB5G5R5;
        SigsOnRightImage1->Picture->Bitmap->LoadFromResourceName(0, "SigsOnRight");
        SigsOnRightImage2->Picture->Bitmap->LoadFromResourceName(0, "SigsOnRight");
        SigsOnRightImage1->Transparent = true;
        SigsOnRightImage2->Transparent = true;
        SigsOnRightImage1->Picture->Bitmap->TransparentColor = clB5G5R5;
        SigsOnRightImage2->Picture->Bitmap->TransparentColor = clB5G5R5;

/* Don't need this - load icon directly into both Interface form & Application (via Project - Options - Application - Load Icon)
     RailwayIcon = new TPicture;
     RailwayIcon->Icon->LoadFromResourceName(0, "Icon1.ico");
     Icon = RailwayIcon->Icon;
     Application->Icon = RailwayIcon->Icon;
*/

        AnsiString NL = '\n';
        const AnsiString TTLabelStr1 = "Start new train" + NL + "Start new service from a split" + NL + "Start new service from another service" + NL +
            "Start new non-repeating shuttle finish service" + NL + "Start new shuttle train at a timetabled stop" + NL +
            "Start new shuttle service from a feeder";

        const AnsiString TTLabelStr2 = "Pass" + NL + "Be joined by another train" + NL + "Front split" + NL + "Rear split" + NL + "Change direction of train";

        const AnsiString TTLabelStr3 = "Finish && form a new service" + NL + "Finish && join another train" + NL + "Finish && exit railway" + NL +
            "Finish && repeat shuttle, finally remain here" + NL + "Finish && repeat shuttle, finally form a finishing service" + NL +
            "Finish non-repeating shuttle feeder service" + NL + "Finish && remain here";

        const AnsiString TTLabelStr4 = "HH:MM" + NL + "HH:MM" + NL + "HH:MM" + NL + "HH:MM" + NL + "HH:MM" + NL + "HH:MM" + NL + "HH:MM" + NL + "HH:MM" + NL +
            "HH:MM" + NL + "HH:MM" + NL + "HH:MM" + NL + "HH:MM" + NL + "HH:MM" + NL + "HH:MM" + NL + "HH:MM" + NL + "HH:MM" + NL + "HH:MM" + NL + "     " + NL + "R";

        const AnsiString TTLabelStr5 = "HH:MM ';' Location" + NL + "HH:MM ';' HH:MM ';' Location";

        const AnsiString TTLabelStr6 = "+ rear element ID - space - front element ID [+ optional ';S']" + NL + "+ ref. of the train that splits" + NL +
            "+ other service ref." + NL + "+ shuttle service ref." + NL + "+ rear element ID - space - front element ID ';' linked shuttle ref." + NL +
            "+ linked shuttle service ref. ';' feeder service ref." + NL + "+ location" + NL + "+ joining train ref." + NL + "+ new service ref." + NL +
            "+ new service ref." + NL + "    " + NL + "+ new service ref." + NL + "+ ref. of train to join" + NL +
            "+ list of valid exit element IDs (at least 1) separated by spaces" + NL + "+ linked shuttle service ref.";

        const AnsiString TTLabelStr7 = "Arrival OR departure time (program will determine which from the context) + location." + NL +
            "Arrival time, departure time (with no events between) + location";

        const AnsiString TTLabelStr9 = "Timetable entries" + NL + "(service references etc.)";
        const AnsiString TTLabelStr11 = "Timetable" + NL + "start time";

        const AnsiString TTLabelStr12 = "NB: WITHIN SERVICES commas must" + NL + "not be used (have special meanings)," + NL +
            "and semicolons may only be used to" + NL + "separate service components.";

        const AnsiString TTLabelStr13 = "+ linked shuttle service ref. ';' finishing service ref." + NL + "+ linked shuttle service ref.";

        const AnsiString TTLabelStr15 = "Repeat the service + ';' minutes between repeats ';' digit increment ';' number of repeats (last line of service)";

        TTLabel1->Caption = TTLabelStr1;
        TTLabel2->Caption = TTLabelStr2;
        TTLabel3->Caption = TTLabelStr3;
        TTLabel4->Caption = TTLabelStr4;
        TTLabel5->Caption = TTLabelStr5;
        TTLabel6->Caption = TTLabelStr6;
        TTLabel7->Caption = TTLabelStr7;
        TTLabel9->Caption = TTLabelStr9;
        TTLabel11->Caption = TTLabelStr11;
        TTLabel12->Caption = TTLabelStr12;
        TTLabel13->Caption = TTLabelStr13;
        TTLabel15->Caption = TTLabelStr15;

        // pick up transparent colour from file if there is one & set it to the stored value if it's valid else set to black

        AnsiString ColourStr = "";
        std::ifstream ColFile((CurDir + "\\Background.col").c_str());
        if(ColFile.fail())
        {
            Utilities->clTransparent = clB0G0R0; // default black background;
        }
        else
        {
            if(!(Utilities->CheckAndReadFileString(ColFile, ColourStr)))
            {
                Utilities->clTransparent = clB0G0R0; // default black background;
            }
            else if((ColourStr != "white") && (ColourStr != "black") && (ColourStr != "blue"))
            {
                Utilities->clTransparent = clB0G0R0; // default black background;
            }
            else
            {
                if(ColourStr == "white")
                {
                    Utilities->clTransparent = TColor(0xFFFFFF);
                }
                else if(ColourStr == "blue")
                {
                    Utilities->clTransparent = TColor(0x330000);
                }
                else
                {
                    Utilities->clTransparent = TColor(0);
                }
                ColFile.close(); // added at v2.3.0, should have been in earlier
            }
        }

        Utilities->RHSignalFlag = false; // new at v2.3.0 for RH signals, always left hand on startup
        AnsiString RHSigStr = "";
        std::ifstream SignalFile((CurDir + "\\Signal.hnd").c_str());
        if(SignalFile.fail())
        {
            SigImagePanel->Caption = "Signals will be on the left hand side of the track";
            SigsOnLeftImage1->Visible = true;
            SigsOnLeftImage2->Visible = true;
            SigsOnRightImage1->Visible = false;
            SigsOnRightImage2->Visible = false;
            SignalFile.close(); // close ifstream & open ofstream
            std::ofstream SignalFile((CurDir + "\\Signal.hnd").c_str());
            if(!SignalFile.fail()) // if it does fail then it will revert to LHS anyway on next load unless select RH sigs
            {
                Utilities->SaveFileString(SignalFile, "LHSignals");
            }
            SignalFile.close();
        }
        else
        {
            if(Utilities->CheckAndReadFileString(SignalFile, RHSigStr)) // if it fails then do nothing
            {
                if(RHSigStr == "RHSignals")
                {
                    RailGraphics->ConvertSignalsToOppositeHand(1); // always left hand initially when start program
                    ConverttoRightHandSignalsMenuItem->Caption = "Convert to Left Hand Signals";
                    if(Track->SignalAspectBuildMode == TTrack::GroundSignalBuild)
                    {
                        LoadGroundSignalGlyphs(1);
                    }
                    else
                    {
                        LoadNormalSignalGlyphs(1);
                    }
                    SigImagePanel->Caption = "Signals will be on the right hand side of the track";
                    SigsOnLeftImage1->Visible = false;
                    SigsOnLeftImage2->Visible = false;
                    SigsOnRightImage1->Visible = true;
                    SigsOnRightImage2->Visible = true;
                    SignalFile.close();
                }
                else
                {
                    ConverttoRightHandSignalsMenuItem->Caption = "Convert to Right Hand Signals";
                    SigImagePanel->Caption = "Signals will be on the left hand side of the track";
                    SigsOnLeftImage1->Visible = true;
                    SigsOnLeftImage2->Visible = true;
                    SigsOnRightImage1->Visible = false;
                    SigsOnRightImage2->Visible = false;
                    SignalFile.close(); // close ifstream & open ofstream
                    std::ofstream SignalFile((CurDir + "\\Signal.hnd").c_str());
                    if(!SignalFile.fail()) // if it does fail then it will revert to LHS anyway on next load unless select RH sigs
                    {
                        Utilities->SaveFileString(SignalFile, "LHSignals");
                    }
                    SignalFile.close();
                }
            }
        }

        SelectBitmap->TransparentColor = Utilities->clTransparent;
        RailGraphics->ChangeAllTransparentColours(Utilities->clTransparent, clB5G5R5); // original colour is as loaded at this stage - white
        if(Utilities->clTransparent != clB5G5R5)
            TextBox->Color = clB3G3R3;
        RailGraphics->SetUpAllDerivitiveGraphics(Utilities->clTransparent);

        MainScreen->Canvas->Brush->Color = Utilities->clTransparent;
        MainScreen->Canvas->FillRect(MainScreen->ClientRect);

        std::ifstream SplashFile((CurDir + "\\GNU").c_str());
        if(SplashFile.fail())
        {
            ShowMessage(
                "This program is free software released under the terms of the GNU General Public License Version 3, as published by the Free Software Foundation.  It may be used or redistributed in accordance with that license and is released in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details - you should have received a copy along with this program but if not see <http://www.gnu.org/licenses/>.");
            std::ofstream SplashFile((CurDir + "\\GNU").c_str());
            if(!SplashFile.fail())
                SplashFile.close();
        }

        if((Screen->Width < 1024) || (Screen->Height < 768))
        {
            ShowMessage("Please note that this program works best with a screen resolution of at least 1024 x 768.  Please change if possible");
        }

        SkipFormResizeEvent = true; // added at v2.1.0
        MasterClock->Enabled = true;
        Visible = true; // make Interface form visible (set to false at design time)    autocalls FormResize so it is skipped
        WindowState = wsMaximized; // need this for full screen at start                autocalls FormResize so it is skipped
        MTBFEditBox->Left = MainScreen->Left + MainScreen->Width - MTBFEditBox->Width + 30; // new v2.4.0 30 is to place it above the positional panel
                                                                                                     // has to come after Visible = true or doesn't show
        MTBFLabel->Left = MainScreen->Left + MainScreen->Width - MTBFEditBox->Width + 30 - 55; // new v2.4.0 placed above and to the left of MTBFEditBox
        PositionalPanel->Left = MainScreen->Left + MainScreen->Width; // changed at v2.4.0
        PositionalPanel->Top = MainScreen->Top; // changed at v2.4.0
        PositionalPanel->Height = MainScreen->Height; // changed at v2.4.0
        AllSetUpFlag = true;
        MissedTicks = 0;
        TotalTicks = 0;
        Level1Mode = BaseMode;
        SetLevel1Mode(131); // to reset background colour mode menu choices
        Screen->Cursor = TCursor(-2); // Arrow
        SkipFormResizeEvent = false; // added at v2.1.0
        SelectedGraphicFileName = ""; // only set to null here so always has a value after use LoadUserGraphic

        FloatingPanel->Color = TColor(0xF0FFFF); // new v2.2.0, corrects floating panel background colour in Windows 10
        PerformancePanel->Color = TColor(0xCCCCCC); // new v2.2.0 as above
        OperatorActionPanel->Color = TColor(0xCCCCCC); // new v2.2.0 as above
        DevelopmentPanel->Color = TColor(0xCCCCCC); // new v2.2.0 as above
        TTStartTimeBox->Color = TColor(0x99FFFF); // cream
        HighlightPanel->Color = TColor(0x33CCFF);
        TrainController->OpActionPanelHintDelayCounter = 0;
        MTBFEditBox->Visible = false; // new at v2.4.0
        MTBFLabel->Visible = false;
        TrainController->AvHoursIntValue = 0;
        TrainController->MTBFHours = 0;
        TTStartTimePtr = 0;
        TTFirstServicePtr = 0;
        TTLastServicePtr = 0;

        LastNonCtrlOrShiftKeyDown = -1; //set to no key

        SigImagePanel->Left = (Interface->Width - SigImagePanel->Width) / 2; // added for v2.3.0

        // below added at v2.4.0 so able to load session files with the correct decimal point
        Utilities->DecimalPoint = '.'; // default case is full stop
        char *LocalNumericInformation = setlocale(LC_NUMERIC, ""); // need this to set lconv to the environment's numeric format
        Utilities->SetLocaleResultOK = true;
        if(LocalNumericInformation == "") // call failed, don't change decimal point in Utilities.cpp
        {
            Utilities->SetLocaleResultOK = false;
        }
        struct lconv Locale; // store this structure in memory (accessed via locale.h in Utilities.h)
        struct lconv *conv = &Locale;
        // read the locality conversion structure
        conv = localeconv(); // this is what updates the structure
        Utilities->DecimalPoint = conv->decimal_point[0];
    }

    catch(const EFOpenError &e)
    {
        TMsgDlgButtons But;
        But << mbOK;
        MessageDlg(e.Message + " - program must terminate", mtError, But, 0);
        Application->Terminate();
    }

    catch(const Exception &e)
    {
        TMsgDlgButtons But;
        But << mbOK;
        AnsiString Message = "A fatal error occurred during the program setup process, the program must terminate.  Message = " + e.Message;
        MessageDlg(Message, mtError, But, 0); // this message given first in case can't create the error log
        ErrorLog(115, e.Message);
        Application->Terminate();
    }
}

// ---------------------------------------------------------------------------

__fastcall TInterface::~TInterface()
{ // destructor
    try
    {
        SkipFormResizeEvent = true; // added at v2.1.0
        delete NonSigRouteStartMarker;
        delete SigRouteStartMarker;
        delete AutoRouteStartMarker;
        delete PointFlash;
        delete SelectBitmap;
        delete TrainController;
        delete EveryPrefDir;
        delete ConstructRoute;
        delete ConstructPrefDir;
        delete AllRoutes;
        delete Track;
        delete TextHandler;
        delete HiddenDisplay;
        delete HiddenScreen;
        delete Display;
        delete RailGraphics;
        delete Utilities;
    }
    catch(const Exception &e)
    {
        ErrorLog(116, e.Message);
    }
}
////autocalls FormResize at end

// ---------------------------------------------------------------------------

void __fastcall TInterface::FormCreate(TObject *Sender)
{ // these functions have to be defined here to take effect when application activated & deactivated
    try
    {
        Application->OnDeactivate = AppDeactivate;
        Application->OnActivate = AppActivate;
    }
    catch(const Exception &e)
    {
        ErrorLog(117, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::AppDeactivate(TObject *Sender)
{ // pause operation if operating & stop the master clock
    try
    {
        if((Level1Mode == OperMode) && (Level2OperMode == Operating))
        {
            if(Track->RouteFlashFlag) // in case route building - cancels the route, freezes otherwise - reported
            { // by Matt Blades 30/06/11
                RevertToOriginalRouteSelector(14);
                ConstructRoute->RouteFlash.RouteFlashVector.clear();
                ConstructRoute->ClearRoute();
                Screen->Cursor = TCursor(-2); // Arrow
                Track->RouteFlashFlag = false;
                ClearandRebuildRailway(48); // to get rid of displayed route
            }
            if(Track->PointFlashFlag)
    // added at v1.3.1 to prevent lockup for flashing points when deactivates.  Notified by Ian Walker in his email of 25/03/13.
            {
                PointFlash->PlotOriginal(42, Display);
                Track->PointFlashFlag = false;
                DivergingPointVectorPosition = -1;
                Screen->Cursor = TCursor(-2); // Arrow
            }
            Level2OperMode = Paused;
            SetLevel2OperMode(2);
        }
        MasterClock->Enabled = false;
    }
    catch(const Exception &e)
    {
        ErrorLog(118, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::AppActivate(TObject *Sender)
{ // restart the master clock providing Interface constructor has run
    try
    {
        if(AllSetUpFlag)
        {
            MasterClock->Enabled = true;
        }
    }
    catch(const Exception &e)
    {
        ErrorLog(119, e.Message);
    }
}

// ---------------------------------------------------------------------------

UnicodeString TInterface::GetVersion()
{
    DWORD VersionHandle;
    DWORD VersionSize;
    LPBYTE pBuffer;
    UnicodeString strVersion = L"N/A";

    VersionSize = GetFileVersionInfoSizeW(Application->ExeName.c_str(), &VersionHandle);
    if(VersionSize)
    {
        pBuffer = new BYTE[VersionSize];

        if(GetFileVersionInfoW(Application->ExeName.c_str(), VersionHandle, VersionSize, pBuffer))
        {
            VS_FIXEDFILEINFO *fi;
            UINT buflen;

            // uncomment strVersion and HIWORD alternates below when future CI implemented: sas@2.1.0
            if(VerQueryValueW(pBuffer, L"\\", (void**)&fi, &buflen))
            {
                // strVersion.sprintf(L"%d.%d.%d (Build %d)",
                strVersion.sprintf(L"%d.%d.%d", HIWORD(fi->dwFileVersionMS), LOWORD(fi->dwFileVersionMS), HIWORD(fi->dwFileVersionLS)
                        // HIWORD(fi->dwFileVersionLS), LOWORD(fi->dwFileVersionLS)
                    );
            }
        }

        delete[]pBuffer;
    }

    return L" v" + strVersion;
}

// ---------------------------------------------------------------------------
// Track Build Interface
// ---------------------------------------------------------------------------
void __fastcall TInterface::BuildTrackMenuItemClick(TObject *Sender) // Mode Menu Item
{
    try
    {
        TrainController->LogEvent("BuildTrackMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",BuildTrackMenuItemClick");
        Level1Mode = TrackMode;
        SetLevel1Mode(0);
        Utilities->CallLogPop(1159);
    }
    catch(const Exception &e)
    {
        ErrorLog(120, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::AddTrackButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("AddTrackButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",AddTrackButtonClick");
        Level1Mode = TrackMode;
        SetLevel1Mode(38);
        Level2TrackMode = AddTrack;
        SetLevel2TrackMode(2);
        Utilities->CallLogPop(1162);
    }
    catch(const Exception &e)
    {
        ErrorLog(121, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::SpeedButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("SpeedButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",SpeedButtonClick");
        if(((TSpeedButton*)Sender)->Down)
        {
            CurrentSpeedButton = (TSpeedButton*)Sender;
// TrainController->LogEvent("SpeedButtonClick, " + CurrentSpeedButton->Tag); //v 1.3.1 - using non-AnsiString CurrentSpeedButton->Tag sends (for an unknown reason) a completely wrong string to LogEvent, usually "...(behind this message)"
            TrainController->LogEvent("SpeedButtonClick, " + AnsiString(CurrentSpeedButton->Tag)); // new version //use for v1.3.2
        }
        else
            CurrentSpeedButton = 0;
        SelectionValid = false;
        ReselectMenuItem->Enabled = false;
        Utilities->CallLogPop(1163);
    }
    catch(const Exception &e)
    {
        ErrorLog(122, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::TrackOKButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("TrackOKButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",TrackOKButtonClick");
        SelectionValid = false;
        ResetChangedFileDataAndCaption(0, true);
        bool LocError;
        int HLoc, VLoc;
        // erase any corrupted PrefDirs then rebuild track & PrefDir vectors
// EveryPrefDir->EraseCorruptedElementsAfterTrackBuild();  not needed after dispensed with blank track elements for erased elements
        if(!(Track->TryToConnectTrack(0, LocError, HLoc, VLoc, true))) // true for give messages
        // if successful repositions TrackVector & builds TrackMap
        {
            if(LocError) // links not complete or other error - show offending element
            {
                while((Display->DisplayOffsetH - HLoc) > 0)
                    Display->DisplayOffsetH -= (Utilities->ScreenElementWidth / 2); // use 30 instead of 60 so less likely to appear behind the message box
                while((HLoc - Display->DisplayOffsetH) > (Utilities->ScreenElementWidth - 1))
                    Display->DisplayOffsetH += (Utilities->ScreenElementWidth / 2);
                while((Display->DisplayOffsetV - VLoc) > 0)
                    Display->DisplayOffsetV -= (Utilities->ScreenElementHeight / 2); // use 18 instead of 36 so less likely to appear behind the message box
                while((VLoc - Display->DisplayOffsetV) > (Utilities->ScreenElementHeight - 1))
                    Display->DisplayOffsetV += (Utilities->ScreenElementHeight / 2);
                ClearandRebuildRailway(0);
                Display->InvertElement(0, HLoc * 16, VLoc * 16);
                ShowMessage("Incomplete track or other error - see inverted element (may be behind this message)");
                ClearandRebuildRailway(1); // to clear inversion
                Level1Mode = TrackMode;
                SetLevel1Mode(39);
                Level2TrackMode = AddTrack; // go to add track regardless of where started from
                SetLevel2TrackMode(3);
                Utilities->CallLogPop(0);
                return;
            }
            else
            { // reach here if there are no track elements
                ShowMessage("Unable to set any track links");
                Level1Mode = TrackMode;
                SetLevel1Mode(40);
                Level2TrackMode = AddTrack;
                SetLevel2TrackMode(4); // go to add track regardless of where started from
                Utilities->CallLogPop(1);
                return;
            }
        }
        else
        {
            // success ('TrackFinished' set in TryToConnectTrack)
            EveryPrefDir->RebuildPrefDirVector(0); // from TrackMap
            ShowMessage("Successful Completion");
        }
// success if reach here ('TrackFinished' set in TryToConnectTrack)
        if(Level2TrackMode == AddTrack)
        {
            Level1Mode = TrackMode;
            SetLevel1Mode(41);
            SetLevel2TrackMode(5);
        }
        else
        {
            Level1Mode = TrackMode;
            SetLevel1Mode(36); // back to TrackMode if not in AddTrack mode
        }
        Utilities->CallLogPop(2);
    }
    catch(const Exception &e)
    {
        ErrorLog(3, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::SetGapsButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("SetGapsButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",SetGapsButtonClick");
        SelectionValid = false;
        ReselectMenuItem->Enabled = false;
        Level1Mode = TrackMode;
        SetLevel1Mode(42);
        Level2TrackMode = GapSetting;
        SetLevel2TrackMode(6);
        Utilities->CallLogPop(1164);
    }
    catch(const Exception &e)
    {
        ErrorLog(123, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::AddTextButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("AddTextButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",AddTextButtonClick");
        Level1Mode = TrackMode;
        SetLevel1Mode(43);
        Level2TrackMode = AddText;
        SetLevel2TrackMode(7);
        Utilities->CallLogPop(1165);
    }
    catch(const Exception &e)
    {
        ErrorLog(124, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::MoveTextOrGraphicButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("MoveTextOrGraphicButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",MoveTextOrGraphicButtonClick");
        Level1Mode = TrackMode;
        SetLevel1Mode(44);
        Level2TrackMode = MoveTextOrGraphic;
        SetLevel2TrackMode(8);
        Utilities->CallLogPop(1166);
    }
    catch(const Exception &e)
    {
        ErrorLog(125, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::TextBoxKeyPress(TObject *Sender, char &Key)
{
    try
    {
        TrainController->LogEvent("TextBoxKeyPress," + AnsiString(Key));
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",TextBoxKeyPress," + AnsiString(Key));
        if(Key == '\x0D') // CR
        {
            if(TextBox->Text != "") // if blank then don't save
            {
                if(Display->GetFont()->Color == clB5G5R5) // white
                {
                    TFont *TempFont = new TFont;
                    TempFont->Assign(Display->GetFont());
                    TempFont->Color = clB0G0R0; // change to black for vector & saving
                    Display->SetFont(TempFont);
                    delete TempFont;
                }
                TFont *DisplayFont = Display->GetFont();
                TTextItem TempText = TTextItem(Text_X, Text_Y, TextBox->Text, DisplayFont);
                TempText.Font = DisplayFont; // may have been changed in above constructor when returned as reference
                TextHandler->EnterAndDisplayNewText(0, TempText, Text_X, Text_Y);
            }
            EditMenu->Enabled = true;
            TextBox->Visible = false;
            SetLevel2TrackMode(56); // to enable 'move text' if first text item added
        }
        else if(Key == '\x1B') // escape
        {
            TextBox->Visible = false;
        }
        Utilities->CallLogPop(3);
    }
    catch(const Exception &e)
    {
        ErrorLog(4, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::LocationNameButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("LocationNameButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",LocationNameButtonClick");
        Level1Mode = TrackMode;
        SetLevel1Mode(45);
        Level2TrackMode = AddLocationName;
        SetLevel2TrackMode(9);
        Utilities->CallLogPop(1167);
    }
    catch(const Exception &e)
    {
        ErrorLog(126, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::LocationNameKeyUp(TObject *Sender, WORD &Key, TShiftState Shift)
{
    try
    {
        TrainController->LogEvent("LocationNameKeyUp," + AnsiString(Key));
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",LocationNameKeyUp," + AnsiString(Key));
        if(Track->LNPendingList.empty())
        {
            ShowMessage("Error, location name being entered without an entry in LNPendingList");
            Level1Mode = TrackMode;
            SetLevel1Mode(46);
            Level2TrackMode = AddLocationName;
            SetLevel2TrackMode(10);
            Utilities->CallLogPop(4);
            return;
        }
        if(Key == '\x1B') // escape
        {
            Track->LNPendingList.clear(); // get rid of existing entry
            Level1Mode = TrackMode;
            SetLevel1Mode(47);
            Level2TrackMode = AddLocationName;
            SetLevel2TrackMode(11);
            Utilities->CallLogPop(5);
            return;
        }
        if(Key == '\x0D')
        {
            Screen->Cursor = TCursor(-11); // Hourglass;
            SetTrackBuildImages(8);
            AnsiString ExistingName;
            if(Track->LNPendingList.front() > -1)
                ExistingName = Track->InactiveTrackElementAt(27, Track->LNPendingList.front()).LocationName;
            else
                ExistingName = Track->TrackElementAt(425, -1 - (Track->LNPendingList.front())).LocationName;
            if(Track->LocationNameAllocated(1, LocationNameTextBox->Text) && (ExistingName != LocationNameTextBox->Text))
            { // name allocated to a different location
                UnicodeString MessageStr = UnicodeString("Another location named '") + LocationNameTextBox->Text +
                    UnicodeString("' already exists.  If you continue its name will be erased.  Do you wish to continue?");
                int button = Application->MessageBox(MessageStr.c_str(), L"Warning!", MB_YESNO | MB_ICONWARNING);
                if(button == IDNO)
                {
                    Track->LNPendingList.clear(); // get rid of existing entry
                    Screen->Cursor = TCursor(-2); // Arrow
                    Level1Mode = TrackMode;
                    SetLevel1Mode(48);
                    Level2TrackMode = AddLocationName;
                    SetLevel2TrackMode(12);
                    Utilities->CallLogPop(6);
                    return;
                }
                Track->EraseLocationAndActiveTrackElementNames(1, LocationNameTextBox->Text);
                Track->EnterLocationName(1, LocationNameTextBox->Text, false);
                int HPos, VPos;
                bool UseExistingPosition = false;
                if(EraseLocationNameText(0, LocationNameTextBox->Text, HPos, VPos))
                {;
                } // condition not used
                // above may be redundant at v1.1.0 due to name erase in EnterLocationName
                // but, the location to be named may also have an existing name, in which case that needs to be erased
                // and the position re-used
                if(ExistingName != "")
                {
                    if(EraseLocationNameText(3, ExistingName, HPos, VPos))
                        UseExistingPosition = true; // may be redundant at v1.1.0 due to name erase in EnterLocationName
                }
                AddLocationNameText(0, LocationNameTextBox->Text, HPos, VPos, UseExistingPosition);
                Screen->Cursor = TCursor(-2); // Arrow
                Level1Mode = TrackMode;
                SetLevel1Mode(49);
                Level2TrackMode = AddLocationName;
                SetLevel2TrackMode(13);
                Utilities->CallLogPop(7);
                return;
            }
            else if(Track->LocationNameAllocated(2, LocationNameTextBox->Text) && (ExistingName == LocationNameTextBox->Text))
            { // same name being entered again
                Track->LNPendingList.clear(); // get rid of existing entry as the location already has this name
                                              // but in case the name is not already in text vector erase it and re-add it
                                              // if it wasn't in the vector erasing it has no effect
                int HPos, VPos;
                bool UseExistingPosition = false;
                if(EraseLocationNameText(2, LocationNameTextBox->Text, HPos, VPos))
                    UseExistingPosition = true;
                // above may be redundant at v1.1.0 due to name erase in EnterLocationName
                AddLocationNameText(2, LocationNameTextBox->Text, HPos, VPos, UseExistingPosition);
                Screen->Cursor = TCursor(-2); // Arrow
                Level1Mode = TrackMode;
                SetLevel1Mode(50);
                Level2TrackMode = AddLocationName;
                SetLevel2TrackMode(14);
                Utilities->CallLogPop(8);
                return;
            }
            else
            { // either a new name for an unnamed location, or a different name for a named location
              // check validity of entry
                AnsiString LocStr = LocationNameTextBox->Text;
                LocStr = LocStr.Trim(); // strip leading & trailing spaces, and control characters
                LocationNameTextBox->Text = LocStr; // reset this as used below
/* drop this, now covered by ...Trim() above
            //strip leading spaces
            while((LocStr != "") && (LocStr[1] == ' '))
                {
                LocStr = LocStr.SubString(2, LocStr.Length()-1);
                }
*/
                if((LocStr != "") && (LocStr[1] >= '0') && (LocStr[1] <= '9')) // can't begin with a number
                {
                    Screen->Cursor = TCursor(-2); // Arrow
                    ShowMessage("Location name can't begin with a number");
                    Level1Mode = TrackMode;
                    SetLevel1Mode(51);
                    Level2TrackMode = AddLocationName;
                    SetLevel2TrackMode(15);
                    Utilities->CallLogPop(776);
                    return;
                }
                if(LocStr.Length() > 50)
                {
                    Screen->Cursor = TCursor(-2); // Arrow
                    ShowMessage("Location name too long, 50 characters maximum");
                    Level1Mode = TrackMode;
                    SetLevel1Mode(122);
                    Level2TrackMode = AddLocationName;
                    SetLevel2TrackMode(55);
                    Utilities->CallLogPop(1735);
                    return;
                }
                for(int x = 1; x <= LocStr.Length(); x++)
                {
                    char Ch = LocStr[x];
                    if((Ch != ' ') && (Ch != '&') && (Ch != '(') && (Ch != ')') && (Ch != ':') && (Ch != 39) && (Ch != '.') && (Ch != '-') && (Ch != '+') &&
                        (Ch != '/') && ((Ch < '0') || (Ch > '9')) && ((Ch < 'A') || (Ch > 'Z')) && ((Ch < 'a') || (Ch > 'z')))
                    {
                        Screen->Cursor = TCursor(-2); // Arrow
                        ShowMessage(
                            "Location name contains one or more invalid characters, must be alphanumeric, brackets, space, full stop, colon, inverted comma, '-', '+', '/' or '&&'");
                        Level1Mode = TrackMode;
                        SetLevel1Mode(52);
                        Level2TrackMode = AddLocationName;
                        SetLevel2TrackMode(16);
                        Utilities->CallLogPop(777);
                        return;
                    }
                }
                if(LocStr == "cdt") // this has Time:Command which could be confused with Time:Loc
                {
                    Screen->Cursor = TCursor(-2); // Arrow
                    ShowMessage("Location name cannot be 'cdt', this name would interfere with the timetable");
                    Level1Mode = TrackMode;
                    SetLevel1Mode(53);
                    Level2TrackMode = AddLocationName;
                    SetLevel2TrackMode(17);
                    Utilities->CallLogPop(778);
                    return;
                }
                Track->EnterLocationName(2, LocStr, false);
                // need to check if the location already has a name, and if so erase it from the textvector
                int HPos, VPos;
                bool UseExistingPosition = false;
                if(ExistingName != "")
                {
                    if(EraseLocationNameText(1, ExistingName, HPos, VPos))
                        UseExistingPosition = true; // may be redundant at v1.1.0 due to name erase in EnterLocationName
                }
                AddLocationNameText(1, LocationNameTextBox->Text, HPos, VPos, UseExistingPosition);
                Screen->Cursor = TCursor(-2); // Arrow
                Level1Mode = TrackMode;
                SetLevel1Mode(54);
                Level2TrackMode = AddLocationName;
                SetLevel2TrackMode(18);
                Utilities->CallLogPop(9);
                return;
            }
        }
        Screen->Cursor = TCursor(-2); // Arrow
        Utilities->CallLogPop(10);
    }
    catch(const Exception &e)
    {
        ErrorLog(5, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::SetLengthsButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("SetLengthsButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",SetLengthsButtonClick");
        SelectLengthsFlag = false;
        ConstructPrefDir->ExternalClearPrefDirAnd4MultiMap(); // added for extended distances
        Level1Mode = TrackMode;
        SetLevel1Mode(55);
        Level2TrackMode = DistanceStart;
        SetLevel2TrackMode(19);
        Utilities->CallLogPop(1168);
    }
    catch(const Exception &e)
    {
        ErrorLog(127, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::LengthOKButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("LengthOKButtonClick," + DistanceBox->Text + "," + SpeedLimitBox->Text);
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",LengthOKButtonClick");
        int Dist = 0, SpeedLimit = 0;
        AnsiString DistanceStr = DistanceBox->Text;
        if(SelectLengthsFlag && (DistanceStr == ""))
            DistanceStr = "No change";
        AnsiString SpeedStr = SpeedLimitBox->Text;
        if(SelectLengthsFlag && (SpeedStr == ""))
            SpeedStr = "No change";
        if(SelectLengthsFlag)
        {
            if(DistanceStr == "No change")
                Dist = -1; // i.e.don't change
            if(SpeedStr == "No change")
                SpeedLimit = -1; // i.e.don't change
        }
        else
        {
            if(DistanceStr == AnsiString(OverallDistance))
                Dist = -1; // i.e.don't change
            if((SpeedStr == "Mixed") || (SpeedStr == AnsiString(OverallSpeedLimit)))
                SpeedLimit = -1; // i.e.don't change
        }
        if(((Dist != -1) && (DistanceStr.Length() > 6)) || ((SpeedLimit != -1) && (SpeedStr.Length() > 6)))
        {
            ShowMessage("One or more entries too long");
            Utilities->CallLogPop(11);
            return;
        }
        if((DistanceStr == "") || (SpeedStr == ""))
        {
            ShowMessage("One or more entries blank");
            Utilities->CallLogPop(12);
            return;
        }
        if(SelectLengthsFlag && (Dist != -1))
        {
            for(int x = 1; x <= DistanceStr.Length(); x++)
            {
                if((DistanceStr[x] < '0') || (DistanceStr[x] > '9'))
                {
                    ShowMessage("Track length value must be a positive whole number, or blank for no change");
                    Utilities->CallLogPop(1415);
                    return;
                }
            }
        }
        if(!SelectLengthsFlag)
        {
            for(int x = 1; x <= DistanceStr.Length(); x++)
            {
                if((DistanceStr[x] < '0') || (DistanceStr[x] > '9'))
                {
                    ShowMessage("Distance must be a positive whole number");
                    Utilities->CallLogPop(13);
                    return;
                }
            }
        }
        if(SelectLengthsFlag && (SpeedLimit != -1))
        {
            for(int x = 1; x <= SpeedStr.Length(); x++)
            {
                if((SpeedStr[x] < '0') || (SpeedStr[x] > '9'))
                {
                    ShowMessage("Speed limit must be a positive whole number, or blank for no change");
                    Utilities->CallLogPop(1416);
                    return;
                }
            }
        }
        if(!SelectLengthsFlag && (SpeedStr != "Mixed"))
        {
            for(int x = 1; x <= SpeedStr.Length(); x++)
            {
                if((SpeedStr[x] < '0') || (SpeedStr[x] > '9'))
                {
                    ShowMessage("Speed limit must be a positive whole number, or 'Mixed'");
                    Utilities->CallLogPop(14);
                    return;
                }
            }
        }
        if(Dist != -1)
            Dist = DistanceStr.ToInt();
        if(SpeedLimit != -1)
            SpeedLimit = SpeedStr.ToInt();
/* don't need this with new condition below
    if(SelectLengthsFlag && (Dist != -1) && (Dist < 20))
        {
        ShowMessage("Track length value must be a minimum of 20m, setting to 20m");
        Dist = 20;
        }
*/
        if(((Dist != -1) && (Dist < 20)) || ((SpeedLimit != -1) && (SpeedLimit < 10)) || ((SpeedLimit != -1) && (SpeedLimit > TTrain::MaximumSpeedLimit)))
        // new limiting values for v0.6 (used only to fail at either value 0); added TTrain::MaxSpeedLimit at v2.1.0
        {
            ShowMessage("Lengths must be 20m or more, and speeds must be between 10km/h and 400km/h"); // changed at v2.1.0 to limit max speed
            Utilities->CallLogPop(15);
            return;
        }
        DistanceBox->Text = "";
        SpeedLimitBox->Text = "";
        if(SelectLengthsFlag)
        {
            int LowSelectHLoc = SelectBitmapHLoc;
            int HighSelectHLoc = SelectBitmapHLoc + (SelectBitmap->Width / 16);
            int LowSelectVLoc = SelectBitmapVLoc;
            int HighSelectVLoc = SelectBitmapVLoc + (SelectBitmap->Height / 16);
            bool FoundFlag;
            bool NamedLocPresent = false;
            if((Dist != -1) && (Dist != DefaultTrackLength))
            {
                for(int x = LowSelectHLoc; x < HighSelectHLoc; x++)
                {
                    for(int y = LowSelectVLoc; y < HighSelectVLoc; y++)
                    {
                        if(Track->IsPlatformOrNamedNonStationLocationPresent(6, x, y))
                            NamedLocPresent = true;
                    }
                }
            }
            if(NamedLocPresent && (Dist < 50)) // changed in v2.4.0
            {
                ShowMessage("Note: Named location elements are quite short. If they are too short the simulation might depart too far from reality.");
            }

            if(NamedLocPresent && (Dist > 200)) // changed in v2.4.0
            {
                ShowMessage("Note: Named location elements are quite long. If they are too long the simulation might depart too far from reality.");
            }

            for(int x = LowSelectHLoc; x < HighSelectHLoc; x++)
            {
                for(int y = LowSelectVLoc; y < HighSelectVLoc; y++)
                {
                    int VecPos = Track->GetVectorPositionFromTrackMap(34, x, y, FoundFlag);
                    if(FoundFlag)
                    {
                        if(Dist > -1) // && !(Track->IsPlatformOrNamedNonStationLocationPresent(7, x, y)))
                        {
                            Track->TrackElementAt(692, VecPos).Length01 = Dist;
                            if(Track->TrackElementAt(693, VecPos).Length23 != -1)
                            {
                                Track->TrackElementAt(694, VecPos).Length23 = Dist;
                            }
                        }
                        if(SpeedLimit > -1)
                        {
                            Track->TrackElementAt(695, VecPos).SpeedLimit01 = SpeedLimit;
                            if(Track->TrackElementAt(696, VecPos).SpeedLimit23 != -1)
                            {
                                Track->TrackElementAt(697, VecPos).SpeedLimit23 = SpeedLimit;
                            }
                        }
                    }
                }
            }
            TrackLengthPanel->Visible = false;
            SelectLengthsFlag = false; // go back to normal distance setting mode
        }
        else
        {
            SetTrackLengths(1, Dist, SpeedLimit);
        }
        Level1Mode = TrackMode;
        SetLevel1Mode(57);
        Level2TrackMode = DistanceStart;
        SetLevel2TrackMode(21);
        Utilities->CallLogPop(16);
    }
    catch(const Exception &e)
    {
        ErrorLog(6, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::LengthCancelButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("LengthCancelButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",LengthCancelButtonClick");
        DistanceBox->Text = "";
        SpeedLimitBox->Text = "";
        TrackLengthPanel->Visible = false;
        SelectLengthsFlag = false; // go back to normal distance setting mode
        Level1Mode = TrackMode;
        SetLevel1Mode(59);
        Level2TrackMode = DistanceStart;
        SetLevel2TrackMode(23);
        Utilities->CallLogPop(1169);
    }
    catch(const Exception &e)
    {
        ErrorLog(128, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::ResetDefaultLengthButtonClick(TObject *Sender)
{
    try
    {
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",ResetDefaultLengthButtonClick");
        TMsgDlgButtons Buttons;
        Buttons << mbYes << mbNo;
        if(MessageDlg("This will reset the selected elements to default lengths & speed limits.  Proceed?", mtWarning, Buttons, 0) == mrNo)
        {
            // leave all as was before
            Utilities->CallLogPop(17);
            return;
        }
        else
        {
            TrainController->LogEvent("Accepted ResetDefaultLengthButtonClick");
            DistanceBox->Text = "";
            SpeedLimitBox->Text = "";
            if(SelectLengthsFlag)
            {
                int LowSelectHLoc = SelectBitmapHLoc;
                int HighSelectHLoc = SelectBitmapHLoc + (SelectBitmap->Width / 16);
                int LowSelectVLoc = SelectBitmapVLoc;
                int HighSelectVLoc = SelectBitmapVLoc + (SelectBitmap->Height / 16);
                bool FoundFlag;
                for(int x = LowSelectHLoc; x < HighSelectHLoc; x++)
                {
                    for(int y = LowSelectVLoc; y < HighSelectVLoc; y++)
                    {
                        int VecPos = Track->GetVectorPositionFromTrackMap(35, x, y, FoundFlag);
                        if(FoundFlag)
                        {
                            Track->TrackElementAt(698, VecPos).Length01 = DefaultTrackLength;
                            if(Track->TrackElementAt(699, VecPos).Length23 != -1)
                            {
                                Track->TrackElementAt(700, VecPos).Length23 = DefaultTrackLength;
                            }
                            Track->TrackElementAt(701, VecPos).SpeedLimit01 = DefaultTrackSpeedLimit;
                            if(Track->TrackElementAt(702, VecPos).SpeedLimit23 != -1)
                            {
                                Track->TrackElementAt(703, VecPos).SpeedLimit23 = DefaultTrackSpeedLimit;
                            }
                        }
                    }
                }
                TrackLengthPanel->Visible = false;
// ClearandRebuildRailway(47); don't need this
                SelectLengthsFlag = false; // go back to normal distance setting mode
            }
            else
            {
                TrackLengthPanel->Visible = false;
                bool FoundFlag;
                if(ConstructPrefDir->PrefDirSize() == 0)
                {
                    Utilities->CallLogPop(1120);
                    return;
                }
                for(unsigned int x = 0; x < ConstructPrefDir->PrefDirSize(); x++)
                {
                    TPrefDirElement PrefDirElement = ConstructPrefDir->GetFixedPrefDirElementAt(169, x);
                    TTrackElement & TrackElement = Track->TrackElementAt(37, Track->GetVectorPositionFromTrackMap(40, PrefDirElement.HLoc, PrefDirElement.VLoc,
                        FoundFlag));
                    if((TrackElement.TrackType == Points) || (TrackElement.TrackType == Crossover) || (TrackElement.TrackType == Bridge))
    // only set the relevant track to default length & speed limit
                    {
                        if((PrefDirElement.GetELinkPos() < 2) && (PrefDirElement.GetXLinkPos() < 2)) // could be one of each for points
                        {
                            TrackElement.Length01 = DefaultTrackLength;
                            TrackElement.SpeedLimit01 = DefaultTrackSpeedLimit; // 200km/h = 125mph
                        }
                        else
                        {
                            TrackElement.Length23 = DefaultTrackLength;
                            TrackElement.SpeedLimit23 = DefaultTrackSpeedLimit; // 200km/h = 125mph
                        }
                    }
                    else // any other 1 track element, including platforms being present
                    {
                        if((PrefDirElement.GetELinkPos() > 1) && (PrefDirElement.GetXLinkPos() > 1))
                        {
                            throw Exception("Error, XLinkPos > 1 in SetOneDefaultTrackLength at " + AnsiString(TrackElement.HLoc) + " & " +
                                AnsiString(TrackElement.VLoc));
                        }
                        TrackElement.Length01 = DefaultTrackLength;
                        TrackElement.SpeedLimit01 = DefaultTrackSpeedLimit; // 200km/h = 125mph
                        TrackElement.Length23 = -1;
                        TrackElement.SpeedLimit23 = -1;
                    }
                }
            }
            Level1Mode = TrackMode;
            SetLevel1Mode(61);
            Level2TrackMode = DistanceStart;
            SetLevel2TrackMode(25);
        }
        Utilities->CallLogPop(18);
    }
    catch(const Exception &e)
    {
        ErrorLog(7, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::RestoreAllDefaultLengthsButtonClick(TObject *Sender)
{
    try
    {
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",RestoreAllDefaultLengthsButtonClick");
        TMsgDlgButtons Buttons;
        Buttons << mbYes << mbNo;
        if(MessageDlg("This will reset ALL track elements to default lengths & speed limits.  Proceed?", mtWarning, Buttons, 0) == mrNo)
        {
            // leave all as was before
            Utilities->CallLogPop(19);
            return;
        }
        else
        {
            Track->SetAllDefaultLengthsAndSpeedLimits(1);
        }
        TrainController->LogEvent("Accepted RestoreAllDefaultLengthsButtonClick");
        DistanceBox->Text = "";
        SpeedLimitBox->Text = "";
        TrackLengthPanel->Visible = false;
        SelectLengthsFlag = false; // go back to normal distance setting mode
        Level1Mode = TrackMode;
        SetLevel1Mode(63);
        Level2TrackMode = DistanceStart;
        SetLevel2TrackMode(27);
        Utilities->CallLogPop(20);
    }
    catch(const Exception &e)
    {
        ErrorLog(8, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::ExitTrackButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("ExitTrackButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",ExitTrackButtonClick");
        if(Level2TrackMode == CutMoving)
        {
            Level2TrackMode = Pasting; // to paste the selection
            SetLevel2TrackMode(53);
        }
        DevelopmentPanel->Visible = false; // development use only
        ScreenGridFlag = false;
        SelectionValid = false;
        Track->SelectGraphicVector.clear();
        // delete all unwanted TPictures in UserGraphicMap
        if(!Track->UserGraphicMap.empty()) // if empty skip it
        {
            TTrack::TUserGraphicMap::iterator UGMIt = Track->UserGraphicMap.begin();
            do
            {
                bool GraphicFoundInVector = false;
                for(TTrack::TUserGraphicVector::iterator UGVIt = Track->UserGraphicVector.begin(); UGVIt < Track->UserGraphicVector.end(); UGVIt++)
                {
                    if(UGMIt->first == UGVIt->FileName)
                    {
                        GraphicFoundInVector = true;
                        break;
                    }
                }
                if(!GraphicFoundInVector)
                {
                    delete UGMIt->second;
                    Track->UserGraphicMap.erase(UGMIt);
                    UGMIt = Track->UserGraphicMap.begin(); // reset the iterator because erasing an element it points to invalidates it & if use it after then
                                                           // behaviour is undefined, the iteration will end eventually because the map has got shorter after an erase
                }
                else
                {
                    UGMIt++;
                }
            }
            while(UGMIt != Track->UserGraphicMap.end());
        }
        Level1Mode = BaseMode;
        SetLevel1Mode(2);
        Utilities->CallLogPop(1170);
    }
    catch(const Exception &e)
    {
        ErrorLog(129, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::TextOrUserGraphicGridButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("TextOrUserGraphicGridButtonClick," + AnsiString(TextOrUserGraphicGridVal));
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",TextOrUserGraphicGridButtonClick");
        if(TextOrUserGraphicGridVal == 1)
        {
            TextOrUserGraphicGridVal = 2;
            TextOrUserGraphicGridButton->Glyph->LoadFromResourceName(0, "PixelPrecision2");
        }
        else if(TextOrUserGraphicGridVal == 2)
        {
            TextOrUserGraphicGridVal = 4;
            TextOrUserGraphicGridButton->Glyph->LoadFromResourceName(0, "PixelPrecision4");
        }
        else if(TextOrUserGraphicGridVal == 4)
        {
            TextOrUserGraphicGridVal = 8;
            TextOrUserGraphicGridButton->Glyph->LoadFromResourceName(0, "PixelPrecision8");
        }
        else if(TextOrUserGraphicGridVal == 8)
        {
            TextOrUserGraphicGridVal = 16;
            TextOrUserGraphicGridButton->Glyph->LoadFromResourceName(0, "PixelPrecision16");
        }
        else
        {
            TextOrUserGraphicGridVal = 1;
            TextOrUserGraphicGridButton->Glyph->LoadFromResourceName(0, "PixelPrecision1");
        }
        Utilities->CallLogPop(1171);
    }
    catch(const Exception &e)
    {
        ErrorLog(130, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::SigAspectButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("SigAspectButtonClick," + AnsiString(Track->SignalAspectBuildMode));
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",SigAspectButtonClick");
        if(Track->SignalAspectBuildMode == TTrack::FourAspectBuild)
        {
            Track->SignalAspectBuildMode = TTrack::ThreeAspectBuild;
            SigAspectButton->Glyph->LoadFromResourceName(0, "ThreeAspect");
        }
        else if(Track->SignalAspectBuildMode == TTrack::ThreeAspectBuild)
        {
            Track->SignalAspectBuildMode = TTrack::TwoAspectBuild;
            SigAspectButton->Glyph->LoadFromResourceName(0, "TwoAspect");
        }
        else if(Track->SignalAspectBuildMode == TTrack::TwoAspectBuild)
        {
            Track->SignalAspectBuildMode = TTrack::GroundSignalBuild;
            SigAspectButton->Glyph->LoadFromResourceName(0, "GroundSig");
// set all signal glyphs to ground signals
            LoadGroundSignalGlyphs(0);
        }
        else
        {
            Track->SignalAspectBuildMode = TTrack::FourAspectBuild;
            SigAspectButton->Glyph->LoadFromResourceName(0, "FourAspect");
// set all signal glyphs to normal signals
            LoadNormalSignalGlyphs(0);
        }
        Utilities->CallLogPop(1869);
    }
    catch(const Exception &e)
    {
        ErrorLog(180, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::FontButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("FontButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",FontButtonClick");
        FontDialog->Font = Display->GetFont();
        FontDialog->Execute();
        if(FontDialog->Font->Color == clB5G5R5)
            FontDialog->Font->Color = clB0G0R0; // don't store white in font, will display black as white on
        // dark backgrounds
        Display->SetFont(FontDialog->Font);
        if(TextBox->Visible)
            TextBox->SetFocus();
        else if(LocationNameTextBox->Visible)
            LocationNameTextBox->SetFocus();
        Utilities->CallLogPop(1172);
    }
    catch(const Exception &e)
    {
        ErrorLog(131, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::ScreenGridButtonClick(TObject *Sender)
{
    try
    {
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",ScreenGridButtonClick");
        if(ScreenGridFlag)
        {
            TrainController->LogEvent("ScreenGridButtonClick + ScreenGrid off");
            ScreenGridFlag = false;
        }
        else
        {
            TrainController->LogEvent("ScreenGridButtonClick + ScreenGrid on");
            ScreenGridFlag = true;
        }
        ClearandRebuildRailway(28);
        Utilities->CallLogPop(89);
    }
    catch(const Exception &e)
    {
        ErrorLog(33, e.Message);
    }
}

// ---------------------------------------------------------------------------
// PrefDir Interface
// ---------------------------------------------------------------------------
void __fastcall TInterface::PlanPrefDirsMenuItemClick(TObject *Sender) // Mode Menu Item
{
    try
    {
        TrainController->LogEvent("PlanPrefDirsMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",PlanPrefDirsMenuItemClick");
        Level1Mode = PrefDirMode;
        SetLevel1Mode(3);
        Utilities->CallLogPop(1173);
    }
    catch(const Exception &e)
    {
        ErrorLog(132, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::AddPrefDirButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("AddPrefDirButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",AddPrefDirButtonClick");
        if(ConstructPrefDir->PrefDirSize() == 0)
        {
            ShowMessage("No preferred direction selection");
            Utilities->CallLogPop(22);
            return;
        }
        Screen->Cursor = TCursor(-11); // Hourglass;
        if(ConstructPrefDir->ValidatePrefDir(1))
            EveryPrefDir->ConsolidatePrefDirs(0, ConstructPrefDir);
        Level1Mode = PrefDirMode;
        SetLevel1Mode(4);
        Screen->Cursor = TCursor(-2); // Arrow
        Utilities->CallLogPop(23);
    }
    catch(const Exception &e)
    {
        ErrorLog(10, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::DeleteAllPrefDirButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("DeleteAllPrefDirButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",DeleteAllPrefDirButtonClick");
        TMsgDlgButtons Buttons;
        Buttons << mbYes << mbNo;
        if(MessageDlg("Do you really want to clear all preferred directions?", mtWarning, Buttons, 0) == mrNo)
        {
            Utilities->CallLogPop(24);
            return;
        }
        // leave all as was before pressed DeleteAllPrefDirButton
        else
        {
            ResetChangedFileDataAndCaption(1, false);
            EveryPrefDir->ExternalClearPrefDirAnd4MultiMap();
            ConstructPrefDir->ExternalClearPrefDirAnd4MultiMap();
            Level1Mode = PrefDirMode;
            SetLevel1Mode(5);
        }
        Utilities->CallLogPop(25);
    }
    catch(const Exception &e)
    {
        ErrorLog(11, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::DeleteOnePrefDirButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("DeleteOnePrefDirButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",DeleteOnePrefDirButtonClick");
        ResetChangedFileDataAndCaption(18, false);
// RlyFile = false; - don't alter this just for PrefDir changes
        Screen->Cursor = TCursor(-11); // Hourglass;
        for(unsigned int x = 0; x < ConstructPrefDir->PrefDirSize(); x++)
        {
            EveryPrefDir->EraseFromPrefDirVectorAnd4MultiMap(1, ConstructPrefDir->GetFixedPrefDirElementAt(178, x).HLoc,
                ConstructPrefDir->GetFixedPrefDirElementAt(179, x).VLoc);
        }
        ConstructPrefDir->ExternalClearPrefDirAnd4MultiMap();
        Level1Mode = PrefDirMode;
        SetLevel1Mode(81); // all PrefDir truncated
        Screen->Cursor = TCursor(-2); // Arrow
        Utilities->CallLogPop(1591);
    }
    catch(const Exception &e)
    {
        ErrorLog(46, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::ExitPrefDirButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("ExitPrefDirButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",ExitPrefDirButtonClick");
        Level1Mode = BaseMode;
        SetLevel1Mode(6);
        Utilities->CallLogPop(1554);
    }
    catch(const Exception &e)
    {
        ErrorLog(133, e.Message);
    }
}

// ---------------------------------------------------------------------------
// Operate Railway Interface
// ---------------------------------------------------------------------------
void __fastcall TInterface::OperateRailwayMenuItemClick(TObject *Sender) // Mode Menu Item
{
    try
    {
        TrainController->LogEvent("OperateRailwayMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",OperateRailwayMenuItemClick");
        TTrain::NextTrainID = 0; // reset to 0 whenever enter operating mode
        AllRoutes->NextRouteID = 0; // reset to 0 whenever enter operating mode
        Level1Mode = OperMode;
        SetLevel1Mode(7);
        Utilities->CallLogPop(26);
    }
    catch(const Exception &e)
    {
        ErrorLog(12, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::OperateButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("StartOperationButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",OperateButtonClick");
        if((Level2OperMode == NoOperMode) || (Level2OperMode == Paused) || (Level2OperMode == PreStart))
        {
            Level2OperMode = Operating;
            SetLevel2OperMode(0);
        }
        else
        {
            Level2OperMode = Paused;
            SetLevel2OperMode(1);
        }
        Utilities->CallLogPop(1175);
    }
    catch(const Exception &e)
    {
        ErrorLog(37, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::AutoSigsButtonClick(TObject *Sender)
    // must have PrefDirs to be available
{
    try
    {
        TrainController->LogEvent("AutoSigsButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",AutoSigsButtonClick");
        AutoSigsFlag = true;
        PreferredRoute = true;
        ConsecSignalsRoute = true;

        AutoSigsButton->Enabled = false;
        SigPrefButton->Enabled = true;
        UnrestrictedButton->Enabled = true;

        InfoPanel->Visible = true;
        if(Level2OperMode == PreStart)
            InfoPanel->Caption = "PRE-START:  Select AUTOMATIC SIGNAL ROUTE start signal, or left click points to change manually";
        else
            InfoPanel->Caption = "OPERATING:  Select AUTOMATIC SIGNAL ROUTE start signal, or left click points to change manually";
        InfoCaptionStore = InfoPanel->Caption;
        AutoRouteStartMarker->PlotOriginal(1, Display); // if overlay not plotted will ignore
        SigRouteStartMarker->PlotOriginal(2, Display); // if overlay not plotted will ignore
        NonSigRouteStartMarker->PlotOriginal(3, Display); // if overlay not plotted will ignore
        RouteMode = RouteNotStarted;
        Utilities->CallLogPop(28);
    }
    catch(const Exception &e)
    {
        ErrorLog(14, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::SigPrefButtonClick(TObject *Sender)
    // must have PrefDirs to be available
{
    try
    {
        TrainController->LogEvent("SigPrefButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",SigPrefButtonClick");
        AutoSigsFlag = false;
        PreferredRoute = true;
        ConsecSignalsRoute = true;

        AutoSigsButton->Enabled = true;
        SigPrefButton->Enabled = false;
        UnrestrictedButton->Enabled = true;

        InfoPanel->Visible = true;
        if(Level2OperMode == PreStart)
            InfoPanel->Caption = "PRE-START:  Select PREFERRED ROUTE start signal, or left click points to change manually";
        else
            InfoPanel->Caption = "OPERATING:  Select PREFERRED ROUTE start signal, or left click points to change manually";
        InfoCaptionStore = InfoPanel->Caption;
        AutoRouteStartMarker->PlotOriginal(4, Display); // if overlay not plotted will ignore
        SigRouteStartMarker->PlotOriginal(5, Display); // if overlay not plotted will ignore
        NonSigRouteStartMarker->PlotOriginal(6, Display); // if overlay not plotted will ignore
        RouteMode = RouteNotStarted;
        Utilities->CallLogPop(29);
    }
    catch(const Exception &e)
    {
        ErrorLog(15, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::UnrestrictedButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("NoSigNonPrefButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",NoSigNonPrefButtonClick");
        AutoSigsFlag = false;
        PreferredRoute = false;
        ConsecSignalsRoute = false;
        if(EveryPrefDir->PrefDirSize() > 0)
        {
            AutoSigsButton->Enabled = true;
            SigPrefButton->Enabled = true;
            UnrestrictedButton->Enabled = false;
        }
        else
        {
            AutoSigsButton->Enabled = false;
            SigPrefButton->Enabled = false;
            UnrestrictedButton->Enabled = false;
        }
        InfoPanel->Visible = true;
        if(Level2OperMode == PreStart)
            InfoPanel->Caption = "PRE-START:  Select UNRESTRICTED ROUTE start location, or left click points to change manually";
        else
            InfoPanel->Caption = "OPERATING:  Select UNRESTRICTED ROUTE start location, or left click points to change manually";
        InfoCaptionStore = InfoPanel->Caption;
        AutoRouteStartMarker->PlotOriginal(7, Display); // if overlay not plotted will ignore
        SigRouteStartMarker->PlotOriginal(8, Display); // if overlay not plotted will ignore
        NonSigRouteStartMarker->PlotOriginal(9, Display); // if overlay not plotted will ignore
        RouteMode = RouteNotStarted;
        Utilities->CallLogPop(30);
    }
    catch(const Exception &e)
    {
        ErrorLog(16, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::RouteCancelButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("RouteCancelButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",RouteCancelButtonClick");
        RouteCancelFlag = true;
        InfoPanel->Visible = true;
        InfoPanel->Caption = "ROUTE CANCELLING:  Right click on truncate element, first element to cancel (anywhere else to skip)";
        RouteCancelButton->Enabled = false;
        AutoRouteStartMarker->PlotOriginal(32, Display); // if overlay not plotted will ignore
        SigRouteStartMarker->PlotOriginal(33, Display); // if overlay not plotted will ignore
        NonSigRouteStartMarker->PlotOriginal(34, Display); // if overlay not plotted will ignore
        Utilities->CallLogPop(1176);
    }
    catch(const Exception &e)
    {
        ErrorLog(35, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::PerformanceLogButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("PerformanceLogButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",PerformanceLogButtonClick");
        if(!ShowPerformancePanel)
        {
            ShowPerformancePanel = true;
            PerformancePanel->Visible = true;
            PerformanceLogButton->Glyph->LoadFromResourceName(0, "HideLog");
        }
        else
        {
            ShowPerformancePanel = false;
            PerformancePanel->Visible = false;
            PerformanceLogButton->Glyph->LoadFromResourceName(0, "ShowLog");
        }
        Utilities->CallLogPop(1177);
    }
    catch(const Exception &e)
    {
        ErrorLog(36, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::ExitOperationButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("ExitOperationButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",ExitOperationButtonClick");

        UnicodeString MessageStr = "Please note that the session will be lost if it hasn't been saved.  Do you still wish to exit?";
        TrainController->StopTTClockFlag = true; // so TTClock stopped during MasterClockTimer function
        TrainController->RestartTime = TrainController->TTClockTime;
        int button = Application->MessageBox(MessageStr.c_str(), L"Please confirm", MB_YESNO);
        TrainController->BaseTime = TDateTime::CurrentDateTime();
        TrainController->StopTTClockFlag = false;
        if(button == IDNO)
        {
            Utilities->CallLogPop(751);
            return;
        }
        Track->ResetSignals(1);
        Track->ResetPoints(1);
        TrainController->SendPerformanceSummary(0, Utilities->PerformanceFile); // must come before trains finished becuase examines the train vectors
        Utilities->PerformanceFile.close();
        TrainController->UnplotTrains(1);
        TrainController->FinishedOperation(0);
        RouteMode = None;
        PreferredRoute = true;
        ConsecSignalsRoute = true;
        AllRoutes->AllRoutesClear();
        ShowPerformancePanel = false;
        PerformanceLogButton->Glyph->LoadFromResourceName(0, "ShowLog");
        ShowOperatorActionPanel = false; // new at v2.2.0
        OperatorActionButton->Glyph->LoadFromResourceName(0, "ShowOpActionPanel"); // new v2.2.0
        PerformanceLogBox->Lines->Clear();
        PerformancePanel->Visible = false;
        PerformancePanel->Top = MainScreen->Top + MainScreen->Height - PerformancePanel->Height;
        PerformancePanel->Left = MainScreen->Left;
// TipButton->Glyph->LoadFromResourceName(0, "ShowLog"); //'Trains in play' new at v2.2.0
        OAListBox->Clear();
        OperatorActionPanel->Visible = false;
        OperatorActionPanel->Top = MainScreen->Top + MainScreen->Height - OperatorActionPanel->Height;
        OperatorActionPanel->Left = MainScreen->Left + MainScreen->Width - OperatorActionPanel->Width; ;
        TrainController->ContinuationAutoSigVector.clear();
        AllRoutes->LockedRouteVector.clear();
        Level1Mode = BaseMode;
        SetLevel1Mode(8); // calls Clearand...
        Utilities->CallLogPop(1555);
    }
    catch(const Exception &e)
    {
        ErrorLog(13, e.Message);
    }
}

// ---------------------------------------------------------------------------
// Menu Interface (for items not already covered above)
// ---------------------------------------------------------------------------
void __fastcall TInterface::LoadRailwayMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("LoadRailwayMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",LoadRailwayMenuItemClick");
        if(!ClearEverything(1))
        {
            Utilities->CallLogPop(1139);
            return;
        }
        // LoadRailwayDialog->Filter = "Development file (*.dev)|*.dev|Railway file (*.rly)|*.rly"; //as was
        // changed at v2.0.0 (Embarcadero change) to show all files together
        LoadRailwayDialog->Filter = "Railway files (*.rly or *.dev)|*.rly; *.dev";
        if(LoadRailwayDialog->Execute())
        {
            TrainController->LogEvent("LoadRailway " + AnsiString(LoadRailwayDialog->FileName));
            LoadRailway(0, AnsiString(LoadRailwayDialog->FileName));
        }
        // else ShowMessage("Load Aborted"); drop this
        // Display->Update(); //display updated in ClearandRebuildRailway
        Track->CalcHLocMinEtc(9);
        Level1Mode = BaseMode;
        TrainController->AvHoursIntValue = 0;
        TrainController->MTBFHours = 0;
        SetLevel1Mode(11); // calls Clearand... to plot the new railway
        Utilities->CallLogPop(31);
    }
    catch(const Exception &e)
    {
        ErrorLog(17, e.Message);
    }
}
// ---------------------------------------------------------------------------

void TInterface::LoadRailway(int Caller, AnsiString LoadFileName)
{ // display of the loaded railway covered in the calling routine
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",LoadRailway," + LoadFileName);
    if(FileIntegrityCheck(0, LoadFileName.c_str()))
    {
        Screen->Cursor = TCursor(-11); // Hourglass;
        std::ifstream VecFile(LoadFileName.c_str());
        if(!(VecFile.fail()))
        {
            AnsiString TempString = Utilities->LoadFileString(VecFile); // version number
            int TempOffsetHHome = Utilities->LoadFileInt(VecFile);
            int TempOffsetVHome = Utilities->LoadFileInt(VecFile);
            bool GraphicsFollow = false;
            // can't load DisplayOffsetH & VHome until after LoadTrack as that calls TrackClear & zeroes them
// load track elements
            Track->LoadTrack(1, VecFile, GraphicsFollow);
// load text elements
            TextHandler->LoadText(0, VecFile);
// load PrefDir elements
            EveryPrefDir->LoadPrefDir(0, VecFile);
            if(GraphicsFollow)
            {
// load user graphics
                Track->LoadGraphics(0, VecFile, CurDir + "\\" + USERGRAPHICS_DIR_NAME); // include path to Graphics folder
            }
            EveryPrefDir->CheckPrefDirAgainstTrackVector(0); // clears PrefDir if any discrepancies found
            VecFile.close();
            Display->DisplayOffsetHHome = TempOffsetHHome;
            Display->DisplayOffsetVHome = TempOffsetVHome;
            Display->ResetZoomInOffsets();

            TFont *TempFont = new TFont; // if try to alter MainScreen->Canvas->Font directly it won't change the style for some reason
            TempFont->Style.Clear();
            TempFont->Name = "MS Sans Serif"; // reset font, else stays set to last displayed text font
            TempFont->Size = 10;
            TempFont->Color = clB0G0R0;
            TempFont->Charset = (TFontCharset)(0);
            MainScreen->Canvas->Font->Assign(TempFont);
            delete TempFont;

// calculate starting zoomed out offset values - same as when zoom out button clicked
            int OVOffH_NVCentre = Display->DisplayOffsetH - (1.5 * Utilities->ScreenElementWidth);
// start zoomout centre at DisplayOffsetH + 30 - zoomout width/2 = -(1.5 * 60)
            int LeftExcess = OVOffH_NVCentre - Track->GetHLocMin();
            int RightExcess = Track->GetHLocMax() - OVOffH_NVCentre - ((4 * Utilities->ScreenElementWidth) - 1);
            if((LeftExcess > 0) && (RightExcess > 0))
                Display->DisplayZoomOutOffsetH = OVOffH_NVCentre;
            else if((LeftExcess > 0) && (RightExcess <= 0))
                Display->DisplayZoomOutOffsetH = OVOffH_NVCentre + ((RightExcess) / (Utilities->ScreenElementWidth / 2)) *
                    (Utilities->ScreenElementWidth / 2); // normalise to nearest half screen
            else if((LeftExcess <= 0) && (RightExcess > 0))
                Display->DisplayZoomOutOffsetH = OVOffH_NVCentre - ((LeftExcess) / (Utilities->ScreenElementWidth / 2)) * (Utilities->ScreenElementWidth / 2);
            else
                Display->DisplayZoomOutOffsetH = OVOffH_NVCentre; // no excess at either side, so display in centre

            int OVOffV_NVCentre = Display->DisplayOffsetV - (1.5 * Utilities->ScreenElementHeight);
            int TopExcess = OVOffV_NVCentre - Track->GetVLocMin();
            int BotExcess = Track->GetVLocMax() - OVOffV_NVCentre - ((4 * Utilities->ScreenElementHeight) - 1);
            if((TopExcess > 0) && (BotExcess > 0))
                Display->DisplayZoomOutOffsetV = OVOffV_NVCentre;
            else if((TopExcess > 0) && (BotExcess <= 0))
                Display->DisplayZoomOutOffsetV = OVOffV_NVCentre + ((BotExcess) / (Utilities->ScreenElementHeight / 2)) *
                    (Utilities->ScreenElementHeight / 2); // normalise to nearest half screen
            else if((TopExcess <= 0) && (BotExcess > 0))
                Display->DisplayZoomOutOffsetV = OVOffV_NVCentre - ((TopExcess) / (Utilities->ScreenElementHeight / 2)) * (Utilities->ScreenElementHeight / 2);
            else
                Display->DisplayZoomOutOffsetV = OVOffV_NVCentre; // no excess at either side, so display in centre
// all above same as when zoom out button clicked
            Display->DisplayZoomOutOffsetVHome = Display->DisplayZoomOutOffsetV; // now set zoomed out 'home' values
            Display->DisplayZoomOutOffsetHHome = Display->DisplayZoomOutOffsetH;

            SavedFileName = AnsiString(LoadRailwayDialog->FileName); // includes the full PrefDir
            if(SavedFileName != "") // shouldn't be "" at this stage but leave in as a safeguard
            {
                char LastChar = SavedFileName[SavedFileName.Length()];
                if((LastChar == 'y') || (LastChar == 'Y'))
                {
                    if(!(Track->IsReadyForOperation()))
                    {
                        ShowMessage("Railway not ready for operation so unable to load as a .rly file.  Loading as a new railway under development");
                        SavedFileName = "";
                        RlyFile = false;
                        RailwayTitle = "";
                        TimetableTitle = "";
                        SetCaption(5);
                        Track->CalcHLocMinEtc(1);
                        Screen->Cursor = TCursor(-2); // Arrow
                        Level1Mode = BaseMode;
                        SetLevel1Mode(9);
                        Utilities->CallLogPop(1136);
                        return;
                    }
                    else
                    {
                        RlyFile = true;
                    }
                }
                else
                {
                    RlyFile = false;
                }
            }
            else
            {
                RlyFile = false;
            }
            FileChangedFlag = false;
            for(int x = AnsiString(LoadRailwayDialog->FileName).Length(); x > 0; x--)
            {
                if(AnsiString(LoadRailwayDialog->FileName)[x] == '\\')
                {
                    RailwayTitle = AnsiString(LoadRailwayDialog->FileName).SubString(x + 1, AnsiString(LoadRailwayDialog->FileName).Length() - x - 4);
                    TimetableTitle = "";
                    SetCaption(6);
                    break;
                }
            }
        } // if(VecFile)
        else
            ShowMessage("File open failed prior to load");
        Screen->Cursor = TCursor(-2); // Arrow
    } // if(FileIntegrityCheck(LoadRailwayDialog->FileName.c_str()))
    else
        ShowMessage("File integrity check failed - unable to load");
    Utilities->CallLogPop(1774);
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::SaveMenuItemClick(TObject *Sender)
{
// save under existing name
// no need to alter RlyFile for saving under existing name

    try
    {
        TrainController->LogEvent("SaveMenuItemClick, " + SavedFileName);
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",SaveMenuItemClick");
        Screen->Cursor = TCursor(-11); // Hourglass;
        std::ofstream VecFile(SavedFileName.c_str());
        if(!(VecFile.fail()))
        {
            Utilities->SaveFileString(VecFile, ProgramVersion);
            Utilities->SaveFileInt(VecFile, Display->DisplayOffsetHHome);
            Utilities->SaveFileInt(VecFile, Display->DisplayOffsetVHome);
            // save track elements
            if(Track->UserGraphicVector.empty())
            {
                Track->SaveTrack(3, VecFile, false); // false for no graphics (**Active elements** saved as marker)
            }
            else
            {
                Track->SaveTrack(8, VecFile, true); // true for graphics to be saved (**Active elements**1 saved as marker)
            }
            // save text elements
            TextHandler->SaveText(0, VecFile);
            // save PrefDir elements
            EveryPrefDir->SavePrefDirVector(0, VecFile);
            if(!Track->UserGraphicVector.empty())
            {
                // save user graphics
                Track->SaveUserGraphics(0, VecFile);
            }
            FileChangedFlag = false;
            VecFile.close();
        }
        else
            ShowMessage("File open failed prior to save");
        Screen->Cursor = TCursor(-2); // Arrow
        Level1Mode = BaseMode;
        SetLevel1Mode(12); // to disable the save option
        Utilities->CallLogPop(1178);
    }
    catch(const Exception &e)
    {
        ErrorLog(135, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::SaveAsMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("SaveAsMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",SaveAsMenuItemClick");
        SaveAsSubroutine(0);
        Utilities->CallLogPop(32);
    }
    catch(const Exception &e)
    {
        ErrorLog(18, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::SaveImageNoGridMenuItemClick(TObject *Sender)
{ // need to stop clock in case invoke during operation
    try
    {
        TrainController->LogEvent("SaveImageNoGridMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",SaveImageNoGridMenuItemClick");
        if(!DirectoryExists(CurDir + "\\" + IMAGE_DIR_NAME))
        {
            ShowMessage("Failed to find folder " + IMAGE_DIR_NAME + " in the folder where 'railway.exe' resides.  Image can't be saved");
            Utilities->CallLogPop(1695);
            return;
        }
        Screen->Cursor = TCursor(-11); // Hourglass;
        TrainController->StopTTClockFlag = true; // so TTClock stopped during MasterClockTimer function
        TrainController->RestartTime = TrainController->TTClockTime;
        AnsiString ImageFileName = TDateTime::CurrentDateTime().FormatString("dd-mm-yyyy hh.nn.ss");
        // format "16/06/2009 20:55:17"
        // avoid characters in filename:=   / \ : * ? " < > |
        ImageFileName = CurDir + "\\" + IMAGE_DIR_NAME + "\\RailwayImage " + ImageFileName + "; " + RailwayTitle + ".bmp";
        AnsiString ShortName = "";
        for(int x = ImageFileName.Length(); x > 0; x--)
        {
            if(ImageFileName[x] == '\\')
            {
                ShortName = ImageFileName.SubString(x + 1, ImageFileName.Length() - x - 4);
                break;
            }
        }
        ShowMessage("A bitmap file named " + ShortName + " will be created in the Images folder");
        Graphics::TBitmap *RailwayImage = new Graphics::TBitmap;
        RailwayImage->PixelFormat = pf8bit; // needed to ensure compatibility with track

        int HPosMin = Track->GetHLocMin() * 16;
        int HPosMax = (Track->GetHLocMax() + 1) * 16;
        int VPosMin = Track->GetVLocMin() * 16;
        int VPosMax = (Track->GetVLocMax() + 1) * 16;
        RailwayImage->Width = HPosMax - HPosMin;
        RailwayImage->Height = VPosMax - VPosMin;

        // need to check if there is any text that extends past HPosMax or below VPosMax
        // use font height x text length x .7 for new rh calc & font height * 1.5 for new bottom calc
        if(!TextHandler->TextVector.empty())
        {
            for(TTextHandler::TTextVectorIterator TextPtr = TextHandler->TextVector.begin(); TextPtr != TextHandler->TextVector.end(); TextPtr++)
            {
                int NewWidth = (TextPtr->HPos - HPosMin) + (abs(TextPtr->Font->Height) * TextPtr->TextString.Length() * 0.7);
                int NewHeight = (TextPtr->VPos - VPosMin) + (abs(TextPtr->Font->Height) * 1.5);
                if(NewWidth > RailwayImage->Width)
                {
                    RailwayImage->Width = NewWidth;
                }
                if(NewHeight > RailwayImage->Height)
                {
                    RailwayImage->Height = NewHeight;
                }
            }
        }

        RailwayImage->Canvas->Brush->Color = clB5G5R5; // set it all to white initially
        TRect Rect(0, 0, RailwayImage->Width, RailwayImage->Height);
        RailwayImage->Canvas->FillRect(Rect);

        // write graphics first so text & track overwrite
        Track->WriteGraphicsToImage(0, RailwayImage);
        // then write text so track overwrites
        TextHandler->WriteTextToImage(0, RailwayImage);
        Track->WriteTrackToImage(0, RailwayImage);

        RailwayImage->SaveToFile(ImageFileName);
        delete RailwayImage;
        TrainController->BaseTime = TDateTime::CurrentDateTime();
        TrainController->StopTTClockFlag = false;
        Screen->Cursor = TCursor(-2); // Arrow
        Utilities->CallLogPop(1535);
    }
    catch(const Exception &e)
    {
        if(e.Message.Pos("torage") > 0) // 'storage', avoid capitals as may be OS dependent
        {
            Screen->Cursor = TCursor(-2); // Arrow;
            UnicodeString MessageStr = "Insufficient memory available to store this image";
            Application->MessageBox(MessageStr.c_str(), L"", MB_OK | MB_ICONWARNING);
        }
        else
        {
            ErrorLog(42, e.Message);
        }
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::SaveImageAndGridMenuItemClick(TObject *Sender)
{ // need to stop clock in case invoke during operation
    try
    {
        TrainController->LogEvent("SaveImageAndGridMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",SaveImageAndGridMenuItemClick");
        if(!DirectoryExists(CurDir + "\\" + IMAGE_DIR_NAME))
        {
            ShowMessage("Failed to find folder " + IMAGE_DIR_NAME + " in the folder where 'railway.exe' resides.  Image can't be saved");
            Utilities->CallLogPop(1696);
            return;
        }
        Screen->Cursor = TCursor(-11); // Hourglass;
        TrainController->StopTTClockFlag = true; // so TTClock stopped during MasterClockTimer function
        TrainController->RestartTime = TrainController->TTClockTime;
        AnsiString ImageFileName = TDateTime::CurrentDateTime().FormatString("dd-mm-yyyy hh.nn.ss");
        // format "16/06/2009 20:55:17"
        // avoid characters in filename:=   / \ : * ? " < > |
        ImageFileName = CurDir + "\\" + IMAGE_DIR_NAME + "\\RailwayImage " + ImageFileName + "; " + RailwayTitle + ".bmp";
        AnsiString ShortName = "";
        for(int x = ImageFileName.Length(); x > 0; x--)
        {
            if(ImageFileName[x] == '\\')
            {
                ShortName = ImageFileName.SubString(x + 1, ImageFileName.Length() - x - 4);
                break;
            }
        }
        ShowMessage("A bitmap file named " + ShortName + " will be created in the Images folder");
        Graphics::TBitmap *RailwayImage = new Graphics::TBitmap;
        RailwayImage->PixelFormat = pf8bit; // needed to ensure compatibility with track
        int HPosMin = Track->GetHLocMin() * 16;
        int HPosMax = (Track->GetHLocMax() + 1) * 16;
        int VPosMin = Track->GetVLocMin() * 16;
        int VPosMax = (Track->GetVLocMax() + 1) * 16;
        RailwayImage->Width = HPosMax - HPosMin;
        RailwayImage->Height = VPosMax - VPosMin;

        // need to check if there is any text that extends past HPosMax or below VPosMax
        // use font height x text length x .7 for new rh calc & font height * 1.5 for new bottom calc
        if(!TextHandler->TextVector.empty())
        {
            for(TTextHandler::TTextVectorIterator TextPtr = TextHandler->TextVector.begin(); TextPtr != TextHandler->TextVector.end(); TextPtr++)
            {
                int NewWidth = (TextPtr->HPos - HPosMin) + (abs(TextPtr->Font->Height) * TextPtr->TextString.Length() * 0.7);
                int NewHeight = (TextPtr->VPos - VPosMin) + (abs(TextPtr->Font->Height) * 1.5);
                if(NewWidth > RailwayImage->Width)
                {
                    RailwayImage->Width = NewWidth;
                }
                if(NewHeight > RailwayImage->Height)
                {
                    RailwayImage->Height = NewHeight;
                }
            }
        }

        RailwayImage->Canvas->Brush->Color = clB5G5R5; // set it all to white initially
        TRect Rect(0, 0, RailwayImage->Width, RailwayImage->Height);
        RailwayImage->Canvas->FillRect(Rect);

        // write the grid first so all else on top
        for(int x = 0; x < ((RailwayImage->Width) / 16); x++)
        {
            for(int y = 0; y < ((RailwayImage->Height) / 16); y++)
            {
                RailwayImage->Canvas->Draw((x * 16), (y * 16), RailGraphics->bmGrid); // graphic is black on white so no need to change
            }
        }
        // write graphics next so text & track overwrite
        Track->WriteGraphicsToImage(1, RailwayImage);
        // then write text so track overwrites
        TextHandler->WriteTextToImage(1, RailwayImage);
        Track->WriteTrackToImage(1, RailwayImage);
        RailwayImage->SaveToFile(ImageFileName);
        delete RailwayImage;
        TrainController->BaseTime = TDateTime::CurrentDateTime();
        TrainController->StopTTClockFlag = false;
        Screen->Cursor = TCursor(-2); // Arrow
        Utilities->CallLogPop(1536);
    }
    catch(const Exception &e)
    {
        if(e.Message.Pos("torage") > 0) // 'storage', avoid capitals as may be OS dependent
        {
            Screen->Cursor = TCursor(-2); // Arrow;
            UnicodeString MessageStr = "Insufficient memory available to store this image";
            Application->MessageBox(MessageStr.c_str(), L"", MB_OK | MB_ICONWARNING);
        }
        else
        {
            ErrorLog(43, e.Message);
        }
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::SaveImageAndPrefDirsMenuItemClick(TObject *Sender)
{ // need to stop clock in case invoke during operation
    try
    {
        TrainController->LogEvent("SaveImageAndPrefDirsMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",SaveImageAndPrefDirsMenuItemClick");
        if(!DirectoryExists(CurDir + "\\" + IMAGE_DIR_NAME))
        {
            ShowMessage("Failed to find folder " + IMAGE_DIR_NAME + " in the folder where 'railway.exe' resides.  Image can't be saved");
            Utilities->CallLogPop(1697);
            return;
        }
        Screen->Cursor = TCursor(-11); // Hourglass;
        TrainController->StopTTClockFlag = true; // so TTClock stopped during MasterClockTimer function
        TrainController->RestartTime = TrainController->TTClockTime;
        AnsiString ImageFileName = TDateTime::CurrentDateTime().FormatString("dd-mm-yyyy hh.nn.ss");
        // format "16/06/2009 20:55:17"
        // avoid characters in filename:=   / \ : * ? " < > |
        ImageFileName = CurDir + "\\" + IMAGE_DIR_NAME + "\\RailwayImage " + ImageFileName + "; " + RailwayTitle + ".bmp";
        AnsiString ShortName = "";
        for(int x = ImageFileName.Length(); x > 0; x--)
        {
            if(ImageFileName[x] == '\\')
            {
                ShortName = ImageFileName.SubString(x + 1, ImageFileName.Length() - x - 4);
                break;
            }
        }
        ShowMessage("A bitmap file named " + ShortName + " will be created in the Images folder");
        Graphics::TBitmap *RailwayImage = new Graphics::TBitmap;
        RailwayImage->PixelFormat = pf8bit; // needed to ensure compatibility with track
        int HPosMin = Track->GetHLocMin() * 16;
        int HPosMax = (Track->GetHLocMax() + 1) * 16;
        int VPosMin = Track->GetVLocMin() * 16;
        int VPosMax = (Track->GetVLocMax() + 1) * 16;
        RailwayImage->Width = HPosMax - HPosMin;
        RailwayImage->Height = VPosMax - VPosMin;

        // need to check if there is any text that extends past HPosMax or below VPosMax
        // use font height x text length x .7 for new rh calc & font height * 1.5 for new bottom calc
        if(!TextHandler->TextVector.empty())
        {
            for(TTextHandler::TTextVectorIterator TextPtr = TextHandler->TextVector.begin(); TextPtr != TextHandler->TextVector.end(); TextPtr++)
            {
                int NewWidth = (TextPtr->HPos - HPosMin) + (abs(TextPtr->Font->Height) * TextPtr->TextString.Length() * 0.7);
                int NewHeight = (TextPtr->VPos - VPosMin) + (abs(TextPtr->Font->Height) * 1.5);
                if(NewWidth > RailwayImage->Width)
                {
                    RailwayImage->Width = NewWidth;
                }
                if(NewHeight > RailwayImage->Height)
                {
                    RailwayImage->Height = NewHeight;
                }
            }
        }

        RailwayImage->Canvas->Brush->Color = clB5G5R5; // set it all to white initially
        TRect Rect(0, 0, RailwayImage->Width, RailwayImage->Height);
        RailwayImage->Canvas->FillRect(Rect);

        // write graphics first so text & track overwrite
        Track->WriteGraphicsToImage(2, RailwayImage);
        // then write text so track overwrites
        TextHandler->WriteTextToImage(2, RailwayImage);
        Track->WriteTrackToImage(2, RailwayImage);
        EveryPrefDir->WritePrefDirToImage(0, RailwayImage);
        RailwayImage->SaveToFile(ImageFileName);
        delete RailwayImage;
        TrainController->BaseTime = TDateTime::CurrentDateTime();
        TrainController->StopTTClockFlag = false;
        Screen->Cursor = TCursor(-2); // Arrow
        Utilities->CallLogPop(1566);
    }
    catch(const Exception &e)
    {
        if(e.Message.Pos("torage") > 0) // 'storage', avoid capitals as may be OS dependent
        {
            Screen->Cursor = TCursor(-2); // Arrow;
            UnicodeString MessageStr = "Insufficient memory available to store this image";
            Application->MessageBox(MessageStr.c_str(), L"", MB_OK | MB_ICONWARNING);
        }
        else
        {
            ErrorLog(45, e.Message);
        }
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::SaveOperatingImageMenuItemClick(TObject *Sender)
{ // need to stop clock
    try
    {
        TrainController->LogEvent("SaveOperatingImageMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",SaveOperatingImageMenuItemClick");
        if(!DirectoryExists(CurDir + "\\" + IMAGE_DIR_NAME))
        {
            ShowMessage("Failed to find folder " + IMAGE_DIR_NAME + " in the folder where 'railway.exe' resides.  Image can't be saved");
            Utilities->CallLogPop(1702);
            return;
        }
        Screen->Cursor = TCursor(-11); // Hourglass;
        TrainController->StopTTClockFlag = true; // so TTClock stopped during MasterClockTimer function
        TrainController->RestartTime = TrainController->TTClockTime;

        AnsiString TimetableTimeStr = Utilities->Format96HHMMSS(TrainController->TTClockTime);
        TimetableTimeStr = TimetableTimeStr.SubString(1, 2) + '.' + TimetableTimeStr.SubString(4, 2) + '.' + TimetableTimeStr.SubString(7, 2);
        AnsiString ImageFileName = TDateTime::CurrentDateTime().FormatString("dd-mm-yyyy hh.nn.ss");
        // format "16/06/2009 20:55:17"
        // avoid characters in filename:=   / \ : * ? " < > |
        ImageFileName = CurDir + "\\" + IMAGE_DIR_NAME + "\\RailwayImage " + ImageFileName + "; Timetable time " + TimetableTimeStr + "; " + RailwayTitle +
            "; " + TimetableTitle + ".bmp";
        AnsiString ShortName = "";
        for(int x = ImageFileName.Length(); x > 0; x--)
        {
            if(ImageFileName[x] == '\\')
            {
                ShortName = ImageFileName.SubString(x + 1, ImageFileName.Length() - x - 4);
                break;
            }
        }
        ShowMessage("A bitmap file named " + ShortName + " will be created in the Images folder");
        Graphics::TBitmap *RailwayImage = new Graphics::TBitmap;
        RailwayImage->PixelFormat = pf8bit; // needed to ensure compatibility with track
        int HPosMin = Track->GetHLocMin() * 16;
        int HPosMax = (Track->GetHLocMax() + 1) * 16;
        int VPosMin = Track->GetVLocMin() * 16;
        int VPosMax = (Track->GetVLocMax() + 1) * 16;
        RailwayImage->Width = HPosMax - HPosMin;
        RailwayImage->Height = VPosMax - VPosMin;

        // need to check if there is any text that extends past HPosMax or below VPosMax
        // use font height x text length x .7 for new rh calc & font height * 1.5 for new bottom calc
        if(!TextHandler->TextVector.empty())
        {
            for(TTextHandler::TTextVectorIterator TextPtr = TextHandler->TextVector.begin(); TextPtr != TextHandler->TextVector.end(); TextPtr++)
            {
                int NewWidth = (TextPtr->HPos - HPosMin) + (abs(TextPtr->Font->Height) * TextPtr->TextString.Length() * 0.7);
                int NewHeight = (TextPtr->VPos - VPosMin) + (abs(TextPtr->Font->Height) * 1.5);
                if(NewWidth > RailwayImage->Width)
                {
                    RailwayImage->Width = NewWidth;
                }
                if(NewHeight > RailwayImage->Height)
                {
                    RailwayImage->Height = NewHeight;
                }
            }
        }

        RailwayImage->Canvas->Brush->Color = clB5G5R5; // set it all to white initially
        TRect Rect(0, 0, RailwayImage->Width, RailwayImage->Height);
        RailwayImage->Canvas->FillRect(Rect);

        // write graphics first so text & track overwrite
        Track->WriteGraphicsToImage(3, RailwayImage);
        // then write text so track overwrites
        TextHandler->WriteTextToImage(3, RailwayImage);
        Track->WriteOperatingTrackToImage(0, RailwayImage); // need points with single fillets, signals with colours, gaps all connected
        AllRoutes->WriteAllRoutesToImage(0, RailwayImage);
// add any locked route markers
        if(!AllRoutes->LockedRouteVector.empty())
        {
            for(TAllRoutes::TLockedRouteVectorIterator LRVIT = AllRoutes->LockedRouteVector.end() - 1; LRVIT >= AllRoutes->LockedRouteVector.begin(); LRVIT--)
            {
                TOneRoute Route = AllRoutes->GetFixedRouteAt(167, LRVIT->RouteNumber);
                int x = Route.PrefDirSize() - 1;
                bool BreakFlag = false;
                TPrefDirElement PrefDirElement = Route.GetFixedPrefDirElementAt(188, x);
                while(PrefDirElement.GetTrackVectorPosition() != LRVIT->TruncateTrackVectorPosition)
                {
                    RailwayImage->Canvas->Draw((PrefDirElement.HLoc - Track->GetHLocMin()) * 16, (PrefDirElement.VLoc - Track->GetVLocMin()) * 16,
                        RailGraphics->LockedRouteCancelPtr[PrefDirElement.GetELink()]);
                    if(!(AllRoutes->TrackIsInARoute(13, PrefDirElement.Conn[PrefDirElement.GetELinkPos()],
                        PrefDirElement.ConnLinkPos[PrefDirElement.GetELinkPos()])))
                    {
                        BreakFlag = true;
                        break; // train removed earlier element from route so stop here
                    }
                    x--;
                    PrefDirElement = Route.GetFixedPrefDirElementAt(180, x);
                }
                if(!BreakFlag)
                {
                    if(PrefDirElement.GetTrackVectorPosition() == LRVIT->TruncateTrackVectorPosition)
                    {
                        RailwayImage->Canvas->Draw((PrefDirElement.HLoc - Track->GetHLocMin()) * 16, (PrefDirElement.VLoc - Track->GetVLocMin()) * 16,
                            RailGraphics->LockedRouteCancelPtr[PrefDirElement.GetELink()]);
                    }
                }
            }
        }
        TrainController->WriteTrainsToImage(0, RailwayImage);
        RailwayImage->SaveToFile(ImageFileName);
        delete RailwayImage;
        TrainController->BaseTime = TDateTime::CurrentDateTime();
        TrainController->StopTTClockFlag = false;
        Screen->Cursor = TCursor(-2); // Arrow
        Utilities->CallLogPop(1703);
    }
    catch(const Exception &e)
    {
        if(e.Message.Pos("torage") > 0) // 'storage', avoid capitals as may be OS dependent
        {
            Screen->Cursor = TCursor(-2); // Arrow;
            UnicodeString MessageStr = "Insufficient memory available to store this image";
            Application->MessageBox(MessageStr.c_str(), L"", MB_OK | MB_ICONWARNING);
        }
        else
        {
            ErrorLog(113, e.Message);
        }
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::SaveHeaderMenu1Click(TObject *Sender)
{
//
    try
    {
        TrainController->LogEvent("SaveHeaderMenu1Click");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",SaveHeaderMenu1Click");
        if(Sender == SaveSessionButton)
        {
            SaveSessionFlag = true;
        }
        else if(SavedFileName == "") // use 'Save As' function
        {
            SaveAsSubroutine(1);
        }
        else // ordinary save
        {
            Screen->Cursor = TCursor(-11); // Hourglass;
            std::ofstream VecFile(SavedFileName.c_str());
            if(!(VecFile.fail()))
            {
                Utilities->SaveFileString(VecFile, ProgramVersion);
                Utilities->SaveFileInt(VecFile, Display->DisplayOffsetHHome);
                Utilities->SaveFileInt(VecFile, Display->DisplayOffsetVHome);
                // save track elements
                if(Track->UserGraphicVector.empty())
                {
                    Track->SaveTrack(9, VecFile, false); // false for no graphics (**Active elements** saved as marker)
                }
                else
                {
                    Track->SaveTrack(10, VecFile, true); // true for graphics to be saved (**Active elements**1 saved as marker)
                }
                // save text elements
                TextHandler->SaveText(5, VecFile);
                // save PrefDir elements
                EveryPrefDir->SavePrefDirVector(8, VecFile);
                if(!Track->UserGraphicVector.empty())
                {
                    // save user graphics
                    Track->SaveUserGraphics(1, VecFile);
                }
                FileChangedFlag = false;
                VecFile.close();
            }
            else
                ShowMessage("Railway failed to save - can't open file");
            Screen->Cursor = TCursor(-2); // Arrow
        }
        Utilities->CallLogPop(1552);
    }
    catch(const Exception &e)
    {
        ErrorLog(44, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::LoadSessionMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("LoadSessionMenuItemClick");
        LoadSessionFlag = true; // load session within ClockTimer2
    }
    catch(const Exception &e)
    {
        ErrorLog(136, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::ClearAllMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("ClearAllMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",ClearAllMenuItemClick");
        if(ClearEverything(2))
        {;
        } // no change in action on result
        Level1Mode = BaseMode;
        SetLevel1Mode(126);
        Utilities->CallLogPop(1179);
    }
    catch(const Exception &e)
    {
        ErrorLog(137, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::ExportTTMenuItemClick(TObject *Sender)
{ // no need to stop clock as can't be called when railway operating
    try
    {
        TrainController->LogEvent("ExportTTMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",ExportTTMenuItemClick");
        if(!DirectoryExists(CurDir + "\\" + FORMATTEDTT_DIR_NAME))
        {
            ShowMessage("Failed to find folder " + FORMATTEDTT_DIR_NAME + " in the folder where 'railway.exe' resides.  Timetable can't be exported");
            Utilities->CallLogPop(1699);
            return;
        }
        Screen->Cursor = TCursor(-11); // Hourglass;
        // no need to stop clock as can't select this if operating
        TrainController->CreateFormattedTimetable(0, RailwayTitle, TimetableTitle, CurDir);
        Screen->Cursor = TCursor(-2); // Arrow
        Utilities->CallLogPop(1573);
    }
    catch(const Exception &e)
    {
        ErrorLog(138, e.Message);
    }
}
// ---------------------------------------------------------------------------
// Timetable editing functions

/* Note that during early development the timetable was created outside the program as a .csv file using Excel, it was only later that
   the editing functions within the program were developed.  Much of the original structure was preserved though to avoid rewriting the
   code interpretation functions in TrainUnit.cpp.  This is why commas are used as service event separators, and why it is necessary to
   convert them to CRLFs for display and back again for internal storage.  It is acknowledged that all this makes the editing functions
   somewhat cumbersome, and, as ever, if I was starting again I wouldn't do it like that!

   CR & LF review:
   These cause problems by the way that different subroutines handle them.

   AnsiStrings can incorporate CRLFs, but the end of an AnsiString is marked by a '\0' character as in 'C' strings.

   In the fstream functions 'getline(char_type* s, streamsize n)' extracts characters from the stream and puts them in buffer 's' until
   (a) n-1 characters are stored + '\0' after the n-1 characters;
   (b) a '\n' (CRLF) character is found in the stream, in which case a '\0' is added to the buffer after the text that immediately
   precedes the CRLF in the stream; and
   (c) an eof() is found in the stream, in which case a '\0' is added to the buffer at the end of the text.
   Note that if no characters are stored a '\0' is still stored in position [0] of the buffer.

   The << operator in ofstreams, when used with a null terminated string, doesn't store the null.  If it is required it has to be
   sent explicitly, e.g. file << '\0'.  Presumably the same applies for CRLF terminated strings.

*/
// ---------------------------------------------------------------------------

void __fastcall TInterface::CreateTimetableMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("CreateTimetableMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",CreateTimetableMenuItemClick");
        CreateEditTTFileName = "";
        TimetableEditVector.clear();
        TimetableEditPanel->Visible = true;
        HighlightPanel->Visible = false;
        TimetablePanel->Visible = true;
        TimetablePanel->BringToFront(); // in case SaveRailway button visible, want it hidden else obscures the panel text
        ShowHideTTButton->Glyph->LoadFromResourceName(0, "Hide");
        OneEntryTimetableMemo->Clear();
        AllEntriesTTListBox->Clear();
        TTStartTimeBox->Text = "";
        AddSubMinsBox->Text = "";
        OneEntryTimetableContents = "";
        LocationNameComboBox->Clear();
        TimetableTitle = ""; // unload any loaded timetable.  Added here at v2.1.0
        TrainController->TrainDataVector.clear(); // unload any loaded timetable.    Added here at v2.1.0
        SetCaption(9); // added at v2.1.0 as formerly retained earlier loaded tt name in error
        TimetableChangedFlag = false;
        TimetableValidFlag = false;
        TTEntryChangedFlag = false;
        TimetableChangedInAZOrderFlag = false;
        AZOrderButton->Caption = AnsiString("A-Z Order");
        AZOrderButton->Hint = AnsiString("Arrange services in alphabetical order       Toggle with Shift+ Z");
        CopiedEntryFlag = false;
        NewEntryInPreparationFlag = false;
        CopiedEntryStr = "";
        TEVPtr = 0;
        TTCurrentEntryPtr = 0, TTStartTimePtr = 0;
        TTFirstServicePtr = 0;
        TTLastServicePtr = 0; // all set to null to begin with

// populate LocationNameComboBox if a railway is loaded, but first compile the ActiveTrackElementNameMap
        TTrack::TActiveTrackElementNameMapEntry ActiveTrackElementNameMapEntry;
        Track->ActiveTrackElementNameMap.clear();
        for(unsigned int x = 0; x < Track->TrackVector.size(); x++)
        {
            if((Track->TrackVector.at(x).ActiveTrackElementName != "") && (Track->ContinuationNameMap.find(Track->TrackVector.at(x).ActiveTrackElementName))
                == Track->ContinuationNameMap.end())
            { // exclude any name that appears in a continuation, error message given in tt validation if try to include such a name in a tt
                ActiveTrackElementNameMapEntry.first = Track->TrackVector.at(x).ActiveTrackElementName;
                ActiveTrackElementNameMapEntry.second = 0; // this is a dummy value
                Track->ActiveTrackElementNameMap.insert(ActiveTrackElementNameMapEntry);
            }
        }
        Track->ActiveTrackElementNameMapCompiledFlag = true;
        if(!(Track->ActiveTrackElementNameMap.empty()))
        {
            LocationNameComboBox->Text = "Location names";
// new version at beta v0.2b
            for(TTrack::TActiveTrackElementNameIterator ATENIT = Track->ActiveTrackElementNameMap.begin(); ATENIT != Track->ActiveTrackElementNameMap.end();
            ATENIT++)
            {
                LocationNameComboBox->Items->Add(ATENIT->first); // continuations excluded during compilation, but a location that includes
                                                                 // continuations as well as other track will be included - earlier version
                                                                 // would have excluded them
            }

/* old version using LocationNameMultiMap, changed to use ActiveTrackElementNames to avoid including lone concourses and named non-station
    locations
        TStringList *StringList = new TStringList;
        StringList->Clear();//probably already empty but help file doesn't say so
        StringList->Sorted = false;//for now
        for(TTrack::TLocationNameMultiMapIterator LNMIT = Track->LocationNameMultiMap.begin(); LNMIT != Track->LocationNameMultiMap.end(); LNMIT++)
            {
            NewKey = LNMIT->first;
            if(OldKey != NewKey)//only add new values
                {
                if(Track->ContinuationNameMap.find(NewKey) == Track->ContinuationNameMap.end())//not a continuation
                    {
                    StringList->Add(NewKey);
                    OldKey = NewKey;
                    }
                }
            }
        StringList->Sort();
        for(int x=0;x<StringList->Count;x++)
            {
            LocationNameComboBox->Items->Add(StringList->Strings[x]);
            }
        delete StringList;
*/
        }
        else
        {
            LocationNameComboBox->Text = "No locations (listed when a railway with names is loaded)";
        }
        Level1Mode = TimetableMode;
        SetLevel1Mode(82);
        Utilities->CallLogPop(1595);
    }
    catch(const Exception &e)
    {
        ErrorLog(47, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::EditTimetableMenuItemClick(TObject *Sender)
/* The .ttb file contains a sequence of AnsiStrings separated by null characters.  CRLFs may be embedded within the AnsiStrings,
                 * to cause newlines when displayed.  Each AnsiString corresponds to a timetable 'entry'
*/
{
    try
    {
        TrainController->LogEvent("EditTimetableMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",EditTimetableMenuItemClick");
        TimetableDialog->Filter = "Timetable file (*.ttb)|*ttb";
        CreateEditTTFileName = "";
        TimetableEditVector.clear();
        TimetableEditPanel->Visible = true;
        HighlightPanel->Visible = false;
        TimetablePanel->Visible = true;
        TimetablePanel->BringToFront(); // in case SaveRailway button visible, want it hidden else obscures the panel text
        ShowHideTTButton->Glyph->LoadFromResourceName(0, "Hide");
        OneEntryTimetableMemo->Clear();
        AllEntriesTTListBox->Clear();
        TTStartTimeBox->Text = "";
        AddSubMinsBox->Text = "";
        OneEntryTimetableContents = "";
        LocationNameComboBox->Clear();
        TimetableTitle = ""; // unload any loaded timetable.  Moved here from below at v2.1.0 for consistency with CreateTimetable
        TrainController->TrainDataVector.clear(); // unload any loaded timetable.  Moved here from below at v2.1.0 for consistency with CreateTimetable
        SetCaption(8); // added at v2.1.0 as formerly retained earlier loaded tt name in error
        TEVPtr = 0;
        TTCurrentEntryPtr = 0, TTStartTimePtr = 0;
        TTFirstServicePtr = 0;
        TTLastServicePtr = 0; // all set to null to begin with
        if(TimetableDialog->Execute())
        {
            CreateEditTTFileName = AnsiString(TimetableDialog->FileName);
            TrainController->LogEvent("EditTimetable " + CreateEditTTFileName);
            std::ifstream TTBLFile(CreateEditTTFileName.c_str(), std::ios_base::binary); // open in binary to examine each character
            if(TTBLFile.is_open())
            {
                // check doesn't contain any non-ascii characters except CR, LF & '\0', and isn't empty
                char c;
                while(!TTBLFile.eof())
                {
                    TTBLFile.get(c);
                    if((c < 32) && (c != 13) && (c != 10) && (c != '\0')) // char is signed by default so values > 127 will be caught as treated as -ve
                    {
                        ShowMessage("Timetable file is empty or contains non-ascii characters, codes must be between 20 and 127, or CR or LF");
                        TTBLFile.close();
                        Utilities->CallLogPop(1612);
                        return;
                    }
                }
                TTBLFile.close();
            }
            else
            {
                ShowMessage("Failed to open timetable file, make sure it's not open in another application");
                Utilities->CallLogPop(1597);
                return;
            }
            // reopen again in binary mode so the "\r\n" pairs stay as they are rather than being entered as '\n'
            Delay(4, 100); // 100mSec delay between closing & re-opening file
            TTBLFile.open(CreateEditTTFileName.c_str(), std::ios_base::binary);
            if(TTBLFile.is_open())
            {
                TTBLFile.clear(); // to clear eofbit from last read
                TTBLFile.seekg(0); // shouldn't be needed but include for safety
                TimetableChangedFlag = false;
                TimetableValidFlag = false;
                TTEntryChangedFlag = false;
                TimetableChangedInAZOrderFlag = false;
                AZOrderButton->Caption = AnsiString("A-Z Order");
                AZOrderButton->Hint = AnsiString("Arrange services in alphabetical order       Toggle with Shift+ Z");
                NewEntryInPreparationFlag = false;
                CopiedEntryStr = "";
                CopiedEntryFlag = false;
// CreateEditTTFileName = TimetableDialog->FileName;
                for(int x = CreateEditTTFileName.Length(); x > 0; x--)
                {
                    if(CreateEditTTFileName[x] == '\\')
                    {
                        CreateEditTTTitle = CreateEditTTFileName.SubString(x + 1, CreateEditTTFileName.Length() - x - 4);
                        break;
                    }
                }
                char *TimetableEntryString = new char[10000];
                while(true)
                {
                    TTBLFile.getline(TimetableEntryString, 10000, '\0'); // pick up the entire AnsiString, including any embedded newlines
                    if(TTBLFile.eof() && (TimetableEntryString[0] == '\0')) // stores a null in 1st position if doesn't load any characters
                    { // may still have eof even if read a line, and
                                                                        // if so need to process it
                        break;
                    }
                    AnsiString OneLine(TimetableEntryString);
                    TimetableEditVector.push_back(OneLine);
                }
                TTBLFile.close();
                delete TimetableEntryString;
                // here with TimetableEditVector compiled
            }
            else
            {
                ShowMessage("Failed to open timetable file, make sure it's not open in another application");
                Utilities->CallLogPop(1654);
                return;
            }
        }
        else // cancelled dialog [section prior to CallLogPop added for v1.3.2 to clear timetable screen if cancel button pressed]
        {
            CreateEditTTFileName = "";
// set to null to allow a check during error file saving, if not null save the tt being edited to the file (see entry in ExitTTModeButtonClick)
            CreateEditTTTitle = ""; // as above
            Level1Mode = BaseMode;
            SetLevel1Mode(132);
            Utilities->CallLogPop(1633);
            return;
        }

        CompileAllEntriesMemoAndSetPointers(0);
        if(TimetableEditVector.empty())
        {
            Level1Mode = TimetableMode;
            SetLevel1Mode(89);
            Utilities->CallLogPop(1614);
            return;
        }

// all now set where can be
        TTCurrentEntryPtr = TimetableEditVector.begin();

// populate LocationNameComboBox if a railway is loaded, but first compile the ActiveTrackElementNameMap
        TTrack::TActiveTrackElementNameMapEntry ActiveTrackElementNameMapEntry;
        Track->ActiveTrackElementNameMap.clear();
        for(unsigned int x = 0; x < Track->TrackVector.size(); x++)
        {
            if((Track->TrackVector.at(x).ActiveTrackElementName != "") && (Track->ContinuationNameMap.find(Track->TrackVector.at(x).ActiveTrackElementName))
                == Track->ContinuationNameMap.end())
            { // exclude any name that appears in a continuation, error message given in tt validation if try to include such a name in a tt
                ActiveTrackElementNameMapEntry.first = Track->TrackVector.at(x).ActiveTrackElementName;
                ActiveTrackElementNameMapEntry.second = 0; // this is a dummy value
                Track->ActiveTrackElementNameMap.insert(ActiveTrackElementNameMapEntry);
            }
        }
        Track->ActiveTrackElementNameMapCompiledFlag = true;
        if(!(Track->ActiveTrackElementNameMap.empty()))
        {
            LocationNameComboBox->Text = "Location names";
// new version for beta v0.2b
            for(TTrack::TActiveTrackElementNameIterator ATENIT = Track->ActiveTrackElementNameMap.begin(); ATENIT != Track->ActiveTrackElementNameMap.end();
            ATENIT++)
            {
                LocationNameComboBox->Items->Add(ATENIT->first); // continuations excluded during compilation, but a location that includes
                                                                 // continuations as well as other track will be included - earlier version
                                                                 // would have excluded them
            }
        }
        else
        {
            LocationNameComboBox->Text = "No locations (listed when a railway with names is loaded)";
        }
        Level1Mode = TimetableMode;
        SetLevel1Mode(83);
        Utilities->CallLogPop(1596);
    }
    catch(const Exception &e)
    {
        ErrorLog(48, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::ShowHideTTButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("ShowHideTTButtonClick");
        if(TimetableEditPanel->Visible)
        {
            ShowHideTTButton->Glyph->LoadFromResourceName(0, "Show");
            TimetableEditPanel->Visible = false;
            ShowHideTTButton->Hint = "Show the timetable editor          Shift S";
// InfoPanel->Visible = false;                          //changed at v1.3.0 to make it clearer that still in TT mode
            InfoPanel->Caption = "Timetable mode: editor hidden"; // as above
        }
        else
        {
            ShowHideTTButton->Glyph->LoadFromResourceName(0, "Hide");
            TimetableEditPanel->Visible = true;
            ShowHideTTButton->Hint = "Hide the timetable editor to see the railway          Shift H";
            Level1Mode = TimetableMode;
            SetLevel1Mode(124);
        }
    }
    catch(const Exception &e)
    {
        ErrorLog(139, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::NextTTEntryButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("NextTTEntryButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",NextTTEntryButtonClick");
        if((TTCurrentEntryPtr == 0) || TimetableEditVector.empty())
        {
            Utilities->CallLogPop(1683);
            return;
        }
        if(TTCurrentEntryPtr < (TimetableEditVector.end() - 1))
            TTCurrentEntryPtr++;
        TTEntryChangedFlag = false;
        int TopPos = AllEntriesTTListBox->TopIndex; // need to store this & reset it after SetLevel1Mode to prevent the scroll
        // position changing in AllEntriesTTListBox
        Level1Mode = TimetableMode;
        SetLevel1Mode(85);
        if((TTCurrentEntryPtr - TimetableEditVector.begin()) < TopPos)
        {
            AllEntriesTTListBox->TopIndex = TTCurrentEntryPtr - TimetableEditVector.begin();
        }
        else if((TTCurrentEntryPtr - TimetableEditVector.begin()) > (TopPos + 45))
        {
            AllEntriesTTListBox->TopIndex = TTCurrentEntryPtr - TimetableEditVector.begin() - 45;
        }
        else
        {
            AllEntriesTTListBox->TopIndex = TopPos;
        }
        Utilities->CallLogPop(1605);
    }
    catch(const Exception &e)
    {
        ErrorLog(50, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::PreviousTTEntryButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("PreviousTTEntryButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",PreviousTTEntryButtonClick");
        if((TTCurrentEntryPtr == 0) || TimetableEditVector.empty())
        {
            Utilities->CallLogPop(1684);
            return;
        }
        if(TTCurrentEntryPtr > TimetableEditVector.begin())
            TTCurrentEntryPtr--;
        TTEntryChangedFlag = false;
        int TopPos = AllEntriesTTListBox->TopIndex; // need to store this & reset it after SetLevel1Mode to prevent the scroll
        // position changing in AllEntriesTTListBox
        Level1Mode = TimetableMode;
        SetLevel1Mode(86);
        if((TTCurrentEntryPtr - TimetableEditVector.begin()) < TopPos)
        {
            AllEntriesTTListBox->TopIndex = TTCurrentEntryPtr - TimetableEditVector.begin();
        }
        else if((TTCurrentEntryPtr - TimetableEditVector.begin()) > (TopPos + 45))
        {
            AllEntriesTTListBox->TopIndex = TTCurrentEntryPtr - TimetableEditVector.begin() - 45;
        }
        else
        {
            AllEntriesTTListBox->TopIndex = TopPos;
        }
        Utilities->CallLogPop(1607);
    }
    catch(const Exception &e)
    {
        ErrorLog(51, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::NewTTEntryButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("NewTTEntryButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",NewTTEntryButtonClick");
        OneEntryTimetableMemo->Clear();
        OneEntryTimetableMemo->SetFocus();
        NewEntryInPreparationFlag = true;
        Level1Mode = TimetableMode;
        SetLevel1Mode(103);
        Utilities->CallLogPop(1615);
    }
    catch(const Exception &e)
    {
        ErrorLog(52, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::AddMinsButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("AddMinsButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",AddMinsButtonClick");
        bool ValidFlag = true;
        for(int x = 1; x <= AddSubMinsBox->Text.Length(); x++)
        {
            if((AddSubMinsBox->Text[x] > '9') || (AddSubMinsBox->Text[x] < '0')) // tested in TTHandler but check here as a safeguard
            {
                ValidFlag = false;
                break;
            }
        }
        if(ValidFlag)
        {
            if(AddSubMinsBox->Text.ToInt() == 0)
                ValidFlag = false;
        }
        if((TTCurrentEntryPtr == 0) || (*TTCurrentEntryPtr == "") || (AddSubMinsBox->Text == "") || !ValidFlag)
        {
            Utilities->CallLogPop(1649);
            return;
        }
        TDateTime DummyTime;
        int AddMins = AddSubMinsBox->Text.ToInt();
        for(int x = 0; x < OneEntryTimetableMemo->Lines->Count; x++)
        {
            for(int y = 1; y < (OneEntryTimetableMemo->Lines->Strings[x].Length() - 3); y++)
            {
                if(TrainController->CheckTimeValidity(25, OneEntryTimetableMemo->Lines->Strings[x].SubString(y, 5), DummyTime))
                {
                    int Mins = OneEntryTimetableMemo->Lines->Strings[x].SubString(y + 3, 2).ToInt();
                    int Hrs = OneEntryTimetableMemo->Lines->Strings[x].SubString(y, 2).ToInt();
                    Mins += AddMins;
                    while(Mins >= 60)
                    {
                        Mins -= 60;
                        Hrs++;
                    }
                    if(Hrs > 95)
                    {
                        ShowMessage("One or more times excessive, not permitted to exceed 95 hours");
                        Utilities->CallLogPop(1650);
                        return;
                    }
                    AnsiString MinsStr = AnsiString(Mins), HrsStr = AnsiString(Hrs);
                    if(Mins < 10)
                        MinsStr = "0" + MinsStr;
                    if(Hrs < 10)
                        HrsStr = "0" + HrsStr;
                    int StrLength = OneEntryTimetableMemo->Lines->Strings[x].Length();
                    AnsiString NewString = OneEntryTimetableMemo->Lines->Strings[x].SubString(1, (y - 1)); // up to but not including the time
                    NewString += HrsStr + ':' + MinsStr;
                    NewString += OneEntryTimetableMemo->Lines->Strings[x].SubString((y + 5), (StrLength - y - 4));
                    OneEntryTimetableMemo->Lines->Strings[x] = NewString;
                }
            }
        }

        OneEntryTimetableMemo->HideSelection = true;
        OneEntryTimetableMemo->SelStart = 0; // need this & next command to set cursor to the top
        OneEntryTimetableMemo->SelLength = 0;
        TimetableValidFlag = false;
        TimetableChangedFlag = true;
        TTEntryChangedFlag = true;
        Level1Mode = TimetableMode;
        SetLevel1Mode(91);
        Utilities->CallLogPop(1617);
    }
    catch(const Exception &e)
    {
        ErrorLog(54, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::SubMinsButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("SubMinsButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",SubMinsButtonClick");
        bool ValidFlag = true;
        for(int x = 1; x <= AddSubMinsBox->Text.Length(); x++)
        {
            if((AddSubMinsBox->Text[x] > '9') || (AddSubMinsBox->Text[x] < '0')) // tested in TTHandler but check here as a safeguard
            {
                ValidFlag = false;
                break;
            }
        }
        if(ValidFlag)
        {
            if(AddSubMinsBox->Text.ToInt() == 0)
                ValidFlag = false;
        }
        if((TTCurrentEntryPtr == 0) || (*TTCurrentEntryPtr == "") || (AddSubMinsBox->Text == "") || !ValidFlag)
        {
            Utilities->CallLogPop(1659);
            return;
        }
        TDateTime DummyTime;
        int SubMins = AddSubMinsBox->Text.ToInt();
        for(int x = 0; x < OneEntryTimetableMemo->Lines->Count; x++)
        {
            for(int y = 1; y < (OneEntryTimetableMemo->Lines->Strings[x].Length() - 3); y++)
            {
                if(TrainController->CheckTimeValidity(28, OneEntryTimetableMemo->Lines->Strings[x].SubString(y, 5), DummyTime))
                {
                    int Mins = OneEntryTimetableMemo->Lines->Strings[x].SubString(y + 3, 2).ToInt();
                    int Hrs = OneEntryTimetableMemo->Lines->Strings[x].SubString(y, 2).ToInt();
                    Mins -= SubMins;
                    while(Mins < 0)
                    {
                        Mins += 60;
                        Hrs--;
                    }
                    if(Hrs < 0)
                    {
                        ShowMessage("One or more times are now before 00:00, this is not permitted");
                        Utilities->CallLogPop(1660);
                        return;
                    }
                    AnsiString MinsStr = AnsiString(Mins), HrsStr = AnsiString(Hrs);
                    if(Mins < 10)
                        MinsStr = "0" + MinsStr;
                    if(Hrs < 10)
                        HrsStr = "0" + HrsStr;
                    int StrLength = OneEntryTimetableMemo->Lines->Strings[x].Length();
                    AnsiString NewString = OneEntryTimetableMemo->Lines->Strings[x].SubString(1, (y - 1)); // up to but not including the time
                    NewString += HrsStr + ':' + MinsStr;
                    NewString += OneEntryTimetableMemo->Lines->Strings[x].SubString((y + 5), (StrLength - y - 4));
                    OneEntryTimetableMemo->Lines->Strings[x] = NewString;
                }
            }
        }
        OneEntryTimetableMemo->HideSelection = true;
        OneEntryTimetableMemo->SelStart = 0; // need this & next command to set cursor to the top
        OneEntryTimetableMemo->SelLength = 0;
        TimetableValidFlag = false;
        TimetableChangedFlag = true;
        TTEntryChangedFlag = true;
        Level1Mode = TimetableMode;
        SetLevel1Mode(92);
        Utilities->CallLogPop(1618);
    }
    catch(const Exception &e)
    {
        ErrorLog(55, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::CopyTTEntryButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("CopyTTEntryButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",CopyTTEntryButtonClick");
        if(TTCurrentEntryPtr == 0)
        {
            Utilities->CallLogPop(1636);
            return;
        }
        CopiedEntryStr = *TTCurrentEntryPtr;
        CopiedEntryFlag = true;
        int TopPos = AllEntriesTTListBox->TopIndex; // need to store this & reset it after SetLevel1Mode to prevent the scroll
                                                    // position changing in AllEntriesTTListBox
        Level1Mode = TimetableMode;
        SetLevel1Mode(93);
        if((TTCurrentEntryPtr - TimetableEditVector.begin()) < TopPos)
        {
            AllEntriesTTListBox->TopIndex = TTCurrentEntryPtr - TimetableEditVector.begin();
        }
        else if((TTCurrentEntryPtr - TimetableEditVector.begin()) > (TopPos + 45))
        {
            AllEntriesTTListBox->TopIndex = TTCurrentEntryPtr - TimetableEditVector.begin() - 45;
        }
        else
        {
            AllEntriesTTListBox->TopIndex = TopPos;
        }
        Utilities->CallLogPop(1619);
    }
    catch(const Exception &e)
    {
        ErrorLog(56, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::CutTTEntryButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("CutTTEntryButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",CutTTEntryButtonClick");
        if(TTCurrentEntryPtr == 0) // || (*TTCurrentEntryPtr == ""))//safeguard
        {
            Utilities->CallLogPop(1674);
            return;
        }
        CopiedEntryStr = *TTCurrentEntryPtr;
        CopiedEntryFlag = true;
        int OldVectorPos = TTCurrentEntryPtr - TimetableEditVector.begin(); // vector pointers unreliable after an erase,
                                                                            // so use the position in the vector
        TimetableEditVector.erase(TTCurrentEntryPtr);
// now need to rebuild all the pointers & the AllEntriesTTListBox so repeat the process from EditTimetableMenuItemClick
// pick up the start time if there is one
        TimetableChangedFlag = true;
        TimetableValidFlag = false;
        TTEntryChangedFlag = false;
        TEVPtr = 0;
        TTCurrentEntryPtr = 0, TTStartTimePtr = 0;
        TTFirstServicePtr = 0;
        TTLastServicePtr = 0; // all set to null to begin with
        int TopPos = AllEntriesTTListBox->TopIndex; // need to store this & reset it after SetLevel1Mode to prevent the scroll
                                                                                                                        // position changing in AllEntriesTTListBox
        AllEntriesTTListBox->Clear();
        CompileAllEntriesMemoAndSetPointers(10);
        if(TimetableEditVector.empty())
        {
            Level1Mode = TimetableMode;
            SetLevel1Mode(109);
            Utilities->CallLogPop(1777);
            return;
        }

// reset the TTCurrentEntryPtr to the Entry after the erased one if there is one
// but vector pointers unreliable after an erase, so use the position in the vector
        if(OldVectorPos >= TimetableEditVector.end() - TimetableEditVector.begin() - 1)
        {
            TTCurrentEntryPtr = TimetableEditVector.end() - 1;
        }
        else
        {
            TTCurrentEntryPtr = TimetableEditVector.begin() + OldVectorPos; // no need to add one as will be one further on because of erase
        }
        if(TTCurrentEntryPtr == 0)
        {
            OneEntryTimetableMemo->Clear();
        }
        Level1Mode = TimetableMode;
        SetLevel1Mode(115);
        if((TTCurrentEntryPtr - TimetableEditVector.begin()) < TopPos)
        {
            AllEntriesTTListBox->TopIndex = TTCurrentEntryPtr - TimetableEditVector.begin();
        }
        else if((TTCurrentEntryPtr - TimetableEditVector.begin()) > (TopPos + 45))
        {
            AllEntriesTTListBox->TopIndex = TTCurrentEntryPtr - TimetableEditVector.begin() - 45;
        }
        else
        {
            AllEntriesTTListBox->TopIndex = TopPos;
        }
        Utilities->CallLogPop(1676);
    }
    catch(const Exception &e)
    {
        ErrorLog(111, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::PasteTTEntryButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("PasteTTEntryButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",PasteTTEntryButtonClick");
        if(TTCurrentEntryPtr == 0) // || (CopiedEntryStr == "")) allow blank copies
        {
            Utilities->CallLogPop(1637);
            return;
        }
        int OldVectorPos = TTCurrentEntryPtr - TimetableEditVector.begin(); // vector pointers unreliable after an insert
        TimetableEditVector.insert(TTCurrentEntryPtr + 1, CopiedEntryStr); // inserts before the indicated pointer position, i.e. immediately
                                                                           // after the current Entry - may be at the end
        TimetableChangedFlag = true;
        TimetableValidFlag = false;
        TTEntryChangedFlag = false;
        TEVPtr = 0;
        TTCurrentEntryPtr = 0, TTStartTimePtr = 0;
        TTFirstServicePtr = 0;
        TTLastServicePtr = 0; // all set to null to begin with
        int TopPos = AllEntriesTTListBox->TopIndex; // need to store this & reset it after SetLevel1Mode to prevent the scroll
                                                                                                                        // position changing in AllEntriesTTListBox
        AllEntriesTTListBox->Clear();
        CompileAllEntriesMemoAndSetPointers(2);
        if(TimetableEditVector.empty())
        {
            Level1Mode = TimetableMode;
            SetLevel1Mode(110);
            Utilities->CallLogPop(1778);
            return;
        }
// restore TTCurrentEntryPtr
        TTCurrentEntryPtr = TimetableEditVector.begin() + OldVectorPos;
        TTCurrentEntryPtr++; // advance the pointer to the pasted entry
// CopiedEntryStr = "";//revert to null - no, allow multiple copies
        Level1Mode = TimetableMode;
        SetLevel1Mode(94);
        if((TTCurrentEntryPtr - TimetableEditVector.begin()) < TopPos)
        {
            AllEntriesTTListBox->TopIndex = TTCurrentEntryPtr - TimetableEditVector.begin();
        }
        else if((TTCurrentEntryPtr - TimetableEditVector.begin()) > (TopPos + 45))
        {
            AllEntriesTTListBox->TopIndex = TTCurrentEntryPtr - TimetableEditVector.begin() - 45;
        }
        else
        {
            AllEntriesTTListBox->TopIndex = TopPos;
        }
        Utilities->CallLogPop(1620);
    }
    catch(const Exception &e)
    {
        ErrorLog(57, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::DeleteTTEntryButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("DeleteTTEntryButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",DeleteTTEntryButtonClick");
        if(TTCurrentEntryPtr == 0)
        {
            Utilities->CallLogPop(1645);
            return;
        }
        UnicodeString MessageStr = "Are you sure this entry should be deleted?";
        int button = Application->MessageBox(MessageStr.c_str(), L"Please confirm", MB_YESNO);
        if(button == IDNO)
        {
            Utilities->CallLogPop(1663);
            return;
        }
        int OldVectorPos = TTCurrentEntryPtr - TimetableEditVector.begin(); // vector pointers unreliable after an erase,
                                                                            // so use the position in the vector
        TimetableEditVector.erase(TTCurrentEntryPtr);

// now need to rebuild all the pointers & the AllEntriesTTListBox so repeat the process from EditTimetableMenuItemClick
// pick up the start time if there is one
        TimetableChangedFlag = true;
        TimetableValidFlag = false;
        TTEntryChangedFlag = false;
        TEVPtr = 0;
        TTCurrentEntryPtr = 0, TTStartTimePtr = 0;
        TTFirstServicePtr = 0;
        TTLastServicePtr = 0; // all set to null to begin with
        int TopPos = AllEntriesTTListBox->TopIndex; // need to store this & reset it after SetLevel1Mode to prevent the scroll
                                                                                                                        // position changing in AllEntriesTTListBox
        AllEntriesTTListBox->Clear();
        CompileAllEntriesMemoAndSetPointers(3);
        if(TimetableEditVector.empty())
        {
            Level1Mode = TimetableMode;
            SetLevel1Mode(111);
            Utilities->CallLogPop(1779);
            return;
        }
// reset the TTCurrentEntryPtr to the Entry after the erased one if there is one
// but vector pointers unreliable after an erase, so use the position in the vector
        if(OldVectorPos >= TimetableEditVector.end() - TimetableEditVector.begin() - 1)
        {
            TTCurrentEntryPtr = TimetableEditVector.end() - 1;
        }
        else
        {
            TTCurrentEntryPtr = TimetableEditVector.begin() + OldVectorPos; // no need to add one as will be one further on because of erase
        }
        if(TTCurrentEntryPtr == 0)
        {
            OneEntryTimetableMemo->Clear();
        }
        Level1Mode = TimetableMode;
        SetLevel1Mode(95);
        if((TTCurrentEntryPtr - TimetableEditVector.begin()) < TopPos)
        {
            AllEntriesTTListBox->TopIndex = TTCurrentEntryPtr - TimetableEditVector.begin();
        }
        else if((TTCurrentEntryPtr - TimetableEditVector.begin()) > (TopPos + 45))
        {
            AllEntriesTTListBox->TopIndex = TTCurrentEntryPtr - TimetableEditVector.begin() - 45;
        }
        else
        {
            AllEntriesTTListBox->TopIndex = TopPos;
        }
        Utilities->CallLogPop(1621);
    }
    catch(const Exception &e)
    {
        ErrorLog(58, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::SaveTTEntryButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("SaveTTEntryButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",SaveTTEntryButtonClick");
/* allow blank lines to be saved
           AnsiString ContentStr = OneEntryTimetableMemo->Text;
           if((ContentStr == "\r\n") || (ContentStr == "\n") || (ContentStr == ""))
            {
            Utilities->CallLogPop(1679);
            return;
            }
*/
        AnsiString TempStr = "";
        bool ActiveLine = false;
        if(TTCurrentEntryPtr > 0)
        {
            if(*TTCurrentEntryPtr != "")
            {
                if((TTCurrentEntryPtr >= TTFirstServicePtr) && (TTCurrentEntryPtr <= TTLastServicePtr) && ((*TTCurrentEntryPtr)[1] != '*'))
                {
                    ActiveLine = true;
                    // need to add commas after each line in OneEntryTimetableMemo exept the last, where have '\0'
                    for(int x = 0; x < OneEntryTimetableMemo->Lines->Count; x++)
                    {
                        for(int y = 1; y <= OneEntryTimetableMemo->Lines->Strings[x].Length(); y++)
                        {
                            TempStr += OneEntryTimetableMemo->Lines->Strings[x][y];
                        }
                        if(x < (OneEntryTimetableMemo->Lines->Count - 1))
                        {
                            TempStr += ',';
                        }
                        // No need to add a '\n' as a '\0' is added automatically as a string delimiter.  If add '\n' then it is treated as a blank line and
                        // ends the timetable
                    }
                    // strip any excess commas from the end
                    if(TempStr != "")
                    {
                        while(TempStr[TempStr.Length()] == ',')
                        {
                            TempStr = TempStr.SubString(1, TempStr.Length() - 1);
                            if(TempStr == "")
                                break;
                        }
                    }
                }
            }
        }
        if(!ActiveLine)
        {
            TempStr = OneEntryTimetableMemo->Text; // Note that if the entry was intended as a service but goes in as plain text because
                                                   // the service & entry pointers aren't yet set, then CRLFs will be converted to commas in
                                                   // CompileAllEntriesMemoAndSetPointers if it appears after the start time
                                                   // and before a blank line or end of file, so the syntax check will work OK
        }
        if(AZOrderButton->Caption == AnsiString("Original Order"))
        {
            TimetableChangedInAZOrderFlag = true;
        }
        TimetableValidFlag = false;
        TimetableChangedFlag = true;
        TTEntryChangedFlag = false;
        int TopPos;
        if(TTCurrentEntryPtr == 0)
        {
            NewEntryInPreparationFlag = true;
        }
        if(!NewEntryInPreparationFlag)
        {
            (*TTCurrentEntryPtr) = TempStr;
            // need to reset the AllEntriesTTListBox in case the headcode has changed
            int OldVectorPos = TTCurrentEntryPtr - TimetableEditVector.begin(); // vector pointers unreliable after an insert
            TopPos = AllEntriesTTListBox->TopIndex; // need to store this & reset it after SetLevel1Mode to prevent the scroll
                                                                             // position changing in AllEntriesTTListBox
            AllEntriesTTListBox->Clear();
            CompileAllEntriesMemoAndSetPointers(4);
            if(TimetableEditVector.empty())
            {
                Level1Mode = TimetableMode;
                SetLevel1Mode(112);
                Utilities->CallLogPop(1780);
                return;
            }
            // restore TTCurrentEntryPtr
            TTCurrentEntryPtr = TimetableEditVector.begin() + OldVectorPos;
        }
        else
        {
            NewEntryInPreparationFlag = false;
            if(TTCurrentEntryPtr != 0)
            {
                int OldVectorPos = TTCurrentEntryPtr - TimetableEditVector.begin(); // vector pointers unreliable after an insert
                TimetableEditVector.insert(TTCurrentEntryPtr + 1, TempStr); // inserts before the indicated pointer position, which may be at the end
                TTCurrentEntryPtr = TimetableEditVector.begin() + OldVectorPos;
                TTCurrentEntryPtr++;
            }
            else
            {
                TimetableEditVector.insert(TimetableEditVector.end(), TempStr); // inserts before the indicated pointer position
                TTCurrentEntryPtr = TimetableEditVector.end() - 1;
            }
            int OldVectorPos = TTCurrentEntryPtr - TimetableEditVector.begin(); // save the current position
            TopPos = AllEntriesTTListBox->TopIndex; // need to store this & reset it after SetLevel1Mode to prevent the scroll
                                                                             // position changing in AllEntriesTTListBox
            AllEntriesTTListBox->Clear();
            CompileAllEntriesMemoAndSetPointers(5);
            if(TimetableEditVector.empty())
            {
                Level1Mode = TimetableMode;
                SetLevel1Mode(113);
                Utilities->CallLogPop(1781);
                return;
            }
// reset the TTCurrentEntryPtr after CompileAllEntriesMemoAndSetPointers
            if(OldVectorPos >= TimetableEditVector.end() - TimetableEditVector.begin() - 1)
            {
                TTCurrentEntryPtr = TimetableEditVector.end() - 1;
            }
            else
            {
                TTCurrentEntryPtr = TimetableEditVector.begin() + OldVectorPos;
            }
        }
        Level1Mode = TimetableMode;
        SetLevel1Mode(96);
        if((TTCurrentEntryPtr - TimetableEditVector.begin()) < TopPos)
        {
            AllEntriesTTListBox->TopIndex = TTCurrentEntryPtr - TimetableEditVector.begin();
        }
        else if((TTCurrentEntryPtr - TimetableEditVector.begin()) > (TopPos + 45))
        {
            AllEntriesTTListBox->TopIndex = TTCurrentEntryPtr - TimetableEditVector.begin() - 45;
        }
        else
        {
            AllEntriesTTListBox->TopIndex = TopPos;
        }
        Utilities->CallLogPop(1622);
    }
    catch(const Exception &e)
    {
        ErrorLog(59, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::SaveTTButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("SaveTTButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",SaveTTButtonClick");
        if(TimetableEditVector.empty())
        {
            ShowMessage("Timetable is empty, can't save an empty timetable");
            Utilities->CallLogPop(1685);
            return;
        }
        std::ofstream TTBLFile;
        if(CreateEditTTFileName != "")
        {
            TTBLFile.open(CreateEditTTFileName.c_str(), std::ios_base::binary); // if text then each time sees a "\r\n" pair enters "\r\n\n" because '\n'
            // on its own causes "\r\n' to ne inserted, binary just enters characters as they are
        }
        else
        {
            if(SaveTTDialog->Execute())
            {
                CreateEditTTFileName = AnsiString(SaveTTDialog->FileName);
                for(int x = CreateEditTTFileName.Length(); x > 0; x--)
                {
                    if(CreateEditTTFileName[x] == '\\')
                    {
                        CreateEditTTTitle = CreateEditTTFileName.SubString(x + 1, CreateEditTTFileName.Length() - x - 4);
                        break;
                    }
                }
                TTBLFile.open(CreateEditTTFileName.c_str(), std::ios_base::binary); // if text then each time sees a "\r\n" pair enters "\r\n\n" because '\n'
                // on its own causes "\r\n' to ne inserted, binary just enters characters as they are
            }
            else //cancelled dialog
            {
                Level1Mode = TimetableMode;
                SetLevel1Mode(137);
                Utilities->CallLogPop(2205);
                return;
            }
        }
        if(TTBLFile.is_open())
        {
            for(TEVPtr = TimetableEditVector.begin(); TEVPtr != TimetableEditVector.end(); TEVPtr++)
            {
                TTBLFile << (*TEVPtr).c_str() << '\0';
            }
            TimetableChangedFlag = false;
            TTBLFile.close();
        }
        else
        {
            ShowMessage(CreateEditTTFileName + " failed to open, ensure not already open in another application");
        }
        Level1Mode = TimetableMode;
        SetLevel1Mode(97);
        Utilities->CallLogPop(1623);
    }
    catch(const Exception &e)
    {
        ErrorLog(60, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::SaveTTAsButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("SaveTTAsButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",SaveTTAsButtonClick");
        if(TimetableEditVector.empty())
        {
            ShowMessage("Timetable is empty, can't save an empty timetable");
            Utilities->CallLogPop(1686);
            return;
        }
        std::ofstream TTBLFile;
        if(SaveTTDialog->Execute())
        {
            CreateEditTTFileName = SaveTTDialog->FileName;
            for(int x = SaveTTDialog->FileName.Length(); x > 0; x--)
            {
                if(SaveTTDialog->FileName[x] == '\\')
                {
                    CreateEditTTTitle = SaveTTDialog->FileName.SubString(x + 1, SaveTTDialog->FileName.Length() - x - 4);
                    break;
                }
            }
            TTBLFile.open(CreateEditTTFileName.c_str(), std::ios_base::binary); // if text then each time sees a "\r\n" pair enters "\r\n\n" because '\n'
            // on its own causes "\r\n' to ne inserted, binary just enters characters as they are
        }
        else //cancelled dialog
        {
            Level1Mode = TimetableMode;
            SetLevel1Mode(138);
            Utilities->CallLogPop(2206);
            return;
        }
        if(TTBLFile.is_open())
        {
            for(TEVPtr = TimetableEditVector.begin(); TEVPtr != TimetableEditVector.end(); TEVPtr++)
            {
                TTBLFile << (*TEVPtr).c_str() << '\0';
            }
            TimetableChangedFlag = false;
            TTBLFile.close();
        }
        else
        {
            ShowMessage(CreateEditTTFileName + " failed to open, ensure not already open in another application");
        }
        Level1Mode = TimetableMode;
        SetLevel1Mode(117);
        Utilities->CallLogPop(1667);
    }
    catch(const Exception &e)
    {
        ErrorLog(108, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::TTServiceSyntaxCheckButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("TTServiceSyntaxCheckButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",TTServiceSyntaxCheckButtonClick");
        int Count = 1; // anything > 0 OK as if 0 still seeking a start time
        bool EndOfFile = false;
        bool FinalCallFalse = false;
        bool GiveMessagesTrue = true;
        bool CheckLocationsExistInRailway = false;
        if(RlyFile)
            CheckLocationsExistInRailway = true;
// TrainController->AnyHeadCodeValid = true; //don't fail here because of an unrestricted headcode, if no good will find when validate (dropped at v0.6b)
        if(TrainController->ProcessOneTimetableLine(2, Count, *TTCurrentEntryPtr, EndOfFile, FinalCallFalse, GiveMessagesTrue, CheckLocationsExistInRailway))
        // return true for success
        {
            ShowMessage(
                "The basic syntax seems OK but this check is very limited.  Other aspects can only be checked by validating the whole timetable with the appropriate railway (.rly) loaded");
        }
        Level1Mode = TimetableMode;
        SetLevel1Mode(98);
        Utilities->CallLogPop(1624);
    }
    catch(const Exception &e)
    {
        ErrorLog(61, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::ValidateTimetableButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("ValidateTimetableButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",ValidateTimetableButtonClick");
        // reset all message flags, stops them being given twice   new at v2.4.0
        TrainController->SSHigh = false;
        TrainController->MRSHigh = false;
        TrainController->MRSLow = false;
        TrainController->MassHigh = false;
        TrainController->BFHigh = false;
        TrainController->BFLow = false;
        TrainController->PwrHigh = false;
        TrainController->SigSHigh = false;
        TrainController->SigSLow = false;
        if(CreateEditTTFileName == "")
        {
            Utilities->CallLogPop(1664);
            return;
        }
        bool CheckLocationsExistInRailwayTrue = true;
        if(TrainController->TimetableIntegrityCheck(2, CreateEditTTFileName.c_str(), true, CheckLocationsExistInRailwayTrue)) // messages  = true
        {
            Screen->Cursor = TCursor(-11); // Hourglass;
            std::ifstream TTBLFile(CreateEditTTFileName.c_str(), std::ios_base::binary);
            if(TTBLFile.is_open())
            {
                if(BuildTrainDataVectorForValidateFile(0, TTBLFile, true, CheckLocationsExistInRailwayTrue)) // messages  = true
                {
                    ShowMessage("Timetable integrity OK");
                    TimetableValidFlag = true;
// TrainController->TrainDataVector.clear(); keep this so can export a formatted tt
                };
            }
            else
            {
                ShowMessage("Failed to open timetable file, make sure it's not open in another application");
            }
            Screen->Cursor = TCursor(-2); // Arrow
        } // if(TimetableIntegrityCheck
        else
        {
// ShowMessage("Timetable preliminary integrity check failed"); dropped in v2.4.0 as messages given in all called functions
        }
        Level1Mode = TimetableMode;
        SetLevel1Mode(99);
        Utilities->CallLogPop(1625);
    }
    catch(const Exception &e)
    {
        ErrorLog(62, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::MoveTTEntryUpButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("MoveTTEntryUpButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",MoveTTEntryUpButtonClick");
        if(TTCurrentEntryPtr == 0)
        {
            Utilities->CallLogPop(1634);
            return;
        }
        if(TTCurrentEntryPtr < (TimetableEditVector.begin() + 1)) // shouldn't reach here but return if do
        {
            Utilities->CallLogPop(1632);
            return;
        }
        TEVPtr = TTCurrentEntryPtr - 1; // find earlier Entry
        AnsiString TempStr = *TEVPtr;
        *TEVPtr = *TTCurrentEntryPtr;
        *TTCurrentEntryPtr = TempStr;
        TTCurrentEntryPtr = TEVPtr;
        TimetableChangedFlag = true;
        TimetableValidFlag = false;

// now rebuild AllEntriesTTListBox, DisplayOneTTLineInPanel will highlight it
        int TopPos = AllEntriesTTListBox->TopIndex; // need to store this & reset it after SetLevel1Mode to prevent the scroll
                                                                          // position changing in AllEntriesTTListBox
        AllEntriesTTListBox->Clear();
        int OldVectorPos = TTCurrentEntryPtr - TimetableEditVector.begin(); // save the old position
        CompileAllEntriesMemoAndSetPointers(6);
// reset the TTCurrentEntryPtr after CompileAllEntriesMemoAndSetPointers
        if(OldVectorPos >= TimetableEditVector.end() - TimetableEditVector.begin() - 1)
        {
            TTCurrentEntryPtr = TimetableEditVector.end() - 1;
        }
        else
        {
            TTCurrentEntryPtr = TimetableEditVector.begin() + OldVectorPos;
        }
        Level1Mode = TimetableMode;
        SetLevel1Mode(100);
        if((TTCurrentEntryPtr - TimetableEditVector.begin()) < TopPos)
        {
            AllEntriesTTListBox->TopIndex = TTCurrentEntryPtr - TimetableEditVector.begin();
        }
        else if((TTCurrentEntryPtr - TimetableEditVector.begin()) > (TopPos + 45))
        {
            AllEntriesTTListBox->TopIndex = TTCurrentEntryPtr - TimetableEditVector.begin() - 45;
        }
        else
        {
            AllEntriesTTListBox->TopIndex = TopPos;
        }
        Utilities->CallLogPop(1626);
    }
    catch(const Exception &e)
    {
        ErrorLog(63, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::MoveTTEntryDownButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("MoveTTEntryDownButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",MoveTTEntryDownButtonClick");
        if(TTCurrentEntryPtr == 0)
        {
            Utilities->CallLogPop(1635);
            return;
        }
        if(TTCurrentEntryPtr >= (TimetableEditVector.end() - 1)) // shouldn't reach here but return if do
        {
            Utilities->CallLogPop(1678);
            return;
        }
        TEVPtr = TTCurrentEntryPtr + 1; // find later Entry
        AnsiString TempStr = *TEVPtr;
        *TEVPtr = *TTCurrentEntryPtr;
        *TTCurrentEntryPtr = TempStr;
        TTCurrentEntryPtr = TEVPtr;
        TimetableChangedFlag = true;
        TimetableValidFlag = false;

// now rebuild AllEntriesTTListBox, DisplayOneTTLineInPanel will highlight it
        int TopPos = AllEntriesTTListBox->TopIndex; // need to store this & reset it after SetLevel1Mode to prevent the scroll
                                                                          // position changing in AllEntriesTTListBox
        AllEntriesTTListBox->Clear();
        int OldVectorPos = TTCurrentEntryPtr - TimetableEditVector.begin(); // save the old position
        CompileAllEntriesMemoAndSetPointers(7);
// reset the TTCurrentEntryPtr after CompileAllEntriesMemoAndSetPointers
        if(OldVectorPos >= TimetableEditVector.end() - TimetableEditVector.begin() - 1)
        {
            TTCurrentEntryPtr = TimetableEditVector.end() - 1;
        }
        else
        {
            TTCurrentEntryPtr = TimetableEditVector.begin() + OldVectorPos;
        }
        Level1Mode = TimetableMode;
        SetLevel1Mode(101);
        if((TTCurrentEntryPtr - TimetableEditVector.begin()) < TopPos)
        {
            AllEntriesTTListBox->TopIndex = TTCurrentEntryPtr - TimetableEditVector.begin();
        }
        else if((TTCurrentEntryPtr - TimetableEditVector.begin()) > (TopPos + 45))
        {
            AllEntriesTTListBox->TopIndex = TTCurrentEntryPtr - TimetableEditVector.begin() - 45;
        }
        else
        {
            AllEntriesTTListBox->TopIndex = TopPos;
        }
        Utilities->CallLogPop(1627);
    }
    catch(const Exception &e)
    {
        ErrorLog(64, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::CancelTTEntryButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("CancelTTActionButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",CancelTTActionButtonClick");
        TTEntryChangedFlag = false;
        if(NewEntryInPreparationFlag)
        {
            NewEntryInPreparationFlag = false;
            OneEntryTimetableMemo->Clear();
        }
        int TopPos = AllEntriesTTListBox->TopIndex; // need to store this & reset it after SetLevel1Mode to prevent the scroll
                                                    // position changing in AllEntriesTTListBox
        Level1Mode = TimetableMode;
        SetLevel1Mode(102);
        if((TTCurrentEntryPtr - TimetableEditVector.begin()) < TopPos)
        {
            AllEntriesTTListBox->TopIndex = TTCurrentEntryPtr - TimetableEditVector.begin();
        }
        else if((TTCurrentEntryPtr - TimetableEditVector.begin()) > (TopPos + 45))
        {
            AllEntriesTTListBox->TopIndex = TTCurrentEntryPtr - TimetableEditVector.begin() - 45;
        }
        else
        {
            AllEntriesTTListBox->TopIndex = TopPos;
        }
        Utilities->CallLogPop(1630);
    }
    catch(const Exception &e)
    {
        ErrorLog(102, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::RestoreTTButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("RestoreTTButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",RestoreTTButtonClick");
        if(TimetableChangedFlag)
        {
            UnicodeString MessageStr = "All changes to the timetable will be lost - proceed?";
            int button = Application->MessageBox(MessageStr.c_str(), L"Please confirm", MB_YESNO);
            if(button == IDNO)
            {
                Utilities->CallLogPop(1651);
                return;
            }
        }

        // repeat from EditTimetableMenuItemClick, but no need to check for non-ascii characters
        // open in binary mode so the "\r\n" pairs stay as they are rather than being entered as '\n'
        std::ifstream TTBLFile(CreateEditTTFileName.c_str(), std::ios_base::binary);
        if(TTBLFile.is_open())
        {
            TimetableChangedFlag = false;
            TimetableValidFlag = false;
            TTEntryChangedFlag = false;
            NewEntryInPreparationFlag = false;
            CopiedEntryFlag = false;
            CopiedEntryStr = "";
            TimetableEditVector.clear();
            OneEntryTimetableMemo->Clear();
            AllEntriesTTListBox->Clear();
            TTStartTimeBox->Text = "";
            AddSubMinsBox->Text = "";
            TEVPtr = 0;
            TTCurrentEntryPtr = 0, TTStartTimePtr = 0;
            TTFirstServicePtr = 0;
            TTLastServicePtr = 0; // all set to null to begin with
            char *TimetableEntryString = new char[10000];
            while(true)
            {
                TTBLFile.getline(TimetableEntryString, 10000, '\0'); // pick up the entire AnsiString, including any embedded newlines
                if(TTBLFile.eof() && (TimetableEntryString[0] == '\0')) // stores a null in 1st position if doesn't load any characters
                { // may still have eof even if read a line, and
                                                                     // if so need to process it
                    break;
                }
                AnsiString OneLine(TimetableEntryString);
                TimetableEditVector.push_back(OneLine);
            }
            TTBLFile.close();
            delete TimetableEntryString;
            // here with TimetableEditVector compiled
        }
        else
        {
            ShowMessage("Failed to open timetable file, make sure it's not open in another application");
            Utilities->CallLogPop(1655);
            return;
        }

        CompileAllEntriesMemoAndSetPointers(8);
        if(TimetableEditVector.empty())
        {
            Level1Mode = TimetableMode;
            SetLevel1Mode(114);
            Utilities->CallLogPop(1782);
            return;
        }
// all now set where can be
        TTCurrentEntryPtr = TimetableEditVector.begin();
// end of repeat from EditTimetableMenuItemClick

        Level1Mode = TimetableMode;
        SetLevel1Mode(104);
        Utilities->CallLogPop(1652);
    }
    catch(const Exception &e)
    {
        ErrorLog(104, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::ExportTTButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("ExportTTButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",ExportTTButtonClick");
        if(!DirectoryExists(CurDir + "\\" + FORMATTEDTT_DIR_NAME))
        {
            ShowMessage("Failed to find folder " + FORMATTEDTT_DIR_NAME + " in the folder where 'railway.exe' resides.  Timetable can't be exported");
            Utilities->CallLogPop(1698);
            return;
        }
        Screen->Cursor = TCursor(-11); // Hourglass;
        AnsiString TTTitle;
        if(RlyFile && TimetableValidFlag && (CreateEditTTFileName != ""))
        {
            for(int x = CreateEditTTFileName.Length(); x > 0; x--) // first need to strip out the timetable title from the full name
            {
                if(CreateEditTTFileName[x] == '\\')
                {
                    TTTitle = CreateEditTTFileName.SubString(x + 1, CreateEditTTFileName.Length() - x - 4);
                    break;
                }
            }
            TrainController->CreateFormattedTimetable(1, RailwayTitle, TTTitle, CurDir);
        }
        Screen->Cursor = TCursor(-2); // Arrow
        Level1Mode = TimetableMode;
        SetLevel1Mode(116);
        Utilities->CallLogPop(1662);
    }
    catch(const Exception &e)
    {
        ErrorLog(107, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::TTTextButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("TTTextButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",TTTextButtonClick");
/*
           if(TTStartTimePtr == 0)
           {
            OneEntryTimetableMemo->Clear();
            TTStartTimeBox->SetFocus();
            Utilities->CallLogPop(1673);
            return;
            }
*/
        int SelPos = OneEntryTimetableMemo->SelStart;
        AnsiString FirstPart = OneEntryTimetableMemo->Text.SubString(1, SelPos);
        AnsiString LastPart = OneEntryTimetableMemo->Text.SubString(SelPos + 1, OneEntryTimetableMemo->Text.Length() - SelPos);
        OneEntryTimetableMemo->Text = FirstPart + ((TButton*)Sender)->Caption + LastPart;
        OneEntryTimetableMemo->SelStart = SelPos + ((TButton*)Sender)->Caption.Length();
        TTEntryChangedFlag = true;
        OneEntryTimetableMemo->SetFocus();
        Level1Mode = TimetableMode;
        SetLevel1Mode(119);
        Utilities->CallLogPop(1672);
    }
    catch(const Exception &e)
    {
        ErrorLog(110, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::ExitTTModeButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("ExitTTCreateEditButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",ExitTTCreateEditButtonClick");
        if(TimetableChangedFlag)
        {
            UnicodeString MessageStr = "The timetable has changed.\n\nAre you sure you want to exit without saving it?";
            int button = Application->MessageBox(MessageStr.c_str(), L"Please confirm", MB_YESNO);
            if(button == IDNO)
            {
                Utilities->CallLogPop(1603);
                return;
            }
        }
        TimetableChangedFlag = false;
        CreateEditTTFileName = ""; // set to null to allow a check during error file saving, if not null save the tt being edited to the file
                                   // added for Beta v0.2b
        CreateEditTTTitle = ""; // as above
        Level1Mode = BaseMode;
        SetLevel1Mode(84);
        Utilities->CallLogPop(1606);
    }
    catch(const Exception &e)
    {
        ErrorLog(49, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::LocationNameComboBoxClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("LocationNameComboBoxClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",LocationNameComboBoxClick");
        if(TTStartTimePtr != 0)
        {
            LocationNameComboBox->SelectAll();
            int SelPos = OneEntryTimetableMemo->SelStart;
            AnsiString FirstPart = OneEntryTimetableMemo->Text.SubString(1, SelPos);
            AnsiString LastPart = OneEntryTimetableMemo->Text.SubString(SelPos + 1, OneEntryTimetableMemo->Text.Length() - SelPos);
            OneEntryTimetableMemo->Text = FirstPart + LocationNameComboBox->SelText + LastPart;
            OneEntryTimetableMemo->SelStart = SelPos + LocationNameComboBox->SelText.Length();
            TTEntryChangedFlag = true;
            OneEntryTimetableMemo->SetFocus();
            Level1Mode = TimetableMode;
            SetLevel1Mode(118);
        }
        Utilities->CallLogPop(1669);
    }
    catch(const Exception &e)
    {
        ErrorLog(109, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::OneEntryTimetableMemoKeyUp(TObject *Sender, WORD &Key, TShiftState Shift)
{
    try
    {
// TrainController->LogEvent("OneEntryTimetableMemoKeyUp");  drop this - too many entries
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",OneEntryTimetableMemoKeyUp");
        if(OneEntryTimetableMemo->Text == OneEntryTimetableContents)
        {
            Utilities->CallLogPop(1716);
            return;
        }
        TimetableChangedFlag = true;
        TTEntryChangedFlag = true;
        TimetableValidFlag = false;
        Level1Mode = TimetableMode;
        SetLevel1Mode(127);
        Utilities->CallLogPop(1629);
    }
    catch(const Exception &e)
    {
        ErrorLog(66, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::AddSubMinsBoxKeyUp(TObject *Sender, WORD &Key, TShiftState Shift)
{
// forces a recheck for whether addmins/submins buttons should be enabled
    try
    {
        TrainController->LogEvent("AddSubMinsBoxKeyUp");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",AddSubMinsBoxKeyUp");
        Level1Mode = TimetableMode;
        SetLevel1Mode(108);
        Utilities->CallLogPop(1658);
    }
    catch(const Exception &e)
    {
        ErrorLog(106, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::LocationNameComboBoxKeyUp(TObject *Sender, WORD &Key, TShiftState Shift)
{
    try
    {
        TrainController->LogEvent("LocationNameComboBoxKeyUp");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",LocationNameComboBoxKeyUp");
        if(!Track->LocationNameMultiMap.empty())
        {
            LocationNameComboBox->Text = "Location names";
        }
        else
        {
            LocationNameComboBox->Text = "No locations (listed when a railway with names is loaded)";
        }
        Utilities->CallLogPop(1677);
    }
    catch(const Exception &e)
    {
        ErrorLog(112, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::AllEntriesTTListBoxMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
// Select the item pointed to unless a 'save entry' is pending in which case ignore
    try
    {
        TrainController->LogEvent("AllEntriesTTListBoxMouseUp," + AnsiString(X) + "," + AnsiString(Y));
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",AllEntriesTTListBoxMouseUp," + AnsiString(X) + "," + AnsiString(Y));
        if((TTCurrentEntryPtr == 0) || TimetableEditVector.empty())
        {
            Utilities->CallLogPop(1687);
            return;
        }
        if(TTEntryChangedFlag || NewEntryInPreparationFlag) // if a save/cancel pending don't permit anything else
        {
            Utilities->CallLogPop(1688);
            return;
        }
        // find item required - 13 pixels per line of text
        int TopPos = AllEntriesTTListBox->TopIndex; // need to store this & reset it after SetLevel1Mode to prevent the scroll
                                                    // position changing in AllEntriesTTListBox
        if((TopPos + (Y / 13)) >= AllEntriesTTListBox->Items->Count)
        {
            TTCurrentEntryPtr = TimetableEditVector.end() - 1;
        }
        else
        {
            TTCurrentEntryPtr = TimetableEditVector.begin() + (Y / 13) + TopPos;
        }
        int OldVectorPos = TTCurrentEntryPtr - TimetableEditVector.begin(); // save the old position
        CompileAllEntriesMemoAndSetPointers(9);
// reset the TTCurrentEntryPtr after CompileAllEntriesMemoAndSetPointers
        if(OldVectorPos >= TimetableEditVector.end() - TimetableEditVector.begin() - 1)
        {
            TTCurrentEntryPtr = TimetableEditVector.end() - 1;
        }
        else
        {
            TTCurrentEntryPtr = TimetableEditVector.begin() + OldVectorPos;
        }
        Level1Mode = TimetableMode;
        SetLevel1Mode(120);
        AllEntriesTTListBox->TopIndex = TopPos; // reset it after SetLevel1Mode to prevent the scroll position changing
        Utilities->CallLogPop(1648);
    }
    catch(const Exception &e)
    {
        ErrorLog(103, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::OAListBoxMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
// Mouseup rather than Mousedown so shows floating label when over train
    try
    {
        TrainController->LogEvent("OAListBoxMouseUp," + AnsiString(X) + "," + AnsiString(Y));
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",OAListBoxMouseUp," + AnsiString(X) + "," + AnsiString(Y));
        if(Track->RouteFlashFlag || Track->PointFlashFlag) // no action
        {
            Utilities->CallLogPop(2087);
            return;
        }
        if(TrainController->OpTimeToActMultiMap.empty())
        {
            Utilities->CallLogPop(2088);
            return;
        }
        TTrainController::TOpTimeToActMultiMapIterator OACurrentEntryPtr;
        // find item required - 13 pixels per line of text
        int TopPos = OAListBox->TopIndex;
        int OAIndex;
        if((TopPos + (Y / 13)) >= OAListBox->Items->Count) // if click beyond end of list ignore
        {
            Utilities->CallLogPop(2089);
            return;
        }
        else
        {
            OACurrentEntryPtr = TrainController->OpTimeToActMultiMap.begin();
            std::advance(OACurrentEntryPtr, ((Y / 13) + TopPos));
        }
        int HPos;
        int VPos;
        int TrackVectorPosition;
        int TrainIDorTVPos = OACurrentEntryPtr->second.second;
        if(TrainIDorTVPos >= 0) // running train, so value is the TrainID
        {
            if(TrainController->TrainExistsAtIdent(0, TrainIDorTVPos)) // added at v2.4.0 in case train removed but still in OA list as not updated yet
            // see LiWinDom error report on Discord 23/04/20. Also needed for click OAListBox before any trains show,
            // as notified by Rokas Serys by email on 16/05/20
            {
                TrackVectorPosition = TrainController->TrainVectorAtIdent(43, TrainIDorTVPos).LeadElement;
            }
            else
            {
                Utilities->CallLogPop(2155); // if not there then ignore
                return;
            }
        }
        else // train to enter at a continuation, so value is -TVPos of continuation - 1
        {
            TrackVectorPosition = -(TrainIDorTVPos + 1);
        }
        HPos = (Track->TrackElementAt(926, TrackVectorPosition).HLoc * 16);
        VPos = (Track->TrackElementAt(927, TrackVectorPosition).VLoc * 16);
        // now want to set the offsets to display HPos & VPos in the centre of the screen
        Display->DisplayOffsetH = (HPos - MainScreen->Width / 2) / 16; // ScreenPosH = HPos - (DisplayOffsetH * 16)
        Display->DisplayOffsetV = (VPos - MainScreen->Height / 2) / 16;
        int ScreenPosH = HPos - (Display->DisplayOffsetH * 16);
        int ScreenPosV = VPos - (Display->DisplayOffsetV * 16);
        if(Display->ZoomOutFlag) // panel displays in either zoom mode
        {
            Display->ZoomOutFlag = false;
            SetPausedOrZoomedInfoCaption(6);
        }
        ClearandRebuildRailway(72); // if was zoomed out this displays the track because until ZoomOutFlag reset PlotOutput plots nothing
        TPoint MainScreenPoint(ScreenPosH + 8, ScreenPosV + 8); // new v2.2.0 add 8 to centre pointer in element
        TPoint CursPos = MainScreen->ClientToScreen(MainScreenPoint); // accurate funtion to convert from local to global co-ordinates
        Mouse->CursorPos = CursPos;
        Utilities->CallLogPop(2090);
    }
    catch(const Exception &e)
    {
        ErrorLog(200, e.Message);
    }
}

// ---------------------------------------------------------------------------

void TInterface::CompileAllEntriesMemoAndSetPointers(int Caller)
{
    enum
    {
        PreStartTime, ActiveSegment, PostEnd} Segment;
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",CompileAllEntriesMemoAndSetPointers");
    AllEntriesTTListBox->Clear();
    TEVPtr = 0;
    TTStartTimePtr = 0;
    TTFirstServicePtr = 0;
    TTLastServicePtr = 0; // all set to null to begin with
    if(TimetableEditVector.empty())
    {
        TTCurrentEntryPtr = 0;
        Utilities->CallLogPop(1681);
        return;
    }
    Segment = PreStartTime;
    for(TEVPtr = TimetableEditVector.begin(); TEVPtr != TimetableEditVector.end(); TEVPtr++)
    {
        if(Segment == PreStartTime) // looking for the start time
        {
            TDateTime TempTime; // dummy
            if(TrainController->CheckTimeValidity(33, *TEVPtr, TempTime))
            {
                TTStartTimePtr = TEVPtr; // TTStartTimeBox text set in TTHandler
                AllEntriesTTListBox->Items->Add("START " + (*TEVPtr).SubString(1, 5));
                Segment = ActiveSegment;
                continue;
            }
            else
            {
                if(*TEVPtr == "")
                {
                    AllEntriesTTListBox->Items->Add("- Blank");
                }
                else
                {
                    AnsiString CurrentStr = *TEVPtr;
                    if(CurrentStr != "") // strip any non alphanumeric characters (specifically \r or \n)
                    {
                        CurrentStr = CurrentStr.SubString(1, 10); // limit length for LH window
                        for(int x = 1; x < CurrentStr.Length(); x++)
                        {
                            if((CurrentStr[x] < 32) || (CurrentStr[1] > 126))
                            {
                                CurrentStr = CurrentStr.SubString(1, (x - 1));
                            }
                        }
                    }
                    AllEntriesTTListBox->Items->Add("- " + CurrentStr);
                }
                continue;
            }
        }
        if(Segment == ActiveSegment)
        {
            if(*TEVPtr != "")
            {
                if((*TEVPtr)[1] != '*')
                {
                    TrainController->StripSpaces(5, *TEVPtr);
                    ConvertCRLFsToCommas(0, *TEVPtr); // This needed because an entry intended as a service might have skipped the conversion in
                    // SaveTTEntryButtonClick - see comment in that function
                    if(TTFirstServicePtr == 0)
                    {
                        TTFirstServicePtr = TEVPtr;
                    }
                    TTLastServicePtr = TEVPtr;
                }
                AnsiString Entry = *TEVPtr;
                if(Entry[1] == '*')
                    Entry = "Comment";
                else
                {
                    int SCPos = Entry.Pos(';'); // semicolon
                    int CPos = Entry.Pos(','); // comma
                    // 5 possibilities: no comma & no semicolon - just text - enter 1st 12 characters
                    // both, comma before semicolon, i.e text on its own line, semicolon on next line - e.g. service headcode but no
                    // description - enter the text up to the comma
                    // both, semicolon before comma, normal - enter text up to the semicolon
                    // comma & no semicolon, one or more lines of text without any semicolons - enter the text up to the comma
                    // semicolon & no comma - enter text up to the semicolon
                    if((CPos == 0) && (SCPos == 0))
                    {
                        Entry = Entry.SubString(1, 12);
                    }
                    else if((CPos > 0) && (SCPos > 0) && (CPos < SCPos))
                    {
                        Entry = Entry.SubString(1, CPos - 1);
                    }
                    else if((CPos > 0) && (SCPos > 0) && (CPos > SCPos))
                    {
                        Entry = Entry.SubString(1, SCPos - 1);
                    }
                    else if((CPos > 0) && (SCPos == 0))
                    {
                        Entry = Entry.SubString(1, CPos - 1);
                    }
                    else
                    {
                        Entry = Entry.SubString(1, SCPos - 1);
                    }
                }
                AllEntriesTTListBox->Items->Add(Entry);
                continue;
            }
            else
            {
                Segment = PostEnd;
                AllEntriesTTListBox->Items->Add("END (Blank)");
                continue;
            }
        }
        if(Segment == PostEnd)
        {
            if(*TEVPtr == "")
            {
                AllEntriesTTListBox->Items->Add("+ Blank");
            }
            else
            {
                AnsiString CurrentStr = *TEVPtr;
                if(CurrentStr != "") // strip any non alphanumeric characters (specifically \r or \n)
                {
                    CurrentStr = CurrentStr.SubString(1, 10);
                    for(int x = 1; x < CurrentStr.Length(); x++)
                    {
                        if((CurrentStr[x] < 32) || (CurrentStr[1] > 126))
                        {
                            CurrentStr = CurrentStr.SubString(1, (x - 1));
                        }
                    }
                }
                AllEntriesTTListBox->Items->Add("+ " + CurrentStr);
            }
            continue;
        }
    }
    if(TTStartTimePtr == 0)
    {
        TTStartTimeBox->Text = "";
    }
    TTCurrentEntryPtr = TTLastServicePtr; // may well be reset outside this function but need to ensure that it has a valid value on exit, even if it's null
    Utilities->CallLogPop(1680);
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::AZOrderButtonClick(TObject *Sender)
{
    try
    {
        if(TimetableEditVector.empty())
        {
            return; // should be able to access this if it is but keep in for safety
        }
        TrainController->LogEvent("AZOrderClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",AZOrderClick");
        if(AZOrderButton->Caption == AnsiString("A-Z Order"))
        {
            TTEVPtr SortStart, SortEnd;
            UnicodeString MessageStr =
                "If you wish to preserve the original order don't make any changes whilst in alphabetical order.\n\nIn that case use alphabetical order to find the service required, click it to display it, then revert to the original order where the same service will be displayed and can be changed.";
            Application->MessageBox(MessageStr.c_str(), L"Please Note:", MB_OK | MB_ICONWARNING);
            TTSelectedEntry = *TTCurrentEntryPtr;
            OriginalTimetableEditVector = TimetableEditVector;
            SortStart = TimetableEditVector.begin(); // if no start time set sort from beginning
            if(TTFirstServicePtr != NULL)
            {
                SortStart = TTFirstServicePtr;
            }
            SortEnd = TimetableEditVector.end(); // if no last service set sort to end
            if(TTLastServicePtr != NULL)
            {
                SortEnd = TTLastServicePtr + 1;
            }
            std::sort(SortStart, SortEnd);
            CompileAllEntriesMemoAndSetPointers(11);
            bool CurrentEntryChanged = false;
            for(TTimetableEditVector::iterator x = TimetableEditVector.begin(); x < TimetableEditVector.end(); x++)
            {
                if(TTSelectedEntry == *x)
                {
                    TTCurrentEntryPtr = x;
                    CurrentEntryChanged = true;
                }
            }
            if(!CurrentEntryChanged)
            {
                TTCurrentEntryPtr = TTStartTimePtr;
            }
            AZOrderButton->Caption = AnsiString("Original Order");
            AZOrderButton->Hint = AnsiString("Arrange services in original order       Toggle with Shift+ Z");
        }
        else
        {
            if(TimetableChangedInAZOrderFlag)
            {
                UnicodeString MessageStr =
                    "Reverting to the original order will discard any changes made whilst in alphabetical order.\n\nTo preserve the changes click 'No', then save the timetable or use 'save as' if you wish to keep the original timetable.\n\nDo you wish to proceed?";
                int button = Application->MessageBox(MessageStr.c_str(), L"Warning!", MB_YESNO | MB_ICONWARNING);
                if(button == IDNO)
                {
                    TimetableChangedFlag = true;
                    TimetableValidFlag = false;
                    Level1Mode = TimetableMode;
                    SetLevel1Mode(135);
                    Utilities->CallLogPop(2166);
                    return;
                }
            }
            TTSelectedEntry = *TTCurrentEntryPtr;
            TimetableEditVector = OriginalTimetableEditVector;
            CompileAllEntriesMemoAndSetPointers(12);
            bool CurrentEntryChanged = false;
            for(TTimetableEditVector::iterator x = TimetableEditVector.begin(); x < TimetableEditVector.end(); x++)
            {
                if(TTSelectedEntry == *x)
                {
                    TTCurrentEntryPtr = x;
                    CurrentEntryChanged = true;
                }
            }
            if(!CurrentEntryChanged)
            {
                TTCurrentEntryPtr = TTStartTimePtr;
            }
            AZOrderButton->Caption = AnsiString("A-Z Order");
            AZOrderButton->Hint = AnsiString("Arrange services in alphabetical order       Toggle with Shift+ Z");
        }
        TimetableChangedFlag = true;
        TimetableValidFlag = false;
        TimetableChangedInAZOrderFlag = false;
        Level1Mode = TimetableMode;
        SetLevel1Mode(136);
        Utilities->CallLogPop(2165);
    }
    catch(const Exception &e)
    {
        ErrorLog(211, e.Message);
    }
}

// ---------------------------------------------------------------------------

void TInterface::ConvertCRLFsToCommas(int Caller, AnsiString &ConvStr)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",ConvertCRLFsToCommas," + ConvStr);
    AnsiString OutStr = "";
    int x = 1; // AnsiString arrays start at 1

    while(x < ConvStr.Length()) // skip the last character as looking for CRLF pairs, i.e. '\r' followed by '\n'
    {
        if((ConvStr[x] == '\r') && (ConvStr[x + 1] == '\n'))
        {
            OutStr += ',';
            x++;
            x++;
        }
        else
        {
            OutStr += ConvStr[x];
            x++;
        }
    }
    if(x == ConvStr.Length())
        OutStr += ConvStr[x]; // add the last character

// strip any excess commas from the end
    if(OutStr != "")
    {
        while(OutStr[OutStr.Length()] == ',')
        {
            OutStr = OutStr.SubString(1, OutStr.Length() - 1);
            if(OutStr == "")
                break; // if consisted of just commas then without this would fail on range error when becomes a null string
        }
    }
    ConvStr = OutStr;
    if(ConvStr == "")
        ConvStr = ','; // don't return a null or will fail, OK to return a comma on its own as will be ignored during ProcessOneTimetableLine
                                      // when AllCommas will be true
    Utilities->CallLogPop(1846);
}

// ---------------------------------------------------------------------------

void TInterface::TimetableHandler()
{
/* CreateEditTTFileName set if a TT file loaded (even if empty), the pointers TTStartTimePtr, TTFirstServicePtr, TTLastServicePtr provide
   relevant information - if null then not set.  TTCurrentEntryPtr is set to the Entry to be displayed or null if there's no start time or no
   entries
*/
    Utilities->CallLog.push_back(Utilities->TimeStamp() + ",TimetableHandler");
    PreviousTTEntryButton->Enabled = false;
    NextTTEntryButton->Enabled = false;
    AddMinsButton->Enabled = false;
    SubMinsButton->Enabled = false;
    CopyTTEntryButton->Enabled = false;
    CutTTEntryButton->Enabled = false;
    PasteTTEntryButton->Enabled = false;
    DeleteTTEntryButton->Enabled = false;
    SaveTTEntryButton->Enabled = false;
    SaveTTButton->Enabled = false;
    SaveTTAsButton->Enabled = false;
    ValidateTimetableButton->Enabled = false;
    AZOrderButton->Enabled = false;
    TTServiceSyntaxCheckButton->Enabled = false;
    NewTTEntryButton->Enabled = false;
    MoveTTEntryUpButton->Enabled = false;
    MoveTTEntryDownButton->Enabled = false;
    CancelTTEntryButton->Enabled = false;
    RestoreTTButton->Enabled = false;
    ExportTTButton->Enabled = false;
    ExitTTModeButton->Enabled = true;

    if(!TimetableEditVector.empty() && !TTEntryChangedFlag && !NewEntryInPreparationFlag)
    {
        AZOrderButton->Enabled = true;
    }

    if(TimetableChangedFlag)
        TimetableValidFlag = false; // should always be the case anyway but include here to be sure

    if(CreateEditTTFileName == "")
    {
        TimetableNameLabel->Caption = "Creating new timetable: not yet saved";
    }
    else
    {
        TimetableNameLabel->Caption = "Editing timetable: " + CreateEditTTTitle;
    }

    if(TTStartTimePtr != 0) // Null means start time not yet set
    {
        TTStartTimeBox->Text = (*TTStartTimePtr).SubString(1, 5); // 1st 5 chars = time if validity check OK
    }
// start time now set & displayed

    if(!TTEntryChangedFlag && !NewEntryInPreparationFlag)
    {
        InfoPanel->Visible = true;
        InfoPanel->Caption = "Select option or change entry";
        if(RailwayTitle != "")
        {
            ShowHideTTButton->Enabled = true;
        }
        else
        {
            ShowHideTTButton->Enabled = false;
        }
        ExitTTModeButton->Enabled = true;
        AllEntriesTTListBox->Enabled = true;
        AnsiString AnsiAddSubText(AddSubMinsBox->Text);
        if((AnsiAddSubText != "") && AreAnyTimesInCurrentEntry())
        {
            bool ValidFlag = true;
            for(int x = 1; x <= AnsiAddSubText.Length(); x++)
            {
                if((AnsiAddSubText[x] > '9') || (AnsiAddSubText[x] < '0'))
                {
                    ValidFlag = false;
                    break;
                }
            }
            if(ValidFlag)
            {
                if(AnsiAddSubText.ToInt() != 0)
                {
                    AddMinsButton->Enabled = true;
                    SubMinsButton->Enabled = true;
                }
            }
        }
        if((CreateEditTTFileName != "") && TimetableChangedFlag)
        {
            RestoreTTButton->Enabled = true;
        }
        if(!TimetableValidFlag && RlyFile && !TimetableChangedFlag && (CreateEditTTFileName != ""))
        { // Need !TimetableChangedFlag because the changed TT must be saved before validation - it's the TT file that is checked
          // so if it is changed but not saved, the 'correct' file will check OK but the changed TT may well not be valid
            ValidateTimetableButton->Enabled = true;
        }
        if(TimetableValidFlag && RlyFile && (CreateEditTTFileName != ""))
        {
            ExportTTButton->Enabled = true;
        }
        if(TTCurrentEntryPtr != 0)
        {
            CopyTTEntryButton->Enabled = true;
            CutTTEntryButton->Enabled = true;
            DeleteTTEntryButton->Enabled = true;
        }
        if((TimetableChangedFlag) && !TimetableEditVector.empty())
        {
            SaveTTButton->Enabled = true;
        }
        if(!TimetableEditVector.empty())
        {
            SaveTTAsButton->Enabled = true;
        }
        if(!NewEntryInPreparationFlag)
        {
            NewTTEntryButton->Enabled = true;
        }
        if((TTCurrentEntryPtr > 0) && !TimetableEditVector.empty())
        {
            if((TimetableEditVector.end() - 1) > TTCurrentEntryPtr)
            {
                NextTTEntryButton->Enabled = true;
                MoveTTEntryDownButton->Enabled = true;
            }
            if(TimetableEditVector.begin() < TTCurrentEntryPtr)
            {
                PreviousTTEntryButton->Enabled = true;
                MoveTTEntryUpButton->Enabled = true;
            }
        }
        if(TTCurrentEntryPtr > 0)
        {
            if(*TTCurrentEntryPtr != "")
            {
                if((TTCurrentEntryPtr >= TTFirstServicePtr) && (TTCurrentEntryPtr <= TTLastServicePtr) && ((*TTCurrentEntryPtr)[1] != '*'))
                {
                    TTServiceSyntaxCheckButton->Enabled = true;
                }
            }
        }
        if(CopiedEntryFlag)
        {
            PasteTTEntryButton->Enabled = true;
        }
        OneEntryTimetableMemo->Clear(); // don't clear if Entry changed
        if(TTCurrentEntryPtr > 0)
        {
// if(*TTCurrentEntryPtr != "")  leave this out or fails to highlight blank line entries
            if((TTCurrentEntryPtr > TTStartTimePtr) && (TTCurrentEntryPtr <= TTLastServicePtr) && ((*TTCurrentEntryPtr)[1] != '*'))
            {
                bool ServiceEntry = true;
                DisplayOneTTLineInPanel(0, *TTCurrentEntryPtr, ServiceEntry);
            }
            else
            {
                bool ServiceEntry = false;
                DisplayOneTTLineInPanel(1, *TTCurrentEntryPtr, ServiceEntry);
            }
        }
    }
    else
    {
        CancelTTEntryButton->Enabled = true;
        SaveTTEntryButton->Enabled = true;
        ShowHideTTButton->Enabled = false;
        ExitTTModeButton->Enabled = false;
        AllEntriesTTListBox->Enabled = false; // to stop entries being selected
        InfoPanel->Caption = "Add or change entry then save it, or cancel";
        InfoPanel->Visible = true;
    }
    Utilities->CallLogPop(1600);
}

// ---------------------------------------------------------------------------
void TInterface::DisplayOneTTLineInPanel(int Caller, AnsiString Data, bool ServiceEntry)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",DisplayOneTTLineInPanel," + Data + ", " +
        AnsiString((short)ServiceEntry));
    OneEntryTimetableMemo->Clear();
    if(ServiceEntry)
    {
        TrainController->StripSpaces(1, Data);
        while(true)
        {
            int CommaPos = Data.Pos(',');
            if((CommaPos == 0) && (Data != ""))
            {
                CommaPos = Data.Length() + 1;
            }
            OneEntryTimetableMemo->Lines->Add(Data.SubString(1, CommaPos - 1));
            if(Data.Length() <= CommaPos)
                break;
            Data = Data.SubString(CommaPos + 1, Data.Length() - CommaPos);
        }
    }
    else
    {
        OneEntryTimetableMemo->Text = Data;
    }
    int TotalLines = OneEntryTimetableMemo->Lines->Count; // remove excess lines at bottom

    while((OneEntryTimetableMemo->Lines->Strings[TotalLines - 1] == "") || (OneEntryTimetableMemo->Lines->Strings[TotalLines - 1] == "\r\n"))
    {
        OneEntryTimetableMemo->Lines->Delete(TotalLines - 1);
        TotalLines--;
        if(TotalLines < 1)
            break;
    }
    OneEntryTimetableMemo->HideSelection = true;
    OneEntryTimetableMemo->SelStart = 0; // need this & next command to set cursor to the top
    OneEntryTimetableMemo->SelLength = 0;
    OneEntryTimetableContents = OneEntryTimetableMemo->Text;
    Utilities->CallLogPop(1602);
}
// ---------------------------------------------------------------------------

void TInterface::HighlightOneEntryInAllEntriesTTListBox(int Caller, int Position)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",HighlightOneEntryInAllEntriesTTListBox," + AnsiString(Position));
    if(TimetableEditVector.empty() || (AllEntriesTTListBox->Items->Count == 0))
    {
        HighlightPanel->Top = 32;
        HighlightPanel->Caption = "";
        HighlightPanel->Width = 100;
        HighlightPanel->Visible = false;
    }
    else
    {
        AnsiString CurrentStr = AllEntriesTTListBox->Items->Strings[Position];
        if(CurrentStr != "") // strip any non alphanumeric characters (specifically \r or \n)
        {
            for(int x = 1; x < CurrentStr.Length(); x++)
            {
                if((CurrentStr[x] < 32) || (CurrentStr[1] > 126))
                {
                    CurrentStr = CurrentStr.SubString(1, (x - 1));
                }
            }
        }
        HighlightPanel->Top = 32 + (Position * 13) - (AllEntriesTTListBox->TopIndex * 13);
        if(HighlightPanel->Top < 32)
            HighlightPanel->Visible = false;
        else
            HighlightPanel->Visible = true; // doesn't matter if goes off the bottom as it becomes invisible as then it's off its parent panel
        HighlightPanel->Caption = CurrentStr;
        if(AllEntriesTTListBox->Items->Count > 46)
            HighlightPanel->Width = 82;
        else
            HighlightPanel->Width = 100;
    }
    Utilities->CallLogPop(1709);
}

// ---------------------------------------------------------------------------
bool TInterface::AreAnyTimesInCurrentEntry()
{
    if((TTCurrentEntryPtr == 0) || (*TTCurrentEntryPtr == ""))
    {
        return false;
    }
    TDateTime DummyTime;
    bool TimesPresent = false;

    for(int x = 0; x < OneEntryTimetableMemo->Lines->Count; x++)
    {
        for(int y = 1; y < (OneEntryTimetableMemo->Lines->Strings[x].Length() - 3); y++)
        {
            if(TrainController->CheckTimeValidity(20, OneEntryTimetableMemo->Lines->Strings[x].SubString(y, 5), DummyTime))
            {
                TimesPresent = true;
                break;
            }
        }
        if(TimesPresent)
            break;
    }
    return TimesPresent;
}

// ---------------------------------------------------------------------------
// end of Timetable editing functions
// ---------------------------------------------------------------------------
void __fastcall TInterface::ExitMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("ExitMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",ExitMenuItemClick");
        if(!FileChangedFlag && !(Track->IsTrackFinished()) && (EveryPrefDir->PrefDirSize() > 0))
        {
            UnicodeString MessageStr =
                "Note that leaving the track unlinked will cause preferred directions to be lost on reloading.  Prevent by linking the track then resaving.  Do you still wish to exit?";
            int button = Application->MessageBox(MessageStr.c_str(), L"Please confirm", MB_YESNO);
            if(button == IDNO)
            {
                Utilities->CallLogPop(1711);
                return;
            }
        }
        if(FileChangedFlag)
        {
            UnicodeString MessageStr = "The railway has changed, exit without saving?";
            int button = Application->MessageBox(MessageStr.c_str(), L"Please confirm", MB_YESNO);
            if(button == IDNO)
            {
                Utilities->CallLogPop(1180);
                return;
            }
        }
        if((TempTTFileName != "") && FileExists(TempTTFileName))
        {
            DeleteFile(TempTTFileName);
        }
        Utilities->CallLogPop(1181);
        Application->Terminate();
    }
    catch(const Exception &e)
    {
        ErrorLog(140, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::TrackInfoOnOffMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("TrackInfoOnOffMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",TrackInfoOnOffMenuItemClick");
        if(TrackInfoOnOffMenuItem->Caption == "Show")
        {
            TrackInfoOnOffMenuItem->Caption = "Hide";
        }
        else
        {
            TrackInfoOnOffMenuItem->Caption = "Show";
        }
        Utilities->CallLogPop(1183);
    }
    catch(const Exception &e)
    {
        ErrorLog(173, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::TrainStatusInfoOnOffMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("TrainStatusInfoOnOffMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",TrainStatusInfoOnOffMenuItemClick");
        if(TrainStatusInfoOnOffMenuItem->Caption == "Show Status")
        {
            TrainStatusInfoOnOffMenuItem->Caption = "Hide Status";
        }
        else
        {
            TrainStatusInfoOnOffMenuItem->Caption = "Show Status";
        }
        Utilities->CallLogPop(1184);
    }
    catch(const Exception &e)
    {
        ErrorLog(141, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::TrainTTInfoOnOffMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("TrainTTInfoOnOffMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",TrainTTInfoOnOffMenuItemClick");
        if(TrainTTInfoOnOffMenuItem->Caption == "Show Timetable")
        {
            TrainTTInfoOnOffMenuItem->Caption = "Hide Timetable";
        }
        else
        {
            TrainTTInfoOnOffMenuItem->Caption = "Show Timetable";
        }
        Utilities->CallLogPop(1185);
    }
    catch(const Exception &e)
    {
        ErrorLog(142, e.Message);
    }
}

// ---------------------------------------------------------------------------
// Dragging Functions
// ---------------------------------------------------------------------------
void __fastcall TInterface::AcceptDragging(TObject *Sender, TObject *Source, int X, int Y, TDragState State, bool &Accept)
{
// allow in zoom out mode
    try
    {
// TrainController->LogEvent("AcceptDragging"); drop this, have too  many
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",AcceptDragging");
        if((Source == PerformancePanel) || (Source == PerformancePanelLabel) || (Source == PerformanceLogBox))
        {
            Accept = true;
            int PPLeft = PerformancePanel->Left;
            int PPTop = PerformancePanel->Left;

            PPLeft = Mouse->CursorPos.x - PerformancePanelDragStartX;
            PPTop = Mouse->CursorPos.y - PerformancePanelDragStartY;
            if((PPLeft + PerformancePanel->Width) < 32)
                PPLeft = 32 - PerformancePanel->Width;
            if(PPLeft > (MainScreen->Left + MainScreen->Width))
                PPLeft = MainScreen->Left + MainScreen->Width;
            if((PPTop + PerformancePanel->Height) < MainScreen->Top)
                PPTop = MainScreen->Top - PerformancePanel->Height;
            if(PPTop > (MainScreen->Top + MainScreen->Height - 20))
                PPTop = MainScreen->Top + MainScreen->Height - 20;
            PerformancePanel->Left = PPLeft;
            PerformancePanel->Top = PPTop;
        }
        else if((Source == OperatorActionPanel) || (Source == OAPanelLabel))
        // not the listbox because that used for selecting trains
        {
            Accept = true;
            int OALeft = OperatorActionPanel->Left;
            int OATop = OperatorActionPanel->Left;

            OALeft = Mouse->CursorPos.x - OperatorActionPanelDragStartX;
            OATop = Mouse->CursorPos.y - OperatorActionPanelDragStartY;
            if((OALeft + OperatorActionPanel->Width) < 32)
                OALeft = 32 - OperatorActionPanel->Width;
            if(OALeft > (MainScreen->Left + MainScreen->Width))
                OALeft = MainScreen->Left + MainScreen->Width;
            if((OATop + OperatorActionPanel->Height) < MainScreen->Top)
                OATop = MainScreen->Top - OperatorActionPanel->Height;
            if(OATop > (MainScreen->Top + MainScreen->Height - 20))
                OATop = MainScreen->Top + MainScreen->Height - 20;
            OperatorActionPanel->Left = OALeft;
            OperatorActionPanel->Top = OATop;
        }
        else
            Accept = false;
        Utilities->CallLogPop(1186);
    }
    catch(const Exception &e)
    {
        ErrorLog(143, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::PerformancePanelStartDrag(TObject *Sender, TDragObject *&DragObject)
{
// allow in zoom out mode
    try
    {
        TrainController->LogEvent("PerformancePanelStartDrag");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",PerformancePanelStartDrag");
        PerformancePanelDragStartX = Mouse->CursorPos.x - PerformancePanel->Left;
        PerformancePanelDragStartY = Mouse->CursorPos.y - PerformancePanel->Top;
        Utilities->CallLogPop(1187);
    }
    catch(const Exception &e)
    {
        ErrorLog(144, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::OperatorActionPanelStartDrag(TObject *Sender, TDragObject *&DragObject) // new v2.2.0

{
// allow in zoom out mode
    try
    {
        TrainController->LogEvent("OperatorActionPanelStartDrag");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",OperatorActionPanelStartDrag");
        OperatorActionPanelDragStartX = Mouse->CursorPos.x - OperatorActionPanel->Left;
        OperatorActionPanelDragStartY = Mouse->CursorPos.y - OperatorActionPanel->Top;
        Utilities->CallLogPop(2091);
    }
    catch(const Exception &e)
    {
        ErrorLog(201, e.Message);
    }
}

// ---------------------------------------------------------------------------
// Mouse Functions
// ---------------------------------------------------------------------------
void __fastcall TInterface::MainScreenMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
    // caller function - stops master clock
{
// have to allow in zoom out mode
    try
    {
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",MainScreenMouseDown," + AnsiString(Button) + "," + AnsiString(X) + "," + AnsiString(Y));
        bool ClockState = Utilities->Clock2Stopped;
        Utilities->Clock2Stopped = true;

        RestoreFocusPanel->Enabled = true; // these added at v2.0.0 to restore navigation keys to move screen when a panel had focus
        RestoreFocusPanel->Visible = true; // because then these buttons just cycled through the panel buttons.  Added in place of the
        RestoreFocusPanel->SetFocus(); // section in ClockTimer2 where focus restored every clock cycle, because then the help screen
        RestoreFocusPanel->Visible = false; // was hidden.  At least now help is only hidden when the screen clicked, which is normal
        RestoreFocusPanel->Enabled = false; // behaviour, and can tell user that can restore navigation keys just by clicking the screen

        if(!Track->RouteFlashFlag && !Track->PointFlashFlag)
        {
            if(!Display->ZoomOutFlag)
                MainScreenMouseDown2(0, Button, Shift, X, Y);
            else
                MainScreenMouseDown3(0, Button, Shift, X, Y);
        }
        Utilities->Clock2Stopped = ClockState;
        Utilities->CallLogPop(33);
    }
    catch(const Exception &e)
    {
        ErrorLog(19, e.Message);
    }
}

// ---------------------------------------------------------------------------
void TInterface::MainScreenMouseDown2(int Caller, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    try
    {
        TrainController->LogEvent("MainScreenMouseDown2," + AnsiString(Button) + "," + AnsiString(X) + "," + AnsiString(Y));
        Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",MainScreenMouseDown2," + AnsiString(Button) + "," + AnsiString(X) +
            "," + AnsiString(Y));
        // unplot GapFlash graphics if plotted & cancel gap flashing (either key down)
        // but not in ZoomOut mode - so can switch between modes & keep gaps flashing
        if(Track->GapFlashFlag && !Display->ZoomOutFlag)
        {
            Track->GapFlashGreen->PlotOriginal(35, Display);
            Track->GapFlashRed->PlotOriginal(36, Display);
            Track->GapFlashFlag = false;
        }
        int HLoc, VLoc;
        Track->GetTrackLocsFromScreenPos(1, HLoc, VLoc, X, Y);
        int NoOffsetX, NoOffsetY;
        Track->GetTruePositionsFromScreenPos(0, NoOffsetX, NoOffsetY, X, Y);
        if(Button == mbRight) // track, PrefDir or text erase, PrefDir/route truncate, or take signaller control of train
        {
            // this routine new at v2.1.0.  Allows railway moving for zoom-in mode when no element at HLoc & VLoc
            int Dummy; // unused in next function
            if(!Track->TrackElementPresentAtHV(0, HLoc, VLoc) && !Track->InactiveTrackElementPresentAtHV(0, HLoc, VLoc) && !Track->UserGraphicPresentAtHV(0, X,
                Y, Dummy))
            {
                StartWholeRailwayMoveHPos = X;
                StartWholeRailwayMoveVPos = Y;
                WholeRailwayMoving = true;
                Screen->Cursor = TCursor(-22); // Four arrows;
            }

            else if(Level2TrackMode == AddText)
            {
                TrainController->LogEvent("mbRight + AddText");
                ResetChangedFileDataAndCaption(2, true);
                if(TextHandler->TextFound(0, NoOffsetX, NoOffsetY))
                {
                    if(TextHandler->TextErase(0, NoOffsetX, NoOffsetY)) // erase text in vector
                    {
                        ClearandRebuildRailway(4);
                        if(NoRailway())
                        {
                            EditMenu->Enabled = false;
                        }
                        else
                            EditMenu->Enabled = true;
                    }
                }
                SetLevel2TrackMode(57); // to remove 'move text' if last text item removed
                Utilities->CallLogPop(34);
                return;
            }
            else if(Level2TrackMode == AddGraphic)
            {
                TrainController->LogEvent("mbRight + AddGraphic");
                if(Track->UserGraphicVector.empty()) // no user graphics
                {
                    Utilities->CallLogPop(2180);
                    return;
                }
                int UGIVecPos;
                if(Track->UserGraphicPresentAtHV(1, X, Y, UGIVecPos))
                {
                    Track->UserGraphicVector.erase(Track->UserGraphicVector.begin() + UGIVecPos);
                    ClearandRebuildRailway(78);
                    if(NoRailway())
                    {
                        EditMenu->Enabled = false;
                    }
                    else
                        EditMenu->Enabled = true;
                }
                Utilities->CallLogPop(2181);
                return;
            }

            else if(Level2TrackMode == AddTrack)
            {
                TrainController->LogEvent("mbRight + AddTrack");
                bool TrackEraseSuccessfulFlag;
                int ErasedTrackVectorPosition;
                Screen->Cursor = TCursor(-11); // Hourglass;
                Track->EraseTrackElement(1, HLoc, VLoc, ErasedTrackVectorPosition, TrackEraseSuccessfulFlag, true);
                if(TrackEraseSuccessfulFlag)
                {
                    if(ErasedTrackVectorPosition > -1)
                        EveryPrefDir->RealignAfterTrackErase(0, ErasedTrackVectorPosition);
                    ResetChangedFileDataAndCaption(3, true);
                    SetTrackBuildImages(9);
                    ClearandRebuildRailway(5); // to ensure location named elements plotted correctly & replot the grid if required
                    SetGapsButton->Enabled = false; // if conditions have changed need to reset buttons, best not calling SetLevel2TrackMode as that
                    TrackOKButton->Enabled = false; // calls Clearand.. if gridflag set & takes a long time
                    if(Track->GapsUnset(1))
                    {
                        SetGapsButton->Enabled = true;
                    }
                    // only enable if there are gaps still to be set (returns false for no track)
                    else
                    {
                        if(!(Track->NoActiveTrack(0)) && !(Track->IsTrackFinished()))
                        {
                            TrackOKButton->Enabled = true;
                        }
                        // TrackOK only enabled if track exists, there are no unset gaps, and track not finished
                    }
                    if(!(Track->IsTrackFinished())) // can only set lengths for several elements together if TrackFinished
                    {
                        SetLengthsButton->Enabled = false;
                    }
                    if(NoRailway())
                    {
                        EditMenu->Enabled = false;
                    }
                    else
                        EditMenu->Enabled = true;
                }
                Screen->Cursor = TCursor(-2); // Arrow
                Utilities->CallLogPop(35);
                return;
            }
            else if(Level2TrackMode == DistanceContinuing) // new for extended distances (similar to PrefDirContinuing)
            {
                TrainController->LogEvent("mbRight + DistanceContinuing");
                ResetChangedFileDataAndCaption(4, true);
                bool LeadingPointsAtLastElement = false;
                if(ConstructPrefDir->GetPrefDirTruncateElement(0, HLoc, VLoc))
                {
                    if(ConstructPrefDir->PrefDirSize() == 0)
                    {
                        Level1Mode = TrackMode;
                        SetLevel1Mode(64);
                        Level2TrackMode = DistanceStart;
                        SetLevel2TrackMode(51); // calls ClearandRebuildRailway to show length erased & sets back to start
                        Utilities->CallLogPop(1526);
                        return;
                    }
                    ConstructPrefDir->PrefDirMarker(1, PrefDirCall, true, Display);
                    OverallDistance = 0, OverallSpeedLimit = -1;
                    ConstructPrefDir->CalcDistanceAndSpeed(0, OverallDistance, OverallSpeedLimit, LeadingPointsAtLastElement);
                    if(!LeadingPointsAtLastElement)
                    {
                        TrackLengthPanel->Visible = true;
                        TrackLengthPanel->SetFocus();
                        InfoPanel->Visible = true;
                        InfoPanel->Caption = "DISTANCE/SPEED SETTING:  Continue or set values (overall length), or right click to cancel/truncate";
                        RestoreAllDefaultLengthsButton->Enabled = true;
                        ResetDefaultLengthButton->Enabled = true;
                        LengthOKButton->Enabled = true;
                        DistanceBox->Text = AnsiString(OverallDistance);
                        if(OverallSpeedLimit > -1)
                            SpeedLimitBox->Text = AnsiString(OverallSpeedLimit);
                        else
                            SpeedLimitBox->Text = "Mixed";
                    }
                    else
                    {
                        TrackLengthPanel->Visible = true;
                        TrackLengthPanel->SetFocus();
                        InfoPanel->Visible = true;
                        InfoPanel->Caption = "DISTANCE/SPEED SETTING:  Can't end on leading points, continue or truncate";
                        RestoreAllDefaultLengthsButton->Enabled = false;
                        ResetDefaultLengthButton->Enabled = false;
                        LengthOKButton->Enabled = false;
                    }
                    ClearandRebuildRailway(53);
                }
                Utilities->CallLogPop(36);
                return;
            }

            else if(Level2PrefDirMode == PrefDirContinuing) // truncate
            {
                TrainController->LogEvent("mbRight + PrefDirContinuing");
                ResetChangedFileDataAndCaption(5, false);
// RlyFile = false; - don't alter this just for PrefDir changes
                if(ConstructPrefDir->GetPrefDirTruncateElement(1, HLoc, VLoc))
                {
                    if(ConstructPrefDir->PrefDirSize() == 0)
                    {
                        Level1Mode = PrefDirMode;
                        SetLevel1Mode(14); // all PrefDir truncated
                        Utilities->CallLogPop(37);
                        return;
                    }
                    ConstructPrefDir->PrefDirMarker(8, PrefDirCall, true, Display);
                }
                Level2PrefDirMode = PrefDirContinuing;
                SetLevel2PrefDirMode(0); // calls ClearandRebuildRailway to show length erased & sets back to start
                Utilities->CallLogPop(38);
                return;
            }

            else if((Level1Mode == PrefDirMode) && (Level2PrefDirMode != PrefDirContinuing) && (Level2PrefDirMode != PrefDirSelecting)) // delete element
            {
                TrainController->LogEvent("mbRight + != PrefDirContinuing");
                ResetChangedFileDataAndCaption(6, false);
// RlyFile = false; - don't alter this just for PrefDir changes
                EveryPrefDir->EraseFromPrefDirVectorAnd4MultiMap(0, HLoc, VLoc);
                Level1Mode = PrefDirMode;
                SetLevel1Mode(15); // calls ClearandRebuildRailway to show length erased & sets back to start
                Utilities->CallLogPop(39);
                return;
            }

            else if((Level2OperMode == Operating) || (Level2OperMode == PreStart)) // disallow when paused, but allow some parts in prestart
            {
                TrainController->LogEvent("mbRight + OperMode");
                bool FoundFlag;
                int VecPos = Track->GetVectorPositionFromTrackMap(1, HLoc, VLoc, FoundFlag);
                if(FoundFlag && (Level2OperMode != PreStart)) // disallow signaller control in PreStart
                {
                    SelectedTrainID = Track->TrackElementAt(426, VecPos).TrainIDOnElement;
                    // signaller control of train
                    if(SelectedTrainID > -1)
                    {
                        TTrain Train = TrainController->TrainVectorAtIdent(23, SelectedTrainID);
                        Train.LeavingUnderSigControlAtContinuation = false;
                        if((Train.LeadElement == -1) || (Track->TrackElementAt(788, Train.LeadElement).Conn[Train.LeadExitPos] == -1))
                        {
                            if(Train.TrainMode == Signaller)
                            {
                                Train.LeavingUnderSigControlAtContinuation = true;
                            }
                        }
                        if((Train.Stopped()) || (Train.TrainFailed && !(Train.TrainMode == Signaller)) ||
                            ((Train.TrainMode == Signaller) && !Train.SignallerStoppingFlag && !Train.LeavingUnderSigControlAtContinuation &&
                            !Train.StepForwardFlag))
                        // don't allow signaller popup menu in timetable mode unless stopped,
                        // or when coming to a stop or leaving at a continuation when under signaller control
                        // or when failed
                        {
                            // don't allow selection if another stopped train at a bridge position
                            if(Track->TrackElementAt(630, VecPos).TrackType == Bridge)
                            {
                                int TrainID01 = Track->TrackElementAt(631, VecPos).TrainIDOnBridgeTrackPos01;
                                int TrainID23 = Track->TrackElementAt(632, VecPos).TrainIDOnBridgeTrackPos23;
                                if((TrainID01 > -1) && (TrainID23 > -1))
                                {
                                    TrainController->StopTTClockMessage(0, "Can't select a train at a bridge when another train is at the same bridge");
                                    Utilities->CallLogPop(1103);
                                    return;
                                }
                            }
                            if(Train.TrainMode == Timetable)
                            {
                                TakeSignallerControlMenuItem->Enabled = true;
                                TimetableControlMenuItem->Enabled = false;
                                ChangeDirectionMenuItem->Enabled = false;
                                MoveForwardsMenuItem->Enabled = false;
                                SignallerJoinedByMenuItem->Enabled = false;
                                RepairFailedTrainMenuItem->Enabled = false;
                                StepForwardMenuItem->Enabled = false;
                                RemoveTrainMenuItem->Enabled = false;
                                PassRedSignalMenuItem->Enabled = false;
                                SignallerControlStopMenuItem->Enabled = false;
                            }
                            else // signaller mode
                            {
                                TakeSignallerControlMenuItem->Enabled = false;
                                if((Train.Crashed) || (Train.Derailed))
                                {
                                    TimetableControlMenuItem->Enabled = false;
                                    ChangeDirectionMenuItem->Enabled = false;
                                    MoveForwardsMenuItem->Enabled = false;
                                    SignallerJoinedByMenuItem->Enabled = false;
                                    RepairFailedTrainMenuItem->Enabled = false;
                                    StepForwardMenuItem->Enabled = false;
                                    PassRedSignalMenuItem->Enabled = false;
                                    SignallerControlStopMenuItem->Enabled = false;
                                    RemoveTrainMenuItem->Enabled = true;
                                }
                                else if(Train.Stopped())
                                {
                                    if(Train.TimetableFinished)
                                    {
                                        TimetableControlMenuItem->Enabled = false;
                                    }
                                    else
                                    {
                                        if(Train.RestoreTimetableLocation == "") // en route
                                        {
                                            TimetableControlMenuItem->Enabled = true;
                                        }
                                        else
                                        {
                                            // obtain train location & check if OK for restoration of tt control
                                            AnsiString LocName = "";
                                            if(Train.LeadElement > -1)
                                            {
                                                LocName = Track->TrackElementAt(802, Train.LeadElement).ActiveTrackElementName;
                                            }
                                            if((LocName == "") && (Train.MidElement > -1))
                                            {
                                                LocName = Track->TrackElementAt(803, Train.MidElement).ActiveTrackElementName;
                                            }
                                            if(Train.RestoreTimetableLocation == LocName)
                                            {
                                                TimetableControlMenuItem->Enabled = true;
                                            }
                                            else
                                            {
                                                TimetableControlMenuItem->Enabled = false;
                                            }
                                        }
                                    }
// don't allow ChangeDirection if lead or mid elements (but not lag or next) -1, or lead, mid, lag or next elements continuations
                                    ChangeDirectionMenuItem->Enabled = true;
                                    if(Train.LeadElement > -1)
                                    {
                                        if(Track->TrackElementAt(794, Train.LeadElement).TrackType == Continuation)
                                        {
                                            ChangeDirectionMenuItem->Enabled = false;
                                        }
                                        if(Track->TrackElementAt(791, Train.LeadElement).Conn[Train.LeadExitPos] > -1)
                                        {
                                            if(Track->TrackElementAt(792, (Track->TrackElementAt(793, Train.LeadElement).Conn[Train.LeadExitPos]))
                                                .TrackType == Continuation)
                                            {
                                                ChangeDirectionMenuItem->Enabled = false;
                                            }
                                        }
                                    }
                                    else
                                        ChangeDirectionMenuItem->Enabled = false;
                                    if(Train.MidElement > -1)
                                    {
                                        if(Track->TrackElementAt(795, Train.MidElement).TrackType == Continuation)
                                        {
                                            ChangeDirectionMenuItem->Enabled = false;
                                        }
                                    }
                                    else
                                        ChangeDirectionMenuItem->Enabled = false;
                                    if(Train.LagElement > -1)
                                    {
                                        if(Track->TrackElementAt(796, Train.LagElement).TrackType == Continuation)
                                        {
                                            ChangeDirectionMenuItem->Enabled = false;
                                        }
                                    }
                                    RemoveTrainMenuItem->Enabled = true;
                                    SignallerControlStopMenuItem->Enabled = false;
                                    SignallerJoinedByMenuItem->Enabled = false;
                                    RepairFailedTrainMenuItem->Enabled = false;
                                    StepForwardMenuItem->Enabled = false;
                                    MoveForwardsMenuItem->Enabled = false;
                                    PassRedSignalMenuItem->Enabled = false;
                                    if(Train.AbleToMove(0))
                                    {
                                        MoveForwardsMenuItem->Enabled = true;
                                        if(!Train.LeavingUnderSigControlAtContinuation)
                                            StepForwardMenuItem->Enabled = true; // added 'if' condition for v1.3.2 due to Carwyn Thomas error,
                                    } // fails on trying to calc AutoSig time delay for resetting signals
                                    if(Train.AbleToMoveButForSignal(0)) // may not be in AutoSigs route but disallow anyway as not needed at continuation
                                    {
                                        PassRedSignalMenuItem->Enabled = true;
                                        StepForwardMenuItem->Enabled = true;
                                    }
                                    TTrain *AdjacentTrain;
                                    if(Train.IsThereAnAdjacentTrain(0, AdjacentTrain))
                                    {
                                        SignallerJoinedByMenuItem->Enabled = true;
                                    }
                                    if(Train.TrainFailed)
                                    {
                                        RepairFailedTrainMenuItem->Enabled = true;
                                    }
                                }
                                else // train moving under signaller control - only permit restoration of TT control when stopped as could be in
                                     // mid move, & SetTrainMovementValues only intended to be called when stopped
                                {
                                    TimetableControlMenuItem->Enabled = false;
                                    ChangeDirectionMenuItem->Enabled = false;
                                    RemoveTrainMenuItem->Enabled = false;
                                    MoveForwardsMenuItem->Enabled = false;
                                    SignallerJoinedByMenuItem->Enabled = false;
                                    RepairFailedTrainMenuItem->Enabled = false;
                                    PassRedSignalMenuItem->Enabled = false;
                                    StepForwardMenuItem->Enabled = false;
                                    SignallerControlStopMenuItem->Enabled = true;
                                }
                            }
                            TrainHeadCodeMenuItem->Caption = Train.HeadCode + ":";
                            TrainController->StopTTClockFlag = true; // so TTClock stopped during MasterClockTimer function
                            TrainController->RestartTime = TrainController->TTClockTime;
                            PopupMenu->Popup(X, Y); // menu stops everything so reset timetable time when restarts
                            TrainController->BaseTime = TDateTime::CurrentDateTime();
                            TrainController->StopTTClockFlag = false;
                            Utilities->CallLogPop(40);
                            return;
                        }
                    }
                }

                if(RouteMode == RouteContinuing) // clear a single element (clears whether use left or right mouse button) +allow in PreStart
                {
                    TrainController->LogEvent("mbRight + RouteContinuing");
                    RevertToOriginalRouteSelector(0);
                    Utilities->CallLogPop(41);
                    return;
                }

                else if(RouteCancelFlag) // allow in PreStart
                {
                    TrainController->LogEvent("mbRight + RouteCancelFlag");
                    Screen->Cursor = TCursor(-11); // Hourglass;
                    // stop clock as sometimes takes several seconds
                    TrainController->StopTTClockFlag = true; // so TTClock stopped during MasterClockTimer function
                    TrainController->RestartTime = TrainController->TTClockTime;
                    if(AllRoutes->GetAllRoutesTruncateElement(0, HLoc, VLoc, ConsecSignalsRoute)) // updates LockedRouteClass
                    {
                        ClearandRebuildRailway(6); // to replot new shorter route
                    }
                    RevertToOriginalRouteSelector(1);
                    TrainController->BaseTime = TDateTime::CurrentDateTime();
                    TrainController->StopTTClockFlag = false;
                    Screen->Cursor = TCursor(-2); // Arrow
                }

                else // gap flashing, don't allow to interfere with RouteCancelFlag
                {
                    TrainController->LogEvent("mbRight, GapFlashingInOperOrPreStartMode");
                    int Position;
                    TTrackElement TrackElement;
                    if(Track->FindNonPlatformMatch(4, HLoc, VLoc, Position, TrackElement));
                    {
                        if((TrackElement.TrackType == GapJump) && (TrackElement.Conn[0] > -1))
                        {
                            if((TrackElement.TrainIDOnElement == -1) && (Track->TrackElementAt(818, TrackElement.Conn[0]).TrainIDOnElement == -1))
                            { // don't flash if train on either gap element
                                Track->GapFlashGreenPosition = TrackElement.Conn[0];
                                Track->GapFlashGreen->SetScreenHVSource(5, Track->TrackElementAt(434, Track->GapFlashGreenPosition).HLoc * 16,
                                    Track->TrackElementAt(435, Track->GapFlashGreenPosition).VLoc * 16);
                                Track->GapFlashGreen->LoadOriginalExistingGraphic(1, 4, 4, 8, 8,
                                    Track->TrackElementAt(436, Track->GapFlashGreenPosition).GraphicPtr);
                                Track->GapFlashRedPosition = Position;
                                Track->GapFlashRed->SetScreenHVSource(6, Track->TrackElementAt(437, Track->GapFlashRedPosition).HLoc * 16,
                                    Track->TrackElementAt(438, Track->GapFlashRedPosition).VLoc * 16);
                                Track->GapFlashRed->LoadOriginalExistingGraphic(2, 4, 4, 8, 8,
                                    Track->TrackElementAt(439, Track->GapFlashRedPosition).GraphicPtr);
                                Track->GapFlashFlag = true;
                            }
                        }
                    }
                    Utilities->CallLogPop(42);
                    return; // covers above else & included here in case any more usermodes added later
                }
            }

            // deal with gap selection - if no other right button selection - apply for any mode (also included in OperMode above)
            TrainController->LogEvent("mbRight, GapFlashingNotOperOrPreStartMode");
            int Position;
            TTrackElement TrackElement;
            if(Track->FindNonPlatformMatch(18, HLoc, VLoc, Position, TrackElement));
            {
                if((TrackElement.TrackType == GapJump) && (TrackElement.Conn[0] > -1))
                {
                    if((TrackElement.TrainIDOnElement == -1) && (Track->TrackElementAt(819, TrackElement.Conn[0]).TrainIDOnElement == -1))
                    { // don't flash if train on either gap element
                        Track->GapFlashGreenPosition = TrackElement.Conn[0];
                        Track->GapFlashGreen->SetScreenHVSource(7, Track->TrackElementAt(806, Track->GapFlashGreenPosition).HLoc * 16,
                            Track->TrackElementAt(807, Track->GapFlashGreenPosition).VLoc * 16);
                        Track->GapFlashGreen->LoadOriginalExistingGraphic(3, 4, 4, 8, 8, Track->TrackElementAt(808, Track->GapFlashGreenPosition).GraphicPtr);
                        Track->GapFlashRedPosition = Position;
                        Track->GapFlashRed->SetScreenHVSource(8, Track->TrackElementAt(809, Track->GapFlashRedPosition).HLoc * 16,
                            Track->TrackElementAt(810, Track->GapFlashRedPosition).VLoc * 16);
                        Track->GapFlashRed->LoadOriginalExistingGraphic(4, 4, 4, 8, 8, Track->TrackElementAt(811, Track->GapFlashRedPosition).GraphicPtr);
                        Track->GapFlashFlag = true;
                    }
                }
            }
            Utilities->CallLogPop(67);
            return; // covers above else & included here in case any more usermodes added later
        }

// Left Mouse Button Functions
        if(RouteCancelFlag)
            RevertToOriginalRouteSelector(2);
        mbLeftDown = true;

        if(Level2TrackMode == AddTrack)
        {
            TrainController->LogEvent("mbLeft + AddTrack");
            Screen->Cursor = TCursor(-11); // Hourglass;
            ResetChangedFileDataAndCaption(7, true);
            bool TrackLinkingRequiredFlag;
            int CurrentTag;
            TSpeedButton *TempSpeedButton = 0;
            if(CurrentSpeedButton)
            {
                CurrentTag = CurrentSpeedButton->Tag;
                TempSpeedButton = CurrentSpeedButton;
            }
            else
                CurrentTag = 0;
            bool InternalChecks = true;
            Track->PlotAndAddTrackElement(1, CurrentTag, 0, HLoc, VLoc, TrackLinkingRequiredFlag, InternalChecks);
            // above now has extra zero 'Aspect' parameter at v2.2.0 so can distinguish between adding track and pasting
            EditMenu->Enabled = true;
            if(Track->NamedLocationElementAt(1, HLoc, VLoc))
                ClearandRebuildRailway(7); // so named location graphics plotted correctly
            if(TrackLinkingRequiredFlag)
            {
                Track->SetTrackFinished(false);
            }
            SetTrackBuildImages(10);
            SetGapsButton->Enabled = false; // if conditions have changed need to reset buttons, best not calling SetLevel2TrackMode as that
            TrackOKButton->Enabled = false; // calls Clearand.. if gridflag set & takes a long time
            if(Track->GapsUnset(2))
            {
                SetGapsButton->Enabled = true;
            }
            // only enable if there are gaps still to be set (returns false for no track)
            else
            {
                if(!(Track->NoActiveTrack(1)) && !(Track->IsTrackFinished()))
                {
                    TrackOKButton->Enabled = true;
                }
                // TrackOK only enabled if track exists, there are no unset gaps, and track not finished
            }
            if(!(Track->IsTrackFinished())) // can only set lengths for several elements together if TrackFinished
            {
                SetLengthsButton->Enabled = false;
            }
            if(TempSpeedButton) // restore button if was pressed
            {
                CurrentSpeedButton = TempSpeedButton;
                CurrentSpeedButton->Down = true;
            }
            Screen->Cursor = TCursor(-2); // Arrow
            Utilities->CallLogPop(44);
            return;
        }

        else if(Level2TrackMode == AddGraphic)
        {
            TrainController->LogEvent("mbLeft + AddGraphic");
            ResetChangedFileDataAndCaption(24, false);
            TUserGraphicItem NewGI;
            TTrack::TUserGraphicMap::iterator UGMIt = Track->UserGraphicMap.find(SelectedGraphicFileName);
            if(UGMIt != Track->UserGraphicMap.end()) // if it is the end then nothing was found
            {
                NewGI.UserGraphic = UGMIt->second;
                NewGI.Width = UGMIt->second->Width;
                NewGI.Height = UGMIt->second->Height;
                NewGI.FileName = SelectedGraphicFileName;
                NewGI.HPos = X + (Display->DisplayOffsetH * 16);
                NewGI.VPos = Y + (Display->DisplayOffsetV * 16);
                Track->UserGraphicVector.push_back(NewGI);
                Display->PlotAndAddUserGraphic(1, NewGI);
            }
            else
            {
                ShowMessage("Unable to find graphic file " + SelectedGraphicFileName + ". Check it still exists.");
                Utilities->CallLogPop(2195);
                return;
            }
            MoveTextOrGraphicButton->Enabled = true;
            EditMenu->Enabled = true;
            Utilities->CallLogPop(2182);
            return;
        }

        else if(Level2TrackMode == AddLocationName)
        {
            TrainController->LogEvent("mbLeft + AddLocationName");
            ResetChangedFileDataAndCaption(8, true);
            bool FoundFlag;
            TTrackElement TrackElement;
            AnsiString NameString;
            TTrack::TIMPair InactivePair = Track->GetVectorPositionsFromInactiveTrackMap(1, HLoc, VLoc, FoundFlag);
            if(!FoundFlag)
            {
                Utilities->CallLogPop(45);
                return; // inactive element not found (has to be a platform or named non-station location, can't select any other element)
            }
            TTrackElement& InactiveTrackElement1 = Track->InactiveTrackElementAt(28, InactivePair.first);
            TTrackElement& InactiveTrackElement2 = Track->InactiveTrackElementAt(29, InactivePair.second); // may be same element if only 1
            TTrackElement& ValidElement = InactiveTrackElement1;
            unsigned int ValidPosition;
            if((InactiveTrackElement1.TrackType != Platform) && (InactiveTrackElement2.TrackType != Platform) &&
                (InactiveTrackElement1.TrackType != NamedNonStationLocation) && (InactiveTrackElement2.TrackType != NamedNonStationLocation) &&
                (InactiveTrackElement1.TrackType != Concourse) && (InactiveTrackElement2.TrackType != Concourse))
            {
                Utilities->CallLogPop(46);
                return; // element not valid
            }
            if((InactiveTrackElement1.TrackType == Platform) || (InactiveTrackElement1.TrackType == NamedNonStationLocation) ||
                (InactiveTrackElement1.TrackType == Concourse))
            {
                ValidElement = InactiveTrackElement1;
                ValidPosition = InactivePair.first;
            }
            else if((InactiveTrackElement2.TrackType == Platform) || (InactiveTrackElement2.TrackType == NamedNonStationLocation) ||
                (InactiveTrackElement2.TrackType == Concourse))
            {
                ValidElement = InactiveTrackElement2;
                ValidPosition = InactivePair.second;
            }
            // now have required element as ValidElement & position in InactiveTrackvector as ValidPosition

            // put a square box round element to show selection
            Display->Rectangle(0, HLoc * 16, VLoc * 16, clB0G0R5, 0, 2);
            LocationNameTextBox->Visible = true;
            LocationNameTextBox->SetFocus();
            NameString = Track->GetLocationName(ValidPosition);
            LocationNameTextBox->Text = NameString;
            InfoPanel->Visible = true;
            InfoPanel->Caption = "NAMING LOCATIONS:  Enter name, 'Carriage Return' to accept, 'Escape' to quit";

            Track->LNPendingList.clear();
            Track->LNPendingList.insert(Track->LNPendingList.end(), ValidPosition);
            Level2TrackMode = NoTrackMode; // if leave as AddLocationName can select other squares before enter name
            Utilities->CallLogPop(47);
            return;
        }

        else if(Level2TrackMode == DistanceStart) // new for extended distances - similar to !PrefDirContinuing
                                                  // prior to selecting start element
        {
            TrainController->LogEvent("mbLeft + DistanceStart");
            ResetChangedFileDataAndCaption(9, true);
            if(ConstructPrefDir->GetPrefDirStartElement(0, HLoc, VLoc))
            {
                ConstructPrefDir->PrefDirMarker(9, PrefDirCall, true, Display);
                Level1Mode = TrackMode;
                SetLevel1Mode(65);
                Level2TrackMode = DistanceContinuing;
                SetLevel2TrackMode(30);
            }
            Utilities->CallLogPop(48);
            return;
        }

        else if(Level2TrackMode == DistanceContinuing) // new for extended distances - similar to PrefDirContinuing
                                                       // prior to selecting finish element
        {
            TrainController->LogEvent("mbLeft + DistanceContinuing");
            ResetChangedFileDataAndCaption(10, true);
            bool FinishElement = false, LeadingPointsAtLastElement = false;
            Screen->Cursor = TCursor(-11); // Hourglass;
            if((ConstructPrefDir->PrefDirSize() != 1) || (ConstructPrefDir->GetFixedPrefDirElementAt(181, 0).HLoc != HLoc) ||
                (ConstructPrefDir->GetFixedPrefDirElementAt(182, 0).VLoc != VLoc))
            { // not same as start element
                if(ConstructPrefDir->GetNextPrefDirElement(0, HLoc, VLoc, FinishElement))
                {
                    ConstructPrefDir->PrefDirMarker(3, PrefDirCall, true, Display);
                    OverallDistance = 0, OverallSpeedLimit = -1;
                    ConstructPrefDir->CalcDistanceAndSpeed(1, OverallDistance, OverallSpeedLimit, LeadingPointsAtLastElement);
                    if(FinishElement)
                    {
                        TrackLengthPanel->Visible = true;
                        TrackLengthPanel->SetFocus();
                        InfoPanel->Visible = true;
                        InfoPanel->Caption = "DISTANCE/SPEED SETTING:  Set values (overall length), or right click to cancel/truncate";
                        RestoreAllDefaultLengthsButton->Enabled = true;
                        ResetDefaultLengthButton->Enabled = true;
                        LengthOKButton->Enabled = true;
                        DistanceBox->Text = AnsiString(OverallDistance);
                        if(OverallSpeedLimit > -1)
                            SpeedLimitBox->Text = AnsiString(OverallSpeedLimit);
                        else
                            SpeedLimitBox->Text = "Mixed";
                        ClearandRebuildRailway(52);
                        Screen->Cursor = TCursor(-2); // Arrow
                        Utilities->CallLogPop(1527);
                        return;
                    }
                    else
                    {
                        if(!LeadingPointsAtLastElement)
                        {
                            TrackLengthPanel->Visible = true;
                            TrackLengthPanel->SetFocus();
                            InfoPanel->Visible = true;
                            InfoPanel->Caption = "DISTANCE/SPEED SETTING:  Continue or set values (overall length), or right click to cancel/truncate";
                            RestoreAllDefaultLengthsButton->Enabled = true;
                            ResetDefaultLengthButton->Enabled = true;
                            LengthOKButton->Enabled = true;
                            DistanceBox->Text = AnsiString(OverallDistance);
                            if(OverallSpeedLimit > -1)
                                SpeedLimitBox->Text = AnsiString(OverallSpeedLimit);
                            else
                                SpeedLimitBox->Text = "Mixed";
                            // Level2TrackMode = DistanceContinuing;
                            // SetLevel2TrackMode();
                        }
                        else
                        {
                            TrackLengthPanel->Visible = true;
                            TrackLengthPanel->SetFocus();
                            InfoPanel->Visible = true;
                            InfoPanel->Caption = "DISTANCE/SPEED SETTING:  Can't end on leading points, need to continue or truncate";
                            RestoreAllDefaultLengthsButton->Enabled = false;
                            ResetDefaultLengthButton->Enabled = false;
                            LengthOKButton->Enabled = false;
                            // Level2TrackMode = DistanceContinuing;
                            // SetLevel2TrackMode();
                        }
                    }
                }
            }
            else // same as start element
            {
                ConstructPrefDir->ExternalClearPrefDirAnd4MultiMap();
                Level2TrackMode = DistanceStart;
                SetLevel2TrackMode(54);
                Screen->Cursor = TCursor(-2); // Arrow
                Utilities->CallLogPop(1713);
                return;
            }
            ClearandRebuildRailway(51);
            Screen->Cursor = TCursor(-2); // Arrow
            Utilities->CallLogPop(1490);
            return;
        }

        else if(Level2TrackMode == GapSetting)
        {
            TrainController->LogEvent("mbLeft + GapSetting");
            ResetChangedFileDataAndCaption(11, true);
            // HighLightOneGap already called once from SetLevel2TrackMode so have all gap element values set
            // & it is highlighted
            if(!(Track->FindSetAndDisplayMatchingGap(1, HLoc, VLoc)))
            {
                Utilities->CallLogPop(50);
                return; // true if finds one
            }
            InfoPanel->Visible = true;
            InfoPanel->Caption = "CONNECTING GAPS:  Connecting element selected";
            Display->Update(); // resurrected when Update() dropped from PlotOutput etc
            Delay(0, 500); // 500 msec delay before next selection requested

            // ClearandRebuildRailway(8);//get rid of gap selections
            // need to call this later when new gap displayed, else old gap remains

            // now back to highlighting next gap
            // bool LocError = false;
            if(!(HighLightOneGap(1, HLoc, VLoc)))
            {
                // all gaps set
                ShowMessage("All gaps set");
                if(Level2TrackMode == AddTrack)
                {
                    Level1Mode = TrackMode;
                    SetLevel1Mode(66);
                    SetLevel2TrackMode(31);
                }
                else
                {
                    Level1Mode = TrackMode;
                    SetLevel1Mode(37);
                }
                ClearandRebuildRailway(9); // get rid of last gap ellipse
                Utilities->CallLogPop(51);
                return;
            }
            // here if one gap highlighted so return to user to allow corresponding gap to be selected
            // by another call to MainScreenMouseDown
        }

        else if(Level2TrackMode == AddText)
        {
            TrainController->LogEvent("mbLeft + AddText");
            ResetChangedFileDataAndCaption(12, true);
            // X & Y are relative to Display output, but TextBox is placed relative to Form
            // if mouse position on first character of an existing piece of text reload it into the editor

            bool TextFoundFlag = false;
            int TrueX = 0, TrueY = 0;
            AnsiString ExistingText = "";
            TTextHandler::TTextVectorIterator TextPtr;
            TFont *ExistingTextFont = new TFont;
            int ExistingTextHPos = 0, ExistingTextVPos = 0;
            Track->GetTruePositionsFromScreenPos(1, TrueX, TrueY, X, Y);
            if(!TextHandler->TextVector.empty())
            {
                for(TextPtr = (TextHandler->TextVector.end() - 1); TextPtr >= TextHandler->TextVector.begin(); TextPtr--)
                {
                    if((TrueX >= (*TextPtr).HPos) && (TrueX < ((*TextPtr).HPos + abs((*TextPtr).Font->Height))) && (TrueY >= (*TextPtr).VPos) && (TrueY <
                        ((*TextPtr).VPos + abs((*TextPtr).Font->Height))))
                    {
                        ExistingText = (*TextPtr).TextString;
                        ExistingTextFont->Assign((*TextPtr).Font);
                        ExistingTextHPos = (*TextPtr).HPos;
                        ExistingTextVPos = (*TextPtr).VPos;
                        TextFoundFlag = true;
                        TextHandler->TextErase(9, TrueX, TrueY);
                        break;
                    } // if ....
                } // for TextPtr...
            } // if !TextVector...

            if(TextFoundFlag)
            {
                TextBox->Left = ExistingTextHPos + Display->Left() - (Display->DisplayOffsetH * 16) - 3;
                TextBox->Top = ExistingTextVPos + Display->Top() - (Display->DisplayOffsetV * 16) - 3;
                TextBox->Font->Assign(ExistingTextFont);
                Display->SetFont(ExistingTextFont);
                Text_X = ExistingTextHPos;
                Text_Y = ExistingTextVPos;
            }
            else
            {
                TextBox->Left = (TextOrUserGraphicGridVal * div((Display->Left() + X), TextOrUserGraphicGridVal).quot) - 3;
                TextBox->Top = (TextOrUserGraphicGridVal * div((Display->Top() + Y), TextOrUserGraphicGridVal).quot) - 3;
                TextBox->Font->Assign(Display->GetFont());
                Text_X = TextOrUserGraphicGridVal * div(NoOffsetX, TextOrUserGraphicGridVal).quot;
                Text_Y = TextOrUserGraphicGridVal * div(NoOffsetY, TextOrUserGraphicGridVal).quot;
            }
            TextBox->Visible = true;
            TextBox->SetFocus();
            if(TextFoundFlag)
                TextBox->Text = ExistingText;
            else
                TextBox->Text = "New Text: CR=end, ESC=quit";
            TextBox->Width = (abs(TextBox->Font->Height) * TextBox->Text.Length() * 0.7);
            TextBox->SelectAll();
            delete ExistingTextFont;
            ClearandRebuildRailway(29); // to remove old text if replaced
            Level2TrackMode = AddText;
            Utilities->CallLogPop(1775);
            return; // If text input go no further
        }

        else if(Level2TrackMode == MoveTextOrGraphic)
        {
            TrainController->LogEvent("mbLeft + MoveTextOrGraphic");
            ResetChangedFileDataAndCaption(13, true);
            // int HPosInput;// = X + (Display->DisplayOffsetH * 16);
            // int VPosInput;// = Y + (Display->DisplayOffsetV * 16);
            // Track->GetTruePositionsFromScreenPos(HPosInput, VPosInput, X, Y);
            // StartX = X + (Display->DisplayOffsetH * 16);
            // StartY = Y + (Display->DisplayOffsetV * 16);
            Track->GetTruePositionsFromScreenPos(2, StartX, StartY, X, Y);
            TextHandler->TextMove(0, StartX, StartY, TextItem, TextMoveHPos, TextMoveVPos, TextFoundFlag);
            if(!TextFoundFlag) // give precedence to text
            {
                Track->UserGraphicMove(0, StartX, StartY, UserGraphicVectorNumber, UserGraphicMoveHPos, UserGraphicMoveVPos, UserGraphicFoundFlag);
            }
            Utilities->CallLogPop(53);
            return; // if text move selected don't permit anything else
        }

        else if(Level2TrackMode == TrackSelecting)
/* When 'select' chosen from the Edit menu (only available in 'AddTrack') conditions are set ready to enclose a rectangular screen area
           using MouseMove.  When MouseDown occurs the starting point is marked (wrt whole railway, not just the screen) and stored in
           SelectStartPair.  If the mouse button is released and a new start position selected then the earlier one is discarded.  Providing the
           button is held down subsequent actions occur during MouseMove (to display the changing rectangle) and MouseUp to define the final
           selected rectangle.
*/
        {
            TrainController->LogEvent("mbLeft + TrackSelecting");
            ClearandRebuildRailway(10); // to get rid of earlier rectangles
            SelectStartPair.first = HLoc;
            SelectStartPair.second = VLoc;
        }

        else if((Level2TrackMode == CopyMoving) || (Level2TrackMode == CutMoving))
/* The same actions apply on MouseDown whether Copy or Cut selected from the menu.  First the horizontal and vertical mouse position is
           checked and unless it lies within the selected rectangle and not within 4 pixels of an edge the pickup fails and the function returns.
           Otherwise flag SelectPickedUp is set to true (to allow it to move during MouseMove and remain in place at MouseUp) and the mouse position
           is saved in SelectBitmapMouseLocX & Y for use later in MouseMove & MouseUp.
*/
        {
            TrainController->LogEvent("mbLeft + CopyMoving or CutMoving");
            ResetChangedFileDataAndCaption(14, true);
            if((X < ((SelectBitmapHLoc - Display->DisplayOffsetH) * 16) + 4) ||
                (X > ((SelectBitmapHLoc + (SelectBitmap->Width / 16) - Display->DisplayOffsetH) * 16) - 4))
            {
                SelectPickedUp = false;
                Utilities->CallLogPop(54);
                return; // within 4 pixels of outside of horizontal area (4 pixels are so can't push selection off edge of screen)
            }
            if((Y < ((SelectBitmapVLoc - Display->DisplayOffsetV) * 16) + 4) ||
                (Y > ((SelectBitmapVLoc + (SelectBitmap->Height / 16) - Display->DisplayOffsetV) * 16) - 4))
            {
                SelectPickedUp = false;
                Utilities->CallLogPop(55);
                return; // within 4 pixels of outside of vertical area (4 pixels are so can't push selection off edge of screen)
            }
            else
            {
                SelectPickedUp = true;
            }
            SelectBitmapMouseLocX = X;
            SelectBitmapMouseLocY = Y;
        }

        else if((Level1Mode == PrefDirMode) && (Level2PrefDirMode != PrefDirContinuing) && (Level2PrefDirMode != PrefDirSelecting))
        {
            TrainController->LogEvent("mbLeft + != PrefDirContinuing");
            ResetChangedFileDataAndCaption(15, false);
// RlyFile = false; - don't alter this just for PrefDir changes
            if(ConstructPrefDir->GetPrefDirStartElement(1, HLoc, VLoc))
            {
                ConstructPrefDir->PrefDirMarker(2, PrefDirCall, true, Display);
                Level2PrefDirMode = PrefDirContinuing;
                SetLevel2PrefDirMode(1);
            }
            Utilities->CallLogPop(56);
            return;
        }

        else if(Level2PrefDirMode == PrefDirContinuing)
        {
            TrainController->LogEvent("mbLeft + PrefDirContinuing");
            ResetChangedFileDataAndCaption(16, false);
// RlyFile = false; - don't alter this just for PrefDir changes
            bool FinishElement;
            Screen->Cursor = TCursor(-11); // Hourglass;
            if((ConstructPrefDir->PrefDirSize() != 1) || (ConstructPrefDir->GetFixedPrefDirElementAt(183, 0).HLoc != HLoc) ||
                (ConstructPrefDir->GetFixedPrefDirElementAt(184, 0).VLoc != VLoc))
            { // not same as start element
                if(ConstructPrefDir->GetNextPrefDirElement(1, HLoc, VLoc, FinishElement))
                {
                    ConstructPrefDir->PrefDirMarker(10, PrefDirCall, true, Display);
                    if(FinishElement)
                    {
                        ShowMessage("Preferred direction added");
                        EveryPrefDir->ConsolidatePrefDirs(1, ConstructPrefDir); // at 31
                        Level1Mode = PrefDirMode;
                        SetLevel1Mode(16);
                        Screen->Cursor = TCursor(-2); // Arrow
                        Utilities->CallLogPop(57);
                        return;
                    }
                    else
                    {
                        Level2PrefDirMode = PrefDirContinuing;
                        SetLevel2PrefDirMode(2);
                    }
                    // set again since 1st time
                    // PrefDir vector only had start element & Truncate wasn't enabled, also need
                    // to do the checks for Loop & End for each element as it is added
                }
            }
            else // same as start element
            {
                ConstructPrefDir->ExternalClearPrefDirAnd4MultiMap();
                Level1Mode = PrefDirMode;
                SetLevel1Mode(121);
                Screen->Cursor = TCursor(-2); // Arrow
                Utilities->CallLogPop(1714);
                return;
            }
            Screen->Cursor = TCursor(-2); // Arrow
            Utilities->CallLogPop(58);
            return;
        }

        else if(Level2PrefDirMode == PrefDirSelecting)
        {
            TrainController->LogEvent("mbLeft + PrefDirSelecting");
            ClearandRebuildRailway(56); // to get rid of earlier rectangles
            SelectStartPair.first = HLoc;
            SelectStartPair.second = VLoc;
        }

        else if(Level1Mode == OperMode)
        {
            if((Level2OperMode == Operating) && CallingOnButton->Down && CallingOnButton->Enabled)
            {
                TrainController->LogEvent("mbLeft + Operating & CallingOnButton->Down");
                int Position;
                TTrackElement TrackElement;
                if(Track->FindNonPlatformMatch(2, HLoc, VLoc, Position, TrackElement))
                {
                    if(TrackElement.TrackType != SignalPost)
                    {
                        CallingOnButton->Down = false;
// InfoPanel->Visible = false;  //dropped at v1.3.0, not sure what purpose intended to serve but don't want to lose the info panel as did with this here also added line below to reset
                        RevertToOriginalRouteSelector(16);
                        Utilities->CallLogPop(59);
                        return;
                    }
                    for(unsigned int x = 0; x < TrainController->TrainVector.size(); x++)
                    {
                        if(TrainController->TrainVectorAt(8, x).CallingOnFlag)
                        {
                            if((Track->TrackElementAt(428, TrainController->TrainVectorAt(26, x).LeadElement).Conn[TrainController->TrainVectorAt(27,
                                x).LeadExitPos] == Position) && (TrackElement.Config[Track->TrackElementAt(429, TrainController->TrainVectorAt(28,
                                x).LeadElement).ConnLinkPos[TrainController->TrainVectorAt(12, x).LeadExitPos]] == Connection))
                            {
                                // found it!
/*
                            if(TrackElement.SpeedTag == 68)
                                {
                                Display->PlotOutput(0, (HLoc * 16), (VLoc * 16), RailGraphics->bm68CallingOn);
                                }
                            if(TrackElement.SpeedTag == 69)
                                {
                                Display->PlotOutput(1, (HLoc * 16), (VLoc * 16), RailGraphics->bm69CallingOn);
                                }
                            if(TrackElement.SpeedTag == 70)
                                {
                                Display->PlotOutput(2, (HLoc * 16), (VLoc * 16), RailGraphics->bm70CallingOn);
                                }
                            if(TrackElement.SpeedTag == 71)
                                {
                                Display->PlotOutput(3, (HLoc * 16), (VLoc * 16), RailGraphics->bm71CallingOn);
                                }
                            if(TrackElement.SpeedTag == 72)
                                {
                                Display->PlotOutput(4, (HLoc * 16), (VLoc * 16), RailGraphics->bm72CallingOn);
                                }
                            if(TrackElement.SpeedTag == 73)
                                {
                                Display->PlotOutput(5, (HLoc * 16), (VLoc * 16), RailGraphics->bm73CallingOn);
                                }
                            if(TrackElement.SpeedTag == 74)
                                {
                                Display->PlotOutput(6, (HLoc * 16), (VLoc * 16), RailGraphics->bm74CallingOn);
                                }
                            if(TrackElement.SpeedTag == 75)
                                {
                                Display->PlotOutput(7, (HLoc * 16), (VLoc * 16), RailGraphics->bm75CallingOn);
                                }
*/
                                Track->TrackElementAt(430, Position).CallingOnSet = true;
                                Track->PlotSignal(13, Track->TrackElementAt(893, Position), Display);
// added at v 1.3.0 in place of the above to ensure ground signals (as well as others) plot correctly for proceed
                                // have to call after CallingOnSet becomes true & can't use TrackElement as that still has CallingOnSet false
                                ClearandRebuildRailway(69); // added at v1.3.0 to replot route on element after PlotSignal above
                                TrainController->TrainVectorAt(14, x).AllowedToPassRedSignal = true;
                                TrainController->TrainVectorAt(29, x).BeingCalledOn = true;
                                CallingOnButton->Down = false;
                                SetRouteButtonsInfoCaptionAndRouteNotStarted(10);

// set an unrestricted route into the station (just to the first platform) from the stop signal (note that it may be last in an autosigs
// route) but remove any single route elements first (can't reach here if constructing a route), else may try to extend a route that
// has been removed (only a precaution, shouldn't cause any probs whether single route set or not)
                                for(unsigned int x = 0; x < AllRoutes->AllRoutesSize(); x++)
                                {
                                    if(AllRoutes->GetFixedRouteAt(192, x).PrefDirSize() == 1)
                                    {
                                        // only allow route element to be removed if not selected for a route start otherwise
                                        // StartSelectionRouteID will be set & will fail at convert
                                        if(AllRoutes->GetFixedRouteAt(195, x).RouteID != ConstructRoute->StartSelectionRouteID.GetInt())
                                        {
                                            TPrefDirElement PDE = AllRoutes->GetFixedRouteAt(193, x).GetFixedPrefDirElementAt(212, 0);
                                            AllRoutes->RemoveRouteElement(21, PDE.HLoc, PDE.VLoc, PDE.GetELink());
                                            TrainController->LogEvent("SingleRouteElementRemovedDuringCallon, H = " + AnsiString(PDE.HLoc) + ", V = " +
                                                AnsiString(PDE.VLoc));
                                        }
                                    }
                                }

// find the correct entry in CallonVector - i.e. where Position == RouteStartEntry
                                for(unsigned int x = 0; x < AllRoutes->CallonVector.size(); x++)
                                {
                                    if(AllRoutes->CallonVector.at(x).RouteStartPosition == Position)
                                    { // found it
                                        if(!(AllRoutes->CallonVector.at(x).RouteOrPartRouteSet))
    // if RouteOrPartRouteSet false then set an unrestricted route into platform
                                        {
                                            bool PointsChanged = false;
                                            IDInt ReqPosRouteID(-1);
                                            TOneRoute *NewRoute = new TOneRoute;
                                            bool CallonTrue = true;
                                            bool ConsecSignalsRouteFalse = false;
                                            if(NewRoute->GetNonPreferredRouteStartElement(1,
                                                Track->TrackElementAt(841, AllRoutes->CallonVector.at(x).RouteStartPosition).HLoc,
                                                Track->TrackElementAt(842, AllRoutes->CallonVector.at(x).RouteStartPosition).VLoc, ConsecSignalsRouteFalse,
                                                CallonTrue))
                                            {
                                                if(NewRoute->GetNextNonPreferredRouteElement(1,
                                                    Track->TrackElementAt(843, AllRoutes->CallonVector.at(x).PlatformPosition).HLoc,
                                                    Track->TrackElementAt(844, AllRoutes->CallonVector.at(x).PlatformPosition).VLoc, ConsecSignalsRouteFalse,
                                                    CallonTrue, ReqPosRouteID, PointsChanged))
                                                {
                                                    if(!PointsChanged) // shouldn't be changed, something wrong if true so don't plot route
                                                    {
                                                        NewRoute->ConvertAndAddNonPreferredRouteSearchVector(3, ReqPosRouteID);
                                                        ClearandRebuildRailway(67); // to plot the route (only finds one so won't call repeatedly)
                                                    }
                                                }
                                            }
                                            delete NewRoute;
                                        }
                                    }
                                }
// InfoPanel->Visible = false;
                                Utilities->CallLogPop(60);
                                return;
                            }
                        }
                    }
                }
                CallingOnButton->Down = false;
                RevertToOriginalRouteSelector(3);
                Utilities->CallLogPop(61);
                return;
            }
/* get 1st element, first check if selected points, not in existing route, & in RouteNotStarted mode
               if so, set all the flash values, Track->PointFlashFlag & start time, then exit for flasher to take over.
               If any of above conditions not met then treat as route selection, setting route flasher if
               route continuing.
*/

            if((Level2OperMode == Operating) || (Level2OperMode == PreStart)) // not 'else if' as both may apply
                                                                              // disallow route setting if paused
            {
                if(Level2OperMode == PreStart)
                {
                    PointsFlashDuration = 0.0;
                    Track->LevelCrossingBarrierUpFlashDuration = 0.0;
                    Track->LevelCrossingBarrierDownFlashDuration = 0.0;
                }
                else
                {
                    float TempSpeedVal = 1; // added for v2.3.0 to keep durations same as x1 values for slow speeds
                    if(TTClockSpeed < 1)
                        TempSpeedVal = TTClockSpeed;
                    PointsFlashDuration = AllRoutes->PointsDelay * TempSpeedVal;
                    Track->LevelCrossingBarrierUpFlashDuration = AllRoutes->LevelCrossingBarrierUpDelay * TempSpeedVal;
                    Track->LevelCrossingBarrierDownFlashDuration = AllRoutes->LevelCrossingBarrierDownDelay * TempSpeedVal;
                }
                if(RouteMode == RouteNotStarted)
                {
                    TrainController->LogEvent("mbLeft + RouteNotStarted");
                    int Position;
                    TTrackElement TrackElement;
                    if(Track->FindNonPlatformMatch(3, HLoc, VLoc, Position, TrackElement))
                    {
                        if((TrackElement.TrackType == Points) && !(AllRoutes->TrackIsInARoute(1, Position, 0))
                            && !Track->PointFlashFlag && !Track->RouteFlashFlag)
                        // Flash selected points & changeover if appropriate
                        // need !Track->PointFlashFlag to prevent another point being selected while another is flashing, & !Track->RouteFlashFlag
                        // to ensure user only does one thing at a time
                        {
                            if(TrackElement.TrainIDOnElement > -1)
                            {
                                TrainController->StopTTClockMessage(1, "Can't change points under a train!");
                                Utilities->CallLogPop(62);
                                return;
                            }
                            PointFlash->SetScreenHVSource(1, TrackElement.HLoc * 16, TrackElement.VLoc * 16);

/*
                               This used to try to allow any linked trailing edges to cause both points to change, but no good if
                               there are two adjacent crossovers, where both trailing edges are linked to two different points.
                               The wrong link might be chosen.  Also doubtful if applying a strict order of checks would work, since
                               may be obscure configurations that would be wrong.  This function bypasses the MatchingPoint check, which
                               ensures that there are no obscure links.  Hence better to stick with original.

                               //check if trailing edge linked to another point trailing edge
                                            int DivergingPosition = TrackElement.Conn[1];
                                            TTrackElement DivergingElement = Track->TrackElementAt(431, TrackElement.Conn[1]);
                                            DivergingPointVectorPosition = -1;
                                            if((DivergingElement.TrackType == Points) &&
                                                    ((DivergingElement.Conn[1] == Position) || (DivergingElement.Conn[3] == Position)))
                                                {
                                                if(AllRoutes->TrackIsInARoute(, DivergingPosition))
                                                    {
                                                    ShowMessage("Linked points Locked");
                                                    }
                                                else DivergingPointVectorPosition = DivergingPosition;
                                                }
                                            else
                                                {
                                                DivergingPosition = TrackElement.Conn[3];
                                                DivergingElement = Track->TrackElementAt(432, TrackElement.Conn[3]);
                                                if((DivergingElement.TrackType == Points) &&
                                                        ((DivergingElement.Conn[1] == Position) || (DivergingElement.Conn[3] == Position)))
                                                    {
                                                    if(AllRoutes->TrackIsInARoute(, DivergingPosition))
                                                        {
                                                        ShowMessage("Linked points locked");
                                                        }
                                                    else DivergingPointVectorPosition = DivergingPosition;
                                                    }
                                                }
                                            Track->PointFlashFlag = true;
                                            PointFlashVectorPosition = Position;
                                            PointFlashStartTime = TrainController->TTClockTime;
                                            [close curly bracket -  if include it matches earlier non-commented one!]
*/
                            TTrackElement DivergingElement = Track->TrackElementAt(433, TrackElement.Conn[3]);
                            int DivergingPosition = TrackElement.Conn[3];
                            if((DivergingElement.TrackType == Points) && (DivergingElement.Conn[3] == Position) && (Track->MatchingPoint(1, Position,
                                DivergingPosition))) // full match inc same attributes
                            {
                                if(AllRoutes->TrackIsInARoute(4, DivergingPosition, 0))
                                {
                                    TrainController->StopTTClockMessage(2, "Linked points locked");
                                }
                                else
                                {
                                    Track->PointFlashFlag = true;
                                    PointFlashVectorPosition = Position;
                                    DivergingPointVectorPosition = DivergingPosition;
                                    PointFlashStartTime = TrainController->TTClockTime;
                                }
                            }
                            else // no matching point, just change this point
                            {
                                Track->PointFlashFlag = true;
                                PointFlashVectorPosition = Position;
                                DivergingPointVectorPosition = -1;
                                PointFlashStartTime = TrainController->TTClockTime;
                            }
                        }
/* drop manual changing of level crossings - only allow changing by setting a route through them
                    else if((Track->IsLCAtHV(23, HLoc, VLoc) && !Track->PointFlashFlag && !Track->RouteFlashFlag))//level crossing
                        {
                        TTrack::TFlashLevelCrossing FLC;
                        FLC.LCHLoc = HLoc;
                        FLC.LCVLoc = VLoc;
                        FLC.LCChangeStartTime = TrainController->TTClockTime;
                        FLC.LCBaseElementSpeedTag = TrackElement.SpeedTag;
                        if(Track->IsLCBarrierDownAtHV(0, HLoc, VLoc))
                            {
                            FLC.LCChangeDuration = LevelCrossingBarrierUpFlashDuration;
                            FLC.BarrierState = TTrack::Raising;
                            }
                        else
                            {
                            FLC.LCChangeDuration = LevelCrossingBarrierDownFlashDuration;
                            FLC.BarrierState = TTrack::Lowering;
                            }
                        Track->SetLinkedLevelCrossingBarrierAttributes(, HLoc, VLoc, 2);//set attr to 2 for changing state
                        Track->ChangingLCVector.push_back(FLC);
                        }
*/ else // route start
                        {
                            if(AutoSigsFlag)
                            {
                                AutoRouteStartMarker->SetScreenHVSource(2, TrackElement.HLoc * 16, TrackElement.VLoc * 16);
                                AutoRouteStartMarker->LoadOriginalScreenGraphic(0);
                            }
                            else if(ConsecSignalsRoute)
                            {
                                SigRouteStartMarker->SetScreenHVSource(3, TrackElement.HLoc * 16, TrackElement.VLoc * 16);
                                SigRouteStartMarker->LoadOriginalScreenGraphic(1);
                            }
                            else
                            {
                                NonSigRouteStartMarker->SetScreenHVSource(4, TrackElement.HLoc * 16, TrackElement.VLoc * 16);
                                NonSigRouteStartMarker->LoadOriginalScreenGraphic(2);
                            }
                            if(PreferredRoute)
                            {
                                if(!Track->PointFlashFlag && !Track->RouteFlashFlag) // don't allow a route to start if a point changing or
                                // another route building
                                {
                                    ConstructRoute->ClearRoute(); // in case not empty though should be
                                    if(ConstructRoute->GetPreferredRouteStartElement(0, HLoc, VLoc, EveryPrefDir, ConsecSignalsRoute, AutoSigsFlag))
                                    {
                                        if(AutoSigsFlag)
                                            AutoRouteStartMarker->PlotOverlay(1, Display);
                                        else
                                            SigRouteStartMarker->PlotOverlay(2, Display);
                                        RouteMode = RouteContinuing;
                                        InfoPanel->Visible = true;
                                        if(Level2OperMode == PreStart)
                                            InfoPanel->Caption = "PRE-START:  Select next route location";
                                        else
                                            InfoPanel->Caption = "OPERATING:  Select next route location";
                                    }
                                }
                                Utilities->CallLogPop(63);
                                return;
                            }
                            else // nonpreferred route
                            {
                                if(!Track->PointFlashFlag && !Track->RouteFlashFlag) // don't allow a route to start if a point changing or
                                // another route building
                                {
                                    ConstructRoute->ClearRoute(); // in case not empty though should be
                                    bool CallonFalse = false;
                                    if(ConstructRoute->GetNonPreferredRouteStartElement(0, HLoc, VLoc, ConsecSignalsRoute, CallonFalse))
                                    {
                                        NonSigRouteStartMarker->PlotOverlay(3, Display);
                                        RouteMode = RouteContinuing;
                                        InfoPanel->Visible = true;
                                        if(Level2OperMode == PreStart)
                                            InfoPanel->Caption = "PRE-START:  Select next route location";
                                        else
                                            InfoPanel->Caption = "OPERATING:  Select next route location";
                                    }
                                }
                                Utilities->CallLogPop(64);
                                return;
                            } // NonPreferred route
                        } // TrackType != Points
                    } // if(Track->FindNonPlatformMatch(HLoc, VLoc, Position, TrackElement))
                } // if(RouteMode == RouteNotStarted)
                else // RouteContinuing
                {
                    TrainController->LogEvent("mbLeft + RouteContinuing");
                    TrainController->StopTTClockFlag = true; // so TTClock stopped during MasterClockTimer function
                    TrainController->RestartTime = TrainController->TTClockTime;
                    RouteMode = RouteNotStarted;
                    AutoRouteStartMarker->PlotOriginal(14, Display); // if overlay not plotted will ignore
                    SigRouteStartMarker->PlotOriginal(15, Display); // if overlay not plotted will ignore
                    NonSigRouteStartMarker->PlotOriginal(16, Display); // if overlay not plotted will ignore
                    Screen->Cursor = TCursor(-11); // Hourglass - also set to an hourglass when flashing, after found required
                                                                  // element, but this sets it to an hourglass while searching
                    bool PointsChanged = false;
                    if(PreferredRoute)
                    {
                        // route added to AllRoutes in GetNextRouteElement if valid
                        // int ReqPosRouteNumber;
                        if(ConstructRoute->GetNextPreferredRouteElement(0, HLoc, VLoc, EveryPrefDir, ConsecSignalsRoute, AutoSigsFlag,
                            ConstructRoute->ReqPosRouteID, PointsChanged))
                        {
                            Track->RouteFlashFlag = true;
                            PreferredRouteFlag = true;
                            float TempSpeedVal = 1; // added for v2.3.0 to keep durations same as x1 values for slow speeds
                            if(TTClockSpeed < 1)
                                TempSpeedVal = TTClockSpeed;
                            if(Level2OperMode == PreStart)
                                RouteFlashDuration = 0.0;
                            else if(PointsChanged)
                                RouteFlashDuration = AllRoutes->PointsDelay * TempSpeedVal;
                            else
                                RouteFlashDuration = AllRoutes->SignalsDelay * TempSpeedVal;
                            ConstructRoute->SetRouteFlashValues(1, AutoSigsFlag, true); // true for ConsecSignalsRoute
                            RouteFlashStartTime = TrainController->TTClockTime;
                        }
                        else
                        {
                            RevertToOriginalRouteSelector(4);
                        }
                        Screen->Cursor = TCursor(-2); // Arrow
                        TrainController->BaseTime = TDateTime::CurrentDateTime();
                        TrainController->StopTTClockFlag = false;
                        Utilities->CallLogPop(65);
                        return;
                    }
                    else
                    {
                        bool CallonFalse = false;
                        if(ConstructRoute->GetNextNonPreferredRouteElement(0, HLoc, VLoc, ConsecSignalsRoute, CallonFalse, ConstructRoute->ReqPosRouteID,
                            PointsChanged))
                        {
                            Track->RouteFlashFlag = true;
                            PreferredRouteFlag = false;
                            float TempSpeedVal = 1; // added for v2.3.0 to keep durations same as x1 values for slow speeds
                            if(TTClockSpeed < 1)
                                TempSpeedVal = TTClockSpeed;
                            if(Level2OperMode == PreStart)
                                RouteFlashDuration = 0.0;
                            else if(PointsChanged)
                                RouteFlashDuration = AllRoutes->PointsDelay * TempSpeedVal;
                            else
                                RouteFlashDuration = AllRoutes->SignalsDelay * TempSpeedVal;
                            ConstructRoute->SetRouteFlashValues(2, false, false);
                            RouteFlashStartTime = TrainController->TTClockTime;
                        }
                        else
                        {
                            RevertToOriginalRouteSelector(5);
                        }
                    }
                    TrainController->BaseTime = TDateTime::CurrentDateTime();
                    TrainController->StopTTClockFlag = false;
                    Screen->Cursor = TCursor(-2); // Arrow
                }
                Utilities->CallLogPop(66);
                return;
            }
        }

        Utilities->CallLogPop(68);
    }
    catch(const Exception &e)
    {
        ErrorLog(20, e.Message);
    }
}

// ---------------------------------------------------------------------------

void TInterface::MainScreenMouseDown3(int Caller, TMouseButton Button, TShiftState Shift, int X, int Y)
    // ZoomOut mode
{
// NB: DisplayZoomOutOffsetH & V take account of the Min & Max H & V values so don't need these again
    try
    {
        TrainController->LogEvent("MainScreenMouseDown3," + AnsiString(Button) + "," + AnsiString(X) + "," + AnsiString(Y));
        Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",MainScreenMouseDown3," + AnsiString(Button) + "," + AnsiString(X) +
            "," + AnsiString(Y));
        if(Button != mbLeft)
        {
            // this routine new at v2.1.0.  Allows railway moving for zoom-out mode
            StartWholeRailwayMoveHPos = X;
            StartWholeRailwayMoveVPos = Y;
            WholeRailwayMoving = true;
            Screen->Cursor = TCursor(-22); // Four arrows;
        }
        else
        {
            InfoPanel->Visible = false; // reset infopanel in case not set later
            InfoPanel->Caption = "";
            int HRounding, VRounding;
            int TruePosH = (X / 4) + Display->DisplayZoomOutOffsetH;
            int TruePosV = (Y / 4) + Display->DisplayZoomOutOffsetV;
            // find nearest screen centre - from 30 to 210 horiz & from 18 to 126 vert
            if(TruePosH < 0)
                HRounding = -(Utilities->ScreenElementWidth / 4);
            else
                HRounding = (Utilities->ScreenElementWidth / 4);
            int CentreH = (((TruePosH + HRounding) / (Utilities->ScreenElementWidth / 2)) * (Utilities->ScreenElementWidth / 2));
            while((CentreH - Track->GetHLocMax()) >= (Utilities->ScreenElementWidth / 2))
                CentreH -= (Utilities->ScreenElementWidth / 2);
            while((Track->GetHLocMin() - CentreH) >= (Utilities->ScreenElementWidth / 2))
                CentreH += (Utilities->ScreenElementWidth / 2);
            if(TruePosV < 0)
                VRounding = -(Utilities->ScreenElementHeight / 4);
            else
                VRounding = (Utilities->ScreenElementHeight / 4);
            int CentreV = (((TruePosV + VRounding) / (Utilities->ScreenElementHeight / 2)) * (Utilities->ScreenElementHeight / 2));
            while((CentreV - Track->GetVLocMax()) >= (Utilities->ScreenElementHeight / 2))
                CentreV -= (Utilities->ScreenElementHeight / 2);
            while((Track->GetVLocMin() - CentreV) >= (Utilities->ScreenElementHeight / 2))
                CentreV += (Utilities->ScreenElementHeight / 2);
            Display->DisplayOffsetH = CentreH - (Utilities->ScreenElementWidth / 2);
            Display->DisplayOffsetV = CentreV - (Utilities->ScreenElementHeight / 2);

            TLevel2OperMode TempLevel2OperMode = Level2OperMode;
            if(Level1Mode == BaseMode)
                SetLevel1Mode(17);
            else if(Level1Mode == TrackMode)
            {
                // set edit menu items
                SetInitialTrackModeEditMenu();
                PreventGapOffsetResetting = true; // when return from zoom by clicking screen don't force a return to the
                                                  // displayed gap, user wants to display the clicked area
                SetLevel2TrackMode(32); // revert to earlier track mode from zoom
                PreventGapOffsetResetting = false;
            }
            else if(Level1Mode == PrefDirMode)
            {
                if(Level2PrefDirMode == PrefDirContinuing)
                    SetLevel2PrefDirMode(3); // revert to earlier PrefDir mode from zoom
                else
                    SetLevel1Mode(33); // if PrefDirSelecting revert to normap PrefDirMode
            }
            // else if(Level1Mode == TrackMode) SetLevel1Mode();//just revert to basic track mode from zoom
            // else if(Level1Mode == PrefDirMode) SetLevel1Mode();//just revert to basic PrefDir mode from zoom
            else if(Level1Mode == TimetableMode)
            {
                InfoPanel->Visible = false;
            }
            // Not OperMode or RestartSessionOperMode as that resets the performance file
            else if(TempLevel2OperMode == Operating) // similar to SetLevel2OperMode but without resetting BaseTime
            {
                OperateButton->Enabled = true;
                OperateButton->Glyph->LoadFromResourceName(0, "PauseGraphic");
                ExitOperationButton->Enabled = true;
                SetRouteButtonsInfoCaptionAndRouteNotStarted(0);
            }
            else if(TempLevel2OperMode == Paused) // similar to SetLevel2OperMode but without resetting RestartTime
            {
                OperateButton->Enabled = true;
                OperateButton->Glyph->LoadFromResourceName(0, "RunGraphic");
                ExitOperationButton->Enabled = true;
                TTClockAdjButton->Enabled = true;
                SetRouteButtonsInfoCaptionAndRouteNotStarted(6);
                DisableRouteButtons(0);
            }
            else if(TempLevel2OperMode == PreStart)
            {
                OperateButton->Enabled = true;
                OperateButton->Glyph->LoadFromResourceName(0, "RunGraphic");
                ExitOperationButton->Enabled = true;
                TTClockAdjButton->Enabled = true;
                SetRouteButtonsInfoCaptionAndRouteNotStarted(8);
            }
            Display->ZoomOutFlag = false; // reset this after level modes called so gap flash stays set if set to begin with
            SetPausedOrZoomedInfoCaption(0);
            ClearandRebuildRailway(44);
        }
        Utilities->CallLogPop(69);
    }
    catch(const Exception &e)
    {
        ErrorLog(21, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::MainScreenMouseMove(TObject *Sender, TShiftState Shift, int X, int Y)
{
    try
    {
        // TrainController->LogEvent("MainScreenMouseMove," + AnsiString(X) + "," + AnsiString(Y));    //dropped at v0.6, too many events
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",MainScreenMouseMove," + AnsiString(X) + "," + AnsiString(Y));

        if(!mbLeftDown && WholeRailwayMoving) // new at v2.1.0
        {
            TrainController->StopTTClockFlag = true; // so TTClock stopped during MasterClockTimer function
            TrainController->RestartTime = TrainController->TTClockTime;
            if(X < 0)
                X = 0; // ensure pointer stays within display area
            if(X > (MainScreen->Width - 1))
                X = MainScreen->Width - 1;
            if(Y < 0)
                Y = 0;
            if(Y > (MainScreen->Height - 1))
                Y = MainScreen->Height - 1;

            if(!Display->ZoomOutFlag)
            {
                int StartOffsetX = (X - StartWholeRailwayMoveHPos) % 16;
                int StartOffsetY = (Y - StartWholeRailwayMoveVPos) % 16;
                if((abs(X - StartWholeRailwayMoveHPos) >= 16) || (abs(Y - StartWholeRailwayMoveVPos) >= 16))
                {
                    int NewH = X - StartWholeRailwayMoveHPos;
                    int NewV = Y - StartWholeRailwayMoveVPos;
                    Display->DisplayOffsetH -= NewH / 16;
                    Display->DisplayOffsetV -= NewV / 16;
                    StartWholeRailwayMoveHPos = X - StartOffsetX;
                    StartWholeRailwayMoveVPos = Y - StartOffsetY;
                    ClearandRebuildRailway(71);
                    if((Level2TrackMode == TrackSelecting) || (Level2PrefDirMode == PrefDirSelecting))
                    {
                        Display->PlotDashedRect(3, SelectRect);
                    }
                }
            }

            else
            {
                int StartZOffsetX = (X - StartWholeRailwayMoveHPos) % 4;
                int StartZOffsetY = (Y - StartWholeRailwayMoveVPos) % 4;
                if((abs(X - StartWholeRailwayMoveHPos) >= 4) || (abs(Y - StartWholeRailwayMoveVPos) >= 4))
                {
                    int NewH = X - StartWholeRailwayMoveHPos;
                    int NewV = Y - StartWholeRailwayMoveVPos;
                    Display->DisplayZoomOutOffsetH -= NewH / 4;
                    Display->DisplayZoomOutOffsetV -= NewV / 4;
                    StartWholeRailwayMoveHPos = X - StartZOffsetX;
                    StartWholeRailwayMoveVPos = Y - StartZOffsetY;
                    Display->ClearDisplay(10);
                    Track->PlotSmallRailway(8, Display);
                }
            }
            TrainController->BaseTime = TDateTime::CurrentDateTime();
            TrainController->StopTTClockFlag = false;
        }

        else if(mbLeftDown)
        {
            if(Level2TrackMode == TrackSelecting)
/* [Repeated from MouseDown] - When 'select' chosen from the Edit menu (only available in 'AddTrack') conditions are set ready to enclose a rectangular screen area
                                using MouseMove.  When MouseDown occurs the starting point is marked (wrt whole railway, not just the screen) and stored in
                                SelectStartPair.  If the mouse button is released and a new start position selected then the earlier one is discarded.  Providing the
                                button is held down subsequent actions occur during MouseMove (to display the changing rectangle) and MouseUp to define the final
                                selected rectangle.
                                [New] At this point the select starting position has been defined in SelectStartPair, and the current mouse position is defined (wrt whole
                                railway) in HLoc & VLoc from the screen positions X & Y by GetTrackLocsFromScreenPos.  Both are incremented so that the rectangle
                                includes the current point (if no mouse movement at all occurs then a 1 x 1 rectangle is displayed).  Limits are set to prevent the
                                displayed rectangle extending off screen.  Edges are set at 60 & 36 rather than 59 & 35 because the defined rectangle excludes the
                                rightmost and bottom HLoc & VLoc values, if 59 & 35 were used the right & bottom screen edges wouldn't be reached.  A TRect is then
                                defined from SelectStartPair and the HLoc/VLoc values, Clearand... called to clear earlier rectangles, and a dashed edge drawn round
                                the selection.
*/
            {
                TrainController->LogEvent("MouseMove + TrackSelecting");
                int CurrentHLoc, CurrentVLoc, StartHLoc = SelectStartPair.first, StartVLoc = SelectStartPair.second;
                Track->GetTrackLocsFromScreenPos(2, CurrentHLoc, CurrentVLoc, X, Y);
                // to make the rectangle inclusive of the start and current points, need to increase the HLoc value of the
                // rightmost point and the VLoc value of the bottommost point
                if(CurrentHLoc >= StartHLoc)
                    CurrentHLoc++;
                else
                    StartHLoc++;
                if(CurrentVLoc >= StartVLoc)
                    CurrentVLoc++;
                else
                    StartVLoc++;
                if(CurrentHLoc - Display->DisplayOffsetH > Utilities->ScreenElementWidth)
                    CurrentHLoc = Display->DisplayOffsetH + Utilities->ScreenElementWidth;
                if(CurrentVLoc - Display->DisplayOffsetV > Utilities->ScreenElementHeight)
                    CurrentVLoc = Display->DisplayOffsetV + Utilities->ScreenElementHeight;
                if(CurrentHLoc - Display->DisplayOffsetH < 0)
                    CurrentHLoc = Display->DisplayOffsetH;
                if(CurrentVLoc - Display->DisplayOffsetV < 0)
                    CurrentVLoc = Display->DisplayOffsetV;
                TRect TempRect(StartHLoc, StartVLoc, CurrentHLoc, CurrentVLoc);
                ClearandRebuildRailway(14); // to clear earlier rectangles
                Display->PlotDashedRect(0, TempRect);
                Display->Update(); // resurrected when Update() dropped from PlotOutput etc
            }

            else if(Level2PrefDirMode == PrefDirSelecting)
            {
                TrainController->LogEvent("MouseMove + PrefDirSelecting");

                int CurrentHLoc, CurrentVLoc, StartHLoc = SelectStartPair.first, StartVLoc = SelectStartPair.second;
                Track->GetTrackLocsFromScreenPos(5, CurrentHLoc, CurrentVLoc, X, Y);
                // to make the rectangle inclusive of the start and current points, need to increase the HLoc value of the
                // rightmost point and the VLoc value of the bottommost point
                if(CurrentHLoc >= StartHLoc)
                    CurrentHLoc++;
                else
                    StartHLoc++;
                if(CurrentVLoc >= StartVLoc)
                    CurrentVLoc++;
                else
                    StartVLoc++;
                if(CurrentHLoc - Display->DisplayOffsetH > Utilities->ScreenElementWidth)
                    CurrentHLoc = Display->DisplayOffsetH + Utilities->ScreenElementWidth;
                if(CurrentVLoc - Display->DisplayOffsetV > Utilities->ScreenElementHeight)
                    CurrentVLoc = Display->DisplayOffsetV + Utilities->ScreenElementHeight;
                if(CurrentHLoc - Display->DisplayOffsetH < 0)
                    CurrentHLoc = Display->DisplayOffsetH;
                if(CurrentVLoc - Display->DisplayOffsetV < 0)
                    CurrentVLoc = Display->DisplayOffsetV;
                TRect TempRect(StartHLoc, StartVLoc, CurrentHLoc, CurrentVLoc);
                ClearandRebuildRailway(57); // to clear earlier rectangles
                Display->PlotDashedRect(2, TempRect);
                Display->Update(); // need to keep this since Update() not called for PlotSmallOutput as too slow
            }

            else if(((Level2TrackMode == CopyMoving) || (Level2TrackMode == CutMoving)) && SelectPickedUp)
/* [Repeated from MouseDown] - The same actions apply on MouseDown whether Copy or Cut selected from the menu.  First the horizontal and vertical mouse position is
                               checked and unless it lies within the selected rectangle and not within 4 pixels of an edge the pickup fails and the function returns.
                               Otherwise flag SelectPickedUp is set to true (to allow it to move during MouseMove and remain in place at MouseUp) and the mouse position
                               is saved in SelectBitmapMouseLocX & Y for use later in MouseMove & MouseUp.
                               [New] - The same actions apply on MouseMove whether Copy or Cut selected from the menu.  The X & Y mouse positions are checked and set to
                               stay within the display area.  Then the current selection H & V positions are stored in NewSelectBitmapHLoc & VLoc.
                               These change continually while the mouse and the selection are moving, they are only read on MouseUp to retain the position that it then
                               occupies.  Clearand... is called finally to clear earlier selection displays.
*/
            {
                TrainController->LogEvent("MouseMove + Copy or CutMoving & SelectPickedUp");
                if(X < 0)
                    X = 0; // ensure pointer stays within display area
                if(X > (MainScreen->Width - 1))
                    X = MainScreen->Width - 1;
                if(Y < 0)
                    Y = 0;
                if(Y > (MainScreen->Height - 1))
                    Y = MainScreen->Height - 1;
                NewSelectBitmapHLoc = (X - SelectBitmapMouseLocX) / 16 + SelectBitmapHLoc;
                NewSelectBitmapVLoc = (Y - SelectBitmapMouseLocY) / 16 + SelectBitmapVLoc;
                ClearandRebuildRailway(15); // plots SelectBitmap at the position given by NewSelectBitmapHLoc & ...VLoc
            }

            else if((Level2TrackMode == MoveTextOrGraphic) && TextFoundFlag)
            {
                TrainController->LogEvent("MouseMove + MoveTextOrGraphic & TextFoundFlag");
                int NewHPos = TextOrUserGraphicGridVal * (div(X - StartX, TextOrUserGraphicGridVal).quot) + TextMoveHPos + Display->DisplayOffsetH * 16;
                NewHPos = TextOrUserGraphicGridVal * (div(NewHPos, TextOrUserGraphicGridVal).quot);
                int NewVPos = TextOrUserGraphicGridVal * (div(Y - StartY, TextOrUserGraphicGridVal).quot) + TextMoveVPos + Display->DisplayOffsetV * 16;
                NewVPos = TextOrUserGraphicGridVal * (div(NewVPos, TextOrUserGraphicGridVal).quot);

                TextHandler->TextPtrAt(26, TextItem)->HPos = NewHPos;
                TextHandler->TextPtrAt(27, TextItem)->VPos = NewVPos;
                ClearandRebuildRailway(41);
            }

            else if((Level2TrackMode == MoveTextOrGraphic) && UserGraphicFoundFlag)
            {
                TrainController->LogEvent("MouseMove + MoveTextOrGraphic & UserGraphicFoundFlag");
                int NewHPos = TextOrUserGraphicGridVal * (div(X - StartX, TextOrUserGraphicGridVal).quot) + UserGraphicMoveHPos + Display->DisplayOffsetH * 16;
                NewHPos = TextOrUserGraphicGridVal * (div(NewHPos, TextOrUserGraphicGridVal).quot);
                int NewVPos = TextOrUserGraphicGridVal * (div(Y - StartY, TextOrUserGraphicGridVal).quot) + UserGraphicMoveVPos + Display->DisplayOffsetV * 16;
                NewVPos = TextOrUserGraphicGridVal * (div(NewVPos, TextOrUserGraphicGridVal).quot);

                Track->UserGraphicVectorAt(24, UserGraphicVectorNumber).HPos = NewHPos;
                Track->UserGraphicVectorAt(25, UserGraphicVectorNumber).VPos = NewVPos;
                ClearandRebuildRailway(79);
            }
        }
        Utilities->CallLogPop(70);
    }
    catch(const Exception &e)
    {
        ErrorLog(22, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::MainScreenMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
/* [Repeated from MouseDown] - When 'select' chosen from the Edit menu (only available in 'AddTrack') conditions are set ready to enclose a rectangular screen area
   using MouseMove.  When MouseDown occurs the starting point is marked (wrt whole railway, not just the screen) and stored in
   SelectStartPair.  If the mouse button is released and a new start position selected then the earlier one is discarded.  Providing the
   button is held down subsequent actions occur during MouseMove (to display the changing rectangle) and MouseUp to define the final
   selected rectangle.
   [Repeated from MouseMove] - At this point the select starting position has been defined in SelectStartPair, and the current mouse position is defined (wrt whole
   railway) in HLoc & VLoc from the screen positions X & Y by GetTrackLocsFromScreenPos.  Both are incremented so that the rectangle
   includes the current point (if no mouse movement at all occurs then a 1 x 1 rectangle is displayed).  Limits are set to prevent the
   displayed rectangle extending off screen.  Edges are set at 60 & 36 rather than 59 & 35 because the defined rectangle excludes the
   rightmost and bottom HLoc & VLoc values, if 59 & 35 were used the right & bottom screen edges wouldn't be reached.  A TRect is then
   defined from SelectStartPair and the HLoc/VLoc values, Clearand... called to clear earlier rectangles, and a dashed edge drawn round
   the selection.
   [New] This function can take some time so an houglass cursor is displayed.  The rectangle is fully defined, so the final screen X & Y
   values are translated into HLoc & VLoc values (wrt whole railway) and SelectEndPair set using them.  The rectangle can be defined in any
   direction, so the end points may be before or after the starting points for both horizontal and vertical directions.  Therefore the
   rectangle that will be used subsequently - SelectRect - is defined from SelectStart and SelectEnd allowing for any direction.  Screen
   limits are set as during MouseMove, and a dashed edge drawn as before.  Then a check is made to see if the final rectangle has any area,
   and if not 'Select' mode is kept and the function ends so that a new rectangle can be drawn, otherwise new menu items Cut, Copy & Delete,
   are enabled.  Now the SelectBitmap is made ready by filling with white prior to the track bitmaps being copied.  If this isn't done the
   track bitmaps are loaded from the top left hand corner and the rest becomes black - not what is wanted!  The SelectVector (defined in
   TrackUnit) is then loaded with the elements enclosed by the rectangle, top to bottom and left to right, active track elements first then
   inactive track elements.  Empty squares are ignored as are default (erased) elements.  Now the SelectVector is read and the corresponding
   element bitmaps transferred to SelectBitmap in the appropriate positions, then a dashed border added.  Finally the cursor is changed back
   to an arrow.
*/
    try
    {
        TrainController->LogEvent("MainScreenMouseUp," + AnsiString(Button) + "," + AnsiString(X) + "," + AnsiString(Y));
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",MainScreenMouseUp," + AnsiString(Button) + "," + AnsiString(X) + "," + AnsiString(Y));
        WholeRailwayMoving = false; // added at v2.1.0
        Screen->Cursor = TCursor(-2); // Arrow; (to reset from four arrows when moving) added at v2.1.0
        if((Level2TrackMode == TrackSelecting) && mbLeftDown)
        {
            TrainController->LogEvent("MouseUp + TrackSelecting + mbLeftDown");
            Screen->Cursor = TCursor(-11); // Hourglass;
            int EndHLoc, EndVLoc, StartHLoc = SelectStartPair.first, StartVLoc = SelectStartPair.second;
            Track->GetTrackLocsFromScreenPos(3, EndHLoc, EndVLoc, X, Y); // these values don't allow for offsets so add in later
// to make the rectangle inclusive of the start and current points, need to increase the HLoc value of the
// rightmost point and the VLoc value of the bottommost point
            if(EndHLoc >= StartHLoc)
                EndHLoc++;
            else
                StartHLoc++;
            if(EndVLoc >= StartVLoc)
                EndVLoc++;
            else
                StartVLoc++;
            if(StartHLoc >= EndHLoc)
            {
                SelectRect.left = EndHLoc;
                SelectRect.right = StartHLoc;
            }
            else
            {
                SelectRect.left = StartHLoc;
                SelectRect.right = EndHLoc;
            }
            if(StartVLoc >= EndVLoc)
            {
                SelectRect.top = EndVLoc;
                SelectRect.bottom = StartVLoc;
            }
            else
            {
                SelectRect.top = StartVLoc;
                SelectRect.bottom = EndVLoc;
            }
            if(SelectRect.right - Display->DisplayOffsetH > Utilities->ScreenElementWidth)
                SelectRect.right = Display->DisplayOffsetH + Utilities->ScreenElementWidth;
            if(SelectRect.bottom - Display->DisplayOffsetV > Utilities->ScreenElementHeight)
                SelectRect.bottom = Display->DisplayOffsetV + Utilities->ScreenElementHeight;
            if(SelectRect.left - Display->DisplayOffsetH < 0)
                SelectRect.left = Display->DisplayOffsetH;
            if(SelectRect.top - Display->DisplayOffsetV < 0)
                SelectRect.top = Display->DisplayOffsetV;
            Display->PlotDashedRect(1, SelectRect);
            SelectBitmapHLoc = SelectRect.left;
            SelectBitmapVLoc = SelectRect.top;
            if((SelectRect.top == SelectRect.bottom) || (SelectRect.left == SelectRect.right))
            {
                SelectionValid = false;
                SetInitialTrackModeEditMenu();
                mbLeftDown = false;
                Screen->Cursor = TCursor(-2); // Arrow;
                Utilities->CallLogPop(71);
                return; // no rectangle
            }
            else
            {
                ReselectMenuItem->Enabled = false;
                CutMenuItem->Enabled = true;
                CopyMenuItem->Enabled = true;
                FlipMenuItem->Enabled = true;
                MirrorMenuItem->Enabled = true;
                RotRightMenuItem->Enabled = true;
                RotLeftMenuItem->Enabled = true;
                RotateMenuItem->Enabled = true;
                PasteMenuItem->Enabled = false;
// PasteWithAttributesMenuItem->Enabled = false;  //new menu item for v2.2.0 only enabled after cutting
                DeleteMenuItem->Enabled = true;
                if(Track->IsTrackFinished())
                    SelectLengthsMenuItem->Enabled = true; // only permit if finished because reverts to DistanceStart
                else
                    SelectLengthsMenuItem->Enabled = false; // and that can only be used if track linked
                SelectBiDirPrefDirsMenuItem->Visible = false;
                CancelSelectionMenuItem->Enabled = true;
                // set SelectBitmap
                SelectBitmap->Width = (SelectRect.right - SelectRect.left) * 16;
                SelectBitmap->Height = (SelectRect.bottom - SelectRect.top) * 16;

                // fill it with transparent white (i.e. use Draw) else graphics all plot from top left hand corner
                for(int H = 0; H < (SelectBitmap->Width) / 16; H++)
                {
                    for(int V = 0; V < (SelectBitmap->Height) / 16; V++)
                    {
                        SelectBitmap->Canvas->Draw(H * 16, V * 16, RailGraphics->bmSolidBgnd);
                        // NB in above if use bmTransparent it ISN'T transparent, but if use the non-transparent bmSolidBgnd it IS transparent
                        // presumably superimposing a transparent bitmap onto a transparent bitmap makes the result non-transparent!
                    }
                }

                // store elements in Track->SelectVector, active elements first then inactive so active element plotted first during paste
                // clear the vector first
                Track->SelectVectorClear();
                TTrackElement TempElement; // default element
                bool FoundFlag;
                for(int x = SelectRect.left; x < SelectRect.right; x++)
                {
                    for(int y = SelectRect.top; y < SelectRect.bottom; y++)
                    {
                        int ATVecPos = Track->GetVectorPositionFromTrackMap(2, x, y, FoundFlag);
                        if(FoundFlag)
                        {
                            TempElement = Track->TrackElementAt(440, ATVecPos);
                            if(TempElement.SpeedTag > 0)
                                Track->SelectPush(TempElement); // don't store erase elements
                        }
                    }
                }
                // now store inactive elements
                for(int x = SelectRect.left; x < SelectRect.right; x++)
                {
                    for(int y = SelectRect.top; y < SelectRect.bottom; y++)
                    {
                        TTrack::TIMPair IATVecPair = Track->GetVectorPositionsFromInactiveTrackMap(2, x, y, FoundFlag);
                        if(FoundFlag)
                        {
                            TempElement = Track->InactiveTrackElementAt(30, IATVecPair.first);
                            Track->SelectPush(TempElement); // only want SpeedTag & location set, rest defaults
                            if(IATVecPair.second != IATVecPair.first) // 2 elements stored at location, i.e. platforms
                            {
                                TempElement = Track->InactiveTrackElementAt(31, IATVecPair.second);
                                Track->SelectPush(TempElement);
                            }
                        }
                    }
                }
                // store text items
                int LowSelectHPos = SelectRect.left * 16;
                int HighSelectHPos = SelectRect.right * 16;
                int LowSelectVPos = SelectRect.top * 16;
                int HighSelectVPos = SelectRect.bottom * 16;
                TextHandler->SelectTextVector.clear();
                if(!TextHandler->TextVector.empty()) // skip iteration if empty else have an error
                {
                    for(TTextHandler::TTextVectorIterator TextPtr = TextHandler->TextVector.begin(); TextPtr < TextHandler->TextVector.end(); TextPtr++)
                    {
                        if((TextPtr->HPos >= LowSelectHPos) && (TextPtr->HPos < HighSelectHPos) && (TextPtr->VPos >= LowSelectVPos) && (TextPtr->VPos <
                            HighSelectVPos))
                        {
                            // have to create a new TextItem in order to create a new Font object
                            // BUT: only create new items where they don't appear as named location names
                            // in SelectVector, since those names shouldn't be copied or pasted.
                            // NB:  altered for PasteWithAttributes - at v2.2.0 save the named element but prefix it with "##**"
                            // so can paste or not depending on which type of paste is being used (unlikely to use that in a real name)
                            bool SelectVectorNamedElement = false;
                            AnsiString SelectTextString; // new at v2.2.0
                            for(unsigned int x = 0; x < Track->SelectVector.size(); x++)
                            {
                                if(Track->SelectVector.at(x).LocationName == TextPtr->TextString)
                                {
                                    SelectVectorNamedElement = true;
                                    break;
                                }
                            }
                            if(SelectVectorNamedElement) // changed at v2.2.0
                            {
                                SelectTextString = "##**" + TextPtr->TextString; // new at v2.2.0
                            }
                            else // new at v2.2.0
                            {
                                SelectTextString = TextPtr->TextString;
                            }
                            TTextItem TextItem(TextPtr->HPos, TextPtr->VPos, SelectTextString, TextPtr->Font);
                            TextHandler->SelectTextVector.push_back(TextItem); // changed at v2.2.0
                        }
                    }
                }
                // store graphic items, but first clear SelectGraphicVector
                Track->SelectGraphicVector.clear();
                if(!Track->UserGraphicVector.empty()) // skip iteration if empty else have an error
                {
                    for(TTrack::TUserGraphicVector::iterator UserGraphicPtr = Track->UserGraphicVector.begin(); UserGraphicPtr < Track->UserGraphicVector.end();
                    UserGraphicPtr++)
                    {
                        if((UserGraphicPtr->HPos >= LowSelectHPos) && ((UserGraphicPtr->HPos + UserGraphicPtr->Width) < HighSelectHPos) &&
                            (UserGraphicPtr->VPos >= LowSelectVPos) && ((UserGraphicPtr->VPos + UserGraphicPtr->Height) < HighSelectVPos))
                        {
                            Track->SelectGraphicVector.push_back(*UserGraphicPtr);
                        }
                    }
                }
// new method - direct copying of existing selection so text included
                TRect Dest(0, 0, SelectBitmap->Width, SelectBitmap->Height);
                TRect Source(((SelectRect.left - Display->DisplayOffsetH) * 16), ((SelectRect.top - Display->DisplayOffsetV) * 16),
                    ((SelectRect.right - Display->DisplayOffsetH) * 16), ((SelectRect.bottom - Display->DisplayOffsetV) * 16));
                SelectBitmap->Canvas->CopyRect(Dest, MainScreen->Canvas, Source);
                SelectionValid = true;
            }
            Screen->Cursor = TCursor(-2); // Arrow;
        }

        else if((Level2PrefDirMode == PrefDirSelecting) && mbLeftDown)
        {
            TrainController->LogEvent("MouseUp + PrefDirSelecting + mbLeftDown");
            Screen->Cursor = TCursor(-11); // Hourglass;

            int EndHLoc, EndVLoc, StartHLoc = SelectStartPair.first, StartVLoc = SelectStartPair.second;
            Track->GetTrackLocsFromScreenPos(6, EndHLoc, EndVLoc, X, Y); // these values don't allow for offsets so add in later
// to make the rectangle inclusive of the start and current points, need to increase the HLoc value of the
// rightmost point and the VLoc value of the bottommost point
            if(EndHLoc >= StartHLoc)
                EndHLoc++;
            else
                StartHLoc++;
            if(EndVLoc >= StartVLoc)
                EndVLoc++;
            else
                StartVLoc++;
            if(StartHLoc >= EndHLoc)
            {
                SelectRect.left = EndHLoc;
                SelectRect.right = StartHLoc;
            }
            else
            {
                SelectRect.left = StartHLoc;
                SelectRect.right = EndHLoc;
            }
            if(StartVLoc >= EndVLoc)
            {
                SelectRect.top = EndVLoc;
                SelectRect.bottom = StartVLoc;
            }
            else
            {
                SelectRect.top = StartVLoc;
                SelectRect.bottom = EndVLoc;
            }
            if(SelectRect.right - Display->DisplayOffsetH > Utilities->ScreenElementWidth)
                SelectRect.right = Display->DisplayOffsetH + Utilities->ScreenElementWidth;
            if(SelectRect.bottom - Display->DisplayOffsetV > Utilities->ScreenElementHeight)
                SelectRect.bottom = Display->DisplayOffsetV + Utilities->ScreenElementHeight;
            if(SelectRect.left - Display->DisplayOffsetH < 0)
                SelectRect.left = Display->DisplayOffsetH;
            if(SelectRect.top - Display->DisplayOffsetV < 0)
                SelectRect.top = Display->DisplayOffsetV;
            Display->PlotDashedRect(9, SelectRect);
            SelectBitmapHLoc = SelectRect.left;
            SelectBitmapVLoc = SelectRect.top;
            if((SelectRect.top == SelectRect.bottom) || (SelectRect.left == SelectRect.right))
            {
                SetInitialPrefDirModeEditMenu();
                mbLeftDown = false;
                Screen->Cursor = TCursor(-2); // Arrow;
                Utilities->CallLogPop(1551);
                return; // no rectangle
            }
            else
            {
                SelectBiDirPrefDirsMenuItem->Enabled = true;
                CancelSelectionMenuItem->Enabled = true;
                // don't need SelectBitmap for PrefDir selection

                // store active elements in Track->SelectVector, ignore inactive elements
                // clear the vector first
                Track->SelectVectorClear();
                TTrackElement TempElement; // default element
                bool FoundFlag;
                for(int x = SelectRect.left; x < SelectRect.right; x++)
                {
                    for(int y = SelectRect.top; y < SelectRect.bottom; y++)
                    {
                        int ATVecPos = Track->GetVectorPositionFromTrackMap(43, x, y, FoundFlag);
                        if(FoundFlag)
                        {
                            TempElement = Track->TrackElementAt(729, ATVecPos);
                            if(TempElement.SpeedTag > 0)
                                Track->SelectPush(TempElement); // don't store erase elements
                        }
                    }
                }
            }
            Screen->Cursor = TCursor(-2); // Arrow;
        }

        else if(((Level2TrackMode == CopyMoving) || (Level2TrackMode == CutMoving)) && mbLeftDown && SelectPickedUp)
/* [Repeated from MouseDown] - The same actions apply on MouseDown whether Copy or Cut selected from the menu.  First the horizontal and vertical mouse position is
                   checked and unless it lies within the selected rectangle and not within 4 pixels of an edge the pickup fails and the function returns.
                   Otherwise flag SelectPickedUp is set to true (to allow it to move during MouseMove and remain in place at MouseUp) and the mouse position
                   is saved in SelectBitmapMouseLocX & Y for use later in MouseMove & MouseUp.
                   [Repeated from MouseMove] - The same actions apply on MouseMove whether Copy or Cut selected from the menu.  The X & Y mouse positions are checked and set to
                   stay within the display area.  Then the current selection H & V positions are stored in NewSelectBitmapHLoc & VLoc.
                   These change continually while the mouse and the selection are moving, they are only read on MouseUp to retain the position that it then
                   occupies.  Clearand... is called finally to clear earlier selection displays.
                   [New] - The only action here is to transfer the values of NewSelectBitmapHLoc & VLoc to SelectBitmapHLoc & VLoc so that the selection
                   stays in the same position (Clearand... checks whether the mouse is moving (both mbLeftDown & SelectPickedUp true) or stopped (either
                   mbLeftDown or SelectPickedUp false) and uses NewSelectBitmapHLoc & VLoc or SelectBitmapHLoc & SelectBitmapVLoc respectively.
*/
        {
            TrainController->LogEvent("MouseUp + Copy or CutMoving + mbLeftDown + SelectPickedUp");
            SelectBitmapHLoc = NewSelectBitmapHLoc;
            SelectBitmapVLoc = NewSelectBitmapVLoc;
        }

        mbLeftDown = false;
        Track->CalcHLocMinEtc(11);
        Utilities->CallLogPop(72);
    }
    catch(const Exception &e)
    {
        ErrorLog(23, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::MasterClockTimer(TObject *Sender)
{
    try
    {
        // don't call LogEvent here as would occur too often
        // have to allow in zoomout mode
        if(ErrorLogCalledFlag)
            return; // don't continue after an error

        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",MasterClockTimer");
        // put counter outside Clock2 as that may be missed
        LCResetCounter++;
// this checks LCs every 20 clock ticks (1 sec) & raises barriers if no route & no train present, to avoid delays due to too frequent calls
        if(LCResetCounter > 19)
            LCResetCounter = 0;
        WarningFlashCount++;
        if(WarningFlashCount > 4)
            WarningFlashCount = 0;
        if(WarningFlashCount == 0)
        {
            WarningFlash = !WarningFlash;
        }

        if(Utilities->CallLog.size() > 50) // use CTRL ALT 2 to see CallLogSize as program operates
        {
            throw Exception("Warning - Utilities->CallLog contains more than 50 items"); // check before clock stopped
        }

        if(!TrainController->StopTTClockFlag && (Level2OperMode == Operating))
        // stopped during 'Paused', when modal windows appear - Popup menu & ShowMessage, and at other times
        {
            // RestartTime is TTClockTime when operation pauses (timetable start time initially),
            // BaseTime is CurrentDateTime() when operation restarts

// clock speed multiplier
            double RealTimeDouble = double(TDateTime::CurrentDateTime() - TrainController->BaseTime);
            TrainController->TTClockTime = TDateTime(TTClockSpeed * RealTimeDouble) + TrainController->RestartTime;
// TrainController->TTClockTime = TDateTime::CurrentDateTime() - TrainController->BaseTime + TrainController->RestartTime;
        }

        TotalTicks++;
        if(Utilities->Clock2Stopped)
        {
            MissedTicks++;
            Utilities->CallLogPop(774);
            return;
        }
        Utilities->Clock2Stopped = true; //don't allow overlapping calls
        ClockTimer2(0);
        Utilities->Clock2Stopped = false;
        Utilities->CallLogPop(73);
    }
    catch(const Exception &e)
    {
        ErrorLog(24, e.Message);
    }
}

// ---------------------------------------------------------------------------

void TInterface::ClockTimer2(int Caller)
{
// called every 50mSec
    try
    {
        // have to allow in zoomout mode
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",ClockTimer2");

        // dropped at 2.0.0 because RestoreFocusPanel->SetFocus(); hides the help screen
        // If a button holds focus then all that is needed is to click the screen and the arrow keys work correctly

/* Dropped when new .chm help file introduced at v2.0.0 - this hid it after ~20ms.  Replaced by a new section in
           MainScreenMouseDown where focus restored to screen when click anywhere on screen, allowing navigation keys to
           move screen when clicked if focus had been captured by another panel when these keys just cycle through the panel buttons

           bool FocusRestoreAllowedFlag = true; //added at v1.3.0

           if(TextBox->Focused() || DistanceBox->Focused() || SpeedLimitBox->Focused() || LocationNameTextBox->Focused() || MileEdit->Focused() || ChainEdit->Focused() || YardEdit->Focused() ||
                SpeedEditBox2->Focused() || LocationNameComboBox->Focused() || AddSubMinsBox->Focused() || SpeedEditBox->Focused() || PowerEditBox->Focused() || OneEntryTimetableMemo->Focused() ||
                AddPrefDirButton->Focused()) //Added at v1.3.0.  If any of these has focus then they keep it until they release it.  AddPrefDirButton is included as it should keep focus
            FocusRestoreAllowedFlag = false; //when it has it - eases the setting of PrefDirs, also this button becomes disabled after use so focus returns to Interface naturally

           if(!Focused() && FocusRestoreAllowedFlag && (GetAsyncKeyState(VK_LBUTTON) >= 0) && (GetAsyncKeyState(VK_RBUTTON) >= 0)) //condition added at v1.3.0 to ensure focus returned to
           //Interface (so arrow keys work to move screen) & not left at any of the buttons or other Windows controls
           //include the Windows API functions to test that the mouse buttons are not down (strictly only need left but user may have mapped the left onto the right so test both) - if not
           //tested then don't always respond to button clicks on navigation and other buttons because the focus can be grabbed back from the button by RestoreFocusPanel before the button
           //can respond (takes about 200mSec from click to response) a delay is also included to doubly avoid the button losing focus as above
            {
            ClockTimer2Count++; //doesn't matter what value it starts at on first use, it will soon revert to 0
            if(ClockTimer2Count > 10) ClockTimer2Count = 0; //half second delay
            if(ClockTimer2Count == 0)
                {
                RestoreFocusPanel->Visible = true;
                RestoreFocusPanel->Enabled = true;
                RestoreFocusPanel->BringToFront();
                //RestoreFocusPanel->SetFocus();    //to remove focus from anything else
                RestoreFocusPanel->Enabled = false; //to remove focus from RestoreFocusPanel & return it to Interface
                RestoreFocusPanel->Visible = false;
                }
            }
           else ClockTimer2Count = 0; //reset to 0 so ensure full delay occurs before RestoreFocusPanel grabs focus from anything else
*/

        CallLogTickerLabel->Caption = Utilities->CallLog.size(); // diagnostic test function to ensure all CallLogs are popped - visibility
        // toggled by 'Ctrl Alt 2' when Interface form has focus

        // set current time
        TDateTime Now = TrainController->TTClockTime;

        TrainController->OpTimeToActUpdateCounter++;
///<new v2.2.0, controls 2 second updating for OpTimeToActPanel
        if(TrainController->OpTimeToActUpdateCounter >= 40)
            TrainController->OpTimeToActUpdateCounter = 0;

        if(OperatorActionPanel->Visible)
            TrainController->OpActionPanelHintDelayCounter++;
///<new v2.2.0, delay on start operation
        if(TrainController->OpActionPanelHintDelayCounter >= 80)
            TrainController->OpActionPanelHintDelayCounter = 80; // new at v2.2.0

        TrainController->RandomFailureCounter++; // new at v2.4.0 counts up for 53 seconds then resets
        if(TrainController->RandomFailureCounter >= 1060)
        {
            TrainController->RandomFailureCounter = 0;
        }

// Update Displayed Clock - resets to 0 at 96hours
        ClockLabel->Caption = Utilities->Format96HHMMSS(TrainController->TTClockTime);

// Below added at v2.1.0 to ensure WholeRailwayMoving flag reset when not moving (when rh mouse button up) as sometimes misses
// MouseUp events, probably due to a clash between a moving event and a mouse up event. Note that checks that both mouse buttons are up because
// function only checks the physical buttons, not the logical buttons.  Most sig bit of return value set form key down.
        if(WholeRailwayMoving && (GetAsyncKeyState(VK_LBUTTON) >= 0) && (GetAsyncKeyState(VK_RBUTTON) >= 0))
        {
            WholeRailwayMoving = false;
            Screen->Cursor = TCursor(-2); // Arrow
        }

// save session if required
        if(SaveSessionFlag)
        {
            SaveSession(0);
            SaveSessionFlag = false;
        }
// load session if required
        if(LoadSessionFlag)
        {
            if(ClearEverything(3))
            {
                LoadSession(0);
            }
            LoadSessionFlag = false;
        }

// check if any LCs need barriers raising

        if((Level1Mode == OperMode) && ((Level2OperMode == Operating) || (Level2OperMode == PreStart)))
        {
            if((LCResetCounter == 0) && !TrainController->StopTTClockFlag)
            {
                for(int x = Track->BarriersDownVector.size() - 1; x >= 0; x--)
                {
                    bool TrainPresent = false;
                    if(Track->AnyLinkedLevelCrossingElementsWithRoutesOrTrains(0, Track->BarriersDownVector.at(x).HLoc, Track->BarriersDownVector.at(x).VLoc,
                        TrainPresent))
                    {
                        if(TrainPresent)
                        {
                            Track->BarriersDownVector.at(x).TrainPassed = true;
                        }
                    }
                    else
                    {
                        Track->LCChangeFlag = true;
                        TTrack::TActiveLevelCrossing CLC = Track->BarriersDownVector.at(x);
                        // check if have exceeded the allowance (3 minutes for a train having passed or 0 for not) and add it to the overall excess time
                        TDateTime TempExcessLCDownTime;
                        if(Track->BarriersDownVector.at(x).TrainPassed)
                            TempExcessLCDownTime = TrainController->TTClockTime - CLC.StartTime - TDateTime(180.0 / 86400);
                        else
                            TempExcessLCDownTime = TrainController->TTClockTime - CLC.StartTime;
                        if(TempExcessLCDownTime > TDateTime(0))
                            TrainController->ExcessLCDownMins += (double(TempExcessLCDownTime) * 1440);

                        CLC.StartTime = TrainController->TTClockTime; // reset these 3 members
                        CLC.ChangeDuration = Track->LevelCrossingBarrierUpFlashDuration;
                        CLC.BarrierState = TTrack::Raising;
                        Track->SetLinkedLevelCrossingBarrierAttributes(0, CLC.HLoc, CLC.VLoc, 2); // set attr to 2 for changing state
                        Track->ChangingLCVector.push_back(CLC);
                        Track->BarriersDownVector.erase(Track->BarriersDownVector.begin() + x);
                    }
                }
            }
        }
// clear LCChangeFlag if no LCs changing
        if(Track->ChangingLCVector.empty())
        {
            Track->LCChangeFlag = false;
        }

// remove any single route elements if operating, but only if not constructing a route, else if extending the single route
// element it may be removed prior to conversion & cause an error

// note that if a train enters at a continuation and a signal is next but one to the continuation then the route element at that
// signal won't be removed because the train's LagElement is still -1 and trains only remove route elements when LagElement is > -1.
// This also means that a preferred route can't be cancelled as it's under a train, but it's probably not worth adding a patch just for
// this, it shouldn't interfere with operation.
        if((Level1Mode == OperMode) && (Level2OperMode == Operating) && !Track->RouteFlashFlag)
        {
            bool ElementRemovedFlag = false; // introduced at v0.6 to avoid calling Clearand.... multiple times
            for(unsigned int x = 0; x < AllRoutes->AllRoutesSize(); x++)
            {
                if(AllRoutes->GetFixedRouteAt(187, x).PrefDirSize() == 1)
                {
                    // only allow route element to be removed if not selected for a route start otherwise StartSelectionRouteID will be
                    // set & will fail at convert
                    if(AllRoutes->GetFixedRouteAt(194, x).RouteID != ConstructRoute->StartSelectionRouteID.GetInt())
                    {
                        TPrefDirElement PDE = AllRoutes->GetFixedRouteAt(188, x).GetFixedPrefDirElementAt(198, 0);
                        AllRoutes->RemoveRouteElement(20, PDE.HLoc, PDE.VLoc, PDE.GetELink());
                        ElementRemovedFlag = true;
                        TrainController->LogEvent("SingleRouteElementRemoved, H = " + AnsiString(PDE.HLoc) + ", V = " + AnsiString(PDE.VLoc));
                    }
                }
            }
            if(!Display->ZoomOutFlag && ElementRemovedFlag)
                ClearandRebuildRailway(66); // if zoomed out ignore, will display correctly when zoom in
            // if leave the Zoomout condition out then the zoom out will spontaneously cancel and the track won't display because
            // PlotOutput returns if zoomed out, and the zoom out flag isn't reset until the end of Clearand.....
            // this was moved outside the for.. next.. loop in v0.6 as it could be called multiple times and slowed down operation (noticeable with a fast clock)
        }
// stop clock if hover over a warning
        bool WH1 = (Mouse->CursorPos.x >= ClientOrigin.x + OutputLog1->Left) && (Mouse->CursorPos.x < (ClientOrigin.x + OutputLog1->Width + OutputLog1->Left))
            && (Mouse->CursorPos.y >= ClientOrigin.y + OutputLog1->Top) && (Mouse->CursorPos.y < (ClientOrigin.y + OutputLog1->Height + OutputLog1->Top))
            && OutputLog1->Caption != "";
        bool WH2 = (Mouse->CursorPos.x >= ClientOrigin.x + OutputLog2->Left) && (Mouse->CursorPos.x < (ClientOrigin.x + OutputLog2->Width + OutputLog2->Left))
            && (Mouse->CursorPos.y >= ClientOrigin.y + OutputLog2->Top) && (Mouse->CursorPos.y < (ClientOrigin.y + OutputLog2->Height + OutputLog2->Top))
            && OutputLog2->Caption != "";
        bool WH3 = (Mouse->CursorPos.x >= ClientOrigin.x + OutputLog3->Left) && (Mouse->CursorPos.x < (ClientOrigin.x + OutputLog3->Width + OutputLog3->Left))
            && (Mouse->CursorPos.y >= ClientOrigin.y + OutputLog3->Top) && (Mouse->CursorPos.y < (ClientOrigin.y + OutputLog3->Height + OutputLog3->Top))
            && OutputLog3->Caption != "";
        bool WH4 = (Mouse->CursorPos.x >= ClientOrigin.x + OutputLog4->Left) && (Mouse->CursorPos.x < (ClientOrigin.x + OutputLog4->Width + OutputLog4->Left))
            && (Mouse->CursorPos.y >= ClientOrigin.y + OutputLog4->Top) && (Mouse->CursorPos.y < (ClientOrigin.y + OutputLog4->Height + OutputLog4->Top))
            && OutputLog4->Caption != "";
        bool WH5 = (Mouse->CursorPos.x >= ClientOrigin.x + OutputLog5->Left) && (Mouse->CursorPos.x < (ClientOrigin.x + OutputLog5->Width + OutputLog5->Left))
            && (Mouse->CursorPos.y >= ClientOrigin.y + OutputLog5->Top) && (Mouse->CursorPos.y < (ClientOrigin.y + OutputLog5->Height + OutputLog5->Top))
            && OutputLog5->Caption != "";
        bool WH6 = (Mouse->CursorPos.x >= ClientOrigin.x + OutputLog6->Left) && (Mouse->CursorPos.x < (ClientOrigin.x + OutputLog6->Width + OutputLog6->Left))
            && (Mouse->CursorPos.y >= ClientOrigin.y + OutputLog6->Top) && (Mouse->CursorPos.y < (ClientOrigin.y + OutputLog6->Height + OutputLog6->Top))
            && OutputLog6->Caption != "";
        bool WH7 = (Mouse->CursorPos.x >= ClientOrigin.x + OutputLog7->Left) && (Mouse->CursorPos.x < (ClientOrigin.x + OutputLog7->Width + OutputLog7->Left))
            && (Mouse->CursorPos.y >= ClientOrigin.y + OutputLog7->Top) && (Mouse->CursorPos.y < (ClientOrigin.y + OutputLog7->Height + OutputLog7->Top))
            && OutputLog7->Caption != "";
        bool WH8 = (Mouse->CursorPos.x >= ClientOrigin.x + OutputLog8->Left) && (Mouse->CursorPos.x < (ClientOrigin.x + OutputLog8->Width + OutputLog8->Left))
            && (Mouse->CursorPos.y >= ClientOrigin.y + OutputLog8->Top) && (Mouse->CursorPos.y < (ClientOrigin.y + OutputLog8->Height + OutputLog8->Top))
            && OutputLog8->Caption != "";
        bool WH9 = (Mouse->CursorPos.x >= ClientOrigin.x + OutputLog9->Left) && (Mouse->CursorPos.x < (ClientOrigin.x + OutputLog9->Width + OutputLog9->Left))
            && (Mouse->CursorPos.y >= ClientOrigin.y + OutputLog9->Top) && (Mouse->CursorPos.y < (ClientOrigin.y + OutputLog9->Height + OutputLog9->Top))
            && OutputLog9->Caption != "";
        bool WH10 = (Mouse->CursorPos.x >= ClientOrigin.x + OutputLog10->Left) &&
            (Mouse->CursorPos.x < (ClientOrigin.x + OutputLog10->Width + OutputLog10->Left)) && (Mouse->CursorPos.y >= ClientOrigin.y + OutputLog10->Top) &&
            (Mouse->CursorPos.y < (ClientOrigin.y + OutputLog10->Height + OutputLog10->Top)) && OutputLog10->Caption != "";

        if(WH1 || WH2 || WH3 || WH4 || WH5 || WH6 || WH7 || WH8 || WH9 || WH10)
        {
            if(!WarningHover)
            {
                TrainController->StopTTClockFlag = true; // so TTClock stopped during MasterClockTimer function
                TrainController->RestartTime = TrainController->TTClockTime;
                WarningHover = true;
            }
        }
        else if(WarningHover)
        {
            WarningHover = false;
            TrainController->BaseTime = TDateTime::CurrentDateTime();
            TrainController->StopTTClockFlag = false;
        }

// development panel - visibility toggled by 'Ctrl Alt 3' when Interface form has focus
        if(DevelopmentPanel->Visible)
        {
            int Position;
            TTrackElement TrackElement;
            AnsiString Type[15] =
            {"Simple", "Crossover", "Points", "Buffers", "Bridge", "SignalPost", "Continuation", "Platform", "GapJump", "FootCrossing", "Unused", "Concourse",
                "Parapet", "NamedNonStationLocation", "Erase"};

            int ScreenX = Mouse->CursorPos.x - MainScreen->ClientOrigin.x;
            int ScreenY = Mouse->CursorPos.y - MainScreen->ClientOrigin.y;
            int HLoc, VLoc;
            AnsiString MouseStr = "Posx: " + AnsiString(ScreenX) + "; Posy: " + AnsiString(ScreenY);
            DevelopmentPanel->Caption = CurDir + " " + MouseStr;
            Track->GetTrackLocsFromScreenPos(7, HLoc, VLoc, ScreenX, ScreenY);
            if(Track->FindNonPlatformMatch(1, HLoc, VLoc, Position, TrackElement))
            {
                DevelopmentPanel->Caption = MouseStr + "; TVPos: " + AnsiString(Position) + "; H: " + AnsiString(HLoc) + "; V: " + AnsiString(VLoc) +
                    "; SpTg: " + AnsiString(TrackElement.SpeedTag) + "; Type: " + Type[TrackElement.TrackType] + "; Att: " + AnsiString(TrackElement.Attribute)
                    + "; TrID: " + AnsiString(TrackElement.TrainIDOnElement) + "; TrID01: " + AnsiString(TrackElement.TrainIDOnBridgeTrackPos01) +
                    "; TrID23: " + AnsiString(TrackElement.TrainIDOnBridgeTrackPos23) + "; " + TrackElement.LocationName + "; " +
                    TrackElement.ActiveTrackElementName;
// + "; OAHintCtr: " + TrainController->OpActionPanelHintDelayCounter;
            }
        }

// highlight timetable entry if in tt mode (have to call this regularly so will scroll with the listbox)
        if(Level1Mode == TimetableMode)
        {
            if(!TimetableEditVector.empty() && (TTCurrentEntryPtr > 0))
            {
                HighlightOneEntryInAllEntriesTTListBox(1, TTCurrentEntryPtr - TimetableEditVector.begin());
            }
            else
            {
                HighlightOneEntryInAllEntriesTTListBox(2, 0);
            }
        }

// set cursor
        if(Track->PointFlashFlag || Track->RouteFlashFlag)
        {
            if(!TempCursorSet)
            {
                TempCursor = Screen->Cursor;
                TempCursorSet = true;
            }
            Screen->Cursor = TCursor(-11); // Hourglass
        }
        else
        {
            if(TempCursorSet)
            {
                Screen->Cursor = TempCursor;
                TempCursorSet = false;
            }
        }

        if(Level2OperMode == Operating)
        {
            TrainController->Operate(0); // ensure this called AFTER the single element route removal to ensure any single elements removed
                                         // prior to CallingOnAllowed being called (in UpdateTrain) as that sets a route from the stop signal
            if((TrainController->OpTimeToActUpdateCounter == 0) && (OperatorActionPanel->Visible))
            {
                UpdateOperatorActionPanel(0); // new at v2.2.0 to update panel when train OpTimeToAct updated
            }
            TrainController->SignallerTrainRemovedOnAutoSigsRoute = false; // added at v1.3.0 to ensure doesn't persist beyone one call
        }

// plot trains in ZoomOut mode & flash trains where attention needed alternately on & off at each call
// by examining Flash
        if((Level1Mode == OperMode) && (Display->ZoomOutFlag))
        {
            TrainController->PlotAllTrainsInZoomOutMode(0, WarningFlash);
        }

// Deal with any flashing graphics
        if((WarningFlashCount == 0) && !TrainController->StopTTClockFlag)
        {
            FlashingGraphics(0, Now); // only call when WarningFlash changes
            if(Level1Mode == OperMode)
            {
                if(WarningFlash)
                {
                    if(TrainController->CrashWarning)
                    {
                        CrashImage->Visible = true;
                    }
                    if(TrainController->DerailWarning)
                    {
                        DerailImage->Visible = true;
                    }
                    if(TrainController->SPADWarning)
                    {
                        SPADImage->Visible = true;
                    }
                    if(TrainController->TrainFailedWarning)
                    {
                        TrainFailedImage->Visible = true;
                    }
                    if(TrainController->CallOnWarning)
                    {
                        CallOnImage->Visible = true;
                    }
                    if(TrainController->SignalStopWarning)
                    {
                        SignalStopImage->Visible = true;
                    }
                    if(TrainController->BufferAttentionWarning)
                    {
                        BufferAttentionImage->Visible = true;
                    }
                }
                else
                {
                    CrashImage->Visible = false;
                    DerailImage->Visible = false;
                    SPADImage->Visible = false;
                    TrainFailedImage->Visible = false;
                    CallOnImage->Visible = false;
                    SignalStopImage->Visible = false;
                    BufferAttentionImage->Visible = false;
                }
            }
            else
            {
                CrashImage->Visible = false;
                DerailImage->Visible = false;
                SPADImage->Visible = false;
                TrainFailedImage->Visible = false;
                CallOnImage->Visible = false;
                SignalStopImage->Visible = false;
                BufferAttentionImage->Visible = false;
            }
        } // if(WarningFlashCount == 0)
          // set buttons etc as appropriate
        SetSaveMenuAndButtons(0);
        // if forced route cancellation flag set redisplay to clear the cancelled route
        if(AllRoutes->RebuildRailwayFlag && !Display->ZoomOutFlag)
        {
            ClearandRebuildRailway(16);
            AllRoutes->RebuildRailwayFlag = false;
        }
        // deal with approach locking
        ApproachLocking(0, TrainController->TTClockTime);
        // deal with ContinuationAutoSigList
        ContinuationAutoSignals(0, TrainController->TTClockTime);
        // FloatingLabel function
        if((TrackInfoOnOffMenuItem->Caption == "Hide") || (TrainStatusInfoOnOffMenuItem->Caption == "Hide Status") ||
            (TrainTTInfoOnOffMenuItem->Caption == "Hide Timetable"))
        {
            TrackTrainFloat(0);
        }
        else
        {
            FloatingPanel->Visible = false;
        }
        // PerformanceLog check function
/*
        if(IsPerformancePanelObscuringFloatingLabel(0) && (ShowPerformancePanel))
        {
            PerformancePanel->Visible = false;
        }
        else
        {
*/
        if(ShowPerformancePanel)
        {
            PerformancePanel->Visible = true;
        }
        else
        {
            PerformancePanel->Visible = false;
        }

        if(ShowOperatorActionPanel)
        {
            OperatorActionPanel->Visible = true;
        }
        else
        {
            OperatorActionPanel->Visible = false;
        }

// }

        // check if a moving train is present on a route-under-construction start element & cancel it if so
        if(RouteMode == RouteContinuing)
        {
            bool FoundFlag;
            int RouteStartVecPos;
            if(AutoSigsFlag)
                RouteStartVecPos = Track->GetVectorPositionFromTrackMap(7, (AutoRouteStartMarker->GetHPos()) / 16, (AutoRouteStartMarker->GetVPos()) / 16,
                FoundFlag);
            else if(ConsecSignalsRoute)
                RouteStartVecPos = Track->GetVectorPositionFromTrackMap(8, (SigRouteStartMarker->GetHPos()) / 16, (SigRouteStartMarker->GetVPos()) / 16,
                FoundFlag);
            else
                RouteStartVecPos = Track->GetVectorPositionFromTrackMap(9, (NonSigRouteStartMarker->GetHPos()) / 16, (NonSigRouteStartMarker->GetVPos()) / 16,
                FoundFlag);
            if(FoundFlag && (RouteStartVecPos > -1))
            {
                TTrackElement TrackElement = Track->TrackElementAt(485, RouteStartVecPos);
                if(TrackElement.TrainIDOnElement > -1)
                {
                    if(!(TrainController->TrainVectorAtIdent(2, TrackElement.TrainIDOnElement).Stopped()))
                    {
                        RevertToOriginalRouteSelector(10);
                        // replot train as above erases the front element of the train
                        TrainController->TrainVectorAtIdent(3, TrackElement.TrainIDOnElement).PlotTrain(1, Display);
                    }
                }
            }
        }
        Utilities->CallLogPop(81);
    }
    catch(const Exception &e)
    {
        ErrorLog(25, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::CallingOnButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("CallingOnButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",CallingOnButtonClick");
        if(CallingOnButton->Down)
        {
            // CallingOnButton->Down = true;
            InfoPanel->Visible = true;
            InfoPanel->Caption = "CALLING ON:  Select signal for call on";
        }
        else
        {
            // CallingOnButton->Down = false;
            RevertToOriginalRouteSelector(11);
        }
        AutoRouteStartMarker->PlotOriginal(29, Display); // if overlay not plotted will ignore
        SigRouteStartMarker->PlotOriginal(30, Display); // if overlay not plotted will ignore
        NonSigRouteStartMarker->PlotOriginal(31, Display); // if overlay not plotted will ignore
        CallingOnButton->Enabled = false;
// added at v1.3.0 to ensure doesn't retain focus - will be re-enabled during ClockTimer2 (in SetSaveMenuAndButtons) if required
        Utilities->CallLogPop(82);
    }
    catch(const Exception &e)
    {
        ErrorLog(26, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::ScreenLeftButtonClick(TObject *Sender)
{
    try
    {
        // have to allow in zoomout mode
        TrainController->LogEvent("ScreenLeftButtonClick" + AnsiString((short)ShiftKey) + AnsiString((short)CtrlKey));
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",ScreenLeftButtonClick" + AnsiString((short)ShiftKey) + AnsiString((short)CtrlKey));
        Screen->Cursor = TCursor(-11); // Hourglass;
        ScreenLeftButton->Enabled = false; // to make multiple key presses less likely (not entirely successful)
        if(!Display->ZoomOutFlag)
        {
            if(CtrlKey)
            {
                Display->DisplayOffsetH -= 2;
            }
            else if(ShiftKey)
            {
                Display->DisplayOffsetH -= Utilities->ScreenElementWidth;
            }
            else
            {
                Display->DisplayOffsetH -= Utilities->ScreenElementWidth / 2;
            }
            ClearandRebuildRailway(22);
            if((Level2TrackMode == TrackSelecting) || (Level2PrefDirMode == PrefDirSelecting))
            {
                Display->PlotDashedRect(10, SelectRect);
            }
        }
        else
        {
            if(CtrlKey)
            {
                Display->DisplayZoomOutOffsetH -= 2;
            }
            else if(ShiftKey)
            {
                Display->DisplayZoomOutOffsetH -= (4 * Utilities->ScreenElementWidth);
            }
            else
            {
                Display->DisplayZoomOutOffsetH -= Utilities->ScreenElementWidth;
            }
            Display->ClearDisplay(0);
            Track->PlotSmallRailway(2, Display);
            if(Level2TrackMode == GapSetting)
                Track->PlotSmallRedGap(0);
        }
        ScreenLeftButton->Enabled = true;
        Screen->Cursor = TCursor(-2); // Arrow
        Utilities->CallLogPop(83);
    }
    catch(const Exception &e)
    {
        ErrorLog(27, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::ScreenRightButtonClick(TObject *Sender)
{
    try
    {
        // have to allow in zoomout mode
        TrainController->LogEvent("ScreenRightButtonClick" + AnsiString((short)ShiftKey) + AnsiString((short)CtrlKey));
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",ScreenRightButtonClick" + AnsiString((short)ShiftKey) + AnsiString((short)CtrlKey));
        Screen->Cursor = TCursor(-11); // Hourglass;
        ScreenRightButton->Enabled = false; // to make multiple key presses less likely (not entirely successful)
        if(!Display->ZoomOutFlag)
        {
            if(CtrlKey)
            {
                Display->DisplayOffsetH += 2;
            }
            else if(ShiftKey)
            {
                Display->DisplayOffsetH += Utilities->ScreenElementWidth;
            }
            else
            {
                Display->DisplayOffsetH += Utilities->ScreenElementWidth / 2;
            }
            ClearandRebuildRailway(23);
            if((Level2TrackMode == TrackSelecting) || (Level2PrefDirMode == PrefDirSelecting))
            {
                Display->PlotDashedRect(4, SelectRect);
            }
        }
        else
        {
            if(CtrlKey)
            {
                Display->DisplayZoomOutOffsetH += 2;
            }
            else if(ShiftKey)
            {
                Display->DisplayZoomOutOffsetH += (4 * Utilities->ScreenElementWidth);
            }
            else
            {
                Display->DisplayZoomOutOffsetH += Utilities->ScreenElementWidth;
            }
            Display->ClearDisplay(1);
            Track->PlotSmallRailway(3, Display);
            if(Level2TrackMode == GapSetting)
                Track->PlotSmallRedGap(1);
        }
        ScreenRightButton->Enabled = true;
        Screen->Cursor = TCursor(-2); // Arrow
        Utilities->CallLogPop(84);
    }
    catch(const Exception &e)
    {
        ErrorLog(28, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::ScreenDownButtonClick(TObject *Sender)
{
    try
    {
        // have to allow in zoomout mode
        TrainController->LogEvent("ScreenDownButtonClick" + AnsiString((short)ShiftKey) + AnsiString((short)CtrlKey));
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",ScreenDownButtonClick" + AnsiString((short)ShiftKey) + AnsiString((short)CtrlKey));
        Screen->Cursor = TCursor(-11); // Hourglass;
        ScreenDownButton->Enabled = false; // to make multiple key presses less likely (not entirely successful)
        // BUT - it does prevent it from retaining focus - so can use the cursor keys to scroll the display without being captured by the buttons
        if(!Display->ZoomOutFlag)
        {
            if(CtrlKey)
            {
                Display->DisplayOffsetV += 2;
            }
            else if(ShiftKey)
            {
                Display->DisplayOffsetV += Utilities->ScreenElementHeight;
            }
            else
            {
                Display->DisplayOffsetV += Utilities->ScreenElementHeight / 2;
            }
            ClearandRebuildRailway(24);
            if((Level2TrackMode == TrackSelecting) || (Level2PrefDirMode == PrefDirSelecting))
            {
                Display->PlotDashedRect(5, SelectRect);
            }
        }
        else
        {
            if(CtrlKey)
            {
                Display->DisplayZoomOutOffsetV += 2;
            }
            else if(ShiftKey)
            {
                Display->DisplayZoomOutOffsetV += (4 * Utilities->ScreenElementHeight);
            }
            else
            {
                Display->DisplayZoomOutOffsetV += Utilities->ScreenElementHeight;
            }
            Display->ClearDisplay(2);
            Track->PlotSmallRailway(4, Display);
            if(Level2TrackMode == GapSetting)
                Track->PlotSmallRedGap(2);
        }
        ScreenDownButton->Enabled = true;
        Screen->Cursor = TCursor(-2); // Arrow
        Utilities->CallLogPop(85);
    }
    catch(const Exception &e)
    {
        ErrorLog(29, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::ScreenUpButtonClick(TObject *Sender)
{
    try
    {
        // have to allow in zoomout mode
        TrainController->LogEvent("ScreenUpButtonClick" + AnsiString((short)ShiftKey) + AnsiString((short)CtrlKey));
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",ScreenUpButtonClick" + AnsiString((short)ShiftKey) + AnsiString((short)CtrlKey));
        Screen->Cursor = TCursor(-11); // Hourglass;
        ScreenUpButton->Enabled = false; // to make multiple key presses less likely (not entirely successful)
        if(!Display->ZoomOutFlag)
        {
            if(CtrlKey)
            {
                Display->DisplayOffsetV -= 2;
            }
            else if(ShiftKey)
            {
                Display->DisplayOffsetV -= Utilities->ScreenElementHeight;
            }
            else
            {
                Display->DisplayOffsetV -= Utilities->ScreenElementHeight / 2;
            }
            ClearandRebuildRailway(25);
            if((Level2TrackMode == TrackSelecting) || (Level2PrefDirMode == PrefDirSelecting))
            {
                Display->PlotDashedRect(6, SelectRect);
            }
        }
        else
        {
            if(CtrlKey)
            {
                Display->DisplayZoomOutOffsetV -= 2;
            }
            else if(ShiftKey)
            {
                Display->DisplayZoomOutOffsetV -= (4 * Utilities->ScreenElementHeight);
            }
            else
            {
                Display->DisplayZoomOutOffsetV -= Utilities->ScreenElementHeight;
            }
            Display->ClearDisplay(3);
            Track->PlotSmallRailway(5, Display);
            if(Level2TrackMode == GapSetting)
                Track->PlotSmallRedGap(3);
        }
        ScreenUpButton->Enabled = true;
        Screen->Cursor = TCursor(-2); // Arrow
        Utilities->CallLogPop(86);
    }
    catch(const Exception &e)
    {
        ErrorLog(30, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::ZoomButtonClick(TObject *Sender)
{
    try
    {
        // have to allow in zoomout mode
        TrainController->LogEvent("ZoomButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",ZoomButtonClick");
        Screen->Cursor = TCursor(-11); // Hourglass;
        ZoomButton->Enabled = false; // this takes focus away so the arrow keys can move the display (v2.0.0)
        if(Display->ZoomOutFlag) // i.e resume zoomed in view
        {
            TrainController->LogEvent("ZoomButtonClick + ZoomOutFlag");
// TLevel2OperMode TempLevel2OperMode = Level2OperMode;
            if(Level1Mode == BaseMode)
            {
                InfoPanel->Visible = false; // reset infopanel in case not set later
                InfoPanel->Caption = "";
                SetLevel1Mode(18);
            }
            else if(Level1Mode == TrackMode)
            {
                InfoPanel->Visible = false; // reset infopanel in case not set later
                InfoPanel->Caption = "";
                // set edit menu items
                SetInitialTrackModeEditMenu();
                SetLevel2TrackMode(33); // revert to earlier track mode from zoom
            }
            else if(Level1Mode == PrefDirMode)
            {
                if(Level2PrefDirMode != PrefDirContinuing)
                    SetLevel1Mode(19); // to redisplay infopanel caption "...select start..."
                else
                    SetLevel2PrefDirMode(4); // revert to PrefDirContinuing PrefDir mode
            }
// else if(Level1Mode == TrackMode) SetLevel1Mode();//just revert to basic track mode from zoom
// else if(Level1Mode == PrefDirMode) SetLevel1Mode();//just revert to basic PrefDir mode from zoom
            else if(Level1Mode == TimetableMode)
            {
                InfoPanel->Visible = false;
            }
            // Don't include OperMode or RestartSessionOperMode as they reset the performance file
            else if(Level2OperMode == Operating) // similar to SetLevel2OperMode but without resetting BaseTime
            {
                OperateButton->Enabled = true;
                OperateButton->Glyph->LoadFromResourceName(0, "PauseGraphic");
                ExitOperationButton->Enabled = true;
                SetRouteButtonsInfoCaptionAndRouteNotStarted(1);
            }
            else if(Level2OperMode == Paused) // similar to SetLevel2OperMode but without resetting RestartTime
            {
                OperateButton->Enabled = true;
                OperateButton->Glyph->LoadFromResourceName(0, "RunGraphic");
                ExitOperationButton->Enabled = true;
                TTClockAdjButton->Enabled = true;
                SetRouteButtonsInfoCaptionAndRouteNotStarted(5);
                DisableRouteButtons(1);
            }
            else if(Level2OperMode == PreStart)
            {
                OperateButton->Enabled = true;
                OperateButton->Glyph->LoadFromResourceName(0, "RunGraphic");
                ExitOperationButton->Enabled = true;
                TTClockAdjButton->Enabled = true;
                SetRouteButtonsInfoCaptionAndRouteNotStarted(9);
            }
            Display->ZoomOutFlag = false; // reset this after level modes called so gap flash stays set if set to begin with
            SetPausedOrZoomedInfoCaption(1);
            ClearandRebuildRailway(43); // need to call this after ZoomOutFlag reset to display track, even if Clearand... already called
                                        // earlier during level mode setting - because until ZoomOutFlag reset PlotOutput plots nothing
        }
        else // set zoomed out view
        {
            TrainController->LogEvent("ZoomButtonClick + != ZoomOutFlag");
            Display->ZoomOutFlag = true;
            SetPausedOrZoomedInfoCaption(2);
            FileMenu->Enabled = false;
            ModeMenu->Enabled = false;
            EditMenu->Enabled = false;
            TextBox->Visible = false;
            LocationNameTextBox->Visible = false;
            TTClockAdjButton->Enabled = false;
// DisablePanelsStoreMainMenuStates();//ensure Display->ZoomOutFlag set true before calling
            // start assuming normal view is at centre of ZoomOut & calc excesses at each side
            int OVOffH_NVCentre = Display->DisplayOffsetH - (1.5 * Utilities->ScreenElementWidth);
// start zoomout centre at DisplayOffsetH + 30 - zoomout width/2 = -(1.5 * 60)
            int LeftExcess = OVOffH_NVCentre - Track->GetHLocMin();
            int RightExcess = Track->GetHLocMax() - OVOffH_NVCentre - ((4 * Utilities->ScreenElementWidth) - 1);
            if((LeftExcess > 0) && (RightExcess > 0))
                Display->DisplayZoomOutOffsetH = OVOffH_NVCentre;
            else if((LeftExcess > 0) && (RightExcess <= 0))
                Display->DisplayZoomOutOffsetH = OVOffH_NVCentre + ((RightExcess) / (Utilities->ScreenElementWidth / 2)) *
                    (Utilities->ScreenElementWidth / 2); // normalise to nearest screen
            else if((LeftExcess <= 0) && (RightExcess > 0))
                Display->DisplayZoomOutOffsetH = OVOffH_NVCentre - ((LeftExcess) / (Utilities->ScreenElementWidth / 2)) * (Utilities->ScreenElementWidth / 2);
            else
                Display->DisplayZoomOutOffsetH = OVOffH_NVCentre; // no excess at either side, so display in centre

            int OVOffV_NVCentre = Display->DisplayOffsetV - (1.5 * Utilities->ScreenElementHeight);
            int TopExcess = OVOffV_NVCentre - Track->GetVLocMin();
            int BotExcess = Track->GetVLocMax() - OVOffV_NVCentre - ((4 * Utilities->ScreenElementHeight) - 1);
            if((TopExcess > 0) && (BotExcess > 0))
                Display->DisplayZoomOutOffsetV = OVOffV_NVCentre;
            else if((TopExcess > 0) && (BotExcess <= 0))
                Display->DisplayZoomOutOffsetV = OVOffV_NVCentre + ((BotExcess) / (Utilities->ScreenElementHeight / 2)) *
                    (Utilities->ScreenElementHeight / 2); // normalise to nearest half screen
            else if((TopExcess <= 0) && (BotExcess > 0))
                Display->DisplayZoomOutOffsetV = OVOffV_NVCentre - ((TopExcess) / (Utilities->ScreenElementHeight / 2)) * (Utilities->ScreenElementHeight / 2);
            else
                Display->DisplayZoomOutOffsetV = OVOffV_NVCentre; // no excess at either side, so display in centre

            Display->ClearDisplay(4);
            Track->PlotSmallRailway(6, Display);
            TrainController->PlotAllTrainsInZoomOutMode(1, WarningFlash);
            if(Level2TrackMode == GapSetting)
                Track->PlotSmallRedGap(4);
            ZoomButton->Glyph->LoadFromResourceName(0, "ZoomIn");
        }
        Screen->Cursor = TCursor(-2); // Arrow
        ZoomButton->Enabled = true; // restore, see above
        Utilities->CallLogPop(87);
    }
    catch(const Exception &e)
    {
        ErrorLog(31, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::HomeButtonClick(TObject *Sender)
{
    try
    {
        // have to allow in zoomout mode
        TrainController->LogEvent("HomeButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",HomeButtonClick");
        Screen->Cursor = TCursor(-11); // Hourglass;
        HomeButton->Enabled = false; // this takes focus away so the arrow keys can move the display (v2.0.0)
        if(!Display->ZoomOutFlag) // zoomed in mode
        {
            TrainController->LogEvent("HomeButtonClick + zoomed in mode");
            Display->ResetZoomInOffsets();
            ClearandRebuildRailway(27);
            if((Level2TrackMode == TrackSelecting) || (Level2PrefDirMode == PrefDirSelecting))
            {
                Display->PlotDashedRect(7, SelectRect);
            }
        }
        else
        {
            // zoomed out mode
            // start assuming normal view is at centre of ZoomOut & calc excesses at each side
            TrainController->LogEvent("HomeButtonClick + zoomed out mode");
            Display->ResetZoomOutOffsets();
            Display->ClearDisplay(9);
            Track->PlotSmallRailway(7, Display);
            if(Level2TrackMode == GapSetting)
                Track->PlotSmallRedGap(5);
        }
        Screen->Cursor = TCursor(-2); // Arrow
        HomeButton->Enabled = true; // restore, see above
        Utilities->CallLogPop(88);
    }
    catch(const Exception &e)
    {
        ErrorLog(32, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::NewHomeButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("NewHomeButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",NewHomeButtonClick");
        NewHomeButton->Enabled = false; // this takes focus away so the arrow keys can move the display (v2.0.0)
        if(!Display->ZoomOutFlag) // zoomed in mode
        {
            Display->DisplayOffsetHHome = Display->DisplayOffsetH;
            Display->DisplayOffsetVHome = Display->DisplayOffsetV;
            ResetChangedFileDataAndCaption(23, false); // false because no major changes made
        }
        else
        {
            Display->DisplayZoomOutOffsetHHome = Display->DisplayZoomOutOffsetH;
            Display->DisplayZoomOutOffsetVHome = Display->DisplayZoomOutOffsetV;
        }
        Utilities->CallLogPop(1188);
        NewHomeButton->Enabled = true; // restore, see above
    }
    catch(const Exception &e)
    {
        ErrorLog(174, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::EditMenuClick(TObject *Sender)
    // added at v2.1.0 to allow CTRL+X, CTRL+C & CTRL+V in edit menu (see case BaseMode for more information)
{
    try
    {
        CopyMenuItem->ShortCut = TextToShortCut("Ctrl+C");
        CutMenuItem->ShortCut = TextToShortCut("Ctrl+X");
        PasteMenuItem->ShortCut = TextToShortCut("Ctrl+V");
    }
    catch(const Exception &e)
    {
        ErrorLog(196, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::SelectMenuItemClick(TObject *Sender)
{
// draw a rectangle with the left mouse button, enclosing whole 16 x 16 squares
    try
    {
        TrainController->LogEvent("SelectMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",SelectMenuItemClick");
        if(Level1Mode == TrackMode)
        {
            SelectionValid = false;
            Level2TrackMode = TrackSelecting;
            SetLevel2TrackMode(34);
        }
        else if(Level1Mode == PrefDirMode)
        {
            Level2PrefDirMode = PrefDirSelecting;
            SetLevel2PrefDirMode(5);
        }
        Utilities->CallLogPop(1189);
    }
    catch(const Exception &e)
    {
        ErrorLog(145, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::ReselectMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("ReselectMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",ReselectMenuItemClick");
        if((SelectBitmap->Height == 0) || (SelectBitmap->Width == 0))
        {
            Utilities->CallLogPop(1424);
            return;
        }

        int TLHCH = SelectBitmapHLoc;
        int TLHCV = SelectBitmapVLoc;
        int BRHCH = TLHCH + (SelectBitmap->Width / 16);
        int BRHCV = TLHCV + (SelectBitmap->Height / 16);
        TRect NewSelectRect(TLHCH, TLHCV, BRHCH, BRHCV);
        SelectRect = NewSelectRect;
        Display->PlotDashedRect(8, SelectRect);
        // set bitmap to reselected area (may be different if flip or mirror had been selected earlier)
        TRect Dest(0, 0, SelectBitmap->Width, SelectBitmap->Height);
        TRect Source(((SelectRect.left - Display->DisplayOffsetH) * 16), ((SelectRect.top - Display->DisplayOffsetV) * 16),
            ((SelectRect.right - Display->DisplayOffsetH) * 16), ((SelectRect.bottom - Display->DisplayOffsetV) * 16));
        SelectBitmap->Canvas->CopyRect(Dest, MainScreen->Canvas, Source);

        SelectionValid = true;
        ReselectMenuItem->Enabled = false;
        CutMenuItem->Enabled = true;
        CopyMenuItem->Enabled = true;
        FlipMenuItem->Enabled = true;
        MirrorMenuItem->Enabled = true;
        RotRightMenuItem->Enabled = true;
        RotLeftMenuItem->Enabled = true;
        RotateMenuItem->Enabled = true;
        PasteMenuItem->Enabled = false;
        DeleteMenuItem->Enabled = true;
        if(Track->IsTrackFinished())
            SelectLengthsMenuItem->Enabled = true; // only permit if finished because reverts to DistanceStart
        else
            SelectLengthsMenuItem->Enabled = false; // and that can only be used if track linked
        SelectBiDirPrefDirsMenuItem->Visible = false;
        CancelSelectionMenuItem->Enabled = true;
        mbLeftDown = false;
        // Level1Mode = TrackMode;
        // SetLevel1Mode(68);
        Level2TrackMode = TrackSelecting;
        SetLevel2TrackMode(47);
        Utilities->CallLogPop(1425);
    }
    catch(const Exception &e)
    {
        ErrorLog(146, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::CutMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("CutMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",CutMenuItemClick");
        // Level1Mode = TrackMode;
        // SetLevel1Mode(69);
        Level2TrackMode = CutMoving;
        SetLevel2TrackMode(35);
        Utilities->CallLogPop(1190);
    }
    catch(const Exception &e)
    {
        ErrorLog(147, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::CopyMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("CopyMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",CopyMenuItemClick");
        // Level1Mode = TrackMode;
        // SetLevel1Mode(70);
        Level2TrackMode = CopyMoving;
        SetLevel2TrackMode(36);
        Utilities->CallLogPop(1191);
    }
    catch(const Exception &e)
    {
        ErrorLog(148, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::FlipMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("FlipMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",FlipMenuItemClick");
        // reset values in SelectVector
        int VerSum = SelectRect.top + SelectRect.bottom - 1;
        for(unsigned int x = 0; x < Track->SelectVectorSize(); x++)
        {
            // Note:  (changed again in v2.4.0 to keep attributes) need to change flip, mirror & 180deg functions as only change speedtag without changing anything else.
            // This didn't matter before new paste with attributes added at v2.2.0 as a new element was built from the speedtag,
            // but now if do a reselect then cut and paste with attributes the wrong graphic is pasted and all other attributes
            // are wrong. Need to rebuild a new TrackElement from the new speedtag and use that in the select vector.
            // Note that if use Flip, mirror etc then all attributes lost anyway so ok to build a basic element.
            int VLoc = VerSum - Track->SelectVectorAt(8, x).VLoc;
            int HLoc = Track->SelectVectorAt(7, x).HLoc;
            TTrackElement TE = Track->BuildBasicElementFromSpeedTag(0, Track->FlipArray[Track->SelectVectorAt(0, x).SpeedTag]);
            TE.VLoc = VLoc;
            TE.HLoc = HLoc;

            TE.ActiveTrackElementName = Track->SelectVectorAt(37, x).ActiveTrackElementName; // these new in v2.4.0 so keeps attributes
            TE.LocationName = Track->SelectVectorAt(38, x).LocationName;
            TE.Length01 = Track->SelectVectorAt(39, x).Length01;
            TE.Length23 = Track->SelectVectorAt(40, x).Length23;
            TE.SpeedLimit01 = Track->SelectVectorAt(41, x).SpeedLimit01;
            TE.SpeedLimit23 = Track->SelectVectorAt(42, x).SpeedLimit23;
            TE.SigAspect = Track->SelectVectorAt(43, x).SigAspect;
            Track->SelectVectorAt(26, x) = TE;
        }
        // reset values in SelectTextVector
        for(unsigned int x = 0; x < TextHandler->SelectTextVectorSize(0); x++)
        {
            TTextItem *TextItem = TextHandler->SelectTextPtrAt(0, x);
            // also subtract font height, brings position approximately right
            TextItem->VPos = ((VerSum * 16) + 15) - TextItem->VPos - abs(TextItem->Font->Height);
        }
        // reset values in SelectGraphicVector so the midpoint of the graphic flips about the midline of the selection
        for(unsigned int x = 0; x < Track->SelectGraphicVector.size(); x++)
        {
            int MidVPosBeforeFlip = Track->SelectGraphicVector.at(x).VPos + (Track->SelectGraphicVector.at(x).Height) / 2;
            int MidVPosAfterFlip = ((VerSum * 16) + 15) - MidVPosBeforeFlip;
            int TopPosAfterFlip = MidVPosAfterFlip - (Track->SelectGraphicVector.at(x).Height) / 2;
            Track->SelectGraphicVector.at(x).VPos = TopPosAfterFlip;
        }
        Level2TrackMode = Pasting;
        SetLevel2TrackMode(48);
        Utilities->CallLogPop(1426);
    }
    catch(const Exception &e)
    {
        ErrorLog(149, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::MirrorMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("MirrorMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",MirrorMenuItemClick");
        // reset values in SelectVector
        int HorSum = SelectRect.left + SelectRect.right - 1;
        for(unsigned int x = 0; x < Track->SelectVectorSize(); x++)
        {
            // See note above for FlipMenuItem relating to mods for v2.2.0
            int VLoc = Track->SelectVectorAt(22, x).VLoc;
            int HLoc = HorSum - Track->SelectVectorAt(6, x).HLoc;
            TTrackElement TE = Track->BuildBasicElementFromSpeedTag(1, Track->MirrorArray[Track->SelectVectorAt(27, x).SpeedTag]);
            TE.VLoc = VLoc;
            TE.HLoc = HLoc;

            TE.ActiveTrackElementName = Track->SelectVectorAt(44, x).ActiveTrackElementName; // these new in v2.4.0 so keeps attributes
            TE.LocationName = Track->SelectVectorAt(45, x).LocationName;
            TE.Length01 = Track->SelectVectorAt(46, x).Length01;
            TE.Length23 = Track->SelectVectorAt(47, x).Length23;
            TE.SpeedLimit01 = Track->SelectVectorAt(48, x).SpeedLimit01;
            TE.SpeedLimit23 = Track->SelectVectorAt(49, x).SpeedLimit23;
            TE.SigAspect = Track->SelectVectorAt(50, x).SigAspect;

// if(Track->SelectVectorAt(28, x).TrackType == SignalPost) TE.SigAspect = Track->SelectVectorAt(29, x).SigAspect;//TrackType will be the same
            Track->SelectVectorAt(30, x) = TE;
// Track->SelectVectorAt(, x).HLoc = HorSum - Track->SelectVectorAt(, x).HLoc;
// Track->SelectVectorAt(, x).SpeedTag = Track->MirrorArray[Track->SelectVectorAt(, x).SpeedTag];
        }
        // reset values in SelectTextVector
        for(unsigned int x = 0; x < TextHandler->SelectTextVectorSize(1); x++)
        {
            TTextItem *TextItem = TextHandler->SelectTextPtrAt(1, x);
            // also subtract half font height for each letter of text, brings position approximately right
            TextItem->HPos = ((HorSum * 16) + 15) - TextItem->HPos - (TextItem->TextString.Length() * 0.5 * abs(TextItem->Font->Height));
        }
        // reset values in SelectGraphicVector so the midpoint of the graphic mirrors about the midline of the selection
        for(unsigned int x = 0; x < Track->SelectGraphicVector.size(); x++)
        {
            int MidHPosBeforeMirror = Track->SelectGraphicVector.at(x).HPos + (Track->SelectGraphicVector.at(x).Width) / 2;
            int MidHPosAfterMirror = ((HorSum * 16) + 15) - MidHPosBeforeMirror;
            int LeftPosAfterMirror = MidHPosAfterMirror - (Track->SelectGraphicVector.at(x).Width) / 2;
            if(LeftPosAfterMirror < (SelectRect.left * 16)) // shouldn't go below left but check
            {
                LeftPosAfterMirror = SelectRect.left * 16;
            }
            Track->SelectGraphicVector.at(x).HPos = LeftPosAfterMirror;
        }
        Level2TrackMode = Pasting;
        SetLevel2TrackMode(49);
        Utilities->CallLogPop(1427);
    }
    catch(const Exception &e)
    {
        ErrorLog(150, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::RotateMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("Rotate180MenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",Rotate180MenuItemClick");
        // reset values in SelectVector
        int HorSum = SelectRect.left + SelectRect.right - 1;
        int VerSum = SelectRect.top + SelectRect.bottom - 1;
        for(unsigned int x = 0; x < Track->SelectVectorSize(); x++)
        {
            // See note above for FlipMenuItem relating to mods for v2.2.0
            int VLoc = VerSum - Track->SelectVectorAt(23, x).VLoc;
            int HLoc = HorSum - Track->SelectVectorAt(36, x).HLoc;
            TTrackElement TE = Track->BuildBasicElementFromSpeedTag(2, Track->MirrorArray[Track->FlipArray[Track->SelectVectorAt(31, x).SpeedTag]]);
            TE.VLoc = VLoc;
            TE.HLoc = HLoc;

            TE.ActiveTrackElementName = Track->SelectVectorAt(51, x).ActiveTrackElementName; // these new in v2.4.0 so keeps attributes
            TE.LocationName = Track->SelectVectorAt(52, x).LocationName;
            TE.Length01 = Track->SelectVectorAt(53, x).Length01;
            TE.Length23 = Track->SelectVectorAt(54, x).Length23;
            TE.SpeedLimit01 = Track->SelectVectorAt(55, x).SpeedLimit01;
            TE.SpeedLimit23 = Track->SelectVectorAt(56, x).SpeedLimit23;
            TE.SigAspect = Track->SelectVectorAt(57, x).SigAspect;

// if(Track->SelectVectorAt(32, x).TrackType == SignalPost) TE.SigAspect = Track->SelectVectorAt(33, x).SigAspect; dropped in v2.4.0 for above
            Track->SelectVectorAt(34, x) = TE;
// TTrackElement &TempEl = Track->SelectVectorAt(13, x);
// TempEl.HLoc = HorSum - TempEl.HLoc;
// TempEl.VLoc = VerSum - TempEl.VLoc;
// TempEl.SpeedTag = Track->MirrorArray[Track->FlipArray[TempEl.SpeedTag]];
        }
        // reset values in SelectTextVector
        for(unsigned int x = 0; x < TextHandler->SelectTextVectorSize(2); x++)
        {
            TTextItem *TextItem = TextHandler->SelectTextPtrAt(2, x);
            // also subtract half font height for each letter of text, brings position approximately right horizontally
            TextItem->HPos = ((HorSum * 16) + 15) - TextItem->HPos - (TextItem->TextString.Length() * 0.5 * abs(TextItem->Font->Height));
            // also subtract font height, brings position approximately right vertically
            TextItem->VPos = ((VerSum * 16) + 15) - TextItem->VPos - abs(TextItem->Font->Height);
        }
        // reset flip values in SelectGraphicVector so the midpoint of the graphic flips about the midline of the selection
        for(unsigned int x = 0; x < Track->SelectGraphicVector.size(); x++)
        {
            int MidVPosBeforeFlip = Track->SelectGraphicVector.at(x).VPos + (Track->SelectGraphicVector.at(x).Height) / 2;
            int MidVPosAfterFlip = ((VerSum * 16) + 15) - MidVPosBeforeFlip;
            int TopPosAfterFlip = MidVPosAfterFlip - (Track->SelectGraphicVector.at(x).Height) / 2;
            if(TopPosAfterFlip < (SelectRect.top * 16)) // shouldn't go above top but check
            {
                TopPosAfterFlip = SelectRect.top * 16;
            }
            Track->SelectGraphicVector.at(x).VPos = TopPosAfterFlip;
        }
        // reset mirror in SelectGraphicVector so the midpoint of the graphic mirrors about the midline of the selection
        for(unsigned int x = 0; x < Track->SelectGraphicVector.size(); x++)
        {
            int MidHPosBeforeMirror = Track->SelectGraphicVector.at(x).HPos + (Track->SelectGraphicVector.at(x).Width) / 2;
            int MidHPosAfterMirror = ((HorSum * 16) + 15) - MidHPosBeforeMirror;
            int LeftPosAfterMirror = MidHPosAfterMirror - (Track->SelectGraphicVector.at(x).Width) / 2;
            if(LeftPosAfterMirror < (SelectRect.left * 16)) // shouldn't go below left but check
            {
                LeftPosAfterMirror = SelectRect.left * 16;
            }
            Track->SelectGraphicVector.at(x).HPos = LeftPosAfterMirror;
        }
        // Level1Mode = TrackMode;
        // SetLevel1Mode(73);
        Level2TrackMode = Pasting;
        SetLevel2TrackMode(50);
        Utilities->CallLogPop(1435);
    }
    catch(const Exception &e)
    {
        ErrorLog(151, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::RotRightMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("RotateRight90MenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",RotateRight90MenuItemClick");
        Screen->Cursor = TCursor(-11); // Hourglass
        // check first if a square and if not give message & quit
        if((SelectRect.right - SelectRect.left) != (SelectRect.bottom - SelectRect.top))
        {
            // use left vertical side to make square & keep top lh corner unless rhs would exceed display, in which case use the right vertical & keep to rh corner
            int VertSize = SelectRect.bottom - SelectRect.top;
            if((SelectRect.Left + VertSize - Display->DisplayOffsetH) > Utilities->ScreenElementWidth)
            {
                // use right hand vertical & make square to left of that
                SelectRect.left = SelectRect.right - VertSize;
            }
            else
            {
                SelectRect.right = SelectRect.left + VertSize;
            }
            ClearandRebuildRailway(74);
            Display->PlotDashedRect(11, SelectRect);
            int button = Application->MessageBox
                (L"Original selection adjusted to make it square. 'OK' to keep this selection or 'Cancel' to make a new selection",
                L"Left click and hold here to move this message box", MB_OKCANCEL);
            if(button == IDCANCEL)
            {
                ResetSelectRect();
                Level1Mode = TrackMode; // call this first to clear everything, then set PrefDir mode
                SetLevel1Mode(133);
                Level2TrackMode = TrackSelecting;
                SetLevel2TrackMode(59);
                ClearandRebuildRailway(75);
                Screen->Cursor = TCursor(-2); // Arrow
                Utilities->CallLogPop(2121);
                return;
            }
        }
        // set SelectBitmap (only need the dimensions here as not moving the selection)
        SelectBitmapHLoc = SelectRect.left;
        SelectBitmapVLoc = SelectRect.top;
        SelectBitmap->Width = (SelectRect.right - SelectRect.left) * 16;
        SelectBitmap->Height = (SelectRect.bottom - SelectRect.top) * 16;
        // store track elements and text in select vectors
        Track->SelectVectorClear();
        TTrackElement TempElement; // default element
        bool FoundFlag;
        for(int x = SelectRect.left; x < SelectRect.right; x++)
        {
            for(int y = SelectRect.top; y < SelectRect.bottom; y++)
            {
                int ATVecPos = Track->GetVectorPositionFromTrackMap(57, x, y, FoundFlag);
                if(FoundFlag)
                {
                    TempElement = Track->TrackElementAt(959, ATVecPos);
                    if(TempElement.SpeedTag > 0)
                        Track->SelectPush(TempElement);
                }
            }
        }
        // now store inactive elements
        for(int x = SelectRect.left; x < SelectRect.right; x++)
        {
            for(int y = SelectRect.top; y < SelectRect.bottom; y++)
            {
                TTrack::TIMPair IATVecPair = Track->GetVectorPositionsFromInactiveTrackMap(28, x, y, FoundFlag);
                if(FoundFlag)
                {
                    TempElement = Track->InactiveTrackElementAt(126, IATVecPair.first);
                    Track->SelectPush(TempElement); // only want SpeedTag & location set, rest defaults
                    if(IATVecPair.second != IATVecPair.first) // 2 elements stored at location, i.e. platforms
                    {
                        TempElement = Track->InactiveTrackElementAt(127, IATVecPair.second);
                        Track->SelectPush(TempElement);
                    }
                }
            }
        }
        // store text items
        int LowSelectHPos = SelectRect.left * 16;
        int HighSelectHPos = SelectRect.right * 16;
        int LowSelectVPos = SelectRect.top * 16;
        int HighSelectVPos = SelectRect.bottom * 16;
        TextHandler->SelectTextVector.clear();
        if(!TextHandler->TextVector.empty()) // skip iteration if empty else have an error
        {
            for(TTextHandler::TTextVectorIterator TextPtr = TextHandler->TextVector.begin(); TextPtr < TextHandler->TextVector.end(); TextPtr++)
            {
                if((TextPtr->HPos >= LowSelectHPos) && (TextPtr->HPos < HighSelectHPos) && (TextPtr->VPos >= LowSelectVPos) && (TextPtr->VPos < HighSelectVPos))
                {
                    // have to create a new TextItem in order to create a new Font object
                    // BUT: only create new items where they don't appear as named location names
                    // in SelectVector, since those names shouldn't be copied or pasted.
                    // NB:  altered for PasteWithAttributes - at v2.2.0 save the named element but prefix it with "##**"
                    // so can paste or not depending on which type of paste is being used (unlikely to use that in a real name)
                    bool SelectVectorNamedElement = false;
                    AnsiString SelectTextString; // new at v2.2.0
                    for(unsigned int x = 0; x < Track->SelectVector.size(); x++)
                    {
                        if(Track->SelectVector.at(x).LocationName == TextPtr->TextString)
                        {
                            SelectVectorNamedElement = true;
                            break;
                        }
                    }
                    if(SelectVectorNamedElement) // changed at v2.2.0
                    {
                        SelectTextString = "##**" + TextPtr->TextString; // new at v2.2.0
                    }
                    else // new at v2.2.0
                    {
                        SelectTextString = TextPtr->TextString;
                    }
                    TTextItem TextItem(TextPtr->HPos, TextPtr->VPos, SelectTextString, TextPtr->Font);
                    TextHandler->SelectTextVector.push_back(TextItem); // changed at v2.2.0
                }
            }
        }
        // store graphic items, but first clear SelectGraphicVector
        Track->SelectGraphicVector.clear();
        if(!Track->UserGraphicVector.empty()) // skip iteration if empty else have an error
        {
            for(TTrack::TUserGraphicVector::iterator UserGraphicPtr = Track->UserGraphicVector.begin(); UserGraphicPtr < Track->UserGraphicVector.end();
            UserGraphicPtr++)
            {
                if((UserGraphicPtr->HPos >= LowSelectHPos) && ((UserGraphicPtr->HPos + UserGraphicPtr->Width) < HighSelectHPos) && (UserGraphicPtr->VPos >=
                    LowSelectVPos) && ((UserGraphicPtr->VPos + UserGraphicPtr->Height) < HighSelectVPos))
                {
                    Track->SelectGraphicVector.push_back(*UserGraphicPtr);
                }
            }
        }
        // now transform the H & V for rh rotate
        for(unsigned int x = 0; x < Track->SelectVectorSize(); x++)
        {
            int HLoc = SelectRect.bottom - 1 + SelectRect.left - Track->SelectVectorAt(74, x).VLoc;
            int VLoc = SelectRect.top - SelectRect.left + Track->SelectVectorAt(75, x).HLoc;
            TTrackElement TE = Track->BuildBasicElementFromSpeedTag(3, Track->RotRightArray[Track->SelectVectorAt(78, x).SpeedTag]);
            TE.VLoc = VLoc;
            TE.HLoc = HLoc;

            TE.ActiveTrackElementName = Track->SelectVectorAt(58, x).ActiveTrackElementName; // these new in v2.4.0 so keeps attributes
            TE.LocationName = Track->SelectVectorAt(59, x).LocationName;
            TE.Length01 = Track->SelectVectorAt(60, x).Length01;
            TE.Length23 = Track->SelectVectorAt(61, x).Length23;
            TE.SpeedLimit01 = Track->SelectVectorAt(62, x).SpeedLimit01;
            TE.SpeedLimit23 = Track->SelectVectorAt(63, x).SpeedLimit23;
            TE.SigAspect = Track->SelectVectorAt(64, x).SigAspect;
            Track->SelectVectorAt(65, x) = TE;
        }
        // reset values in SelectTextVector
        for(unsigned int x = 0; x < TextHandler->SelectTextVectorSize(3); x++)
        {
// no point trying to locate text properly as it stays horizontal so will always be wrongly placed, just list all itels vertically at lhs
// & if a lot then some will extend beyond the selection
            TTextItem *TextItem = TextHandler->SelectTextPtrAt(3, x);
            // also subtract half font height for each letter of text, brings position approximately right horizontally
            TextItem->HPos = (SelectRect.left) * 16;
            TextItem->VPos = (SelectRect.top + x) * 16;
        }
        // reset values in SelectGraphicVector
        for(unsigned int x = 0; x < Track->SelectGraphicVector.size(); x++)
        {
            int MidHPosBeforeRotate = Track->SelectGraphicVector.at(x).HPos + Track->SelectGraphicVector.at(x).Width / 2;
            int MidVPosBeforeRotate = Track->SelectGraphicVector.at(x).VPos + Track->SelectGraphicVector.at(x).Height / 2;
            int MidHPosAfterRotate = ((SelectRect.bottom * 16) - 1) + (SelectRect.left * 16) - MidVPosBeforeRotate;
            int MidVPosAfterRotate = ((SelectRect.top - SelectRect.left) * 16) + MidHPosBeforeRotate;
            int LeftPosAfterRotate = MidHPosAfterRotate - (Track->SelectGraphicVector.at(x).Width) / 2;
            int TopPosAfterRotate = MidVPosAfterRotate - (Track->SelectGraphicVector.at(x).Height) / 2;
            Track->SelectGraphicVector.at(x).HPos = LeftPosAfterRotate;
            Track->SelectGraphicVector.at(x).VPos = TopPosAfterRotate;
        }
        Screen->Cursor = TCursor(-2); // Arrow
        Level2TrackMode = Pasting;
        SetLevel2TrackMode(60);
        Utilities->CallLogPop(2122);
    }
    catch(const Exception &e)
    {
        ErrorLog(205, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::RotLeftMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("RotateLeft90MenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",RotateLeft90MenuItemClick");
        Screen->Cursor = TCursor(-11); // Hourglass;
        // check first if a square and if not give message & quit
        if((SelectRect.right - SelectRect.left) != (SelectRect.bottom - SelectRect.top))
        {
            // use left vertical side to make square & keep top lh corner unless rhs would exceed display, in which case use the right vertical & keep to rh corner
            int VertSize = SelectRect.bottom - SelectRect.top;
            if((SelectRect.Left + VertSize - Display->DisplayOffsetH) > Utilities->ScreenElementWidth)
            {
                // use right hand vertical & make square to left of that
                SelectRect.left = SelectRect.right - VertSize;
            }
            else
            {
                SelectRect.right = SelectRect.left + VertSize;
            }
            ClearandRebuildRailway(76);
            Display->PlotDashedRect(12, SelectRect);
            int button = Application->MessageBox
                (L"Original selection adjusted to make it square. 'OK' to keep this selection or 'Cancel' to make a new selection",
                L"Left click and hold here to move this message box", MB_OKCANCEL);
            if(button == IDCANCEL)
            {
                ResetSelectRect();
                Level1Mode = TrackMode; // call this first to clear everything, then set PrefDir mode
                SetLevel1Mode(134);
                Level2TrackMode = TrackSelecting;
                SetLevel2TrackMode(61);
                ClearandRebuildRailway(77);
                Screen->Cursor = TCursor(-2); // Arrow
                Utilities->CallLogPop(2123);
                return;
            }
        }
        // set SelectBitmap (only need the dimensions here as not moving the selection)
        SelectBitmapHLoc = SelectRect.left;
        SelectBitmapVLoc = SelectRect.top;
        SelectBitmap->Width = (SelectRect.right - SelectRect.left) * 16;
        SelectBitmap->Height = (SelectRect.bottom - SelectRect.top) * 16;
        // store track elements and text in select vectors
        Track->SelectVectorClear();
        TTrackElement TempElement; // default element
        bool FoundFlag;
        for(int x = SelectRect.left; x < SelectRect.right; x++)
        {
            for(int y = SelectRect.top; y < SelectRect.bottom; y++)
            {
                int ATVecPos = Track->GetVectorPositionFromTrackMap(58, x, y, FoundFlag);
                if(FoundFlag)
                {
                    TempElement = Track->TrackElementAt(960, ATVecPos);
                    if(TempElement.SpeedTag > 0)
                        Track->SelectPush(TempElement);
                }
            }
        }
        // now store inactive elements
        for(int x = SelectRect.left; x < SelectRect.right; x++)
        {
            for(int y = SelectRect.top; y < SelectRect.bottom; y++)
            {
                TTrack::TIMPair IATVecPair = Track->GetVectorPositionsFromInactiveTrackMap(29, x, y, FoundFlag);
                if(FoundFlag)
                {
                    TempElement = Track->InactiveTrackElementAt(128, IATVecPair.first);
                    Track->SelectPush(TempElement); // only want SpeedTag & location set, rest defaults
                    if(IATVecPair.second != IATVecPair.first) // 2 elements stored at location, i.e. platforms
                    {
                        TempElement = Track->InactiveTrackElementAt(129, IATVecPair.second);
                        Track->SelectPush(TempElement);
                    }
                }
            }
        }
        // store text items
        int LowSelectHPos = SelectRect.left * 16;
        int HighSelectHPos = SelectRect.right * 16;
        int LowSelectVPos = SelectRect.top * 16;
        int HighSelectVPos = SelectRect.bottom * 16;
        TextHandler->SelectTextVector.clear();
        if(!TextHandler->TextVector.empty()) // skip iteration if empty else have an error
        {
            for(TTextHandler::TTextVectorIterator TextPtr = TextHandler->TextVector.begin(); TextPtr < TextHandler->TextVector.end(); TextPtr++)
            {
                if((TextPtr->HPos >= LowSelectHPos) && (TextPtr->HPos < HighSelectHPos) && (TextPtr->VPos >= LowSelectVPos) && (TextPtr->VPos < HighSelectVPos))
                {
                    // have to create a new TextItem in order to create a new Font object
                    // BUT: only create new items where they don't appear as named location names
                    // in SelectVector, since those names shouldn't be copied or pasted.
                    // NB:  altered for PasteWithAttributes - at v2.2.0 save the named element but prefix it with "##**"
                    // so can paste or not depending on which type of paste is being used (unlikely to use that in a real name)
                    bool SelectVectorNamedElement = false;
                    AnsiString SelectTextString; // new at v2.2.0
                    for(unsigned int x = 0; x < Track->SelectVector.size(); x++)
                    {
                        if(Track->SelectVector.at(x).LocationName == TextPtr->TextString)
                        {
                            SelectVectorNamedElement = true;
                            break;
                        }
                    }
                    if(SelectVectorNamedElement) // changed at v2.2.0
                    {
                        SelectTextString = "##**" + TextPtr->TextString; // new at v2.2.0
                    }
                    else // new at v2.2.0
                    {
                        SelectTextString = TextPtr->TextString;
                    }
                    TTextItem TextItem(TextPtr->HPos, TextPtr->VPos, SelectTextString, TextPtr->Font);
                    TextHandler->SelectTextVector.push_back(TextItem); // changed at v2.2.0
                }
            }
        }
        // store graphic items, but first clear SelectGraphicVector
        Track->SelectGraphicVector.clear();
        if(!Track->UserGraphicVector.empty()) // skip iteration if empty else have an error
        {
            for(TTrack::TUserGraphicVector::iterator UserGraphicPtr = Track->UserGraphicVector.begin(); UserGraphicPtr < Track->UserGraphicVector.end();
            UserGraphicPtr++)
            {
                if((UserGraphicPtr->HPos >= LowSelectHPos) && ((UserGraphicPtr->HPos + UserGraphicPtr->Width) < HighSelectHPos) && (UserGraphicPtr->VPos >=
                    LowSelectVPos) && ((UserGraphicPtr->VPos + UserGraphicPtr->Height) < HighSelectVPos))
                {
                    Track->SelectGraphicVector.push_back(*UserGraphicPtr);
                }
            }
        }
        // now transform the H & V for lh rotate
        for(unsigned int x = 0; x < Track->SelectVectorSize(); x++)
        {
            int HLoc = SelectRect.left - SelectRect.top + Track->SelectVectorAt(76, x).VLoc;
            int VLoc = SelectRect.bottom - 1 + SelectRect.left - Track->SelectVectorAt(77, x).HLoc;
            TTrackElement TE = Track->BuildBasicElementFromSpeedTag(4, Track->RotLeftArray[Track->SelectVectorAt(79, x).SpeedTag]);
            TE.VLoc = VLoc;
            TE.HLoc = HLoc;

            TE.ActiveTrackElementName = Track->SelectVectorAt(66, x).ActiveTrackElementName; // these new in v2.4.0 so keeps attributes
            TE.LocationName = Track->SelectVectorAt(67, x).LocationName;
            TE.Length01 = Track->SelectVectorAt(68, x).Length01;
            TE.Length23 = Track->SelectVectorAt(69, x).Length23;
            TE.SpeedLimit01 = Track->SelectVectorAt(70, x).SpeedLimit01;
            TE.SpeedLimit23 = Track->SelectVectorAt(71, x).SpeedLimit23;
            TE.SigAspect = Track->SelectVectorAt(72, x).SigAspect;
            Track->SelectVectorAt(73, x) = TE;
        }
        // reset values in SelectTextVector
        for(unsigned int x = 0; x < TextHandler->SelectTextVectorSize(4); x++)
        {
// no point trying to locate text properly as it stays horizontal so will always be wrongly placed, just list all itels vertically at lhs
// & if a lot then some will extend beyond the selection
            TTextItem *TextItem = TextHandler->SelectTextPtrAt(4, x);
            // also subtract half font height for each letter of text, brings position approximately right horizontally
            TextItem->HPos = (SelectRect.left) * 16;
            TextItem->VPos = (SelectRect.top + x) * 16;
        }
        // reset values in SelectGraphicVector
        for(unsigned int x = 0; x < Track->SelectGraphicVector.size(); x++)
        {
            int MidHPosBeforeRotate = Track->SelectGraphicVector.at(x).HPos + Track->SelectGraphicVector.at(x).Width / 2;
            int MidVPosBeforeRotate = Track->SelectGraphicVector.at(x).VPos + Track->SelectGraphicVector.at(x).Height / 2;
            int MidHPosAfterRotate = ((SelectRect.left - SelectRect.top) * 16) + MidVPosBeforeRotate;
            int MidVPosAfterRotate = ((SelectRect.bottom * 16) - 1) + (SelectRect.left * 16) - MidHPosBeforeRotate;
            int LeftPosAfterRotate = MidHPosAfterRotate - (Track->SelectGraphicVector.at(x).Width) / 2;
            int TopPosAfterRotate = MidVPosAfterRotate - (Track->SelectGraphicVector.at(x).Height) / 2;
            Track->SelectGraphicVector.at(x).HPos = LeftPosAfterRotate;
            Track->SelectGraphicVector.at(x).VPos = TopPosAfterRotate;
        }
        Screen->Cursor = TCursor(-2); // Arrow
        Level2TrackMode = Pasting;
        SetLevel2TrackMode(62);
        Utilities->CallLogPop(2124);
    }
    catch(const Exception &e)
    {
        ErrorLog(206, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::PasteMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("PasteMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",PasteMenuItemClick");
        // Level1Mode = TrackMode;
        // SetLevel1Mode(74);
        Level2TrackMode = Pasting;
        SetLevel2TrackMode(58);
        Utilities->CallLogPop(2060);
    }
    catch(const Exception &e)
    {
        ErrorLog(198, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::DeleteMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("DeleteMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",DeleteMenuItemClick");
        // Level1Mode = TrackMode;
        // SetLevel1Mode(75);
        Level2TrackMode = Deleting;
        SetLevel2TrackMode(38);
        Utilities->CallLogPop(1193);
    }
    catch(const Exception &e)
    {
        ErrorLog(153, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::SelectLengthsMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("SelectLengthsMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",SelectLengthsMenuItemClick");
        TrackElementPanel->Visible = false;
        TrackLengthPanel->Visible = true;
        TrackLengthPanel->SetFocus();
        SelectLengthsFlag = true;
        InfoPanel->Visible = true;
        InfoPanel->Caption = "DISTANCE/SPEED SETTING:  Set values or leave blank for no change";
        ShowMessage("Note: length value will apply to each element's track within the selection");
        DistanceBox->Text = "";
        SpeedLimitBox->Text = "";
        SelectBitmapHLoc = SelectRect.left;
        SelectBitmapVLoc = SelectRect.top;
        ResetChangedFileDataAndCaption(19, true); // true for NonPrefDirChangesMade
        Utilities->CallLogPop(1414);
    }
    catch(const Exception &e)
    {
        ErrorLog(154, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::SelectBiDirPrefDirsMenuItemClick(TObject *Sender)
{
/* SelectVector contains all the track elements (and inactive elements but don't need them), so create up to 4 PrefDir
   elements from each one, and add each into ConstructPrefDir, then when all added use ConsolidatePrefDirs to add to EveryPrefDir
*/
    try
    {
        TrainController->LogEvent("SelectBiDirPrefDirsMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",SelectBiDirPrefDirsMenuItemClick");
        ConstructPrefDir->ExternalClearPrefDirAnd4MultiMap();
        bool FoundFlag = false;
        if(Track->SelectVector.empty())
        {
            Utilities->CallLogPop(1550);
            return;
        }
        for(unsigned int x = 0; x < Track->SelectVectorSize(); x++)
        {
            TTrackElement TE = Track->SelectVectorAt(14, x);
            int VecPos = Track->GetVectorPositionFromTrackMap(42, TE.HLoc, TE.VLoc, FoundFlag);
            if(FoundFlag)
            {
                if((TE.TrackType == Points) || (TE.TrackType == Bridge) || (TE.TrackType == Crossover)) // 2-track element
                {
                    TPrefDirElement PE0(TE, TE.Link[0], 0, TE.Link[1], 1, VecPos);
                    ConstructPrefDir->ExternalStorePrefDirElement(0, PE0);
                    TPrefDirElement PE1(TE, TE.Link[1], 1, TE.Link[0], 0, VecPos);
                    ConstructPrefDir->ExternalStorePrefDirElement(1, PE1);
                    TPrefDirElement PE2(TE, TE.Link[2], 2, TE.Link[3], 3, VecPos);
                    ConstructPrefDir->ExternalStorePrefDirElement(2, PE2);
                    TPrefDirElement PE3(TE, TE.Link[3], 3, TE.Link[2], 2, VecPos);
                    ConstructPrefDir->ExternalStorePrefDirElement(3, PE3);
                }
                else if((TE.TrackType == Simple) || (TE.TrackType == Buffers) || (TE.TrackType == SignalPost) || (TE.TrackType == Continuation) ||
                    (TE.TrackType == GapJump) || (TE.TrackType == FootCrossing))
                // need to list these explicitly since inactive elements will still be 'found' if there is an active element
                // at the same position
                {
                    TPrefDirElement PE0(TE, TE.Link[0], 0, TE.Link[1], 1, VecPos);
                    ConstructPrefDir->ExternalStorePrefDirElement(4, PE0);
                    TPrefDirElement PE1(TE, TE.Link[1], 1, TE.Link[0], 0, VecPos);
                    ConstructPrefDir->ExternalStorePrefDirElement(5, PE1);
                }
            }
        }
        EveryPrefDir->ConsolidatePrefDirs(2, ConstructPrefDir);
        ResetChangedFileDataAndCaption(22, false);
        // RlyFile = false; - don't alter this just for PrefDir changes
        Level1Mode = BaseMode; // call this first to clear everything, then set PrefDir mode
        SetLevel1Mode(30);
        Level1Mode = PrefDirMode;
        SetLevel1Mode(31); // calls Clearand... to display all PrefDirs
        Utilities->CallLogPop(1549);
    }
    catch(const Exception &e)
    {
        ErrorLog(155, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::CancelSelectionMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("CancelSelectionClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",CancelSelectionClick");
        ClearandRebuildRailway(46); // to remove the selection outline
        SelectionValid = false;
        Track->CopyFlag = false;
        Track->SkipLocationNameMultiMapCheck = false;
        ResetSelectRect();
        if(Level1Mode == TrackMode)
        {
            SetLevel1Mode(76);
            Level2TrackMode = AddTrack;
            SetLevel2TrackMode(39);
        }
        else if(Level1Mode == PrefDirMode)
        {
            SetLevel1Mode(32);
        }
        Utilities->CallLogPop(1413);
    }
    catch(const Exception &e)
    {
        ErrorLog(156, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::LoadTimetableMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("LoadTimetableMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",LoadTimetableMenuItemClick");
        TimetableDialog->Filter = "Timetable file (*.ttb)|*.ttb";
        // reset all message flags, stops them being given twice  new at v2.4.0
        TrainController->SSHigh = false;
        TrainController->MRSHigh = false;
        TrainController->MRSLow = false;
        TrainController->MassHigh = false;
        TrainController->BFHigh = false;
        TrainController->BFLow = false;
        TrainController->PwrHigh = false;
        TrainController->SigSHigh = false;
        TrainController->SigSLow = false;
        if(TimetableDialog->Execute())
        {
            TrainController->LogEvent("LoadTimetable " + TimetableDialog->FileName);
            bool CheckLocationsExistInRailwayTrue = true;
            if(TrainController->TimetableIntegrityCheck(0, AnsiString(TimetableDialog->FileName).c_str(), true, CheckLocationsExistInRailwayTrue))
    // true for GiveMessages
            {
                Screen->Cursor = TCursor(-11); // Hourglass;
                std::ifstream TTBLFile(AnsiString(TimetableDialog->FileName).c_str(), std::ios_base::binary);
                if(TTBLFile.is_open())
                {
                    bool SessionFileFalse = false;
                    if(BuildTrainDataVectorForLoadFile(0, TTBLFile, true, CheckLocationsExistInRailwayTrue, SessionFileFalse)) // true for GiveMessages
                    {
                        SaveTempTimetableFile(0, TimetableDialog->FileName);
                    } // don't need an 'else' as messages given in BuildTrainDataVectorForLoadFile
                }
                else
                {
                    ShowMessage("Failed to open timetable file, make sure it's not open in another application");
                }
                Screen->Cursor = TCursor(-2); // Arrow
            } // if(TimetableIntegrityCheck
            else
                ShowMessage("Timetable preliminary integrity check failed - unable to load");
        } // if(TimetableDialog->Execute())
          // else ShowMessage("Load Aborted");
        Utilities->CallLogPop(752);
    }
    catch(const Exception &e)
    {
        ErrorLog(34, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::TakeSignallerControlMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("SignallerControl1Click");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",SignallerControl1Click");
        TTrain &Train = TrainController->TrainVectorAtIdent(17, SelectedTrainID);
        Train.SignallerStoppingFlag = false;
        Train.TrainMode = Signaller;
        if(Train.MaxRunningSpeed > Train.SignallerMaxSpeed)
        {
            Train.MaxRunningSpeed = Train.SignallerMaxSpeed;
        }
        if(Train.Stopped())
            Train.SignallerStopped = true; // condition added at v2.4.0 to allow for taking sig control of failed moving trains
        Train.CallingOnFlag = false; // in case was set, wouldn't start anyway if called on as SignallerStopped = true
        RailGraphics->ChangeForegroundColour(25, Train.HeadCodePosition[0], Train.FrontCodePtr, clFrontCodeSignaller, Train.BackgroundColour);
        Train.PlotTrain(5, Display);
        AnsiString LocName = "";
        if(Train.LeadElement > -1)
        {
            LocName = Track->TrackElementAt(633, Train.LeadElement).ActiveTrackElementName;
        }
        if((LocName == "") && (Train.MidElement > -1))
        {
            LocName = Track->TrackElementAt(634, Train.MidElement).ActiveTrackElementName;
        }

        // store the value that allow restoration of tt control or not - RestoreTimetableLocation
        if(Train.StoppedAtLocation && (LocName != ""))
        {
            Train.RestoreTimetableLocation = LocName;
        }
        else
        {
            Train.RestoreTimetableLocation = "";
        }

        // check whether need to offer 'pass red signal'
        if(!Train.StoppedAtSignal && Train.StoppedAtLocation)
        {
            int NextElementPosition = Track->TrackElementAt(775, Train.LeadElement).Conn[Train.LeadExitPos];
            int NextEntryPos = Track->TrackElementAt(776, Train.LeadElement).ConnLinkPos[Train.LeadExitPos];
            if((NextElementPosition > -1) && (NextEntryPos > -1))
            {
                if((Track->TrackElementAt(777, NextElementPosition).Config[Track->GetNonPointsOppositeLinkPos(NextEntryPos)] == Signal) &&
                    (Track->TrackElementAt(778, NextElementPosition).Attribute == 0))
                { // set both StoppedAtLocation & StoppedAtSignal, so that 'pass red signal' is offered in popup menu rather than move
                  // forwards, but don't change the background colour so still shows as stopped at location
                    Train.StoppedAtSignal = true;
                }
            }
        }
        // find element ID if no locname
        if((LocName == "") && Train.LeadElement > -1)
        {
            LocName = Track->TrackElementAt(635, Train.LeadElement).ElementID;
        }
        if((LocName == "") && (Train.MidElement > -1))
        {
            LocName = Track->TrackElementAt(636, Train.MidElement).ElementID;
        }
        Train.LogAction(0, Train.HeadCode, "", TakeSignallerControl, LocName, TDateTime(0), false); // TDateTime is a dummy entry, false for no warning
        Utilities->CallLogPop(1772);
    }
    catch(const Exception &e)
    {
        ErrorLog(157, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::TimetableControlMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("TimetableControlMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",TimetableControlMenuItemClick");
        TTrain &Train = TrainController->TrainVectorAtIdent(18, SelectedTrainID);
        Train.SignallerStoppingFlag = false;
        Train.TrainMode = Timetable;
        Train.SignallerStopped = false;
        Train.StoppedAfterSPAD = false;
        Train.SPADFlag = false;
        Train.MaxRunningSpeed = Train.TimetableMaxRunningSpeed;
        RailGraphics->ChangeForegroundColour(18, Train.HeadCodePosition[0], Train.FrontCodePtr, clFrontCodeTimetable, Train.BackgroundColour);
// red headcode[0]
        Train.PlotTrain(6, Display);
        AnsiString LocName = "";
        if(Train.LeadElement > -1)
        {
            LocName = Track->TrackElementAt(645, Train.LeadElement).ActiveTrackElementName;
        }
        if((LocName == "") && (Train.MidElement > -1))
        {
            LocName = Track->TrackElementAt(647, Train.MidElement).ActiveTrackElementName;
        }
        if((LocName == "") && Train.LeadElement > -1)
        {
            LocName = Track->TrackElementAt(646, Train.LeadElement).ElementID;
        }
        if((LocName == "") && (Train.MidElement > -1))
        {
            LocName = Track->TrackElementAt(648, Train.MidElement).ElementID;
        }
        if((Train.ActionVectorEntryPtr->LocationType == AtLocation) && (LocName == Train.ActionVectorEntryPtr->LocationName))
        {
            Train.StoppedAtLocation = true;
            Train.LastActionTime = TrainController->TTClockTime; // by itself this only affects trains that have still to arrive, if waiting to
                                                                 // depart the departure time & TRS time have already been calculated so need to
                                                                 // force a recalculation - see below
            Train.DepartureTimeSet = false; // force it to be recalculated based on new LastActionTime (if waiting to arrive this is false anyway)
            if(!Train.TrainFailed)
            {
                Train.PlotTrainWithNewBackgroundColour(28, clStationStopBackground, Display); // pale green
            }
            // ok to call PlotTrainWithNewBackgroundColour here as PlotElements already set to Lead, Mid & Lag elements
            if((Train.ActionVectorEntryPtr->FormatType == TimeLoc) && (Train.ActionVectorEntryPtr->ArrivalTime >= TDateTime(0)))
            { // Timetable indicates that train still waiting to arrive for a TimeLoc arrival so send message and mark as arrived
                Train.LogAction(28, Train.HeadCode, "", Arrive, LocName, Train.ActionVectorEntryPtr->ArrivalTime, Train.ActionVectorEntryPtr->Warning);
                Train.ActionVectorEntryPtr++; // advance pointer past arrival  //added at v1.2.0
            }
            else if((Train.ActionVectorEntryPtr->FormatType == TimeTimeLoc) && !(Train.TimeTimeLocArrived))
            { // Timetable indicates that train still waiting to arrive for a TimeTimeLoc arrival so send message and mark as arrived
                Train.LogAction(29, Train.HeadCode, "", Arrive, LocName, Train.ActionVectorEntryPtr->ArrivalTime, Train.ActionVectorEntryPtr->Warning);
                Train.TimeTimeLocArrived = true;
                // NB: No need for 'Train.ActionVectorEntryPtr++' because still to act on the departure time
            }
        }
        else
        {
            int NextElementPos = -1; // addition for v1.3.2 due to Carwyn Thomas error
            int NextEntryPos = -1; // ---ditto---
            if(Train.LeadElement > -1) // ---ditto---
            { // ---ditto---
                NextElementPos = Track->TrackElementAt(658, Train.LeadElement).Conn[Train.LeadExitPos]; // had 'int' prefix before additions
                NextEntryPos = Track->TrackElementAt(659, Train.LeadElement).ConnLinkPos[Train.LeadExitPos]; // ---ditto---
            } // ---ditto---
            // ok to call PlotTrainWithNewBackgroundColour here as PlotElements already set to Lead, Mid & Lag elements
            if(!Train.TrainFailed)
            {
                Train.PlotTrainWithNewBackgroundColour(31, clNormalBackground, Display); // to remove other background if was present, moved from
            } // within Train.AbleToMove at v2.4.0 to cancel signal stop background
            if(Train.AbleToMove(1)) // if has no power
            {
                Train.EntrySpeed = 0; // moved from below for v1.3.2 after Carwyn Thomas error
                Train.EntryTime = TrainController->TTClockTime; // ---Ditto---
                Train.FirstHalfMove = true; // ---Ditto---
                if((NextElementPos > -1) && (NextEntryPos > -1)) // changed from if(NextElementPos >= 0) as above
                {
                    // Train.EntrySpeed = 0;
                    // Train.EntryTime = TrainController->TTClockTime;
                    // Train.FirstHalfMove = true;
                    Train.SetTrainMovementValues(15, NextElementPos, NextEntryPos);
                }
                // else follow the continuations   //added these 3 conditions for v1.3.2 after Carwyn Thomas error
                else if((Train.LeadElement > -1) && (Track->TrackElementAt(894, Train.LeadElement).TrackType == Continuation))
                {
                    Train.SetTrainMovementValues(21, Train.LeadElement, Train.LeadEntryPos); // Use LeadElement for calcs if lead is a continuation
                }
                else if((Train.MidElement > -1) && (Track->TrackElementAt(895, Train.MidElement).TrackType == Continuation))
                {
                    Train.SetTrainMovementValues(22, Train.MidElement, Train.MidEntryPos); // Use MidElement for calcs if Mid is a continuation
                }
                else if((Train.LagElement > -1) && (Track->TrackElementAt(896, Train.LagElement).TrackType == Continuation))
                {
                    Train.SetTrainMovementValues(23, Train.LagElement, Train.LagEntryPos); // Use LagElement for calcs if Lag is a continuation
                }
            }
            else if(Train.StoppedAtSignal)
            {
                // ok to call PlotTrainWithNewBackgroundColour here as PlotElements already set to Lead, Mid & Lag elements
                if(!Train.TrainFailed)
                {
                    Train.PlotTrainWithNewBackgroundColour(45, clSignalStopBackground, Display);
                }
                // TrainController->LogActionError(42, Train.HeadCode, "", SignalHold, Track->TrackElementAt(757, NextElementPos).ElementID);
            }
        }
        Train.LogAction(1, Train.HeadCode, "", RestoreTimetableControl, LocName, TDateTime(0), false); // TDateTime is a dummy entry, false for no warning
        Utilities->CallLogPop(1195);
    }
    catch(const Exception &e)
    {
        ErrorLog(158, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::ChangeDirectionMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("ChangeDirectionMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",ChangeDirectionMenuItemClick");
        TTrain &Train = TrainController->TrainVectorAtIdent(19, SelectedTrainID);
        Train.SignallerStoppingFlag = false;
        Train.SignallerChangeTrainDirection(0); // this unplots & replots train, which checks for facing signal and sets StoppedAtSignal if req'd
        Train.SignallerStopped = true;
        AnsiString LocName = "";
        if(Train.LeadElement > -1)
        {
            LocName = Track->TrackElementAt(637, Train.LeadElement).ActiveTrackElementName;
        }
        if((LocName == "") && (Train.MidElement > -1))
        {
            LocName = Track->TrackElementAt(638, Train.MidElement).ActiveTrackElementName;
        }
        if((LocName == "") && Train.LeadElement > -1)
        {
            LocName = Track->TrackElementAt(639, Train.LeadElement).ElementID;
        }
        if((LocName == "") && (Train.MidElement > -1))
        {
            LocName = Track->TrackElementAt(640, Train.MidElement).ElementID;
        }
        Train.LogAction(2, Train.HeadCode, "", SignallerChangeDirection, LocName, TDateTime(0), false); // TDateTime is a dummy entry, false for no warning
        Utilities->CallLogPop(1196);
    }
    catch(const Exception &e)
    {
        ErrorLog(159, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::MoveForwardsMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("MoveForwardsMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",MoveForwardsMenuItemClick");
        TTrain &Train = TrainController->TrainVectorAtIdent(20, SelectedTrainID);
        Train.SignallerStoppingFlag = false;
        if(!Train.AbleToMove(2))
        { // shouldn't be here as when unable to move MoveForwards shouldn't be enabled, but leave in as a precaution
            Utilities->CallLogPop(1197);
            return;
        }
        Train.SignallerStopped = false;
        Train.StoppedAfterSPAD = false; // in case had been set
        Train.SPADFlag = false;
        Train.StoppedAtLocation = false; // may not have been set but reset anyway
        // ok to call PlotTrainWithNewBackgroundColour here as PlotElements already set to Lead, Mid & Lag elements
        Train.PlotTrainWithNewBackgroundColour(29, clNormalBackground, Display);
        Train.EntrySpeed = 0;
        Train.EntryTime = TrainController->TTClockTime;
        Train.FirstHalfMove = true;
        int NextElementPos = -1; // addition for v1.3.2 due to Carwyn Thomas error
        int NextEntryPos = -1; // ---ditto---
        if(Train.LeadElement > -1) // ---ditto---
        { // ---ditto---
            NextElementPos = Track->TrackElementAt(652, Train.LeadElement).Conn[Train.LeadExitPos]; // had 'int' prefix before additions
            NextEntryPos = Track->TrackElementAt(657, Train.LeadElement).ConnLinkPos[Train.LeadExitPos]; // ---ditto---
        } // ---ditto---
        if((NextElementPos > -1) && (NextEntryPos > -1))
        {
            Train.SetTrainMovementValues(14, NextElementPos, NextEntryPos); // NextElement is the element to be entered
        }
        // else follow the continuations
        else if((Train.LeadElement > -1) && (Track->TrackElementAt(784, Train.LeadElement).TrackType == Continuation))
        {
            Train.SetTrainMovementValues(17, Train.LeadElement, Train.LeadEntryPos); // Use LeadElement for calcs if lead is a continuation
        }
        else if((Train.MidElement > -1) && (Track->TrackElementAt(785, Train.MidElement).TrackType == Continuation))
        {
            Train.SetTrainMovementValues(18, Train.MidElement, Train.MidEntryPos); // Use MidElement for calcs if Mid is a continuation
        }
        else if((Train.LagElement > -1) && (Track->TrackElementAt(786, Train.LagElement).TrackType == Continuation))
        {
            Train.SetTrainMovementValues(19, Train.LagElement, Train.LagEntryPos); // Use LagElement for calcs if Lag is a continuation
        }
        Train.LogAction(3, Train.HeadCode, "", SignallerMoveForwards, "", TDateTime(0), false); // TDateTime is a dummy entry, false for no warning
        Utilities->CallLogPop(1198);
    }
    catch(const Exception &e)
    {
        ErrorLog(160, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::SignallerJoinedByMenuItemClick(TObject *Sender)
{ // new at v2.4.0
    try
    {
        TrainController->LogEvent("JoinedByMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",JoinedByMenuItemClick");
        TTrain *TrainToBeJoinedBy;
        TTrain &ThisTrain = TrainController->TrainVectorAtIdent(45, SelectedTrainID);
        if(ThisTrain.IsThereAnAdjacentTrain(1, TrainToBeJoinedBy)) // this must come before both powers zero check in order to set a valid TrainToBeJoinedBy
        {
            if(TrainToBeJoinedBy->TrainMode != Signaller)
            {
                TrainController->StopTTClockMessage(91, "Adjacent train must be under signaller control in order to join");
                Utilities->CallLogPop(2156);
                return;
            }
            // here if there is an adjacent train under signaller control
            if((TrainToBeJoinedBy->PowerAtRail < 1) && (ThisTrain.PowerAtRail < 1))
            {
                ShowMessage("Can't join two trains when both are without power");
                Utilities->CallLogPop(2157);
                return;
            }
            AnsiString TrainToBeJoinedByHeadCode = TrainToBeJoinedBy->HeadCode;
            // set new values for mass etc
            double OtherBrakeForce = TrainToBeJoinedBy->MaxBrakeRate * TrainToBeJoinedBy->Mass;
            double OwnBrakeForce = ThisTrain.MaxBrakeRate * ThisTrain.Mass;
            double CombinedBrakeRate = (OtherBrakeForce + OwnBrakeForce) / (TrainToBeJoinedBy->Mass + ThisTrain.Mass);
            ThisTrain.Mass += TrainToBeJoinedBy->Mass;
            ThisTrain.MaxBrakeRate = CombinedBrakeRate;
            ThisTrain.PowerAtRail += TrainToBeJoinedBy->PowerAtRail;
            ThisTrain.AValue = sqrt(2 * ThisTrain.PowerAtRail / ThisTrain.Mass);

            TrainToBeJoinedBy->TrainGone = true; // this will cause other train to be deleted
            TrainToBeJoinedBy->JoinedOtherTrainFlag = true;
            AnsiString LocName = "";
            if(ThisTrain.LeadElement > -1)
            {
                LocName = Track->TrackElementAt(979, ThisTrain.LeadElement).ActiveTrackElementName;
            }
            if((LocName == "") && (ThisTrain.MidElement > -1))
            {
                LocName = Track->TrackElementAt(980, ThisTrain.MidElement).ActiveTrackElementName;
            }
            if((LocName == "") && ThisTrain.LeadElement > -1)
            {
                LocName = Track->TrackElementAt(981, ThisTrain.LeadElement).ElementID;
            }
            if((LocName == "") && (ThisTrain.MidElement > -1))
            {
                LocName = Track->TrackElementAt(982, ThisTrain.MidElement).ElementID;
            }
            ThisTrain.StoppedWithoutPower = true;
            if(ThisTrain.PowerAtRail >= 1)
            {
                ThisTrain.StoppedWithoutPower = false;
            }
            ThisTrain.TrainFailed = false; // if had failed then no longer failed, even if joining train has no power
            if(!ThisTrain.StoppedAtLocation)
            {
                // ok to call PlotTrainWithNewBackgroundColour here as PlotElements already set to Lead, Mid & Lag elements
                ThisTrain.PlotTrainWithNewBackgroundColour(49, clSignallerStopped, Display);
            }
            else
            {
                // ok to call PlotTrainWithNewBackgroundColour here as PlotElements already set to Lead, Mid & Lag elements
                ThisTrain.PlotTrainWithNewBackgroundColour(50, clStationStopBackground, Display);
            }
            ThisTrain.SignallerStopped = true; // maybe as well as stopped without power, thought that takes precedence in floating window
            ThisTrain.LogAction(34, ThisTrain.HeadCode, TrainToBeJoinedBy->HeadCode, SignallerJoin, LocName, TDateTime(0), false); // TDateTime isn't used
            ThisTrain.ZeroPowerNoFrontSplitMessage = false; // added at v2.4.0, no need to include TrainToBeJoinedBy as that will be removed
            ThisTrain.ZeroPowerNoRearSplitMessage = false;
            ThisTrain.FailedTrainNoFinishJoinMessage = false;
            ThisTrain.ZeroPowerNoJoinedByMessage = false;
            ThisTrain.ZeroPowerNoCDTMessage = false;
            ThisTrain.ZeroPowerNoNewServiceMessage = false;
            ThisTrain.ZeroPowerNoNewShuttleFromNonRepeatMessage = false;
            ThisTrain.ZeroPowerNoRepeatShuttleMessage = false;
            ThisTrain.ZeroPowerNoRepeatShuttleOrNewServiceMessage = false;
            Utilities->CallLogPop(2158);
        }
    }
    catch(const Exception &e)
    {
        ErrorLog(207, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::RepairFailedTrainMenuItemClick(TObject *Sender)
{ // added at v2.4.0
    try
    {
        TrainController->LogEvent("RepairFailedTrainMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",RepairFailedTrainMenuItemClick");
        TTrain &Train = TrainController->TrainVectorAtIdent(46, SelectedTrainID);
        Train.TrainFailed = false;
        Train.StoppedWithoutPower = false;
        Train.SignallerStopped = true;
        if(!Train.StoppedAtLocation)
        {
            // ok to call PlotTrainWithNewBackgroundColour here as PlotElements already set to Lead, Mid & Lag elements
            Train.PlotTrainWithNewBackgroundColour(51, clSignallerStopped, Display);
        }
        else
        {
            // ok to call PlotTrainWithNewBackgroundColour here as PlotElements already set to Lead, Mid & Lag elements
            Train.PlotTrainWithNewBackgroundColour(52, clStationStopBackground, Display);
        }
        Train.PowerAtRail = Train.OriginalPowerAtRail; // recover from original value, new at v2.4.0
        Train.AValue = sqrt(2 * Train.PowerAtRail / Train.Mass);
        Train.SetTrainMovementValues(24, Train.LeadElement, Train.LeadEntryPos);
        AnsiString LocName = "";
        if(Train.LeadElement > -1)
        {
            LocName = Track->TrackElementAt(983, Train.LeadElement).ActiveTrackElementName;
        }
        if((LocName == "") && (Train.MidElement > -1))
        {
            LocName = Track->TrackElementAt(984, Train.MidElement).ActiveTrackElementName;
        }
        if((LocName == "") && Train.LeadElement > -1)
        {
            LocName = Track->TrackElementAt(985, Train.LeadElement).ElementID;
        }
        if((LocName == "") && (Train.MidElement > -1))
        {
            LocName = Track->TrackElementAt(986, Train.MidElement).ElementID;
        }
        Train.LogAction(35, Train.HeadCode, "", RepairFailedTrain, LocName, TrainController->TTClockTime, false); // false for no warning
        Train.ZeroPowerNoFrontSplitMessage = false;
        Train.ZeroPowerNoRearSplitMessage = false;
        Train.FailedTrainNoFinishJoinMessage = false;
        Train.ZeroPowerNoJoinedByMessage = false;
        Train.ZeroPowerNoCDTMessage = false;
        Train.ZeroPowerNoNewServiceMessage = false;
        Train.ZeroPowerNoNewShuttleFromNonRepeatMessage = false;
        Train.ZeroPowerNoRepeatShuttleMessage = false;
        Train.ZeroPowerNoRepeatShuttleOrNewServiceMessage = false;
        Utilities->CallLogPop(2159);
    }
    catch(const Exception &e)
    {
        ErrorLog(208, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::SignallerControlStopMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("SignallerControlStopMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",SignallerControlStopMenuItemClick");
        TTrain &Train = TrainController->TrainVectorAtIdent(35, SelectedTrainID);
        Train.LeavingUnderSigControlAtContinuation = false;
        if(Train.LeadElement > -1)
        {
            if(Track->TrackElementAt(787, Train.LeadElement).Conn[Train.LeadExitPos] > -1)
            {
                Train.SignallerStoppingFlag = true;
                Train.SignallerStopBrakeRate = 0;
                Train.LogAction(24, Train.HeadCode, "", SignallerControlStop, "", TDateTime(0), false); // TDateTime is a dummy entry, false for no warning
            }
            else
                Train.LeavingUnderSigControlAtContinuation = true;
        }
        else
            Train.LeavingUnderSigControlAtContinuation = true;
        Utilities->CallLogPop(1553);
    }
    catch(const Exception &e)
    {
        ErrorLog(161, e.Message);
    }
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::PassRedSignalMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("PassRedSignalMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",PassRedSignalMenuItemClick");
        TTrain &Train = TrainController->TrainVectorAtIdent(21, SelectedTrainID);
        Train.SignallerStoppingFlag = false;
        int NextElementPos = Track->TrackElementAt(712, Train.LeadElement).Conn[Train.LeadExitPos];
        if(NextElementPos < 0)
        {
            throw Exception("Error, no element in front in PassRedSignalMenuItemClick");
        }
        TTrackElement &TrackElement = Track->TrackElementAt(653, NextElementPos);
/* drop this error as may be some circumstances where behind a signal in sig mode but not stopped at signal
           if(!Train.StoppedAtSignal)
            {
            throw Exception("Error, not StoppedAtSignal in PassRedSignalMenuItemClick");
            }
*/
        if(TrackElement.TrackType != SignalPost)
        {
            throw Exception("Error, next element not a signal type in PassRedSignalMenuItemClick");
        }
        Train.SignallerStopped = false;
        Train.StoppedAtLocation = false; // may have started at station in signaller mode and also at a red signal, in this case both SignallerStopped
                                         // and StoppedAtLocation are set but the background colour stays pale green for station, not signal,
                                         // since no need to alert the user
        Train.StoppedAfterSPAD = false; // in case had been set
        Train.SPADFlag = false;
        // ok to call PlotTrainWithNewBackgroundColour here as PlotElements already set to Lead, Mid & Lag elements
        Train.PlotTrainWithNewBackgroundColour(32, clNormalBackground, Display);
        Train.AllowedToPassRedSignal = true;
        Train.LogAction(4, Train.HeadCode, "", SignallerPassRedSignal, "", TDateTime(0), false); // TDateTime is a dummy entry, false for no warning
        Utilities->CallLogPop(1199);
    }
    catch(const Exception &e)
    {
        ErrorLog(162, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::StepForwardMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("StepForwardMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",StepForwardMenuItemClick");
        TTrain &Train = TrainController->TrainVectorAtIdent(24, SelectedTrainID);
        Train.SignallerStoppingFlag = false;
        Train.SignallerStopped = false;
        Train.StoppedAtLocation = false; // may have started at station in signaller mode and also at a red signal, in this case both SignallerStopped
                                            // and StoppedAtLocation are set but the background colour stays pale green for station, not signal,
                                            // since no need to alert the user
        Train.StoppedAfterSPAD = false; // in case had been set
        Train.SPADFlag = false;
        Train.StepForwardFlag = true;
        Train.AllowedToPassRedSignal = true; // in case at a signal, will clear when half-way into next element whether a signal or not
        // ok to call PlotTrainWithNewBackgroundColour here as PlotElements already set to Lead, Mid & Lag elements
        Train.PlotTrainWithNewBackgroundColour(46, clNormalBackground, Display);
        Train.LogAction(32, Train.HeadCode, "", SignallerStepForward, "", TDateTime(0), false); // TDateTime is a dummy entry, false for no warning
        int NextElementPos = -1;
// addition for v1.3.2 due to Carwyn Thomas error: can't select StepForwardMenuItem if exiting at a continuation but leave this in anyway
        int NextEntryPos = -1; // ---ditto---
        if(Train.LeadElement > -1) // ---ditto---
        { // ---ditto---
            NextElementPos = Track->TrackElementAt(804, Train.LeadElement).Conn[Train.LeadExitPos]; // had 'int' prefix before additions
            NextEntryPos = Track->TrackElementAt(805, Train.LeadElement).ConnLinkPos[Train.LeadExitPos]; // ---ditto---
        } // ---ditto---
        if((NextElementPos > -1) && (NextEntryPos > -1))
        { // call this after StepForwardFlag set
            Train.EntrySpeed = 0;
            Train.EntryTime = TrainController->TTClockTime;
            Train.FirstHalfMove = true;
            Train.SetTrainMovementValues(20, NextElementPos, NextEntryPos); // NextElement is the element to be entered
        }
        Utilities->CallLogPop(1800);
    }
    catch(const Exception &e)
    {
        ErrorLog(163, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::RemoveTrainMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("RemoveTrainMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",RemoveTrainMenuItemClick");
        TTrain &Train = TrainController->TrainVectorAtIdent(22, SelectedTrainID);
        if((!Train.Derailed) && (!Train.Crashed))
        {
            TrainController->StopTTClockFlag = true; // so TTClock stopped during MasterClockTimer function
            TrainController->RestartTime = TrainController->TTClockTime;
            UnicodeString Message = UnicodeString(Train.HeadCode) + " will be removed from the railway - proceed?";
            int button = Application->MessageBox(Message.c_str(), L"Please confirm", MB_YESNO);
            TrainController->BaseTime = TDateTime::CurrentDateTime();
            TrainController->StopTTClockFlag = false;
            if(button == IDNO)
            {
                Utilities->CallLogPop(1801);
                return;
            }
        }
        Train.SignallerStoppingFlag = false;
        Train.TrainGone = true; // will be removed by TTrainController::Operate
        Train.SignallerRemoved = true;
        Train.TrainDataEntryPtr->TrainOperatingDataVector.at(Train.RepeatNumber).RunningEntry = Exited;
        AnsiString LocName = "";
        if(Train.LeadElement > -1)
        {
            LocName = Track->TrackElementAt(641, Train.LeadElement).ActiveTrackElementName;
        }
        if((LocName == "") && (Train.MidElement > -1))
        {
            LocName = Track->TrackElementAt(642, Train.MidElement).ActiveTrackElementName;
        }
        if((LocName == "") && Train.LeadElement > -1)
        {
            LocName = Track->TrackElementAt(643, Train.LeadElement).ElementID;
        }
        if((LocName == "") && (Train.MidElement > -1))
        {
            LocName = Track->TrackElementAt(644, Train.MidElement).ElementID;
        }
        TTrackElement *TrackElementPtr;
        int RouteNumber;
        TAllRoutes::TRouteType RouteType;
        if(Train.LeadElement > -1)
        {
            TrackElementPtr = &(Track->TrackElementAt(673, Train.LeadElement));
            // remove TrainIDs from track element, added at v2.4.0
            if(TrackElementPtr->TrackType == Bridge)
            {
                if(Train.LeadExitPos > 1)
                {
                    TrackElementPtr->TrainIDOnBridgeTrackPos23 = -1;
                }
                else
                {
                    TrackElementPtr->TrainIDOnBridgeTrackPos01 = -1;
                }
            }
            else
            {
                TrackElementPtr->TrainIDOnElement = -1;
            }
            // reset any CallingOnSet flags for signals, if facing wrong way doesn't matter, shouldn't be set anyay
            if((TrackElementPtr->TrackType == SignalPost) && TrackElementPtr->CallingOnSet)
            {
                TrackElementPtr->CallingOnSet = false;
                Track->PlotSignal(6, *TrackElementPtr, Display);
            }
// [added at v1.3.0] here check if on an automatic signals route and if so reset signals for the entire route from the
// start of the route - after the train has been removed, use LeadElement and also MidElement (if no autosigs route at LeadElement) just to be sure
            RouteType = AllRoutes->GetRouteTypeAndNumber(27, Train.LeadElement, Train.LeadEntryPos, RouteNumber);
            if(RouteType == TAllRoutes::AutoSigsRoute)
            {
                AllRoutes->SignallerRemovedTrainAutoRoute = AllRoutes->GetFixedRouteAt(215, RouteNumber);
                TrainController->SignallerTrainRemovedOnAutoSigsRoute = true;
            }
// end of addition
        }
        if(Train.MidElement > -1)
        {
            TrackElementPtr = &(Track->TrackElementAt(674, Train.MidElement));
            // remove TrainIDs from track element, added at v2.4.0
            if(TrackElementPtr->TrackType == Bridge)
            {
                if(Train.MidExitPos > 1)
                {
                    TrackElementPtr->TrainIDOnBridgeTrackPos23 = -1;
                }
                else
                {
                    TrackElementPtr->TrainIDOnBridgeTrackPos01 = -1;
                }
            }
            else
            {
                TrackElementPtr->TrainIDOnElement = -1;
            }
            if((TrackElementPtr->TrackType == SignalPost) && TrackElementPtr->CallingOnSet)
            {
                TrackElementPtr->CallingOnSet = false;
                Track->PlotSignal(7, *TrackElementPtr, Display);
            }
// [added at v1.3.0 as above]
            if(!TrainController->SignallerTrainRemovedOnAutoSigsRoute)
            {
                RouteType = AllRoutes->GetRouteTypeAndNumber(28, Train.MidElement, Train.MidEntryPos, RouteNumber);
                if(RouteType == TAllRoutes::AutoSigsRoute)
                {
                    AllRoutes->SignallerRemovedTrainAutoRoute = AllRoutes->GetFixedRouteAt(216, RouteNumber);
                    TrainController->SignallerTrainRemovedOnAutoSigsRoute = true;
                }
            }
// end of addition
        }
        if(Train.LeadElement > -1)
    // addition for v1.3.2 after Carwyn Thomas fault reported 24/05/15 - need to check if exiting at continuation (LeadElement == -1) as if so fails at next line
        {
            if(Track->TrackElementAt(675, Train.LeadElement).Conn[Train.LeadExitPos] > -1)
            {
                TrackElementPtr = &(Track->TrackElementAt(676, Track->TrackElementAt(677, Train.LeadElement).Conn[Train.LeadExitPos]));
                if((TrackElementPtr->TrackType == SignalPost) && TrackElementPtr->CallingOnSet)
                {
                    TrackElementPtr->CallingOnSet = false;
                    Track->PlotSignal(8, *TrackElementPtr, Display);
                }
            }
        }
        Train.LogAction(5, Train.HeadCode, "", RemoveTrain, LocName, TDateTime(0), false); // TDateTime is a dummy entry, false for no warning
        if(Train.ActionVectorEntryPtr->Command != "Frh") // if remaining at location no point in sending 'failed to terminate' message
        {
            Train.SendMissedActionLogs(0, -1, Train.ActionVectorEntryPtr); // -1 is a marker for send messages for all remaining
        } // entries, including Fer if present
        Utilities->CallLogPop(1200);
    }
    catch(const Exception &e)
    {
        ErrorLog(164, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::ErrorButtonClick(TObject *Sender)
    // to terminate after error message given
{
    ErrorMessage->Visible = false;
    ErrorButton->Visible = false;
    Display->GetImage()->Visible = true;
    Application->Terminate();
}

// ---------------------------------------------------------------------------
void __fastcall TInterface::PerformancePanelLabelStartDrag(TObject *Sender, TDragObject *&DragObject)
{
    try
    {
        TrainController->LogEvent("PerformancePanelLabelStartDrag");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",PerformancePanelLabelStartDrag");
        PerformancePanelDragStartX = Mouse->CursorPos.x - PerformancePanel->Left;
        PerformancePanelDragStartY = Mouse->CursorPos.y - PerformancePanel->Top;
        Utilities->CallLogPop(1202);
    }
    catch(const Exception &e)
    {
        ErrorLog(165, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::FormClose(TObject *Sender, TCloseAction &Action)
{
    try
    {
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",FormClose");
        if(!FileChangedFlag && !(Track->IsTrackFinished()) && (EveryPrefDir->PrefDirSize() > 0))
        {
            UnicodeString MessageStr =
                "Note that leaving the track unlinked will cause preferred directions to be lost on reloading.  Prevent by linking the track then resaving.  Do you still wish to exit?";
            int button = Application->MessageBox(MessageStr.c_str(), L"Please confirm", MB_YESNO);
            if(button == IDNO)
            {
                Action = caNone; // prevents form & application from closing
                Utilities->CallLogPop(1712);
                return;
            }
        }
        if(FileChangedFlag || TimetableChangedFlag)
        {
            UnicodeString MessStr = "";
            if(FileChangedFlag && TimetableChangedFlag)
            {
                MessStr = UnicodeString("The railway and the timetable have both changed, exit without saving either?");
            }
            else if(FileChangedFlag)
            {
                MessStr = UnicodeString("The railway has changed, exit without saving?");
            }
            else
            {
                MessStr = UnicodeString("The timetable has changed, exit without saving?");
            }
            int button = Application->MessageBox(MessStr.c_str(), L"Please confirm", MB_YESNO);
            if(button == IDNO)
            {
                Action = caNone; // prevents form & application from closing
                Utilities->CallLogPop(1133);
                return;
            }
        }

        if(Level1Mode == OperMode)
        {
            UnicodeString MessageStr = "Please note that the session will be lost if it hasn't been saved.  Do you still wish to exit?";
            TrainController->StopTTClockFlag = true; // so TTClock stopped during MasterClockTimer function
            TrainController->RestartTime = TrainController->TTClockTime;
            int button = Application->MessageBox(MessageStr.c_str(), L"Please confirm", MB_YESNO);
            TrainController->BaseTime = TDateTime::CurrentDateTime();
            TrainController->StopTTClockFlag = false;
            if(button == IDNO)
            {
                Action = caNone; // prevents form & application from closing
                Utilities->CallLogPop(969);
                return;
            }
            TrainController->SendPerformanceSummary(1, Utilities->PerformanceFile);
            Utilities->PerformanceFile.close();
        }
        if((TempTTFileName != "") && FileExists(TempTTFileName))
        {
            DeleteFile(TempTTFileName);
        }
        Utilities->CallLogPop(971);
    }
    catch(const Exception &e)
    {
        ErrorLog(166, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
// TrainController->LogEvent("FormKeyDown," + AnsiString(Key));
// drop event log as have too many spurious entries
    try
    {
        if((Shift.Contains(ssAlt)) && (Shift.Contains(ssCtrl)))
        {
            if(Key == '2')
            {
                if(CallLogTickerLabel->Visible)
                {
                    CallLogTickerLabel->Visible = false;
                }
                else
                {
                    CallLogTickerLabel->Visible = true;
                }
            }
            else if(Key == '3')
            {
                if(DevelopmentPanel->Visible)
                {
                    DevelopmentPanel->Visible = false;
                }
                else
                {
                    DevelopmentPanel->Visible = true;
                    DevelopmentPanel->BringToFront();
                }
            }
            else if(Key == '4')
            {
                TestFunction();
            }
        }
        else if(Shift.Contains(ssCtrl) && !Shift.Contains(ssShift) && !Shift.Contains(ssAlt))
        {
            CtrlKey = true;
        }
        else if(Shift.Contains(ssShift) && !Shift.Contains(ssCtrl) && !Shift.Contains(ssAlt))
        {
            ShiftKey = true;
        }

// below added at v1.3.0 to allow keyboard scrolling as well as mouse button scrolling - see user suggestion on Features & Requests forum 30/09/12
// the NonCTRLOrSHIFTKeyUpFlag prevents repeated viewpoint movements without keys being re-pressed
// note that use the OnKeyDown event rather than OnKeyPress as suggested by the user so that the CTRL & SHIFT keys can be taken into account

//at v2.4.2 the flag changed to LastNonCtrlOrShiftKeyDown to make condition specific to last key, because when a message given the key up event
//is not seen as the form does not have focus, so with the flag no shortcut key will work on the first press, with this only the same shortcut key
//won't work on first press and that is less likely to be used a second time on either side of the message

        if((Key != VK_SHIFT) && (Key != VK_CONTROL))
        {
            if(LastNonCtrlOrShiftKeyDown == Key)  //same key still down rejected
            {
                return;
            }
            else
            {
                LastNonCtrlOrShiftKeyDown = Key;
            }
        }

        if(Key == VK_UP)
        {
            if(ScreenUpButton->Enabled)
                ScreenUpButton->Click();
        }
        else if(Key == VK_DOWN)
        {
            if(ScreenDownButton->Enabled)
                ScreenDownButton->Click();
        }
        else if(Key == VK_LEFT)
        {
            if(ScreenLeftButton->Enabled)
                ScreenLeftButton->Click();
        }
        else if(Key == VK_RIGHT)
        {
            if(ScreenRightButton->Enabled)
                ScreenRightButton->Click();
        }
        else if(Key == VK_HOME)
        {
            if(HomeButton->Enabled)
                HomeButton->Click();
        }
// end of 1.3.0 addition
        else if(Key == VK_END) // added at v2.2.0 to toggle zoom using 'End' key
        {
            if(ZoomButton->Enabled)
                ZoomButton->Click();
        }
        else if(Key == VK_END) // added at v2.2.0 to toggle zoom using 'End' key
        {
            if(ZoomButton->Enabled)
                ZoomButton->Click();
        }

//below added for v2.4.2 to add more keyboard shortcuts
        if(DistanceBox->Focused() || SpeedLimitBox->Focused() || MileEdit->Focused() || ChainEdit->Focused() || YardEdit->Focused() || SpeedEditBox2->Focused()
           || MTBFEditBox->Focused() || LocationNameTextBox->Focused() || TextBox->Focused() || PowerEditBox->Focused() || SpeedEditBox->Focused()
           || AddSubMinsBox->Focused() || OneEntryTimetableMemo->Focused())
        {// prevent letter keys interfering when these boxes have focus - many are mutually exclusive but include them all
            return;
        }

        if(Shift.Contains(ssShift) && !Shift.Contains(ssAlt) && !Shift.Contains(ssCtrl) && NewHomeButton->Enabled && NewHomeButton->Visible)
        {
            if(Level1Mode != TimetableMode && (Key == 'H' || Key == 'h'))//TimetablePanel uses Shift H too so disable this when it's in use
            {
                NewHomeButton->Click();
            }
        }

//Operating panel
        if(Level1Mode == OperMode && OperatingPanel->Enabled && OperatingPanel->Visible && !Shift.Contains(ssShift) && !Shift.Contains(ssAlt))
        {  //use Shift.Contains(ssShift etc instead of ShiftKey as that not set if pressed second after Ctrl key pressed
            if(!Shift.Contains(ssCtrl))
            {
                if(OperateButton->Visible && OperateButton->Enabled)
                {
                    if(Level2OperMode == Operating && (Key == 'P' || Key == 'p'))
                    {
                        OperateButton->Click();
                    }
                    else if((Level2OperMode == Paused || Level2OperMode == PreStart) && (Key == 'R' || Key == 'r'))
                    {
                        OperateButton->Click();
                    }
                }
                if(PresetAutoSigRoutesButton->Visible && PresetAutoSigRoutesButton->Enabled && (Key == 'A' || Key == 'a'))
                {
                    PresetAutoSigRoutesButton->Click();
                }
                if(PerformanceLogButton->Visible && PerformanceLogButton->Enabled && (Key == 'L' || Key == 'l'))
                {
                    PerformanceLogButton->Click();
                }
                if(CallingOnButton->Visible && CallingOnButton->Enabled && (Key == 'O' || Key == 'o'))
                {
                    CallingOnButton->Click();
                }
                if(OperatorActionButton->Visible && OperatorActionButton->Enabled && (Key == 'D' || Key == 'd'))
                {
                    OperatorActionButton->Click();
                }
                if(RouteCancelButton->Visible && RouteCancelButton->Enabled && (Key == 'C' || Key == 'c'))
                {
                    RouteCancelButton->Click();
                }
                if(TTClockAdjButton->Visible && TTClockAdjButton->Enabled && (Key == 'T' || Key == 't'))
                {
                    TTClockAdjButton->Click();
                }
                if(AutoSigsButton->Visible && AutoSigsButton->Enabled && Key == '1') //route buttons - autosigs
                {
                    AutoSigsButton->Click();
                }
                if(SigPrefButton->Visible && SigPrefButton->Enabled && Key == '2') //route buttons - prefdir
                {
                    SigPrefButton->Click();
                }
                if(UnrestrictedButton->Visible && UnrestrictedButton->Enabled && Key == '3') //route buttons - unrestricted
                {
                    UnrestrictedButton->Click();
                }
                if(ExitOperationButton->Visible && ExitOperationButton->Enabled && Key == '\x1b')
                {
                    ExitOperationButton->Click();
                }
            }
            else  //CtrlKey down
            {
                if(SaveSessionButton->Visible && SaveSessionButton->Enabled)
                {
                    SaveMenuItem->ShortCut = 0; //It's normally 16467 (Ctrl S) & will be restored in FormKeyUp. Has to be reset or will grab the key combination
                    if(Key == 'S' || Key == 's')//so this will never execute
                    {
                        SaveSessionButton->Click();
                    }
                }
            }
        }

//Timetable clock adjust panel
        if(Level1Mode == OperMode && TTClockAdjPanel->Enabled && TTClockAdjPanel->Visible && !Shift.Contains(ssCtrl) && !Shift.Contains(ssAlt))
        {  //use Shift.Contains(ssShift etc instead of ShiftKey as that not set if pressed second after Ctrl key pressed
            if(Shift.Contains(ssShift))
            {
                if(TTClockExitButton->Visible && TTClockExitButton->Enabled && (Key == 'A' || Key == 'a'))
                {
                    TTClockExitButton->Click();
                }
                if(TTClockResetButton->Visible && TTClockResetButton->Enabled && (Key == 'R' || Key == 'r'))
                {
                    TTClockResetButton->Click();
                }
            }
        }

//Track build panel
        if((Level1Mode == TrackMode) && TrackBuildPanel->Visible && TrackBuildPanel->Enabled)
        {
            if(Shift.Contains(ssShift) && !Shift.Contains(ssCtrl) && !Shift.Contains(ssAlt))
            {
                if(AddTrackButton->Visible && AddTrackButton->Enabled && (Key == 'A' || Key == 'a')) //add/remove track elements
                {
                    AddTrackButton->Click();
                }
                if(SigAspectButton->Visible && SigAspectButton->Enabled && (Key == 'S' || Key == 's')) //cycle through signal aspects
                {
                    SigAspectButton->Click();
                }
                if(TrackOKButton->Visible && TrackOKButton->Enabled && (Key == 'L' || Key == 'l')) //link track
                {
                    TrackOKButton->Click();
                }
                if(FontButton->Visible && FontButton->Enabled && (Key == 'F' || Key == 'f')) //change font
                {
                    FontButton->Click();
                }
                if(LocationNameButton->Visible && LocationNameButton->Enabled && (Key == 'N' || Key == 'n')) //name locations
                {
                    LocationNameButton->Click();
                }
                if(SetLengthsButton->Visible && SetLengthsButton->Enabled && (Key == 'D' || Key == 'd')) //set distances/speeds
                {
                    SetLengthsButton->Click();
                }
                if(AddTextButton->Visible && AddTextButton->Enabled && (Key == 'T' || Key == 't')) //add text
                {
                    AddTextButton->Click();
                }
                if(ScreenGridButton->Visible && ScreenGridButton->Enabled && (Key == 'G' || Key == 'g')) //toggle grid
                {
                    ScreenGridButton->Click();
                }
                if(MoveTextOrGraphicButton->Visible && MoveTextOrGraphicButton->Enabled && (Key == 'M' || Key == 'm')) //move text or graphic
                {
                    MoveTextOrGraphicButton->Click();
                }
                if(UserGraphicButton->Visible && UserGraphicButton->Enabled && (Key == 'I' || Key == 'i')) //insert image
                {
                    UserGraphicButton->Click();
                }
                if(SetGapsButton->Visible && SetGapsButton->Enabled && (Key == 'J' || Key == 'j')) //join gaps
                {
                    SetGapsButton->Click();
                }
            }
            if(Shift.Contains(ssCtrl) && !Shift.Contains(ssShift) && !Shift.Contains(ssAlt))
            {
                if(SaveRailwayTBPButton->Visible && SaveRailwayTBPButton->Enabled) //save railway in trackbuild mode
                {
                    SaveMenuItem->ShortCut = 0; //It's normally 16467 (Ctrl S) & will be restored in FormKeyUp. Has to be reset or will grab the key combination
                    if(Key == 'S' || Key == 's')//so this will never execute
                    {
                        SaveRailwayTBPButton->Click();
                    }
                }
            }
            if(!Shift.Contains(ssCtrl) && !Shift.Contains(ssShift) && !Shift.Contains(ssAlt))
            {
                if((ExitTrackButton->Visible && ExitTrackButton->Enabled) && Key == '\x1b') //escape key
                {
                    ExitTrackButton->Click();
                }
            }
        }

//PrefDir panel
        if(Level1Mode == PrefDirMode && PrefDirPanel->Visible && PrefDirPanel->Enabled && !Shift.Contains(ssAlt))
        {
            if(!Shift.Contains(ssShift) && !Shift.Contains(ssCtrl))
            {
                if((ExitPrefDirButton->Visible && ExitPrefDirButton->Enabled) && Key == '\x1b') //escape key
                {
                    ExitPrefDirButton->Click();
                }
            }
            if(!Shift.Contains(ssShift) && Shift.Contains(ssCtrl))
            {
                if(SaveRailwayPDPButton->Visible && SaveRailwayPDPButton->Enabled)
                {
                    SaveMenuItem->ShortCut = 0; //It's normally 16467 (Ctrl S) & will be restored in FormKeyUp. Has to be reset or will grab the key combination
                    if(Key == 'S' || Key == 's')//so this will never execute
                    {
                        SaveRailwayPDPButton->Click();
                    }
                }
            }
            if(Shift.Contains(ssShift) && !Shift.Contains(ssCtrl))
            {
                if(AddPrefDirButton->Visible && AddPrefDirButton->Enabled && (Key == 'A' || Key == 'a')) //add pref dir
                {
                    AddPrefDirButton->Click();
                }
                if(DeleteOnePrefDirButton->Visible && DeleteOnePrefDirButton->Enabled && (Key == 'D' || Key == 'd')) //delete one pref dir
                {
                    DeleteOnePrefDirButton->Click();
                }
                if(DeleteAllPrefDirButton->Visible && DeleteAllPrefDirButton->Enabled && (Key == 'C' || Key == 'c')) //delete all pref dirs
                {
                    DeleteAllPrefDirButton->Click();
                }
            }
        }
//Note that save button in BaseMode is handled by Ctrl S from the File menu

//Timetable panel
        if(Level1Mode == TimetableMode && TimetablePanel->Visible && TimetablePanel->Enabled && !Shift.Contains(ssAlt))
        {
            if(!Shift.Contains(ssShift) && !Shift.Contains(ssCtrl))
            {
                if(ExitTTModeButton->Visible && ExitTTModeButton->Enabled && Key == '\x1b') //escape key
                {
                    ExitTTModeButton->Click();
                }
            }
            if(Shift.Contains(ssShift) && !Shift.Contains(ssCtrl)) //show/hide timetable edit panel
            {
                if(ShowHideTTButton->Visible && ShowHideTTButton->Enabled)
                {
                    if(!TimetableEditPanel->Visible)
                    {
                        if(Key == 'S' || Key == 's')
                        {
                            ShowHideTTButton->Click();
                        }
                    }
                    else if(Key == 'H' || Key == 'h')
                    {
                        ShowHideTTButton->Click();
                    }
                }
            }
        }

//Timetable edit panel
        if(Level1Mode == TimetableMode && TimetableEditPanel->Visible && TimetableEditPanel->Enabled && !Shift.Contains(ssAlt))
        {
            if(Shift.Contains(ssShift) && !Shift.Contains(ssCtrl))
            {
                if(PreviousTTEntryButton->Enabled && (Key == 'L' || Key == 'l'))
                {
                    PreviousTTEntryButton->Click();
                }
                if(NextTTEntryButton->Enabled && (Key == 'N' || Key == 'n'))
                {
                    NextTTEntryButton->Click();
                }
                if(MoveTTEntryUpButton->Enabled && (Key == 'U' || Key == 'u'))
                {
                    MoveTTEntryUpButton->Click();
                }
                if(MoveTTEntryDownButton->Enabled && (Key == 'D' || Key == 'd'))
                {
                    MoveTTEntryDownButton->Click();
                }
                if(CopyTTEntryButton->Enabled && (Key == 'C' || Key == 'c'))
                {
                    CopyTTEntryButton->Click();
                }
                if(CutTTEntryButton->Enabled && (Key == 'X' || Key == 'x'))
                {
                    CutTTEntryButton->Click();
                }
                if(PasteTTEntryButton->Enabled && (Key == 'P' || Key == 'p'))
                {
                    PasteTTEntryButton->Click();
                }
                if(DeleteTTEntryButton->Enabled && (Key == 'E' || Key == 'e'))
                {
                    DeleteTTEntryButton->Click();
                }
/*              if(SaveTTEntryButton->Enabled && (Key == 'E' || Key == 'e'))  //can't have save while editing entry as adds the letter to the entry
                {
                    SaveTTEntryButton->Click();
                }
              if(CancelTTActionButton->Enabled && (Key == 'K' || Key == 'k')) //can't have cancel while editing entry as adds the letter to the entry
                {
                    CancelTTActionButton->Click();
                }
*/
                if(NewTTEntryButton->Enabled && (Key == 'I' || Key == 'i'))
                {
                    NewTTEntryButton->Click();
                }
                if(AZOrderButton->Enabled && (Key == 'Z' || Key == 'z'))
                {
                    AZOrderButton->Click();
                }
                if(AddMinsButton->Enabled && (Key == 'M' || Key == 'm'))
                {
                    AddMinsButton->Click();
                }
                if(SubMinsButton->Enabled && (Key == 'B' || Key == 'b'))
                {
                    SubMinsButton->Click();
                }
                if(TTServiceSyntaxCheckButton->Enabled && (Key == 'Q' || Key == 'q'))
                {
                    TTServiceSyntaxCheckButton->Click();
                }
                if(ValidateTimetableButton->Enabled && (Key == 'V' || Key == 'v'))
                {
                    ValidateTimetableButton->Click();
                }
                if(SaveTTButton->Enabled && (Key == 'T' || Key == 't'))
                {
                    SaveTTButton->Click();
                }
                if(SaveTTAsButton->Enabled && (Key == 'A' || Key == 'a'))
                {
                    SaveTTAsButton->Click();
                }
                if(RestoreTTButton->Enabled && (Key == 'R' || Key == 'r'))
                {
                    RestoreTTButton->Click();
                }
                if(ExportTTButton->Enabled && (Key == 'O' || Key == 'o'))
                {
                    ExportTTButton->Click();
                }
            }
        }


//Information menu
        if(FloatingInfoMenu->Enabled && !Shift.Contains(ssAlt) && Shift.Contains(ssCtrl) && Shift.Contains(ssShift))
        {
            if(Key == 'I' || Key == 'i')   //toggle track info
            {
                TrackInfoOnOffMenuItem->Click();
            }
            else if(TrainInfoMenuItem->Enabled)
            {
                if(Key == 'S' || Key == 's') //toggle train status info
                {
                    TrainStatusInfoOnOffMenuItem->Click();
                }
                else if(Key == 'T' || Key == 't') //toggle train timetable info
                {
                    TrainTTInfoOnOffMenuItem->Click();
                }

            }
        }
//end of 2.4.2 addition

    }
    catch(const Exception &e)
    {
        ErrorLog(167, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::FormKeyUp(TObject *Sender, WORD &Key, TShiftState Shift)
{
    if((Key != VK_SHIFT) && (Key != VK_CONTROL))
        LastNonCtrlOrShiftKeyDown = -1; //reset value to no key down
    CtrlKey = false;
    ShiftKey = false;
    SaveMenuItem->ShortCut = 16467; //restore Ctrl S for save menu in case set to 0 in FormKeyDown
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::OutputLog1MouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if((Button == mbRight) && Level2OperMode == Operating)
    {
        OutputLog1->Caption = "";
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::OutputLog2MouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if((Button == mbRight) && Level2OperMode == Operating)
    {
        OutputLog2->Caption = "";
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::OutputLog3MouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if((Button == mbRight) && Level2OperMode == Operating)
    {
        OutputLog3->Caption = "";
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::OutputLog4MouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if((Button == mbRight) && Level2OperMode == Operating)
    {
        OutputLog4->Caption = "";
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::OutputLog5MouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if((Button == mbRight) && Level2OperMode == Operating)
    {
        OutputLog5->Caption = "";
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::OutputLog6MouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if((Button == mbRight) && Level2OperMode == Operating)
    {
        OutputLog6->Caption = "";
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::OutputLog7MouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if((Button == mbRight) && Level2OperMode == Operating)
    {
        OutputLog7->Caption = "";
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::OutputLog8MouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if((Button == mbRight) && Level2OperMode == Operating)
    {
        OutputLog8->Caption = "";
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::OutputLog9MouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if((Button == mbRight) && Level2OperMode == Operating)
    {
        OutputLog9->Caption = "";
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::OutputLog10MouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if((Button == mbRight) && Level2OperMode == Operating)
    {
        OutputLog10->Caption = "";
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::AboutMenuItemClick(TObject *Sender)
{
    try
    {
        if((Level1Mode == OperMode) && (Level2OperMode != PreStart))
    // if PreStart leave as is [Modified at v1.2.0 - formerly just 'if((Level1Mode == OperMode)']
        {
            Level2OperMode = Paused;
            SetLevel2OperMode(3);
            MasterClock->Enabled = false;
        }
        AboutForm->ShowModal();
    }
    catch(const Exception &e)
    {
        ErrorLog(168, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::OpenHelpMenuItemClick(TObject *Sender)
{
    try
    {
        // Helpfile allocated during construction of Interface
        Application->HelpKeyword(u"Introduction"); // added at v2.0.0 for .chm help file
    }
    catch(const Exception &e)
    {
        ErrorLog(175, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::RailwayWebSiteMenuItemClick(TObject *Sender)
{
    const UnicodeString Link = "http://www.railwayoperationsimulator.com";
    ::ShellExecute(Handle, NULL, (Link).c_str(), NULL, NULL, SW_SHOWNORMAL);
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::BlackBgndMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("BlackBgndMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",BlackBgndMenuItemClick");
        std::ofstream ColFile((CurDir + "\\Background.col").c_str());
        if(ColFile.fail())
        {
            // ShowMessage("Failed to store colour file, program will default to a black background when next loaded");
            // no need for message as will revert to black by default
        }
        else
        {
            Utilities->SaveFileString(ColFile, "black");
            ColFile.close(); // added at v2.3.0, should have been in earlier
        }
        TColor OldTransparentColour = Utilities->clTransparent;
        Utilities->clTransparent = TColor(0);
        SelectBitmap->TransparentColor = Utilities->clTransparent;
        RailGraphics->ChangeAllTransparentColours(Utilities->clTransparent, OldTransparentColour);
        TextBox->Color = clB3G3R3;
        RailGraphics->SetUpAllDerivitiveGraphics(Utilities->clTransparent);

        MainScreen->Canvas->Brush->Color = Utilities->clTransparent;
        MainScreen->Canvas->FillRect(MainScreen->ClientRect);
        Level1Mode = BaseMode;
        SetLevel1Mode(128);
        Utilities->CallLogPop(1797);
    }
    catch(const Exception &e)
    {
        ErrorLog(170, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::WhiteBgndMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("WhiteBgndMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",WhiteBgndMenuItemClick");
        std::ofstream ColFile((CurDir + "\\Background.col").c_str());
        if(ColFile.fail())
        {
            ShowMessage("Failed to store colour file, program will default to a black background when next loaded");
        }
        else
        {
            Utilities->SaveFileString(ColFile, "white");
            ColFile.close(); // added at v2.3.0, should have been in earlier
        }
        TColor OldTransparentColour = Utilities->clTransparent;
        Utilities->clTransparent = TColor(0xFFFFFF);
        SelectBitmap->TransparentColor = Utilities->clTransparent;
        RailGraphics->ChangeAllTransparentColours(Utilities->clTransparent, OldTransparentColour);
        TextBox->Color = clB5G5R5;
        RailGraphics->SetUpAllDerivitiveGraphics(Utilities->clTransparent);

        MainScreen->Canvas->Brush->Color = Utilities->clTransparent;
        MainScreen->Canvas->FillRect(MainScreen->ClientRect);
        Level1Mode = BaseMode;
        SetLevel1Mode(129);
        Utilities->CallLogPop(1798);
    }
    catch(const Exception &e)
    {
        ErrorLog(171, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::BlueBgndMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("BlueBgndMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",BlueBgndMenuItemClick");
        std::ofstream ColFile((CurDir + "\\Background.col").c_str());
        if(ColFile.fail())
        {
            ShowMessage("Failed to store colour file, program will default to a black background when next loaded");
        }
        else
        {
            Utilities->SaveFileString(ColFile, "blue");
            ColFile.close(); // added at v2.3.0, should have been in earlier
        }
        TColor OldTransparentColour = Utilities->clTransparent;
        Utilities->clTransparent = TColor(0x330000);
        SelectBitmap->TransparentColor = Utilities->clTransparent;
        RailGraphics->ChangeAllTransparentColours(Utilities->clTransparent, OldTransparentColour);
        TextBox->Color = clB3G3R3;
        RailGraphics->SetUpAllDerivitiveGraphics(Utilities->clTransparent);

        MainScreen->Canvas->Brush->Color = Utilities->clTransparent;
        MainScreen->Canvas->FillRect(MainScreen->ClientRect);
        Level1Mode = BaseMode;
        SetLevel1Mode(130);
        Utilities->CallLogPop(1799);
    }
    catch(const Exception &e)
    {
        ErrorLog(172, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::SpeedToggleButtonClick(TObject *Sender)
{
    if(SpeedTopLabel->Caption == "mph")
    {
        SpeedTopLabel->Caption = "km/h";
        SpeedBottomLabel->Caption = "mph";
    }
    else
    {
        SpeedTopLabel->Caption = "mph";
        SpeedBottomLabel->Caption = "km/h";
    }
    // swap values to match toggle state
    UnicodeString SavedTopValue = SpeedEditBox->Text;
    UnicodeString SavedBottomValue = SpeedVariableLabel->Caption;

    SpeedEditBox->Text = SavedBottomValue;
    SpeedVariableLabel->Caption = SavedTopValue;
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::SpeedToggleButton2Click(TObject *Sender)
{
    if(SpeedTopLabel2->Caption == "mph")
    {
        SpeedTopLabel2->Caption = "km/h";
        SpeedBottomLabel2->Caption = "mph";
    }
    else
    {
        SpeedTopLabel2->Caption = "mph";
        SpeedBottomLabel2->Caption = "km/h";
    }
    // swap values to match toggle state
    UnicodeString SavedTopValue = SpeedEditBox2->Text;
    UnicodeString SavedBottomValue = SpeedVariableLabel2->Caption;

    SpeedEditBox2->Text = SavedBottomValue;
    SpeedVariableLabel2->Caption = SavedTopValue;
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::SpeedEditBoxKeyUp(TObject *Sender, WORD &Key, TShiftState Shift)
{
    try
    {
        TrainController->LogEvent("SpeedEditBoxKeyUp," + AnsiString(Key));
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",SpeedEditBoxKeyUp," + AnsiString(Key));
        bool ErrorFlag = false, TooBigFlag = false;
        if(SpeedEditBox->Text.Length() > 0)
        {
            if(SpeedEditBox->Text.Length() > 5)
            {
                TooBigFlag = true;
            }
            for(int x = 1; x <= SpeedEditBox->Text.Length(); x++)
            {
                if((SpeedEditBox->Text[x] < '0') || (SpeedEditBox->Text[x] > '9'))
                {
                    SpeedVariableLabel->Caption = "Entry error";
                    ErrorFlag = true;
                    break;
                }
                if(TooBigFlag)
                {
                    SpeedVariableLabel->Caption = "Too big";
                    break;
                }
            }
            if(!ErrorFlag && !TooBigFlag)
            {
/*
                  1 mph =  1.609344 km/h
                  1 km/h = 0.621371 mph
*/
                if(SpeedTopLabel->Caption == "mph")
                {
                    // do mph-to-km/h conversion
                    int MPH = SpeedEditBox->Text.ToInt();
                    int KPH = (MPH * 1.609344) + 0.5;
                    SpeedVariableLabel->Caption = UnicodeString(KPH);
                }
                else
                {
                    // do km/h-to-mph conversion
                    int KPH = SpeedEditBox->Text.ToInt();
                    int MPH = (KPH * 0.621371) + 0.5;
                    SpeedVariableLabel->Caption = UnicodeString(MPH);
                }
            }
        }
        else
        {
            SpeedVariableLabel->Caption = "";
        }
        Utilities->CallLogPop(1865);
    }
    catch(const EConvertError &ec) // thrown for ToInt() conversion error; shouldn't occur but include to prevent a crash
    {
        SpeedVariableLabel->Caption = "Entry error";
    }
    catch(const Exception &e)
    {
        ErrorLog(176, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::PowerToggleButtonClick(TObject *Sender)
{
    if(PowerTopLabel->Caption == "HP")
    {
        PowerTopLabel->Caption = "kW";
        PowerBottomLabel->Caption = "HP";
    }
    else
    {
        PowerTopLabel->Caption = "HP";
        PowerBottomLabel->Caption = "kW";
    }
    // swap values to match toggle state
    UnicodeString SavedTopValue = PowerEditBox->Text;
    UnicodeString SavedBottomValue = PowerVariableLabel->Caption;

    PowerEditBox->Text = SavedBottomValue;
    PowerVariableLabel->Caption = SavedTopValue;
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::PowerEditBoxKeyUp(TObject *Sender, WORD &Key, TShiftState Shift)
{
    try
    {
        TrainController->LogEvent("PowerEditBoxKeyUp," + AnsiString(Key));
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",PowerEditBoxKeyUp," + AnsiString(Key));
        bool ErrorFlag = false, TooBigFlag = false;
        if(PowerEditBox->Text.Length() > 0)
        {
            if(PowerEditBox->Text.Length() > 8)
            {
                TooBigFlag = true;
            }
            for(int x = 1; x <= PowerEditBox->Text.Length(); x++)
            {
                if((PowerEditBox->Text[x] < '0') || (PowerEditBox->Text[x] > '9'))
                {
                    PowerVariableLabel->Caption = "Entry error";
                    ErrorFlag = true;
                    break;
                }
                if(TooBigFlag)
                {
                    PowerVariableLabel->Caption = "Too big";
                    break;
                }
            }
            if(!ErrorFlag && !TooBigFlag)
            {
/*
                   1 kW = 1.340482574 HP
                   1 HP = 0.745699872 kW
*/
                if(PowerTopLabel->Caption == "HP")
                {
                    // do HP-to-kW conv
                    int HP = PowerEditBox->Text.ToInt();
                    int KW = (HP * 0.745699872) + 0.5;
                    PowerVariableLabel->Caption = UnicodeString(KW);
                }
                else
                {
                    // do kW-to-HP conv
                    int KW = PowerEditBox->Text.ToInt();
                    int HP = (KW * 1.340482574) + 0.5;
                    PowerVariableLabel->Caption = UnicodeString(HP);
                }
            }
        }
        else
        {
            PowerVariableLabel->Caption = "";
        }
        Utilities->CallLogPop(1868);
    }
    catch(const EConvertError &ec) // thrown for ToInt() conversion error; shouldn't occur but include to prevent a crash
    {
        PowerVariableLabel->Caption = "Entry error";
    }
    catch(const Exception &e)
    {
        ErrorLog(179, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::SpeedEditBox2KeyUp(TObject *Sender, WORD &Key, TShiftState Shift)
{
    try
    {
        TrainController->LogEvent("SpeedEditBox2KeyUp," + AnsiString(Key));
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",SpeedEditBox2KeyUp," + AnsiString(Key));
        bool ErrorFlag = false, TooBigFlag = false;
        if(SpeedEditBox2->Text.Length() > 0)
        {
            if(SpeedEditBox2->Text.Length() > 5)
            {
                TooBigFlag = true;
            }
            for(int x = 1; x <= SpeedEditBox2->Text.Length(); x++)
            {
                if((SpeedEditBox2->Text[x] < '0') || (SpeedEditBox2->Text[x] > '9'))
                {
                    SpeedVariableLabel2->Caption = "Entry error";
                    ErrorFlag = true;
                    break;
                }
                if(TooBigFlag)
                {
                    SpeedVariableLabel2->Caption = "Too big";
                    break;
                }
            }
            if(!ErrorFlag && !TooBigFlag)
            {
/*
                   1 mph =  1.609344 km/h
                   1 km/h = 0.621371 mph
*/
                if(SpeedTopLabel2->Caption == "mph")
                {
                    // do mph-to-km/h conversion
                    int MPH = SpeedEditBox2->Text.ToInt();
                    int KPH = (MPH * 1.609344) + 0.5;
                    SpeedVariableLabel2->Caption = AnsiString(KPH);
                }
                else
                {
                    // do km/h-to-mph conversion
                    int KPH = SpeedEditBox2->Text.ToInt();
                    int MPH = (KPH * 0.621371) + 0.5;
                    SpeedVariableLabel2->Caption = AnsiString(MPH);
                }
            }
        }
        else
        {
            SpeedVariableLabel2->Caption = "";
        }
        Utilities->CallLogPop(1866);
    }
    catch(const EConvertError &ec) // thrown for ToInt() conversion error; shouldn't occur but include to prevent a crash
    {
        SpeedVariableLabel2->Caption = "Entry error";
    }
    catch(const Exception &e)
    {
        ErrorLog(177, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::LengthEditKeyUp(TObject *Sender, WORD &Key, TShiftState Shift)
{
    try
    {
        TrainController->LogEvent("LengthEditKeyUp," + AnsiString(Key));
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",LengthEditKeyUp," + AnsiString(Key));
        bool ErrorFlag = false, TooLongFlag = false;
        if((MileEdit->Text.Length() > 0) && (MileEdit->Text.Length() < 6))
        {
            for(int x = 1; x <= MileEdit->Text.Length(); x++)
            {
                if((MileEdit->Text[x] < '0') || (MileEdit->Text[x] > '9'))
                {
                    MetreVariableLabel->Caption = "Entry error";
                    ErrorFlag = true;
                    break;
                }
            }
        }
        if((ChainEdit->Text.Length() > 0) && (ChainEdit->Text.Length() < 6))
        {
            for(int x = 1; x <= ChainEdit->Text.Length(); x++)
            {
                if((ChainEdit->Text[x] < '0') || (ChainEdit->Text[x] > '9'))
                {
                    MetreVariableLabel->Caption = "Entry error";
                    ErrorFlag = true;
                    break;
                }
            }
        }
        if((YardEdit->Text.Length() > 0) && (YardEdit->Text.Length() < 6))
        {
            for(int x = 1; x <= YardEdit->Text.Length(); x++)
            {
                if((YardEdit->Text[x] < '0') || (YardEdit->Text[x] > '9'))
                {
                    MetreVariableLabel->Caption = "Entry error";
                    ErrorFlag = true;
                    break;
                }
            }
        }
        if((MileEdit->Text.Length() > 5) || (ChainEdit->Text.Length() > 5) || (YardEdit->Text.Length() > 5))
        {
            TooLongFlag = true;
            MetreVariableLabel->Caption = "Too big";
        }
        if(!ErrorFlag && !TooLongFlag)
        {
            int Miles = 0, Chains = 0, Yards = 0, Metres = 0;
            if(MileEdit->Text.Length() > 0)
            {
                Miles = MileEdit->Text.ToInt();
            }
            if(ChainEdit->Text.Length() > 0)
            {
                Chains = ChainEdit->Text.ToInt();
            }
            if(YardEdit->Text.Length() > 0)
            {
                Yards = YardEdit->Text.ToInt();
            }
            Metres = int((Miles * 1609.344) + (Chains * 20.1168) + (Yards * 0.9144) + 0.5);
            MetreVariableLabel->Caption = AnsiString(Metres);
        }
        if((MileEdit->Text.Length() == 0) && (ChainEdit->Text.Length() == 0) && (YardEdit->Text.Length() == 0))
        {
            MetreVariableLabel->Caption = "";
        }
        Utilities->CallLogPop(1867);
    }
    catch(const EConvertError &ec) // thrown for ToInt() conversion error; shouldn't occur but include to prevent a crash
    {
        MetreVariableLabel->Caption = "Entry error";
    }
    catch(const Exception &e)
    {
        ErrorLog(178, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::TTClockAdjButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("TTClockAdjButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",TTClockAdjButtonClick");
//        Utilities->Clock2Stopped = true; // to keep panel buttons disabled, restarted on exit
        Display->HideWarningLog(0); //because this panel overwrites it
        TTClockAdjPanel->Visible = true;
        TTClockAdjButton->Enabled = false;
/*
        OperatingPanelLabel->Caption = "Disabled";   all these now dealt with in ClockTimer2
        OperatingPanel->Enabled = false;
        ZoomButton->Enabled = false;
        HomeButton->Enabled = false;
        NewHomeButton->Enabled = false;
        ScreenLeftButton->Enabled = false;
        ScreenRightButton->Enabled = false;
        ScreenUpButton->Enabled = false;
        ScreenDownButton->Enabled = false;
*/
        Utilities->CallLogPop(1875);
    }
    catch(const Exception &e)
    {
        ErrorLog(181, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::TTClockExitButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("TTClockExitButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",TTClockExitButtonClick");
        TTClockAdjPanel->Visible = false;
        TTClockAdjButton->Enabled = true;
/*  these dealt with in ClockTimer2
        ZoomButton->Enabled = true;
        HomeButton->Enabled = true;
        NewHomeButton->Enabled = true;
        ScreenLeftButton->Enabled = true;
        ScreenRightButton->Enabled = true;
        ScreenUpButton->Enabled = true;
        ScreenDownButton->Enabled = true;
        OperatingPanel->Enabled = true;
        OperatingPanelLabel->Caption = "Operation";
*/
        Display->ShowWarningLog(0);
        double TTClockTimeChange = double(TrainController->RestartTime) - PauseEntryRestartTime;
        if((TTClockSpeed != PauseEntryTTClockSpeed) || (TTClockTimeChange > 0.000347))
    // 30 seconds, min increase is 1 minute & don't trust doubles to stay exactly equal
        {
            AnsiString Message =
                "Changes have been made to the timetable clock - you may wish to save a session before resuming operation.\nTo cancel all changes click 'Adjust the timetable clock' then click the reset button BEFORE resuming operation.";
            ShowMessage(Message);
        }
//        Utilities->Clock2Stopped = false; // as above
        LastNonCtrlOrShiftKeyDown = -1; //to restore the ability to reselect T after adj panel hidden (FormKeyUp doesn't work because the Interface form doesn't have focus)
        Utilities->CallLogPop(1876);
    }
    catch(const Exception &e)
    {
        ErrorLog(182, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::TTClockx2ButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("TTClockx2ButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",TTClockx2ButtonClick");
        TTClockSpeed = 2;
        TTClockSpeedLabel->Caption = "x2";
        TrainController->MTBFHours = double(TrainController->AvHoursIntValue) / TTClockSpeed;
        Utilities->CallLogPop(1878);
    }
    catch(const Exception &e)
    {
        ErrorLog(184, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::TTClockx4ButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("TTClockx4ButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",TTClockx4ButtonClick");
        TTClockSpeed = 4;
        TTClockSpeedLabel->Caption = "x4";
        TrainController->MTBFHours = double(TrainController->AvHoursIntValue) / TTClockSpeed;
        Utilities->CallLogPop(1883);
    }
    catch(const Exception &e)
    {
        ErrorLog(189, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::TTClockx8ButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("TTClockx8ButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",TTClockx8ButtonClick");
        TTClockSpeed = 8;
        TTClockSpeedLabel->Caption = "x8";
        TrainController->MTBFHours = double(TrainController->AvHoursIntValue) / TTClockSpeed;
        Utilities->CallLogPop(1884);
    }
    catch(const Exception &e)
    {
        ErrorLog(190, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::TTClockx16ButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("TTClockx16ButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",TTClockx16ButtonClick");
        TTClockSpeed = 16;
        TTClockSpeedLabel->Caption = "x16";
        TrainController->MTBFHours = double(TrainController->AvHoursIntValue) / TTClockSpeed;
        Utilities->CallLogPop(1885);
    }
    catch(const Exception &e)
    {
        ErrorLog(191, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::TTClockx1ButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("TTClockx1ButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",TTClockx1ButtonClick");
        TTClockSpeed = 1;
        TTClockSpeedLabel->Caption = "x1";
        TrainController->MTBFHours = double(TrainController->AvHoursIntValue) / TTClockSpeed;
        Utilities->CallLogPop(1886);
    }
    catch(const Exception &e)
    {
        ErrorLog(192, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::TTClockxHalfButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("TTClockxHalfButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",TTClockxHalfButtonClick");
        TTClockSpeed = 0.5;
        TTClockSpeedLabel->Caption = "x1/2";
        TrainController->MTBFHours = double(TrainController->AvHoursIntValue) / TTClockSpeed;
        Utilities->CallLogPop(1887);
    }
    catch(const Exception &e)
    {
        ErrorLog(193, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::TTClockxQuarterButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("TTClockxQuarterButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",TTClockxQuarterButtonClick");
        TTClockSpeed = 0.25;
        TTClockSpeedLabel->Caption = "x1/4";
        TrainController->MTBFHours = double(TrainController->AvHoursIntValue) / TTClockSpeed;
        Utilities->CallLogPop(1888);
    }
    catch(const Exception &e)
    {
        ErrorLog(194, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::TTClockxEighthButtonClick(TObject *Sender)
{ // added for v2.3.0 for very big railways
    try
    {
        TrainController->LogEvent("TTClockxEighthButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",TTClockxEighthButtonClick");
        TTClockSpeed = 0.125;
        TTClockSpeedLabel->Caption = "x1/8";
        TrainController->MTBFHours = double(TrainController->AvHoursIntValue) / TTClockSpeed;
        Utilities->CallLogPop(2099);
    }
    catch(const Exception &e)
    {
        ErrorLog(203, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::TTClockxSixteenthButtonClick(TObject *Sender)
{ // added for v2.3.0 for very big railways
    try
    {
        TrainController->LogEvent("TTClockxSixteenthButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",TTClockxSixteenthButtonClick");
        TTClockSpeed = 0.0625;
        TTClockSpeedLabel->Caption = "x1/16";
        TrainController->MTBFHours = double(TrainController->AvHoursIntValue) / TTClockSpeed;
        Utilities->CallLogPop(2100);
    }
    catch(const Exception &e)
    {
        ErrorLog(204, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::TTClockAdd1hButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("TTClockAdd1hButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",TTClockAdd1hButtonClick");
        double TTClockIncrement = 1.0 / 24;
        TrainController->RestartTime += TDateTime(TTClockIncrement);
        TrainController->TTClockTime = TrainController->RestartTime;
        ClockLabel->Caption = Utilities->Format96HHMMSS(TrainController->TTClockTime);
        Utilities->CallLogPop(1879);
    }
    catch(const Exception &e)
    {
        ErrorLog(185, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::TTClockAdd10mButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("TTClockAdd10mButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",TTClockAdd10mButtonClick");
        double TTClockIncrement = 1.0 / 144;
        TrainController->RestartTime += TDateTime(TTClockIncrement);
        TrainController->TTClockTime = TrainController->RestartTime;
        ClockLabel->Caption = Utilities->Format96HHMMSS(TrainController->TTClockTime);
        Utilities->CallLogPop(1881);
    }
    catch(const Exception &e)
    {
        ErrorLog(187, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::TTClockAdd1mButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("TTClockAdd1mButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",TTClockAdd1mButtonClick");
        double TTClockIncrement = 1.0 / 1440;
        TrainController->RestartTime += TDateTime(TTClockIncrement);
        TrainController->TTClockTime = TrainController->RestartTime;
        ClockLabel->Caption = Utilities->Format96HHMMSS(TrainController->TTClockTime);
        Utilities->CallLogPop(1882);
    }
    catch(const Exception &e)
    {
        ErrorLog(188, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::TTClockResetButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("TTClockResetButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",TTClockResetButtonClick");
        TrainController->RestartTime = TDateTime(PauseEntryRestartTime);
        TrainController->TTClockTime = TrainController->RestartTime;
        ClockLabel->Caption = Utilities->Format96HHMMSS(TrainController->TTClockTime);
        TTClockSpeed = PauseEntryTTClockSpeed;
        if(TTClockSpeed == 2)
            TTClockSpeedLabel->Caption = "x2";
        else if(TTClockSpeed == 4)
            TTClockSpeedLabel->Caption = "x4";
        else if(TTClockSpeed == 8)
            TTClockSpeedLabel->Caption = "x8";
        else if(TTClockSpeed == 16)
            TTClockSpeedLabel->Caption = "x16";
        else if(TTClockSpeed == 0.5)
            TTClockSpeedLabel->Caption = "x1/2";
        else if(TTClockSpeed == 0.25)
            TTClockSpeedLabel->Caption = "x1/4";
        else if(TTClockSpeed == 0.125)
            TTClockSpeedLabel->Caption = "x1/8";
        else if(TTClockSpeed == 0.0625)
            TTClockSpeedLabel->Caption = "x1/16";
        else
        {
            TTClockSpeed = 1;
            TTClockSpeedLabel->Caption = "x1";
        }
        Utilities->CallLogPop(1880);
    }
    catch(const Exception &e)
    {
        ErrorLog(186, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::PresetAutoSigRoutesButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("PresetAutoSigRoutesButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",PresetAutoSigRoutesButtonClick");
        InfoPanel->Caption = "PRE-START:  Presetting automatic signal routes";
        OperatingPanelLabel->Caption = "Disabled";
        OperatingPanel->Enabled = false; // becomes re-enabled during the call to ClockTimer2
        ZoomButton->Enabled = false;
        HomeButton->Enabled = false;
        NewHomeButton->Enabled = false;
        ScreenLeftButton->Enabled = false;
        ScreenRightButton->Enabled = false;
        ScreenUpButton->Enabled = false;
        ScreenDownButton->Enabled = false;

        Screen->Cursor = TCursor(-11); // Hourglass
        TPrefDirElement StartElement, EndElement;
        bool PointsChanged, AtLeastOneSet = false;
        int LastIteratorValue = 0;
        while(true)
        {
            if(!EveryPrefDir->GetStartAndEndPrefDirElements(0, StartElement, EndElement, LastIteratorValue))
                break;
            // rest of routine here - i.e. build the routes
            ConstructRoute->ClearRoute(); // in case not empty though should be
            AtLeastOneSet = true;
            if(ConstructRoute->GetPreferredRouteStartElement(1, StartElement.HLoc, StartElement.VLoc, EveryPrefDir, true, true))
    // true for both ConsecSignalsRoute & AutoSigsFlag
            {}
            if(ConstructRoute->GetNextPreferredRouteElement(1, EndElement.HLoc, EndElement.VLoc, EveryPrefDir, true, true, ConstructRoute->ReqPosRouteID,
                PointsChanged))
            {}
            ConstructRoute->ConvertAndAddPreferredRouteSearchVector(3, ConstructRoute->ReqPosRouteID, true); // true for AutoSigsFlag
        }
        if(AtLeastOneSet)
        {
            RevertToOriginalRouteSelector(15);
            ClearandRebuildRailway(68);
        }
        else
        {
            ShowMessage("No presettable automatic signal routes are available");
        }
        Screen->Cursor = TCursor(-2); // Arrow
        Utilities->CallLogPop(1994);
    }
    catch(const Exception &e)
    {
        ErrorLog(195, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::FormResize(TObject *Sender) // new at v2.1.0
{
    try
    {
        if(!SkipFormResizeEvent) // to avoid calling during startup and especially during shutdown
        { // else fails on shutdown because HiddenScreen & other things no longer exist
            int DispW = (Interface->Width - 64 - 16) / 16;
// will truncate down to a multiple of 16  (64 = side panels and 16 compensates for excess width of Interface)
            int DispH = (Interface->Height - 192) / 16;
            MainScreen->Width = DispW * 16;
            MainScreen->Height = DispH * 16;
            Utilities->ScreenElementWidth = DispW;
            Utilities->ScreenElementHeight = DispH;
            HiddenScreen->Width = MainScreen->Width;
            HiddenScreen->Height = MainScreen->Height;
            PerformancePanel->Top = MainScreen->Top + MainScreen->Height - PerformancePanel->Height;
            PerformancePanel->Left = MainScreen->Left;
            OperatorActionPanel->Top = MainScreen->Top + MainScreen->Height - OperatorActionPanel->Height; // new at v2.2.0
            OperatorActionPanel->Left = MainScreen->Left + MainScreen->Width - OperatorActionPanel->Width;
            SigImagePanel->Left = (Interface->Width - SigImagePanel->Width) / 2; // added for v2.3.0
            DevelopmentPanel->Top = MainScreen->Top + MainScreen->Height - DevelopmentPanel->Height; // new v2.2.0
            DevelopmentPanel->Left = MainScreen->Left + MainScreen->Width - DevelopmentPanel->Width; // new v2.2.0
            MTBFEditBox->Left = MainScreen->Left + MainScreen->Width - MTBFEditBox->Width + 32; // new v2.4.0 32 is to place it above the positional panel
            MTBFLabel->Left = MainScreen->Left + MainScreen->Width - MTBFEditBox->Width + 30 - 55; // new v2.4.0 placed above and to the left of MTBFEditBox
            PositionalPanel->Left = MainScreen->Left + MainScreen->Width; // changed at v2.4.0
            PositionalPanel->Top = MainScreen->Top; // changed at v2.4.0
            PositionalPanel->Height = MainScreen->Height; // changed at v2.4.0

            if(!Display->ZoomOutFlag)
            {
                ClearandRebuildRailway(70);
            }
            else
            {
                Display->ClearDisplay(11);
                Track->PlotSmallRailway(9, Display);
            }
            Display->Update();
        }
    }
    catch(const Exception &e)
    {
        ErrorLog(197, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::OperatorActionButtonClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("OperatorActionButtonClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",OperatorActionButtonClick");
        if(!ShowOperatorActionPanel)
        {
            ShowOperatorActionPanel = true;
            OperatorActionPanel->Visible = true;
            TrainController->OpActionPanelVisible = true;
            OperatorActionButton->Glyph->LoadFromResourceName(0, "HideOpActionPanel");
        }
        else
        {
            ShowOperatorActionPanel = false;
            OperatorActionPanel->Visible = false;
            TrainController->OpActionPanelVisible = false;
            OperatorActionButton->Glyph->LoadFromResourceName(0, "ShowOpActionPanel");
        }
        Utilities->CallLogPop(2073);
    }
    catch(const Exception &e)
    {
        ErrorLog(199, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::ConverttoRightHandSignalsMenuItemClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("ConverttoRightHandSignalsMenuItemClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",ConverttoRightHandSignalsMenuItemClick");
        RailGraphics->ConvertSignalsToOppositeHand(2);
        if(Utilities->RHSignalFlag) // RH sigs after conversion
        {
            ConverttoRightHandSignalsMenuItem->Caption = "Convert to Left Hand Signals";
            if(Track->SignalAspectBuildMode == TTrack::GroundSignalBuild)
            {
                LoadGroundSignalGlyphs(2);
            }
            else
            {
                LoadNormalSignalGlyphs(2);
            }
            SigImagePanel->Caption = "Signals will be on the right hand side of the track";
            SigsOnLeftImage1->Visible = false;
            SigsOnLeftImage2->Visible = false;
            SigsOnRightImage1->Visible = true;
            SigsOnRightImage2->Visible = true;
            std::ofstream SigFile((CurDir + "\\Signal.hnd").c_str());
            if(SigFile.fail())
            {
                ShowMessage("Failed to store right hand signal setting, program will default to left hand signals when next loaded");
            }
            else
            {
                Utilities->SaveFileString(SigFile, "RHSignals");
            }
        }
        else // LH sigs after conversion
        {
            ConverttoRightHandSignalsMenuItem->Caption = "Convert to Right Hand Signals";
            if(Track->SignalAspectBuildMode == TTrack::GroundSignalBuild)
            {
                LoadGroundSignalGlyphs(3);
            }
            else
            {
                LoadNormalSignalGlyphs(3);
            }
            SigImagePanel->Caption = "Signals will be on the left hand side of the track";
            SigsOnRightImage1->Visible = false;
            SigsOnRightImage2->Visible = false;
            SigsOnLeftImage1->Visible = true;
            SigsOnLeftImage2->Visible = true;
            std::ofstream SigFile((CurDir + "\\Signal.hnd").c_str());
            if(SigFile.fail())
            {
                // no need for message as will default to LH: ShowMessage("Failed to store left hand signal setting, program will default to left hand signals when next loaded");
            }
            else
            {
                Utilities->SaveFileString(SigFile, "LHSignals");
            }
        }
        Utilities->CallLogPop(2097);
    }
    catch(const Exception &e)
    {
        ErrorLog(202, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::MTBFEditBoxKeyUp(TObject *Sender, WORD &Key, TShiftState Shift)

{
    try
    {
        TrainController->LogEvent("MTBFEditBoxKeyUp," + AnsiString(Key));
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",MTBFEditBoxKeyUp," + AnsiString(Key));
        if((Level1Mode != OperMode) || (Level2OperMode != PreStart))
        {
            Utilities->CallLogPop(2160);
            return;
        }
        bool TooBigFlag = false, BadCharsFlag = false;
        TrainController->AvHoursIntValue = 0;
        TrainController->MTBFHours = 0;
        if(MTBFEditBox->Text.Length() > 0)
        {
            for(int x = 1; x <= MTBFEditBox->Text.Length(); x++)
            {
                if((MTBFEditBox->Text[x] < '0') || (MTBFEditBox->Text[x] > '9'))
                {
                    BadCharsFlag = true;
                    break;
                }
            }
            if(!BadCharsFlag)
            {
                if(StrToInt(MTBFEditBox->Text) > 10000)
                {
                    TooBigFlag = true;
                }
            }
            if(TooBigFlag)
            {
                ShowMessage("Maximum value allowed is 10,000");
                MTBFEditBox->Text = "";
                TrainController->AvHoursIntValue = 0;
                TrainController->MTBFHours = 0;
                Utilities->CallLogPop(2161);
                return;
            }
            if(BadCharsFlag)
            {
                ShowMessage("Value must be a whole number with no special characters");
                MTBFEditBox->Text = "";
                TrainController->AvHoursIntValue = 0;
                TrainController->MTBFHours = 0;
                Utilities->CallLogPop(2162);
                return;
            }
            TrainController->AvHoursIntValue = StrToInt(MTBFEditBox->Text); // ok if user enters 0 as that means no failures
            TrainController->MTBFHours = double(TrainController->AvHoursIntValue) / TTClockSpeed;
        }
        if(TrainController->AvHoursIntValue == 0)
        {
            MTBFEditBox->Text = "";
            TrainController->MTBFHours = 0;
        }
        Utilities->CallLogPop(2163);
    }
    catch(const Exception &e)
    {
        ErrorLog(209, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::MTBFEditBoxClick(TObject *Sender)
{
    try
    {
        TrainController->LogEvent("MTBFEditBoxClick");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",MTBFEditBoxClick");
        if((Level1Mode != OperMode) || (Level2OperMode != PreStart))
        {
            MTBFEditBox->ReadOnly = true; // it should be anyway but include here for safety
            TrainController->StopTTClockMessage(92,
                "Values can only be entered or changed in Pre-Start mode\ni.e. after selecting 'Operate railway' but before clicking 'Run'");
        }
        Utilities->CallLogPop(2164);
    }
    catch(const Exception &e)
    {
        ErrorLog(210, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::UserGraphicButtonClick(TObject *Sender)
{
    try
    {
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",MTBFEditBoxClick");
        LengthConversionPanel->Visible = false;
        SpeedConversionPanel->Visible = false;
        DistanceKey->Visible = false;
        TrackElementPanel->Visible = false;
        SigAspectButton->Enabled = false;
        Level2TrackMode = SelectGraphic;
        SetLevel2TrackMode(63);
        Display->Update();
        if(SelectedGraphicFileName != "")
        {
            UserGraphicReselectPanel->Visible = true;
        }
        else
        {
            UserGraphicReselectPanel->Visible = false;
            LoadUserGraphic(0);
        }
        Utilities->CallLogPop(2183);
    }
    catch(const Exception &e)
    {
        ErrorLog(212, e.Message);
    }
}

// ---------------------------------------------------------------------------

void __fastcall TInterface::ReselectUserGraphicClick(TObject *Sender)
{
    try
    {
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",ReselectUserGraphicClick");
        TrainController->LogEvent("ReselectUserGraphicClick " + SelectedGraphicFileName);
        UserGraphicReselectPanel->Visible = false;
        TTrack::TUserGraphicMap::iterator UGMIt = Track->UserGraphicMap.find(SelectedGraphicFileName);
        if(UGMIt == Track->UserGraphicMap.end())
        {
            ShowMessage("Unable to find graphic file " + SelectedGraphicFileName + ". Check it still exists.");
            Utilities->CallLogPop(2196);
            return;
        }
        Level2TrackMode = AddGraphic;
        SetLevel2TrackMode(64);
        Utilities->CallLogPop(2184);
    }
    catch(const Exception &e)
    {
        ErrorLog(213, e.Message);
    }
}
// ---------------------------------------------------------------------------

void __fastcall TInterface::SelectNewGraphicClick(TObject *Sender)
{
    try
    {
        Utilities->CallLog.push_back(Utilities->TimeStamp() + ",SelectNewGraphicClick");
        UserGraphicReselectPanel->Visible = false;
        LoadUserGraphic(1);
        Utilities->CallLogPop(2185);
    }
    catch(const Exception &e)
    {
        ErrorLog(214, e.Message);
    }
}

// ---------------------------------------------------------------------------
// end of fastcalls & directly associated functions
// ---------------------------------------------------------------------------

void TInterface::ClearandRebuildRailway(int Caller) // now uses HiddenScreen to help avoid flicker
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",ClearandRebuildRailway");
    bool ClockState = Utilities->Clock2Stopped;

    Utilities->Clock2Stopped = true;
    HiddenDisplay->ClearDisplay(6);
    Track->RebuildUserGraphics(0, HiddenDisplay); // new at v2.4.0, plot first so all else overwrites, including the grid if selected
    if(ScreenGridFlag && (Level1Mode == TrackMode))
    {
        int WidthNum = int(MainScreen->Width / 160) + 1;
        int HeightNum = int(MainScreen->Height / 144) + 1;
        for(int x = 0; x < WidthNum; x++)
        {
            for(int y = 0; y < HeightNum; y++)
            {
                HiddenDisplay->PlotAbsolute(0, x * 160, y * 144, RailGraphics->GridBitmap);
            }
        }
    }

// TextHandler->RebuildFromTextVector(1, HiddenDisplay); //This now incorporated in RebuildTrackAndText so that text is plotted after inactive
// elements but before active elements.  This is so text can overwite stations and non-station named locations.

    Track->RebuildTrackAndText(4, HiddenDisplay, (Level1Mode != OperMode));

// Display->Output->Invalidate();  experiment, needs TDisplay Output to be public.  Trying to invoke the white flashes that
// used to occur frequently without Disp->Update() in PlotOriginal

    // OperMode LCs plotted below
    if(Level2TrackMode == GapSetting)
    {
        Track->ShowSelectedGap(1, HiddenDisplay);
    }

    if(Level1Mode == PrefDirMode)
    {
        if(EveryPrefDir->PrefDirSize() > 0)
        {
            EveryPrefDir->EveryPrefDirMarker(0, HiddenDisplay);
        }
        if((Level2PrefDirMode == PrefDirContinuing) && (ConstructPrefDir->PrefDirSize() > 0))
        {
            ConstructPrefDir->PrefDirMarker(5, PrefDirCall, true, HiddenDisplay);
        }
    }

    if(Level1Mode == TrackMode)
    {
        if(Track->NonFootCrossingNamedLocationExists(0))
        {
            LocationNameButton->Enabled = true;
        }
        else
        {
            LocationNameButton->Enabled = false;
        }
    }

    if(Level2TrackMode == DistanceStart)
    {
        Track->LengthMarker(0, HiddenDisplay);
        DistanceKey->Visible = true;
        DistancesMarked = true;
        LengthConversionPanel->Visible = true;
        SpeedConversionPanel->Visible = true;
    }

    if(Level2TrackMode == DistanceContinuing) // for extended distances
    {
        if(ConstructPrefDir->PrefDirSize() > 0)
        {
            ConstructPrefDir->PrefDirMarker(11, PrefDirCall, true, HiddenDisplay);
            Track->LengthMarker(2, HiddenDisplay);
            DistanceKey->Visible = true;
            DistancesMarked = true;
            LengthConversionPanel->Visible = true;
            SpeedConversionPanel->Visible = true;
        }
    }

    if((Level2TrackMode == TrackSelecting) && DistancesMarked)
    // this is to keep the distance markers if they are already present when Select is chosen, in case user wishes to choose SelectLengths,
    // don't need to display ConstructPrefDir marker as that only needed in DistanceContinuing mode
    {
        Track->LengthMarker(1, HiddenDisplay);
        DistanceKey->Visible = true;
    }

    if((Level2TrackMode != TrackSelecting) && (Level2TrackMode != DistanceContinuing) && (Level2TrackMode != DistanceStart))
    // cancel DistancesMarked if exit from any of these modes
    {
        DistancesMarked = false;
        DistanceKey->Visible = false;
        LengthConversionPanel->Visible = false; // added at v1.3.1 to remove when distance/speed setting exited
        SpeedConversionPanel->Visible = false; // added at v1.3.1 to remove when distance/speed setting exited
    }

    if(mbLeftDown && SelectPickedUp && ((Level2TrackMode == CopyMoving) || (Level2TrackMode == CutMoving)))
    // in process of moving so use NewSelectBitmapHLoc & VLoc
    {
        HiddenDisplay->PlotOutput(8, NewSelectBitmapHLoc * 16, NewSelectBitmapVLoc * 16, SelectBitmap);
    }

    else if((!mbLeftDown || !SelectPickedUp) && ((Level2TrackMode == CopyMoving) || (Level2TrackMode == CutMoving)))
    // not in process of moving or failed to click mouse within selection so use SelectBitmapHLoc & VLoc
    {
        HiddenDisplay->PlotOutput(9, SelectBitmapHLoc * 16, SelectBitmapVLoc * 16, SelectBitmap);
    }

    if(Level1Mode == OperMode)
    {
        AllRoutes->MarkAllRoutes(0, HiddenDisplay);
        if(!AllRoutes->LockedRouteVector.empty())
        {
            for(TAllRoutes::TLockedRouteVectorIterator LRVIT = AllRoutes->LockedRouteVector.end() - 1; LRVIT >= AllRoutes->LockedRouteVector.begin(); LRVIT--)
            {
                if(!(AllRoutes->TrackIsInARoute(7, LRVIT->LastTrackVectorPosition, LRVIT->LastXLinkPos)))
                {
                    AllRoutes->LockedRouteVector.erase(LRVIT);
                    // if end element not in route then a train must have entered it from the wrong end and erased the whole route,
                    // hence no longer needed so get rid of it (end of route can't be points, crossover or bridge so danger of
                    // route being on the other track of a 2-track element doesn't arise)
                    continue;
                }
                TOneRoute Route = AllRoutes->GetFixedRouteAt(0, LRVIT->RouteNumber);
                int x = Route.PrefDirSize() - 1;
                bool BreakFlag = false;
                TPrefDirElement PrefDirElement = Route.GetFixedPrefDirElementAt(1, x);
                while(PrefDirElement.GetTrackVectorPosition() != LRVIT->TruncateTrackVectorPosition)
                {
                    HiddenDisplay->PlotOutput(10, (PrefDirElement.HLoc) * 16, (PrefDirElement.VLoc) * 16,
                        RailGraphics->LockedRouteCancelPtr[PrefDirElement.GetELink()]);
                    if(!(AllRoutes->TrackIsInARoute(8, PrefDirElement.Conn[PrefDirElement.GetELinkPos()],
                        PrefDirElement.ConnLinkPos[PrefDirElement.GetELinkPos()])))
                    {
                        BreakFlag = true;
                        break; // train removed earlier element from route so stop here
                    }
                    x--;
                    PrefDirElement = Route.GetFixedPrefDirElementAt(2, x);
                }
                if(!BreakFlag)
                {
                    if(PrefDirElement.GetTrackVectorPosition() == LRVIT->TruncateTrackVectorPosition)
                    {
                        HiddenDisplay->PlotOutput(11, (PrefDirElement.HLoc) * 16, (PrefDirElement.VLoc) * 16,
                            RailGraphics->LockedRouteCancelPtr[PrefDirElement.GetELink()]);
                    }
                }
            }
        }

        if(RouteMode == RouteContinuing)
        {
            AutoRouteStartMarker->PlotOriginal(23, HiddenDisplay);
// system thinks overlay is already plotted, so plot original to reset the OverlayPlotted flag
            SigRouteStartMarker->PlotOriginal(24, HiddenDisplay);
            NonSigRouteStartMarker->PlotOriginal(25, HiddenDisplay);
            if(AutoSigsFlag)
                AutoRouteStartMarker->PlotOverlay(7, HiddenDisplay);
            else if(ConsecSignalsRoute)
                SigRouteStartMarker->PlotOverlay(8, HiddenDisplay);
            else
                NonSigRouteStartMarker->PlotOverlay(9, HiddenDisplay);
        }

        if(Track->PointFlashFlag)
        {
            // need to reset the screen location for picking up the original graphic
            int Left, Top; // Embarcadero change - these missing in error from Borland file
            Track->GetScreenPositionsFromTruePos(1, Left, Top, PointFlash->GetHPos(), PointFlash->GetVPos());
            // note that the above Pos values are wrt layout, not the screen, but the Left & Top values are wrt screen
            PointFlash->SetSourceRect(Left, Top);
            PointFlash->LoadOriginalScreenGraphic(4); // reload from new position
            // doesn't matter whether Flash was on or off when this function called as will sort itself out later (may miss a flash but won't be noticeable)
        }

        // now plot level crossings (must be after routes). These don't need any base elements to be plotted as they are already plotted.
        // In order to avoid plotting the whole LC for every element of a LC a TempMarker is used
        for(unsigned int x = 0; x < Track->LCVector.size(); x++)
        {
            (Track->InactiveTrackVector.begin() + (*(Track->LCVector.begin() + x)))->TempMarker = false;
        }
        for(unsigned int x = 0; x < Track->LCVector.size(); x++)
        {
            int BaseSpeedTag;
            TTrackElement ATE;
            TTrackElement ITE = *(Track->InactiveTrackVector.begin() + (*(Track->LCVector.begin() + x)));
            {
                BaseSpeedTag = Track->GetTrackElementFromTrackMap(0, ITE.HLoc, ITE.VLoc).SpeedTag;
                if(ITE.TempMarker == false)
                {
                    if(ITE.Attribute == 0)
                    {
                        Track->PlotPlainRaisedLinkedLevelCrossingBarriersAndSetMarkers(0, BaseSpeedTag, ITE.HLoc, ITE.VLoc, HiddenDisplay);
                    }
                    else if(ITE.Attribute == 1)
                    {
                        Track->PlotPlainLoweredLinkedLevelCrossingBarriersAndSetMarkers(0, BaseSpeedTag, ITE.HLoc, ITE.VLoc, HiddenDisplay);
                    }
                    // if ITE->Attribute == 2 then LC is changing, FlashingGraphics will take care of flashing & final plotting
                    // won't set marker but no real time lost in this case
                }
            }}
        TrainController->ReplotTrains(0, HiddenDisplay);
    }

    Display->ZoomOutFlag = false;
    ZoomButton->Glyph->LoadFromResourceName(0, "ZoomOut");
    MainScreen->Picture->Bitmap->Assign(HiddenScreen->Picture->Bitmap);
    Display->Update(); // resurrected when Update() dropped from PlotOutput etc
    Utilities->Clock2Stopped = ClockState;
    Utilities->CallLogPop(91);
}

// ---------------------------------------------------------------------------

bool TInterface::HighLightOneGap(int Caller, int &HLoc, int &VLoc)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",HighLightOneGap");
    if(Track->FindAndHighlightAnUnsetGap(1)) // true if find one
    {
        if(!PreventGapOffsetResetting) // don't reset display position if returning from zoomout mode
        {
            while((Display->DisplayOffsetH - Track->GetGapHLoc()) > 0)
                Display->DisplayOffsetH -= (Utilities->ScreenElementWidth / 2); // use 30 instead of 60 so less likely to appear behind the message box
            while((Track->GetGapHLoc() - Display->DisplayOffsetH) > (Utilities->ScreenElementWidth - 1))
                Display->DisplayOffsetH += (Utilities->ScreenElementWidth / 2);
            while((Display->DisplayOffsetV - Track->GetGapVLoc()) > 0)
                Display->DisplayOffsetV -= (Utilities->ScreenElementHeight / 2); // use 18 instead of 36 so less likely to appear behind the message box
            while((Track->GetGapVLoc() - Display->DisplayOffsetV) > (Utilities->ScreenElementHeight - 1))
                Display->DisplayOffsetV += (Utilities->ScreenElementHeight / 2);
        }
        InfoPanel->Visible = true;
        InfoPanel->Caption = "CONNECTING GAPS:  Click on connecting element";
        ClearandRebuildRailway(31); // get rid of earlier gap selection
        Utilities->CallLogPop(92);
        return true; // return as one now identified & over to MainScreenMouseDown with Level2TrackMode = GapSetting
    }
    Utilities->CallLogPop(93);
    return false; // no unset ones left to find
}

// ---------------------------------------------------------------------------

bool TInterface::ClearEverything(int Caller)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",ClearEverything");
    if(FileChangedFlag)
    {
        UnicodeString MessageStr = "The railway has changed, close it without saving?";
        int button = Application->MessageBox(MessageStr.c_str(), L"Please confirm", MB_YESNO);
        if(button == IDNO)
        {
            Utilities->CallLogPop(1140);
            return false;
        }
    }
    Display->ClearDisplay(7);
    HiddenDisplay->ClearDisplay(8);

    Display->DisplayOffsetH = 0;
    Display->DisplayOffsetV = 0;
    Display->DisplayOffsetHHome = 0;
    Display->DisplayOffsetVHome = 0;
    Display->DisplayZoomOutOffsetH = 0;
    Display->DisplayZoomOutOffsetV = 0;

// these ensure that all persistent vectors, maps & multimaps etc are cleared
    delete TrainController;
    delete EveryPrefDir;
    delete ConstructRoute;
    delete ConstructPrefDir;
    delete AllRoutes;
    delete Track;
    delete TextHandler;
// NB can't delete & recreate Utilities or will lose the CallLog file & have errors due to log being empty when try to
// pop earlier pushed values
// OK though as no containers in Utilities that need to clear & PerformanceFile recreated when begin to operate a later
// railway
    TextHandler = new TTextHandler;
    Track = new TTrack;
    AllRoutes = new TAllRoutes;
    ConstructPrefDir = new TOnePrefDir;
    ConstructRoute = new TOneRoute;
    EveryPrefDir = new TOnePrefDir;
    TrainController = new TTrainController;
    PerformanceLogBox->Lines->Clear();
    ResetAll(1);
    Utilities->CallLogPop(94);
    return true;
}

// ---------------------------------------------------------------------------

bool TInterface::FileIntegrityCheck(int Caller, char *FileName) const // true for success
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",FileIntegrityCheck," + AnsiString(FileName));
    std::ifstream VecFile(FileName);

    if(VecFile.is_open())
    {
        if(!Utilities->CheckFileStringZeroDelimiter(VecFile)) // Program version
        {
            VecFile.close();
            Utilities->CallLogPop(1805);
            return false;
        }
        if(!Utilities->CheckFileInt(VecFile, -1000000, 1000000)) // DisplayOffsetHHome
        {
            VecFile.close();
            Utilities->CallLogPop(1440);
            return false;
        }
        if(!Utilities->CheckFileInt(VecFile, -1000000, 1000000)) // DisplayOffsetVHome
        {
            VecFile.close();
            Utilities->CallLogPop(1441);
            return false;
        }
        bool GraphicsFollow = false;
        int NumberOfActiveElements;
        if(!(Track->CheckTrackElementsInFile(1, NumberOfActiveElements, GraphicsFollow, VecFile))) // for new loads
        {
            VecFile.close();
            Utilities->CallLogPop(95);
            return false;
        }
        if(!(TextHandler->CheckTextElementsInFile(0, VecFile)))
        {
            VecFile.close();
            Utilities->CallLogPop(96);
            return false;
        }
        if(!(EveryPrefDir->CheckOnePrefDir(0, NumberOfActiveElements, VecFile)))
        {
            VecFile.close();
            Utilities->CallLogPop(97);
            return false;
        }
        if(GraphicsFollow)
        {
            if(!Track->CheckUserGraphics(0, VecFile, CurDir + "\\" + USERGRAPHICS_DIR_NAME)) // include path to Graphics folder
            {
                VecFile.close();
                Utilities->CallLogPop(2186);
                return false;
            }
        }
        VecFile.close();
    }
    else
    {
        Utilities->CallLogPop(1153);
        return false;
    }
    Utilities->CallLogPop(98);
    return true;
}

// ---------------------------------------------------------------------------

void TInterface::Delay(int Caller, double Msec)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",Delay," + AnsiString(Msec));
    TDateTime First, Second;
    bool Finished = false;

    First = TDateTime::CurrentDateTime();
    double TimeVal1 = 86400000 * double(First); // no of msec in a day

    while(!Finished)
    {
        Second = TDateTime::CurrentDateTime();
        double TimeVal2 = 86400000 * double(Second);
        if((TimeVal2 - TimeVal1) > Msec)
            Finished = true;
    }
    Utilities->CallLogPop(1203);
}

// ---------------------------------------------------------------------------

void TInterface::ResetCurrentSpeedButton(int Caller)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",ResetCurrentSpeedButton");
    if(CurrentSpeedButton)
        CurrentSpeedButton->Down = false;
    CurrentSpeedButton = 0;
    Utilities->CallLogPop(1204);
}

// ---------------------------------------------------------------------------

bool TInterface::MovingTrainPresentOnFlashingRoute(int Caller)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",MovingTrainPresentOnFlashingRoute");
    int TrainID;

    if(ConstructRoute->SearchVectorSize() == 0)
    {
        Utilities->CallLogPop(99);
        return false;
    }
    for(unsigned int x = 0; x < ConstructRoute->SearchVectorSize(); x++)
    {
        TPrefDirElement PrefDirElement = ConstructRoute->GetFixedSearchElementAt(14, x);
        if(PrefDirElement.TrackType == Bridge)
        {
            if(PrefDirElement.GetXLinkPos() < 2)
                TrainID = Track->TrackElementAt(486, PrefDirElement.GetTrackVectorPosition()).TrainIDOnBridgeTrackPos01;
            else
                TrainID = Track->TrackElementAt(487, PrefDirElement.GetTrackVectorPosition()).TrainIDOnBridgeTrackPos23;
        }
        else
            TrainID = Track->TrackElementAt(488, PrefDirElement.GetTrackVectorPosition()).TrainIDOnElement;
        if((TrainID > -1) && !(TrainController->TrainVectorAtIdent(4, TrainID).Stopped()))
        {
            Utilities->CallLogPop(100);
            return true;
        }
        // check for crossed diagonal fouling by train added at v1.2.0
        int TrainID; // not used
        int LinkNumber1 = PrefDirElement.Link[PrefDirElement.GetELinkPos()];
        int LinkNumber2 = PrefDirElement.Link[PrefDirElement.GetXLinkPos()];
        if((LinkNumber1 == 1) || (LinkNumber1 == 3) || (LinkNumber1 == 7) || (LinkNumber1 == 9))
        {
            if(Track->DiagonalFouledByTrain(1, PrefDirElement.HLoc, PrefDirElement.VLoc, LinkNumber1, TrainID))
            {
                Utilities->CallLogPop(2037);
                return true;
            }
        }
        if((LinkNumber2 == 1) || (LinkNumber2 == 3) || (LinkNumber2 == 7) || (LinkNumber2 == 9))
        {
            if(Track->DiagonalFouledByTrain(2, PrefDirElement.HLoc, PrefDirElement.VLoc, LinkNumber2, TrainID))
            {
                Utilities->CallLogPop(2038);
                return true;
            }
        }
    }
    Utilities->CallLogPop(101);
    return false;
}

// ---------------------------------------------------------------------------

void TInterface::RevertToOriginalRouteSelector(int Caller)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",RevertToOriginalRouteSelector");
    AutoRouteStartMarker->PlotOriginal(26, Display); // if overlay not plotted will ignore
    SigRouteStartMarker->PlotOriginal(27, Display); // if overlay not plotted will ignore
    NonSigRouteStartMarker->PlotOriginal(28, Display); // if overlay not plotted will ignore
    RouteCancelFlag = false;
    if(AllRoutes->AllRoutesVector.size() > AllRoutes->LockedRouteVector.size())
    {
        RouteCancelButton->Enabled = true;
    }
    else
    {
        RouteCancelButton->Enabled = false;
    }
    RouteMode = RouteNotStarted;
    ConstructRoute->StartSelectionRouteID = IDInt(-1);
// reset here so that a n element that has been selected and then not doesn't remain set as a single element
    InfoPanel->Visible = true;
    if(Level2OperMode != Paused)
    {
        InfoPanel->Caption = InfoCaptionStore;
    }
    Utilities->CallLogPop(102);
}

// ---------------------------------------------------------------------------

// usermode functions below
void TInterface::SetLevel1Mode(int Caller)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",SetLevel1Mode");
    if(!Display->ZoomOutFlag)
    {
        Track->GapFlashGreen->PlotOriginal(39, Display);
        Track->GapFlashRed->PlotOriginal(40, Display);
        Track->GapFlashFlag = false;
    }
// GapFlash resets when any mode selected unless zoomed out
// note that if selecting zoom back in then this will be called before ZoomOutFlag is reset so won't
// reset GapFlashFlag
    switch(Level1Mode) // use the data member
    {
    case BaseMode:
        CopyMenuItem->ShortCut = TextToShortCut(""); // added these for v2.1.0 to set default values after use of the 'Edit' menu during track building
        CutMenuItem->ShortCut = TextToShortCut(""); // to allow normal cutting/copying/pasting, especially in timetable construction or editing
        PasteMenuItem->ShortCut = TextToShortCut("");
        Track->ActiveTrackElementNameMapCompiledFlag = false;
        Level2TrackMode = NoTrackMode;
        Level2PrefDirMode = NoPrefDirMode;
        Level2OperMode = NoOperMode;
        LengthConversionPanel->Visible = false;
        SpeedConversionPanel->Visible = false;
        TimetableEditPanel->Visible = false;
        TrackBuildPanel->Visible = false;
        TrackElementPanel->Visible = false;
        LocationNameTextBox->Visible = false;
        TextBox->Visible = false;
        TrackLengthPanel->Visible = false;
        InfoPanel->Visible = false;
        PrefDirPanel->Visible = false;
        TimetablePanel->Visible = false;
        OperatingPanel->Visible = false;
        PrefDirKey->Visible = false;
        TrackLinkedImage->Visible = false;
        TrackNotLinkedImage->Visible = false;
        GapsSetImage->Visible = false;
        GapsNotSetImage->Visible = false;
        LocationNamesSetImage->Visible = false;
        LocationNamesNotSetImage->Visible = false;
        ModeMenu->Enabled = true;
        FileMenu->Enabled = true;
        EditMenu->Enabled = false;
        BuildTrackMenuItem->Enabled = true;
        SigAspectButton->Enabled = false;
        Track->ChangingLCVector.clear();
        Track->BarriersDownVector.clear();
        Track->ResetLevelCrossings(0);
        ConverttoRightHandSignalsMenuItem->Enabled = false; // new at v2.3.0
        SigImagePanel->Visible = false; // new at v2.3.0
        MTBFEditBox->Visible = false; // new at v2.4.0
        MTBFLabel->Visible = false;
        if(Track->IsTrackFinished())
        {
            PlanPrefDirsMenuItem->Enabled = true;
            if(TimetableTitle != "")
            {
                OperateRailwayMenuItem->Enabled = true;
            }
            else
            {
                OperateRailwayMenuItem->Enabled = false;
            }
        }
        else
        {
            PlanPrefDirsMenuItem->Enabled = false;
            OperateRailwayMenuItem->Enabled = false;
        }
        if(RlyFile)
        {
            LoadTimetableMenuItem->Enabled = true;
        }
        else
        {
            LoadTimetableMenuItem->Enabled = false;
        }
        LoadRailwayMenuItem->Enabled = true;
        if(NoRailway())
        {
            SaveAsMenuItem->Enabled = false;
            ImageMenu->Enabled = false;
            SaveImageAndGridMenuItem->Enabled = false;
            SaveImageNoGridMenuItem->Enabled = false;
            SaveImageAndPrefDirsMenuItem->Enabled = false;
            SaveOperatingImageMenuItem->Enabled = false;
            BlackBgndMenuItem->Enabled = false;
            WhiteBgndMenuItem->Enabled = false;
            BlueBgndMenuItem->Enabled = false;
            ConverttoRightHandSignalsMenuItem->Enabled = true; // new at v2.3.0
            SigImagePanel->Visible = true; // new at v2.3.0
            if(Utilities->clTransparent != TColor(0))
            {
                BlackBgndMenuItem->Enabled = true;
            }
            if(Utilities->clTransparent != TColor(0xFFFFFF))
            {
                WhiteBgndMenuItem->Enabled = true;
            }
            if(Utilities->clTransparent != TColor(0x330000))
            {
                BlueBgndMenuItem->Enabled = true;
            }
            ClearAllMenuItem->Enabled = false;
            InfoPanel->Visible = true;
            InfoPanel->Caption = "Select an option from the File, Mode or Help menus";
        }
        else
        {
            InfoPanel->Visible = false;
            SaveAsMenuItem->Enabled = true;
            ImageMenu->Enabled = true;
            SaveImageAndGridMenuItem->Enabled = true;
            SaveImageNoGridMenuItem->Enabled = true;
            if(EveryPrefDir->PrefDirSize() > 0)
                SaveImageAndPrefDirsMenuItem->Enabled = true;
            else
                SaveImageAndPrefDirsMenuItem->Enabled = false;
            BlackBgndMenuItem->Enabled = false;
            WhiteBgndMenuItem->Enabled = false;
            BlueBgndMenuItem->Enabled = false;
            SaveOperatingImageMenuItem->Enabled = false;
            ClearAllMenuItem->Enabled = true;
        }
        if(SavedFileName == "")
        {
            SaveMenuItem->Enabled = false;
        }
        else if(!FileChangedFlag)
        {
            SaveMenuItem->Enabled = false;
        }
        else if((SavedFileName[SavedFileName.Length()] == 'y') || (SavedFileName[SavedFileName.Length()] == 'Y')) // 'rly' file
        {
            if(!(Track->IsReadyForOperation()))
            {
                SaveMenuItem->Enabled = false; // can't save under its old name as not now a .rly file
            }
            else
            {
                SaveMenuItem->Enabled = true; // must have changed some of the PrefDirs (because FileChangedFlag is true)
            }
        }
        else
            SaveMenuItem->Enabled = true;
        LoadSessionMenuItem->Enabled = true;
        ExitMenuItem->Enabled = true;
        ScreenGridFlag = false;
        TrainController->CrashWarning = false;
        TrainController->DerailWarning = false;
        TrainController->SPADWarning = false;
        TrainController->TrainFailedWarning = false;
        TrainController->CallOnWarning = false;
        TrainController->SignalStopWarning = false;
        TrainController->BufferAttentionWarning = false;
        UserGraphicReselectPanel->Visible = false;
        ClearandRebuildRailway(32); // to get rid of unwanted displays (eg distance markers)
        SetTrackBuildImages(13);
        break;

    case TimetableMode:
        Level2TrackMode = NoTrackMode;
        Level2PrefDirMode = NoPrefDirMode;
        Level2OperMode = NoOperMode;
        ModeMenu->Enabled = false;
        SigImagePanel->Visible = false; // new at v2.3.0
        FileMenu->Enabled = false;
        EditMenu->Enabled = false;
        FloatingInfoMenu->Enabled = false;
        ImageMenu->Enabled = false;
        TimetableEditPanel->BringToFront();
        TimetableHandler();
        break;

    case TrackMode:
        if(Level2TrackMode == CutMoving)
        {
            Level2TrackMode = Pasting; // paste the selection
            SetLevel2TrackMode(52);
        }
        Level2TrackMode = NoTrackMode;
        Level2PrefDirMode = NoPrefDirMode;
        Level2OperMode = NoOperMode;
        ResetCurrentSpeedButton(0);
        TrackBuildPanel->Visible = true;
        TrackBuildPanelLabel->Caption = "Build/modify";
        TrackElementPanel->Visible = false;
        TrackLengthPanel->Visible = false;
        PrefDirPanel->Visible = false;
        TimetablePanel->Visible = false;
        OperatingPanel->Visible = false;
        InfoPanel->Visible = false;
        InfoPanel->Caption = "";
        LocationNameTextBox->Visible = false;
        TextBox->Visible = false;
        ModeMenu->Enabled = false;
        SigImagePanel->Visible = false; // new at v2.3.0
        FileMenu->Enabled = false;
        // set edit menu items
        SetInitialTrackModeEditMenu();
        // track buttons
        AddTrackButton->Enabled = true;
        if(Track->NonFootCrossingNamedLocationExists(1))
        {
            LocationNameButton->Enabled = true;
        }
        else
        {
            LocationNameButton->Enabled = false;
        }
        ScreenGridButton->Enabled = true;
        ExitTrackButton->Enabled = true;
        SetGapsButton->Enabled = false;
        TrackOKButton->Enabled = false;
        if(Track->GapsUnset(5))
        {
            SetGapsButton->Enabled = true;
        }
        // only enable if there are gaps still to be set (returns false for no track)
        else
        {
            if(!(Track->NoActiveTrack(2)) && !(Track->IsTrackFinished()))
            {
                TrackOKButton->Enabled = true;
            }
            // TrackOK only enabled if track exists, there are no unset gaps, and track not finished
        }
        SetLengthsButton->Enabled = false;
        if(Track->IsTrackFinished()) // can only set lengths for several elements together if TrackFinished
        {
            SetLengthsButton->Enabled = true;
        }
        // text buttons
        AddTextButton->Enabled = true;
        TextOrUserGraphicGridButton->Enabled = true;
        FontButton->Enabled = true;
        MoveTextOrGraphicButton->Enabled = false;
        if(TextHandler->TextVectorSize(9) > 0)
        {
            MoveTextOrGraphicButton->Enabled = true;
        }
        if(!Track->UserGraphicVector.empty())
        {
            MoveTextOrGraphicButton->Enabled = true;
        }
        SelectionValid = false;
        SetTrackBuildImages(1);
        TimetableTitle = "";
        SetCaption(0);
        break;

    case PrefDirMode:
        Level2TrackMode = NoTrackMode;
        Level2PrefDirMode = NoPrefDirMode;
        Level2OperMode = NoOperMode;
        PrefDirPanel->Visible = true;
        PrefDirPanelLabel->Caption = "Preferred direction selection";

        InfoPanel->Visible = true;
        InfoPanel->Caption = "PREFERRED DIRECTION SETTING:  Select preferred direction start location (right click to erase)";
        PrefDirKey->Visible = true;
        ModeMenu->Enabled = false;
        SigImagePanel->Visible = false; // new at v2.3.0
        FileMenu->Enabled = false;
// set edit menu items
        SetInitialPrefDirModeEditMenu();
        AddPrefDirButton->Enabled = false;
        DeleteOnePrefDirButton->Enabled = false;
        ConstructPrefDir->ExternalClearPrefDirAnd4MultiMap();
        if(EveryPrefDir->PrefDirSize() > 0)
        {
            DeleteAllPrefDirButton->Visible = true;
            DeleteAllPrefDirButton->Enabled = true;
            SaveImageAndPrefDirsMenuItem->Enabled = true;
        }
        else
        {
            DeleteAllPrefDirButton->Enabled = false;
            SaveImageAndPrefDirsMenuItem->Enabled = false;
        }
        ExitPrefDirButton->Enabled = true;
        ClearandRebuildRailway(33); // to mark PrefDirs & clear earlier PrefDir markers
// TimetableTitle = ""; no need to unload timetable if only PrefDirs being changed
// SetCaption();
        break;

    case OperMode: // if there are any PrefDirs, set to SigPref, else to NoSigNonPref; start in Paused mode
        Level2TrackMode = NoTrackMode;
        Level2PrefDirMode = NoPrefDirMode;
        Level2OperMode = PreStart;
        OperatingPanel->Visible = true;
        OperatingPanelLabel->Caption = "Operation";

        CallingOnButton->Visible = false;
        PresetAutoSigRoutesButton->Visible = true;
        PresetAutoSigRoutesButton->Enabled = true;
        InfoPanel->Visible = true;
        SigImagePanel->Visible = false; // new at v2.3.0
        ModeMenu->Enabled = false;
        FileMenu->Enabled = false;
        EditMenu->Enabled = false;
        ImageMenu->Enabled = true;
        SaveImageAndGridMenuItem->Enabled = true;
        SaveImageNoGridMenuItem->Enabled = true;
        if(EveryPrefDir->PrefDirSize() > 0)
            SaveImageAndPrefDirsMenuItem->Enabled = true;
        else
            SaveImageAndPrefDirsMenuItem->Enabled = false;
        SaveOperatingImageMenuItem->Enabled = true;
        AutoSigsFlag = false;
        MTBFEditBox->Visible = true; // visible at pre-start whether any value set or not, so can set a value if required
        if(TrainController->AvHoursIntValue > 0)
        {
            MTBFEditBox->Text = AnsiString(TrainController->AvHoursIntValue);
        }
        else
        {
            MTBFEditBox->Text = "";
        }
        MTBFEditBox->ReadOnly = false; // because this is prestart mode
        MTBFLabel->Visible = true;
        MTBFLabel->Caption = "Mean time between\ntrain failures in\ntimetable hours";
        TrainController->MTBFHours = double(TrainController->AvHoursIntValue) / TTClockSpeed;
        if(EveryPrefDir->PrefDirSize() > 0)
        {
            ConsecSignalsRoute = true;
            PreferredRoute = true;
        }
        else // no PrefDirs
        {
            ConsecSignalsRoute = false;
            PreferredRoute = false;
        }

        OperateButton->Enabled = true;
        OperateButton->Glyph->LoadFromResourceName(0, "RunGraphic");
        ExitOperationButton->Enabled = true;
        TTClockAdjButton->Enabled = true;
        ShowPerformancePanel = false;
        PerformanceLogButton->Glyph->LoadFromResourceName(0, "ShowLog");
        ShowOperatorActionPanel = false; // new at v2.2.0
        OperatorActionButton->Glyph->LoadFromResourceName(0, "ShowOpActionPanel"); // new v2.2.0

        SetRouteButtonsInfoCaptionAndRouteNotStarted(2);

        Utilities->Clock2Stopped = false;
        TrainController->RestartTime = TrainController->TimetableStartTime;
        PauseEntryRestartTime = double(TrainController->RestartTime);
        PauseEntryTTClockSpeed = 1;
        TTClockSpeed = 1;
        TTClockSpeedLabel->Caption = "x1";
        TrainController->TTClockTime = TrainController->TimetableStartTime;

        PerformanceFileName = TDateTime::CurrentDateTime().FormatString("dd-mm-yyyy hh.nn.ss");
        // format "16/06/2009 20:55:17"
        // avoid characters in filename:=   / \ : * ? " < > |
        PerformanceFileName = CurDir + "\\" + PERFLOG_DIR_NAME + "\\Log " + PerformanceFileName + "; " + RailwayTitle + "; " + TimetableTitle + ".txt";

        Utilities->PerformanceFile.open(PerformanceFileName.c_str(), std::ios_base::out);
        if(Utilities->PerformanceFile.fail())
        {
            ShowMessage("Performance logfile failed to open, logs won't be saved. Ensure that there is a folder named " + PERFLOG_DIR_NAME +
                " in the folder where the 'Railway.exe' program file resides");
        }
        SetPausedOrZoomedInfoCaption(3);
// DisableRouteButtons(2); enable route setting or pre-start
// DisablePanelsStoreMainMenuStates();
        TrainController->ContinuationAutoSigVector.clear(); // for restarting after earlier run
        AllRoutes->LockedRouteVector.clear(); // for restarting after earlier run
// TrainController->Operate(1);//plot trains that are present at TT start time, ready for running - no, allow route plotting prior to train entries

// reset all performance indicators
        TrainController->OnTimeArrivals = 0;
        TrainController->LateArrivals = 0;
        TrainController->EarlyArrivals = 0;
        TrainController->OnTimePasses = 0;
        TrainController->LatePasses = 0;
        TrainController->EarlyPasses = 0;
        TrainController->OnTimeDeps = 0;
        TrainController->LateDeps = 0;
        TrainController->MissedStops = 0;
        TrainController->OtherMissedEvents = 0;
        TrainController->UnexpectedExits = 0;
        TrainController->IncorrectExits = 0;
        TrainController->SPADEvents = 0;
        TrainController->SPADRisks = 0;
        TrainController->CrashedTrains = 0;
        TrainController->Derailments = 0;
        TrainController->TotArrDepPass = 0;
        TrainController->TotLateArrMins = 0;
        TrainController->TotEarlyArrMins = 0;
        TrainController->TotLatePassMins = 0;
        TrainController->TotEarlyPassMins = 0;
        TrainController->TotLateDepMins = 0;
        TrainController->ExcessLCDownMins = 0;

        TrainController->OpActionPanelHintDelayCounter = 0; // new at v2.2.0 to reset hint delay
        OAListBox->Clear();
        OAListBox->Items->Add(L""); // hints for OpActionPanel
        OAListBox->Items->Add(L"");
        OAListBox->Items->Add(L"");
        OAListBox->Items->Add(L"Left click");
        OAListBox->Items->Add(L"headcode");
        OAListBox->Items->Add(L"to locate train");
        OAListBox->Items->Add(L"");
        OAListBox->Items->Add(L"");
        OAListBox->Items->Add(L"");
        OAListBox->Items->Add(L"");
        OAListBox->Items->Add(L"Left click and");
        OAListBox->Items->Add(L"hold grey area");
        OAListBox->Items->Add(L"to move panel");

        ClearandRebuildRailway(55); // so points display with one fillet
        break;

    case RestartSessionOperMode: // restart in Paused mode after a session load, sets both Level1Mode & Level2OperMode
        Level1Mode = OperMode;
// Level2OperMode = Paused; this is now loaded during LoadInterface & could be PreStart of Paused
        Level2TrackMode = NoTrackMode;
        Level2PrefDirMode = NoPrefDirMode;
        OperatingPanel->Visible = true;
        OperatingPanelLabel->Caption = "Operation";

        CallingOnButton->Visible = true;
        PresetAutoSigRoutesButton->Visible = false;
        InfoPanel->Visible = true;
        ModeMenu->Enabled = false;
        SigImagePanel->Visible = false; // new at v2.3.0
        FileMenu->Enabled = false;
        EditMenu->Enabled = false;
        ImageMenu->Enabled = true;
        SaveImageAndGridMenuItem->Enabled = true;
        SaveImageNoGridMenuItem->Enabled = true;
        if(EveryPrefDir->PrefDirSize() > 0)
            SaveImageAndPrefDirsMenuItem->Enabled = true;
        else
            SaveImageAndPrefDirsMenuItem->Enabled = false;
        SaveOperatingImageMenuItem->Enabled = true;

        OperateButton->Enabled = true;
        OperateButton->Glyph->LoadFromResourceName(0, "RunGraphic");
        ExitOperationButton->Enabled = true;
        TTClockAdjButton->Enabled = true;
        SetRouteButtonsInfoCaptionAndRouteNotStarted(3);
        SetPausedOrZoomedInfoCaption(4);
        if(Level2OperMode == Paused)
            DisableRouteButtons(3); // could be PreStart or Paused
        TrainController->TTClockTime = TrainController->RestartTime;
        PauseEntryRestartTime = double(TrainController->RestartTime);
        PauseEntryTTClockSpeed = 1;
        TTClockSpeed = 1;
        TTClockSpeedLabel->Caption = "x1";
        TrainController->SetWarningFlags(0);
        ShowPerformancePanel = false; // added at v2.2.0
        ShowOperatorActionPanel = false; // new at v2.2.0
        TrainController->OpActionPanelHintDelayCounter = 0; // new at v2.2.0 to reset hint dela
        OAListBox->Clear();
        OAListBox->Items->Add(L""); // hints for OpActionPanel
        OAListBox->Items->Add(L"");
        OAListBox->Items->Add(L"");
        OAListBox->Items->Add(L"Left click");
        OAListBox->Items->Add(L"headcode");
        OAListBox->Items->Add(L"to locate train");
        OAListBox->Items->Add(L"");
        OAListBox->Items->Add(L"");
        OAListBox->Items->Add(L"");
        OAListBox->Items->Add(L"");
        OAListBox->Items->Add(L"Left click and");
        OAListBox->Items->Add(L"hold grey area");
        OAListBox->Items->Add(L"to move panel");
        if((TrainController->AvHoursIntValue > 0) || (Level2OperMode == PreStart)) // only visible if already set or if still in prestart mode
        {
            MTBFEditBox->Visible = true;
            if(TrainController->AvHoursIntValue > 0)
            {
                MTBFEditBox->Text = AnsiString(TrainController->AvHoursIntValue);
            }
            else
            {
                MTBFEditBox->Text = "";
            }
            MTBFEditBox->ReadOnly = false; // because this is still prestart mode
            MTBFLabel->Visible = true;
            MTBFLabel->Caption = "Mean time between\ntrain failures in\ntimetable hours";
            TrainController->MTBFHours = double(TrainController->AvHoursIntValue) / TTClockSpeed;
        }
        else
        {
            MTBFEditBox->Visible = false;
            MTBFEditBox->Text = "";
            MTBFEditBox->ReadOnly = true; // because this is not prestart mode
            MTBFLabel->Visible = false;
            MTBFLabel->Caption = "Mean time between\ntrain failures in\ntimetable hours";
            TrainController->MTBFHours = 0;
        }
        break;

    default:
        // No further recursion in BaseMode so OK
        Level1Mode = BaseMode;
        SetLevel1Mode(29);
        break;
    }
    Utilities->CallLogPop(103);
}

// ---------------------------------------------------------------------------

void TInterface::SetLevel2TrackMode(int Caller)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",SetLevel2TrackMode");
    if(Level1Mode != TrackMode)
    {
        // No further recursion in BaseMode so OK
        Level1Mode = BaseMode;
        SetLevel1Mode(20);
        Utilities->CallLogPop(1115);
        return;
    }
    if(Level2TrackMode == NoTrackMode)
    {
        Utilities->CallLogPop(104);
        return;
    }
    switch(Level2TrackMode) // use the data member
    {
    case AddTrack:
        ResetCurrentSpeedButton(1);
        InfoPanel->Visible = true;
        InfoPanel->Caption = "ADDING TRACK:  Select element then left click to add it.  Right click an element to remove it.";
        LengthConversionPanel->Visible = false; // in case had been in distance setting mode
        SpeedConversionPanel->Visible = false; // in case had been in distance setting mode
        TrackElementPanel->Visible = true;
        TrackElementPanel->Enabled = true;
        SigAspectButton->Visible = true;
        SigAspectButton->Enabled = true;
        ClearandRebuildRailway(34); // to replot grid if required & clear any other unwanted items
        SetTrackBuildImages(4);
        SetLengthsButton->Enabled = false;
        if(Track->IsTrackFinished()) // can only set lengths for several elements together if TrackFinished
        {
            SetLengthsButton->Enabled = true;
        }
        UserGraphicReselectPanel->Visible = false;
        SelectLengthsFlag = false; // in case still set though probably won't be
        break;

    case AddGraphic:
        InfoPanel->Visible = true;
        InfoPanel->Caption = "ADDING GRAPHIC:  Left click layout to add SELECTED graphic, right click to remove ANY graphic.";
        break;

    case SelectGraphic:
        InfoPanel->Visible = true;
        InfoPanel->Caption = "SELECTING USER GRAPHIC:  Select the graphic file then add as many as necessary to the layout.";
        break;

    case GapSetting:
        int HLoc, VLoc, Count;
        Count = Track->NumberOfGaps(0);
        if(div(Count, 2).rem == 1) // condition OK
        {
            ShowMessage("Can't connect, there are an odd number of gaps");
            Level1Mode = TrackMode;
            SetLevel1Mode(77);
            Level2TrackMode = AddTrack;
            // No further recursion in AddTrack so OK
            SetLevel2TrackMode(40);
            Utilities->CallLogPop(105);
            return;
        }
        if(!HighLightOneGap(2, HLoc, VLoc)) // condition OK
        // need to call this here to start gap setting process off,
        // called in MainScreenMouseDown hereafter.  Function returns false for either a LocError (links not yet
        // complete) or no more gaps to be highlighted
        {
            // shouldn't reach here as later gaps covered in MainScreenMouseDown but leave & give error message
            ShowMessage("Error - Even number of gaps but all set after first call to HighLightOneGap");
            Level1Mode = TrackMode;
            SetLevel1Mode(78);
            Level2TrackMode = AddTrack;
            // No further recursion in AddTrack so OK
            SetLevel2TrackMode(41);
            Utilities->CallLogPop(106);
            return; // all gaps set
        }
        InfoPanel->Visible = true;
        InfoPanel->Caption = "CONNECTING GAPS:  Click on connecting gap";
        UserGraphicReselectPanel->Visible = false;
        SetTrackBuildImages(5);
        break;

    case AddText:
        InfoPanel->Visible = true;
        InfoPanel->Caption = "ADDING/EDITING TEXT: Left click to add, left click first letter to edit (+CR), or remove (+Esc)";
        if(TextHandler->TextVectorSize(13) > 0)
        {
            MoveTextOrGraphicButton->Enabled = true;
        }
        else
        {
            MoveTextOrGraphicButton->Enabled = false;
        }
        UserGraphicReselectPanel->Visible = false;
        ClearandRebuildRailway(58); // to drop DistanceKey if was displayed
        break;

    case MoveTextOrGraphic:
        InfoPanel->Visible = true;
        InfoPanel->Caption = "MOVING TEXT OR GRAPHIC: If text left click first letter, if graphic left click anywhere, then drag";
        UserGraphicReselectPanel->Visible = false;
        ClearandRebuildRailway(59); // to drop DistanceKey if was displayed
        break;

    case AddLocationName:
        InfoPanel->Visible = true;
        InfoPanel->Caption = "NAMING LOCATIONS:  Click on location element to add or change name";
        ClearandRebuildRailway(35); // to get rid of earlier red rectangle
        UserGraphicReselectPanel->Visible = false;
        SetTrackBuildImages(12);
        break;

    case DistanceStart:
        InfoPanel->Visible = true;
        InfoPanel->Caption = "DISTANCE/SPEED SETTING:  Select first location (only non-default elements marked)";
        DistanceKey->Visible = true;
        LengthConversionPanel->Visible = true;
        SpeedConversionPanel->Visible = true;
        UserGraphicReselectPanel->Visible = false;
        ClearandRebuildRailway(36); // to get rid of earlier unwanted markings
        break;

    case DistanceContinuing:
        InfoPanel->Visible = true;
        if(ConstructPrefDir->PrefDirSize() == 1)
            InfoPanel->Caption = "DISTANCE/SPEED SETTING:  Select next location";
        else
            InfoPanel->Caption = "DISTANCE/SPEED SETTING:  Continue or set values (overall length), or right click to cancel/truncate";
        UserGraphicReselectPanel->Visible = false;
        ClearandRebuildRailway(54); // to remove earlier end marker if present
        break;

    case TrackSelecting:
        Track->CopyFlag = false;
        if(!SelectionValid)
            ResetSelectRect(); // so a viewpoint change before a new SelectRect chosen doesn't redisplay
                                                // the old SelectRect (only called when entered from SelectMenuItemClick, & not from
                                                // ReselectMenuItemClick)
        InfoPanel->Visible = true;
        InfoPanel->Caption = "SELECTING:  Select area - click left mouse && drag";
        SelectMenuItem->Enabled = false;
        ReselectMenuItem->Enabled = false;
        CancelSelectionMenuItem->Enabled = true;
        UserGraphicReselectPanel->Visible = false;
        break;

    case CopyMoving:
        Track->CopyFlag = true;
        InfoPanel->Visible = true;
        InfoPanel->Caption = "COPYING:  Left click in selection && drag";
        CutMenuItem->Enabled = false;
        CopyMenuItem->Enabled = false;
        FlipMenuItem->Enabled = false;
        MirrorMenuItem->Enabled = false;
        RotRightMenuItem->Enabled = false;
        RotLeftMenuItem->Enabled = false;
        RotateMenuItem->Enabled = false;
        PasteMenuItem->Enabled = true;
// PasteWithAttributesMenuItem->Enabled = false;  //new at v2.2.0 - don't allow the option if copying
        DeleteMenuItem->Enabled = false;
        SelectLengthsMenuItem->Enabled = false;
        SelectBiDirPrefDirsMenuItem->Visible = false;
        CancelSelectionMenuItem->Enabled = false;
        SelectBitmapHLoc = SelectRect.left;
        SelectBitmapVLoc = SelectRect.top;
        SetTrackBuildImages(6);
        UserGraphicReselectPanel->Visible = false;
        break;

    case CutMoving:
        { // have to use braces as otherwise the default case bypasses the initialisation of these local variables
         // erase track elements within selected region
            Track->CopyFlag = false;
            bool EraseSuccessfulFlag, NeedToLink = false, TextChangesMade = false, GraphicChangesMade = false; ;
            int ErasedTrackVectorPosition;
            Screen->Cursor = TCursor(-11); // Hourglass;
            InfoPanel->Visible = true;
            InfoPanel->Caption = "CUT PROCESSING: Please do not click the mouse";
            InfoPanel->Update();
            for(int H = SelectRect.left; H < SelectRect.right; H++)
            {
                for(int V = SelectRect.top; V < SelectRect.bottom; V++)
                {
                    Track->EraseTrackElement(2, H, V, ErasedTrackVectorPosition, EraseSuccessfulFlag, false);
                    if(EraseSuccessfulFlag)
                    {
                        if(ErasedTrackVectorPosition > -1)
                            EveryPrefDir->RealignAfterTrackErase(1, ErasedTrackVectorPosition);
                        NeedToLink = true;
                    }
                }
            }
        // erase text elements within selected region
            int LowSelectHPos = SelectRect.left * 16;
            int HighSelectHPos = SelectRect.right * 16;
            int LowSelectVPos = SelectRect.top * 16;
            int HighSelectVPos = SelectRect.bottom * 16;
            if(!TextHandler->TextVector.empty()) // skip iteration if empty else have an error
            {
                for(TTextHandler::TTextVectorIterator TextPtr = (TextHandler->TextVector.end() - 1); TextPtr >= TextHandler->TextVector.begin();
                TextPtr--) // reverse to prevent skipping during erase
                {
                    if((TextPtr->HPos >= LowSelectHPos) && (TextPtr->HPos < HighSelectHPos) && (TextPtr->VPos >= LowSelectVPos) && (TextPtr->VPos <
                        HighSelectVPos))
                    {
                        if(TextHandler->TextErase(1, TextPtr->HPos, TextPtr->VPos))
                        {;
                        } // unused condition
                        TextChangesMade = true;
                    }
                }
            }
        // erase graphic elements that fall wholly within region to be overwritten
            if(!Track->UserGraphicVector.empty()) // skip iteration if empty else have an error
            {
                for(TTrack::TUserGraphicVector::iterator GraphicPtr = (Track->UserGraphicVector.end() - 1); GraphicPtr >= Track->UserGraphicVector.begin();
                GraphicPtr--) // reverse to prevent skipping during erase
                {
                    if((GraphicPtr->HPos >= LowSelectHPos) && ((GraphicPtr->HPos + GraphicPtr->Width) < HighSelectHPos) && (GraphicPtr->VPos >= LowSelectVPos)
                        && ((GraphicPtr->VPos + GraphicPtr->Height) < HighSelectVPos))
                    {
                        Track->UserGraphicVector.erase(GraphicPtr);
                        GraphicChangesMade = true;
                    }
                }
            }
            Track->CheckMapAndTrack(11); // test
            Track->CheckMapAndInactiveTrack(10); // test
            Track->CheckLocationNameMultiMap(19); // test
            Screen->Cursor = TCursor(-2); // Arrow;
        // Track->SetTrackFinished(!NeedToLink);  This is an error (see Sam Wainwright email of 24/08/17 & devhistory.txt
        // if track not linked to begin with then becomes linked if NeedToLink false
            if(NeedToLink)
                Track->SetTrackFinished(false); // corrected for v2.1.0
            InfoPanel->Caption = "CUTTING:  Left click in selection && drag";
            CutMenuItem->Enabled = false;
            CopyMenuItem->Enabled = false;
            FlipMenuItem->Enabled = false;
            MirrorMenuItem->Enabled = false;
            RotRightMenuItem->Enabled = false;
            RotLeftMenuItem->Enabled = false;
            RotateMenuItem->Enabled = false;
            PasteMenuItem->Enabled = true;
// PasteWithAttributesMenuItem->Enabled = true;  //new at v2.2.0 - option enabled if cutting
            DeleteMenuItem->Enabled = false;
            SelectLengthsMenuItem->Enabled = false;
            SelectBiDirPrefDirsMenuItem->Visible = false;
            CancelSelectionMenuItem->Enabled = false;
            SelectBitmapHLoc = SelectRect.left;
            SelectBitmapVLoc = SelectRect.top;
            if(NeedToLink || TextChangesMade || GraphicChangesMade)
            {
                ResetChangedFileDataAndCaption(20, true); // true for NonPrefDirChangesMade
            }
            ClearandRebuildRailway(37); // to overplot the erased elements with SelectBitmap
            UserGraphicReselectPanel->Visible = false;
            SetTrackBuildImages(7);
        } break;

    case Pasting:
        { // have to use braces as otherwise the default case bypasses the initialisation of these local variables
            Track->SkipLocationNameMultiMapCheck = true;
            ResetChangedFileDataAndCaption(17, true);
            int HDiff = SelectBitmapHLoc - SelectRect.left;
            int VDiff = SelectBitmapVLoc - SelectRect.top;
            bool NeedToLink = false;
            bool TrackLinkingRequiredFlag;
            Screen->Cursor = TCursor(-11); // Hourglass;
            InfoPanel->Visible = true;
            InfoPanel->Caption = "PASTING: Please wait";
            InfoPanel->Update();
// erase track elements
            int LowSelectHLoc = SelectBitmapHLoc;
            int HighSelectHLoc = SelectBitmapHLoc + (SelectBitmap->Width / 16);
            int LowSelectVLoc = SelectBitmapVLoc;
            int HighSelectVLoc = SelectBitmapVLoc + (SelectBitmap->Height / 16);
            bool TrackEraseSuccessfulFlag; // needed but not used here
            int ErasedTrackVectorPosition;
// new quick method of erasing, only need H & V values
            for(int x = LowSelectHLoc; x < HighSelectHLoc; x++)
            {
                for(int y = LowSelectVLoc; y < HighSelectVLoc; y++)
                {
                    Track->EraseTrackElement(5, x, y, ErasedTrackVectorPosition, TrackEraseSuccessfulFlag, false);
                    if(ErasedTrackVectorPosition > -1)
                        EveryPrefDir->RealignAfterTrackErase(2, ErasedTrackVectorPosition);
                }
            }

// erase text elements that fall within region to be overwritten
            int LowSelectHPos = SelectBitmapHLoc * 16;
            int HighSelectHPos = (SelectBitmapHLoc * 16) + SelectBitmap->Width;
            int LowSelectVPos = SelectBitmapVLoc * 16;
            int HighSelectVPos = (SelectBitmapVLoc * 16) + SelectBitmap->Height;
            if(!TextHandler->TextVector.empty()) // skip iteration if empty else have an error
            {
                for(TTextHandler::TTextVectorIterator TextPtr = (TextHandler->TextVector.end() - 1); TextPtr >= TextHandler->TextVector.begin();
                TextPtr--) // reverse to prevent skipping during erase
                {
                    if((TextPtr->HPos >= LowSelectHPos) && (TextPtr->HPos < HighSelectHPos) && (TextPtr->VPos >= LowSelectVPos) && (TextPtr->VPos <
                        HighSelectVPos))
                    {
                        if(TextHandler->TextErase(2, TextPtr->HPos, TextPtr->VPos))
                        {;
                        } // unused condition
                    }
                }
            }
// erase graphic elements that fall wholly within region to be overwritten
            if(!Track->UserGraphicVector.empty()) // skip iteration if empty else have an error
            {
                for(TTrack::TUserGraphicVector::iterator GraphicPtr = (Track->UserGraphicVector.end() - 1); GraphicPtr >= Track->UserGraphicVector.begin();
                GraphicPtr--) // reverse to prevent skipping during erase
                {
                    if((GraphicPtr->HPos >= LowSelectHPos) && ((GraphicPtr->HPos + GraphicPtr->Width) < HighSelectHPos) && (GraphicPtr->VPos >= LowSelectVPos)
                        && ((GraphicPtr->VPos + GraphicPtr->Height) < HighSelectVPos))
                    {
                        Track->UserGraphicVector.erase(GraphicPtr);
                    }
                }
            }
        // change the H & V values in SelectVector to the new positions in case Reselect chosen
            for(unsigned int x = 0; x < Track->SelectVectorSize(); x++)
            {
                Track->SelectVectorAt(35, x).HLoc += HDiff;
                Track->SelectVectorAt(1, x).VLoc += VDiff;
            }

        // add the new track elements
            for(unsigned int x = 0; x < Track->SelectVectorSize(); x++)
            {
                if(Track->CopyFlag) // blank all names if copying, lengths & speedlimits stay
                {
                    Track->SelectVectorAt(80, x).LocationName = "";
                    Track->SelectVectorAt(81, x).ActiveTrackElementName = "";
                }
                bool InternalChecks = false;
// if(Track->PastingWithAttributes) //new at v2.2.0 to select the new funtion & skip multimap checks //drop in v2.4.0
// {
                Track->PlotPastedTrackElementWithAttributes(0, Track->SelectVectorAt(2, x), Track->SelectVectorAt(3, x).HLoc, Track->SelectVectorAt(4, x).VLoc,
                    TrackLinkingRequiredFlag, InternalChecks);
            // new at v2.2.0 & used in place of PlotAndAddTrackElement to keep length & speed values
// }
/* drop this in v2.4.0 as all pastes are past with attributes
            else //'Aspect' parameter added to PlotAndAdd... at v2.2.0 so can plot signals correctly (always four-aspect before)
            {
                int Aspect;
                if(Track->SelectVectorAt(15, x).TrackType != SignalPost) Aspect = 0;  //if an '0' value appears with a SignalPost then must be adding track
                //this combination allows the funtion to distinguish between adding track and plotting with attributes
                else if(Track->SelectVectorAt(16, x).SigAspect == TTrackElement::GroundSignal) Aspect = 1;
                else if(Track->SelectVectorAt(17, x).SigAspect == TTrackElement::TwoAspect) Aspect = 2;
                else if(Track->SelectVectorAt(18, x).SigAspect == TTrackElement::ThreeAspect) Aspect = 3;
                else Aspect = 4;
                Track->PlotAndAddTrackElement(2, Track->SelectVectorAt(19, x).SpeedTag, Aspect, Track->SelectVectorAt(20, x).HLoc, Track->SelectVectorAt(21, x).VLoc, TrackLinkingRequiredFlag, InternalChecks);
            }
*/
                if(TrackLinkingRequiredFlag)
                    NeedToLink = true;
            }

            if(!TextHandler->SelectTextVector.empty()) // skip iteration if empty else have an error
            {
                for(TTextHandler::TTextVectorIterator TextPtr = TextHandler->SelectTextVector.begin(); TextPtr < TextHandler->SelectTextVector.end(); TextPtr++)
                {
                    TextPtr->HPos += HDiff * 16;
                    TextPtr->VPos += VDiff * 16;
                    AnsiString TempString = TextPtr->TextString;
                // have to create a new TextItem in order to create a new Font object
/* drop in v2.4.0 as all pastes are paste with attributes
                if(!Track->PastingWithAttributes) //new at v2.2.0 to deal with the new location prefix '##**'  //drop in v2.4.0
                {
                    if(TextPtr->TextString.SubString(1,4) != "##**") //added for named locations so can delete in a simple paste but
                                                                     //use in PastingWithAttributes
                    {
                        TTextItem TextItem(TextPtr->HPos, TextPtr->VPos, TextPtr->TextString, TextPtr->Font);
                        TextHandler->TextVectorPush(0, TextItem); //if a normal paste include normal text but not location text
                    }
                    else TextPtr->TextString = "";  //delete the name for a simple paste
                }
*/
// else //if pasting with attributes paste all text but strip the '##**' prefix if present
// {
                    if(TextPtr->TextString.SubString(1, 4) == "##**")
                    {
                        TempString = TextPtr->TextString.SubString(5, TextPtr->TextString.Length()); // don't change SelectTextVector value
                        if(Track->CopyFlag)
                        {
                            TextPtr->TextString = ""; // change SelectTextVector value as reselect shouldn't have locations if copied
                            TempString = "";
                        }
                    }
                    TTextItem TextItem(TextPtr->HPos, TextPtr->VPos, TempString, TextPtr->Font);
                    TextHandler->TextVectorPush(4, TextItem);
// }
                }
            }
        // add new graphic items
            if(!Track->SelectGraphicVector.empty()) // skip iteration if empty else have an error
            { // keep contents of SelectVector valid in case reselect
                for(TTrack::TUserGraphicVector::iterator GraphicPtr = Track->SelectGraphicVector.begin(); GraphicPtr < Track->SelectGraphicVector.end();
                GraphicPtr++)
                {
                    GraphicPtr->HPos += HDiff * 16; // for reselect
                    GraphicPtr->VPos += VDiff * 16; // for reselect
                    Track->UserGraphicVector.push_back(*GraphicPtr);
                }
            }
            Track->SkipLocationNameMultiMapCheck = false; // renamed in v2.4.0 - reset the flag after pasting complete, otherwise multimap checks always skipped
            Track->CopyFlag = false;
            Track->CheckMapAndTrack(7); // test
            Track->CheckMapAndInactiveTrack(7); // test
            Track->CheckLocationNameMultiMap(7); // test
        // Track->SetTrackFinished(!NeedToLink);  This is an error (see Sam Wainwright email of 24/08/17 & devhistory.txt
        // if track not linked to begin with then becomes linked if NeedToLink false
            if(NeedToLink)
                Track->SetTrackFinished(false); // corrected for v2.1.0
            Screen->Cursor = TCursor(-2); // Arrow;
            SetTrackBuildImages(14);
            ClearandRebuildRailway(38);
            Level1Mode = TrackMode;
            SetLevel1Mode(79);
            Level2TrackMode = AddTrack;
        // No further recursion in AddTrack so OK
            UserGraphicReselectPanel->Visible = false;
            SetLevel2TrackMode(42);
        } break;

    case Deleting:
        { // have to use braces as otherwise the default case bypasses the initialisation of these local variables
            Track->CopyFlag = false;
            UnicodeString MessageStr = "Selected area will be deleted - proceed?";
            int button = Application->MessageBox(MessageStr.c_str(), L"Please confirm", MB_YESNO);
            if(button == IDNO)
            {
                break;
            }
            bool EraseSuccessfulFlag, NeedToLink = false, TextChangesMade = false, GraphicChangesMade = false;
            int ErasedTrackVectorPosition;
            Screen->Cursor = TCursor(-11); // Hourglass;
            InfoPanel->Visible = true;
            InfoPanel->Caption = "DELETING: Please wait";
            InfoPanel->Update();
            for(int H = SelectRect.left; H < SelectRect.right; H++)
            {
                for(int V = SelectRect.top; V < SelectRect.bottom; V++)
                {
                    Track->EraseTrackElement(3, H, V, ErasedTrackVectorPosition, EraseSuccessfulFlag, false);
                    if(EraseSuccessfulFlag)
                    {
                        if(ErasedTrackVectorPosition > -1)
                            EveryPrefDir->RealignAfterTrackErase(3, ErasedTrackVectorPosition);
                        NeedToLink = true;
                    }
                }
            }
        // erase text elements that fall within selected region
            int LowSelectHPos = SelectRect.left * 16;
            int HighSelectHPos = SelectRect.right * 16;
            int LowSelectVPos = SelectRect.top * 16;
            int HighSelectVPos = SelectRect.bottom * 16;
            if(!TextHandler->TextVector.empty()) // skip iteration if empty else have an error
            {
                for(TTextHandler::TTextVectorIterator TextPtr = (TextHandler->TextVector.end() - 1); TextPtr >= TextHandler->TextVector.begin();
                TextPtr--) // reverse to prevent skipping during erase
                {
                    AnsiString Check = TextPtr->TextString;
                    if((TextPtr->HPos >= LowSelectHPos) && (TextPtr->HPos < HighSelectHPos) && (TextPtr->VPos >= LowSelectVPos) && (TextPtr->VPos <
                        HighSelectVPos))
                    {
                        if(TextHandler->TextErase(3, TextPtr->HPos, TextPtr->VPos))
                        {;
                        } // unused condition
                        TextChangesMade = true;
                    }
                }
            }
        // erase graphic elements that fall within selected region
            if(!Track->UserGraphicVector.empty()) // skip iteration if empty else have an error
            {
                for(TTrack::TUserGraphicVector::iterator GraphicPtr = (Track->SelectGraphicVector.end() - 1); GraphicPtr >= Track->SelectGraphicVector.begin();
                GraphicPtr--) // reverse to prevent skipping during erase
                {
                    if((GraphicPtr->HPos >= LowSelectHPos) && ((GraphicPtr->HPos + GraphicPtr->Width) < HighSelectHPos) && (GraphicPtr->VPos >= LowSelectVPos)
                        && ((GraphicPtr->VPos + GraphicPtr->Height) < HighSelectVPos))
                    {
                        for(TTrack::TUserGraphicVector::iterator UserGraphicPtr = (Track->UserGraphicVector.end() - 1);
                        UserGraphicPtr >= Track->UserGraphicVector.begin(); UserGraphicPtr--) // reverse to prevent skipping during erase
                        {
                            if((UserGraphicPtr->HPos == GraphicPtr->HPos) && (UserGraphicPtr->VPos == GraphicPtr->VPos) &&
                                (UserGraphicPtr->Width == GraphicPtr->Width) && (UserGraphicPtr->Height == GraphicPtr->Height) &&
                                (UserGraphicPtr->FileName == GraphicPtr->FileName))
                            {
                                Track->UserGraphicVector.erase(UserGraphicPtr);
                                GraphicChangesMade = true;
                            }
                        }
                    }
                }
            }
        // clear the selectvectors
            Track->SelectVectorClear();
            TextHandler->SelectTextVector.clear();
            Track->SelectGraphicVector.clear();
            Track->CheckMapAndTrack(10); // test
            Track->CheckMapAndInactiveTrack(9); // test
            Track->CheckLocationNameMultiMap(15); // test
        // Track->SetTrackFinished(!NeedToLink);  This is an error (see Sam Wainwright email of 24/08/17 & devhistory.txt
        // if track not linked to begin with then becomes linked if NeedToLink false
            if(NeedToLink)
                Track->SetTrackFinished(false); // corrected for v2.1.0
            if(NeedToLink || TextChangesMade || GraphicChangesMade)
            {
                ResetChangedFileDataAndCaption(21, true); // true for NonPrefDirChangesMade
            }
            Screen->Cursor = TCursor(-2); // Arrow;
            ClearandRebuildRailway(39);
            Level1Mode = TrackMode;
            SetLevel1Mode(80);
            Level2TrackMode = AddTrack;
        // No further recursion in AddTrack so OK
            UserGraphicReselectPanel->Visible = false;
            SetLevel2TrackMode(43);
        } break;

    default:
        // No further recursion in TrackMode so OK
        Track->CopyFlag = false;
        Level1Mode = TrackMode;
        SetLevel1Mode(21);
        UserGraphicReselectPanel->Visible = false;
        break;
    }
    Utilities->CallLogPop(107);
}

// ---------------------------------------------------------------------------

void TInterface::SetLevel2PrefDirMode(int Caller)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",SetLevel2PrefDirMode");
    if(Level1Mode != PrefDirMode)
    {
        // No further recursion in BaseMode so OK
        Level1Mode = BaseMode;
        SetLevel1Mode(22);
        Utilities->CallLogPop(108);
        return;
    }
    if(Level2PrefDirMode == NoPrefDirMode)
    {
        Utilities->CallLogPop(109);
        return;
    }

    switch(Level2PrefDirMode) // use the data member
    {
    case PrefDirContinuing:
        { // have to use braces as otherwise the default case bypasses the initialisation of these local variables
            InfoPanel->Visible = true;
            if(!Display->ZoomOutFlag) // can't set focus if zoomed out, get an error - added this condition for v0.4d
            {
                AddPrefDirButton->Enabled = true; // this and the line below are to remove focus from any other button that might have it, prior to
                AddPrefDirButton->SetFocus(); // disabling the AddPrefDir button, so pressing enter does nothing, it is reset to the AddPrefDir
            }
            AddPrefDirButton->Enabled = false; // button later if it becomes enabled
            DeleteOnePrefDirButton->Enabled = false;
            bool LeadingPointsAtLastElement = false;
            if(!ConstructPrefDir->EndPossible(0, LeadingPointsAtLastElement))
            {
                if(LeadingPointsAtLastElement) // size must be > 1
                {
                    InfoPanel->Caption = "PREFERRED DIRECTION SETTING:  Can't end on leading points, select next location or truncate";
                    DeleteOnePrefDirButton->Enabled = true;
                }
                else // size == 1, DeleteOnePrefDirButton->Enabled remains false
                {
                    InfoPanel->Caption = "PREFERRED DIRECTION SETTING:  Select next preferred direction location (right click to truncate)";
                }
            }
            else // size > 1 & EndPossible
            {
                InfoPanel->Caption = "PREFERRED DIRECTION SETTING:  Add selection or select next location (right click to truncate)";
                if(!Display->ZoomOutFlag) // can't set focus if zoomed out, get an error - added this condition for v0.4d
                {
                    AddPrefDirButton->Enabled = true;
                    AddPrefDirButton->SetFocus(); // so can just press 'Enter' key
                }
                DeleteOnePrefDirButton->Enabled = true;
            }
            ExitPrefDirButton->Enabled = true;
            ClearandRebuildRailway(40); // to show truncated PrefDirs
        } break;

    case PrefDirSelecting:
        ResetSelectRect(); // so a viewpoint change before a new SelectRect chosen doesn't redisplay the old SelectRect
        InfoPanel->Visible = true;
        InfoPanel->Caption = "SELECTING:  Select area - click left mouse && drag";
        SelectMenuItem->Enabled = false;
        ReselectMenuItem->Enabled = false;
        CancelSelectionMenuItem->Enabled = true;
        break;

    default:
        // No further recursion in PrefDirMode so OK
        Level1Mode = PrefDirMode;
        SetLevel1Mode(23);
        break;
    }
    Utilities->CallLogPop(110);
}

// ---------------------------------------------------------------------------

void TInterface::SetLevel2OperMode(int Caller)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",SetLevel2OperMode");
    if(Level1Mode != OperMode)
    {
        // No further recursion in BaseMode so OK
        Level1Mode = BaseMode;
        SetLevel1Mode(24);
        Utilities->CallLogPop(111);
        return;
    }
    if(Level2OperMode == NoOperMode)
    {
        Utilities->CallLogPop(112);
        return;
    }
    CallingOnButton->Visible = true;
    PresetAutoSigRoutesButton->Visible = false;
    switch(Level2OperMode) // use the data member
    {
    case Operating:
        { // have to use braces as otherwise the default case bypasses the initialisation of local variables
            OperateButton->Enabled = true;
            OperateButton->Glyph->LoadFromResourceName(0, "PauseGraphic");
            ExitOperationButton->Enabled = true;
            TTClockAdjButton->Enabled = false;
            if(TTClockSpeed == 2)
                TTClockSpeedLabel->Caption = "x2";
            else if(TTClockSpeed == 4)
                TTClockSpeedLabel->Caption = "x4";
            else if(TTClockSpeed == 8)
                TTClockSpeedLabel->Caption = "x8";
            else if(TTClockSpeed == 16)
                TTClockSpeedLabel->Caption = "x16";
            else if(TTClockSpeed == 0.5)
                TTClockSpeedLabel->Caption = "x1/2";
            else if(TTClockSpeed == 0.25)
                TTClockSpeedLabel->Caption = "x1/4";
            else if(TTClockSpeed == 0.125)
                TTClockSpeedLabel->Caption = "x1/8";
            else if(TTClockSpeed == 0.0625)
                TTClockSpeedLabel->Caption = "x1/16";
            else
            {
                TTClockSpeed = 1;
                TTClockSpeedLabel->Caption = "x1";
            }
            AnsiString TimeMessage = Utilities->Format96HHMMSS(TDateTime(PauseEntryRestartTime)) + ": ";
            if(TTClockSpeed != PauseEntryTTClockSpeed)
            {
            // send message to performance log
                if(TTClockSpeed == 2)
                    Display->PerformanceLog(6, TimeMessage + "Timetable clock speed changed to twice normal");
                else if(TTClockSpeed == 4)
                    Display->PerformanceLog(7, TimeMessage + "Timetable clock speed changed to four times normal");
                else if(TTClockSpeed == 8)
                    Display->PerformanceLog(8, TimeMessage + "Timetable clock speed changed to eight times normal");
                else if(TTClockSpeed == 16)
                    Display->PerformanceLog(9, TimeMessage + "Timetable clock speed changed to sixteen times normal");
                else if(TTClockSpeed == 0.5)
                    Display->PerformanceLog(10, TimeMessage + "Timetable clock speed changed to half normal");
                else if(TTClockSpeed == 0.25)
                    Display->PerformanceLog(11, TimeMessage + "Timetable clock speed changed to quarter normal");
                else if(TTClockSpeed == 0.125)
                    Display->PerformanceLog(14, TimeMessage + "Timetable clock speed changed to one eighth normal");
                else if(TTClockSpeed == 0.0625)
                    Display->PerformanceLog(15, TimeMessage + "Timetable clock speed changed to one sixteenth normal");
                else
                    Display->PerformanceLog(12, TimeMessage + "Timetable clock speed changed to normal");
            }
            double TTClockTimeChange = double(TrainController->RestartTime) - PauseEntryRestartTime;
            if(TTClockTimeChange > 0.000347) // 30 seconds, min increase is 1 minute & don't trust doubles to stay exactly equal
            {
            // send message to performance log
                int MinsIncrease = ((TTClockTimeChange * 1440) + 0.5); // add 30 secs to ensure truncates correctly
                int HoursIncrease = 0;
                while(MinsIncrease >= 60)
                {
                    HoursIncrease++;
                    MinsIncrease -= 60;
                }
                if(HoursIncrease == 0)
                    TimeMessage += "Timetable clock incremented by " + AnsiString(MinsIncrease) + "m";
                else if(MinsIncrease == 0)
                    TimeMessage += "Timetable clock incremented by " + AnsiString(HoursIncrease) + "h";
                else
                    TimeMessage += "Timetable clock incremented by " + AnsiString(HoursIncrease) + "h " + AnsiString(MinsIncrease) + "m";
                Display->PerformanceLog(13, TimeMessage);
            }
            WarningHover = false;
            SetRouteButtonsInfoCaptionAndRouteNotStarted(4);
            if(TrainController->AvHoursIntValue > 0)
            {
                MTBFEditBox->Visible = true;
                MTBFEditBox->Text = AnsiString(TrainController->AvHoursIntValue);
                MTBFEditBox->ReadOnly = true; // because this is not prestart mode
                MTBFLabel->Visible = true;
                MTBFLabel->Caption = "Mean time between\ntrain failures in\ntimetable hours";
                TrainController->MTBFHours = double(TrainController->AvHoursIntValue) / TTClockSpeed;
            }
            else
            {
                MTBFEditBox->Visible = false;
                MTBFEditBox->Text = "";
                MTBFLabel->Visible = false;
                MTBFLabel->Caption = "Mean time between\ntrain failures in\ntimetable hours";
                TrainController->MTBFHours = 0;
            }
            TrainController->BaseTime = TDateTime::CurrentDateTime();
// StopTTClockFlag already false because TTClock stopped by condition "if(!TrainController->StopTTClockFlag && (Level2OperMode == Operating))" in MasterClockTimer function
        } break;

    case Paused:
        OperateButton->Enabled = true;
        OperateButton->Glyph->LoadFromResourceName(0, "RunGraphic");
        ExitOperationButton->Enabled = true;
        TTClockAdjButton->Enabled = true;
        SetRouteButtonsInfoCaptionAndRouteNotStarted(7);
        DisableRouteButtons(4);
        SetPausedOrZoomedInfoCaption(5);
        TrainController->RestartTime = TrainController->TTClockTime;
// StopTTClockFlag stays false because TTClock stopped by condition "if(!TrainController->StopTTClockFlag && (Level2OperMode == Operating))" in MasterClockTimer function
        PauseEntryRestartTime = double(TrainController->RestartTime);
        PauseEntryTTClockSpeed = TTClockSpeed;
        break;

    // don't need a separate case for PreStart

    default:
        // No further recursion in OperMode so OK
        Level1Mode = OperMode;
        SetLevel1Mode(25);
        break;
    }
    Utilities->CallLogPop(113);
}

// ---------------------------------------------------------------------------

void TInterface::ApproachLocking(int Caller, TDateTime TTClockTime)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",ApproachLocking");
    float LockDelay = 120.0;

    if(!AllRoutes->LockedRouteVector.empty())
    {
        for(TAllRoutes::TLockedRouteVectorIterator LRVIT = AllRoutes->LockedRouteVector.end() - 1; LRVIT >= AllRoutes->LockedRouteVector.begin(); LRVIT--)
        {
            bool BreakFlag = false;
            if(AllRoutes->TrackIsInARoute(5, LRVIT->LastTrackVectorPosition, LRVIT->LastXLinkPos))
            {
                TOneRoute &Route = AllRoutes->GetModifiableRouteAt(0, LRVIT->RouteNumber);
                if((TTClockTime - LRVIT->LockStartTime) > TDateTime(LockDelay / 86400))
                {
                    TrainController->LogEvent("LockedRouteRemoved," + AnsiString(LRVIT->TruncateTrackVectorPosition) + "," +
                        AnsiString(LRVIT->LastTrackVectorPosition));
                    while(Route.LastElementPtr(9)->GetTrackVectorPosition() != LRVIT->TruncateTrackVectorPosition)
                    { // examine the element one earlier in the route than the last
                        if(!(AllRoutes->TrackIsInARoute(6, Route.LastElementPtr(10)->Conn[Route.LastElementPtr(11)->GetELinkPos()],
                            Route.LastElementPtr(12)->ConnLinkPos[Route.LastElementPtr(13)->GetELinkPos()])))
                        {
                            BreakFlag = true;
                        }
                        AllRoutes->RemoveRouteElement(1, Route.LastElementPtr(14)->HLoc, Route.LastElementPtr(15)->VLoc, Route.LastElementPtr(16)->GetELink());
                        if(BreakFlag)
                            break; // train removed earlier element from route so stop here
                    }
                    if(!BreakFlag)
                    { // still need to remove the element at the TruncateTrackVectorPosition
                        if(Route.LastElementPtr(17)->GetTrackVectorPosition() == LRVIT->TruncateTrackVectorPosition)
                        {
                            AllRoutes->RemoveRouteElement(2, Route.LastElementPtr(18)->HLoc, Route.LastElementPtr(19)->VLoc,
                                Route.LastElementPtr(20)->GetELink());
                        }
                    }
                    AllRoutes->CheckMapAndRoutes(10); // test
                    AllRoutes->LockedRouteVector.erase(LRVIT);
                    if(!Display->ZoomOutFlag)
                        ClearandRebuildRailway(17); // to get rid of route graphics
                    RevertToOriginalRouteSelector(13);
                }
            }
            else
            {
                AllRoutes->LockedRouteVector.erase(LRVIT);
                // if end element not in route then a train must have entered it from the wrong end and erased the whole route,
                // hence no longer needed so get rid of it
            }
        }
    }
    Utilities->CallLogPop(743);
}

// ---------------------------------------------------------------------------

void TInterface::ContinuationAutoSignals(int Caller, TDateTime TTClockTime)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",ContinuationAutoSignals");
    if(!TrainController->ContinuationAutoSigVector.empty())
    {
        TTrainController::TContinuationAutoSigVectorIterator AutoSigVectorIT;
        for(AutoSigVectorIT = TrainController->ContinuationAutoSigVector.end() - 1; AutoSigVectorIT >= TrainController->ContinuationAutoSigVector.begin();
        AutoSigVectorIT--)
        {
            // Below added at v2.1.0 to prevent locked autosig continuation routes from clearing signals
            // need to identify the Continuation element in the route & check if it's in a locked route.  If it is then don't call
            // SetTrailingSignalsOnContinuationRoute as all signals must stay red.
            TPrefDirElement TempPrefDirElement;
            int TempLockedVectorNumber;
            int LastRouteElement = AllRoutes->GetFixedRouteAt(220, AutoSigVectorIT->RouteNumber).PrefDirSize() - 1;
            int TVNum = AllRoutes->GetFixedRouteAt(221, AutoSigVectorIT->RouteNumber).GetFixedPrefDirElementAt(246, LastRouteElement).GetTrackVectorPosition();
            // this will be a continuation (error thrown in SetTrailingSignalsOnContinuationRoute if not) & XLinkPos is always 0 for
            // route exiting at a continuation
            if(AllRoutes->IsElementInLockedRouteGetPrefDirElementGetLockedVectorNumber(14, TVNum, 0, TempPrefDirElement, TempLockedVectorNumber))
            {
                continue;
            }
            // end of additions
            if(((TTClockTime - AutoSigVectorIT->PassoutTime) > TDateTime(AutoSigVectorIT->FirstDelay / 86400)) && (AutoSigVectorIT->AccessNumber == 0))
            {
                AllRoutes->SetTrailingSignalsOnContinuationRoute(1, AutoSigVectorIT->RouteNumber, 0);
                AutoSigVectorIT->AccessNumber++;
                continue;
            }
            if(((TTClockTime - AutoSigVectorIT->PassoutTime) > TDateTime(AutoSigVectorIT->SecondDelay / 86400)) && (AutoSigVectorIT->AccessNumber == 1))
            {
                AllRoutes->SetTrailingSignalsOnContinuationRoute(2, AutoSigVectorIT->RouteNumber, 1);
                AutoSigVectorIT->AccessNumber++;
                continue;
            }
            if(((TTClockTime - AutoSigVectorIT->PassoutTime) > TDateTime(AutoSigVectorIT->ThirdDelay / 86400)) && (AutoSigVectorIT->AccessNumber == 2))
            {
                AllRoutes->SetTrailingSignalsOnContinuationRoute(3, AutoSigVectorIT->RouteNumber, 2);
                AutoSigVectorIT->AccessNumber++;
                continue;
            }
        }
        // examine all vector for any expired values & erase
        for(AutoSigVectorIT = TrainController->ContinuationAutoSigVector.end() - 1; AutoSigVectorIT >= TrainController->ContinuationAutoSigVector.begin();
        AutoSigVectorIT--)
        {
            if(AutoSigVectorIT->AccessNumber > 2)
            {
                TrainController->ContinuationAutoSigVector.erase(AutoSigVectorIT); // erase expired entries - reverse interation so OK to erase
            }
        }
    }
    Utilities->CallLogPop(744);
}

// ---------------------------------------------------------------------------

void TInterface::TrackTrainFloat(int Caller)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",TrackTrainFloat");

    TPoint MousePoint = Mouse->CursorPos;
    int ScreenX = MousePoint.x - MainScreen->ClientOrigin.x;
    int ScreenY = MousePoint.y - MainScreen->ClientOrigin.y;

    if((ScreenX > (MainScreen->Width - 1)) || (ScreenY > (MainScreen->Height - 1)) || (ScreenX < 0) || (ScreenY < 0))
    {
        FloatingPanel->Visible = false;
        Utilities->CallLogPop(1432);
        return;
    }

    if(PerformancePanel->Visible)
    {
        if((MousePoint.x >= PerformancePanel->Left) && (MousePoint.x <= (PerformancePanel->Left + PerformancePanel->Width)) &&
            ((MousePoint.y - ClientOrigin.y) >= PerformancePanel->Top) && ((MousePoint.y - ClientOrigin.y) <=
            (PerformancePanel->Top + PerformancePanel->Height)))
        { // dont show floating window if mouse over performance panel
            FloatingPanel->Visible = false;
            Utilities->CallLogPop(1715);
            return;
        }
    }

    if(OperatorActionPanel->Visible) // added at v2.3.0 as showed info from behind panel - thanks to Xeon who notified me in email of 15/10/19
    {
        if((MousePoint.x >= OperatorActionPanel->Left) && (MousePoint.x <= (OperatorActionPanel->Left + OperatorActionPanel->Width)) &&
            ((MousePoint.y - ClientOrigin.y) >= OperatorActionPanel->Top) && ((MousePoint.y - ClientOrigin.y) <=
            (OperatorActionPanel->Top + OperatorActionPanel->Height)))
        { // dont show floating window if mouse over OperatorActionPanel
            FloatingPanel->Visible = false;
            Utilities->CallLogPop(2098);
            return;
        }
    }

    AnsiString TrackFloat = "", TrainStatusFloat = "", TrainTTFloat = "";

    bool ShowTrackFloatFlag = false, ShowTrainStatusFloatFlag = false, ShowTrainTTFloatFlag = false;
    int HLoc, VLoc;

    Track->GetTrackLocsFromScreenPos(4, HLoc, VLoc, ScreenX, ScreenY);

    if(Display->ZoomOutFlag)
    {
        Utilities->CallLogPop(1123);
        return;
    }
    if(TrackInfoOnOffMenuItem->Caption == "Hide")
    {
        bool ActiveTrackFoundFlag = false, InactiveTrackFoundFlag = false, TwoTrack = false;
        AnsiString Length01Str = "", Length23Str = "", SpeedLimit01Str = "", SpeedLimit23Str = "";
        AnsiString StationEntryStopLinkPos1Str = "", StationEntryStopLinkPos2Str = "";
        AnsiString ATrackSN = "", ATrackTN = "", IATrackSN = "", LengthAndSpeedCaption = "";
        AnsiString SigAspectString = ""; // new at version 0.6
        int ActiveVecPos = Track->GetVectorPositionFromTrackMap(5, HLoc, VLoc, ActiveTrackFoundFlag);
        TTrack::TIMPair InactiveVecPositions = Track->GetVectorPositionsFromInactiveTrackMap(3, HLoc, VLoc, InactiveTrackFoundFlag);
        TTrackElement ActiveTrackElement, InactiveTrackElement;
        if(InactiveTrackFoundFlag)
        {
            InactiveTrackElement = Track->InactiveTrackElementAt(32, InactiveVecPositions.first); // only need one for the name
            IATrackSN = InactiveTrackElement.LocationName;
        }
        if(ActiveTrackFoundFlag)
        {
            ActiveTrackElement = Track->TrackElementAt(449, ActiveVecPos);
            ATrackSN = ActiveTrackElement.LocationName;
            StationEntryStopLinkPos1Str = AnsiString(ActiveTrackElement.StationEntryStopLinkPos1);
            StationEntryStopLinkPos2Str = AnsiString(ActiveTrackElement.StationEntryStopLinkPos2);
            ATrackTN = ActiveTrackElement.ActiveTrackElementName;
            if((ATrackTN != "") && (!InactiveTrackFoundFlag || ((InactiveTrackElement.TrackType != Platform) &&
                (InactiveTrackElement.TrackType != NamedNonStationLocation)) ||
                (InactiveTrackElement.LocationName != ActiveTrackElement.ActiveTrackElementName)))
            {
                ShowMessage("Error - Track has timetable name without corresponding plat/named loc");
            }
            if(InactiveTrackFoundFlag && ((InactiveTrackElement.TrackType == Platform) || (InactiveTrackElement.TrackType == NamedNonStationLocation)) &&
                (InactiveTrackElement.LocationName != ActiveTrackElement.ActiveTrackElementName))
            {
                ShowMessage("Error - plat/named loc and track have different names, or plat/named loc named but not track");
            }
            if((ActiveTrackElement.TrackType == Points) || (ActiveTrackElement.TrackType == Bridge) || (ActiveTrackElement.TrackType == Crossover))
            {
                TwoTrack = true;
            }
            Length01Str = AnsiString(ActiveTrackElement.Length01);
            if(Length01Str == "-1")
                Length01Str = "Not Set";
            SpeedLimit01Str = AnsiString(ActiveTrackElement.SpeedLimit01);
            if(SpeedLimit01Str == "-1")
                SpeedLimit01Str = "Not Set";
            if(TwoTrack)
            {
                Length23Str = AnsiString(ActiveTrackElement.Length23);
                if(Length23Str == "-1")
                    Length23Str = "Not Set"; // shouldn't be -1 but leave in
                SpeedLimit23Str = AnsiString(ActiveTrackElement.SpeedLimit23);
                if(SpeedLimit23Str == "-1")
                    SpeedLimit23Str = "Not Set"; // shouldn't be -1 but leave in
                if((ActiveTrackElement.TrackType == Points) && (ActiveTrackElement.SpeedTag < 132))
                {
                    LengthAndSpeedCaption = "Straight track length = " + Length01Str + " m" + '\n' + "Diverging track length = " + Length23Str + " m" + '\n' +
                        "Straight track speed limit = " + SpeedLimit01Str + " km/h" + '\n' + "Diverging track speed limit = " + SpeedLimit23Str + " km/h";
                }
                else if(ActiveTrackElement.TrackType == Points)
                {
                    LengthAndSpeedCaption = "Left diverging track length = " + Length01Str + " m" + '\n' + "Right diverging track length = " + Length23Str +
                        " m" + '\n' + "Left diverging track speed limit = " + SpeedLimit01Str + " km/h" + '\n' + "Right diverging track Speed Limit = " +
                        SpeedLimit23Str + " km/h";
                }
                else if(ActiveTrackElement.TrackType == Crossover)
                // crossover links 0 & 1 = diagonal top left to Bottom right, then horizontal, then vertical
                {
                    if((ActiveTrackElement.SpeedTag == 15) || (ActiveTrackElement.SpeedTag == 46))
                    {
                        LengthAndSpeedCaption = "Horizontal track length = " + Length01Str + " m" + '\n' + "Other track length = " + Length23Str + " m" + '\n' +
                            "Horizontal track speed limit = " + SpeedLimit01Str + " km/h" + '\n' + "Other track speed limit = " + SpeedLimit23Str + " km/h";
                    }
                    else if(ActiveTrackElement.SpeedTag == 47)
                    {
                        LengthAndSpeedCaption = "Horizontal track length = " + Length23Str + " m" + '\n' + "Other track length = " + Length01Str + " m" + '\n' +
                            "Horizontal track speed limit = " + SpeedLimit23Str + " km/h" + '\n' + "Other track speed limit = " + SpeedLimit01Str + " km/h";
                    }
                    else if(ActiveTrackElement.SpeedTag == 45)
                    {
                        LengthAndSpeedCaption = "Vertical track length = " + Length01Str + " m" + '\n' + "Other track length = " + Length23Str + " m" + '\n' +
                            "Vertical track speed limit = " + SpeedLimit01Str + " km/h" + '\n' + "Other track speed limit = " + SpeedLimit23Str + " km/h";
                    }
                    else if(ActiveTrackElement.SpeedTag == 44)
                    {
                        LengthAndSpeedCaption = "Vertical track length = " + Length23Str + " m" + '\n' + "Other track length = " + Length01Str + " m" + '\n' +
                            "Vertical track speed limit = " + SpeedLimit23Str + " km/h" + '\n' + "Other track speed limit = " + SpeedLimit01Str + " km/h";
                    }
                    else if(ActiveTrackElement.SpeedTag == 16)
                    {
                        LengthAndSpeedCaption = "Top left to bottom right track length = " + Length01Str + " m" + '\n' + "Other track length = " + Length23Str +
                            " m" + '\n' + "Top left to bottom right track speed limit = " + SpeedLimit01Str + " km/h" + '\n' + "Other track speed limit = " +
                            SpeedLimit23Str + " km/h";
                    }
                }
                else // bridge
                {
                    LengthAndSpeedCaption = "Top track length = " + Length01Str + " m" + '\n' + "Bottom track length = " + Length23Str + " m" + '\n' +
                        "Top track speed limit = " + SpeedLimit01Str + " km/h" + '\n' + "Bottom track speed limit = " + SpeedLimit23Str + " km/h";
                }
            }
            else
            {
                LengthAndSpeedCaption = "Track length = " + Length01Str + " m" + '\n' + "Track speed limit = " + SpeedLimit01Str + " km/h";
            }
        }
        if(ActiveTrackFoundFlag)
        {
            // note that now the "In timetable..." line removed much of the below could be simplified, but leave as is
            // in case wish to resurrect this line for any reason
            ShowTrackFloatFlag = true;
            if(ATrackTN != "") // has a timetable name & therefore has a valid platform or non-station name
            {
                TrackFloat = "Location = " + ATrackTN + '\n' + LengthAndSpeedCaption + '\n' + "ID = " + AnsiString(ActiveTrackElement.ElementID);
            }
            else if(ATrackSN != "") // no timetable name but location name, i.e. a footcrossing
            {
                TrackFloat = "Location = " + ATrackSN + '\n' + LengthAndSpeedCaption + '\n' + "ID = " + AnsiString(ActiveTrackElement.ElementID);
            }

            else if(InactiveTrackFoundFlag) // no timetable name yet but unnamed inactive element at same location (can't be a parapet if active element there)
            {
                TrackFloat = "Location unnamed\n" + LengthAndSpeedCaption + '\n' + "ID = " + AnsiString(ActiveTrackElement.ElementID);
            }

            else // no timetable or location name, just track
            {
                TrackFloat = LengthAndSpeedCaption + '\n' + "Track Element ID = " + AnsiString(ActiveTrackElement.ElementID);
            }
            if(ActiveTrackElement.TrackType == SignalPost) // new for version 0.6
            {
                if(ActiveTrackElement.SigAspect == TTrackElement::ThreeAspect)
                {
                    SigAspectString = "\nThree-aspect signal";
                }
                else if(ActiveTrackElement.SigAspect == TTrackElement::TwoAspect)
                {
                    SigAspectString = "\nTwo-aspect signal";
                }
                else if(ActiveTrackElement.SigAspect == TTrackElement::GroundSignal)
                {
                    SigAspectString = "\nGround signal";
                }
                else
                {
                    SigAspectString = "\nFour-aspect signal";
                }
                TrackFloat += SigAspectString;
            }
        } // if(ActiveFoundFlag)
        else if(InactiveTrackFoundFlag) // inactive element but no active element,
                                        // i.e. concourse or non-station name at a blank element
        {
            ShowTrackFloatFlag = true;
            if(InactiveTrackElement.TrackType != Parapet)
            {
                if(IATrackSN == "")
                {
                    TrackFloat = "Location unnamed\nID = " + AnsiString(InactiveTrackElement.ElementID);
                }
                else
                {
                    TrackFloat = "Location = " + IATrackSN + '\n' + "ID = " + AnsiString(InactiveTrackElement.ElementID);
                }
            }
            else // it is a parapet, just show the ID
            {
                TrackFloat = "ID = " + AnsiString(InactiveTrackElement.ElementID);
            }
        }
    }
// end of TrackFloat section

    if(Level1Mode == OperMode && ((TrainStatusInfoOnOffMenuItem->Caption == "Hide Status") || (TrainTTInfoOnOffMenuItem->Caption == "Hide Timetable")))
    // if caption is 'Off' label is on
    {
        bool FoundFlag;
        AnsiString FormatOneDPStr = "####0.0";
        AnsiString FormatNoDPStr = "#######0";
// AnsiString Format5DPStr = "####0.00000";     //temporary
        AnsiString MaxBrakeStr = ""; // , EntrySpeedStr="", HalfStr="", FullStr="", MaxAtHalfStr="";//test
        AnsiString SpecialStr = "", MaxSpeedStr = "";
        int VecPos = Track->GetVectorPositionFromTrackMap(6, HLoc, VLoc, FoundFlag);
        if(FoundFlag)
        {
            if(Track->TrackElementAt(450, VecPos).TrainIDOnElement > -1)
            // if a bridge & 2 trains at that position will select the train with TrainIDOnElement set
            {
                TTrain Train = TrainController->TrainVectorAtIdent(1, Track->TrackElementAt(452, VecPos).TrainIDOnElement);
                if(TrainStatusInfoOnOffMenuItem->Caption == "Hide Status")
                {
                    ShowTrainStatusFloatFlag = true;
                    AnsiString HeadCode = "", ServiceReferenceInfo = "", Status = "", CurrSpeedStr = "", BrakePCStr = "", NextStopStr = "", TimeLeftStr = "",
                        TimeToNextMovementStr = "", MassStr = "", PowerStr = "";
                    double CurrSpeed;
                    MassStr = AnsiString::FormatFloat(FormatNoDPStr, ((double)Train.Mass) / 1000); // Te
                    PowerStr = AnsiString::FormatFloat(FormatNoDPStr, Train.PowerAtRail / 1000 / 0.8); // kW
                    if(Train.BeingCalledOn)
                        MaxSpeedStr = "30";
                    else
                        MaxSpeedStr = AnsiString::FormatFloat(FormatNoDPStr, Train.MaxRunningSpeed);
                    TDateTime ElapsedDeltaT = TrainController->TTClockTime - Train.EntryTime;
                    TDateTime FirstHalfTimeDeltaT = Train.ExitTimeHalf - Train.EntryTime;
                    TDateTime SecondHalfTimeDeltaT = Train.ExitTimeFull - Train.EntryTime - FirstHalfTimeDeltaT;
                    TDateTime TimeLeft;
                    double BrakePCRate = Train.BrakeRate * 100.0 / Train.MaxBrakeRate;
                    MaxBrakeStr = AnsiString::FormatFloat(FormatNoDPStr, (Train.MaxBrakeRate * Train.Mass / 9810));
                    HeadCode = Train.HeadCode;
                    if(Train.TrainDataEntryPtr->NumberOfTrains > 1) // Service reference information added at v0.6b
                    {
                        if(Train.RepeatNumber == 0)
                        {
                            if(HeadCode != Train.TrainDataEntryPtr->ServiceReference)
                                ServiceReferenceInfo = "\nFirst service of ref. " + Train.TrainDataEntryPtr->ServiceReference;
                            else
                                ServiceReferenceInfo = "\nFirst service";
                        }
                        else if(HeadCode == Train.TrainDataEntryPtr->ServiceReference)
                            ServiceReferenceInfo = "\nRepeat service no. " + AnsiString(Train.RepeatNumber);
                        else
                            ServiceReferenceInfo = "\nRepeat service no. " + AnsiString(Train.RepeatNumber) + " of ref. " +
                                Train.TrainDataEntryPtr->ServiceReference;
                    }
                    else
                    {
                        if(HeadCode != Train.TrainDataEntryPtr->ServiceReference)
                            ServiceReferenceInfo = "\nService reference " + Train.TrainDataEntryPtr->ServiceReference;
                    }
                    if(Train.Stopped())
                    {
                        if(Train.SignallerStopped)
                            Status = "Stopped on signaller's instruction"; // if stopped for any other reason that will diplay
                        if(Train.NotInService)
                            Status = "Not in service"; // not used so far but leave it in
                        if(Train.StoppedAtBuffers)
                            Status = "Stopped at buffers";
                        if(Train.StoppedAtSignal)
                            Status = "Stopped at signal";
                        if(Train.StoppedForTrainInFront)
                            Status = "Stopped - forward track occupied"; // before station stop as want to display station stop if that set
                        if(Train.StoppedAtLocation)
                            Status = "Stopped at " + Train.ActionVectorEntryPtr->LocationName;
                        if((Train.StoppedAtLocation) && (Train.StoppedForTrainInFront))
                            Status = "Stopped at " + Train.ActionVectorEntryPtr->LocationName + " + forward track occupied";
                        if(Train.StoppedWithoutPower)
                        {
                            if(Train.TrainFailed)
                                Status = "Stopped without power - train failed";
                            else
                                Status = "Stopped without power";
                        }
                        if(Train.StoppedAfterSPAD)
                            Status = "Stopped - signal passed at danger";
                        if(Train.Derailed)
                            Status = "Derailed";
                        if(Train.Crashed)
                            Status = "Crashed";
                        CurrSpeed = 0;
                    }
                    else if(Train.OneLengthAccelDecel)
                    {
                        if(Train.FirstHalfMove)
                        {
                            Status = "Accelerating"; // just display a linear speed rise over half length
                            BrakePCRate = 0; // reset to proper value during braking
                            CurrSpeed = Train.EntrySpeed + ((Train.ExitSpeedHalf - Train.EntrySpeed) * (double(ElapsedDeltaT) / double(FirstHalfTimeDeltaT)));
                        }
                        else
                        {
                            BrakePCRate = Train.BrakeRate * 100.0 / Train.MaxBrakeRate;
                            if(BrakePCRate < 55)
                                Status = "Light braking";
                            else if(BrakePCRate < 90)
                                Status = "Heavy braking";
                            else
                                Status = "Emergency braking";
                            CurrSpeed = Train.ExitSpeedHalf - 3.6 * (Train.BrakeRate * (TrainController->TTClockTime - Train.ExitTimeHalf) * 86400.0);
                        }
                    }
                    else if(Train.BrakeRate > 0.01)
                    {
                        if(BrakePCRate < 55)
                            Status = "Light braking";
                        else if(BrakePCRate < 90)
                            Status = "Heavy braking";
                        else
                            Status = "Emergency braking";
                        CurrSpeed = Train.EntrySpeed - 3.6 * (Train.BrakeRate * ElapsedDeltaT * 86400.0);
                    }

                    else if((Train.BrakeRate <= 0.01) && (Train.ExitSpeedHalf > (Train.EntrySpeed + 0.01)) && Train.FirstHalfMove)
                    {
                        Status = "Accelerating"; // just display a linear speed rise over half length
                        CurrSpeed = Train.EntrySpeed + ((Train.ExitSpeedHalf - Train.EntrySpeed) * (double(ElapsedDeltaT) / double(FirstHalfTimeDeltaT)));
                    }

                    else if((Train.BrakeRate <= 0.01) && (Train.ExitSpeedFull > (Train.ExitSpeedHalf + 0.01)) && !Train.FirstHalfMove)
                    {
                        Status = "Accelerating";
                        CurrSpeed = Train.ExitSpeedHalf +
                            ((Train.ExitSpeedFull - Train.ExitSpeedHalf) * (double(ElapsedDeltaT - FirstHalfTimeDeltaT) / double(SecondHalfTimeDeltaT)));
                    }

                    else if((Train.BrakeRate <= 0.01) && (Train.ExitSpeedFull <= Train.ExitSpeedHalf) && !Train.FirstHalfMove)
                    {
                        if(Train.PowerAtRail < 1)
                        {
                            if(Train.TrainFailed)
                            {
                                Status = "Coasting - train failed";
                            }
                            else
                            {
                                Status = "Coasting - no power";
                            }
                            CurrSpeed = Train.ExitSpeedFull;
                        }
                        else
                        {
                            Status = "Constant speed";
                            CurrSpeed = Train.ExitSpeedFull;
                        }
                    }

                    else // No braking, first half move, ExitSpeedHalf <= EntrySpeed
                    {
                        if(Train.PowerAtRail < 1) // as designed there is no way a vehicle can coast without having failed
                        {
                            if(Train.TrainFailed)
                            {
                                Status = "Coasting - train failed";
                            }
                            else
                            {
                                Status = "Coasting - no power";
                            }
                            CurrSpeed = Train.ExitSpeedHalf;
                        }
                        else
                        {
                            Status = "Constant speed";
                            CurrSpeed = Train.ExitSpeedHalf;
                        }
                    }
                    if(Train.TimetableFinished)
                    {
                        if(Train.TrainMode == Signaller)
                            NextStopStr = "At signaller's discretion";
                        else
                            NextStopStr = "None";
                    }
                    else
                        NextStopStr = Train.FloatingLabelNextString(0, Train.ActionVectorEntryPtr);
                    if(Train.TrainMode == Signaller)
                    {
                        SpecialStr = "Train under signaller control" + AnsiString('\n');
                    }
                    else if(Train.BeingCalledOn && !Train.StoppedAtLocation)
                    {
                        SpecialStr = "Restricted speed - being called on" + AnsiString('\n');
                    }

                    double RemTimeHalf = 86400.0 * double(Train.ExitTimeHalf - TrainController->TTClockTime);
                    if(RemTimeHalf < 0)
                        RemTimeHalf = 0;
                    double RemTimeFull = 86400.0 * double(Train.ExitTimeFull - TrainController->TTClockTime);
                    if(RemTimeFull < 0)
                        RemTimeFull = 0;
                    if(RemTimeHalf > 0)
                        TimeLeft = RemTimeHalf;
                    else
                        TimeLeft = RemTimeFull;
                    TimeToNextMovementStr = "Time to next movement (sec) = " + TimeLeftStr.FormatFloat(FormatOneDPStr, TimeLeft);
                    if(Train.Stopped())
                        TimeToNextMovementStr = "";
                    if(Train.Stopped())
                    {
                        TrainStatusFloat = HeadCode + ": " + Train.TrainDataEntryPtr->Description + ServiceReferenceInfo + '\n' + "Maximum train speed " +
                            MaxSpeedStr + "km/h; Power " + PowerStr + "kW" + '\n' + "Mass " + MassStr + "Te; Brakes " + MaxBrakeStr + "Te" + '\n' + SpecialStr +
                            Status + '\n' + "Next:  " + NextStopStr;
                    }
                    else
                    {
                        TrainStatusFloat = HeadCode + ": " + Train.TrainDataEntryPtr->Description + ServiceReferenceInfo + '\n' + "Maximum train speed " +
                            MaxSpeedStr + "km/h; Power " + PowerStr + "kW" + '\n' + "Mass " + MassStr + "Te; Brakes " + MaxBrakeStr + "Te" + '\n' + SpecialStr +
                            Status + ": " + CurrSpeedStr.FormatFloat(FormatNoDPStr, CurrSpeed) + "km/h" + '\n' + "Next: " + NextStopStr;
                    }
                }
                if(TrainTTInfoOnOffMenuItem->Caption == "Hide Timetable")
                {
                    ShowTrainTTFloatFlag = true;
                    TrainTTFloat = Train.FloatingTimetableString(0, Train.ActionVectorEntryPtr);
                }
            }

            else if(Track->TrackElementAt(666, VecPos).TrackType == Continuation)
            // always give train information if a train present, but if not & either of train status or timetable info
            // selected then give next expected train to enter, or 'No trains expected'
            {
                TrainStatusFloat = "No trains expected";
                TrainTTFloat = "No timetable";
                float EntrySpeed;
                int LineSpeedLimit = Track->TrackElementAt(906, VecPos).SpeedLimit01; // speed only in 01 as a continuation
                if(TrainStatusInfoOnOffMenuItem->Caption == "Hide Status")
                    ShowTrainStatusFloatFlag = true;
                if(TrainTTInfoOnOffMenuItem->Caption == "Hide Timetable")
                    ShowTrainTTFloatFlag = true;
                if(!TrainController->ContinuationTrainExpectationMultiMap.empty())
                {
                    TTrainController::TContinuationTrainExpectationMultiMapIterator CTEIt = TrainController->ContinuationTrainExpectationMultiMap.begin();
                    if(CTEIt != TrainController->ContinuationTrainExpectationMultiMap.end())
                    {
                        while((CTEIt != TrainController->ContinuationTrainExpectationMultiMap.end()) && ((CTEIt->second.VectorPosition != VecPos) ||
                            (CTEIt->second.TrainDataEntryPtr->TrainOperatingDataVector.at(CTEIt->second.RepeatNumber).RunningEntry != NotStarted)))
                        {
                            CTEIt++;
                        }
                        if(CTEIt != TrainController->ContinuationTrainExpectationMultiMap.end())
                        {
                            TTrainDataEntry *TTDEPtr = CTEIt->second.TrainDataEntryPtr;
                            AnsiString ServiceReferenceInfo = "";
                            // Repeat information
                            if(TTDEPtr->NumberOfTrains > 1) // Service reference information
                            {
                                if(CTEIt->second.RepeatNumber == 0)
                                {
                                    if(CTEIt->second.HeadCode != TTDEPtr->ServiceReference)
                                        ServiceReferenceInfo = "\nFirst service of ref. " + TTDEPtr->ServiceReference;
                                    else
                                        ServiceReferenceInfo = "\nFirst service";
                                }
                                else if(CTEIt->second.HeadCode == TTDEPtr->ServiceReference)
                                    ServiceReferenceInfo = "\nRepeat service no. " + AnsiString(CTEIt->second.RepeatNumber);
                                else
                                    ServiceReferenceInfo = "\nRepeat service no. " + AnsiString(CTEIt->second.RepeatNumber) + " of ref. " +
                                        TTDEPtr->ServiceReference;
                            }
                            else
                            {
                                if(CTEIt->second.HeadCode != TTDEPtr->ServiceReference)
                                    ServiceReferenceInfo = "\nService reference " + TTDEPtr->ServiceReference;
                            }
                            if(TTDEPtr->ActionVector.at(0).SignallerControl) // entry at 0 is the start entry
                            {
                                SpecialStr = "\nTrain under signaller control";
                                EntrySpeed = TTDEPtr->SignallerSpeed;
                                if(EntrySpeed > LineSpeedLimit)
                                    EntrySpeed = LineSpeedLimit;
                            }
                            else
                            {
                                EntrySpeed = TTDEPtr->StartSpeed;
                                if(EntrySpeed > LineSpeedLimit)
                                    EntrySpeed = LineSpeedLimit;
                            }
                            if((CTEIt->first + TDateTime(1.0 / 1440)) < TrainController->TTClockTime) // has to be at least 1 min late to show as late
                            {
                                TDateTime TempTime = CTEIt->first;
// need this because CTEIt points to a const object and shouldn't use FormatString on a const object
                                TrainStatusFloat = CTEIt->second.HeadCode + ": " + CTEIt->second.Description + ServiceReferenceInfo + "\nEntry speed " +
                                    AnsiString::FormatFloat(FormatNoDPStr, EntrySpeed) + "km/h" + SpecialStr + "\nDelayed, was due at " +
                                    Utilities->Format96HHMM(TempTime);
                            }
                            else
                            {
                                TDateTime TempTime = CTEIt->first;
// need this because CTEIt points to a const object and shouldn't use FormatString on a const object
                                TrainStatusFloat = CTEIt->second.HeadCode + ": " + CTEIt->second.Description + ServiceReferenceInfo + "\nEntry speed " +
                                    AnsiString::FormatFloat(FormatNoDPStr, EntrySpeed) + "km/h" + SpecialStr + "\nExpected at " +
                                    Utilities->Format96HHMM(TempTime);
                            }
                            if(TrainTTInfoOnOffMenuItem->Caption == "Hide Timetable")
                            {
                                if(!TTDEPtr->ActionVector.at(0).SignallerControl) // if signaller control there's no timetable & SpecialStr covers this
                                {
                                    TrainTTFloat = TrainController->ContinuationEntryFloatingTTString(0, TTDEPtr, CTEIt->second.RepeatNumber,
                                        CTEIt->second.IncrementalMinutes, CTEIt->second.IncrementalDigits);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

// end of TrainFloat section
    AnsiString Caption;

    if(!ShowTrackFloatFlag && !ShowTrainStatusFloatFlag && !ShowTrainTTFloatFlag)
    {
        FloatingPanel->Visible = false;
        Utilities->CallLogPop(1485);
        return; // return with label invisible
    }
    else if(ShowTrackFloatFlag && !ShowTrainStatusFloatFlag && !ShowTrainTTFloatFlag)
    {
        Caption = TrackFloat;
    }
    else if(!ShowTrackFloatFlag && ShowTrainStatusFloatFlag && !ShowTrainTTFloatFlag)
    {
        Caption = TrainStatusFloat;
    }
    else if(ShowTrackFloatFlag && ShowTrainStatusFloatFlag && !ShowTrainTTFloatFlag)
    {
        Caption = TrainStatusFloat + '\n' + '\n' + TrackFloat;
    }
    else if(!ShowTrackFloatFlag && !ShowTrainStatusFloatFlag && ShowTrainTTFloatFlag)
    {
        if(TrainStatusFloat == "No trains expected")
            Caption = TrainStatusFloat;
        else
            Caption = TrainTTFloat;
    }
    else if(ShowTrackFloatFlag && !ShowTrainStatusFloatFlag && ShowTrainTTFloatFlag)
    {
        if(TrainStatusFloat == "No trains expected")
            Caption = TrainStatusFloat + '\n' + '\n' + TrackFloat;
        else
            Caption = TrainTTFloat + '\n' + '\n' + TrackFloat;
    }
    else if(!ShowTrackFloatFlag && ShowTrainStatusFloatFlag && ShowTrainTTFloatFlag)
    {
        if(TrainStatusFloat == "No trains expected")
            Caption = TrainStatusFloat;
        else
            Caption = TrainStatusFloat + '\n' + '\n' + TrainTTFloat;
    }
    else if(ShowTrackFloatFlag && ShowTrainStatusFloatFlag && ShowTrainTTFloatFlag)
    {
        if(TrainStatusFloat == "No trains expected")
            Caption = TrainStatusFloat + '\n' + '\n' + TrackFloat;
        else
            Caption = TrainStatusFloat + '\n' + '\n' + TrainTTFloat + '\n' + '\n' + TrackFloat;
    }

    int Left = ScreenX + MainScreen->Left + 16; // so lhs of window is one element to the right of the mouse pos

// this offset is because window position is relative to the interface form, whereas ScreenX & Y are relative to the MainScreen, which is
// offset 32 to the right and 95 down from the interface form
    if((Left + FloatingPanel->Width) > MainScreen->Left + MainScreen->Width)
        Left = ScreenX - FloatingPanel->Width + 16; // so rhs of window is one element to the left of the mouse pos (+32 would be at mouse pos)
    int Top = ScreenY + MainScreen->Top + 16; // so top of window is one element below the mouse pos (ScreenY + MainScreen->Top would be at mouse pos)

    if((Top + FloatingPanel->Height) > MainScreen->Top + MainScreen->Height)
    {
        Top = ScreenY - FloatingPanel->Height + 79; // so bottom of window is one element above the mouse pos (95 would be at mouse pos)
        // but, top may now be off the top of the screen, if so position at the top of the screen, as always need to see the top, if have to
        // lose something then it's best to be from the bottom
        if(Top < 30) // use 30 instead of MainScreen->Top [95] as top can go off MainScreen providing it doesn't reach the information panel, as that would
                     // obscure the window
        {
            Top = 30;
        }
    }
    if((Left != FloatingPanel->Left) || (Top != FloatingPanel->Top))
    {
        FloatingPanel->Visible = false; // so doesn't flicker when reposition
        FloatingPanel->Left = Left;
        FloatingPanel->Top = Top;
        Utilities->CallLogPop(1917);
        return;
    }

    FloatingLabel->Caption = Caption;
    FloatingPanel->Visible = true;
    FloatingPanel->BringToFront();
    Utilities->CallLogPop(746);
}

// ---------------------------------------------------------------------------

void TInterface::FlashingGraphics(int Caller, TDateTime Now)
    // following section checks to see if GapFlashFlag set & flashes the Gap graphics if so
    // Gap flashing is cancelled on any mousedown event

    // deal with flashing GapFlash graphics (only in basic mode so no need to check for trains)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",FlashingGraphics");
    if(Track->GapFlashFlag && !Display->ZoomOutFlag)
    {
        if(WarningFlash)
        {
            Track->GapFlashGreen->PlotOverlay(4, Display); // only plotted if PlotOverlay reset
            Track->GapFlashRed->PlotOverlay(5, Display);
        }
        else
        {
            Track->GapFlashGreen->PlotOriginal(17, Display); // only plotted if PlotOverlay set
            Track->GapFlashRed->PlotOriginal(18, Display);
        }
    }

    if(Track->GapFlashFlag && Display->ZoomOutFlag)
    {
        if(WarningFlash)
        {
            Display->PlotSmallOutput(0, Track->TrackElementAt(454, Track->GapFlashGreenPosition).HLoc * 4,
                Track->TrackElementAt(455, Track->GapFlashGreenPosition).VLoc * 4, RailGraphics->smBrightGreen);
            Display->PlotSmallOutput(1, Track->TrackElementAt(456, Track->GapFlashRedPosition).HLoc * 4,
                Track->TrackElementAt(457, Track->GapFlashRedPosition).VLoc * 4, RailGraphics->smRed);
            Display->Update();
        }
        else
        {
            Display->PlotSmallOutput(2, Track->TrackElementAt(458, Track->GapFlashGreenPosition).HLoc * 4,
                Track->TrackElementAt(459, Track->GapFlashGreenPosition).VLoc * 4, RailGraphics->smSolidBgnd);
            Display->PlotSmallOutput(3, Track->TrackElementAt(460, Track->GapFlashRedPosition).HLoc * 4,
                Track->TrackElementAt(461, Track->GapFlashRedPosition).VLoc * 4, RailGraphics->smSolidBgnd);
            Display->Update();
        }
    }

// deal with other flashing graphics
    if(Track->RouteFlashFlag && !Display->ZoomOutFlag)
    {
        if((Now - RouteFlashStartTime) < TDateTime(RouteFlashDuration / 86400))
        {
            // cancel if train is moving & arrives on any part of flashing route
            if(MovingTrainPresentOnFlashingRoute(0))
            {
                Track->RouteFlashFlag = false;
                RevertToOriginalRouteSelector(7);
                ClearandRebuildRailway(18); // because using ConstructRoute->RouteFlash.PlotOriginal() can plot wrong point fillet as well as
                                            // original (if proposed route would change point). With this can dispense with ConstructRoute->RouteFlash.PlotOriginal()
                Utilities->CallLogPop(75);
                return;
            }

            InfoPanel->Visible = true;
            if(Level2OperMode == PreStart)
                InfoPanel->Caption = "PRE-START:  Route setting in progress";
            else
                InfoPanel->Caption = "OPERATING:  Route setting in progress";
            if(WarningFlash)
            {
                ConstructRoute->RouteFlash.PlotRouteOverlay(0);
            }
            else
            {
                ConstructRoute->RouteFlash.PlotRouteOriginal(0);
            }
        }
        else
        {
// ConstructRoute->RouteFlash.PlotOriginal(); don't need with clearand....
// stop clock while converting route as can take several seconds
            TrainController->StopTTClockFlag = true; // so TTClock stopped during MasterClockTimer function
            TrainController->RestartTime = TrainController->TTClockTime;
            if(PreferredRouteFlag)
                ConstructRoute->ConvertAndAddPreferredRouteSearchVector(1, ConstructRoute->ReqPosRouteID, AutoSigsFlag);
            else
                ConstructRoute->ConvertAndAddNonPreferredRouteSearchVector(1, ConstructRoute->ReqPosRouteID);
            ConstructRoute->ClearRoute(); // clear it immediately after use so as not to clutter the errorlog
            TrainController->BaseTime = TDateTime::CurrentDateTime();
            TrainController->StopTTClockFlag = false;
            Track->RouteFlashFlag = false;
            RevertToOriginalRouteSelector(8);
            ClearandRebuildRailway(19); // if drop this ensure replot trains after replot routes else route will overwrite a train
        }
    }

    if(Track->RouteFlashFlag && Display->ZoomOutFlag) // must have entered RouteFlash from normal screen so button states stored
    // dropped ZoomOutButton when route or point flashing, but leave this section in in case need to reinstate
    // no need to call Clearand... as that is called when revert to normal mode
    {
        if((Now - RouteFlashStartTime) >= TDateTime(RouteFlashDuration / 86400))
        {
            Track->RouteFlashFlag = false;
            if(PreferredRouteFlag)
            {
                ConstructRoute->ConvertAndAddPreferredRouteSearchVector(2, ConstructRoute->ReqPosRouteID, AutoSigsFlag);
            }
            else
            {
                ConstructRoute->ConvertAndAddNonPreferredRouteSearchVector(2, ConstructRoute->ReqPosRouteID);
            }
            ConstructRoute->ClearRoute(); // clear it immediately after use so as not to clutter the errorlog
        }
    }

    if(Track->PointFlashFlag && !Display->ZoomOutFlag)
    {
        if((Now - PointFlashStartTime) < TDateTime((PointsFlashDuration) / 86400))
        {
            // cancel if train is present on or enters a flashing point, either selected or diverging
            if((PointFlashVectorPosition > -1) && (Track->TrackElementAt(463, PointFlashVectorPosition).TrainIDOnElement > -1))
            {
                PointFlash->PlotOriginal(19, Display);
                Track->PointFlashFlag = false;
                DivergingPointVectorPosition = -1;
                Utilities->CallLogPop(76);
                return;
            }
            if((DivergingPointVectorPosition > -1) && (Track->TrackElementAt(464, DivergingPointVectorPosition).TrainIDOnElement > -1))
            {
                PointFlash->PlotOriginal(20, Display);
                Track->PointFlashFlag = false;
                DivergingPointVectorPosition = -1;
                Utilities->CallLogPop(77);
                return;
            }

            if(WarningFlash)
            {
                PointFlash->LoadOriginalScreenGraphic(3);
                PointFlash->PlotOverlay(6, Display);
            }
            else
            {
                PointFlash->PlotOriginal(21, Display);
            }
        }
        else
        {
            PointFlash->PlotOriginal(22, Display);
            int Att = Track->TrackElementAt(465, PointFlashVectorPosition).Attribute;
            Track->TrackElementAt(466, PointFlashVectorPosition).Attribute = 1 - Att;
            if(DivergingPointVectorPosition > -1)
            {
                Att = Track->TrackElementAt(467, DivergingPointVectorPosition).Attribute;
                Track->TrackElementAt(468, DivergingPointVectorPosition).Attribute = 1 - Att;
                Track->PlotPoints(1, Track->TrackElementAt(471, DivergingPointVectorPosition), Display, false);
            }
            Track->PlotPoints(2, Track->TrackElementAt(474, PointFlashVectorPosition), Display, false);
            Display->Update(); // resurrected when Update() dropped from PlotOutput etc
            Track->PointFlashFlag = false;
            DivergingPointVectorPosition = -1;
        }
    }

    if(Track->PointFlashFlag && Display->ZoomOutFlag)
    // dropped ZoomOutButton when point flashing but leave this section in in case need to reinstate
    {
        if((Now - PointFlashStartTime) < TDateTime((PointsFlashDuration) / 86400))
        {
            int Att = Track->TrackElementAt(475, PointFlashVectorPosition).Attribute;
            Track->TrackElementAt(476, PointFlashVectorPosition).Attribute = 1 - Att;
            if(DivergingPointVectorPosition > -1)
            {
                Att = Track->TrackElementAt(477, DivergingPointVectorPosition).Attribute;
                Track->TrackElementAt(478, DivergingPointVectorPosition).Attribute = 1 - Att;
            }
            Track->PointFlashFlag = false;
            DivergingPointVectorPosition = -1;
        }
    }
// deal with level crossings
    if(!Track->ChangingLCVector.empty() && (Level2OperMode != Paused))
    {
        int H;
        int V;
        for(unsigned int x = 0; x < Track->ChangingLCVector.size(); x++)
        {
            H = Track->ChangingLCVector.at(x).HLoc;
            V = Track->ChangingLCVector.at(x).VLoc;
            if((Now - Track->ChangingLCVector.at(x).StartTime) < TDateTime((Track->ChangingLCVector.at(x).ChangeDuration) / 86400))
            // still flashing
            {
                if(WarningFlash)
                {
                    if(Track->ChangingLCVector.at(x).BarrierState == TTrack::Raising) // closing to trains
                    {
                        Track->PlotRaisedLinkedLevelCrossingBarriers(1, Track->ChangingLCVector.at(x).BaseElementSpeedTag, H, V, Display);
                    }
                    else
                    {
                        Track->PlotLoweredLinkedLevelCrossingBarriers(0, Track->ChangingLCVector.at(x).BaseElementSpeedTag, H, V,
                            Track->ChangingLCVector.at(x).ConsecSignals, Display);
                    }
                }
                else
                {
                    Track->PlotLCBaseElementsOnly(2, Track->ChangingLCVector.at(x).BarrierState, Track->ChangingLCVector.at(x).BaseElementSpeedTag, H, V,
                        Track->ChangingLCVector.at(x).ConsecSignals, Display);
                }
            }
            else
            // flashing period finished
            {
                if(Track->ChangingLCVector.at(x).BarrierState == TTrack::Raising)
                {
                    Track->PlotRaisedLinkedLevelCrossingBarriers(2, Track->ChangingLCVector.at(x).BaseElementSpeedTag, H, V, Display);
                    Track->SetLinkedLevelCrossingBarrierAttributes(4, H, V, 0); // only set attr to 0 when fully raised
                    // attributes set to 2 when changing state, now reset to 0, no other actions needed
                }
                else
                // barriers lowering
                {
                    Track->PlotLoweredLinkedLevelCrossingBarriers(1, Track->ChangingLCVector.at(x).BaseElementSpeedTag, H, V,
                        Track->ChangingLCVector.at(x).ConsecSignals, Display);
                    Track->SetLinkedLevelCrossingBarrierAttributes(5, H, V, 1); // only set attr to 1 when fully lowered
                    bool FoundFlag;
                    int TVPos = Track->GetVectorPositionFromTrackMap(46, H, V, FoundFlag);
                    if(!FoundFlag)
                    {
                        throw Exception("Failed to find a route at LC position HLoc = " + (AnsiString)H + " VLoc = " + (AnsiString)V);
                    }
                    int RouteNumber;
                    AllRoutes->GetRouteTypeAndNumber(24, TVPos, 0, RouteNumber); // use 0 for LinkPos, could be 1 or 0 as only a single track element
                                                                                 // don't need returned value of RouteType
                    if(RouteNumber > -1) // if train crashed then there won't be a routenumber
                    {
                        AllRoutes->GetFixedRouteAt(196, RouteNumber).SetRouteSignals(8);
                    }
                }
            }
        }
        for(int x = Track->ChangingLCVector.size() - 1; x >= 0; x--)
        // now transfer lowering barrier object from the ChangingLCVector to the BarriersDownVector, reset the start timer (to time the barrier down period)
        // and erase the object from the ChangingLCVector
        {
            if(!Track->IsLCBarrierFlashingAtHV(0, Track->ChangingLCVector.at(x).HLoc, Track->ChangingLCVector.at(x).VLoc))
            {
                if(Track->ChangingLCVector.at(x).BarrierState == TTrack::Lowering)
                {
                    Track->ChangingLCVector.at(x).StartTime = TrainController->TTClockTime;
                    Track->ChangingLCVector.at(x).BarrierState = TTrack::Down;
                    Track->BarriersDownVector.push_back(Track->ChangingLCVector.at(x));
                }
                Track->ChangingLCVector.erase(Track->ChangingLCVector.begin() + x);
            }
        }
    }
    Utilities->CallLogPop(747);
}

// ---------------------------------------------------------------------------

void TInterface::SetSaveMenuAndButtons(int Caller)
    // set boundary, Home, NewHome, ZoomOut, CallingOn buttons & menu items as appropriate
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",SetSaveMenuAndButtons");

// set save railway buttons
    bool SaveRailwayButtonsFlag = true;

    SaveRailwayTBPButton->Visible = true;
    SaveRailwayPDPButton->Visible = true;
    SaveSessionButton->Visible = true;
    if(Level1Mode == OperMode)
    {
        if(Display->ZoomOutFlag || Track->RouteFlashFlag || Track->PointFlashFlag || Track->LCChangeFlag)
        {
            SaveRailwayButtonsFlag = false;
        }
        // set PresetAutoSigRoutesButton enabled or not
        // enable if PreStart & no routes set
        if((Level2OperMode == PreStart) && (AllRoutes->AllRoutesVector.empty()))
        {
            PresetAutoSigRoutesButton->Enabled = true;
        }
        else
        {
            PresetAutoSigRoutesButton->Enabled = false;
        }
    }
    else
    {
        if(!FileChangedFlag || Display->ZoomOutFlag || (Track->NoActiveOrInactiveTrack(11) && (TextHandler->TextVectorSize(14) == 0) && Track->UserGraphicVector.empty()))
        {
            SaveRailwayButtonsFlag = false;
        }
        else if(SavedFileName != "")
        {
            if((SavedFileName[SavedFileName.Length()] == 'y') || (SavedFileName[SavedFileName.Length()] == 'Y')) // 'rly' file
            {
                if(!(Track->IsReadyForOperation()))
                {
                    SaveRailwayButtonsFlag = false; // can't save under its old name as not now a .rly file
                }
            }
        }
    }
    if(SaveRailwayButtonsFlag && (Level1Mode == BaseMode))
    {
        SaveRailwayBaseModeButton->Visible = true;
    }
    else
    {
        SaveRailwayBaseModeButton->Visible = false;
    }
    SaveRailwayTBPButton->Enabled = SaveRailwayButtonsFlag;
    SaveRailwayPDPButton->Enabled = SaveRailwayButtonsFlag;
    SaveRailwayBaseModeButton->Enabled = SaveRailwayButtonsFlag;
    SaveSessionButton->Enabled = SaveRailwayButtonsFlag;

// set formatted timetable menu item
    if(TimetableTitle == "")
    {
        ExportTTMenuItem->Enabled = false;
    }
    else
    {
        ExportTTMenuItem->Enabled = true;
    }

// set info menu items
    if((Track->NoActiveOrInactiveTrack(8)) || Display->ZoomOutFlag || ((Level1Mode == TimetableMode) && (TimetableEditPanel->Visible)))
    {
        FloatingInfoMenu->Enabled = false;
        TrackInfoMenuItem->Enabled = false;
        TrainInfoMenuItem->Enabled = false;
    }
    else
    {
        FloatingInfoMenu->Enabled = true;
        TrackInfoMenuItem->Enabled = true;
        if(Level1Mode == OperMode)
        {
            TrainInfoMenuItem->Enabled = true;
        }
        else
        {
            TrainInfoMenuItem->Enabled = false;
        }
    }

// set all bar CallingOnButton operational to begin with - no, causes flickering of button graphics,
// work on internal flags & then set buttons according to final flag values, then graphic won't change unless
// there has been a legitimate change of state since the last access

    bool ZoomFlag = true, HomeFlag = true, NewHomeFlag = true, ScreenLeftFlag = true, ScreenRightFlag = true, ScreenUpFlag = true, ScreenDownFlag = true,
        TrackBuildPanelEnabledFlag = true, PrefDirPanelEnabledFlag = true, OperatingPanelEnabledFlag = true, TimetablePanelEnabledFlag = true;

    AnsiString TrackBuildPanelLabelCaptionStr = "Build/modify";
    AnsiString PrefDirPanelLabelCaptionStr = "Preferred direction selection";
    AnsiString OperatingPanelLabelCaptionStr = "Operation";
    AnsiString TimetablePanelLabelCaptionStr = "Timetable editor";

    if(!Display->ZoomOutFlag)
    { // prevent if half a screen or less visible (width = 60, height = 36) [Note HLocMin & Max 1 greater than extreme element]
        if((Track->GetHLocMin() - Display->DisplayOffsetH + 1 >= (Utilities->ScreenElementWidth / 2)))
            ScreenLeftFlag = false; // 60 - 30
        if((Track->GetHLocMax() - Display->DisplayOffsetH - 1 < (Utilities->ScreenElementWidth / 2)))
            ScreenRightFlag = false; // 60 - (60 - 30)
        if((Track->GetVLocMin() - Display->DisplayOffsetV + 1 >= (Utilities->ScreenElementHeight / 2)))
            ScreenUpFlag = false; // 36 - 18
        if((Track->GetVLocMax() - Display->DisplayOffsetV - 1 < (Utilities->ScreenElementHeight / 2)))
            ScreenDownFlag = false; // 36 - (36 - 18)
    }
    else
    { // prevent if less than a quarter of a screen visible (width = 240, height = 144)
        if((Track->GetHLocMin() - Display->DisplayZoomOutOffsetH + 1 >= (3 * Utilities->ScreenElementWidth)))
            ScreenLeftFlag = false; // 240 - 60
        if((Track->GetHLocMax() - Display->DisplayZoomOutOffsetH - 1 < Utilities->ScreenElementWidth))
            ScreenRightFlag = false; // 240 - (240 - 60)
        if((Track->GetVLocMin() - Display->DisplayZoomOutOffsetV + 1 >= (3 * Utilities->ScreenElementHeight)))
            ScreenUpFlag = false; // 144 - 36
        if((Track->GetVLocMax() - Display->DisplayZoomOutOffsetV - 1 < Utilities->ScreenElementHeight))
            ScreenDownFlag = false; // 144 - (144 - 36)
    }
    if(Track->NoActiveOrInactiveTrack(6) && TextHandler->TextVector.empty() && Track->UserGraphicVector.empty())
    {
        ZoomFlag = false;
        HomeFlag = false;
        NewHomeFlag = false;
        ScreenLeftFlag = false;
        ScreenRightFlag = false;
        ScreenUpFlag = false;
        ScreenDownFlag = false;
    }

    if(Display->ZoomOutFlag)
    {
// NewHomeFlag = false;
        TrackBuildPanelEnabledFlag = false;
        TrackBuildPanelLabelCaptionStr = "Disabled";
        PrefDirPanelEnabledFlag = false;
        PrefDirPanelLabelCaptionStr = "Disabled";
        OperatingPanelEnabledFlag = false;
        OperatingPanelLabelCaptionStr = "Disabled";
        TimetablePanelEnabledFlag = false;
        TimetablePanelLabelCaptionStr = "Disabled";
    }

    if(Level1Mode == OperMode)
    {
        if(Track->RouteFlashFlag || Track->PointFlashFlag || TTClockAdjPanel->Visible == true) //TTClockAdjPanel added for v2.4.2 to keep it disabled after Clock2Stopped dropped
        {
            OperatingPanelEnabledFlag = false;
            OperatingPanelLabelCaptionStr = "Disabled";
            ZoomFlag = false;
            HomeFlag = false;
            NewHomeFlag = false;
            ScreenLeftFlag = false;
            ScreenRightFlag = false;
            ScreenUpFlag = false;
            ScreenDownFlag = false;
            SaveOperatingImageMenuItem->Enabled = false;
        }
        else
        {
            SaveOperatingImageMenuItem->Enabled = true;
        }
    }

    if(LocationNameTextBox->Visible)
    {
        ZoomFlag = false;
        HomeFlag = false;
        NewHomeFlag = false;
        ScreenLeftFlag = false;
        ScreenRightFlag = false;
        ScreenUpFlag = false;
        ScreenDownFlag = false;
    }

    if(TextBox->Visible) // added at v1.3.0 to prevent screen moving when movement keys pressed during text entry
    {
        ZoomFlag = false;
        HomeFlag = false;
        NewHomeFlag = false;
        ScreenLeftFlag = false;
        ScreenRightFlag = false;
        ScreenUpFlag = false;
        ScreenDownFlag = false;
    }

    if((Level1Mode == TimetableMode) && (TimetableEditPanel->Visible))
    // added at v1.3.0 to prevent screen moving when movement keys pressed during timetable compilation (allowed if TT hidden)
    {
        ZoomFlag = false;
        HomeFlag = false;
        NewHomeFlag = false;
        ScreenLeftFlag = false;
        ScreenRightFlag = false;
        ScreenUpFlag = false;
        ScreenDownFlag = false;
    }

    if((Level2TrackMode == TrackSelecting) || (Level2TrackMode == CopyMoving) || (Level2TrackMode == CutMoving) || (Level2TrackMode == DistanceStart) ||
        (Level2TrackMode == DistanceContinuing) || (Level2PrefDirMode == PrefDirSelecting))
    {
        ZoomFlag = false;
    }

    if(ZoomFlag)
        ZoomButton->Enabled = true;
    else
        ZoomButton->Enabled = false;
    if(HomeFlag)
        HomeButton->Enabled = true;
    else
        HomeButton->Enabled = false;
    if(NewHomeFlag)
        NewHomeButton->Enabled = true;
    else
        NewHomeButton->Enabled = false;
    if(ScreenLeftFlag)
        ScreenLeftButton->Enabled = true;
    else
        ScreenLeftButton->Enabled = false;
    if(ScreenRightFlag)
        ScreenRightButton->Enabled = true;
    else
        ScreenRightButton->Enabled = false;
    if(ScreenUpFlag)
        ScreenUpButton->Enabled = true;
    else
        ScreenUpButton->Enabled = false;
    if(ScreenDownFlag)
        ScreenDownButton->Enabled = true;
    else
        ScreenDownButton->Enabled = false;
    if(OperatingPanelEnabledFlag)
        OperatingPanel->Enabled = true;
    else
        OperatingPanel->Enabled = false;
    if(TrackBuildPanelEnabledFlag)
        TrackBuildPanel->Enabled = true;
    else
        TrackBuildPanel->Enabled = false;
    if(PrefDirPanelEnabledFlag)
        PrefDirPanel->Enabled = true;
    else
        PrefDirPanel->Enabled = false;
    if(TimetablePanelEnabledFlag)
        TimetablePanel->Enabled = true;
    else
        TimetablePanel->Enabled = false;
    TrackBuildPanelLabel->Caption = TrackBuildPanelLabelCaptionStr;
    PrefDirPanelLabel->Caption = PrefDirPanelLabelCaptionStr;
    OperatingPanelLabel->Caption = OperatingPanelLabelCaptionStr;
    TimetablePanelLabel->Caption = TimetablePanelLabelCaptionStr;

// check if any CallingOnFlags set & set button accordingly
    if(Display->ZoomOutFlag)
    {
        CallingOnButton->Enabled = false;
        CallingOnButton->Down = false;
    }
    else
    {
        if(Level2OperMode == Operating)
        {
            bool CallOnValid = false;
            for(unsigned int x = 0; x < TrainController->TrainVector.size(); x++)
            {
                if(TrainController->TrainVectorAt(30, x).CallingOnFlag)
                {
                    CallingOnButton->Enabled = true;
                    CallOnValid = true;
                }
            }
            if(!CallOnValid)
            {
                CallingOnButton->Enabled = false;
                CallingOnButton->Down = false;
            }
        }
        else
        {
            CallingOnButton->Enabled = false;
            CallingOnButton->Down = false;
        }
    }
    Utilities->CallLogPop(970);
}

// ---------------------------------------------------------------------------

void TInterface::ErrorLog(int Caller, AnsiString Message)
{
// create an error file for diagnostic purposes called on detection of a runtime error

// Note: For faults in ClockTimer2, after the catch block in ClockTimer2 which calls this function, execution continues from where
// ClockTimer2 was called, so the Utilities->Clock2Stopped flag is cleared and the whole sequence repeats itself, including the fault, until
// the user presses the Exit button.  Note also that Utilities->CallLogPop, called when ClockTimer (not ClockTimer2) returns, pops the error
// message off the back of the Utilities->CallLog, not the ClockTimer call.  Hence entries keep stacking up, including the push_front entry
// but not the push_back error entry, and when finally printed there is a whole series of entries for the one fault, the number
// depending on the time taken to press Exit.
// Hence introduce an ErrorLogCalledFlag, set to true on first call, and preventing further calls thereafter.

    if(ErrorLogCalledFlag)
        return;

    ErrorLogCalledFlag = true;
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + "," + Message);
    Utilities->CallLog.push_front("Version: " + ProgramVersion + "; Time and date: " + Utilities->DateTimeStamp());
    SaveErrorFile();
    if((TempTTFileName != "") && FileExists(TempTTFileName))
    {
        DeleteFile(TempTTFileName);
    }
    Display->GetImage()->Visible = false;
    PerformancePanel->Visible = false;
    OperatorActionPanel->Visible = false; // new v2.2.0
    TrackBuildPanel->Visible = false;
    TrackElementPanel->Visible = false;
    LocationNameTextBox->Visible = false;
    TextBox->Visible = false;
    TrackLengthPanel->Visible = false;
    InfoPanel->Visible = false;
    PrefDirPanel->Visible = false;
    TimetablePanel->Visible = false;
    TimetableEditPanel->Visible = false;
    OperatingPanel->Visible = false;
    FloatingPanel->Visible = false;
    ModeMenu->Enabled = false;
    SigImagePanel->Visible = false; // new at v2.3.0
    FileMenu->Enabled = false;
    EditMenu->Enabled = false;
    FloatingInfoMenu->Enabled = false;
    HelpMenu->Enabled = false;
// SaveHeaderMenu1->Enabled = false;
    ScreenLeftButton->Visible = false;
    ScreenRightButton->Visible = false;
    ScreenUpButton->Visible = false;
    ScreenDownButton->Visible = false;
    HomeButton->Visible = false;
    NewHomeButton->Visible = false;
    ZoomButton->Visible = false;
    PrefDirKey->Visible = false;
    DistanceKey->Visible = false;
    OutputLog1->Caption = "";
    OutputLog2->Caption = "";
    OutputLog3->Caption = "";
    OutputLog4->Caption = "";
    OutputLog5->Caption = "";
    OutputLog6->Caption = "";
    OutputLog7->Caption = "";
    OutputLog8->Caption = "";
    OutputLog9->Caption = "";
    OutputLog10->Caption = "";
    ErrorMessage->Visible = true;
    ErrorButton->Visible = true;
    Screen->Cursor = TCursor(-2); // Arrow; - in case was an hourglass
// No need for Utilities->CallLogPop as the call log deque has already been written to file & the next action
// is to close the program when the exit button is pressed
}

// ---------------------------------------------------------------------------

bool TInterface::IsPerformancePanelObscuringFloatingLabel(int Caller)
    // not used from v2.2.0 as now allow floating panel & label to overlie performance panel
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",IsPerformancePanelObscuringFloatingLabel");
    if(FloatingPanel->Visible == false)
    {
        Utilities->CallLogPop(1205);
        return false;
    }
// pptop >= flbot, ppbot <= fltop, ppleft >= flright, ppright <= flleft
    if((PerformancePanel->Top >= (FloatingPanel->Top + FloatingPanel->Height)) || ((PerformancePanel->Top + PerformancePanel->Height) <= FloatingPanel->Top) ||
        (PerformancePanel->Left >= (FloatingPanel->Left + FloatingPanel->Width)) || ((PerformancePanel->Left + PerformancePanel->Width) <= FloatingPanel->Left))
    {
        Utilities->CallLogPop(1206);
        return false;
    }
    else
    {
        Utilities->CallLogPop(1207);
        return true;
    }
}
// ---------------------------------------------------------------------------

void TInterface::SetCaption(int Caller)
{
/*
   NamedRailway; RlyFile; NamedTimetable
     n           x          x  "New railway under development";
     y           n          x  RailwayTitle + ": under development";
     y           y          n  RailwayTitle + ": no timetable loaded";
     y           y          y  RailwayTitle + ", " + TimetableTitle;
*/

    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",SetCaption");
    if(RailwayTitle == "")
        Caption = "Railway: New railway under development";
    else if(!RlyFile)
        Caption = "Railway: " + RailwayTitle + " under development";
// else if(TimetableTitle == "") Caption = "Railway: " + RailwayTitle + "; Timetable: none loaded";
    else if(TimetableTitle == "")
        Caption = "Railway: " + RailwayTitle; // changed at v2.1.0, no need to mention TT if none loaded
    else
        Caption = "Railway: " + RailwayTitle + "; Timetable: " + TimetableTitle;
    Utilities->CallLogPop(1208);
}

// ---------------------------------------------------------------------------

void TInterface::ResetAll(int Caller)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",ResetAll");
    LastNonCtrlOrShiftKeyDown = -1; // added at v2.4.2 to no key down
    Track->GapFlashGreenPosition = -1;
    Track->GapFlashRedPosition = -1;
    Track->GapFlashFlag = false;
    Track->RouteFlashFlag = false;
    Track->PointFlashFlag = false;
    DivergingPointVectorPosition = -1;
    AutoSigsFlag = false;
    PreventGapOffsetResetting = false;

    Utilities->Clock2Stopped = false;
    TTClockSpeed = 1;
    TTClockSpeedLabel->Caption = "x1";
    Track->SetTrackFinished(false);
    Track->ActiveTrackElementNameMapCompiledFlag = false;
    CurrentSpeedButton = 0; // not assigned yet
    Display->ResetZoomInOffsets();
    StartX = 0;
    StartY = 0;
    mbLeftDown = false;
    TextOrUserGraphicGridVal = 1;
    TextOrUserGraphicGridButton->Glyph->LoadFromResourceName(0, "PixelPrecision1");
    Track->SignalAspectBuildMode = TTrack::FourAspectBuild;
    SigAspectButton->Glyph->LoadFromResourceName(0, "FourAspect");
    LoadNormalSignalGlyphs(4);
    WarningFlashCount = 0;

    Level1Mode = BaseMode;
    SetLevel1Mode(26);
    RouteMode = None;
    PreferredRoute = true;
    ConsecSignalsRoute = true;
    DevelopmentPanel->Visible = false;

    MainScreen->Canvas->CopyMode = cmSrcCopy;
    FloatingPanel->Visible = false;
    OverallDistance = 0;
    OverallSpeedLimit = -1;
    AllRoutes->RouteTruncateFlag = false;
    CallingOnButton->Down = false;
    Display->ZoomOutFlag = false;
    ScreenGridFlag = false;
    InfoCaptionStore = "";
    ErrorLogCalledFlag = false;
    ErrorMessage->Visible = false;
    TempCursorSet = false;
    TempCursor = TCursor(-2); // Arrow
    WholeRailwayMoving = false; // new at v2.1.0

    TrainController->TTClockTime = TDateTime(0); // default setting
    TTClockAdjPanel->Visible = false;
    TrainController->StopTTClockFlag = false;
    SelectedTrainID = -1;
    SetTrackBuildImages(11);
// TrackInfoOnOffMenuItem->Caption = "Show";  dropped these here at v1.2.0 so don't reset when load a session file
// TrainStatusInfoOnOffMenuItem->Caption = "Show Status";
// TrainTTInfoOnOffMenuItem->Caption = "Show Timetable";
    Track->CalcHLocMinEtc(8);
    FileChangedFlag = false;
    RlyFile = false;
    SaveSessionFlag = false;
    LoadSessionFlag = false;
    SelectionValid = false;
    TimetableChangedFlag = false;
    SavedFileName = "";
    RailwayTitle = "";
    TimetableTitle = "";
    SetCaption(1);
    CreateEditTTFileName = ""; // set to null to allow a check during error file saving, if not null save the tt being edited to the file
                                // added for Beta v0.2b
    CreateEditTTTitle = ""; // as above
    AllRoutes->NextRouteID = 0;
    TTrain::NextTrainID = 0; // reset to 0 whenever enter operating mode
    TFont *TempFont = new TFont; // if try to alter MainScreen->Canvas->Font directly it won't change the style for some reason

    TempFont->Style.Clear();
    TempFont->Name = "MS Sans Serif"; // reset font, else stays set to last displayed text font
    TempFont->Size = 10;
    TempFont->Color = clB0G0R0;
    TempFont->Charset = (TFontCharset)(0);
    MainScreen->Canvas->Font->Assign(TempFont);
    PerformancePanel->Top = MainScreen->Top + MainScreen->Height - PerformancePanel->Height;
    PerformancePanel->Left = MainScreen->Left;
    OperatorActionPanel->Top = MainScreen->Top + MainScreen->Height - OperatorActionPanel->Height; // new v2.2.0
    OperatorActionPanel->Left = MainScreen->Left + MainScreen->Width - OperatorActionPanel->Width; ; // new v2.2.0
    // ScreenRightButton->Left = MainScreen->Width + MainScreen->Left;  //Button values changed at v2.1.0 to allow for screen resizing
    // ScreenLeftButton->Left = ScreenRightButton->Left;
    // ScreenUpButton->Left = ScreenRightButton->Left;
    // ScreenDownButton->Left = ScreenRightButton->Left;
    // HomeButton->Left = ScreenRightButton->Left;
    // NewHomeButton->Left = ScreenRightButton->Left;
    // ZoomButton->Left = ScreenRightButton->Left;
    DevelopmentPanel->Top = MainScreen->Top + MainScreen->Height - DevelopmentPanel->Height;
    DevelopmentPanel->Left = MainScreen->Left + MainScreen->Width - DevelopmentPanel->Width; ; // new v2.2.0

    delete TempFont;
    CtrlKey = false;
    ShiftKey = false;
    Utilities->CallLogPop(1209);
}

// ---------------------------------------------------------------------------

void TInterface::SetTrackBuildImages(int Caller)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",SetTrackBuildImages,");
    if((Level1Mode == OperMode) || RlyFile)
    {
        TrackLinkedImage->Visible = false;
        TrackNotLinkedImage->Visible = false;
        GapsSetImage->Visible = false;
        GapsNotSetImage->Visible = false;
        LocationNamesSetImage->Visible = false;
        LocationNamesNotSetImage->Visible = false;
        Utilities->CallLogPop(1114);
        return;
    }
    else
    {
        if(!Track->NoActiveTrack(9))
        {
            if(Track->IsTrackFinished())
            {
                TrackLinkedImage->Visible = true;
                TrackNotLinkedImage->Visible = false;
            }
            else
            {
                TrackNotLinkedImage->Visible = true;
                TrackLinkedImage->Visible = false;
            }
        }
        else
        {
            TrackLinkedImage->Visible = false;
            TrackNotLinkedImage->Visible = false;
        }

        if(!Track->NoGaps(1))
        {
            if(Track->GapsUnset(6))
            {
                GapsNotSetImage->Visible = true;
                GapsSetImage->Visible = false;
            }
            else
            {
                GapsNotSetImage->Visible = false;
                GapsSetImage->Visible = true;
            }
        }
        else
        {
            GapsNotSetImage->Visible = false;
            GapsSetImage->Visible = false;
        }

        if(!Track->NoNamedLocationElements(1))
        {
            if(Track->LocationsNotNamed(0))
            {
                LocationNamesSetImage->Visible = false;
                LocationNamesNotSetImage->Visible = true;
            }
            else
            {
                LocationNamesSetImage->Visible = true;
                LocationNamesNotSetImage->Visible = false;
            }
        }
        else
        {
            LocationNamesSetImage->Visible = false;
            LocationNamesNotSetImage->Visible = false;
        }
    }
    Utilities->CallLogPop(1113);
}

// ---------------------------------------------------------------------------

void TInterface::ResetChangedFileDataAndCaption(int Caller, bool NonPrefDirChangesMade)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",ResetChangedFileDataAndCaption");
    FileChangedFlag = true;
    if(NonPrefDirChangesMade)
    {
        if(RlyFile) // i.e. was a Railway file but major changes made so class as a new railway
        {
            RailwayTitle = "";
            TimetableTitle = "";
            SavedFileName = "";
            RlyFile = false;
        }
        TimetableTitle = ""; // should have been reset already during user mode change but include here also
        SetTrackBuildImages(15);
    }
    SetCaption(2);
    Utilities->CallLogPop(1210);
}

// ---------------------------------------------------------------------------

void TInterface::SaveSession(int Caller)
{ // ExcessLCDownMins saved as string after ***Interface*** see below
    try
    {
        TrainController->LogEvent("SaveSession");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",SaveSession");
        AnsiString CurrentDateTimeStr = "", TimetableTimeStr = "", SessionFileStr = "";
        Screen->Cursor = TCursor(-11); // Hourglass;
        CurrentDateTimeStr = TDateTime::CurrentDateTime().FormatString("dd-mm-yyyy hh.nn.ss");
        // avoid characters in filename:=   / \ : * ? " < > |
        TimetableTimeStr = Utilities->Format96HHMMSS(TrainController->TTClockTime);
        TimetableTimeStr = TimetableTimeStr.SubString(1, 2) + '.' + TimetableTimeStr.SubString(4, 2) + '.' + TimetableTimeStr.SubString(7, 2);
        SessionFileStr = CurDir + "\\" + SESSION_DIR_NAME + "\\Session " + CurrentDateTimeStr + "; Timetable time " + TimetableTimeStr + "; " + RailwayTitle +
            "; " + TimetableTitle + ".ssn";
        std::ofstream SessionFile(SessionFileStr.c_str());
        if(!(SessionFile.fail()))
        {
            Utilities->SaveFileString(SessionFile, ProgramVersion + ": ***Interface***" + FloatToStr(TrainController->ExcessLCDownMins));
// added ExcessLC... at v2.2.0 as omitted earlier
            SaveInterface(0, SessionFile);
            // save track elements
            Utilities->SaveFileString(SessionFile, "***Track***");
            if(Track->UserGraphicVector.empty())
            {
                Track->SaveTrack(4, SessionFile, false); // false for no graphics (**Active elements** saved as marker)
            }
            else
            {
                Track->SaveTrack(11, SessionFile, true); // true for graphics to be saved (**Active elements**1 saved as marker)
            }
            // save text elements
            Utilities->SaveFileString(SessionFile, "***Text***");
            TextHandler->SaveText(2, SessionFile);
            // save PrefDir elements
            Utilities->SaveFileString(SessionFile, "***PrefDirs***");
            EveryPrefDir->SavePrefDirVector(2, SessionFile);
            if(!Track->UserGraphicVector.empty())
            {
                // save user graphics
                Track->SaveUserGraphics(2, SessionFile);
            }
            // save routes
            Utilities->SaveFileString(SessionFile, "***Routes***");
            AllRoutes->SaveRoutes(0, SessionFile);
            // save LockedRoutes
            Utilities->SaveFileString(SessionFile, "***Locked routes***");
            TrainController->SaveSessionLockedRoutes(0, SessionFile);
            // save ContinuationAutoSigEntries
            Utilities->SaveFileString(SessionFile, "***ContinuationAutoSigEntries***");
            TrainController->SaveSessionContinuationAutoSigEntries(0, SessionFile);
            // save BarriersDownVector
            Utilities->SaveFileString(SessionFile, "***BarriersDownVector***");
            Track->SaveSessionBarriersDownVector(0, SessionFile);
            // save timetable
            Utilities->SaveFileString(SessionFile, "***Timetable***");
            if(!(SaveTimetableToSessionFile(0, SessionFile, SessionFileStr)))
            {
                SessionFile.close();
                DeleteFile(SessionFileStr);
                Screen->Cursor = TCursor(-2); // Arrow;
                TrainController->StopTTClockMessage(3, "Error saving file, unable to save session");
                Utilities->CallLogPop(1150);
                return;
            }
            // save TimetableClock
            Utilities->SaveFileString(SessionFile, "***TimetableClock***");
            Utilities->SaveFileDouble(SessionFile, double(TrainController->TTClockTime));

            // save trains
            Utilities->SaveFileString(SessionFile, "***Trains***");
            TrainController->SaveSessionTrains(0, SessionFile);
            // save performance file
            Utilities->SaveFileString(SessionFile, "***Performance file***");
            SavePerformanceFile(0, SessionFile);
            Utilities->SaveFileString(SessionFile, "***End of performance file***");
// addition at v2.4.0 to save TrainController->AvHoursIntValue + any future additions
            Utilities->SaveFileString(SessionFile, "***Additions after v2.3.1***");
            Utilities->SaveFileInt(SessionFile, TrainController->AvHoursIntValue);
            Utilities->SaveFileInt(SessionFile, TrainController->NumFailures);
            Utilities->SaveFileString(SessionFile, "***Failed Trains***");
            for(unsigned int x = 0; x < TrainController->TrainVector.size(); x++)
            {
                if(TrainController->TrainVectorAt(66, x).TrainFailed)
                {
                    Utilities->SaveFileInt(SessionFile, TrainController->TrainVectorAt(67, x).TrainID);
                    Utilities->SaveFileDouble(SessionFile, TrainController->TrainVectorAt(68, x).OriginalPowerAtRail);
                }
            }
            Utilities->SaveFileInt(SessionFile, -1); // marker for end of failed trains
            Utilities->SaveFileString(SessionFile, "End of file at v2.4.0");
// end of v2.4.0 addition
            SessionFile.close();
            TrainController->StopTTClockMessage(4, "Session saved: Session " + CurrentDateTimeStr + "; Timetable time " + TimetableTimeStr + "; " +
                RailwayTitle + "; " + TimetableTitle + ".ssn");
            LastNonCtrlOrShiftKeyDown = -1; //to restore the ability to reselect Ctrl S after a save (FormKeyUp doesn't work because the Interface form doesn't have focus)
        }
        else
        {
            TrainController->StopTTClockMessage(5, "Session file failed to open, session not saved.  Ensure that there is a folder named " + SESSION_DIR_NAME +
                " in the folder where the 'Railway.exe' program file resides");
        }
        Screen->Cursor = TCursor(-2); // Arrow
        Utilities->CallLogPop(1141);
    }
    catch(const Exception &e)
    {
        ErrorLog(40, e.Message);
    }
}

// ---------------------------------------------------------------------------

void TInterface::LoadSession(int Caller)
    // always loads in 'Paused' or 'PreStart' mode
{
// remember to load the timetable clock
// no routes in build
// prob need to set 'OperMode' then 'Paused', ensure have all info needed for these
// set buttons enabled to correspond to flags on reloading. If no PrefDirs then disable all route buttons
// set RlyFile true
    try
    {
        TrainController->LogEvent("LoadSession");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",LoadSession");
        if(!ClearEverything(4))
        {
            Utilities->CallLogPop(1145);
            return;
        }
        LoadSessionDialog->Filter = "Session file (*.ssn)|*.ssn";
        if(LoadSessionDialog->Execute())
        {
            TrainController->LogEvent("LoadSession " + LoadSessionDialog->FileName);
            Screen->Cursor = TCursor(-11); // Hourglass;
            if(SessionFileIntegrityCheck(0, AnsiString(LoadSessionDialog->FileName).c_str()))
    // if(true)
            {
                std::ifstream SessionFile(AnsiString(LoadSessionDialog->FileName).c_str());
                if(!(SessionFile.fail()))
                {
                    TrainController->AvHoursIntValue = 0; // initial value set at v2.4.0 in case not changed later
                    TrainController->MTBFHours = 0; // initial value set at v2.4.0 in case not changed later
                    AnsiString TempString = Utilities->LoadFileString(SessionFile);
// "version + : ***Interface***" + at v2.2.0 ExcessLCDownMins (omitted earlier)

                    int LastCharBeforeFloat = TempString.LastDelimiter('*'); // added at v2.2.0
                    if((LastCharBeforeFloat == 0) || (LastCharBeforeFloat == TempString.Length()))
    // can't find it or no value for Excess LCDownMins, either way count as zero
                    {
                        TrainController->ExcessLCDownMins = 0;
                    }
                    else
                    {
                        AnsiString FloatStr = TempString.SubString(LastCharBeforeFloat + 1, TempString.Length() - LastCharBeforeFloat);
                        if(!Utilities->CheckStringDouble(FloatStr)) // returns false for empty string or invalid double
                        {
                            TrainController->ExcessLCDownMins = 0;
                        }
                        else
                        {
                            TrainController->ExcessLCDownMins = FloatStr.ToDouble();
                        }
                    } // end of v2.2.0 * v2.4.0 additions

                    LoadInterface(0, SessionFile);
                    int TempDisplayOffsetH = Display->DisplayOffsetH; // stored as they are zeroed when track loaded
                    int TempDisplayOffsetV = Display->DisplayOffsetV;
                    int TempDisplayOffsetHHome = Display->DisplayOffsetHHome;
                    int TempDisplayOffsetVHome = Display->DisplayOffsetVHome;
                    bool GraphicsFollow = false;
                    // load track elements
                    TempString = Utilities->LoadFileString(SessionFile); // ***Track***"
                    Track->LoadTrack(4, SessionFile, GraphicsFollow);
                    // load text elements
                    TempString = Utilities->LoadFileString(SessionFile); // ***Text***"
                    TextHandler->LoadText(1, SessionFile);
                    // load PrefDir elements
                    TempString = Utilities->LoadFileString(SessionFile); // "***PrefDirs***"
                    EveryPrefDir->LoadPrefDir(1, SessionFile);
                    if(GraphicsFollow)
                    {
                        Track->LoadGraphics(1, SessionFile, CurDir + "\\" + USERGRAPHICS_DIR_NAME); // include path to Graphics folder);
                    }
                    if(!EveryPrefDir->CheckPrefDirAgainstTrackVectorNoMessage(0))
                    {
                        SessionFile.close();
                        Screen->Cursor = TCursor(-2); // Arrow;
                        ShowMessage("Corruption in preferred direction section of the session file, session can't be loaded");
                        Utilities->CallLogPop(1438);
                        return;
                    }
                    // load routes
                    TempString = Utilities->LoadFileString(SessionFile); // "***Routes***"
                    if(!AllRoutes->LoadRoutes(0, SessionFile))
                    {
                        SessionFile.close();
                        Screen->Cursor = TCursor(-2); // Arrow;
                        ShowMessage("Corruption in route section of the session file, session can't be loaded");
                        Utilities->CallLogPop(1439);
                        return;
                    }
                    // load LockedRoutes
                    TempString = Utilities->LoadFileString(SessionFile); // "***Locked routes***"
                    TrainController->LoadSessionLockedRoutes(0, SessionFile);
                    // load ContinuationAutoSigEntries
                    TempString = Utilities->LoadFileString(SessionFile); // "***ContinuationAutoSigEntries***"
                    TrainController->LoadSessionContinuationAutoSigEntries(0, SessionFile);
                    // load BarriersDownVector if present, but ensure backwards compatibility with earlier files
                    TempString = Utilities->LoadFileString(SessionFile); // "***BarriersDownVector***" or "***Timetable***"
                    if(TempString == "***BarriersDownVector***")
                    {
                        Track->LoadBarriersDownVector(0, SessionFile);
                        TempString = Utilities->LoadFileString(SessionFile); // "***Timetable***"
                    }
                    // load timetable (marker "***Timetable***" already loaded)
                    if(!(LoadTimetableFromSessionFile(0, SessionFile)))
                    {
                        SessionFile.close();
                        Screen->Cursor = TCursor(-2); // Arrow;
                        ShowMessage("Unable to load timetable section of the session file, session can't be loaded");
                        Utilities->CallLogPop(1151);
                        return;
                    }
                    // TimetableTitle should be loaded at this stage - check
                    if(TimetableTitle == "")
                    {
                        SessionFile.close();
                        Screen->Cursor = TCursor(-2); // Arrow;
                        throw Exception("TimetableTitle null in LoadSession");
                    }
                    // load timetable clock
                    TempString = Utilities->LoadFileString(SessionFile); // ***TimetableClock***
                    TrainController->RestartTime = TDateTime(Utilities->LoadFileDouble(SessionFile)); // ClockTime set in RestartSessionOperMode;
                    // load trains
                    TempString = Utilities->LoadFileString(SessionFile); // ***Trains***
                    TrainController->LoadSessionTrains(0, SessionFile);
                    // load performance file + populate the performance log
                    TempString = Utilities->LoadFileString(SessionFile); // ***Performance file***
                    // first reset the performance file name and open it before reloading it
                    PerformanceFileName = TDateTime::CurrentDateTime().FormatString("dd-mm-yyyy hh.nn.ss");
                    // avoid characters in filename:=   / \ : * ? " < > |
                    PerformanceFileName = CurDir + "\\" + PERFLOG_DIR_NAME + "\\Log " + PerformanceFileName + "; " + RailwayTitle + "; " +
                        TimetableTitle + ".txt";
                    Utilities->PerformanceFile.open(PerformanceFileName.c_str(), std::ios_base::out);
                    if(Utilities->PerformanceFile.fail())
                    {
                        ShowMessage("Performance logfile failed to open, logs won't be saved. Ensure that there is a folder named " + PERFLOG_DIR_NAME +
                            " in the folder where the 'Railway.exe' program file resides");
                    }
                    // now reload the performance file
                    LoadPerformanceFile(0, SessionFile);
                    // addition at v2.4.0
                    char TempChar;
                    SessionFile.get(TempChar);
                    while(!SessionFile.eof() && ((TempChar == '\n') || (TempChar == '\0'))) // when emerge from here either have eof or '*'
                    {
                        SessionFile.get(TempChar);
                    }
                    if(SessionFile.eof()) // end of file
                    {
                        TrainController->AvHoursIntValue = 0;
                        TrainController->MTBFHours = 0;
                        SessionFile.close(); // no TrainController->AvHoursIntValue & no failed trains
                    }
                    else
                    {
                        AnsiString DummyStr = Utilities->LoadFileString(SessionFile); // "**Additions after v2.3.1***"  discarded (first '*' loaded earlier)
                        TrainController->AvHoursIntValue = Utilities->LoadFileInt(SessionFile); // TrainController->AvHoursIntValue added at v2.4.0
                        TrainController->NumFailures = Utilities->LoadFileInt(SessionFile); // number of train failures
                        TrainController->MTBFHours = TrainController->AvHoursIntValue; // TTClockSpeed set to 1 in RestartSessionMode so no need to include here
                        // now load any failed trains along with their OriginalPowerAtRail values
                        DummyStr = Utilities->LoadFileString(SessionFile); // discard "***Failed Trains***"
                        int ID = Utilities->LoadFileInt(SessionFile); // train ID or -1 for no more failed trains
                        double PowerDouble;
                        while(ID != -1)
                        {
                            PowerDouble = Utilities->LoadFileDouble(SessionFile); // ok TrainVector loaded at this stage (loaded in LoadSessionTrains)
                            TrainController->TrainVectorAtIdent(47, ID).TrainFailed = true;
                            TrainController->TrainVectorAtIdent(48, ID).OriginalPowerAtRail = PowerDouble;
                            ID = Utilities->LoadFileInt(SessionFile);
                        }
                        SessionFile.close();
                    }
                    // deal with other settings
                    Display->DisplayOffsetH = TempDisplayOffsetH; // reset to saved values
                    Display->DisplayOffsetV = TempDisplayOffsetV;
                    Display->DisplayOffsetHHome = TempDisplayOffsetHHome;
                    Display->DisplayOffsetVHome = TempDisplayOffsetVHome;
                    // now set attributes to 1 for all LCs with barriers down
                    for(unsigned int x = 0; x < Track->BarriersDownVector.size(); x++)
                    {
                        Track->SetLinkedLevelCrossingBarrierAttributes(2, Track->BarriersDownVector.at(x).HLoc, Track->BarriersDownVector.at(x).VLoc, 1);
                    }
                    Track->ChangingLCVector.clear();
                    Track->CalcHLocMinEtc(10);
                    Level1Mode = RestartSessionOperMode;
                    SetLevel1Mode(27);
                    if(Level2OperMode == PreStart)
    // this section added at v1.3.2 as otherwise only in MainScreenMouseDown2, and if load a session without clicking mouse on screen
                    { // then delay unspecified though seems to be 0
                        PointsFlashDuration = 0.0;
                        Track->LevelCrossingBarrierUpFlashDuration = 0.0;
                        Track->LevelCrossingBarrierDownFlashDuration = 0.0;
                    }
                    else
                    {
                        PointsFlashDuration = AllRoutes->PointsDelay;
                        Track->LevelCrossingBarrierUpFlashDuration = AllRoutes->LevelCrossingBarrierUpDelay;
                        Track->LevelCrossingBarrierDownFlashDuration = AllRoutes->LevelCrossingBarrierDownDelay;
                    }
                    RlyFile = true;
                    SetCaption(3);
                    ClearandRebuildRailway(42);
                }
            }
            else
            {
                ShowMessage("Session file integrity check failed, unable to load session.");
            }
            Screen->Cursor = TCursor(-2); // Arrow;
        }
        Utilities->CallLogPop(1146);
    }
    catch(const Exception &e)
    {
        if((e.Message.Pos("esource") > 0) || (e.Message.Pos("arameter") > 0)) // 'Resource or Parameter, avoid capitals as may be OS dependent
        {
            Screen->Cursor = TCursor(-2); // Arrow;
            OutputLog1->Caption = "";
            OutputLog2->Caption = "";
            OutputLog3->Caption = "";
            OutputLog4->Caption = "";
            OutputLog5->Caption = "";
            OutputLog6->Caption = "";
            OutputLog7->Caption = "";
            OutputLog8->Caption = "";
            OutputLog9->Caption = "";
            OutputLog10->Caption = "";
            UnicodeString MessageStr =
                "Insufficient memory available to load the file, and partial load likely to be corrupt. Application will terminate. Try loading the session as the first action after reloading the program.";
// UnicodeString MessageStr = "Last train loaded = " + UnicodeString(TrainController->LastTrainLoaded); //for debugging session train loading for many trains
            Application->MessageBox(MessageStr.c_str(), L"Out of memory", MB_OK | MB_ICONERROR);
            Application->Terminate();
        }
        else
        {
            ErrorLog(41, e.Message);
        }
    }
}

// ---------------------------------------------------------------------------

void TInterface::SaveInterface(int Caller, std::ofstream &SessionFile)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",SaveInterface");
    if(Level2OperMode == PreStart)
        Utilities->SaveFileString(SessionFile, AnsiString("PreStart"));
    else
        Utilities->SaveFileString(SessionFile, AnsiString("NotPreStart")); // covers both Paused & Operating
    Utilities->SaveFileString(SessionFile, RailwayTitle);
    Utilities->SaveFileString(SessionFile, TimetableTitle);
    Utilities->SaveFileBool(SessionFile, PreferredRoute);
    Utilities->SaveFileBool(SessionFile, ConsecSignalsRoute);
    Utilities->SaveFileBool(SessionFile, AutoSigsFlag);
    Utilities->SaveFileInt(SessionFile, Display->DisplayOffsetH);
    Utilities->SaveFileInt(SessionFile, Display->DisplayOffsetV);
    Utilities->SaveFileInt(SessionFile, Display->DisplayOffsetHHome);
    Utilities->SaveFileInt(SessionFile, Display->DisplayOffsetVHome);
    Utilities->SaveFileInt(SessionFile, Display->DisplayZoomOutOffsetH);
    Utilities->SaveFileInt(SessionFile, Display->DisplayZoomOutOffsetV);
    Utilities->SaveFileString(SessionFile, OutputLog1->Caption);
    Utilities->SaveFileString(SessionFile, OutputLog2->Caption);
    Utilities->SaveFileString(SessionFile, OutputLog3->Caption);
    Utilities->SaveFileString(SessionFile, OutputLog4->Caption);
    Utilities->SaveFileString(SessionFile, OutputLog5->Caption);
    Utilities->SaveFileString(SessionFile, OutputLog6->Caption);
    Utilities->SaveFileString(SessionFile, OutputLog7->Caption);
    Utilities->SaveFileString(SessionFile, OutputLog8->Caption);
    Utilities->SaveFileString(SessionFile, OutputLog9->Caption);
    Utilities->SaveFileString(SessionFile, OutputLog10->Caption);

    Utilities->SaveFileInt(SessionFile, TrainController->OnTimeArrivals);
    Utilities->SaveFileInt(SessionFile, TrainController->LateArrivals);
    Utilities->SaveFileInt(SessionFile, TrainController->EarlyArrivals);
    Utilities->SaveFileInt(SessionFile, TrainController->OnTimePasses);
    Utilities->SaveFileInt(SessionFile, TrainController->LatePasses);
    Utilities->SaveFileInt(SessionFile, TrainController->EarlyPasses);
    Utilities->SaveFileInt(SessionFile, TrainController->OnTimeDeps);
    Utilities->SaveFileInt(SessionFile, TrainController->LateDeps);
    Utilities->SaveFileInt(SessionFile, TrainController->MissedStops);
    Utilities->SaveFileInt(SessionFile, TrainController->OtherMissedEvents);
    Utilities->SaveFileInt(SessionFile, TrainController->UnexpectedExits);
    Utilities->SaveFileInt(SessionFile, TrainController->IncorrectExits);
    Utilities->SaveFileInt(SessionFile, TrainController->SPADEvents);
    Utilities->SaveFileInt(SessionFile, TrainController->SPADRisks);
    Utilities->SaveFileInt(SessionFile, TrainController->CrashedTrains);
    Utilities->SaveFileInt(SessionFile, TrainController->Derailments);
    Utilities->SaveFileDouble(SessionFile, TrainController->TotLateArrMins);
    Utilities->SaveFileDouble(SessionFile, TrainController->TotEarlyArrMins);
    Utilities->SaveFileDouble(SessionFile, TrainController->TotLatePassMins);
    Utilities->SaveFileDouble(SessionFile, TrainController->TotEarlyPassMins);
    Utilities->SaveFileDouble(SessionFile, TrainController->TotLateDepMins);
    // ExcessLCDownMins saved after ***Interface*** at v2.2.0 (omitted in error earlier)
    Utilities->CallLogPop(1211);
}

// ---------------------------------------------------------------------------
void TInterface::LoadInterface(int Caller, std::ifstream &SessionFile)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",LoadInterface");
    AnsiString OpMode = Utilities->LoadFileString(SessionFile);

    if(OpMode == "PreStart")
        Level2OperMode = PreStart;
    else
        Level2OperMode = Paused;
    RailwayTitle = Utilities->LoadFileString(SessionFile);
    SavedFileName = CurDir + "\\" + RAILWAY_DIR_NAME + "\\" + RailwayTitle + ".rly";

    TimetableTitle = Utilities->LoadFileString(SessionFile);
    PreferredRoute = Utilities->LoadFileBool(SessionFile);
    ConsecSignalsRoute = Utilities->LoadFileBool(SessionFile);
    AutoSigsFlag = Utilities->LoadFileBool(SessionFile);
    Display->DisplayOffsetH = Utilities->LoadFileInt(SessionFile);
    Display->DisplayOffsetV = Utilities->LoadFileInt(SessionFile);
    Display->DisplayOffsetHHome = Utilities->LoadFileInt(SessionFile);
    Display->DisplayOffsetVHome = Utilities->LoadFileInt(SessionFile);
    Display->DisplayZoomOutOffsetH = Utilities->LoadFileInt(SessionFile);
    Display->DisplayZoomOutOffsetV = Utilities->LoadFileInt(SessionFile);
    OutputLog1->Caption = Utilities->LoadFileString(SessionFile);
    OutputLog2->Caption = Utilities->LoadFileString(SessionFile);
    OutputLog3->Caption = Utilities->LoadFileString(SessionFile);
    OutputLog4->Caption = Utilities->LoadFileString(SessionFile);
    OutputLog5->Caption = Utilities->LoadFileString(SessionFile);
    OutputLog6->Caption = Utilities->LoadFileString(SessionFile);
    OutputLog7->Caption = Utilities->LoadFileString(SessionFile);
    OutputLog8->Caption = Utilities->LoadFileString(SessionFile);
    OutputLog9->Caption = Utilities->LoadFileString(SessionFile);
    OutputLog10->Caption = Utilities->LoadFileString(SessionFile);

    TrainController->OnTimeArrivals = Utilities->LoadFileInt(SessionFile);
    TrainController->LateArrivals = Utilities->LoadFileInt(SessionFile);
    TrainController->EarlyArrivals = Utilities->LoadFileInt(SessionFile);
    TrainController->OnTimePasses = Utilities->LoadFileInt(SessionFile);
    TrainController->LatePasses = Utilities->LoadFileInt(SessionFile);
    TrainController->EarlyPasses = Utilities->LoadFileInt(SessionFile);
    TrainController->OnTimeDeps = Utilities->LoadFileInt(SessionFile);
    TrainController->LateDeps = Utilities->LoadFileInt(SessionFile);
    TrainController->MissedStops = Utilities->LoadFileInt(SessionFile);
    TrainController->OtherMissedEvents = Utilities->LoadFileInt(SessionFile);
    TrainController->UnexpectedExits = Utilities->LoadFileInt(SessionFile);
    TrainController->IncorrectExits = Utilities->LoadFileInt(SessionFile);
    TrainController->SPADEvents = Utilities->LoadFileInt(SessionFile);
    TrainController->SPADRisks = Utilities->LoadFileInt(SessionFile);
    TrainController->CrashedTrains = Utilities->LoadFileInt(SessionFile);
    TrainController->Derailments = Utilities->LoadFileInt(SessionFile);
    TrainController->TotLateArrMins = Utilities->LoadFileDouble(SessionFile);
    TrainController->TotEarlyArrMins = Utilities->LoadFileDouble(SessionFile);
    TrainController->TotLatePassMins = Utilities->LoadFileDouble(SessionFile);
    TrainController->TotEarlyPassMins = Utilities->LoadFileDouble(SessionFile);
    TrainController->TotLateDepMins = Utilities->LoadFileDouble(SessionFile);
    Utilities->CallLogPop(1212);
}

// ---------------------------------------------------------------------------

bool TInterface::CheckInterface(int Caller, std::ifstream &SessionFile)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",CheckInterface");

    AnsiString OpMode = "";

    if(!Utilities->CheckAndReadFileString(SessionFile, OpMode))
    {
        Utilities->CallLogPop(1767);
        return false;
    }
    else if((OpMode != "PreStart") && (OpMode != "NotPreStart"))
    {
        Utilities->CallLogPop(1768);
        return false;
    }
    if(!Utilities->CheckFileStringZeroDelimiter(SessionFile)) // RailwayTitle
    {
        Utilities->CallLogPop(1213);
        return false;
    }
    if(!Utilities->CheckFileStringZeroDelimiter(SessionFile)) // TimetableTitle
    {
        Utilities->CallLogPop(1214);
        return false;
    }
    if(!Utilities->CheckFileBool(SessionFile))
    {
        Utilities->CallLogPop(1216);
        return false;
    }
    if(!Utilities->CheckFileBool(SessionFile))
    {
        Utilities->CallLogPop(1217);
        return false;
    }
    if(!Utilities->CheckFileBool(SessionFile))
    {
        Utilities->CallLogPop(1218);
        return false;
    }
    if(!Utilities->CheckFileInt(SessionFile, -1000000, 1000000))
    {
        Utilities->CallLogPop(1409);
        return false;
    }
    if(!Utilities->CheckFileInt(SessionFile, -1000000, 1000000))
    {
        Utilities->CallLogPop(1486);
        return false;
    }
    if(!Utilities->CheckFileInt(SessionFile, -1000000, 1000000))
    {
        Utilities->CallLogPop(1487);
        return false;
    }
    if(!Utilities->CheckFileInt(SessionFile, -1000000, 1000000))
    {
        Utilities->CallLogPop(1488);
        return false;
    }
    if(!Utilities->CheckFileInt(SessionFile, -1000000, 1000000))
    {
        Utilities->CallLogPop(1489);
        return false;
    }
    if(!Utilities->CheckFileInt(SessionFile, -1000000, 1000000))
    {
        Utilities->CallLogPop(1528);
        return false;
    }
    if(!Utilities->CheckFileStringZeroDelimiter(SessionFile))
    {
        Utilities->CallLogPop(1725);
        return false;
    }
    if(!Utilities->CheckFileStringZeroDelimiter(SessionFile))
    {
        Utilities->CallLogPop(1726);
        return false;
    }
    if(!Utilities->CheckFileStringZeroDelimiter(SessionFile))
    {
        Utilities->CallLogPop(1727);
        return false;
    }
    if(!Utilities->CheckFileStringZeroDelimiter(SessionFile))
    {
        Utilities->CallLogPop(1728);
        return false;
    }
    if(!Utilities->CheckFileStringZeroDelimiter(SessionFile))
    {
        Utilities->CallLogPop(1730);
        return false;
    }
    if(!Utilities->CheckFileStringZeroDelimiter(SessionFile))
    {
        Utilities->CallLogPop(1731);
        return false;
    }
    if(!Utilities->CheckFileStringZeroDelimiter(SessionFile))
    {
        Utilities->CallLogPop(1732);
        return false;
    }
    if(!Utilities->CheckFileStringZeroDelimiter(SessionFile))
    {
        Utilities->CallLogPop(1733);
        return false;
    }
    if(!Utilities->CheckFileStringZeroDelimiter(SessionFile))
    {
        Utilities->CallLogPop(1734);
        return false;
    }
    if(!Utilities->CheckFileStringZeroDelimiter(SessionFile))
    {
        Utilities->CallLogPop(1789);
        return false;
    }

    if(!Utilities->CheckFileInt(SessionFile, 0, 1000000))
    {
        Utilities->CallLogPop(1737);
        return false;
    }
    if(!Utilities->CheckFileInt(SessionFile, 0, 1000000))
    {
        Utilities->CallLogPop(1738);
        return false;
    }
    if(!Utilities->CheckFileInt(SessionFile, 0, 1000000))
    {
        Utilities->CallLogPop(1739);
        return false;
    }
    if(!Utilities->CheckFileInt(SessionFile, 0, 1000000))
    {
        Utilities->CallLogPop(1740);
        return false;
    }
    if(!Utilities->CheckFileInt(SessionFile, 0, 1000000))
    {
        Utilities->CallLogPop(1741);
        return false;
    }
    if(!Utilities->CheckFileInt(SessionFile, 0, 1000000))
    {
        Utilities->CallLogPop(1742);
        return false;
    }
    if(!Utilities->CheckFileInt(SessionFile, 0, 1000000))
    {
        Utilities->CallLogPop(1743);
        return false;
    }
    if(!Utilities->CheckFileInt(SessionFile, 0, 1000000))
    {
        Utilities->CallLogPop(1744);
        return false;
    }
    if(!Utilities->CheckFileInt(SessionFile, 0, 1000000))
    {
        Utilities->CallLogPop(1745);
        return false;
    }
    if(!Utilities->CheckFileInt(SessionFile, 0, 1000000))
    {
        Utilities->CallLogPop(1746);
        return false;
    }
    if(!Utilities->CheckFileInt(SessionFile, 0, 1000000))
    {
        Utilities->CallLogPop(1747);
        return false;
    }
    if(!Utilities->CheckFileInt(SessionFile, 0, 1000000))
    {
        Utilities->CallLogPop(1748);
        return false;
    }
    if(!Utilities->CheckFileInt(SessionFile, 0, 1000000))
    {
        Utilities->CallLogPop(1749);
        return false;
    }
    if(!Utilities->CheckFileInt(SessionFile, 0, 1000000))
    {
        Utilities->CallLogPop(1750);
        return false;
    }
    if(!Utilities->CheckFileInt(SessionFile, 0, 1000000))
    {
        Utilities->CallLogPop(1751);
        return false;
    }
    if(!Utilities->CheckFileInt(SessionFile, 0, 1000000))
    {
        Utilities->CallLogPop(1752);
        return false;
    }

    if(!Utilities->CheckFileDouble(SessionFile))
    {
        Utilities->CallLogPop(1753);
        return false;
    }
    if(!Utilities->CheckFileDouble(SessionFile))
    {
        Utilities->CallLogPop(1754);
        return false;
    }
    if(!Utilities->CheckFileDouble(SessionFile))
    {
        Utilities->CallLogPop(1755);
        return false;
    }
    if(!Utilities->CheckFileDouble(SessionFile))
    {
        Utilities->CallLogPop(1756);
        return false;
    }
    if(!Utilities->CheckFileDouble(SessionFile))
    {
        Utilities->CallLogPop(1757);
        return false;
    }
    Utilities->CallLogPop(1219);
    return true;
}

// ---------------------------------------------------------------------------

bool TInterface::SaveTimetableToSessionFile(int Caller, std::ofstream &SessionFile, AnsiString SessionFileStr)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",SaveTimetableToSessionFile," + SessionFileStr);
    if(!FileExists(TempTTFileName))
    {
        Utilities->CallLogPop(1862);
        return false;
    }

    SessionFile.close(); // close & re-open in append & binary mode so LF characters copy as LFs & not CRLFs
    SessionFile.open(SessionFileStr.c_str(), std::ios_base::app | std::ios_base::binary); // file pointer set to end

    int Handle = FileOpen(TempTTFileName, fmOpenRead);
    int Count = 0;

    while(Handle < 0) // this type of file use failed when used in SaveTempTimetableFile when used to resave timetable.tmp from
                      // temp .ttb file, but changed that to avoid so many rapid file actions in quick succession & been OK since
                      // then, but nevertheless have 10 retries before giving message to be on safe side
    {
        Handle = FileOpen(TempTTFileName, fmOpenRead);
        Count++;
        Delay(1, 50); // 50mSec delay between tries
        if(Count > 10)
        {
            ShowMessage("Failed to open temporary timetable file.  Unable to save the session");
            Utilities->CallLogPop(1221);
            return false;
        }
    }

    char *Buffer = new char[10000];
    int BytesRead;

    while(true)
    {
        BytesRead = FileRead(Handle, Buffer, 10000);
        SessionFile.write(Buffer, BytesRead);
        if(BytesRead < 10000)
            break;
    }
    delete Buffer;
    FileClose(Handle);

    SessionFile.close(); // close & re-open in append & text out mode as before so can write text
    SessionFile.open(SessionFileStr.c_str(), std::ios_base::app | std::ios_base::out); // file pointer set to end

    Utilities->SaveFileString(SessionFile, "***End***"); // marker for end of timetable
// now need to save the TrainOperatingData so can be loaded back into the timetable as is
    Utilities->SaveFileInt(SessionFile, TrainController->TrainDataVector.size());
    for(unsigned int x = 0; x < TrainController->TrainDataVector.size(); x++)
    {
        Utilities->SaveFileInt(SessionFile, TrainController->TrainDataVector.at(x).TrainOperatingDataVector.size());
        for(unsigned int y = 0; y < TrainController->TrainDataVector.at(x).TrainOperatingDataVector.size(); y++)
        {
            Utilities->SaveFileInt(SessionFile, TrainController->TrainDataVector.at(x).TrainOperatingDataVector.at(y).TrainID);
            Utilities->SaveFileInt(SessionFile, (short)(TrainController->TrainDataVector.at(x).TrainOperatingDataVector.at(y).EventReported));
            Utilities->SaveFileInt(SessionFile, (short)(TrainController->TrainDataVector.at(x).TrainOperatingDataVector.at(y).RunningEntry));
        }
    }
    Utilities->CallLogPop(1220);
    return true;
}

// ---------------------------------------------------------------------------

bool TInterface::SaveTimetableToErrorFile(int Caller, std::ofstream &ErrorFile, AnsiString ErrorFileStr, AnsiString TimetableFileName)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",SaveTimetableToErrorFile," + ErrorFileStr + "," + TimetableFileName);
    if(!FileExists(TimetableFileName))
    {
        Utilities->CallLogPop(1863);
        return false;
    }

    ErrorFile.close(); // close & re-open in append & binary mode so LF characters copy as LFs & not CRLFs
    ErrorFile.open(ErrorFileStr.c_str(), std::ios_base::app | std::ios_base::binary); // file pointer set to end

    int Handle = FileOpen(TimetableFileName, fmOpenRead);
    int Count = 0;

    while(Handle < 0) // this type of file use failed when used in SaveTempTimetableFile when used to resave timetable.tmp from
                      // temp .ttb file, but changed that to avoid so many rapid file actions in quick succession & been OK since
                      // then, but nevertheless have 10 retries before giving message to be on safe side
    {
        Handle = FileOpen(TimetableFileName, fmOpenRead);
        Count++;
        Delay(5, 50); // 50mSec delay between tries
        if(Count > 10)
        {
            Utilities->CallLogPop(1835);
            return false;
        }
    }

    char *Buffer = new char[10000];
    int BytesRead;

    while(true)
    {
        BytesRead = FileRead(Handle, Buffer, 10000);
        ErrorFile.write(Buffer, BytesRead);
        if(BytesRead < 10000)
            break;
    }
    delete Buffer;
    FileClose(Handle);

    ErrorFile.close(); // close & re-open in append & text out mode as before so can write text
    ErrorFile.open(ErrorFileStr.c_str(), std::ios_base::app | std::ios_base::out); // file pointer set to end

    Utilities->SaveFileString(ErrorFile, "***End of timetable***"); // marker for end of timetable
    Utilities->CallLogPop(1836);
    return true;
}

// ---------------------------------------------------------------------------

bool TInterface::LoadTimetableFromSessionFile(int Caller, std::ifstream &SessionFile)
    // the .ttb section is delimited by "***End***"
    // create the temporary timetable file in the working folder exactly like the original
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",LoadTimetableFromSessionFile");
    // reset all message flags, stops them being given twice (shouldn't be needed here but add for safety) //new at v2.4.0
    TrainController->SSHigh = false;
    TrainController->MRSHigh = false;
    TrainController->MRSLow = false;
    TrainController->MassHigh = false;
    TrainController->BFHigh = false;
    TrainController->BFLow = false;
    TrainController->PwrHigh = false;
    TrainController->SigSHigh = false;
    TrainController->SigSLow = false;
    if((TempTTFileName != "") && FileExists(TempTTFileName))
    {
        DeleteFile(TempTTFileName);
    }
    int TempTTFileNumber = 0;

    while(FileExists(CurDir + "\\TmpTT" + AnsiString(TempTTFileNumber) + ".tmp"))
    {
        TempTTFileNumber++;
    }
    TempTTFileName = CurDir + "\\TmpTT" + AnsiString(TempTTFileNumber) + ".tmp";

    std::ofstream TTBFile(TempTTFileName.c_str()); // use text mode as SessionFile is in text mode, so CRLFs read as LFs, and LFs write as CRLFs.
    int Count;
    char Zero = '\0';

    if(!TTBFile.fail())
    {
        char *Buffer = new char[10000]; // can't use LoadFileString as that expects a '\0' delimiter
        char TempChar = (char)(SessionFile.peek()); // have a look at the next character, if it's '\n'
        if(TempChar == '\n')
            SessionFile.get(TempChar); // then get rid of it, else leave it in as part of the first line
        if(!SessionFile.getline(Buffer, 10000, '\0'))
        {
            TTBFile.close();
            DeleteFile(TempTTFileName);
            delete Buffer;
            Utilities->CallLogPop(1222);
            return false;
        }
        Count = 0;
        for(int x = 0; x < 10000; x++)
        {
            if(Buffer[x] != '\0')
                Count++;
            else
                break;
        }
        while(AnsiString(Buffer) != "***End***")
        {
            TTBFile.write(Buffer, Count);
            TTBFile.write(&Zero, 1);
// TTBFile.write(&NewLine, 1);
            if(!SessionFile.getline(Buffer, 10000, '\0'))
            {
                TTBFile.close();
                DeleteFile(TempTTFileName);
                delete Buffer;
                Utilities->CallLogPop(1223);
                return false;
            }
            Count = 0;
            for(int x = 0; x < 10000; x++)
            {
                if(Buffer[x] != '\0')
                    Count++;
                else
                    break;
            }
        }
        TTBFile.close();
        delete Buffer;
// SaveTempTimetableFile(1, TTBFileName); no need, already has required name
// now create the internal timetable from the .tmp file
        bool GiveMessagesFalse = false;
        bool CheckLocationsExistInRailwayTrue = true;
        if(TrainController->TimetableIntegrityCheck(1, TempTTFileName.c_str(), GiveMessagesFalse, CheckLocationsExistInRailwayTrue))
        {
            std::ifstream TTBLFile(TempTTFileName.c_str(), std::ios_base::binary);
            if(TTBLFile.is_open())
            {
                bool SessionFileTrue = true;
                if(!(BuildTrainDataVectorForLoadFile(1, TTBLFile, GiveMessagesFalse, CheckLocationsExistInRailwayTrue, SessionFileTrue)))
                {
                    TTBLFile.close();
                    DeleteFile(TempTTFileName);
                    Utilities->CallLogPop(1224);
                    return false;
                }
            }
            else
            {
                DeleteFile(TempTTFileName);
                Utilities->CallLogPop(1225);
                return false;
            }
        } // if(FileIntegrityCheck(TTBFileName.c_str()))
        else
        {
            DeleteFile(TempTTFileName);
            Utilities->CallLogPop(1226);
            return false;
        }
// DeleteFile(TempTTFileName); no, need to save it for later session saves

        // now need to load the TrainOperatingData so can be loaded back into the timetable
        int NumberOfTrainEntries = Utilities->LoadFileInt(SessionFile);
        if(NumberOfTrainEntries != (int)(TrainController->TrainDataVector.size()))
        {
            Utilities->CallLogPop(1811);
            return false;
        }
        for(int x = 0; x < NumberOfTrainEntries; x++)
        {
            int NumberOfTrains = Utilities->LoadFileInt(SessionFile);
            if(NumberOfTrains != (int)(TrainController->TrainDataVector.at(x).TrainOperatingDataVector.size()))
            {
                Utilities->CallLogPop(1812);
                return false;
            }
            for(int y = 0; y < NumberOfTrains; y++)
            {
                TrainController->TrainDataVector.at(x).TrainOperatingDataVector.at(y).TrainID = Utilities->LoadFileInt(SessionFile);
                TrainController->TrainDataVector.at(x).TrainOperatingDataVector.at(y).EventReported = (TActionEventType)(Utilities->LoadFileInt(SessionFile));
                TrainController->TrainDataVector.at(x).TrainOperatingDataVector.at(y).RunningEntry = (TRunningEntry)(Utilities->LoadFileInt(SessionFile));
            }
        }
        Utilities->CallLogPop(1227);
        return true;
    }
    else
    {
        Utilities->CallLogPop(1228);
        return false;
    }
}

// ---------------------------------------------------------------------------

bool TInterface::CheckTimetableFromSessionFile(int Caller, std::ifstream &SessionFile)
    // the .ttb section is delimited by '\0' followed by "***End***" & has been saved in binary mode, i.e. no '\0'
    // string delimiters between entries, this check function just checks the (non-zero terminated) string entries in the file without
    // trying to build a timetable - that's done during load
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",CheckTimetableFromSessionFile");
    AnsiString OutString;

    if(!Utilities->CheckAndReadFileString(SessionFile, OutString))
    {
        Utilities->CallLogPop(1229);
        return false;
    }
    while(OutString != "***End***")
    {
        if(!Utilities->CheckAndReadFileString(SessionFile, OutString))
        {
            Utilities->CallLogPop(1230);
            return false;
        }
    }
// now need to check the TrainOperatingData, which was saved in text mode
    if(SessionFile.fail())
    {
        Utilities->CallLogPop(1231);
        return false;
    }
    int NumberOfTrainEntries;

    if(!Utilities->CheckAndReadFileInt(SessionFile, 0, 10000, NumberOfTrainEntries))
    {
        Utilities->CallLogPop(1232);
        return false;
    }
    for(int x = 0; x < NumberOfTrainEntries; x++)
    {
        int NumberOfTrains;
        if(!Utilities->CheckAndReadFileInt(SessionFile, 0, 10000, NumberOfTrains))
        {
            Utilities->CallLogPop(1233);
            return false;
        }
        for(int y = 0; y < NumberOfTrains; y++)
        {
            if(!Utilities->CheckFileInt(SessionFile, -1, 1000000)) // TrainID
            {
                Utilities->CallLogPop(1234);
                return false;
            }
            if(!Utilities->CheckFileInt(SessionFile, 0, 30)) // EventReported
            {
                Utilities->CallLogPop(1235);
                return false;
            }
            if(!Utilities->CheckFileInt(SessionFile, 0, 2)) // RunningEntry
            {
                Utilities->CallLogPop(1236);
                return false;
            }
        }
    }
    Utilities->CallLogPop(1237);
    return true;
}

// ---------------------------------------------------------------------------

bool TInterface::BuildTrainDataVectorForLoadFile(int Caller, std::ifstream &TTBLFile, bool GiveMessages, bool CheckLocationsExistInRailway, bool SessionFile)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",BuildTrainDataVectorForLoadFile," + AnsiString((short)GiveMessages));
    TrainController->TrainDataVector.clear(); // get rid of any earlier timetable
    bool EndOfFile = false;
    int Count = 0;
    char *TrainTimetableString = new char[10000]; // enough for ~ 200 stations at 50 chars/station, should be adequate!

    while(!EndOfFile)
    {
        TTBLFile.getline(TrainTimetableString, 10000, '\0');
        if(TTBLFile.eof() && (TrainTimetableString[0] == '\0')) // stores a null in 1st position if doesn't load any characters
        { // may still have eof even if read a line (no CRLF at end), and
                                                               // if so need to process it
            EndOfFile = true;
            break;
        }
        AnsiString OneLine(TrainTimetableString);
        bool FinalCallTrue = true;
        while((Count == 0) && !TrainController->ProcessOneTimetableLine(3, Count, OneLine, EndOfFile, FinalCallTrue, GiveMessages,
            CheckLocationsExistInRailway)) // get rid of lines before the start time
        {
            TTBLFile.getline(TrainTimetableString, 10000, '\0');
            if(TTBLFile.eof() && (TrainTimetableString[0] == '\0')) // see above
            {
                TTBLFile.close();
                throw Exception("Timetable FinalCall error - no start time on own line, Count = 0");
            }
            OneLine = AnsiString(TrainTimetableString);
        }
        // here when have accepted the start time
        if(Count == 0)
        {
            Count++; // increment past the start time
            TTBLFile.getline(TrainTimetableString, 10000, '\0'); // get next line after start time
            if(TTBLFile.eof() && (TrainTimetableString[0] == '\0')) // see above
            {
                EndOfFile = true;
                OneLine = "";
            }
            else
                OneLine = AnsiString(TrainTimetableString);
        }
        if(!TrainController->ProcessOneTimetableLine(4, Count, OneLine, EndOfFile, FinalCallTrue, GiveMessages, CheckLocationsExistInRailway))
        {
            TTBLFile.close();
            throw Exception("Timetable FinalCall error in processing one timetable line, Count = " + AnsiString(Count));
        }
        if(EndOfFile && (Count < 2)) // Timetable must contain at least two relevant lines, one for start time and at least one train
        {
            TTBLFile.close();
            throw Exception("Timetable FinalCall error - too few or no relevant entries, Count = " + AnsiString(Count));
        }
        Count++;
    }
    TTBLFile.close();
    delete TrainTimetableString;
// here when first pass actions completed successfully
    if(!TrainController->SecondPassActions(0, GiveMessages)) // Check for matching join/split HeadCodes, check increasing times & matching split/join
    // times, complete arrival & departure times for each TT line, check & complete train info for each type of start,
    // messages given in function if errors & vector cleared
    {
        if(GiveMessages)
            ShowMessage("Timetable secondary integrity check failed - unable to load");
        Utilities->CallLogPop(1238);
        return false;
    }
    else
    {
// TimetableLoaded = true;
        if(!SessionFile) // new timetable being loaded so need new TimetableTitle, if SessionFile true then already have it from LoadInterface
        {
            for(int x = TimetableDialog->FileName.Length(); x > 0; x--)
            {
                if(TimetableDialog->FileName[x] == '\\')
                {
                    TimetableTitle = TimetableDialog->FileName.SubString(x + 1, TimetableDialog->FileName.Length() - x - 4);
                    SetCaption(4);
                    break;
                }
            }
        }
// if(GiveMessages) //only set BaseMode if have manually loaded a timetable changed for the below in v2.4.0
        if(!SessionFile) // only set BaseMode if have manually loaded a timetable, i.e SessionFile is false
        {
            Level1Mode = BaseMode;
            SetLevel1Mode(28);
        }
    }
    Utilities->CallLogPop(1239);
    return true;
}

// ---------------------------------------------------------------------------

bool TInterface::BuildTrainDataVectorForValidateFile(int Caller, std::ifstream &TTBLFile, bool GiveMessages, bool CheckLocationsExistInRailway)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",BuildTrainDataVectorForValidateFile," + AnsiString((short)GiveMessages));
    TrainController->TrainDataVector.clear(); // get rid of any earlier timetable
    bool EndOfFile = false;
    int Count = 0;
    char *TrainTimetableString = new char[10000]; // enough for ~ 200 stations at 50 chars/station, should be adequate!

    while(!EndOfFile)
    {
        TTBLFile.getline(TrainTimetableString, 10000, '\0');
        if(TTBLFile.eof() && (TrainTimetableString[0] == '\0')) // stores a null in 1st position if doesn't load any characters
        { // may still have eof even if read a line (no CRLF at end), and
                                                               // if so need to process it
            EndOfFile = true;
            break;
        }
        AnsiString OneLine(TrainTimetableString);
        bool FinalCallTrue = true;
        while((Count == 0) && !TrainController->ProcessOneTimetableLine(0, Count, OneLine, EndOfFile, FinalCallTrue, GiveMessages,
            CheckLocationsExistInRailway)) // get rid of lines before the start time
        {
            TTBLFile.getline(TrainTimetableString, 10000, '\0');
            if(TTBLFile.eof() && (TrainTimetableString[0] == '\0')) // see above
            {
                TTBLFile.close();
                throw Exception("Timetable FinalCall error - no start time on own line, Count = 0");
            }
            OneLine = AnsiString(TrainTimetableString);
        }
        // here when have accepted the start time
        if(Count == 0)
        {
            Count++; // increment past the start time
            TTBLFile.getline(TrainTimetableString, 10000, '\0'); // get next line after start time
            if(TTBLFile.eof() && (TrainTimetableString[0] == '\0')) // see above
            {
                EndOfFile = true;
                OneLine = "";
            }
            else
                OneLine = AnsiString(TrainTimetableString);
        }
        if(!TrainController->ProcessOneTimetableLine(1, Count, OneLine, EndOfFile, FinalCallTrue, GiveMessages, CheckLocationsExistInRailway))
        {
            TTBLFile.close();
            throw Exception("Timetable FinalCall error in processing one timetable line, Count = " + AnsiString(Count));
        }
        if(EndOfFile && (Count < 2)) // Timetable must contain at least two relevant lines, one for start time and at least one train
        {
            TTBLFile.close();
            throw Exception("Timetable FinalCall error - too few or no relevant entries, Count = " + AnsiString(Count));
        }
        Count++;
    }
    TTBLFile.close();
    delete TrainTimetableString;
// here when first pass actions completed successfully
    if(!TrainController->SecondPassActions(1, GiveMessages)) // Check for matching join/split HeadCodes, check increasing times & matching split/join
    // times, complete arrival & departure times for each TT line, check & complete train info for each type of start,
    // messages given in function if errors & vector cleared
    {
// if(GiveMessages) ShowMessage("Timetable secondary integrity check failed");
// above dropped in v2.4.0 as all called functions give own messages
        Utilities->CallLogPop(1665);
        return false;
    }
    Utilities->CallLogPop(1666);
    return true;
}

// ---------------------------------------------------------------------------

bool TInterface::SessionFileIntegrityCheck(int Caller, AnsiString FileName)
/* Here need to check as far as timetable, then go back and load the track, because the timetable compilation check
                  relies on the track vector and map being in place.  Won't affect the later load because the vector and map are cleared
                  before loading.

                  Although this appears cumbersome, I initially used tellg() & seekg() to reposition the file pointer without having to do
                  all this backtracking.  However after many problems I eventually found that tellg() sometimes returns false values,
                  which cause problems when reloaded using seekg().  This must be a compiler problem, though I could find no mention of it
                  in the CBuilder4 issues list or on the web.  The full write-up is at the end of this unit.

                  Also, with hindsight, I wish I had just saved and reloaded the timetable vectors rather than the text of the timetable.  That
                  would probably have been easier.  To change it now though would cause compatibility problems with sessions created by earlier versions.
*/
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",SessionFileIntegrityCheck," + FileName);
    std::ifstream InFile(FileName.c_str());
// first pass as far as timetable
    int NumberOfActiveElements;
    bool GraphicsFollow = false;

    if(InFile.is_open())
    {
        if(!Utilities->CheckFileStringZeroDelimiter(InFile))
    // expected to be "***Interface***" for original version or "Version + : ***Interface***" for later releases + ExcessLCDownMins added as float after v2.2.0
        {
            InFile.close();
            Utilities->CallLogPop(1240);
            return false;
        }
        if(!CheckInterface(0, InFile))
        {
            InFile.close();
            Utilities->CallLogPop(1241);
            return false;
        }
        // check track elements
        if(!Utilities->CheckAndCompareFileString(InFile, "***Track***"))
        {
            InFile.close();
            Utilities->CallLogPop(1242);
            return false;
        }
        if(!Track->CheckTrackElementsInFile(2, NumberOfActiveElements, GraphicsFollow, InFile))
        {
            InFile.close();
            Utilities->CallLogPop(1243);
            return false;
        }
        if(InFile.fail())
        {
            InFile.close();
            Utilities->CallLogPop(1244);
            return false;
        }
        // check text elements
        if(!Utilities->CheckAndCompareFileString(InFile, "***Text***"))
        {
            InFile.close();
            Utilities->CallLogPop(1245);
            return false;
        }
        if(!TextHandler->CheckTextElementsInFile(1, InFile))
        {
            InFile.close();
            Utilities->CallLogPop(1246);
            return false;
        }
        if(InFile.fail())
        {
            InFile.close();
            Utilities->CallLogPop(1247);
            return false;
        }
        // check PrefDir elements
        if(!Utilities->CheckAndCompareFileString(InFile, "***PrefDirs***"))
        {
            InFile.close();
            Utilities->CallLogPop(1248);
            return false;
        }
        if(!EveryPrefDir->CheckOnePrefDir(1, NumberOfActiveElements, InFile))
        {
            InFile.close();
            Utilities->CallLogPop(1249);
            return false;
        }
        if(InFile.fail())
        {
            InFile.close();
            Utilities->CallLogPop(1250);
            return false;
        }
        // check graphics
        if(GraphicsFollow)
        {
            if(!Track->CheckUserGraphics(1, InFile, CurDir + "\\" + USERGRAPHICS_DIR_NAME)) // include path to Graphics folder
            {
                InFile.close();
                Utilities->CallLogPop(2187);
                return false;
            }
            if(InFile.fail())
            {
                InFile.close();
                Utilities->CallLogPop(2188);
                return false;
            }
        }
        // check routes
        if(!Utilities->CheckAndCompareFileString(InFile, "***Routes***"))
        {
            InFile.close();
            Utilities->CallLogPop(1251);
            return false;
        }
        if(!AllRoutes->CheckRoutes(0, NumberOfActiveElements, InFile))
        {
            InFile.close();
            Utilities->CallLogPop(1252);
            return false;
        }
        if(InFile.fail())
        {
            InFile.close();
            Utilities->CallLogPop(1253);
            return false;
        }
        // check LockedRoutes
        if(!Utilities->CheckAndCompareFileString(InFile, "***Locked routes***"))
        {
            InFile.close();
            Utilities->CallLogPop(1254);
            return false;
        }
        if(!TrainController->CheckSessionLockedRoutes(0, InFile))
        {
            InFile.close();
            Utilities->CallLogPop(1255);
            return false;
        }
        if(InFile.fail())
        {
            InFile.close();
            Utilities->CallLogPop(1256);
            return false;
        }
        // check ContinuationAutoSigs
        if(!Utilities->CheckAndCompareFileString(InFile, "***ContinuationAutoSigEntries***"))
        {
            InFile.close();
            Utilities->CallLogPop(1257);
            return false;
        }
        if(!TrainController->CheckSessionContinuationAutoSigEntries(0, InFile))
        {
            InFile.close();
            Utilities->CallLogPop(1258);
            return false;
        }
        if(InFile.fail())
        {
            InFile.close();
            Utilities->CallLogPop(1259);
            return false;
        }
        // check BarriersDownVector, but ensure backwards compatibility with earlier files which don't have this section
        AnsiString TempString = Utilities->LoadFileString(InFile);
        if((TempString != "***BarriersDownVector***") && (TempString != "***Timetable***"))
        {
            InFile.close();
            Utilities->CallLogPop(1964);
            return false;
        }
        if(TempString == "***BarriersDownVector***")
        {
            if(!Track->CheckActiveLCVector(0, InFile))
            {
                InFile.close();
                Utilities->CallLogPop(1965);
                return false;
            }
            if(InFile.fail())
            {
                InFile.close();
                Utilities->CallLogPop(1966);
                return false;
            }
            if(!Utilities->CheckAndCompareFileString(InFile, "***Timetable***"))
            {
                InFile.close();
                Utilities->CallLogPop(1260);
                return false;
            }
        }
        // check timetable (marker string already checked immediately above)
        if(!CheckTimetableFromSessionFile(0, InFile))
        {
            InFile.close();
            Utilities->CallLogPop(1261);
            return false;
        }
        if(InFile.fail())
        {
            InFile.close();
            Utilities->CallLogPop(1262);
            return false;
        }
    }
    else
    {
        InFile.close();
        ShowMessage("Session file failed to open, unable to load session.  Ensure that there is a folder named " + SESSION_DIR_NAME +
            " in the folder where the 'Railway.exe' program file resides");
        Utilities->CallLogPop(1263);
        return false;
    }

// now ready for the 2nd pass for timetable loading and checking
    InFile.close();
    InFile.open(FileName.c_str());
    if(InFile.is_open())
    {
        if(!Utilities->CheckFileStringZeroDelimiter(InFile))
        {
            InFile.close();
            Utilities->CallLogPop(1264);
            return false;
        }
        if(!CheckInterface(1, InFile))
        {
            InFile.close();
            Utilities->CallLogPop(1265);
            return false;
        }
        // load track elements
        if(!Utilities->CheckAndCompareFileString(InFile, "***Track***"))
        {
            InFile.close();
            Utilities->CallLogPop(1266);
            return false;
        }
        bool GraphicsFollow = false;
        Track->LoadTrack(2, InFile, GraphicsFollow); // load the track this time
        if(InFile.fail())
        {
            InFile.close();
            Utilities->CallLogPop(1267);
            return false;
        }
        // check text elements
        if(!Utilities->CheckAndCompareFileString(InFile, "***Text***"))
        {
            InFile.close();
            Utilities->CallLogPop(1268);
            return false;
        }
        if(!TextHandler->CheckTextElementsInFile(2, InFile))
        {
            InFile.close();
            Utilities->CallLogPop(1269);
            return false;
        }
        if(InFile.fail())
        {
            InFile.close();
            Utilities->CallLogPop(1270);
            return false;
        }
        // check PrefDir elements
        if(!Utilities->CheckAndCompareFileString(InFile, "***PrefDirs***"))
        {
            InFile.close();
            Utilities->CallLogPop(1271);
            return false;
        }
        if(!EveryPrefDir->CheckOnePrefDir(2, NumberOfActiveElements, InFile))
        {
            InFile.close();
            Utilities->CallLogPop(1272);
            return false;
        }
        if(InFile.fail())
        {
            InFile.close();
            Utilities->CallLogPop(1273);
            return false;
        }
        // check graphics
        if(GraphicsFollow)
        {
            if(!Track->CheckUserGraphics(2, InFile, CurDir + "\\" + USERGRAPHICS_DIR_NAME)) // include path to Graphics folder
            {
                InFile.close();
                Utilities->CallLogPop(2189);
                return false;
            }
            if(InFile.fail())
            {
                InFile.close();
                Utilities->CallLogPop(2190);
                return false;
            }
        }
        // check routes
        if(!Utilities->CheckAndCompareFileString(InFile, "***Routes***"))
        {
            InFile.close();
            Utilities->CallLogPop(1274);
            return false;
        }
        if(!AllRoutes->CheckRoutes(1, NumberOfActiveElements, InFile))
        {
            InFile.close();
            Utilities->CallLogPop(1275);
            return false;
        }
        if(InFile.fail())
        {
            InFile.close();
            Utilities->CallLogPop(1276);
            return false;
        }
        // check LockedRoutes
        if(!Utilities->CheckAndCompareFileString(InFile, "***Locked routes***"))
        {
            InFile.close();
            Utilities->CallLogPop(1277);
            return false;
        }
        if(!TrainController->CheckSessionLockedRoutes(1, InFile))
        {
            InFile.close();
            Utilities->CallLogPop(1278);
            return false;
        }
        if(InFile.fail())
        {
            InFile.close();
            Utilities->CallLogPop(1279);
            return false;
        }
        // check ContinuationAutoSigs
        if(!Utilities->CheckAndCompareFileString(InFile, "***ContinuationAutoSigEntries***"))
        {
            InFile.close();
            Utilities->CallLogPop(1280);
            return false;
        }
        if(!TrainController->CheckSessionContinuationAutoSigEntries(1, InFile))
        {
            InFile.close();
            Utilities->CallLogPop(1281);
            return false;
        }
        if(InFile.fail())
        {
            InFile.close();
            Utilities->CallLogPop(1282);
            return false;
        }
        // check BarriersDownVector, but ensure backwards compatibility with earlier files which don't have this section
        AnsiString TempString = Utilities->LoadFileString(InFile);
        if((TempString != "***BarriersDownVector***") && (TempString != "***Timetable***"))
        {
            InFile.close();
            Utilities->CallLogPop(1967);
            return false;
        }
        if(TempString == "***BarriersDownVector***")
        {
            if(!Track->CheckActiveLCVector(0, InFile))
            {
                InFile.close();
                Utilities->CallLogPop(1968);
                return false;
            }
            if(InFile.fail())
            {
                InFile.close();
                Utilities->CallLogPop(1969);
                return false;
            }
            if(!Utilities->CheckAndCompareFileString(InFile, "***Timetable***"))
            {
                InFile.close();
                Utilities->CallLogPop(1283);
                return false;
            }
        }
        // check timetable (marker string already checked)
        if(!LoadTimetableFromSessionFile(1, InFile))
        {
            InFile.close();
            Utilities->CallLogPop(1284);
            return false;
        }
        if(InFile.fail())
        {
            InFile.close();
            Utilities->CallLogPop(1285);
            return false;
        }
        // check timetable clock
        if(!Utilities->CheckAndCompareFileString(InFile, "***TimetableClock***"))
        {
            InFile.close();
            Utilities->CallLogPop(1286);
            return false;
        }
        if(!Utilities->CheckFileDouble(InFile))
        {
            InFile.close();
            Utilities->CallLogPop(1287);
            return false;
        }
        // check trains
        if(!Utilities->CheckAndCompareFileString(InFile, "***Trains***"))
        {
            InFile.close();
            Utilities->CallLogPop(1288);
            return false;
        }
        if(!TrainController->CheckSessionTrains(0, InFile))
        {
            InFile.close();
            Utilities->CallLogPop(1289);
            return false;
        }
        if(InFile.fail())
        {
            InFile.close();
            Utilities->CallLogPop(1290);
            return false;
        }
        if(!Utilities->CheckAndCompareFileString(InFile, "***Performance file***"))
        {
            InFile.close();
            Utilities->CallLogPop(1291);
            return false;
        }
        if(!CheckPerformanceFile(0, InFile))
        {
            InFile.close();
            Utilities->CallLogPop(1292);
            return false;
        }
        char TempChar;
        InFile.get(TempChar);
        while(!InFile.eof() && ((TempChar == '\n') || (TempChar == '\0'))) // when emerge from here either have eof or '*'
        {
            InFile.get(TempChar);
        }
        if(!InFile.eof()) // additional checks needed
        {
            if(!Utilities->CheckFileString(InFile))
            {
                InFile.close();
                Utilities->CallLogPop(2198);
                return false;
            }
            if(!Utilities->CheckFileInt(InFile, 0, 1000000)) // TrainController->AvHoursIntValue
            {
                InFile.close();
                Utilities->CallLogPop(2199);
                return false;
            }
            if(!Utilities->CheckFileInt(InFile, 0, 1000000)) // number of train failures
            {
                InFile.close();
                Utilities->CallLogPop(2200);
                return false;
            }
            // now check any failed trains along with their OriginalPowerAtRail values
            Utilities->CheckFileString(InFile); // discard "***Failed Trains***"
            int IDVal;
            if(!Utilities->CheckAndReadFileInt(InFile, -1, 1000000, IDVal)) // train ID or -1 for no more failed trains,
            {
                InFile.close();
                Utilities->CallLogPop(2201);
                return false;
            }
            double PowerDouble;
            while(IDVal != -1)
            {
                Utilities->CheckFileDouble(InFile); // original power
                if(!Utilities->CheckAndReadFileInt(InFile, -1, 1000000, IDVal))
                {
                    InFile.close();
                    Utilities->CallLogPop(2202);
                    return false;
                }
            }
        }
        InFile.close();
    }
    else
    {
        InFile.close();
        Utilities->CallLogPop(1293);
        return false;
    }
    Utilities->CallLogPop(1294);
    return true;
}

// ---------------------------------------------------------------------------

void TInterface::LoadPerformanceFile(int Caller, std::ifstream &InFile)
    // Note that the file integrity has already been checked using CheckPerformanceFile
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",LoadPerformanceFile");
    AnsiString TempString = "", Line1 = "", Line2 = "", Line3 = "", Line4 = "", Line5 = "";
    char *Buffer = new char[1000];
    char TempChar;

    InFile.get(TempChar); // '\n'
    InFile.getline(Buffer, 1000);
    TempString = AnsiString(Buffer);
    while(TempString != "***End of performance file***")
    {
        PerformanceLogBox->Lines->Add(TempString);
        Utilities->PerformanceFile << TempString.c_str() << '\n';
        InFile.getline(Buffer, 1000);
        TempString = AnsiString(Buffer);
    }
    delete Buffer;
    Utilities->CallLogPop(1295);
}

// ---------------------------------------------------------------------------

bool TInterface::CheckPerformanceFile(int Caller, std::ifstream &InFile)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",CheckPerformanceFile");
    AnsiString TempString = "";
    char TempChar;

    InFile.get(TempChar);
    if(TempChar != '\n')
    {
        Utilities->CallLogPop(1296);
        return false;
    }
    if(!Utilities->CheckAndReadFileString(InFile, TempString))
    {
        Utilities->CallLogPop(1297);
        return false;
    }
    while(TempString != "***End of performance file***")
    {
        if(!Utilities->CheckAndReadFileString(InFile, TempString))
        {
            Utilities->CallLogPop(1298);
            return false;
        }
    }
    Utilities->CallLogPop(1299);
    return true;
}

// ---------------------------------------------------------------------------

void TInterface::SavePerformanceFile(int Caller, std::ofstream &OutFile)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",SavePerformanceFile");
    AnsiString Text = PerformanceLogBox->Text;

    while(Text != "")
    {
        AnsiString OneLine = Text.SubString(1, Text.Pos('\x0D'));
        while((OneLine.Length() > 0) && OneLine[OneLine.Length()] < ' ')
            OneLine.SetLength(OneLine.Length() - 1); // get rid of trailing control characters
        Text = Text.SubString(Text.Pos('\x0D'), Text.Length());
        while((Text.Length() > 0) && Text[1] < ' ')
            Text = Text.SubString(2, (Text.Length() - 1)); // get rid of leading control characters
        OutFile << OneLine.c_str() << '\n';
    }
    Utilities->CallLogPop(1300);
}

// ---------------------------------------------------------------------------

void TInterface::SetRouteButtonsInfoCaptionAndRouteNotStarted(int Caller)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",SetRouteButtonsInfoCaptionAndRouteNotStarted");
    if(EveryPrefDir->PrefDirSize() > 0)
    {
        if(AutoSigsFlag)
        {
            AutoSigsButton->Enabled = false;
            SigPrefButton->Enabled = true;
            UnrestrictedButton->Enabled = true;
            InfoPanel->Visible = true;
            if(Level2OperMode == PreStart)
                InfoPanel->Caption = "PRE-START:  Select AUTOMATIC SIGNAL ROUTE start signal, or left click points to change manually";
            else
                InfoPanel->Caption = "OPERATING:  Select AUTOMATIC SIGNAL ROUTE start signal, or left click points to change manually";
            InfoCaptionStore = InfoPanel->Caption;
        }
        else if(ConsecSignalsRoute) // PreferredRoute always same as ConsecSignalsRoute
        {
            AutoSigsButton->Enabled = true;
            SigPrefButton->Enabled = false;
            UnrestrictedButton->Enabled = true;
            InfoPanel->Visible = true;
            if(Level2OperMode == PreStart)
                InfoPanel->Caption = "PRE-START:  Select PREFERRED ROUTE start signal, or left click points to change manually";
            else
                InfoPanel->Caption = "OPERATING:  Select PREFERRED ROUTE start signal, or left click points to change manually";
            InfoCaptionStore = InfoPanel->Caption;
        }
        else
        {
            AutoSigsButton->Enabled = true;
            SigPrefButton->Enabled = true;
            UnrestrictedButton->Enabled = false;
            InfoPanel->Visible = true;
            if(Level2OperMode == PreStart)
                InfoPanel->Caption = "PRE-START:  Select UNRESTRICTED ROUTE start location, or left click points to change manually";
            else
                InfoPanel->Caption = "OPERATING:  Select UNRESTRICTED ROUTE start location, or left click points to change manually";
            InfoCaptionStore = InfoPanel->Caption;
        }
    }
    else
    {
        AutoSigsButton->Enabled = false;
        SigPrefButton->Enabled = false;
        UnrestrictedButton->Enabled = false;
        InfoPanel->Visible = true;
        if(Level2OperMode == PreStart)
            InfoPanel->Caption = "PRE-START:  Select UNRESTRICTED ROUTE start location, or left click points to change manually";
        else
            InfoPanel->Caption = "OPERATING:  Select UNRESTRICTED ROUTE start location, or left click points to change manually";
        InfoCaptionStore = InfoPanel->Caption;
    }
    if(AllRoutes->AllRoutesVector.size() > AllRoutes->LockedRouteVector.size())
    {
        RouteCancelButton->Enabled = true;
    }
    else
    {
        RouteCancelButton->Enabled = false;
    }
    RouteMode = RouteNotStarted;
    AutoRouteStartMarker->PlotOriginal(37, Display); // so start marker will replot if had selected start before pause & zoom
    SigRouteStartMarker->PlotOriginal(38, Display);
    NonSigRouteStartMarker->PlotOriginal(41, Display);
    Utilities->CallLogPop(1301);
}

// ---------------------------------------------------------------------------

void TInterface::SetPausedOrZoomedInfoCaption(int Caller)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",SetPausedOrZoomedInfoCaption");
    if(Display->ZoomOutFlag)
    {
        InfoPanel->Visible = true;
        InfoPanel->Caption = "Left click screen to zoom in at that position";
    }
    else if(Level2OperMode == Paused)
    {
        InfoPanel->Visible = true;
        InfoPanel->Caption = "PAUSED:  Railway state changes disabled";
    }
// otherwise do nothing
    Utilities->CallLogPop(1302);
}

// ---------------------------------------------------------------------------

void TInterface::DisableRouteButtons(int Caller)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",DisableRouteButtons");
    RouteCancelButton->Enabled = false;
    AutoSigsButton->Enabled = false;
    SigPrefButton->Enabled = false;
    UnrestrictedButton->Enabled = false;
    Utilities->CallLogPop(1303);
}

// ---------------------------------------------------------------------------

void TInterface::SaveErrorFile()
    // no need for call logging as already failed
{
/*
   In order to reload as a session file:

   NB:  Don't change it to a .txt file, as the '\0' characters [shown as a small square in wordpad] will be changed to spaces if it is subsequently saved
   strip out:-

   [since adding user graphics after prefdirs need to take this into account]

   up to but excluding  ***Interface***
   from & including  ***ConstructPrefDir PrefDirVector***
   to but excluding ***PrefDirs***
   if there is a single line ***UserGraphics*** delete it (won't be present if no graphics)
   from & including  ***ConstructRoute PrefDirVector***
   to but excluding ***Routes***
   from & including ***ChangingLCVector*** to but excluding ***Timetable*** [if have ***No timetable loaded*** then can't use as a session file]
   from & including ***No editing timetable*** or ***Editing timetable - [title]***
   to but excluding ***TimetableClock***
   and save as a .ssn file.

   In order to load as a railway file:

   NB:  Don't change it to a .txt file, as the '\0' characters will be changed to spaces if it is subsequently saved

   note or copy the version information at the top of the file
   copy the two numbers on rows 7 & 8 below ***Interface*** (i.e ***Interface*** = row 0) on their own lines immediately after the ***Track*** line (these become DisplayOffsetH & V)
   strip out up to but excluding ***Track*** - this is needed to keep the \0 entry at end of ***Track*** [shown as a small square in wordpad]
   add the version number either before or instead of ***Track***, ensuring that the \0 is retained
   the next line should contain the number of active elements - leave that in.
   strip out ***Text*** including the \0
   strip out from & including  ***ConstructPrefDir PrefDirVector*** to & including ***PrefDirs*** (and the \0)
   strip out from & including  ***ConstructRoute PrefDirVector*** to the end of the file
   rename as .dev or .rly file

   BUT - note that signals (and points, though they won't show) will be set as they were left.  To reset to red, load a suitable timetable & select
   'Operate' then 'Exit operation'.
*/

/*
   In order to extract a timetable:

   NB:  Don't change it to a .txt file, as the '\0' characters will be changed to spaces if it is subsequently saved

   set wordwrap to window on
   strip out all to and including ***Timetable*** or ***Editing timetable.... depending which is to be saved
   ensure any text before start time ends with /0, otherwise don't need the \0
   strip out all after the final /0 immediately before ***End*** or ***End of timetable***, but ensure leave the final /0
   save as a .ttb file
*/

    Screen->Cursor = TCursor(-11); // Hourglass;
    AnsiString ErrorFileStr = CurDir + "\\errorlog.err";
    std::ofstream ErrorFile(ErrorFileStr.c_str());

    if(!(ErrorFile.fail()))
    {
// save mouse position relative to mainscreen
        int ScreenX = Mouse->CursorPos.x - MainScreen->ClientOrigin.x;
        int ScreenY = Mouse->CursorPos.y - MainScreen->ClientOrigin.y;
        AnsiString MouseStr = "Posx: " + AnsiString(ScreenX) + "; Posy: " + AnsiString(ScreenY);
        Utilities->SaveFileString(ErrorFile, MouseStr);
        Utilities->SaveFileInt(ErrorFile, MissedTicks);
        Utilities->SaveFileInt(ErrorFile, TotalTicks);

// save call stack
        Utilities->SaveFileString(ErrorFile, "***Call stack***");
        for(unsigned int x = 0; x < Utilities->CallLog.size(); x++)
        {
            AnsiString Item = Utilities->CallLog.at(x);
            ErrorFile << Item.c_str() << '\n';
        }
// save event log
        Utilities->SaveFileString(ErrorFile, "***Event log***");
        for(unsigned int x = 0; x < Utilities->EventLog.size(); x++)
        {
            AnsiString Item = Utilities->EventLog.at(x);
            ErrorFile << Item.c_str() << '\n';
        }
// save interface
        Utilities->SaveFileString(ErrorFile, "***Interface***");
        SaveInterface(1, ErrorFile);
// save track elements
        Utilities->SaveFileString(ErrorFile, "***Track***");
        if(Track->UserGraphicVector.empty())
        {
            Track->SaveTrack(2, ErrorFile, false); // false for no graphics (**Active elements** saved as marker)
        }
        else
        {
            Track->SaveTrack(12, ErrorFile, true); // true for graphics to be saved (**Active elements**1 saved as marker)
        }
// save text elements
        Utilities->SaveFileString(ErrorFile, "***Text***");
        TextHandler->SaveText(3, ErrorFile);
// save ConstructPrefDir PrefDirVector elements
        Utilities->SaveFileString(ErrorFile, "***ConstructPrefDir PrefDirVector***");
        ConstructPrefDir->SavePrefDirVector(7, ErrorFile);
// save ConstructPrefDir SearchVector elements
        Utilities->SaveFileString(ErrorFile, "***ConstructPrefDir SearchVector***");
        ConstructPrefDir->SaveSearchVector(0, ErrorFile);
// save EveryPrefDir elements
        Utilities->SaveFileString(ErrorFile, "***PrefDirs***");
        EveryPrefDir->SavePrefDirVector(3, ErrorFile);
        if(!Track->UserGraphicVector.empty())
        {
            // save user graphics
            Utilities->SaveFileString(ErrorFile, "***UserGraphics***");
            Track->SaveUserGraphics(3, ErrorFile);
        }
// save ConstructRoute PrefDirVector
        Utilities->SaveFileString(ErrorFile, "***ConstructRoute PrefDirVector***");
        ConstructRoute->SavePrefDirVector(4, ErrorFile);
// save ConstructRoute SearchVector
        Utilities->SaveFileString(ErrorFile, "***ConstructRoute SearchVector***");
        ConstructRoute->SaveSearchVector(1, ErrorFile);
// save AllRoutes
        Utilities->SaveFileString(ErrorFile, "***Routes***");
        AllRoutes->SaveRoutes(1, ErrorFile);
// save LockedRoutes
        Utilities->SaveFileString(ErrorFile, "***Locked routes***");
        TrainController->SaveSessionLockedRoutes(1, ErrorFile);
// save ContinuationAutoSigEntries
        Utilities->SaveFileString(ErrorFile, "***ContinuationAutoSigEntries***");
        TrainController->SaveSessionContinuationAutoSigEntries(1, ErrorFile);
// save BarriersDownVector
        Utilities->SaveFileString(ErrorFile, "***BarriersDownVector***");
        Track->SaveSessionBarriersDownVector(1, ErrorFile);
// save ChangingLCVector
        Utilities->SaveFileString(ErrorFile, "***ChangingLCVector***");
        Track->SaveChangingLCVector(0, ErrorFile);
// save loaded timetable
        if(TimetableTitle == "")
            Utilities->SaveFileString(ErrorFile, "***No timetable loaded***");
        else
        {
            Utilities->SaveFileString(ErrorFile, "***Timetable***");
            if(!(SaveTimetableToSessionFile(1, ErrorFile, ErrorFileStr)))
            {
                Utilities->SaveFileString(ErrorFile, "***Loaded timetable failed to save***");
            }
        }
// save editing timetable
        if(CreateEditTTTitle == "")
            Utilities->SaveFileString(ErrorFile, "***No editing timetable***");
        else
        {
            Utilities->SaveFileString(ErrorFile, "***Editing timetable - " + CreateEditTTTitle + "***");
            if(!(SaveTimetableToErrorFile(1, ErrorFile, ErrorFileStr, CreateEditTTFileName)))
            {
                Utilities->SaveFileString(ErrorFile, "***Editing timetable failed to save***");
            }
        }
// save TimetableClock
        Utilities->SaveFileString(ErrorFile, "***TimetableClock***");
        Utilities->SaveFileDouble(ErrorFile, double(TrainController->TTClockTime));
// save trains
        Utilities->SaveFileString(ErrorFile, "***Trains***");
        TrainController->SaveSessionTrains(1, ErrorFile);
// save performance file
        Utilities->SaveFileString(ErrorFile, "***Performance file***");
        SavePerformanceFile(1, ErrorFile);
        Utilities->SaveFileString(ErrorFile, "***End of performance file***");
// addition at v2.4.1 to save TrainController->AvHoursIntValue + any future additions
        Utilities->SaveFileString(ErrorFile, "***Additions after v2.3.1***");
        Utilities->SaveFileInt(ErrorFile, TrainController->AvHoursIntValue);
        Utilities->SaveFileInt(ErrorFile, TrainController->NumFailures);
        Utilities->SaveFileString(ErrorFile, "***Failed Trains***");
        for(unsigned int x = 0; x < TrainController->TrainVector.size(); x++)
        {
            if(TrainController->TrainVectorAt(70, x).TrainFailed)
            {
                Utilities->SaveFileInt(ErrorFile, TrainController->TrainVectorAt(71, x).TrainID);
                Utilities->SaveFileDouble(ErrorFile, TrainController->TrainVectorAt(72, x).OriginalPowerAtRail);
            }
        }
        Utilities->SaveFileInt(ErrorFile, -1); // marker for end of failed trains
        Utilities->SaveFileString(ErrorFile, "End of file at v2.4.1");
// end of v2.4.1 addition

        ErrorFile.close();
    }
    else
    {
        TrainController->StopTTClockMessage(6, "Error file failed to open, error log won't be saved.");
    }
    Screen->Cursor = TCursor(-2); // Arrow
}

// ---------------------------------------------------------------------------

void TInterface::SaveTempTimetableFile(int Caller, AnsiString InFileName)
    // the .ttb section is delimited by '\n' followed by "***End***"
    // first create a .ttb file in the working folder exactly like the original

    // Note: this type of file use failed when used to resave timetable.tmp from temp .ttb file, but changed that to avoid so many rapid
    // file actions in quick succession & been OK since then, but nevertheless keep the 10 retries before giving message to be on safe side
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",SaveTempTimetableFile");
    if((TempTTFileName != "") && FileExists(TempTTFileName))
    {
        DeleteFile(TempTTFileName);
    }
    int TempTTFileNumber = 0;

    while(FileExists(CurDir + "\\TmpTT" + AnsiString(TempTTFileNumber) + ".tmp"))
    {
        TempTTFileNumber++;
    }
    TempTTFileName = CurDir + "\\TmpTT" + AnsiString(TempTTFileNumber) + ".tmp";
    int InHandle = FileOpen(InFileName, fmOpenRead);
    int Count = 0;

    while(InHandle < 0) // sometimes fails, have 10 retries before giving message
    {
        InHandle = FileOpen(InFileName, fmOpenRead);
        Count++;
        Delay(2, 50); // 50mSec delay between tries
        if(Count > 10)
        {
            ShowMessage("Failed to open timetable file, make sure it's not open in another application");
            Utilities->CallLogPop(1400);
            return;
        }
    }
    int OutHandle = FileCreate(TempTTFileName);

    Count = 0;
    while(OutHandle < 0) // sometimes fails, have 10 retries before giving message
    {
        OutHandle = FileCreate(TempTTFileName);
        Count++;
        Delay(3, 50); // 50mSec delay between tries
        if(Count > 10)
        {
            ShowMessage("Failed to save temporary timetable file, sessions can't be saved - try again, may only be a temporary problem");
            FileClose(InHandle);
            Utilities->CallLogPop(1401);
            return;
        }
    }
    int CountIn, CountOut;
    char *Buffer = new char[10000]; // can't use LoadFileString as that expects a '\0' delimiter

    while(true)
    {
        CountIn = FileRead(InHandle, Buffer, 10000);
        CountOut = FileWrite(OutHandle, Buffer, CountIn);
        if(CountOut != CountIn)
        {
            ShowMessage("Error in writing to the temporary timetable file, sessions can't be saved - try again, may only be a temporary problem");
            delete Buffer;
            FileClose(InHandle);
            FileClose(OutHandle);
            Utilities->CallLogPop(1402);
            return;
        }
        if(CountIn < 10000)
            break;
    }
    delete Buffer;
    FileClose(InHandle);
    FileClose(OutHandle);
    Utilities->CallLogPop(1403);
}

// ---------------------------------------------------------------------------

void TInterface::SetTrackLengths(int Caller, int Distance, int SpeedLimit) // Distance & SpeedLimit are -1 for no change to that parameter
/*
                  Rules: Platforms are fixed length elements of 100m and aren't changed. Variable length elements can't be less than 20m.
                  above changed in v2.4.0 to be variable as other track, but if <50m or >200m a warning is given

                  Enter with DistanceVector containing the PrefDir to be set, Distance containing the required sum of all element lengths,
                  and SpeedLimit containing the speed limit.  If either of these is -1 (can be -1 separately) then no change is to be made to it.
                  Return for an empty DistanceVector.  Deal first with a single element in the vector, giving a message if there is a platform there.
                  Now set exit link position (XLinkPos) value for the first element in DistanceVector by checking which link connects to the second
                  element in the vector, and give a warning message if fail to find it.  Now have to make two passes through the vector, firstly to
                  sum the fixed lengths, count the number of variable length elements and set the speed limit, and secondly to set the lengths.
                  Firstly store the first XLinkPos so don't have to recalculate it for the second pass.  On the first pass examine each element,
                  incrementing the variable element count or summing the fixed length count as go along, and setting the speed limits providing
                  SpeedLimit isn't -1.  If Distance was -1 then still go through but don't count anything, just set the speed limits.  Recalculate
                  the next XLinkPos for each succeeding element.
                  After the first pass return if Distance was -1 as in that case have now finished.  Otherwise check if the distance to be set is less than
                  the minimum possible within the rules, and if so give a message and return.  Also give a warning message if there aren't any variable length
                  elements.  Now enter the second pass.  In this the length of each variable element is set to int(RemainingDistance/RemainingVarElements) and
                  fixed length elements are ignored.  After each variable length element is set the RemainingDistance and RemainingVarElements are recalculated
                  ready for the next setting.  In this way there is never more than 1 difference between any two variable length elements and the total
                  distance sums exactly to the value required.  A check is made after every variable length element has been set to see whether RemainingDistance
                  and RemainingVarElements are zero, and if they don't reach zero together (which they should after the last variable length element has
                  been set), an error message is given.
*/

{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",SetLengths," + AnsiString(Distance) + "," + AnsiString(SpeedLimit));
    bool FoundFlag;

// ResetDistanceElements(4);
    if(ConstructPrefDir->PrefDirSize() == 0)
    {
        Utilities->CallLogPop(608);
        return;
    }
// must have PrefDir size of at least 2

// first pass to count number of variable length elements, sum fixed lengths & set speed limit
// for version in v2.4.0 have no fixed length elements but leave code as is as much as possible
    int VarElements = 0; // FixedLength = 0; drop this in v2.4.0
    bool NamedLocPresent = false;

    for(unsigned int x = 0; x < ConstructPrefDir->PrefDirSize(); x++)
    {
        TPrefDirElement PrefDirElement = ConstructPrefDir->GetFixedPrefDirElementAt(167, x);
        TTrackElement & TE = Track->TrackElementAt(34, Track->GetVectorPositionFromTrackMap(28, PrefDirElement.HLoc, PrefDirElement.VLoc, FoundFlag));
        if(!FoundFlag)
        {
            throw Exception("Error - failed to find track element at " + AnsiString(TE.HLoc) + " & " + AnsiString(TE.VLoc) + " in SetLengths");
        }
        if((Distance != -1) && (!Track->IsPlatformOrNamedNonStationLocationPresent(2, TE.HLoc, TE.VLoc)))
            VarElements++;
        else if((Distance != -1) && (Track->IsPlatformOrNamedNonStationLocationPresent(3, TE.HLoc, TE.VLoc)))
        {
            VarElements++; // added in v2.4.0 for no fixed elements
            NamedLocPresent = true;
// FixedLength+= DefaultTrackLength; dropped in v2.4.0 for no fixed elements
        }

        if((PrefDirElement.GetELinkPos() < 2) && (PrefDirElement.GetXLinkPos() < 2)) // could be points
        {
            if(SpeedLimit != -1)
                TE.SpeedLimit01 = SpeedLimit;
        }
        else
        {
            if(SpeedLimit != -1)
                TE.SpeedLimit23 = SpeedLimit;
        }
    }
    if(Distance == -1) // can't return before this as need to set speed limits
    {
        Utilities->CallLogPop(612);
        return;
    }

    if((NamedLocPresent) && (VarElements > 0) && ((Distance / VarElements) < 50)) // these two additions are for in v2.4.0
    {
        ShowMessage("Note: Named location elements are quite short. If they are too short the simulation might depart too far from reality.");
    }

    if((NamedLocPresent) && (VarElements > 0) && ((Distance / VarElements) > 200))
    {
        ShowMessage("Note: Named location elements are quite long. If they are too long the simulation might depart too far from reality.");
    }

/* if(NamedLocPresent)    as was
       {
               ShowMessage("Named location lengths won't be changed");
       }
*/

    if((VarElements * 20) > Distance) // removed '+ FixedLength'
    {
        ShowMessage("Required distance is less than the minimum, will set each element to the minimum (20m)");
        Distance = (VarElements * 20); // removed '+ FixedLength'
    }
    if(VarElements == 0)
    {
// ShowMessage("Unable to set distance as all elements are of fixed length");  as was
        ShowMessage("No elements selected"); // probably don't need this but include for safety
        Utilities->CallLogPop(613);
        return;
    }

// second pass, set variable lengths
    int RemainingDistance = Distance, RemainingVarElements = VarElements, NextLength = RemainingDistance / VarElements; // removed ' - FixedLength'

    for(unsigned int x = 0; x < ConstructPrefDir->PrefDirSize(); x++)
    {
        TPrefDirElement PrefDirElement = ConstructPrefDir->GetFixedPrefDirElementAt(168, x);
        TTrackElement & TE = Track->TrackElementAt(35, Track->GetVectorPositionFromTrackMap(29, PrefDirElement.HLoc, PrefDirElement.VLoc, FoundFlag));
// if(!Track->IsPlatformOrNamedNonStationLocationPresent(4, TE.HLoc, TE.VLoc)) //variable lengths  dropped in v2.4.0
// {
        if(NextLength < 20)
            NextLength = 20; // added for safety
        if(TE.TrackType == Points)
        {
            if((PrefDirElement.GetELinkPos() == 1) || (PrefDirElement.GetXLinkPos() == 1))
            {
                TE.Length01 = NextLength;
            }
            else
            {
                TE.Length23 = NextLength;
            }
        }
        else
        {
            if(PrefDirElement.GetELinkPos() < 2)
            {
                TE.Length01 = NextLength;
            }
            else
            {
                TE.Length23 = NextLength;
            }
        }
        RemainingDistance -= NextLength;
        RemainingVarElements--;
        if(RemainingVarElements > 0)
            NextLength = RemainingDistance / RemainingVarElements;
        else
            NextLength = 20;

/* removed these as using integer division & that sometimes problematic.  None of these errors ever reported but be safe
                       if((RemainingDistance == 0) && (RemainingVarElements != 0))
                       {
                               throw Exception("Error RemainingDistance == 0 & RemainingVarElements != 0");
                       }
                       if((RemainingDistance != 0) && (RemainingVarElements == 0))
                       {
                               throw Exception("Error RemainingDistance != 0 & RemainingVarElements == 0");
                       }
*/
// }
    }
    Utilities->CallLogPop(614);
}

// ---------------------------------------------------------------------------

void TInterface::SaveAsSubroutine(int Caller)
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",SaveAsSubroutine");
    if((Track->NoActiveOrInactiveTrack(4) && TextHandler->TextVectorSize(7) == 0) && Track->UserGraphicVector.empty())
        ShowMessage("Nothing to save!");
    else
    {
        if(Track->IsReadyForOperation())
        {
            SaveRailwayDialog->Filter = "Development file (*.dev)|*.dev|Railway file (*.rly)|*.rly";
        }
        else
            SaveRailwayDialog->Filter = "Development file (*.dev)|*.dev";
        if(SaveRailwayDialog->Execute())
        {
            Screen->Cursor = TCursor(-11); // Hourglass;
            TrainController->LogEvent("Save " + SaveRailwayDialog->FileName);
            AnsiString Extension = "";
            if(SaveRailwayDialog->FileName.Length() > 2)
            {
                Extension = AnsiString(SaveRailwayDialog->FileName).SubString(AnsiString(SaveRailwayDialog->FileName).Length() - 2, 3).UpperCase();
            }
            if((Extension == "DEV") || (Track->IsReadyForOperation() && (Extension == "RLY")))
            {
                std::ofstream VecFile(AnsiString(SaveRailwayDialog->FileName).c_str());
                if(!(VecFile.fail()))
                {
                    Utilities->SaveFileString(VecFile, ProgramVersion);
                    Utilities->SaveFileInt(VecFile, Display->DisplayOffsetHHome);
                    Utilities->SaveFileInt(VecFile, Display->DisplayOffsetVHome);
                    // save track elements
                    if(Track->UserGraphicVector.empty())
                    {
                        Track->SaveTrack(1, VecFile, false); // false for no graphics (**Active elements** saved as marker)
                    }
                    else
                    {
                        Track->SaveTrack(13, VecFile, true); // true for graphics to be saved (**Active elements**1 saved as marker)
                    }
                    // save text elements
                    TextHandler->SaveText(1, VecFile);
                    // save PrefDir elements
                    EveryPrefDir->SavePrefDirVector(1, VecFile);
                    if(!Track->UserGraphicVector.empty())
                    {
                        // save user graphics
                        Track->SaveUserGraphics(4, VecFile);
                    }
                    VecFile.close();
                    SavedFileName = SaveRailwayDialog->FileName; // includes the full PrefDir
                    if(SavedFileName != "") // shouldn't be "" at this stage but leave in as a safeguard
                    {
                        char LastChar = SavedFileName[SavedFileName.Length()];
                        if((LastChar == 'y') || (LastChar == 'Y'))
                        {
                            RlyFile = true;
                        }
                        else
                        {
                            RlyFile = false;
                        }
                    }
                    else
                    {
                        RlyFile = false;
                    }
                    FileChangedFlag = false;
                    for(int x = SaveRailwayDialog->FileName.Length(); x > 0; x--)
                    {
                        if(SaveRailwayDialog->FileName[x] == '\\')
                        {
                            RailwayTitle = SaveRailwayDialog->FileName.SubString(x + 1, SaveRailwayDialog->FileName.Length() - x - 4);
                            // TimetableTitle = ""; leave this as is, no need to unload a tt just because saved railway
                            SetCaption(7);
                            break;
                        }
                    }
                    Level1Mode = BaseMode;
                    SetLevel1Mode(13); // to disable the save option
                } // if(!(VecFile.fail()))
                else
                    ShowMessage("File open failed prior to save");
            } // else following  if(!Track->IsReadyForOperation() && (Extension != "DEV"))
            else
            {
                ShowMessage("Can't save: extension must be either '.dev', or '.rly' with railway ready for operation");
            }
            Screen->Cursor = TCursor(-2); // Arrow
        } //if(SaveRailwayDialog->Execute())
    }
    Utilities->CallLogPop(1546);
}

// ---------------------------------------------------------------------------

void TInterface::SetInitialTrackModeEditMenu()
{ // no need for caller or log as only setting values
    CutMenuItem->Visible = true;
    CopyMenuItem->Visible = true;
    FlipMenuItem->Visible = true;
    MirrorMenuItem->Visible = true;
    RotRightMenuItem->Visible = true;
    RotLeftMenuItem->Visible = true;
    RotateMenuItem->Visible = true;
    PasteMenuItem->Visible = true;
// PasteWithAttributesMenuItem->Visible = true;    //added at v2.2.0
    DeleteMenuItem->Visible = true;
    SelectLengthsMenuItem->Visible = true;
    ReselectMenuItem->Visible = true;

    CutMenuItem->Enabled = false;
    CopyMenuItem->Enabled = false;
    FlipMenuItem->Enabled = false;
    MirrorMenuItem->Enabled = false;
    RotRightMenuItem->Enabled = false;
    RotLeftMenuItem->Enabled = false;
    RotateMenuItem->Enabled = false;
    PasteMenuItem->Enabled = false;
// PasteWithAttributesMenuItem->Enabled = false; //new at v2.2.0
    DeleteMenuItem->Enabled = false;
    SelectLengthsMenuItem->Enabled = false;
    if(SelectionValid)
        ReselectMenuItem->Enabled = true;
    else
        ReselectMenuItem->Enabled = false;

    SelectBiDirPrefDirsMenuItem->Visible = false;
    CancelSelectionMenuItem->Enabled = true;
    SelectMenuItem->Enabled = true;

    if(NoRailway())
    {
        EditMenu->Enabled = false;
    }
    else
        EditMenu->Enabled = true;
}

// ---------------------------------------------------------------------------

void TInterface::SetInitialPrefDirModeEditMenu()
{ // no need for caller or log as only setting values
    EditMenu->Enabled = true;

    CutMenuItem->Visible = false;
    CopyMenuItem->Visible = false;
    FlipMenuItem->Visible = false;
    MirrorMenuItem->Visible = false;
    RotRightMenuItem->Visible = false;
    RotLeftMenuItem->Visible = false;
    RotateMenuItem->Visible = false;
    PasteMenuItem->Visible = false;
// PasteWithAttributesMenuItem->Visible = false;   //added at v2.2.0
    DeleteMenuItem->Visible = false;
    SelectLengthsMenuItem->Visible = false;
    ReselectMenuItem->Visible = false;

    SelectBiDirPrefDirsMenuItem->Visible = true;
    SelectBiDirPrefDirsMenuItem->Enabled = false;
    CancelSelectionMenuItem->Enabled = true;
    SelectMenuItem->Enabled = true;
}

// ---------------------------------------------------------------------------

bool TInterface::NoRailway()
{
    return ((Track->NoActiveOrInactiveTrack(5)) && (TextHandler->TextVectorSize(8) == 0) && Track->UserGraphicVector.empty());
}

// ---------------------------------------------------------------------------

void TInterface::ResetSelectRect()
{
    SelectRect.left = 0;
    SelectRect.right = 0;
    SelectRect.top = 0;
    SelectRect.bottom = 0;
}

// ---------------------------------------------------------------------------

bool TInterface::EraseLocationNameText(int Caller, AnsiString Name, int &HPos, int &VPos)
{ // return position of erased name in HPos & VPos, return true for found & erased
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",EraseLocationNameText," + Name);
    bool TextFound = false;

// if(Track->LocationNameMultiMap.find(Name) == Track->LocationNameMultiMap.end()) {} //name not in LocationNameMultiMap, so don't erase from TextVector  //condition dropped at v1.1.4 because of change in EnterLocationNames
/* else */ if(TextHandler->FindText(0, Name, HPos, VPos))
    {
        if(TextHandler->TextErase(4, HPos, VPos))
        {;
        } // condition not used
        TextFound = true;
    }
    Utilities->CallLogPop(1956);
    return TextFound;
}

// ---------------------------------------------------------------------------

void TInterface::AddLocationNameText(int Caller, AnsiString Name, int HPos, int VPos, bool UseEnteredPosition)
{
    if(Name == "")
    {
        return;
    }
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",AddLocationNameText," + Name + "," + AnsiString(HPos) + "," +
        AnsiString(VPos) + "," + AnsiString((short)UseEnteredPosition));
    int VPosHi, VPosLo, TextPosHi, TextPosLo;
    TFont *Font = Display->GetFont();

    if(!UseEnteredPosition)
    {
        if(!Track->FindHighestLowestAndLeftmostNamedElements(0, Name, VPosHi, VPosLo, HPos))
        {
            Utilities->CallLogPop(1561);
            return;
        }
        int Depth = abs(Font->Height); // Height may be negative - see C++Builder Help file
        TextPosHi = VPosHi + 20; // add depth of track element + 4 pixels
        TextPosLo = VPosLo - Depth - 4; // reduce by depth of font + 4 pixels
        int ScreenPosHi = (Display->DisplayOffsetV * 16) + 576;
        int ScreenPosLo = Display->DisplayOffsetV * 16;
        if(TextPosLo >= ScreenPosLo)
            VPos = TextPosLo; // if Lo value on screen then use that - displays above the location
        else if(TextPosHi < ScreenPosHi)
            VPos = TextPosHi;
        else
            VPos = ScreenPosLo + 288; // if location extends to or beyond height of screen the display in centre of screen
    }
    TTextItem TI(HPos, VPos, Name, Font);

    TI.Font = Font; // may have been changed in constructor when returned as reference
    TextHandler->EnterAndDisplayNewText(1, TI, HPos, VPos);
    Utilities->CallLogPop(1558);
}

// ---------------------------------------------------------------------------

void TInterface::TestFunction()
{
    try
    {
/*
        ShowMessage(
           "Interface->Left + Interface->Width " + UnicodeString(Interface->Left + Interface->Width) +
           "\nInterface->Left + MainScreen->Left + MainScreen->Width " +
           UnicodeString(Interface->Left + MainScreen->Left + MainScreen->Width) +
           "\n\nMainScreen->Width " + UnicodeString(MainScreen->Width) +
           "\nMainScreen->Height " + UnicodeString(MainScreen->Height) +
           "\nMainScreen->Top " + UnicodeString(MainScreen->Top) +
           "\nMainScreen->Left " + UnicodeString(MainScreen->Left) +
           " Right " + UnicodeString(MainScreen->Width + MainScreen->Left) +
           "\n\nInterface->Width " + UnicodeString(Interface->Width) +
           "\nInterface->Left " + UnicodeString(Interface->Left) +
           "\nInterface->Top " + UnicodeString(Interface->Top) +
           "\n\nScreenRightButton->Left " + UnicodeString(ScreenRightButton->Left)
           );
*/
/*
      for(unsigned int x=0; x<TrainController->TrainVector.size(); x++)
      {
          if((TrainController->TrainVectorAt(-1, x).HeadCode == "2K02") && (!TrainController->TrainVectorAt(-1, x).TrainOnContinuation(-1)))
          {
              TrainController->TrainVectorAt(-1, x).TrainFailurePending = true;
          }
      }
*/

// throw Exception("Test error");  //generate an error file

// ShowMessage("MissedTicks = " + AnsiString(MissedTicks) + "; TotalTicks = " + AnsiString(TotalTicks));

    }
    catch(const Exception &e)
    {
        ErrorLog(114, e.Message);
    }
}

// ---------------------------------------------------------------------------
/*
  void TInterface::LoadNormalSignalGlyphs(int Caller)    //changed - see below
  {
   Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",LoadNormalSignalGlyphs,");
   SpeedButton68->Glyph->LoadFromResourceName(0, "gl68"); SpeedButton69->Glyph->LoadFromResourceName(0, "gl69");
   SpeedButton70->Glyph->LoadFromResourceName(0, "gl70"); SpeedButton71->Glyph->LoadFromResourceName(0, "gl71");
   SpeedButton72->Glyph->LoadFromResourceName(0, "gl72"); SpeedButton73->Glyph->LoadFromResourceName(0, "gl73");
   SpeedButton74->Glyph->LoadFromResourceName(0, "gl74"); SpeedButton75->Glyph->LoadFromResourceName(0, "gl75");
   Utilities->CallLogPop(**);
  }

  //---------------------------------------------------------------------------

  void TInterface::LoadGroundSignalGlyphs(int Caller)    //changed - see below
  {
   Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",LoadGroundSignalGlyphs,");
   SpeedButton68->Glyph->LoadFromResourceName(0, "bm68grounddblred"); SpeedButton69->Glyph->LoadFromResourceName(0, "bm69grounddblred");
   SpeedButton70->Glyph->LoadFromResourceName(0, "bm70grounddblred"); SpeedButton71->Glyph->LoadFromResourceName(0, "bm71grounddblred");
   SpeedButton72->Glyph->LoadFromResourceName(0, "bm72grounddblred"); SpeedButton73->Glyph->LoadFromResourceName(0, "gl73grounddblred");
   SpeedButton74->Glyph->LoadFromResourceName(0, "gl74grounddblred"); SpeedButton75->Glyph->LoadFromResourceName(0, "bm75grounddblred");
   Utilities->CallLogPop(**);
  }
*/
// ---------------------------------------------------------------------------
void TInterface::LoadNormalSignalGlyphs(int Caller) // changed from the above at v2.3.0 so the signal glyphs change hands
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",LoadNormalSignalGlyphs,");
    SpeedButton68->Glyph = RailGraphics->SpeedBut68NormBlackGlyph;
    SpeedButton69->Glyph = RailGraphics->SpeedBut69NormBlackGlyph;
    SpeedButton70->Glyph = RailGraphics->SpeedBut70NormBlackGlyph;
    SpeedButton71->Glyph = RailGraphics->SpeedBut71NormBlackGlyph;
    SpeedButton72->Glyph = RailGraphics->SpeedBut72NormBlackGlyph;
    SpeedButton73->Glyph = RailGraphics->SpeedBut73NormBlackGlyph;
    SpeedButton74->Glyph = RailGraphics->SpeedBut74NormBlackGlyph;
    SpeedButton75->Glyph = RailGraphics->SpeedBut75NormBlackGlyph;
    Utilities->CallLogPop(1871);
}

// ---------------------------------------------------------------------------

void TInterface::LoadGroundSignalGlyphs(int Caller) // changed from the above at v2.3.0 so the signal glyphs change hands
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",LoadGroundSignalGlyphs,");
    SpeedButton68->Glyph = RailGraphics->SpeedBut68GrndBlackGlyph;
    SpeedButton69->Glyph = RailGraphics->SpeedBut69GrndBlackGlyph;
    SpeedButton70->Glyph = RailGraphics->SpeedBut70GrndBlackGlyph;
    SpeedButton71->Glyph = RailGraphics->SpeedBut71GrndBlackGlyph;
    SpeedButton72->Glyph = RailGraphics->SpeedBut72GrndBlackGlyph;
    SpeedButton73->Glyph = RailGraphics->SpeedBut73GrndBlackGlyph;
    SpeedButton74->Glyph = RailGraphics->SpeedBut74GrndBlackGlyph;
    SpeedButton75->Glyph = RailGraphics->SpeedBut75GrndBlackGlyph;
    Utilities->CallLogPop(1872);
}

// ---------------------------------------------------------------------------

void TInterface::UpdateOperatorActionPanel(int Caller) // new at v2.2.0
    // limit it to 20 entries max
{
    Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",UpdateOperatorActionPanel");
    if(TrainController->OpActionPanelHintDelayCounter >= 80)
    {
        OAListBox->Clear();
    }
    if((!OperatorActionPanel->Visible) || TrainController->OpTimeToActMultiMap.empty() || (TrainController->OpActionPanelHintDelayCounter < 80))
    // new at v2.2.0
    {
        Utilities->CallLogPop(2092);
        return;
    }
    AnsiString OpTimeToActDisplay;
    AnsiString OpTimeToActString;
    AnsiString HeadCode;
    float OpTimeToActFloat;
    TTrainController::THCandTrainPosParam HCandTrainPosParam;

    TrainController->OpTimeToActMultiMapIterator = TrainController->OpTimeToActMultiMap.begin();
    while(TrainController->OpTimeToActMultiMapIterator != TrainController->OpTimeToActMultiMap.end())
    {
        if(OAListBox->Items->Count >= 20)
        {
            break;
        }
        OpTimeToActFloat = TrainController->OpTimeToActMultiMapIterator->first;
        HCandTrainPosParam = TrainController->OpTimeToActMultiMapIterator->second;
        HeadCode = HCandTrainPosParam.first;
        if(OpTimeToActFloat < 0.25) // 15 secs estimated
        {
            OpTimeToActString = "NOW";
        }
        else if(OpTimeToActFloat < 1)
        {
            OpTimeToActString = "<1";
        }
        else
        {
            OpTimeToActString = AnsiString(floor(OpTimeToActFloat));
        }
        if(OpTimeToActFloat < 60)
        {
            OpTimeToActDisplay = HeadCode + AnsiString('\t') + OpTimeToActString;
            OAListBox->Items->Add(OpTimeToActDisplay); // original
        }
        TrainController->OpTimeToActMultiMapIterator++;
    }
    Utilities->CallLogPop(2093);
}

// ---------------------------------------------------------------------------

void TInterface::LoadUserGraphic(int Caller) // new at v2.4.0
{
    try
    {
        TrainController->LogEvent("LoadUserGraphic");
        Utilities->CallLog.push_back(Utilities->TimeStamp() + "," + AnsiString(Caller) + ",LoadUserGraphic");
        LoadUserGraphicDialog;
        if(LoadUserGraphicDialog->Execute())
        {
            TrainController->LogEvent("LoadUserGraphic " + LoadUserGraphicDialog->FileName);
            SelectedGraphicFileName = AnsiString(LoadUserGraphicDialog->FileName); // SelectedGraphicFileName is a class member
            TTrack::TUserGraphicMapEntry UGME;
            TTrack::TUserGraphicMap::iterator UGMIt = Track->UserGraphicMap.find(SelectedGraphicFileName);
            if(UGMIt == Track->UserGraphicMap.end()) // i.e. there isn't an entry for that filename so insert one, else take no action
            {
                UGME.first = SelectedGraphicFileName;
                TPicture *PicPtr = new TPicture;
                PicPtr->LoadFromFile(SelectedGraphicFileName);
                UGME.second = PicPtr;
                if(!Track->UserGraphicMap.insert(UGME).second) // if no failure then the new entry is inserted
                {
                    throw Exception("Map Insertion Error 1 - UserGraphicMap insertion failure for " + SelectedGraphicFileName);
                }
            }
            Level2TrackMode = AddGraphic;
            SetLevel2TrackMode(65);
        }
        Utilities->CallLogPop(2191);
    }
    catch(const EInvalidGraphic &e)
    {
        ShowMessage(
            "Incorrect file format, the file can't be loaded.\nEnsure that the file you want is a valid graphic file with extension .bmp, .gif, .jpg, or .png");
    }
    catch(const Exception &e)
    {
        ErrorLog(215, e.Message);
    }
}

// ---------------------------------------------------------------------------
/*
   Problems with ifstream reading (see 'SessionFileIntegrityCheck(AnsiString FileName)' above):-

   These problems were with Borland C++Builder 4.

   The functions saved in OldFiles\Backups220809Duringifstream testing were used for testing the odd behaviour where the
   ifstream pointer gave different characters using get() and getline(), when reading the timetable entries in the session
   file for 20/08/09 at 18:45 (saved).  All was well until point 48677 in the session file, when for some reason the
   getline(0 & get(0 gave different results.  Many earlier timetable strings had been read OK before that, and it wasn't
   clear what was special about this particular string.
   Later more detailed study found that on reading the string beginning at point 48605 (i.e. the one earlier than above),
   within function CheckNoNewLineAtStartNonZeroTerminatedFileString the file pointer (using tellg()) reduced from 48606 to
   48604 after reading the 'F' character.  Thereafter characters were read correctly but the pointer remained 2 too low.
   This is thought to be a flaw in the compiler.
   Later again additional tellg()s and a seekg()s were included in CheckNoNewLineAtStartNonZeroTerminatedFileString, and
   though these should have had no effect they somehow caused the next getline() within CheckTimetableFromSessionFile to
   read a null, even though the pointer had been reset to its value before the call to
   CheckNoNewLineAtStartNonZeroTerminatedFileString.  Again this seems to be a flaw in the compiler, where the pointer
   that is indicated by tellg and the true pointer within the system can be different.
   Tried the old c++ stream library to see if that worked but it was exactly the same.  Probably because the same code is
   used for both with the new library just defined within the std namespace.
   Success!!  Traced to the putback function failing.  It (apparently) can't be used if the file pointer has been altered
   after the last read that is to be put back.  Corrected that & the most recent session file checked out & loaded OK.
   (note - don't need the ifstream file to be open in output mode for the putback to work)
   But:  the earlier file - 18:45 as above - still fails to advance the file pointer in the middle of checking the
   timetable, it sticks at position 48601.  This position points to 'r' in 'Frh' just before a newline.  Also the file
   integrity is OK up to and after this sticking point.  Oddly though the loading function works fine (i.e. by bypassing
   the integrity check function), though the timetable isn't read directly, it is copied to a new stand-alone timetable
   file and that read by the program.
   Created a new version of CheckNoNewLineAtStartNonZeroTerminatedFileString with 'New' at end, & got rid of all the
   internal digressions & getlines.  This passed the earlier sticking point, but stuck later at 48677, i.e. the 'h' from
   'Frh' at the end of the entry following that for the earlier sticking point.  Here
   CheckNoNewLineAtStartNonZeroTerminatedFileStringNew works fine, with end pointer correctly set at 48680, i.e. after the
   newline, but the subsequent getline() function, although it retrieves the line correctly, the file pointer is set to
   48677, i.e. before the newline, so getline seems to fail to extract the newline character.  Still to check - why doesn't
   CheckNoNewLineAtStartNonZeroTerminatedFileStringNew see 'h' instead of '0' in the subsequent read?  If it did the two
   would tally, though would still be wrong.
   Further investigation:-  CheckNoNewLineAtStartNonZeroTerminatedFileStringNew doesn't seem to recognize the file pointer
   as set by seekg at 48677.  It continues to read at the point it left off earlier, whereas getline() does read at 48677
   & recovers 'h'.  Continuing to apply getline() after the above effect it is found that it doesn't extract newlines after
   reading further lines, but extracts them when read alone i.e. it reads a line then a null in succession, although the
   lines are only separated by single newline characters.

   Need to check:
   1. Does the file read correctly if only get() functions used without getline() and without resetting the file pointer?
   2. Does the file read correctly if only getline() functions used without get() and without resetting the file pointer?
   3.  Does the file read correctly if get() functions alternated with getline() but without resetting the file pointer?

   For 1:  Still goes wrong at usual place, reads 'h' at the same point.  Try not resetting the file pointer with seekg.
   Tried this - got past the earlier point but failed later with a reduction in file pointer after a character read.  In
   fact the reduction was by 40 bytes for reading a single comma!  Try without any tellg's - yes, that got past all the
   timetable OK.  So, works OK with just get() providing no tellg's (& no seekg's).

   For 2:  Works OK using getline().

   For 3:  Gets to end of timetable OK but the next tellg gives a wrong value.  Check if using getline() alone gives a
   wrong tellg.  Tried getline() alone, reached end of TT as before, but gave the same wrong file pos on using tellg.
   Try continuing to see if works OK in spite of tellg giving wrong result.  Yes it works OK.  Hence the problem seems to
   be tellg, which sometimes returns wrong results, and they corrupt things when used in seekg.

   Overall conclusion:  Avoid all tellg's & seekg's.  If need to reset a file position then close and reopen it.
*/

// ---------------------------------------------------------------------------
