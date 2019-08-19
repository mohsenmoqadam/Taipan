%% This is an -*- erlang -*- file.
%%
%% %CopyrightBegin%
%%
%% Copyright Ericsson AB 2010-2016. All Rights Reserved.
%%
%% Licensed under the Apache License, Version 2.0 (the "License");
%% you may not use this file except in compliance with the License.
%% You may obtain a copy of the License at
%%
%%     http://www.apache.org/licenses/LICENSE-2.0
%%
%% Unless required by applicable law or agreed to in writing, software
%% distributed under the License is distributed on an "AS IS" BASIS,
%% WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
%% See the License for the specific language governing permissions and
%% limitations under the License.
%%
%% %CopyrightEnd%

{application, wx,
 [{description, "Yet another graphics system"},
  {vsn, "1.8.3"},
  {modules,
   [
    %% Generated modules
  wxIcon, wxBitmapButton, wxImage, wxGraphicsContext, wxPreviewFrame, wxColourData, wxEvtHandler, wxFileDialog, wxFlexGridSizer, wxPrintDialogData, wxFocusEvent, wxMouseCaptureChangedEvent, wxDCOverlay, wxClipboardTextEvent, wxMoveEvent, wxChoicebook, wxSystemOptions, wxGridCellFloatRenderer, wxGridCellAttr, wxWindowDC, wxColourDialog, wxStatusBar, wxInitDialogEvent, wxEvent, wxXmlResource, wxTaskBarIconEvent, wxPrintout, wxSysColourChangedEvent, wxGridCellRenderer, wxListCtrl, wxLocale, wxGraphicsMatrix, wxBitmap, wxQueryNewPaletteEvent, wxRegion, wxCalendarCtrl, wxSizerItem, wxGridCellBoolRenderer, wxGridSizer, wxFrame, wxNavigationKeyEvent, wxGraphicsRenderer, wxMouseCaptureLostEvent, wxTextEntryDialog, wxIdleEvent, wxStyledTextCtrl, wxChoice, wxListItem, wxSpinCtrl, wxControlWithItems, wxMDIChildFrame, wxStdDialogButtonSizer, wxFontPickerEvent, wxPrintPreview, wxPrintData, wxDirPickerCtrl, wxKeyEvent, wxEraseEvent, wxRadioBox, wxGridCellEditor, wxPalette, wxGridCellNumberRenderer, wxSizeEvent, wxLogNull, wxClientDC, wxAuiManagerEvent, wxTreebook, wxSashLayoutWindow, wxGraphicsObject, wxGridEvent, wx_misc, wxPreviewCanvas, wxTextAttr, wxScrollWinEvent, wxJoystickEvent, wxGraphicsBrush, wxWindowDestroyEvent, wxSetCursorEvent, wxFontDialog, wxGridCellChoiceEditor, wxMirrorDC, wxActivateEvent, wxGraphicsFont, wxStaticText, wxControl, wxIconizeEvent, wxPrinter, wxStaticBitmap, wxFontPickerCtrl, wxGridBagSizer, wxListbook, wxScrollEvent, wxWindowCreateEvent, wxGridCellFloatEditor, wxStyledTextEvent, wxStaticBox, wxBufferedDC, wxTextCtrl, wxMaximizeEvent, wxDateEvent, wxCalendarEvent, wxGauge, wxGridCellTextEditor, wxShowEvent, wxBitmapDataObject, wxFindReplaceDialog, wxTextDataObject, wxGraphicsPath, wxMiniFrame, wxListEvent, wxCursor, wxDialog, wxTopLevelWindow, wxPaintDC, wxTreeCtrl, wxScreenDC, wxPopupWindow, wxFilePickerCtrl, wxPostScriptDC, wxGrid, wxAuiSimpleTabArt, wxScrolledWindow, wxSizerFlags, wxMask, wxFontData, wxSplitterEvent, wxScrollBar, wxMenuEvent, wxCheckBox, wxListItemAttr, wxAuiManager, wxFileDirPickerEvent, wxBoxSizer, wxPageSetupDialog, wxClipboard, wxMouseEvent, wxDirDialog, wxMenu, wxAuiPaneInfo, wxPaintEvent, wxSplitterWindow, wxProgressDialog, wxGridCellNumberEditor, wxListBox, wxNotebookEvent, wxColourPickerEvent, wxMenuItem, wxChildFocusEvent, wxMessageDialog, wxButton, wxDisplayChangedEvent, wxToggleButton, wxToolBar, wxGraphicsPen, wxPaletteChangedEvent, wxArtProvider, wxHtmlEasyPrinting, wxNotebook, wxPasswordEntryDialog, wxFindReplaceData, wxListView, wxSashWindow, wxBufferedPaintDC, wxColourPickerCtrl, wxContextMenuEvent, wxLayoutAlgorithm, wxCheckListBox, wxGridCellBoolEditor, wxMultiChoiceDialog, wxOverlay, wxAuiDockArt, wxHtmlWindow, wxComboBox, wxCommandEvent, wxPanel, wxDataObject, wxImageList, wxNotifyEvent, wxToolTip, wxSlider, wxSizer, wxGBSizerItem, wxPen, wxBrush, wxAuiNotebook, wxGLCanvas, wxAuiNotebookEvent, wxAcceleratorEntry, wxStaticBoxSizer, wxSashEvent, wxUpdateUIEvent, wxHelpEvent, wxPageSetupDialogData, wxMenuBar, wxMemoryDC, wxGridCellStringRenderer, wxTaskBarIcon, wxPopupTransientWindow, wxWindow, wxPrintDialog, wxFileDataObject, wxRadioButton, wxPickerBase, wxCloseEvent, wxCalendarDateAttr, wxDC, wxCaret, wxAcceleratorTable, wxMDIParentFrame, wxPreviewControlBar, wxStaticLine, wxSpinButton, wxGenericDirCtrl, wxToolbook, wxFont, wxDatePickerCtrl, wxSystemSettings, wxMDIClientWindow, wxHtmlLinkEvent, wxTreeEvent, wxSplashScreen, wxDropFilesEvent, wxSpinEvent, wxSingleChoiceDialog, wxAuiTabArt, wxIconBundle, glu, gl,
    %% Handcrafted modules
    wx,
    wx_object,
    wxe_master,
    wxe_server,
    wxe_util
   ]},
  {registered, []},
  {applications, [stdlib, kernel]},
  {env, []},
  {runtime_dependencies, ["stdlib-2.0","kernel-3.0","erts-6.0"]}
 ]}.
