'The build number is 9.00.2980
'*****************************************************************************
'
' Microsoft Windows Media
' Copyright (C) Microsoft Corporation. All rights reserved.
'
' FileName:            WMCMD.VBS
'
' Abstract:            Windows Media Command Line Script Utility
'                      Use Cscript.exe wmcmd.vbs /? for Help
'
'*****************************************************************************

Option Explicit

' Check to see if script is run within cscript host.
if instr( LCase(WScript.Fullname),"cscript.exe" ) = 0 then
  WshShell.Popup "This script must be run using cscript.exe from a command window." , 0 , "Windows Media Encoder Command Script" , 64
  WScript.Quit()
end if

' These variables are part of encoder idl. However vbs cannot use them directly. So these are redefined
dim WMENC_ENCODER_STARTING
dim WMENC_ENCODER_RUNNING
dim WMENC_ENCODER_PAUSED
dim WMENC_ENCODER_STOPPING
dim WMENC_ENCODER_STOPPED
dim WMENC_ENCODER_END_PREPROCESS

dim WMENC_ARCHIVE_LOCAL
dim WMENC_ARCHIVE_RUNNING
dim WMENC_ARCHIVE_PAUSED
dim WMENC_ARCHIVE_STOPPED

dim WMENC_AUDIO
dim WMENC_VIDEO
dim WMENC_SCRIPT
dim WMENC_FILETRANSFER

dim WMENC_SOURCE_START
dim WMENC_SOURCE_STOP
dim WMENC_SOURCE_PREPARE
dim WMENC_SOURCE_UNPREPARE
dim WMENC_START_FILETRANSFER
dim WMENC_STOP_FILETRANSFER

dim WMENC_PROTOCOL_HTTP
dim WMENC_PROTOCOL_PUSH_DISTRIBUTION

dim WMENC_PVM_NONE
dim WMENC_PVM_PEAK
dim WMENC_PVM_UNCONSTRAINED
dim WMENC_PVM_BITRATE_BASED

dim WMA9STD_FOURCC
dim WMA9PRO_FOURCC
dim WMA9LSL_FOURCC
dim WMSPEECH_FOURCC
dim PCM_FOURCC

dim WMV7_FOURCC
dim WMV8_FOURCC
dim WMV9_FOURCC
dim WMS9_FOURCC
dim MP41_FOURCC
dim UNCOMP_FOURCC

dim WMA9STD
dim WMA9PRO
dim WMA9LSL
dim WMSPEECH
dim PCM

dim WMV7
dim WMV8
dim WMV9
dim WMS9
dim MP41
dim UNCOMP

dim WMENC_CONTENT_ONE_AUDIO
dim WMENC_CONTENT_ONE_VIDEO
dim WMENC_CONTENT_ONE_AUDIO_ONE_VIDEO

dim WMENC_VIDEO_STANDARD
dim WMENC_VIDEO_DEINTERLACE
dim WMENC_VIDEO_INVERSETELECINE
dim WMENC_VIDEO_PROCESS_INTERLACED
dim WMENC_VIDEO_TELECINE_AUTO
dim WMENC_VIDEO_TELECINE_AA_TOP
dim WMENC_VIDEO_TELECINE_BB_TOP
dim WMENC_VIDEO_TELECINE_BC_TOP
dim WMENC_VIDEO_TELECINE_CD_TOP
dim WMENC_VIDEO_TELECINE_DD_TOP
dim WMENC_VIDEO_TELECINE_AA_BOTTOM
dim WMENC_VIDEO_TELECINE_BB_BOTTOM
dim WMENC_VIDEO_TELECINE_BC_BOTTOM
dim WMENC_VIDEO_TELECINE_CD_BOTTOM
dim WMENC_VIDEO_TELECINE_DD_BOTTOM
dim WMENC_VIDEO_INTERLACED_AUTO
dim WMENC_VIDEO_INTERLACED_TOP_FIRST
dim WMENC_VIDEO_INTERLACED_BOTTOM_FIRST
dim WMENC_PIXELFORMAT_IYUV
dim WMENC_PIXELFORMAT_I420
dim WMENC_PIXELFORMAT_YV12
dim WMENC_PIXELFORMAT_YUY2
dim WMENC_PIXELFORMAT_UYVY
dim WMENC_PIXELFORMAT_YVYU
dim WMENC_PIXELFORMAT_YVU9
dim WMENC_PIXELFORMAT_RGB24
dim WMENC_PIXELFORMAT_RGB32
dim WMENC_PIXELFORMAT_RGB555
dim WMENC_PIXELFORMAT_RGB565
dim WMENC_PIXELFORMAT_RGB8

WMENC_ENCODER_STARTING = 1
WMENC_ENCODER_RUNNING = 2
WMENC_ENCODER_PAUSED = 3
WMENC_ENCODER_STOPPING = 4
WMENC_ENCODER_STOPPED = 5
WMENC_ENCODER_END_PREPROCESS = 6

WMENC_ARCHIVE_LOCAL = 1

WMENC_ARCHIVE_RUNNING = 1
WMENC_ARCHIVE_PAUSED = 2
WMENC_ARCHIVE_STOPPED = 3

WMENC_AUDIO = 1
WMENC_VIDEO = 2
WMENC_SCRIPT = 4
WMENC_FILETRANSFER = 8

WMENC_SOURCE_START = 1
WMENC_SOURCE_STOP = 2
WMENC_SOURCE_PREPARE = 3
WMENC_SOURCE_UNPREPARE = 4
WMENC_START_FILETRANSFER = 5
WMENC_STOP_FILETRANSFER = 6

WMENC_PROTOCOL_HTTP = 1
WMENC_PROTOCOL_PUSH_DISTRIBUTION = 2

WMENC_PVM_NONE = 1
WMENC_PVM_PEAK = 2
WMENC_PVM_UNCONSTRAINED = 3
WMENC_PVM_BITRATE_BASED = 4

WMA9STD_FOURCC = 353
WMA9PRO_FOURCC = 354
WMA9LSL_FOURCC = 355
WMSPEECH_FOURCC = 10
PCM_FOURCC = 0

WMV7_FOURCC = 827739479
WMV8_FOURCC = 844516695
WMV9_FOURCC = 861293911
WMS9_FOURCC = 844321613
MP41_FOURCC = 1395937357
UNCOMP_FOURCC = 0

WMA9STD = "WMA9STD"
WMA9PRO = "WMA9PRO"
WMA9LSL = "WMA9LSL"
WMSPEECH = "WMSP9"
PCM = "PCM"

WMV7 = "WMV7"
WMV8 = "WMV8"
WMV9 = "WMV9"
WMS9 = "WMS9"
MP41 = "MP41"
UNCOMP = "UNCOMP"

WMENC_CONTENT_ONE_AUDIO = 1
WMENC_CONTENT_ONE_VIDEO = 16
WMENC_CONTENT_ONE_AUDIO_ONE_VIDEO = 17

WMENC_VIDEO_STANDARD = 1
WMENC_VIDEO_DEINTERLACE = 2
WMENC_VIDEO_INVERSETELECINE = 3
WMENC_VIDEO_PROCESS_INTERLACED = 4
WMENC_VIDEO_TELECINE_AA_TOP = &H23
WMENC_VIDEO_TELECINE_BB_TOP = &H33
WMENC_VIDEO_TELECINE_BC_TOP = &H43
WMENC_VIDEO_TELECINE_CD_TOP = &H53
WMENC_VIDEO_TELECINE_DD_TOP = &H63
WMENC_VIDEO_TELECINE_AA_BOTTOM = &H73
WMENC_VIDEO_TELECINE_BB_BOTTOM = &H83
WMENC_VIDEO_TELECINE_BC_BOTTOM = &H93
WMENC_VIDEO_TELECINE_CD_BOTTOM = &HA3
WMENC_VIDEO_TELECINE_DD_BOTTOM = &HB3
WMENC_VIDEO_INTERLACED_AUTO = 4
WMENC_VIDEO_INTERLACED_TOP_FIRST = &H2004
WMENC_VIDEO_INTERLACED_BOTTOM_FIRST = &H3004

WMENC_PIXELFORMAT_IYUV = &H56555949
WMENC_PIXELFORMAT_I420 = &H30323449
WMENC_PIXELFORMAT_YV12 = &H32315659
WMENC_PIXELFORMAT_YUY2 = &H32595559
WMENC_PIXELFORMAT_UYVY = &H59565955
WMENC_PIXELFORMAT_YVYU = &H55595659
WMENC_PIXELFORMAT_YVU9 = &H39555659
WMENC_PIXELFORMAT_RGB24 = &HE436EB7D
WMENC_PIXELFORMAT_RGB32 = &HE436EB7E
WMENC_PIXELFORMAT_RGB555 = &HE436EB7C
WMENC_PIXELFORMAT_RGB565 = &HE436EB7B
WMENC_PIXELFORMAT_RGB8 = &HE436EB7A

dim g_strInput
dim g_strOutput
dim g_strOutputString
dim g_intSessionType
dim g_strProfile
dim g_strTitle
dim g_strAuthor
dim g_strCopyright
dim g_strDescription
dim g_strRating
dim g_strWMEFile
dim g_strProfileSave
dim g_strProfileLoad

dim g_intAudioDevice
dim g_intVideoDevice
dim g_intProfile
dim g_intBroadcast
dim g_strPushServer
dim g_strPublishingPoint
dim g_strPushTemplate
dim g_intMarkInTime
dim g_intMarkOutTime
dim g_intDuration
dim g_intVerbose

dim g_objEncoder
dim g_objSourceGroup
dim g_objAudioSource
dim g_objVideoSource
dim g_objProfile
dim g_objFileSystem

dim g_blnEncoderStarted
dim g_blnEndPreProcess
dim g_blnEncoderStopped
dim g_intErrorCode
dim g_intRunningSource
dim g_blnDevice
dim g_blnAudioOnly
dim g_blnVideoOnly
dim g_blnSilent

dim g_intAudioVBRMode
dim g_strAudioCodec
dim g_strAudioSetting
dim g_intAudioPeakBitrate
dim g_intAudioPeakBuffer
dim g_intAudioSpeechContent
dim g_strAudioSpeechEdl
dim g_intAudioSurroundMix
dim g_intAudioCenterMix
dim g_intAudioLEFMix
dim g_strAudioCodecName

dim g_intVideoVBRMode
dim g_strVideoCodec
dim g_intVideoBitrate
dim g_intVideoWidth
dim g_intVideoHeight
dim g_intVideoFramerate
dim g_intVideoKeydist
dim g_intVideoBuffer
dim g_intVideoQuality
dim g_intVideoPeakBitrate
dim g_intVideoPeakBuffer
dim g_intVideoPreprocess
dim g_strPixelFormat
dim g_intPixelAspectRatioX
dim g_intPixelAspectRatioY
dim g_intVideoPerformance
dim g_strVideoCodecName
dim g_strVideoDevConf
dim g_intMaxPacketSize
dim g_intMinPacketSize

dim i
dim j
dim g_blnCreateCustomProfile

dim g_tStartTime
dim g_tStopTime

dim g_intClipLeft
dim g_intClipRight
dim g_intClipTop
dim g_intClipBottom

g_strInput = ""
g_strOutput = ""
g_strOutputString = ""
g_intSessionType = WMENC_CONTENT_ONE_AUDIO
g_strProfile = ""
g_strTitle = ""
g_strAuthor = ""
g_strCopyright = ""
g_strDescription = ""
g_strRating = ""
g_strWMEFile = ""
g_strProfileSave = ""
g_strProfileLoad = ""
g_strProfile = ""

g_intAudioDevice = -1
g_intVideoDevice = -1
g_intProfile = -1
g_intBroadcast = -1
g_strPushServer = ""
g_strPublishingPoint = ""
g_strPushTemplate = ""
g_intMarkInTime = -1
g_intMarkOutTime = -1
g_intDuration = -1
g_intVerbose = 2

g_blnEncoderStarted = false
g_blnEncoderStopped = false
g_blnEndPreProcess = false
g_blnDevice = false
g_intErrorCode = 0
g_intRunningSource = 0
g_blnAudioOnly = false
g_blnVideoOnly = false

g_intAudioVBRMode = -1
g_strAudioCodec = ""
g_strAudioSetting = ""
g_intAudioPeakBitrate = -1
g_intAudioPeakBuffer = -1
g_intAudioSpeechContent = -1
g_strAudioSpeechEdl = ""
g_intAudioSurroundMix = -1
g_intAudioCenterMix = -1
g_intAudioLEFMix = -1
g_strAudioCodecName = ""

g_intVideoVBRMode = -1
g_strVideoCodec = ""
g_intVideoBitrate = -1
g_intVideoWidth = -1
g_intVideoHeight = -1
g_intVideoFramerate = -1
g_intVideoKeydist = -1
g_intVideoBuffer = -1
g_intVideoQuality = -1
g_intVideoPeakBitrate = -1
g_intVideoPeakBuffer = -1
g_intVideoPreprocess = -1
g_strPixelFormat = ""
g_intPixelAspectRatioX = -1
g_intPixelAspectRatioY = -1
g_intVideoPerformance = -1
g_strVideoCodecName = ""
g_strVideoDevConf = ""
g_intMaxPacketSize = -1
g_intMinPacketSize = -1

g_intClipLeft = -1
g_intClipRight = -1
g_intClipTop = -1
g_intClipBottom = -1

g_blnCreateCustomProfile = true

' Create an encoder object.
set g_objEncoder = WScript.CreateObject( "WMEncEng.WMEncoder" )
WScript.ConnectObject g_objEncoder, "Encoder_"    ' Setup call back events
  
set g_objFileSystem = WScript.CreateObject( "Scripting.FileSystemObject" )
g_objSourceGroup = Null
g_objAudioSource = Null
g_objVideoSource = Null
g_objProfile = NULL

' Read a integer value from the paramters list.
' args is the paramters list to be read.
' intIndex1 is the position of the parameter name.
' intIndex2 is the position of the parameter value.
' It will return the integer if it can read a integer value from the parameter list
' It will return -1 if it can not get a integer value from the paramters list.
function ReadInteger( args, intIndex1, intIndex2 )
  on error resume next
  dim strTypeName

  ' Check the range of index.
  if intIndex2 > UBound(args) or Left( args(intIndex2), 1 ) = "-" then
    OutputInfo "Invalid format for parameter " & args(intIndex1)
    ReadInteger = -1
    exit function
  end if

  strTypeName = typename( eval( args(intIndex2) ) )

  ' Convert it to an integer value.
  ReadInteger = CLng( args(intIndex2) )

  ' The value is not in integer fommat.
  if err.number <> 0 or ( strTypeName <> "Integer" and strTypeName <> "Long" )  then
    OutputInfo "Expect integer value for parameter " & args(intIndex1)
    ReadInteger = -1
    err.Clear
  end if

end function

' Read a float value from the paramters list.
' args is the paramters list to be read.
' intIndex1 is the position of the parameter name.
' intIndex2 is the position of the parameter value.
' It will return the float if it can read a float value from the parameter list
' It will return -1 if it can not get a float value from the paramters list.
function ReadFloat( args, intIndex1, intIndex2 )
  on error resume next
  dim strTypeName

  ' Check the range of index.
  if intIndex2 > UBound(args) or Left( args(intIndex2), 1 ) = "-" then
    OutputInfo "Invalid format for parameter " & args(intIndex1)
    ReadFloat = -1
    exit function
  end if

  strTypeName = typename( eval( args(intIndex2) ) )

  ' Convert it to an integer value.
  ReadFloat = CSng( args(intIndex2) )

  ' The value is not in integer fommat.
  if err.number <> 0 or ( strTypeName <> "Integer" and strTypeName <> "Long" and strTypeName <> "Single" and strTypeName <> "Double" )  then
    OutputInfo "Expect float value for parameter " & args(intIndex1)
    ReadFloat = -1
    err.Clear
  end if

end function

' Read a var value from the paramters list.
' args is the paramters list to be read.
' intIndex1 is the position of the parameter name.
' intIndex2 is the position of the parameter value.
' It will return the var if it can not get a var value from the paramters list.
' It will return "" if it can not get a var value from the paramters list.
function ReadString( args, intIndex1, intIndex2 )
  ' Check the range of index.
  if intIndex2 > UBound(args) or Left( args(intIndex2), 1 ) = "-" then
    OutputInfo "Invalid format for parameter " & args(intIndex1)
    ReadString = ""
    exit function
  end if

  ReadString = args(intIndex2)
end function

function ConvertStringToInteger( strInput )
  on error resume next
  dim strTypeName

  strTypeName = typename( eval( strInput ) )

  ' Convert it to an integer value.
  ConvertStringToInteger = CLng( strInput )

  ' The value is not in integer fommat.
  if err.number <> 0 or ( strTypeName <> "Integer" and strTypeName <> "Long" )  then
    OutputInfo "Invalid format " & strInput & " expecting integer value"
    ConvertStringToInteger = -1
    err.Clear
  end if

end function

' Parse parameters.

if wscript.arguments.Length = 0 then
  ShowHelp()
  wscript.quit()
end if

dim colStrArgs()
reDim colStrArgs( wscript.arguments.Length-1 )
for i = 0 to wscript.arguments.Length-1
  colStrArgs(i) = wscript.arguments(i)
next

if ParseParameters( colStrArgs ) then
    ' Do different things depending on whether input is wme, directory encoding or file encoding
    if g_strInput <> "" and g_objFileSystem.FolderExists(g_strInput) then
    DoDirectoryModeEncoding()
  elseif SetupEncoder() then
    ' Transcode and show statistics information
    if Transcode() and not g_blnSilent then 
      ShowStatistics()
    end if 
  end if
end if

WScript.Quit()

' Output information
function OutputInfo( strInfo )
  WScript.Echo( strInfo )
end function

' Event handler for encoder state
function Encoder_OnStateChange( enumState )
  dim strState
  ' Translate encoder state
  select case  enumState
    case WMENC_ENCODER_STARTING
      strState = "Starting"

    case WMENC_ENCODER_RUNNING
      strState = "Running"
      g_blnEncoderStarted = true

    case WMENC_ENCODER_PAUSED
      strState = "Paused"

    case WMENC_ENCODER_STOPPING
      strState = "Stopping"

    case WMENC_ENCODER_STOPPED
      g_tStopTime = Now()
      strState = "Stopped"
      g_blnEncoderStopped = true

    case WMENC_ENCODER_END_PREPROCESS
      strState = "End Preprorocess"
      g_blnEndPreProcess = true

    case else
      strState = "Unknown State"
  end select

  Encoder_OnStateChange = 1
end function

' Event handler for encoder error
function Encoder_OnError( intResult )
  g_intErrorCode = intResult

  Encoder_OnError = 0
end function

' Parse the parameters of this program.
function ParseParameters( args )
  dim strMarkOutTime
  dim strArguments
  dim strSaveToCfgFile
  dim strLoadFromCfgFile
  dim strCfgFileArguments
  dim colStrConfigArgs
  dim objTextFile

  for i=0 to UBound(args)
    if args(i) = "-s_config" then
      i = i + 1
    else
      strArguments = strArguments & args(i) & " "
    end if

    if args(i) = "-config" then
      strLoadFromCfgFile = ReadString( args, i, i+1 )
      if strLoadFromCfgFile = "" then
        ParseParameters = false
        exit function
      end if
    
      i = i + 1

      strLoadFromCfgFile = g_objFileSystem.GetAbsolutePathName( strLoadFromCfgFile )
      set objTextFile = g_objFileSystem.OpenTextFile( strLoadFromCfgFile, 1 )
      strCfgFileArguments = objTextFile.ReadLine
      objTextFile.Close
      
      strCfgFileArguments = Trim( strCfgFileArguments )
      if strCfgFileArguments = "" then
        OutputInfo "Configuration file " & strLoadFromCfgFile & " is empty"
        ParseParameters = false
        exit function
      end if
      colStrConfigArgs = Split( strCfgFileArguments )
      if ParseParameters( colStrConfigArgs ) = false then
        ParseParameters = false
        exit function
      end if
    end if
  next

  ' OutputInfo "Args = " & strArguments

  if UBound(args) = 0 then
    ' If it has only has one parameter, check the content of this parameter.
    select case LCase( args(0) )  
      case "-devices":
        ' List all audio and video devices.
        ListDevices()
        ParseParameters = false
        exit function

      case "-a_codecs":
        ' List all audio codecs.
        ListAudioCodecs()
        ParseParameters = false
        exit function

      case "-a_formats"
        ' List audio formats
        ListAudioFormats()
        ParseParameters = false
        exit function
      
      case "-v_codecs":
        ' List all video codecs.
        ListVideoCodecs()
        ParseParameters = false
        exit function
      
      case "-help":
        ' Show help information.
        ShowHelp()
        ParseParameters = false
        exit function

      case "-?":
        ' Show help information.
        ShowHelp()
        ParseParameters = false
        exit function

      case "-all?":
        ' Show help information.
        ShowHelp()
        ParseParameters = false
        exit function
      
      case "/?":
        ' Show help information.
        ShowHelp()
        ParseParameters = false
        exit function
      
      case "-help?"
        ' Being up the encoder util chm
        ShowHelpChm()
        ParseParameters = false
        exit function

    end select
  end if

  if UBound(args) = 1 then
    select case LCase( args(0) ) 
      case "-input"
      ' Print out info about file specified
      g_strInput = ReadString( args, 0, 1 )
      if g_strInput = ""  then
        ParseParameters = false
        exit function
      end if

      PrintFileInfo()
      ParseParameters = false
      exit function
    end select
  end if

  ' Check all parameters.
  for i=0 to UBound(args)
  
    select case LCase( args(i) )
        ' Parse all input related stuff
      case "-wme"
        ' Get the WME file name to be load.
        g_strWMEFile = ReadString( args, i, i+1 )
        if g_strWMEFile = "" then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-input"           
        ' Get input source file name.
        g_strInput = ReadString( args, i, i+1 )
        if g_strInput = ""  then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-audioonly"
        g_blnAudioOnly = true

      case "-videoonly"
        g_blnVideoOnly = true

      case "-adevice"
        g_blnDevice = true
        ' Get audio device index.
        if i+1 <= UBound(args) and Left( args(i+1), 1 ) <> "-" then
          g_intAudioDevice = ReadInteger( args, i, i+1 )
          if g_intAudioDevice = -1 then
            ParseParameters = false
            exit function
          end if
          
          i = i + 1
        else
          g_intAudioDevice = 0   ' default audio device
        end if

      case "-vdevice"
        g_blnDevice = true
        ' Get video device index.
        if i+1 <= UBound(args) and Left( args(i+1), 1 ) <> "-" then
          g_intVideoDevice = ReadInteger( args, i, i+1 )
          if g_intVideoDevice = -1 then
            ParseParameters = false
            exit function
          end if
        
          i = i + 1
        else
          g_intVideoDevice = 0     ' default video device
        end if

      ' Parse all profile related stuff
      case "-profile"
        ' Get input source file name.
        g_strProfile = ReadString( args, i, i+1 )
        if g_strProfile = ""  then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      ' Parse audio related stuff
      case "-a_mode"
        g_intAudioVBRMode = ReadInteger( args, i, i+1 )
        if g_intAudioVBRMode = -1 then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-a_codec"
        'Get audio codec name
        g_strAudioCodec = ReadString( args, i, i+1 )
        g_strAudioCodec = UCase( g_strAudioCodec )

        if g_strAudioCodec <> WMA9STD and g_strAudioCodec <> WMA9PRO and g_strAudioCodec <> WMA9LSL and g_strAudioCodec <> WMSPEECH and g_strAudioCodec <> PCM then
          OutputInfo "Please enter correct audio codec index only"
          ParseParameters = false
          exit function
        end if

        if g_strAudioCodec = ""  then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-a_setting"
        'Get audio setting
        g_strAudioSetting = ReadString( args, i, i+1 )
        if g_strAudioSetting = ""  then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-a_peakbitrate"
        'Get audio peakbitrate size
        g_intAudioPeakBitrate = ReadInteger( args, i, i+1 )
        if g_intAudioPeakBitrate = -1  then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-a_peakbuffer"
        'Get audio peakbuffer size
        g_intAudioPeakBuffer = ReadInteger( args, i, i+1 )
        if g_intAudioPeakBuffer = -1  then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-a_content"
        'Get content type for speech
        g_intAudioSpeechContent = ReadInteger( args, i, i+1 )
        if g_intAudioSpeechContent = -1 then
          ParseParameters = false
          exit function
        elseif g_intAudioSpeechContent < 0 or g_intAudioSpeechContent > 2 then
          OutputInfo "Invalid range for parameter " & args(i)
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-a_contentedl"
        'Get EDL file for speech content
        g_strAudioSpeechEdl = ReadString( args, i, i+1 )
        if g_strAudioSpeechEdl = ""  then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-a_folddown6to2"
        'Get EDL file for speech content
        g_intAudioSurroundMix = ReadInteger( args, i, i+1 )
        if g_intAudioSurroundMix = -1  then
          ParseParameters = false
          exit function
        end if
        g_intAudioCenterMix = ReadInteger( args, i, i+2 )
        if g_intAudioCenterMix = -1  then
          ParseParameters = false
          exit function
        end if  
        g_intAudioLEFMix = ReadInteger( args, i, i+3 )
        if g_intAudioLEFMix = -1  then
          ParseParameters = false
          exit function
        end if
    
        i = i + 3

      ' Parse video related stuff
      case "-v_mode"
        g_intVideoVBRMode = ReadInteger( args, i, i+1 )
        if g_intVideoVBRMode = -1 then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-v_codec"
        'Get video codec name
        g_strVideoCodec = ReadString( args, i, i+1 )
        g_strVideoCodec = UCase( g_strVideoCodec )
        if g_strVideoCodec <> WMV7 and g_strVideoCodec <> WMV8 and g_strVideoCodec <> WMV9 and g_strVideoCodec <> WMS9 and g_strVideoCodec <> MP41 and g_strVideoCodec <> UNCOMP then
          OutputInfo "Please enter correct video codec index only"
          ParseParameters = false
          exit function
        end if

        if g_strVideoCodec = ""  then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-v_bitrate"
        'Get video bitrate
        g_intVideoBitrate = ReadInteger( args, i, i+1 )
        if g_intVideoBitrate = -1  then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-v_buffer"
        'Get video buffer
        g_intVideoBuffer = ReadInteger( args, i, i+1 )
        if g_intVideoBuffer = -1  then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-v_width"
        'Get video width
        g_intVideoWidth = ReadInteger( args, i, i+1 )
        if g_intVideoWidth = -1  then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-v_height"
        'Get video height
        g_intVideoHeight = ReadInteger( args, i, i+1 )
        if g_intVideoHeight = -1  then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-v_framerate"
        'Get video framerate
        g_intVideoFramerate = ReadFloat( args, i, i+1 )
        if g_intVideoFramerate = -1  then
          ParseParameters = false
          exit function
        end if
        g_intVideoFramerate = 1000 * g_intVideoFramerate
    
        i = i + 1

      case "-v_keydist"
        'Get video keyframe distance
        g_intVideoKeydist = ReadInteger( args, i, i+1 )
        if g_intVideoKeydist = -1  then
          ParseParameters = false
          exit function
        end if

        g_intVideoKeydist = 1000 * g_intVideoKeydist
    
        i = i + 1

      case "-v_quality"
        'Get video bufferwindow size
        g_intVideoQuality = ReadInteger( args, i, i+1 )
        if g_intVideoQuality = -1  then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-v_peakbitrate"
        'Get video peakbitrate size
        g_intVideoPeakBitrate = ReadInteger( args, i, i+1 )
        if g_intVideoPeakBitrate = -1  then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-v_peakbuffer"
        'Get video peakbuffer size
        g_intVideoPeakBuffer = ReadInteger( args, i, i+1 )
        if g_intVideoPeakBuffer = -1  then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-v_performance"
        'Get video complexity setting
        g_intVideoPerformance = ReadInteger( args, i, i+1 )
        if g_intVideoPerformance <> 0 and g_intVideoPerformance <> 20 and g_intVideoPerformance <> 40 and g_intVideoPerformance <> 60 and g_intVideoPerformance <> 80 and g_intVideoPerformance <> 100 then
          ParseParameters = false
          exit function
        end if

        i = i + 1

      case "-v_preproc"
        'Get video preprocess setting
        g_intVideoPreprocess = ReadInteger( args, i, i+1 )
        if g_intVideoPreprocess = -1  or g_intVideoPreprocess > 18 then
          OutputInfo "Invalid preproc value: " & args(i+1)
          ParseParameters = false
          exit function
        end if
  
        i = i + 1

      case "-pixelformat"
        ' Get pixelformat.
        g_strPixelFormat = ReadString( args, i, i+1 )
        if g_strPixelFormat = "" then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-pixelratio"
        ' Get pixelformat.
        g_intPixelAspectRatioX = ReadInteger( args, i, i+1 )
        g_intPixelAspectRatioY = ReadInteger( args, i, i+2 )

        if g_intPixelAspectRatioX = -1 or g_intPixelAspectRatioY = -1 then
          ParseParameters = false
          exit function
        end if
    
        i = i + 2

      case "-v_clip"
        ' Get clipping values.
        g_intClipLeft   = ReadInteger( args, i, i+1 )
        g_intClipTop    = ReadInteger( args, i, i+2 )
        g_intClipRight  = ReadInteger( args, i, i+3 )
        g_intClipBottom = ReadInteger( args, i, i+4 )

        if g_intClipLeft = -1 or g_intClipTop = -1 or g_intClipRight = -1 or g_intClipBottom = -1 then
          ParseParameters = false
          exit function
        end if

        i = i + 4

      case "-v_profile"
        ' Get video device conformance value
        g_strVideoDevConf = ReadString( args, i, i+1 )
        g_strVideoDevConf = UCase( g_strVideoDevConf )

        if g_strVideoDevConf = ""  then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

            ' Parse all output related stuff
      case "-output"            
        ' Get output file name.
        g_strOutput = ReadString( args, i, i+1 )
        if g_strOutput = "" then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-outputstring"
        ' Read in the string that is to be appended to output file name
        g_strOutputString = ReadString( args, i, i+1 )
        if g_strOutputString = "" then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-broadcast"
        ' Get broadcast port number.
        if i+1 <= UBound(args) and Left( args(i+1), 1 ) <> "-" then
          g_intBroadcast = ReadInteger( args, i, i+1 )
          if g_intBroadcast = -1 then
            ParseParameters = false
            exit function
          end if
    
          i = i + 1
        else
          g_intBroadcast = 8080
        end if

      case "-push"
        ' Get push server, pub point and template name (optional)
        g_strPushServer = ReadString( args, i, i+1 )
        g_strPublishingPoint = ReadString( args, i, i+2 )

        if g_strPushServer = "" or g_strPublishingPoint = "" then
          ParseParameters = false
          exit function
        end if

        ' Check if template name is specified
        if i+1 <= UBound(args) and Left( args(i+3), 1 ) <> "-" then
          g_strPushTemplate = ReadString( args, i, i+3 )
          i = i + 3
        else
            i = i + 2
        end if

      case "-time"
        ' Get the mark in time of the source file.
        g_intMarkInTime = ReadInteger( args, i, i+1 )

        ' Get the markout time of the source file
        strMarkOutTime = ReadString( args, i, i+2 )
        if strMarkOutTime <> "end" then
          g_intMarkOutTime = ConvertStringToInteger( strMarkOutTime )
        else
          g_intMarkOutTime = 0
        end if

        if g_intMarkInTime = -1 or g_intMarkOutTime = -1 then
          ParseParameters = false
          exit function
        end if
    
    
        i = i + 2

      case "-duration"
        ' Get the duration of the encoding session.
        g_intDuration = ReadInteger( args, i, i+1 )
        if g_intDuration = -1 then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-silent"
        g_blnSilent = true

      ' Parse all attributes
      case "-title"
        ' Get the title of the output file.
        g_strTitle = ReadString( args, i, i+1 )
        if g_strTitle = "" then
          ParseParameters = false
          exit function
        end if

        i = i + 1

      case "-author"            
        ' Get the author of the output file.
        g_strAuthor = ReadString( args, i, i+1 )
        if g_strAuthor = "" then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-copyright"           
        ' Get the copyright information of the output file.
        g_strCopyright = ReadString( args, i, i+1 )
        if g_strCopyright = "" then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-description"
        ' Get the descrption of the output file.
        g_strDescription = ReadString( args, i, i+1 )
        if g_strDescription = "" then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-rating"            
        ' Get the rating of the output file.
        g_strRating = ReadString( args, i, i+1 )
        if g_strRating = "" then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-saveprofile"             
        ' Get the profile file name to be saved.
        g_strProfileSave = ReadString( args, i, i+1 )
        if g_strProfileSave = "" then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1
    
      case "-loadprofile"             
        ' Get the profile file name to be loaded.
        g_strProfileLoad = ReadString( args, i, i+1 )
        if g_strProfileLoad = "" then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-verbose"
        ' Get the verbose mode.
        g_intVerbose = ReadInteger( args, i, i+1 )
        if g_intVerbose = -1 then
          ParseParameters = false
          exit function
        end if

        i = i + 1

      case "-s_config"
        ' Save the WEU configuration file
        strSaveToCfgFile = ReadString( args, i, i+1 )
        if strSaveToCfgFile = "" then
          ParseParameters = false
          exit function
        end if

        i = i + 1
      case "-config"
        if strLoadFromCfgFile = "" then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case "-maxpacket"
        g_intMaxPacketSize = ReadInteger( args, i, i+1 )
        if g_intMaxPacketSize = -1 then
          ParseParameters = false
          exit function
        end if

        i = i + 1

      case "-minpacket"
        g_intMinPacketSize = ReadInteger( args, i, i+1 )
        if g_intMinPacketSize = -1 then
          ParseParameters = false
          exit function
        end if
    
        i = i + 1

      case else
        ' Show an error information for unrecognized parameter.
        OutputInfo "Invalid parameter: " & args(i)
        ParseParameters = false
        exit function
    end select
  
  next

  if strSaveToCfgFile <> "" then
    strSaveToCfgFile = g_objFileSystem.GetAbsolutePathName( strSaveToCfgFile )

    ' Add extension if necessary
    if Right( LCase( strSaveToCfgFile ), 4 ) <>  ".weu" then
      strSaveToCfgFile = strSaveToCfgFile & ".weu"
    end if

    CreateOutputFolder( g_objFileSystem.GetParentFolderName( strSaveToCfgFile ) ) 
    set objTextFile = g_objFileSystem.OpenTextFile( strSaveToCfgFile, 2, True )
    objTextFile.Write( strArguments )
    objTextFile.close
  end if

  ParseParameters = true
end function

' Show help information of this program.
function ShowHelp()
  OutputInfo "Encode from files or devices to Windows Media files or streams. Supported"
  OutputInfo "source file formats are .wmv, .wma, .asf, .avi., .wav. .mpg, .mp3, .bmp,"
  OutputInfo "and .jpg."
  OutputInfo ""
  OutputInfo ""
  OutputInfo "Usage for I/O and statistics."
  OutputInfo ""
  OutputInfo "[-wme] <Windows Media Encoder session file>"
  OutputInfo "    Loads an existing Windows Media Encoder session file."
  OutputInfo "[-input] <file or directory name>"
  OutputInfo "    The file or directory to be encoded."
  OutputInfo "    Specify a file or directory name. If you specify a directory, supported"
  OutputInfo "    files in the directory will be encoded to the output directory, using"
  OutputInfo "    the same encoding settings. "
  OutputInfo "    Enclose file and directory names that have spaces in quotations."
  OutputInfo "    For example: -input ""c:\my sample.wmv"""
  OutputInfo "[-adevice] <audio device number>"
  OutputInfo "    The audio capture device to encode from."
  OutputInfo "    Use -devices to list all available capture devices."  
  OutputInfo "    When encoding from devices, you must specify a duration using -duration."      
  OutputInfo "[-vdevice] <video device number>"
  OutputInfo "    The video capture device to encode from."
  OutputInfo "    Use -devices to list all available capture devices."  
  OutputInfo "    When capturing from devices, you must specify a duration using -duration."      
  OutputInfo "[-output] <file or directory name>"
  OutputInfo "    The name of the output file or directory."
  OutputInfo "    If the input is a file, -output corresponds to a file name. If the input"
  OutputInfo "    is a directory, -output corresponds to a directory name."
  OutputInfo "    The output directory will be created if it doesn't already exist."
  OutputInfo "    An extension is automatically appended to output files."
  OutputInfo "    (.wma for audio-only Windows Media files, and .wmv for video-only or"
  OutputInfo "    audio and video Windows Media files.)"
  OutputInfo "[-outputstring] <string>"
  OutputInfo "    The string to be attached to every output file name for directory mode."
  OutputInfo "[-broadcast] <port>"
  OutputInfo "    Broadcasts via HTTP on the port specified. The default port is 8080."
  OutputInfo "[-time] <start time> <end time>"
  OutputInfo "    Specify the time segment (in msec) to be encoded. Use 'end' for"
  OutputInfo "    <end time> if you want to encode to the end of the file."
  OutputInfo "[-silent]"
  OutputInfo "    Prevents statistics from being displayed after encoding is finished."
  OutputInfo "[-config] <input configuration file>"
  OutputInfo "    Inputs arguments from a configuration file. The default extension is .weu."
  OutputInfo "    Parameters in the file can be overridden by later arguments."
  OutputInfo "[-s_config] <output configuration file>"
  OutputInfo "    Creates a configuration file. The file name extension .weu is appended"
  OutputInfo "    automatically."
  OutputInfo ""
  OutputInfo ""
  OutputInfo "Usage for profiles."
  OutputInfo "    You can override parameters in a profile by appending arguments to a"
  OutputInfo "    command."
  OutputInfo ""
  OutputInfo "[-profile] <profile code>"
  OutputInfo "    Specifies a predefined profile to use in the session."
  OutputInfo ""
  OutputInfo "Codes and basic descriptions for the predefined profiles:"
  OutputInfo "Audio/Video:"
  OutputInfo "  av20: Profile_AudioVideo_Modem_28K (actual rate: 20Kbps)"
  OutputInfo "  av32: Profile_AudioVideo_Modem_56K (32 Kbps)"
  OutputInfo "  av100: Profile_AudioVideo_LAN_100K (100 Kbps)"
  OutputInfo "  av225: Profile_AudioVideo_LAN_256K (225 Kbps)"
  OutputInfo "  av350: Profile_AudioVideo_LANDSL_384K (350 Kbps)"
  OutputInfo "  av450: Profile_AudioVideo_LANDSL_768K (450 Kbps)"
  OutputInfo "  av700: Profile_AudioVideo_NearBroadcast_700K (700 Kbps)"
  OutputInfo "  av1400: Profile_AudioVideo_NearBroadcast_1400K (1400 Kbps)"
  OutputInfo "  av350pal: Profile_AudioVideo_Broadband_PAL_384K (350 Kbps)"
  OutputInfo "  av700pal: Profile_AudioVideo_NearBroadcast_PAL_700K (700 Kbps)"
  OutputInfo "  av100_2p: Profile_AudioVideo_LAN_100K_2Pass (100 Kbps)"
  OutputInfo "  av350_2p: Profile_AudioVideo_LANDSL_384K_2Pass (350 Kbps)"
  OutputInfo "  av600vbr: Profile_AudioVideo_FilmVBR_600K (600 Kbps)"
  OutputInfo "  avq97vbr: Profile_AudioVideo_FilmVBR_Quality97 (Quality 97)"
  OutputInfo ""
  OutputInfo "Audio-only:"
  OutputInfo "  a20_1: Profile_AudioOnly_FMRadioMono_28K (20 Kbps)"
  OutputInfo "  a20_2: Profile_AudioOnly_FMRadioStereo_28K (20 Kbps)"
  OutputInfo "  a32: Profile_AudioOnly_Modem_56K (32 Kbps)"
  OutputInfo "  a48: Profile_AudioOnly_NearCDQuality_48K (48 Kbps)"
  OutputInfo "  a64: Profile_AudioOnly_CDQuality_64K (64 Kbps)"
  OutputInfo "  a96: Profile_AudioOnly_CDAudiophileQuality_96K (96 Kbps)"
  OutputInfo "  a128: Profile_AudioOnly_CDAudiophileQuality_128K (128 Kbps)"
  OutputInfo ""
  OutputInfo "Video-only:"
  OutputInfo "  v20: Profile_VideoOnly_Modem_28K (20 Kbps)"
  OutputInfo "  v32: Profile_VideoOnly_Modem_56K (32 Kbps)"
  OutputInfo ""
  OutputInfo "[-loadprofile] <profile file name>"
  OutputInfo "    Specifies a Windows Media Encoder or custom profile to use."
  OutputInfo ""
  OutputInfo ""
  OutputInfo "Usage for audio settings."
  OutputInfo ""
  OutputInfo "[-a_codec] <codec index>"
  OutputInfo "    Audio codec to be used. Use -a_codecs to list available codecs."
  OutputInfo "    Specify codec index:"
  OutputInfo "    WMA9STD: Windows Media Audio 9 (default)."
  OutputInfo "    WMA9PRO: Windows Media Audio 9 Professional"
  OutputInfo "    WMSP9:   Windows Media Audio 9 Voice"
  OutputInfo "    WMA9LSL:  Windows Media Audio 9 Lossless; -a_mode 2 required"
  OutputInfo "    PCM: No compression"
  OutputInfo "[-a_codecs]"
  OutputInfo "    Lists all audio codecs."
  OutputInfo "[-a_content] <mode>"
  OutputInfo "    Audio content mode for the Windows Media Audio v9 Voice codec."
  OutputInfo "    0 = No special mode for the audio content (default)"
  OutputInfo "    1 = Speech mode"
  OutputInfo "    2 = Mixed (speech and music) mode (also requires -a_contentedl)"
  OutputInfo "    You must also specify the Windows Media Audio v9 Voice codec."
  OutputInfo "[-a_contentedl] <file name>"
  OutputInfo "    Specifies the places in audio content where music starts and ends. To do"
  OutputInfo "    this, you must first create an optimization definition file. You"
  OutputInfo "    must specify the Windows Media Audio Voice codec and -a_content 2 when"
  OutputInfo "    when you use the -a_contentedl option."
  OutputInfo "[-a_folddown6to2] <surround mix> <center mix> <LFE>"
  OutputInfo "    Fold-down coefficients for multichannel audio. Use whole numbers only."
  OutputInfo "    Values will be converted to negative numbers. Valid numbers are 0 to 144."
  OutputInfo "    For example, -a_folddown6to2 10 3 3"
  OutputInfo "[-a_formats]"
  OutputInfo "    Lists all audio formats for each codec."
  OutputInfo "[-a_mode] <mode_number>"
  OutputInfo "    Audio encoding to be used."
  OutputInfo "    0: 1-pass CBR (default)."
  OutputInfo "    1: 2-pass CBR."
  OutputInfo "    2: Quality-based VBR."
  OutputInfo "    3: Bit rate-based VBR (two-pass)."
  OutputInfo "    4: Bit rate-based peak VBR (two-pass)." 
  OutputInfo "[-a_peakbitrate] <peak bit rate>"
  OutputInfo "    Specifies the peak bit rate in bits per second for peak bit rate-based"
  OutputInfo "    VBR for audio. If not specified, the peak bit rate is 1.5 times the"
  OutputInfo "    audio bit rate."
  OutputInfo "[-a_peakbuffer] <peak buffer>"
  OutputInfo "    Buffer in msec for audio with peak bit rate-based VBR. If not specified,"
  OutputInfo "    the default of 3000 msec is used."
  OutputInfo "[-a_setting] <setting>"
  OutputInfo "    Specifies the formats for audio setting."
  OutputInfo "    Use -a_formats to list supported audio formats for each codec."
  OutputInfo ""
  OutputInfo "    -a_setting Bitrate_SamplingRate_Channels. For example, -a_setting 48_44_2"
  OutputInfo "    specifies 48 Kbps, 44 kHz, and two channels. The default is 64_44_2."
  OutputInfo ""
  OutputInfo "    If you use quality-based VBR: -a_setting Qxx_SamplingRate_Channels."
  OutputInfo "    For example, -a_setting Q90_44_2 specifies a quality level of 90, 44 kHz,"
  OutputInfo "    and 2 channels."
  OutputInfo ""
  OutputInfo "    If you use the Windows Media Audio Professional 9:" 
  OutputInfo "    -a_setting Bitrate_SamplingRate_Channels_BitDepth."
  OutputInfo "    For example, -a_setting 250_44_2_24 specifies 250 Kbps, 44 kHz, "
  OutputInfo "    two channels, and 24-bit encoding. Use either 16- or 24-bit encoding."
  OutputInfo "    24-bit is the default."
  OutputInfo ""
  OutputInfo "    If you use the Windows Media Audio 9 Lossless codec:"
  OutputInfo "    -a_setting Qxx_SamplingRate_Channels_BitDepth. For example,"
  OutputInfo "    -a_setting Q100_44_2_24 specifies VBR Quality 100, 44 kHz, two channel,"
  OutputInfo "    24-bit encoding. (Quality-based VBR and a quality level of 100 is"
  OutputInfo "    required with this codec.)"
  OutputInfo "[-audioonly]"
  OutputInfo "    Encodes the audio portion of the stream only."
  OutputInfo ""
  OutputInfo ""
  OutputInfo ""
  OutputInfo ""
  OutputInfo "Usage for video settings."
  OutputInfo ""
  OutputInfo "[-v_codec] <codec index>"
  OutputInfo "    Video codec to be used. Use -v_codecs to list available video codecs."
  OutputInfo "    Specify codec index:"
  OutputInfo "       WMV7: Windows Media Video 7."
  OutputInfo "       WMV8: Windows Media Video 8.1."
  OutputInfo "       WMV9: Windows Media Video 9 (default)."
  OutputInfo "       WMS9: Windows Media Video Screen 9."
  OutputInfo "       MP41: ISO MPEG-4 Video v1"
  OutputInfo "       UNCOMP: Full frames (uncompressed)"
  OutputInfo "[-v_width] <width>"
  OutputInfo "    Video frame width in pixels to be encoded. Default: Source video width."
  OutputInfo "[-v_height] <height>"
  OutputInfo "    Video frame height in pixels to be encoded. Default: Source video height."
  OutputInfo "[-v_framerate] <rate>"
  OutputInfo "    Video frame rate in floating point to be encoded. Default: 30 fps."
  OutputInfo "[-v_keydist] <time>"
  OutputInfo "    Key frame distance in seconds for video. Default: 10 seconds."
  OutputInfo "[-v_preproc] <filter number>"
  OutputInfo "    Video preprocessing for certain input sources."
  OutputInfo "    0: None (for progressive video input)."
  OutputInfo "    1: Deinterlace but preserve the size."
  OutputInfo "       For example, 640x480x30 interlace to 640x480x30 progressive."
  OutputInfo "    2: Deinterlace but halve the size."
  OutputInfo "       For example, 640x480x30 interlace to 320x240x30 progressive."
  OutputInfo "    3: Deinterlace, halve the size, and double the frame rate."
  OutputInfo "       For example, 640x480x30 interlace to 320x240x60 progressive."
  OutputInfo "    4: Deinterlace, halve vertical size, and double frame rate."
  OutputInfo "       For example, 320x480x30 interlace to 320x240x60 progressive."
  OutputInfo "    5: Inverse telecine from 30fps to 24fps."
  OutputInfo "    6: Inverse telecine - First field is top field with AA telecine pattern."
  OutputInfo "    7: Inverse telecine - First field is top field with BB telecine pattern."
  OutputInfo "    8: Inverse telecine - First field is top field with BC telecine pattern."
  OutputInfo "    9: Inverse telecine - First field is top field with CD telecine pattern."
  OutputInfo "    10:Inverse telecine - First field is top field with DD telecine pattern."
  OutputInfo "    11:Inverse telecine - First field is bottom field with AA telecine "
  OutputInfo "       pattern."
  OutputInfo "    12:Inverse telecine - First field is bottom field with BB telecine"
  OutputInfo "       pattern."
  OutputInfo "    13:Inverse telecine - First field is bottom field with BC telecine"
  OutputInfo "       pattern."
  OutputInfo "    14:Inverse telecine - First field is bottom field with CD telecine"
  OutputInfo "      pattern."
  OutputInfo "    15:Inverse telecine - First field is bottom field with DD telecine"
  OutputInfo "       pattern."
  OutputInfo "    16:Interlaced encoding."
  OutputInfo "    17:Interlaced encoding - First field is top field."
  OutputInfo "    18:Interlaced encoding - First field is bottom field."
  OutputInfo "    Default: 0: None."
  OutputInfo "[-v_clip] <left> <top> <right> <bottom>"
  OutputInfo "    The region of the image (with respect to source) to be clipped/encoded."
  OutputInfo "    If the right is 0 or greater than width, it is ignored and re-assigned"
  OutputInfo "    to width. If the bottom is 0 or greater than height, it is ignored and"
  OutputInfo "    re-assigned to height."
  OutputInfo "[-v_mode] <mode number>"
  OutputInfo "    Video encoding to be used."
  OutputInfo "    0: 1-pass CBR (default)."
  OutputInfo "    1: 2-pass CBR."
  OutputInfo "    2: Quality-based VBR."
  OutputInfo "    3: Bit rate-based VBR (two-pass)."
  OutputInfo "    4: Peak bit rate-based VBR (two-pass)."
  OutputInfo "[-v_bitrate] <bit rate>"
  OutputInfo "    Video bit rate in bits per second to be encoded. Default: 250000 bps."
  OutputInfo "    Set this to 0 for audio-only coding."
  OutputInfo "[-v_buffer] <buffer>"
  OutputInfo "    Delay buffer in milliseconds for video. Default: 5000 msec."
  OutputInfo "[-v_quality] <quality>"
  OutputInfo "    CBR: Quality/smoothness tradeoff. 0 to 100, 0 being the smoothest."
  OutputInfo "    Default: 75."
  OutputInfo "    Quality-based VBR: Image quality for the video. Encode video to the"
  OutputInfo "    specified quality, regardless of bit rate. Default: 95"
  OutputInfo "[-v_peakbitrate] <peak bit rate>"
  OutputInfo "    Peak bit rate in bits per second for peak bit rate-based VBR for video."
  OutputInfo "    If not specified, the peak bit rate is 1.5 times the video bit rate."
  OutputInfo "[-v_peakbuffer] <peak buffer>"
  OutputInfo "    Buffer in msec for video, with peak bit rate-based VBR. If not specified,"
  OutputInfo "    the default of 5000 msec is used."
  OutputInfo "[-v_performance] <performance>"
  OutputInfo "    Use to adjust hardware performance settings. Possible values: 0, 20, 40,"
  OutputInfo "    60, 80, and 100, with 100 representing the highest quality. If you do not"
  OutputInfo "    specify a value, codec defaults are used."
  OutputInfo "[-v_profile] <device conformance>"
  OutputInfo "    Specifies the category of complexity of the encoded content. Use if you"
  OutputInfo "    are targeting your content for playback on a hardware device other than"
  OutputInfo "    a computer. Some hardware devices only support certain categories. (Refer"
  OutputInfo "    to the documentation for your device for more information.) If you do not"
  OutputInfo "    add the -v_profile option to a command, the correct complexity setting is"
  OutputInfo "    selected automatically during encoding. Possible values are SP (Simple),"
  OutputInfo "    MP (main), or CP (complex)."
  OutputInfo "[-duration] <seconds>"
  OutputInfo "    Amount of time in seconds to encode. Use when sourcing from devices."
  OutputInfo "[-saveprofile] <file name>"
  OutputInfo "    Saves current settings to a file for later reuse. Default file name"
  OutputInfo "    extension is .prx."
  OutputInfo "[-devices]"
  OutputInfo "    Lists audio and video capture devices."
  OutputInfo "[-v_codecs]"
  OutputInfo "    Lists all video codecs."
  OutputInfo "[-videoonly]"
  OutputInfo "    Encodes video stream only."
  OutputInfo "[-pixelratio] <x y>"
  OutputInfo "    Specifies the video pixel aspect ratio."
  OutputInfo "[-pixelformat] <format>"
  OutputInfo "    Specifies the video pixel format. Possible values:"
  OutputInfo "    I420"
  OutputInfo "    IYUV"
  OutputInfo "    RGB24"
  OutputInfo "    RGB32"
  OutputInfo "    RGB555"
  OutputInfo "    RGB565"
  OutputInfo "    RGB8"
  OutputInfo "    UYVY"
  OutputInfo "    YUY2"
  OutputInfo "    YV12"
  OutputInfo "    YVU9"
  OutputInfo "    YVYU"
  OutputInfo "[-maxpacket] <packet size>"
  OutputInfo "    Specifies the maximum packet size in bytes."
  OutputInfo "[-minpacket] <packet size>"
  OutputInfo "    Specifies the minimum packet size in bytes."
  OutputInfo ""
  OutputInfo "[-title] <string>"
  OutputInfo "    Title of the content. Enclose strings with spaces in quotations. For"
  OutputInfo "    example: -title ""Windows Media Sample"""
  OutputInfo "[-author] <string>"
  OutputInfo "[-copyright] <string>"
  OutputInfo "[-description] <string>    "
  OutputInfo "[-rating] <string>"
  OutputInfo "NOTE: The maximum string length for each one is 255.  "
end function

' Brings up the encoder utility chm file
function ShowHelpChm()
  dim strChmFileName, strExeName, strCommandLine
  dim RetVal
  dim objShell, objDir

  'Create a shell object. we will use this to execute the command
  Set objShell = WScript.CreateObject("WScript.Shell")

  'Get the Windows directory object
  set objDir = g_objFileSystem.GetSpecialFolder( 0 )

  'make the executable name ( %windir%\hh.exe ) form the windows dir object
  strExeName = objDir.drive & "\" & objDir.name & "\"
  strExeName = strExeName & "hh.exe" 

  'make the full path name for the chm file to be shown
  strChmFileName = """" & ObjDir.drive & "\program files\windows media components\encoder\wmencutil.chm" & """"

  'make the command line
  strCommandLine = strExeName & " " & strChmFileName

  'run the command and dont wait for the app to finish
  RetVal = objShell.Run( strCommandLine, 4, FALSE )
end function

' Print out info about the input file
function PrintFileInfo()
  dim objFileInfo
  dim strFileInfo
  dim strInput2
    
  strInput2 = g_objFileSystem.GetAbsolutePathName( g_strInput )

  set objFileInfo = WScript.CreateObject( "FileInfo.MediaInfo" )

  objFileInfo.FileName = strInput2

  strFileInfo = objFileInfo.MediaInfo

  OutputInfo strFileInfo
end function

' List audio codecs.
function ListAudioCodecs()
  dim objProfile
  dim objName
  dim intFourCC
  dim intVBRMode
  dim strCodecId
  dim intCount
  
  set objProfile = WScript.CreateObject( "WMENCEng.WMEncProfile2" )

  OutputInfo vbCrLf & "Audio Codecs: "

  ' For all VBR modes 
  for intVBRMode = 1 to 4
    objProfile.VBRMode(WMENC_AUDIO, 0) = intVBRMode

    select case intVBRMode
      case WMENC_PVM_NONE
        OutputInfo "CBR Mode :"

      case WMENC_PVM_PEAK
        OutputInfo "Peak Bit Rate-Based VBR Audio Mode :"

      case WMENC_PVM_UNCONSTRAINED
        OutputInfo "Quality-Based VBR Audio Mode :"

      case WMENC_PVM_BITRATE_BASED
        OutputInfo "Bit Rate-Based VBR Audio Mode :"
  
      case else
        OutputInfo "Unknown Mode"
    end select

    intCount = 0
    ' Enum all audio codecs
    for i=0 to objProfile.AudioCodecCount-1
      objProfile.EnumAudioCodec i, objName

      'Check and display fourcc
      intFourCC = objProfile.GetCodecFourCCFromIndex(WMENC_AUDIO, i)
      select case intFourCC
        case WMA9STD_FOURCC
          strCodecId = WMA9STD

        case WMA9PRO_FOURCC
          strCodecId = WMA9PRO

        case WMA9LSL_FOURCC
          strCodecId = WMA9LSL

        case WMSPEECH_FOURCC
          strCodecId = WMSPEECH

        case PCM_FOURCC
          strCodecId = PCM

        case else
          strCodecId = ""
      end select

      if( strCodecId <> "" ) then  
        OutputInfo vbTab &  "[" & intCount & "] "  & strCodecId & " : " & objName
        intCount = intCount + 1
      end if
    next

    OutputInfo vbCrLf
  next
  
end function

' List video codecs.
function ListVideoCodecs()
  dim objProfile
  dim objName
  dim intFourCC
  dim intVBRMode
  dim strCodecId
  dim intCount
  
  set objProfile = WScript.CreateObject( "WMENCEng.WMEncProfile2" )

  OutputInfo vbCrLf & "Video Codecs: "

  ' For all VBR modes 
  for intVBRMode = 1 to 4
    objProfile.VBRMode(WMENC_VIDEO, 0) = intVBRMode

    select case intVBRMode
      case WMENC_PVM_NONE
        OutputInfo "CBR Mode :"

      case WMENC_PVM_PEAK
        OutputInfo "Peak Bit Rate-Based VBR Audio Mode :"

      case WMENC_PVM_UNCONSTRAINED
        OutputInfo "Quality-Based VBR Audio Mode :"

      case WMENC_PVM_BITRATE_BASED
        OutputInfo "Bit Rate-Based VBR Audio Mode :"
  
      case else
        OutputInfo "Unknown Mode"
    end select

    intCount = 0
    ' Enum all video codecs
    for i=0 to objProfile.VideoCodecCount-1
      objProfile.EnumVideoCodec i, objName

      'Check and display fourcc
      intFourCC = objProfile.GetCodecFourCCFromIndex(WMENC_VIDEO, i)
      select case intFourCC
        case WMV7_FOURCC
          strCodecId = WMV7

        case WMV8_FOURCC
          strCodecId = WMV8

        case WMV9_FOURCC
          strCodecId = WMV9

        case WMS9_FOURCC
          strCodecId = WMS9

        case MP41_FOURCC
          strCodecId = MP41

        case UNCOMP_FOURCC
          strCodecId = UNCOMP

        case else
          strCodecId = ""
      end select

      if( strCodecId <> "" ) then  
        OutputInfo vbTab &  "[" & i & "] "  & strCodecId & " : " & objName
        intCount = intCount + 1
      end if
    next
    OutputInfo vbCrLf
  next


end function

' List audio formats.
function ListAudioFormats()
  dim objProfile
  dim objName
  dim intFourCC
  dim intVBRMode
  dim strCodecId, strAudFormatName, strSetting
  dim intAudioSampleRate, intAudioChannels, intAudioBitsPerSample, intAudioBitrate 
  dim intArgSpaceCount, intBitrateSpaceCount, intSamplingRateSpaceCount, intChannelSpaceCount
  
  intArgSpaceCount = 24
  intBitrateSpaceCount = 16
  intSamplingRateSpaceCount = 20
  intChannelSpaceCount = 10

  set objProfile = WScript.CreateObject( "WMENCEng.WMEncProfile2" )

  OutputInfo vbCrLf & "Audio Codecs: "

  ' For all VBR modes 
  for intVBRMode = 1 to 4
    objProfile.VBRMode(WMENC_AUDIO, 0) = intVBRMode

    select case intVBRMode
      case WMENC_PVM_NONE
        OutputInfo "CBR Mode :"

      case WMENC_PVM_PEAK
        OutputInfo "Peak Bit Rate-Based VBR Audio Mode :"

      case WMENC_PVM_UNCONSTRAINED
        OutputInfo "Quality-Based VBR Audio Mode :"

      case WMENC_PVM_BITRATE_BASED
        OutputInfo "Bit Rate-Based VBR Audio Mode :"
  
      case else
        OutputInfo "Unknown Mode"
    end select

    ' Enum all audio codecs
    for i=0 to objProfile.AudioCodecCount-1
      objProfile.EnumAudioCodec i, objName

      'Check and display fourcc
      intFourCC = objProfile.GetCodecFourCCFromIndex(WMENC_AUDIO, i)
      select case intFourCC
        case WMA9STD_FOURCC
          strCodecId = WMA9STD

        case WMA9PRO_FOURCC
          strCodecId = WMA9PRO

        case WMA9LSL_FOURCC
          strCodecId = WMA9LSL

        case WMSPEECH_FOURCC
          strCodecId = WMSPEECH

        case PCM_FOURCC
          strCodecId = PCM
      end select

      if( strCodecId <> "" ) then  
        OutputInfo vbTab & strCodecId & " : " & objName & " (" & strCodecId & "):"
      else
        OutputInfo vbTab & objName & ":"
      end if

      OutputInfo vbCrLf

      if intVBRMode = WMENC_PVM_UNCONSTRAINED then
        if strCodecId <> WMA9PRO then
          OutputInfo vbTab & vbTab & PadSpaces("Argument", intArgSpaceCount) & PadSpaces("Quality", intBitrateSpaceCount) & PadSpaces("Sampling rate", intSamplingRateSpaceCount) & "Channel"
        else
          OutputInfo vbTab & vbTab & PadSpaces("Argument", intArgSpaceCount) & PadSpaces("Quality", intBitrateSpaceCount) & PadSpaces("Sampling rate", intSamplingRateSpaceCount) & PadSpaces("Channel", intChannelSpaceCount) & "BitsPerSample"
        end if
      else
        if strCodecId <> WMA9PRO and strCodecId <> WMA9LSL then
          OutputInfo vbTab & vbTab & PadSpaces("Argument", intArgSpaceCount) & PadSpaces("Bitrate", intBitrateSpaceCount) & PadSpaces("Sampling rate", intSamplingRateSpaceCount) & "Channel"
        else
          OutputInfo vbTab & vbTab & PadSpaces("Argument", intArgSpaceCount) & PadSpaces("Bitrate", intBitrateSpaceCount) & PadSpaces("Sampling rate", intSamplingRateSpaceCount) & PadSpaces("Channel", intChannelSpaceCount) & "BitsPerSample"
        end if
      end if

      OutputInfo vbTab & vbTab & "------------------------------------------------------------------------------------"

      For j=0 To objProfile.audioFormatCount(i) - 1
        intAudioBitrate = objProfile.EnumAudioFormat(i, j, strAudFormatName, intAudioSampleRate, intAudioChannels, intAudioBitsPerSample)

        if intVBRMode <> WMENC_PVM_UNCONSTRAINED then
          intAudioBitrate = CLng(intAudioBitrate / 1000)
        else
          intAudioBitrate = "Q" & intAudioBitrate
        end if

        if intAudioSampleRate = 88200 then
          intAudioSampleRate = 88
        elseif intAudioSampleRate = 44100 then
          intAudioSampleRate = 44
        elseif intAudioSampleRate = 22050 then
          intAudioSampleRate = 22
        elseif intAudioSampleRate = 11025 then
          intAudioSampleRate = 11
        else
          intAudioSampleRate = Clng(intAudioSampleRate / 1000)
        end if

        if strCodecId <> WMA9PRO and strCodecId <> WMA9LSL then
          strSetting = intAudioBitrate & "_" & intAudioSampleRate & "_" & intAudioChannels
          OutputInfo vbTab & vbTab & PadSpaces(strSetting, intArgSpaceCount) & PadSpaces(intAudioBitrate, intBitrateSpaceCount) & PadSpaces(intAudioSampleRate, intSamplingRateSpaceCount) & intAudioChannels
        else 
          strSetting = intAudioBitrate & "_" & intAudioSampleRate & "_" & intAudioChannels & "_" & intAudioBitsPerSample
          OutputInfo vbTab & vbTab & PadSpaces(strSetting, intArgSpaceCount) & PadSpaces(intAudioBitrate, intBitrateSpaceCount) & PadSpaces(intAudioSampleRate, intSamplingRateSpaceCount) & PadSpaces(intAudioChannels, intChannelSpaceCount) & intAudioBitsPerSample
        end if

      next 
  
      OutputInfo vbCrLf & vbCrLf

    next
    OutputInfo vbCrLf
  next
  
end function


' Add spaces to the end of arg to create intCount characters string
function PadSpaces( arg, intCount )

  if Len(arg) >= intCount then
    PadSpaces = arg
    exit function
  end if

  PadSpaces = arg & Space( intCount - Len(arg) )
end function



' List all audio and video devices.
function ListDevices()
  dim objManager, objPlugin, i, j, k, count

  set objManager = g_objEncoder.SourcePluginInfoManager

  for i=0 to 1
    if i = 0 then
      OutputInfo "audio device:"
    elseif i = 1 then
      OutputInfo "video device:"
    end if
    count = 0
    for j=0 to objManager.Count-1
      set objPlugin = objManager.Item(j)

      if objPlugin.Resources = true then
        if i = 0 then
          if objPlugin.MediaType = 1 or objPlugin.MediaType = 3 then
            ' Enum all devices in this type
            for k=0 to objPlugin.Count-1
              OutputInfo vbTab & count & " : " & objPlugin.Item(k)
              count = count + 1
            next
          end if
        elseif i = 1 then
          if objPlugin.MediaType = 2 or objPlugin.MediaType = 3 or objPlugin.MediaType = 6 then
            ' Enum all devices in this type
            for k=0 to objPlugin.Count-1
              OutputInfo vbTab & count & " : " & objPlugin.Item(k)
              count = count + 1
            next
          end if
        end if
      end if
    next
  next
end function


' Setup encoder using the current settings.
function SetupEncoder( )
  
  if not CreateSourceGroup() then
    SetupEncoder = false
    exit function
  end if

  if not SetupInput() then
    SetupEncoder = false
    exit function
  end if

  if not SetupProfile() then
    SetupEncoder = false
    exit function
  end if

  if not SetupOutput()  then
    SetupEncoder = false
    exit function
  end if

  if not SetupTime() then
    SetupEncoder = false
    exit function
  end if

  if not SetupDisplayInfo() then
    SetupEncoder = false
    exit function
  end if

  g_objEncoder.AutoStop = true

  SetupEncoder = true
end function

' Create Source group by loading from wme or adding a new one
function CreateSourceGroup()

  dim objProfile
  dim intCodecIndex

  ' If a WME file is provided, load the encoding configuration from this WME file.
  if g_strWMEFile <> "" then
    on error resume next

    ' Load enocder session from a WME file
    g_objEncoder.Load( g_objFileSystem.GetAbsolutePathName( g_strWMEFile ) )

    ' Add a new source group.
    set g_objSourceGroup = g_objEncoder.SourceGroupCollection.Item(0)

    set objProfile = g_objSourceGroup.Profile
    set g_objProfile = WScript.CreateObject( "WMENCENG.WMEncProfile2" )
    g_objProfile.LoadFromIWMProfile( objProfile )

    ' If source file has an auido stream, get the audio source.
    if g_objSourceGroup.SourceCount( WMENC_AUDIO ) > 0 then
      set g_objAudioSource = g_objSourceGroup.Source( WMENC_AUDIO, 0 )
      intCodecIndex = g_objProfile.Audience(0).AudioCodec( 0 )
      g_objProfile.EnumAudioCodec intCodecIndex, g_strAudioCodecName
    end if

    ' If source file has a video stream, get the video source.
    if g_objSourceGroup.SourceCount( WMENC_VIDEO ) > 0 then
      set g_objVideoSource = g_objSourceGroup.Source( WMENC_VIDEO, 0 )
      intCodecIndex = g_objProfile.Audience(0).VideoCodec( 0 )
      g_objProfile.EnumVideoCodec intCodecIndex, g_strVideoCodecName
    end if

    g_blnCreateCustomProfile = false

    if g_intMaxPacketSize = -1 then
      g_intMaxPacketSize = objProfile.MaxPacketSize
    end if

    if err.number <> 0 then
      OutputInfo "Loading the session (.wme) file failed: " & g_strWMEFile
      err.Clear
      CreateSourceGroup = false
      exit function
    end if
  else
    ' Create a new source group for this session
    set g_objSourceGroup = g_objEncoder.SourceGroupCollection.Add( "SG_1" )
  end if

  CreateSourceGroup = true
end function

' Setup profile configuration
function SetupProfile()
  on error resume next

  dim objProfile
  dim objAudience
  dim blnRet
  dim strProfileLoad
  dim objFileInfo
  dim intVideoFramerateTemp
  dim intSourceVideoFramerate
  dim strInput2

  ' save the global frame rate, since we might over-write it based on the input files frame rate
  intVideoFramerateTemp = g_intVideoFramerate

  if IsNull(g_objProfile) then
    Set g_objProfile = WScript.CreateObject( "WMENCEng.WMEncProfile2" )
    g_objProfile.ContentType = g_intSessionType
    g_objProfile.ProfileName = "WMCMDProfile"

    Set objAudience = g_objProfile.AddAudience( 10000000 )
  else
    g_objProfile.ContentType = g_intSessionType
    Set objAudience = g_objProfile.Audience( 0 )

  end if


  if g_intVideoFramerate = -1 then
    ' Assume 30 fps unless we can get the input files framerate
    g_intVideoFramerate = 30000
  end if

  strInput2 = g_objFileSystem.GetAbsolutePathName( g_strInput )
  set objFileInfo = WScript.CreateObject( "FileInfo.MediaInfo" )
  objFileInfo.FileName = strInput2
  intSourceVideoFramerate = 100000
  intSourceVideoFramerate = objFileInfo.FrameRate * 1000
  if intSourceVideoFramerate = 0 then
    intSourceVideoFramerate = 100000
  end if

  ' Set the video frame rate to lower of the source input or user specified frame rate for better quality
  if intSourceVideoFramerate < g_intVideoFramerate then
    g_intVideoFramerate = intSourceVideoFramerate
  end if

  ' if profile string is specified load it
  if g_strProfile <> "" then
    blnRet = SetupPredefinedProfile( objAudience )

    g_blnCreateCustomProfile = false

    if not blnRet then
      SetupProfile = false
      exit function
    end if
  end if

  if g_strProfileLoad <> "" then
    strProfileLoad = g_objFileSystem.GetAbsolutePathName( g_strProfileLoad )
    g_objProfile.LoadFromFile strProfileLoad
    g_blnCreateCustomProfile = false
    Set objAudience = g_objProfile.Audience( 0 )

    ' enable two pass if required
    if not isnull( g_objVideoSource ) then        
      if g_objProfile.VBRMode( WMENC_VIDEO, 0 ) = WMENC_PVM_PEAK or g_objProfile.VBRMode( WMENC_VIDEO, 0 ) = WMENC_PVM_BITRATE_BASED then
        g_objVideoSource.PreProcessPass = 1
      end if
      
      if objAudience.VideoFPS(0) > g_intVideoFramerate then
        objAudience.VideoFPS(0) = g_intVideoFramerate
      else
        g_intVideoFramerate = objAudience.VideoFPS(0)
      end if
    end if

    if not isnull( g_objAudioSource ) then        
      if g_objProfile.VBRMode( WMENC_AUDIO, 0 ) = WMENC_PVM_PEAK or g_objProfile.VBRMode( WMENC_AUDIO, 0 ) = WMENC_PVM_BITRATE_BASED then
        g_objAudioSource.PreProcessPass = 1
      end if
    end if
  end if

  ' First time: override the WME or predefined profile with custom values
  blnRet = SetupCustomProfile( objAudience )
  if not blnRet then
    SetupProfile = false
    exit function
  end if

  if g_blnCreateCustomProfile then
    ' Second time: Update profile based on input specified and default values
    LoadProfileDefaults()

    blnRet = SetupCustomProfile( objAudience )
    if not blnRet then
      SetupProfile = false
      exit function
    end if
  end if

  ' Set a few more things like interlaced coding and pixel ratio
  if g_intPixelAspectRatioX <> -1 or g_intPixelAspectRatioY <> -1 then
    g_objProfile.NonSquarePixelMode(0) = true
  end if

  if g_intVideoPreprocess >= 16 and g_intVideoPreprocess <= 18 then
    g_objProfile.InterlaceMode(0) = true
  end if

  if g_intMinPacketSize <> -1 then
    g_objProfile.MinPacketSize = g_intMinPacketSize
  end if

  ' Set the profile in encoder.
  g_objSourceGroup.Profile = g_objProfile

  if g_intMaxPacketSize <> -1 then
    g_objSourceGroup.Profile.MaxPacketSize = g_intMaxPacketSize
  end if

  g_intVideoFramerate = intVideoFramerateTemp

  ' Invalid profile
  if err.number <> 0 then
    OutputInfo "Invalid profile: 0x" & Hex( err.number ) & " " & err.Description
    err.Clear
    SetupProfile = false
    exit function
  end if

  if g_strProfileSave <> "" then
    g_objProfile.SaveToFile g_strProfileSave
  end if

  SetupProfile = true
end function

' Setup ouptut configuration.
function SetupOutput()
  ' Is an output file name provided?
  if g_strOutput <> "" then
    ' Is this output name a directory?
    if g_objFileSystem.FolderExists( g_strOutput ) then
      
      ' If the output name is a directory, create the real output file name.
      ' If the input is a file, use the file name of the input file (extension is not ussed).
      ' If the inputs are devices, use 'output' as the output name.     
      if g_strInput <> "" then
        g_strOutput = g_strOutput & "\\" & g_objFileSystem.GetBaseName( g_strInput )
      else
        g_strOutput = g_strOutput & "\\output"
      end if
    end if

    ' If the output file name doesn't have a extension, create a proper one.
    if g_objFileSystem.GetExtensionName( g_strOutput ) = "" then      

      ' Does "." already exist in the output file name.
      ' If not, append "." to the output file name.
      if Right( g_strOutput, 1 ) <>  "." then
        g_strOutput = g_strOutput & "."
      end if

      ' Does this session have a video source?
      ' If it has video stream, append wmv extension, otherwise wma extension is appended.
      if g_objSourceGroup.SourceCount( WMENC_VIDEO ) > 0 then
        g_strOutput = g_strOutput & "wmv"
      else
        g_strOutput = g_strOutput & "wma"
      end if
    end if

    CreateOutputFolder( g_objFileSystem.GetParentFolderName( g_objFileSystem.GetAbsolutePathName( g_strOutput ) ) )

    g_objEncoder.File.LocalFileName = g_strOutput
  else
    ' Get the output file name from WME file.
    if not IsNull(g_objEncoder.File.LocalFileName) then
      g_strOutput = g_objEncoder.File.LocalFileName
    end if
  end if

  ' Enable HTTP broadcasting for the encoder if a broadcast port is provided.
  if g_intBroadcast <> -1 then
    g_objEncoder.Broadcast.PortNumber( WMENC_PROTOCOL_HTTP ) = g_intBroadcast
  end if

  ' Enable push if push server is specified
  if g_strPushServer <> "" then
    g_objEncoder.Broadcast.ServerName = g_strPushServer
    g_objEncoder.Broadcast.PublishingPoint = g_strPublishingPoint
    if g_strPushTemplate <> "" then
      g_objEncoder.Broadcast.Template = g_strPushTemplate
    end if
  end if

  SetupOutput = true
end function

' Setup display configuration
function SetupDisplayInfo()
  dim objDisplayInfo
  dim objAttributes
  
  set objDisplayInfo = g_objEncoder.DisplayInfo
  set objAttributes = g_objEncoder.Attributes

  ' Is title set?
  if g_strTitle <> "" then
    objDisplayInfo.Title = g_strTitle
  end if

  ' Is author set?
  if g_strAuthor <> "" then
    objDisplayInfo.Author = g_strAuthor
  end if

  ' Is copyright set?
  if g_strCopyright <> "" then
    objDisplayInfo.Copyright = g_strCopyright
  end if

  ' Is description set?
  if g_strDescription <> "" then
    objDisplayInfo.Description = g_strDescription
  end if

  ' Is rating set?
  if g_strRating <> "" then
    objDisplayInfo.Rating = g_strRating
  end if

  ' Add folddown coefficients
  if g_intAudioSurroundMix <> -1 then
    objAttributes.Add "SurroundMix", -g_intAudioSurroundMix
    objAttributes.Add "CenterMix", -g_intAudioCenterMix
    objAttributes.Add "LFEMix", -g_intAudioLEFMix
  end if

  SetupDisplayInfo = true
end function

' Setup a new profile based on predefined profiles
function SetupPredefinedProfile(objAudience)
  dim intAudioCodecIndex, intVideoCodecIndex
  
  '  The following defined built in profiles.  Built in profiles are defined by the following:
  '  Audio:  Codec, channels, sample rate, bit rate, bit depth
  '  Video:  Codec, bit rate, width, height, fpsx1000, buffer (ms), smoothness, key frame 
  '  New built in profiles can be added by creating a new case statement and specifying
  '  audio and video profile settings.
  select case LCase( g_strProfile )
    case "av20":
      SetupAudioAudience objAudience, "WMA9STD", 1, 8000, 5000, 16
      SetupVideoAudience objAudience, "WMV9", 15000, 160, 120, 15000, 5000, 75, 10000

    case "av32":
      SetupAudioAudience objAudience, "WMA9STD", 1, 11025, 10168, 16
      SetupVideoAudience objAudience, "WMV9", 22000, 176, 144, 15000, 5000, 75, 10000

    case "av100":
      SetupAudioAudience objAudience, "WMA9STD", 1, 16000, 16000, 16
      SetupVideoAudience objAudience, "WMV9", 84000, 320, 240, 15000, 5000, 75, 10000

    case "av225":
      SetupAudioAudience objAudience, "WMA9STD", 2, 32000, 40000, 16
      SetupVideoAudience objAudience, "WMV9", 185000, 320, 240, 30000, 5000, 75, 10000

    case "av350":
      SetupAudioAudience objAudience, "WMA9STD", 2, 32000, 48000, 16
      SetupVideoAudience objAudience, "WMV9", 302000, 320, 240, 30000, 5000, 75, 10000

    case "av450":
      SetupAudioAudience objAudience, "WMA9STD", 2, 44100, 64040, 16
      SetupVideoAudience objAudience, "WMV9", 386000, 320, 240, 30000, 5000, 75, 10000

    case "av700":
      SetupAudioAudience objAudience, "WMA9STD", 2, 44100, 64040, 16
      SetupVideoAudience objAudience, "WMV9", 636000, 320, 240, 30000, 5000, 75, 10000

    case "av1400":
      SetupAudioAudience objAudience, "WMA9STD", 2, 44100, 128040, 16
      SetupVideoAudience objAudience, "WMV9", 1272000, 320, 240, 30000, 5000, 75, 10000

    case "av350pal":
      SetupAudioAudience objAudience, "WMA9STD", 2, 32000, 48000, 16
      SetupVideoAudience objAudience, "WMV9", 302000, 384, 288, 25000, 5000, 75, 10000

    case "av700pal":
      SetupAudioAudience objAudience, "WMA9STD", 2, 44100, 64040, 16
      SetupVideoAudience objAudience, "WMV9", 636000, 384, 288, 25000, 5000, 75, 10000

    case "av100_2p":
      SetupAudioAudience objAudience, "WMA9STD", 1, 16000, 16000, 16
      SetupVideoAudience objAudience, "WMV9", 84000, 320, 240, 15000, 5000, 75, 10000
      g_objAudioSource.PreProcessPass = 1
      g_objVideoSource.PreProcessPass = 1

    case "av350_2p"
      SetupAudioAudience objAudience, "WMA9STD", 2, 32000, 48000, 16
      SetupVideoAudience objAudience, "WMV9", 302000, 320, 240, 30000, 5000, 75, 10000
      g_objAudioSource.PreProcessPass = 1
      g_objVideoSource.PreProcessPass = 1

    case "av600vbr":
      g_objProfile.VBRMode(WMENC_VIDEO, 0) = WMENC_PVM_PEAK
      SetupAudioAudience objAudience, "WMA9STD", 2, 44100, 64040, 16
      SetupVideoAudience objAudience, "WMV9", 536000, 320, 240, 24000, 5000, 75, 10000
      
      objAudience.VideoPeakBitrate(0) = 1.5 * objAudience.VideoBitrate(0)
      objAudience.VideoBufferMax(0) = 5000

      ' Turn telecine and two pass video on
      g_objVideoSource.Optimization = WMENC_VIDEO_INVERSETELECINE
      g_objVideoSource.PreProcessPass = 1 

    case "avq97vbr":
      g_objProfile.VBRMode(WMENC_VIDEO, 0) = WMENC_PVM_UNCONSTRAINED
      SetupAudioAudience objAudience, "WMA9STD", 2, 44100, 64040, 16
      SetupVideoAudience objAudience, "WMV9", 536000, 320, 240, 24000, 5000, 97, 10000

      ' Turn telecine on
      g_objVideoSource.Optimization = WMENC_VIDEO_INVERSETELECINE

    case "a20_1":
      SetupAudioAudience objAudience, "WMA9STD", 1, 22050, 20008, 16

    case "a20_2":
      SetupAudioAudience objAudience, "WMA9STD", 2, 22050, 20008, 16

    case "a32":
      SetupAudioAudience objAudience, "WMA9STD", 2, 32000, 32000, 16

    case "a48":
      SetupAudioAudience objAudience, "WMA9STD", 2, 32000, 48024, 16

    case "a64":
      SetupAudioAudience objAudience, "WMA9STD", 2, 44100, 64016, 16

    case "a96":
      SetupAudioAudience objAudience, "WMA9STD", 2, 44100, 96024, 16

    case "a128":
      SetupAudioAudience objAudience, "WMA9STD", 2, 44100, 128016, 16

    case "v20":
      SetupVideoAudience objAudience, "WMV9", 20000, 176, 144, 15000, 5000, 75, 10000

    case "v32":
      SetupVideoAudience objAudience, "WMV9", 32000, 240, 176, 15000, 5000, 75, 10000

    case else
      OutputInfo "No predefined profile found."
      SetupPredefinedProfile = false
      exit function

  end select

  SetupPredefinedProfile = true
end function

' Given audio parameters this will setup the audience
function SetupAudioAudience(objAudience, strAudioCodec, intChannels, intSampleRate, intBitrate, intBitsPerSample)
  dim intAudioCodecIndex

  intAudioCodecIndex = GetAudioCodecIndex(strAudioCodec)
  g_objProfile.EnumAudioCodec intAudioCodecIndex, g_strAudioCodecName
  
  objAudience.AudioCodec(0) = intAudioCodecIndex
  objAudience.SetAudioConfig 0, intChannels, intSampleRate, intBitrate, intBitsPerSample
end function

' Given video parameters this will setup the audience
function SetupVideoAudience(objAudience, strVideoCodec, intBitrate, intWidth, intHeight, intFps, intBufferWindow, intSmoothness, intKeyFrame)
  dim intVideoCodecIndex
  
  if intFps > g_intVideoFramerate then
    intFps = g_intVideoFramerate
  else
    g_intVideoFramerate = intFps
  end if

  intVideoCodecIndex = GetVideoCodecIndex(strVideoCodec)
  g_objProfile.EnumVideoCodec intVideoCodecIndex, g_strVideoCodecName

  objAudience.VideoCodec(0) = intVideoCodecIndex
  objAudience.VideoBitrate(0) = intBitrate
  objAudience.VideoWidth(0) = intWidth
  objAudience.VideoHeight(0) = intHeight
  objAudience.VideoFps(0) = intFps
  objAudience.VideoKeyFrameDistance(0) = intKeyFrame
  objAudience.VideoBufferSize(0) = intBufferWindow

  ' For quality vbr mode use intSmoothness as VBR quality
  if g_objProfile.VBRMode(WMENC_VIDEO, 0) = WMENC_PVM_UNCONSTRAINED then
    objAudience.VideoCompressionQuality(0) = intSmoothness
  else
    objAudience.VideoImageSharpness(0) = intSmoothness
  end if
end function

' Given audio string(eg WMA9STD) this returns the audio codec index
function GetAudioCodecIndex(strAudioCodec)
  dim intFourCC

  select case strAudioCodec
    case WMA9STD
      intFourCC = WMA9STD_FOURCC

    case WMA9PRO
      intFourCC = WMA9PRO_FOURCC

    case WMA9LSL
      intFourCC = WMA9LSL_FOURCC

    case WMSPEECH
      intFourCC = WMSPEECH_FOURCC

    case PCM
      intFourCC = PCM_FOURCC

    case else
      intFourCC = WMA9STD_FOURCC
  end select

  GetAudioCodecIndex = g_objProfile.GetCodecIndexFromFourCC(WMENC_AUDIO, intFourCC)

end function

' Given the video string(eg WMV9) this returns the video codec index
function GetVideoCodecIndex(strVideoCodec)
  dim intFourCC

  select case strVideoCodec
    case WMV7
      intFourCC = WMV7_FOURCC

    case WMV8
      intFourCC = WMV8_FOURCC

    case WMV9
      intFourCC = WMV9_FOURCC

    case WMS9
      intFourCC = WMS9_FOURCC

    case MP41
      intFourCC = MP41_FOURCC

    case UNCOMP
      intFourCC = UNCOMP_FOURCC

    case else
      intFourCC = WMV9_FOURCC
  end select
  
  GetVideoCodecIndex = g_objProfile.GetCodecIndexFromFourCC(WMENC_VIDEO, intFourCC)

end function

' Setup a new profile based on predefined profiles
function SetupCustomProfile(objAudience)
  dim intAudioCodecIndex, intVideoCodecIndex
  dim blnRet

  if not g_blnAudioOnly and not g_blnVideoOnly  then
    if g_strVideoCodec = UNCOMP and g_strAudioCodec <> PCM then
      OutputInfo "Audio codec must be uncompressed when using uncompressed video codec"
      SetupCustomProfile = false
      exit function
    end if

    if g_strVideoCodec <> UNCOMP and g_strAudioCodec = PCM then
      OutputInfo "Video codec must be uncompressed when using uncompressed audio codec"
      SetupCustomProfile = false
      exit function
    end if  
  end if

  if not isnull( g_objAudioSource ) then
    blnRet = SetupCustomAudioProfile( objAudience )
    if not blnRet then
      SetupCustomProfile = false
      exit function
    end if
  end if

  if not isnull( g_objVideoSource ) then
    blnRet = SetupCustomVideoProfile( objAudience )
    if not blnRet then
      SetupCustomProfile = false
      exit function
    end if
  end if

  SetupCustomProfile = true

end function

' Setup the audio part of the custom profile
function SetupCustomAudioProfile(objAudience)
  on error resume next

  ' Setup audio
  dim intAudioCodecIndex, intAudioBitrate, intAudioSampleRate, intAudioChannels, intBitDepth, intFourCC
  dim strTemp
  dim intNextArgStart, intNextArgEnd
  dim blnAudioQuality, blnValidBitrate, blnValidSampleRate, blnValidChannels, blnValidBitDepth


  'Set audio vbr mode
  if g_intAudioVBRMode <> -1 then
    select case g_intAudioVBRMode
      case 0 ' 1 pass CBR

      case 1 ' 2 pass CBR
        g_objAudioSource.PreProcessPass = 1

      case 2 ' Quality VBR mode
        g_objProfile.VBRMode(WMENC_AUDIO, 0) = WMENC_PVM_UNCONSTRAINED

      case 3 ' 2 pass bitrate based VBR
        g_objProfile.VBRMode(WMENC_AUDIO, 0) = WMENC_PVM_BITRATE_BASED
        g_objAudioSource.PreProcessPass = 1

      case 4 ' 2 pass bitrate-based peak constrained VBR
        g_objProfile.VBRMode(WMENC_AUDIO, 0) = WMENC_PVM_PEAK
        g_objAudioSource.PreProcessPass = 1

      case else
        OutputInfo "Invalid -a_mode <mode_number>"
        SetupCustomAudioProfile = false
        exit function
    end select
  end if


  ' Set audio codec
  if g_strAudioCodec <> "" then
    intAudioCodecIndex = GetAudioCodecIndex(g_strAudioCodec)
    intFourCC = g_objProfile.EnumAudioCodec(intAudioCodecIndex, g_strAudioCodecName)

    objAudience.AudioCodec(0) = intAudioCodecIndex
  end if

  'Parse the a_setting string to extract audio bitrate sample rate and stuff like that
  if g_strAudioSetting <> "" then
    intNextArgStart = instr(g_strAudioSetting, "_")
    strTemp = left(g_strAudioSetting, intNextArgStart-1)
  
    if ucase( left(strTemp, 1) ) = "Q" then
      g_intAudioVBRMode = 2
      g_objProfile.VBRMode(WMENC_AUDIO, 0) = WMENC_PVM_UNCONSTRAINED

      blnAudioQuality = true
      intAudioBitrate = ConvertStringToInteger( mid(strTemp, 2 ) )
    else
      intAudioBitrate = ConvertStringToInteger( strTemp )
    end if

    'OutputInfo "****Audio bit rate is " & intAudioBitrate

    intNextArgEnd = instr(intNextArgStart+1, g_strAudioSetting, "_")
    strTemp = mid(g_strAudioSetting, intNextArgStart+1, intNextArgEnd-intNextArgStart-1)
    intAudioSampleRate = ConvertStringToInteger( strTemp )
    'OutputInfo "****Audio sample rate is " & intAudioSampleRate
  
    intNextArgStart = intNextArgEnd
    intNextArgEnd = instr(intNextArgStart+1, g_strAudioSetting, "_")
    if intNextArgEnd > 0 then
      strTemp = mid(g_strAudioSetting, intNextArgStart+1, intNextArgEnd-intNextArgStart-1)
    else
      strTemp = mid(g_strAudioSetting, intNextArgStart+1)
    end if
    intAudioChannels = ConvertStringToInteger( strTemp )
    'OutputInfo "****Audio channels are " & intAudioChannels

    if intNextArgEnd > 0 then
      strTemp = mid(g_strAudioSetting, intNextArgEnd+1)
      intBitDepth = ConvertStringToInteger( strTemp )
    else
      intBitDepth = 16
    end if
    'OutputInfo "****Audio bit depth is " & intBitDepth

    if intAudioBitrate = -1 or intAudioSampleRate = -1 or intAudioChannels = -1 or intBitDepth = -1 then
      SetupCustomAudioProfile = false
      err.Raise &H80070057
      exit function
    end if

    'Convert bitrate and samplerate to actual values
    if blnAudioQuality = false then
      if intAudioBitrate = 1411 then
        intAudioBitrate = 1411200
      elseif intAudioBitrate = 705 then
        intAudioBitrate = 705600
      elseif intAudioBitrate = 353 then
        intAudioBitrate = 352800
      else
        intAudioBitrate = 1000 * intAudioBitrate
      end if
    end if

    ' For peak vbr mode set peakbitrate and peak buffer
    if g_objProfile.VBRMode(WMENC_AUDIO, 0) = WMENC_PVM_PEAK then
      if g_intAudioPeakBuffer <> -1 then
        objAudience.AudioBufferMax(0) = g_intAudioPeakBuffer
      end if

      if g_intAudioPeakBitrate = -1 then
        objAudience.AudioPeakBitrate(0) = 1.5 * intAudioBitrate
      else
        objAudience.AudioPeakBitrate(0) = g_intAudioPeakBitrate
      end if
      'OutputInfo "****Audio peak bit rate is " & objAudience.AudioPeakbitrate(0)
      'OutputInfo "****Audio peak buffer is " & objAudience.AudioBufferMax(0)
    end if

    if intAudioSampleRate = 88 then
      intAudioSampleRate = 88200
    elseif intAudioSampleRate = 44 then
      intAudioSampleRate = 44100
    elseif intAudioSampleRate = 22 then
      intAudioSampleRate = 22050
    elseif intAudioSampleRate = 11 then
      intAudioSampleRate = 11025
    else
      intAudioSampleRate = 1000 * intAudioSampleRate
    end if

    ' Validate values for bitrate, samplerate, channels and depth 
    if blnAudioQuality = false then
      blnValidBitrate = CheckForValidAudioBitRates( intAudioBitrate )
      if blnValidBitrate = false then 
        OutputInfo "Invalid audio bit rate " & intAudioBitrate
        SetupCustomAudioProfile = false
        exit function
      end if
    end if

    blnValidSampleRate = CheckForValidAudioSamplingRates( intAudioSampleRate )
    if blnValidSampleRate = false then 
      OutputInfo "Invalid audio sample rate " & intAudioSampleRate
      SetupCustomAudioProfile = false
      exit function
    end if

    blnValidChannels = CheckForValidAudioChannel( intAudioChannels )
    if blnValidChannels = false then 
      OutputInfo "Invalid audio channels " & intAudioChannels
      SetupCustomAudioProfile = false
      exit function
    end if

    blnValidBitDepth = CheckForValidDepth( intBitDepth )
    if blnValidBitDepth = false then 
      OutputInfo "Invalid bits per sample " & intBitDepth
      SetupCustomAudioProfile = false
      exit function
    end if

    objAudience.SetAudioConfig 0, intAudioChannels, intAudioSampleRate, intAudioBitrate, intBitDepth
  end if

  SetupCustomAudioProfile = true

end function

' Setup the video part of the custom profile
function SetupCustomVideoProfile(objAudience)
  on error resume next

  ' Setup audio
  dim intVideoCodecIndex, intFourCC

  'Set video vbr mode
  if g_intVideoVBRMode <> -1 then
    select case g_intVideoVBRMode
      case 0 ' 1 pass CBR

      case 1 ' 2 pass CBR
        g_objVideoSource.PreProcessPass = 1

      case 2 ' Quality VBR mode
        g_objProfile.VBRMode(WMENC_VIDEO, 0) = WMENC_PVM_UNCONSTRAINED

      case 3 ' 2 pass bitrate based VBR
        g_objProfile.VBRMode(WMENC_VIDEO, 0) = WMENC_PVM_BITRATE_BASED
        g_objVideoSource.PreProcessPass = 1

      case 4 ' 2 pass bitrate-based peak constrained VBR
        g_objProfile.VBRMode(WMENC_VIDEO, 0) = WMENC_PVM_PEAK     
        g_objVideoSource.PreProcessPass = 1

      case else
        OutputInfo "Invalid -v_mode <mode_number>"
        SetupCustomVideoProfile = false
        exit function

    end select
  end if

  ' Set video codec
  if g_strVideoCodec <> "" then
    intVideoCodecIndex = GetVideoCodecIndex(g_strVideoCodec)
    intFourCC = g_objProfile.EnumVideoCodec(intVideoCodecIndex, g_strVideoCodecName)

    objAudience.VideoCodec(0) = intVideoCodecIndex
  end if

  if g_intVideoBitrate <> -1 then
    objAudience.VideoBitrate(0) = g_intVideoBitrate
  end if

  if g_intVideoWidth <> -1 then
    objAudience.VideoWidth(0) = g_intVideoWidth
  end if

  if g_intVideoHeight <> -1 then
    objAudience.VideoHeight(0) = g_intVideoHeight
  end if

  if g_intVideoFramerate <> -1 then
    objAudience.VideoFPS(0) = g_intVideoFramerate
  end if

  if g_intVideoKeydist <> -1 then
    objAudience.VideoKeyFrameDistance(0) = g_intVideoKeydist
  end if

  if g_intVideoBuffer <> -1 then
    objAudience.VideoBufferSize(0) = g_intVideoBuffer
  end if

  ' For quality vbr mode use intSmoothness as VBR quality
  if g_objProfile.VBRMode(WMENC_VIDEO, 0) = WMENC_PVM_UNCONSTRAINED then
    if g_intVideoQuality <> -1 then
      objAudience.VideoCompressionQuality(0) = g_intVideoQuality
    end if
  else
    if g_intVideoQuality <> -1 then
      objAudience.VideoImageSharpness(0) = g_intVideoQuality
    end if
  end if

  ' For peak vbr mode set peakbitrate and peak buffer
  if g_objProfile.VBRMode(WMENC_VIDEO, 0) = WMENC_PVM_PEAK then
    if g_intVideoPeakBuffer <> -1 then
      objAudience.VideoBufferMax(0) = g_intVideoPeakBuffer
    end if

    if g_intVideoPeakBitrate <> -1 then
      if objAudience.VideoBitrate(0) > g_intVideoPeakBitrate then
        objAudience.VideoBitrate(0) = g_intVideoPeakBitrate
      end if
      objAudience.VideoPeakBitrate(0) = g_intVideoPeakBitrate
    end if
  end if

  ' Setup video device conformance
  if g_strVideoDevConf <> "" then
    objAudience.Property( WMENC_VIDEO, 0, "DeviceConformanceTarget") = g_strVideoDevConf
  end if

  SetupCustomVideoProfile = true
end function


' Load the default profile settings
function LoadProfileDefaults()
  dim argsArray

  ' Audio profile settings
  ' Set default codec and/or VBR mode when appropriate
  if g_strAudioSetting <> "" then
    argsArray = Split ( g_strAudioSetting, "_" )
    if UBound( argsArray ) = 3 and left(g_strAudioSetting, 4) <> "Q100" then
      g_strAudioCodec = WMA9PRO
    end if
  end if

  if g_intAudioVBRMode = -1 then
    g_intAudioVBRMode = 0
  end if

  if g_strAudioCodec = "" then
    g_strAudioCodec = "WMA9STD"
  end if

  if g_intAudioPeakBuffer = -1 then
    g_intAudioPeakBuffer = 3000
  end if

  ' Set default -a_setting based on format and mode
  if g_strAudioSetting = "" then
    if g_intAudioVBRMode = 2 and g_strAudioCodec = WMA9PRO then
      g_strAudioSetting = "Q75_44_2_24"
    elseif g_intAudioVBRMode = 2 and g_strAudioCodec = WMA9LSL then
      g_strAudioSetting = "Q100_44_2_16"
    elseif g_intAudioVBRMode = 2 then
      g_strAudioSetting = "Q75_44_2"
    elseif g_intAudioVBRMode = 0 and g_strAudioCodec = WMSPEECH then
      g_strAudioSetting = "12_16_1"
    elseif g_intAudioVBRMode = 0 and g_strAudioCodec = PCM then
      g_strAudioSetting = "705_22_2"
    elseif g_strAudioCodec = WMA9PRO then
      g_strAudioSetting = "128_44_2_24"
    else
      g_strAudioSetting = "64_44_2"
    end if
  end if

  ' Video profile settings
  if g_intVideoVBRMode = -1 then
    g_intVideoVBRMode = 0
  end if

  if g_strVideoCodec = "" then
    g_strVideoCodec = "WMV9"
  end if

  if g_intVideoBitrate = -1 then
    g_intVideoBitrate = 250000
  end if

  if g_intVideoWidth = -1 then
    g_intVideoWidth = 0
  end if

  if g_intVideoHeight = -1 then
    g_intVideoHeight = 0
  end if

  if g_intVideoKeydist = -1 then
    g_intVideoKeydist = 10000
  end if

  if g_intVideoBuffer = -1 then 
    g_intVideoBuffer = 5000
  end if

  if g_intVideoPeakBuffer = -1 then
    g_intVideoPeakBuffer = 5000
  end if


  if g_intVideoPeakBitrate = -1 then
    g_intVideoPeakBitrate = 1.5 * g_intVideoBitrate
  end if

  if g_intVideoQuality = -1 then
    if g_intVideoVBRMode = 2 then
      g_intVideoQuality = 95
    else
      g_intVideoQuality = 75
    end if
  end if
end function

' Setup time configuration
function SetupTime()
  if g_blnDevice then
    ' Duration must be specified when capture from a live device.
    if g_intDuration = -1 then
      OutputInfo "Duration must be specified when capturing from devices."
      SetupTime = false
      exit function
    end if
  end if

  ' Setup markin and markout
  if g_intMarkInTime <> -1 then
    
    ' Set start time for audio source (mark in) if it has a audio stream.
    if not IsNull(g_objAudioSource) then
      g_objAudioSource.MarkIn = g_intMarkInTime
    end if
  
    ' Set start time for video source (mark in) if it has a video stream.
    if not IsNull(g_objVideoSource) then
      g_objVideoSource.MarkIn = g_intMarkInTime
    end if
  end if

  ' Is a mark out time provided?
  if g_intMarkOutTime <> -1 then
    
    ' Set end time for audio source (mark out) if it has a audio stream.
    if not IsNull(g_objAudioSource) then
      g_objAudioSource.MarkOut = g_intMarkOutTime
    end if
    
    ' Set end time for video source (mark out) if it has a video stream.
    if not IsNull(g_objVideoSource) then
      g_objVideoSource.MarkOut = g_intMarkOutTime
    end if
  end if

  SetupTime = true
end function

' Transcode 
function Transcode()
  on error resume next
  
  ' Prepare to encode
  g_objEncoder.PrepareToEncode( true )

  ' Check if we got an error saying markout is greater that end of file. If so set markout to end of file
  if err.number = -1072882855 then
      err.Clear

    if not IsNull(g_objAudioSource) then
      g_objAudioSource.MarkOut = 0
    end if
    
    if not IsNull(g_objVideoSource) then
      g_objVideoSource.MarkOut = 0
    end if

      g_objEncoder.PrepareToEncode( true )
  elseif err.number <> 0 then
    Transcode = false
    OutputInfo "Prepare to encode failed with error " & err.Number & " " & err.Description
    exit function
  end if

  ' Start encoder
  g_intErrorCode = 0
  g_objEncoder.Start()
  if err.number <> 0 then
    Transcode = false
    OutputInfo "Start encoder failed with error " & err.Number & " " & err.Description
    exit function
  end if

  g_tStartTime = Now()
  
  ' Wait for start event
  while not g_blnEncoderStarted and g_intErrorCode = 0
    WScript.Sleep( 1000 )
  wend

  on error goto 0
  
  if g_intDuration >= 0 then
    ' Wait for time
    OutputInfo "Wait for " & g_intDuration & " seconds to stop encoder..."

    dim t1, t2
    
    t1 = Now()

    ' Wait until an error code is received
    do while g_intErrorCode = 0
      WScript.Sleep( 1000 )

      t2 = Now()

      if DateDiff( "s", t1, t2 ) >= g_intDuration then
        OutputInfo "Duration time is reached. Stop encoder..."
        g_objEncoder.Stop()
        exit do
      end if

      if g_objEncoder.RunState = WMENC_ENCODER_STOPPED then
        OutputInfo "Encoding completed before duration time is reached."
        exit do
      end if
    loop
  else
    ShowProgress
  end if

  while not g_blnEncoderStopped and g_intErrorCode = 0
    WScript.Sleep( 1000 )
  wend

  ' Check error event
  if g_intErrorCode = 0 then
    OutputInfo "======== Encoding Completed ========"
    Transcode = true
  else
    OutputInfo "Error occurred in transcoding: Error Code = 0x" & Hex( g_intErrorCode )
    Transcode = false
  end if

end function

' Show statistics information of output
function ShowStatistics()
  dim objProfile

  ' Output statistics for audio stream
  if not IsNull(g_objAudioSource) then
    dim objAudioStats
    
    set objAudioStats = g_objEncoder.Statistics.StreamOutputStats( WMENC_AUDIO, 0, 0 )

    OutputInfo ""
    OutputInfo "Audio :"
    OutputInfo vbTab & "Codec: " & g_strAudioCodecName
    if g_intAudioVBRMode <> 2 then
      OutputInfo vbTab & "Expected bit rate: " & vbTab & objAudioStats.ExpectedBitrate & " bps"
    end if
    OutputInfo vbTab & "Average bit rate: " & vbTab & objAudioStats.AverageBitrate & " bps"
    OutputInfo vbTab & "Expected sample rate: " & vbTab & objAudioStats.ExpectedSampleRate
    OutputInfo vbTab & "Average sample rate: " & vbTab & objAudioStats.AverageSampleRate & ""
    OutputInfo vbTab & "Dropped byte count: " & vbTab & objAudioStats.DroppedByteCount*10000 & " bytes"
    OutputInfo vbTab & "Dropped sample rate: " & vbTab & objAudioStats.DroppedSampleCount*10000 & ""
    OutputInfo vbTab & "Total bytes: " & vbTab & vbTab & objAudioStats.ByteCount*10000 & " bytes"
  end if

  if not IsNull(g_objVideoSource) then
    dim objVideoStats
    
    set objVideoStats = g_objEncoder.Statistics.StreamOutputStats( WMENC_VIDEO, 0, 0 )

    OutputInfo ""
    OutputInfo "Video :"
    OutputInfo vbTab & "Codec: " & g_strVideoCodecName
    if g_intVideoVBRMode <> 2 then
      OutputInfo vbTab & "Expected bit rate: " & vbTab & objVideoStats.ExpectedBitrate & " bps"
    end if
    OutputInfo vbTab & "Average bit rate: " & vbTab & objVideoStats.AverageBitrate & " bps"
    OutputInfo vbTab & "Expected fps: " & vbTab & vbTab & objVideoStats.ExpectedSampleRate / 1000
    OutputInfo vbTab & "Dropped frame count: " & vbTab & objVideoStats.DroppedSampleCount*10000 & ""
    OutputInfo vbTab & "Total coded frames: " & vbTab & objVideoStats.SampleCount*10000 & ""
    OutputInfo vbTab & "Average sample rate: " & vbTab & objVideoStats.AverageSampleRate / 1000 & ""
    OutputInfo vbTab & "Dropped bytes: " & vbTab & vbTab & objVideoStats.DroppedByteCount*10000 & " bytes"
    OutputInfo vbTab & "Total bytes: " & vbTab & vbTab & objVideoStats.ByteCount*10000 & " bytes"
  end if

  ' Output statistics information of output stream
  OutputInfo ""
  OutputInfo "Overall:"
  OutputInfo vbTab & "Encoding time: " & vbTab & vbTab & DateDiff( "s", g_tStartTime, g_tStopTime ) & " seconds"
  OutputInfo vbTab & "Average bit rate: " & vbTab & g_objEncoder.Statistics.WMFOutputStats.AverageBitrate & " bps"
  
  if g_strOutput <> "" then
    dim objFileStats
    
    set objFileStats = g_objEncoder.Statistics.FileArchiveStats
  
    OutputInfo vbTab & "File size: " & vbTab & vbTab & objFileStats.FileSize*10000 & " bytes "
    OutputInfo vbTab & "File duration: " & vbTab & vbTab & objFileStats.FileDuration*10 & " seconds "
  end if

end function

' Setup input configuration
function SetupInput()
  dim strDeviceName

  ' The input file name is used if it is not empty, otherwise device index is used.
  if g_strInput <> "" then
    dim strInput2
    
    strInput2 = g_objFileSystem.GetAbsolutePathName( g_strInput )

    on error resume next

    if g_strProfile <> "" then
      dim strProfileName

      strProfileName = LCase( g_strProfile )
      if Left( strProfileName, 1) = "a" and Mid( strProfileName, 2, 1 ) <> "v"  then
        g_blnAudioOnly = true
      elseif Left( strProfileName, 1) = "v" then
        g_blnVideoOnly = true
      end if
    end if

    ' Add this file into the source group.
    if g_blnAudioOnly then
      set g_objAudioSource = g_objSourceGroup.AddSource( WMENC_AUDIO )
      g_objAudioSource.SetInput( strInput2 )
    elseif g_blnVideoOnly then
      set g_objVideoSource = g_objSourceGroup.AddSource( WMENC_VIDEO )
      g_objVideoSource.SetInput( strInput2 )
    else
      g_objSourceGroup.AutoSetFileSource( strInput2 )

      ' If source file has an auido stream, get the audio source.
      if g_objSourceGroup.SourceCount( WMENC_AUDIO ) > 0 then
        set g_objAudioSource = g_objSourceGroup.Source( WMENC_AUDIO, 0 )
      end if

      ' If source file has a video stream, get the video source.
      if g_objSourceGroup.SourceCount( WMENC_VIDEO ) > 0 then
        set g_objVideoSource = g_objSourceGroup.Source( WMENC_VIDEO, 0 )
      end if

    end if
  elseif g_intAudioDevice <> -1 or g_intVideoDevice <> -1 then
    if g_intAudioDevice <> -1 then
      strDeviceName = GetDeviceNameFromIndex( WMENC_AUDIO, g_intAudioDevice )
      if strDeviceName = "" then
        OutputInfo "Error: Invalid audio device number specified" 
        SetupInput = false
        exit function
      end if
      set g_objAudioSource = g_objSourceGroup.AddSource( WMENC_AUDIO )
      g_objAudioSource.SetInput strDeviceName, "device"
    end if

    if g_intVideoDevice <> -1 then
      strDeviceName = GetDeviceNameFromIndex( WMENC_VIDEO, g_intVideoDevice )
      if strDeviceName = "" then
        OutputInfo "Error: Invalid video device number specified" 
        SetupInput = false
        exit function
      end if
      
      set g_objVideoSource = g_objSourceGroup.AddSource( WMENC_VIDEO )
      g_objVideoSource.SetInput strDeviceName, "device"
    end if
    
  end if

  if g_objSourceGroup.SourceCount( WMENC_AUDIO ) and g_objSourceGroup.SourceCount( WMENC_VIDEO ) then
    g_intSessionType = WMENC_CONTENT_ONE_AUDIO_ONE_VIDEO
  elseif g_objSourceGroup.SourceCount( WMENC_AUDIO ) then
    g_intSessionType = WMENC_CONTENT_ONE_AUDIO
  elseif g_objSourceGroup.SourceCount( WMENC_VIDEO ) then
    g_intSessionType = WMENC_CONTENT_ONE_VIDEO
  end if

  ' Set audio setting for speech
  if not IsNull(g_objAudioSource) then
    SetupAudioSource()
  end if

  ' Set video settings
  if not IsNull(g_objVideoSource) then
    SetupVideoSource()
  end if
   
  if err.number <> 0 then
    OutputInfo "Error: 0x" & Hex(err.number) & " - " & err.description
    err.Clear
    SetupInput = false
    exit function
  end if

  SetupInput = true
end function

'Set speech related settings on audio source
function SetupAudioSource()
  if g_intAudioSpeechContent <> -1 then
    g_objAudioSource.contentmode = g_intAudioSpeechContent
  end if
  if g_strAudioSpeechEdl <> "" then
    g_objAudioSource.contentedl = g_strAudioSpeechEdl
  end if
end function

'Set settings on video source
function SetupVideoSource()
  ' Set video preprocess
  select case g_intVideoPreprocess
    case 0 
      g_objVideoSource.Optimization = WMENC_VIDEO_STANDARD
    case 1
      g_objVideoSource.Optimization = WMENC_VIDEO_DEINTERLACE
    case 2
      g_objVideoSource.Optimization = WMENC_VIDEO_DEINTERLACE
      OutputInfo "Preprocessing mode not supported. Use -height and -width to adjust video size."
    case 3
      g_objVideoSource.Optimization = WMENC_VIDEO_DEINTERLACE
      OutputInfo "Preprocessing mode not supported. Use -height and -width to adjust video size."
    case 4
      g_objVideoSource.Optimization = WMENC_VIDEO_DEINTERLACE
      OutputInfo "Preprocessing mode not supported. Use -height and -width to adjust video size."
    case 5
      g_objVideoSource.Optimization = WMENC_VIDEO_INVERSETELECINE
    case 6
      g_objVideoSource.Optimization = WMENC_VIDEO_TELECINE_AA_TOP
    case 7
      g_objVideoSource.Optimization = WMENC_VIDEO_TELECINE_BB_TOP
    case 8
      g_objVideoSource.Optimization = WMENC_VIDEO_TELECINE_BC_TOP
    case 9
      g_objVideoSource.Optimization = WMENC_VIDEO_TELECINE_CD_TOP
    case 10
      g_objVideoSource.Optimization = WMENC_VIDEO_TELECINE_DD_TOP
    case 11
      g_objVideoSource.Optimization = WMENC_VIDEO_TELECINE_AA_BOTTOM
    case 12
      g_objVideoSource.Optimization = WMENC_VIDEO_TELECINE_BB_BOTTOM
    case 13
      g_objVideoSource.Optimization = WMENC_VIDEO_TELECINE_BC_BOTTOM
    case 14
      g_objVideoSource.Optimization = WMENC_VIDEO_TELECINE_CD_BOTTOM
    case 15
      g_objVideoSource.Optimization = WMENC_VIDEO_TELECINE_DD_BOTTOM
    case 16
      g_objVideoSource.Optimization = WMENC_VIDEO_INTERLACED_AUTO
    case 17
      g_objVideoSource.Optimization = WMENC_VIDEO_INTERLACED_TOP_FIRST
    case 18
      g_objVideoSource.Optimization = WMENC_VIDEO_INTERLACED_BOTTOM_FIRST
  end select

  'Set pixelformat
  select case ucase( g_strPixelFormat )
    case "I420"
      g_objVideoSource.PixelFormat = WMENC_PIXELFORMAT_I420
    case "IYUV"
      g_objVideoSource.PixelFormat = WMENC_PIXELFORMAT_IYUV
    case "RGB24"
      g_objVideoSource.PixelFormat = WMENC_PIXELFORMAT_RGB24
    case "RGB32"
      g_objVideoSource.PixelFormat = WMENC_PIXELFORMAT_RGB32 
    case "RGB555"
      g_objVideoSource.PixelFormat = WMENC_PIXELFORMAT_RGB555 
    case "RGB565"
      g_objVideoSource.PixelFormat = WMENC_PIXELFORMAT_RGB565
    case "RGB8"
      g_objVideoSource.PixelFormat = WMENC_PIXELFORMAT_RGB8
    case "UYVY"
      g_objVideoSource.PixelFormat = WMENC_PIXELFORMAT_UYVY
    case "YUY2"
      g_objVideoSource.PixelFormat = WMENC_PIXELFORMAT_YUY2
    case "YV12"
      g_objVideoSource.PixelFormat = WMENC_PIXELFORMAT_YV12 
    case "YVU9"
      g_objVideoSource.PixelFormat = WMENC_PIXELFORMAT_YVU9 
    case "YVYU"
      g_objVideoSource.PixelFormat = WMENC_PIXELFORMAT_YVYU 
  end select

  'Set pixel aspect ratio
  if g_intPixelAspectRatioX <> -1 then
    g_objVideoSource.PixelAspectRatioX = g_intPixelAspectRatioX
  end if

  if g_intPixelAspectRatioY <> -1 then
    g_objVideoSource.PixelAspectRatioY = g_intPixelAspectRatioY
  end if


  'Set performance
  if g_intVideoPerformance <> -1 then
    g_objEncoder.VideoComplexity = g_intVideoPerformance
  end if

  'Set clipping margins
  if g_intClipLeft <> -1 then
    g_objVideoSource.CroppingLeftMargin = g_intClipLeft
  end if

  if g_intClipRight <> -1 then
    g_objVideoSource.CroppingRightMargin = g_intClipRight
  end if

  if g_intClipTop <> -1 then
    g_objVideoSource.CroppingTopMargin = g_intClipTop
  end if

  if g_intClipBottom <> -1 then
    g_objVideoSource.CroppingBottomMargin = g_intClipBottom
  end if

end function

'This function is used to check if the bit rate specified for the a_setting param is valid
function CheckForValidAudioBitRates( intBitRate )

  select case intBitRate

    case 1411200,768000,705600,640000,440000,384000,352800,320000,256000,192000,160000,128000,96000,80000,_
       64000,48000,40000,32000,22000,20000,17000,16000,12000,10000,8000,6000,5000,4000,0
      CheckForValidAudioBitRates = True

    case else
      CheckForValidAudioBitRates = False

  end select
end function

'This function is used to check if the sampling rate specified for the a_setting param is valid
function CheckForValidAudioSamplingRates( intSamplingRate )

  select case intSamplingRate

    case 96000,88200,48000,44100,32000,22050,16000,11025,8000
      CheckForValidAudioSamplingRates = True

    case else
      CheckForValidAudioSamplingRates = False

  end select
end function

'This function is used to check if the channel specified for the a_setting param is valid
function CheckForValidAudioChannel( intChannel )

  select case intChannel

    case 1,2,6,8
      CheckForValidAudioChannel = True

    case else
      CheckForValidAudioChannel = False

   end select
end function

'This function is used to check if the depth specified for the a_setting param is valid
function CheckForValidDepth( intAudioDepth )

  select Case intAudioDepth

    case 16,20,24
      CheckForValidDepth = True 

    case else
      CheckForValidDepth = False

   end select 
end function

'This function is the top level function for directory based encoding
function DoDirectoryModeEncoding()
  dim strInputFull
  dim strOutputFull

  if g_strOutput = "" then
    OutputInfo "Error: No output directory specified" 
    DoDirectoryModeEncoding = false
    exit function
  end if

  strInputFull = g_objFileSystem.GetAbsolutePathName(g_strInput)
  strOutputFull = g_objFileSystem.GetAbsolutePathName(g_strOutput)

  EncodeFilesInFolder strInputFull, strOutputFull

end function

'This function will create all folders down to the strFolderName level
function CreateOutputFolder( strFolderName )
  if g_objFileSystem.FolderExists(strFolderName) = false then
    CreateOutputFolder( g_objFileSystem.GetParentFolderName( strFolderName ) )
    g_objFileSystem.CreateFolder( strFolderName )
        end if
end function

'This function will encode all files in the input directory and generate output files in output directory
function EncodeFilesInFolder( strInputFolderName, strOutputFolderName )
  dim objSubFoldersCollection
  dim objInputSubFolder
  dim strOutputSubFolderName, strFileType
  dim objFile, objFolder, objFileCollection

  OutputInfo "============== Encoding Files in Folder ============== " & strInputFolderName

  set objFolder = g_objFileSystem.GetFolder( strInputFolderName )
    
  ' First encode all the files contained in the input folder
  set objFileCollection = objFolder.Files

  for each objFile in objFileCollection

    strFileType = right(objFile.name, len(objFile.name) - instrrev(objFile.name,".") )  

    select case lcase(strFileType)
      case "avi", "wav", "mpg", "mpeg", "vob" , "wmv" , "wma", "mp3", "asf"
        g_strInput = strInputFolderName & "\" & objFile.name
        g_strOutput = strOutputFolderName & "\" & Left(objFile.name, len(objFile.Name)-(len(objFile.name) - instrrev(objFile.name,".") + 1)) & g_strOutputString

        OutputInfo "============== Input " & g_strInput & " " & g_strOutput

        ' This will force wmcmd to append wmv/wma extension
        g_strOutput = g_strOutput & "."
      
        ' Encode files
        if SetupEncoder() then
          if Transcode() and not g_blnSilent then 
            ShowStatistics()
          end if 
        end if

        ' reset encoder states for next file 
        g_objEncoder.PrepareToEncode( false )
        g_objEncoder.SourceGroupCollection.Remove( "SG_1" )
        g_objSourceGroup = Null
        g_objAudioSource = Null
        g_objVideoSource = Null
        g_blnEncoderStarted = false
        g_blnEncoderStopped = false
        g_blnEndPreProcess = false

      case else
        OutputInfo "Skipping: " & objFile.name
    end select
  next

    ' Then recurse into subfolders and encode all files there
    set objSubFoldersCollection = objFolder.SubFolders

    for each objInputSubFolder in objSubFoldersCollection
        strOutputSubFolderName = strOutputFolderName & "\" & objInputSubFolder.name

        EncodeFilesInFolder objInputSubFolder, strOutputSubFolderName
    Next

end function

' Get device name from index
function GetDeviceNameFromIndex(enumMediaType, intDeviceIndex)
  dim intLastDevice
  dim objSourceInfo, objSourcePluginManager

  intLastDevice = 0
  set objSourcePlugInManager = g_objEncoder.SourcePlugInInfoManager

  for i = 0 to objSourcePlugInManager.count -1
    set objSourceInfo = objSourcePlugInManager.Item(i)
    
    if objSourceInfo.Resources = true then
      if enumMediaType = WMENC_AUDIO then ' Audio
        ' For audio search audio and audio | video
        if objSourceInfo.MediaType = 1 or objSourceInfo.MediaType = 3 then
          if intDeviceIndex < intLastDevice + objSourceInfo.Count then
            GetDeviceNameFromIndex = objSourceInfo( intDeviceIndex - intLastDevice )
            exit function
          else 
            intLastDevice = intLastDevice + objSourceInfo.Count
          end if
        end if
      else ' Video
        ' For video search video, audio | video and video | script
        if objSourceInfo.MediaType = 2 or objSourceInfo.MediaType = 3 or objSourceInfo.MediaType = 6 then
          if intDeviceIndex < intLastDevice + objSourceInfo.Count Then
            GetDeviceNameFromIndex = objSourceInfo( intDeviceIndex - intLastDevice )
            exit function
          else 
            intLastDevice = intLastDevice + objSourceInfo.Count
          end if
        end if
      end if
    end if
  next

  GetDeviceNameFromIndex = ""

end function

function ShowProgress()
  dim intSleepDuration, intPercent, intADuration, intVDuration, intDuration
  dim intPasses, intLimit

  intPasses = 1

  ' Compute number of passes required
  if not IsNull(g_objAudioSource) then
    if g_objAudioSource.PreProcessPass = 1 then
      intPasses = 2
    end if
  end if

  if not IsNull(g_objVideoSource) then
    if g_objVideoSource.PreProcessPass = 1 then
      intPasses = 2
    end if
  end if

  ' Compute file duration
  if not IsNull(g_objAudioSource) then
    if g_objAudioSource.MarkOut <> 0 then
      intADuration = g_objAudioSource.MarkOut - g_objAudioSource.MarkIn
    else
      intADuration = g_objAudioSource.Duration - g_objAudioSource.MarkIn
    end if
  end if
  if not IsNull(g_objVideoSource) then
    if g_objVideoSource.MarkOut <> 0 then
      intVDuration = g_objVideoSource.MarkOut - g_objVideoSource.MarkIn
    else
      intVDuration = g_objVideoSource.Duration - g_objVideoSource.MarkIn
    end if
  end if

  if intADuration > intVDuration then
    intDuration = intADuration
  else
    intDuration = intVDuration
  end if

  intSleepDuration = 2000
  
  ' For all passes show progress from 0 to 100
  for j=0 to intPasses-1
    intPercent = 0

    if intPasses = 2 and j = 0 then
      OutputInfo "======== Begin Pass1 ========"
    elseif intPasses = 2 and j = 1 then
      OutputInfo "======== Begin Pass2 ========"
    end if
  
    for i=1 to 4
      intLimit = 25 * i
      do while g_objEncoder.RunState <> WMENC_ENCODER_STOPPED and g_intErrorCode = 0 and intPercent < intLimit
        WScript.stdout.Write "."
        WScript.Sleep intSleepDuration
        if intDuration <> 0 then
          intPercent = Int( g_objEncoder.Statistics.EncodingTime * 1000000 / intDuration )
        end if
        if intPasses = 2 and j = 0 and intLimit = 100 and intPercent < 75 then
          ' second pass has already started
          exit Do
        end if
      loop
    
      ' Only show progress if we know the duration of source files
      if intDuration <> 0 and g_intErrorCode = 0 then
        WScript.stdout.Write intLimit &"%."
      end if

      if g_intErrorCode <> 0 then 
        exit for
      end if
    next

    if j = 0 and intPasses = 2 then
      do while not g_blnEndPreProcess and g_intErrorCode = 0
        WScript.Sleep intSleepDuration
      loop
    end if

    if g_intErrorCode <> 0 then 
      exit for
    end if

    OutputInfo ""
  next

end function
