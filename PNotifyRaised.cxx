//==============================================================================
// File:        PNotifyRaised.cxx
//
// Copyright (c) 2017, Phil Harvey, Queen's University
//==============================================================================
#include "PNotifyRaised.h"
#include "PImageWindow.h"

PNotifyRaised::PNotifyRaised()
{
}

PNotifyRaised::~PNotifyRaised()
{
    if (PImageWindow::sNotifyRaised == this) {
        PImageWindow::sNotifyRaised = NULL;
    }
}

void PNotifyRaised::ArmNotifyRaised()
{
    PImageWindow::sNotifyRaised = this;
}

