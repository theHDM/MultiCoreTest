#pragma once
/*  
 *  This header generates hexBoard
 *  layouts based on menu selections.
 *  Lots of dependencies but they're
 *  all common to the main .ino file.
 */
bool generate_layout(hexBoard_Setting_Array& refS) {

  Hex anchorHex(refS[_anchorX].i, refS[_anchorY].i);
  if (!hexBoard.in_bounds(anchorHex)) {
    return false; // 1) anchor hex must be valid -- in range and not a Cmd
  }
  double anchorPitch = (double)refS[_anchorN].i + refS[_anchorC].d / 100.0;
  
  Hex hexA = unitHex[refS[_axisA].i];
  Hex hexB = unitHex[refS[_axisB].i];
  if ((hexA == hexB) || (hexA == hexB * -1)) {
    return false; // 2) axes cannot be parallel
  }
  
  if (refS[_tuneSys].i == _tuneSys_normal) {
    refS[_equaveC].d = 1200.0;
  } else if (refS[_equaveJI].b) {
    refS[_equaveC].d = intervalToCents(refS[_equaveN].d / refS[_equaveD].d);
  }
  if (refS[_equaveC].d <= 0.0) {
    return false; // 3) equave must be valid
  }

  int EDO = (refS[_tuneSys].i == _tuneSys_normal ? 12 : refS[_eqDivs].i);

  float smStepCents = refS[_equaveC].d;
  smStepCents /= refS[_lgSteps].i * refS[_lgToSm].d + refS[_smSteps].i;
  float lgStepCents = smStepCents * refS[_lgToSm].d;
  float JIcentsA = intervalToCents(refS[_JInumA].d / refS[_JIdenA].d);
  float JIcentsB = intervalToCents(refS[_JInumB].d / refS[_JIdenB].d);

  // generate GUI message  

  // assign all the pitches
  for (auto& n : hexBoard.btn) {
    if (!n.isBtn) continue;
    if (!n.isNote) continue; // for now assuming cmd btns are unchanged
    axial_Hex thisHex(n.coord - anchorHex, hexA, hexB);
    // assign scale degrees
    n.midiPitch = anchorPitch;
    switch (refS[_tuneSys].i) {
      case _tuneSys_normal: case _tuneSys_equal: {
        int EDOsteps = refS[_eqStepA].i * thisHex.a + refS[_eqStepB].i * thisHex.b;
        n.scaleEquave = 0;
        while (EDOsteps < 0) {
          --(n.scaleEquave);
          EDOsteps += EDO;
        }
        while (EDOsteps >= EDO) {
          ++(n.scaleEquave);
          EDOsteps -= EDO;
        }
        n.scaleDegree = EDOsteps;
        n.midiPitch += n.scaleEquave * (refS[_equaveC].d / 100.0);
        n.midiPitch += n.scaleDegree * (refS[_equaveC].d / 100.0 / EDO);
        break;
      }      
      case _tuneSys_lg_sm: {
        n.midiPitch += (smStepCents / 100.0)
          * (refS[_smStepA].i * thisHex.a + refS[_smStepB].i * thisHex.b);
        n.midiPitch += (lgStepCents / 100.0)
          * (refS[_lgStepA].i * thisHex.a + refS[_lgStepB].i * thisHex.b);
        break;
      }
      case _tuneSys_just: {
        n.midiPitch += (JIcentsA / 100) * thisHex.a;
        n.midiPitch += (JIcentsB / 100) * thisHex.b;
        break;
      }
      default:
        break;
    }
    n.frequency = MIDItoFreq(n.midiPitch);
    if (n.pixel > 60) n.LEDcodeBase = 0x010000;
    if (n.pixel < 50) n.LEDcodeBase = 0x000001;
  }

  // reflect transposition in frequencies

  // if MTS mode
  // enumerate all the calculated pitches
  // sort ascending, create MTS tuning table
  // assign MIDI note based on this

  // if not MTS mode
  // MIDI note is round midiPitch
  // calculate MPE pitch bend and store it

  // note coloring, this might take a while

  return true;
}