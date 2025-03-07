#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#pragma once

//
// C++ Interface: CKeyboard
//
// Description: Adds features like international keyboard layout mapping on top of the
// platform specific low level keyboard classes.
// Here it must be done only once. Within the other mentioned classes it would have to be done several times.
//
// Keyboards alyways deliver printable characters, logical keys for functional behaviour, modifiers ... alongside
// Based on the same hardware with the same scancodes (also alongside) but delivered with different labels to customers
// the software must solve the mapping to the real labels. This is done here.
// The mapping must be specified by an xml configuration that should be able to access everything available,
// but this allows for double/redundant or ambiguous mapping definition, e.g.
// ASCII/unicode could be derived from scancodes, virtual keys, modifiers and/or other ASCII/unicode.
//
// Author: Team XBMC <>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#include "XBMC_events.h"
#include "system.h" // for DWORD
#include <map>

class CKeyboardStat
{
public:
  CKeyboardStat();
  ~CKeyboardStat();

  void Initialize();
  void Reset();
  void ResetState();
  void Update(XBMC_Event& event);
  bool GetShift() { return m_bShift;};
  bool GetCtrl() { return m_bCtrl;};
  bool GetAlt() { return m_bAlt;};
  bool GetRAlt() { return m_bRAlt;};
  char GetAscii();// { return m_cAscii;}; // FIXME should be replaced completly by GetUnicode()
  wchar_t GetUnicode();// { return m_wUnicode;};
  uint8_t GetVKey() { return m_VKey;};
  unsigned int KeyHeld() const;
  XBMCMod GetModState() const;
  void SetModState(const XBMCMod& modState);
  
  int HandleEvent(XBMC_Event& newEvent);

private:
  bool m_bShift;
  bool m_bCtrl;
  bool m_bAlt;
  bool m_bRAlt;
  char m_cAscii;
  wchar_t m_wUnicode;
  uint8_t m_VKey;

  XBMCKey m_lastKey;
  unsigned int m_lastKeyTime;
  unsigned int m_keyHoldTime;
  bool m_bEvdev;

  typedef std::pair<XBMCKey, XBMCMod> keyType;
  std::map<keyType, XBMCKey> m_translationMap;
};

extern CKeyboardStat g_Keyboard;

#endif
