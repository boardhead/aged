//==============================================================================
// File:        messages.h
//
// Description: messages sent by Aged objects
//
// Created:     12/04/99 - P. Harvey
//
// Copyright (c) 2017, Phil Harvey, Queen's University
//==============================================================================
#ifndef __messages_h__
#define __messages_h__

enum {
    // null message
    kMessageNull = 0x0000,          // ignore this message
    
    // messages without data
    kMessageNewEvent = 0x0001,      // a new event was displayed
    kMessageEventCleared,           // event was cleared
    kMessageColoursChanged,         // the hit color map has changed
    kMessageOpticalChanged,         // the optical constants changed
    kMessageWorkProc,               // patch for work proc callback in ROOT version
    kMessageTriggerChanged,         // event trigger (continuous/single/none) changed
    kMessageDetectorChanged,        // the detector display changed
    kMessageHitSizeChanged,         // the displayed hit size was changed
    kMessageFitSizeChanged,         // the displayed fit size was changed
    kMessageSmoothTextChanged,      // text smoothing has changed
    kMessageSmoothLinesChanged,     // line smoothing has changed
    kMessageHitsChanged,            // the event hits changed
    kMessageFitChanged,             // a displayed fit has changed
    kMessageFitLinesChanged,        // the fit lines setting was changed
    kMessageEvIDFormatChanged,      // the EvID display format changed
    kMessageTimeFormatChanged,      // the format of the time display changed
    kMessageAngleFormatChanged,     // the angle display format changed
    kMessageLabelChanged,           // the image label changed
    kMessageNextTimeAvailable,      // the 'next' time is available for the viewed event
    kMessageShowLabelChanged,       // the state of 'show label' was changed
    kMessageWillWriteSettings,      // we are about to write our settings to file
    kMessageWriteSettingsDone,      // we are done writing our settings to file
    kMessageHitXYZChanged,          // the hit XYZ setting has changed
    kMessageCursorHit,              // the hit cursor has moved
    kMessageAddOverlay,             // add waveform overlay

    // messages from the global speaker (the resource manager)
    // (via PResourceManager::sSpeaker, not ImageData->mSpeaker)
    // - the difference is that all Aged main windows will hear these ones
    kMessageResourceColoursChanged = 0x1001,    // the colours changed
    kMessageTranslationCallback,        // data is (TranslationData *)
    kMessageNewMainDisplayEvent,        // data is (PmtEventRecord *)   

    // messages with data
    kMessage3dCursorMotion,             // data is (PProjImage *)
    kMessageHitDiscarded,               // data is (PProjImage *)

    kMessageHistScalesChanged,          // data is (PHistImage *)

    kLastMessageID  // not used, but saves trying to remember about commas on the last enum
};

#endif // __messages_h__
