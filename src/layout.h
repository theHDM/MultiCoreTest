#pragma once
/*  
 *  This header generates hexBoard
 *  layouts based on menu selections.
 *  Lots of dependencies but they're
 *  all common to the main .ino file.
 */

void generate_MOS_scale(bool* pattern, size_t atN, int L, int S) {
  if ((L == 1) || (S == 1)) {
    for (int i = 0; i < L; ++i) {
      *(pattern + atN + i) = true;
    }
    for (int i = 0; i < S; ++i) {
      *(pattern + atN + L + i) = false;
    }
  } else {
    int K = 1;
    for (int i = 2; i < L; ++i) {
      if ((L == (L / i) * i) && (S == (S / i) * i)) {
        K = i;
      }
    }
    if (K > 1) {
      for (int i = 0; i < K; ++i) {
        generate_MOS_scale(pattern, atN + i * (L/K + S/K), L/K, S/K);
      }
    } else {
      int Mn = (L < S ? L : S);
      int Mx = (L > S ? L : S);
      int z = Mx % Mn;
      int w = Mn - z;
      bool preScale[Mn];
      generate_MOS_scale(preScale, 0, z, w);
      if (L > S) {
        for (int i = 0; i < Mn; ++i) {
          preScale[i] != preScale[i];
        }
      }
      int u = std::ceil((double)Mx / (double)Mn);
      int v = Mx / Mn;
      int n = atN;
      for (int i = 0; i < Mn; ++i) {
        for (int j = 0; j < (L > S ? 1 : (preScale[i] ? u : v)); ++j) {
          *(pattern + (n++)) = true;
        }
        for (int j = 0; j < (L < S ? 1 : (preScale[i] ? u : v)); ++j) {
          *(pattern + (n++)) = false;
        }
      }
    }
  }
}

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

  // tuning system specific calculations
  switch (refS[_tuneSys].i) {
    case _tuneSys_normal: 
    case _tuneSys_equal: {
      int EDO = (refS[_tuneSys].i == _tuneSys_normal ? 12 : refS[_eqDivs].i);
      int spanFifth = (EDO == 13 ? 7 : (EDO == 18 ? 10 : 
                      round(EDO * log2f(1.5f))));
      int spanFourth = EDO - spanFifth;
      int spanSharp  = 7 * spanFifth - 4 * EDO;
      int spanMajor2 = 2 * spanFifth - EDO;
      int spanMinor2 = spanMajor2 - spanSharp;
      int spanMinor3 = spanFourth - spanMajor2;
      int spanMinor7 = spanFifth - spanMinor3;
      break;
    }
    case _tuneSys_lg_sm: {
      float smStepCents = refS[_equaveC].d;
      smStepCents /= refS[_lgSteps].i * refS[_lgToSm].d + refS[_smSteps].i;
      float lgStepCents = smStepCents * refS[_lgToSm].d;
      bool MOS_pattern[refS[_lgSteps].i + refS[_smSteps].i];
      generate_MOS_scale(MOS_pattern, 0, refS[_lgSteps].i, refS[_smSteps].i);
    }
    case _tuneSys_just: {
      float JIcentsA = intervalToCents(refS[_JInumA].d / refS[_JIdenA].d);
      float JIcentsB = intervalToCents(refS[_JInumB].d / refS[_JIdenB].d);
    }
  }

  // TO-DO: generate GUI message

  // assign pitches and palettes
  for (auto& n : hexBoard.btn) {
    if (!n.isBtn) continue;
    if (!n.isNote) continue; // for now assuming cmd btns are unchanged
    
    axial_Hex thisHex(n.coord - anchorHex, hexA, hexB);
    n.midiPitch = anchorPitch;
    switch (refS[_tuneSys].i) {
      case _tuneSys_normal:
      case _tuneSys_equal: {
        int EDOsteps = refS[_eqStepA].i * thisHex.a 
                     + refS[_eqStepB].i * thisHex.b;
        // used for scale-lock and animate-by-octave
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
        // defines pitch for playback
        n.midiPitch += n.scaleEquave * (refS[_equaveC].d / 100.0);
        n.midiPitch += n.scaleDegree * (refS[_equaveC].d / 100.0 / EDO);
        // TO-DO modify EDO based on mode
        // determine palette color based on sharp/flat layout
        if (EDO < 10) { // algorithm doesn't really work below 10 EDO
          n.paletteNum = EDOsteps;
        } else {
          int stepsForPalette = EDOsteps;
               if (EDOsteps >= spanMinor7) { stepsForPalette -= spanMinor7; }
          else if (EDOsteps >= spanFifth)  { stepsForPalette -= spanFifth;  }
          else if (EDOsteps >= spanFourth) { stepsForPalette -= spanFourth; }
          else if (EDOsteps >= spanMinor3) { stepsForPalette -= spanMinor3; }
          switch (stepsForPalette) { // palette repeats the pattern from D to F
            case 0:                      { n.paletteNum =  0; break; }  // D
            case spanMajor2:             { n.paletteNum =  0; break; }  // E
            case spanSharp:              { n.paletteNum =  1; break; }  // D#
            case spanMinor2:             { n.paletteNum =  1; break; }  // Eb
            case spanMajor2 + spanSharp: { n.paletteNum = -1; break; }  // E#
            case spanMinor3 - spanSharp: { n.paletteNum = -1; break; }  // Fb
            default: { // for larger EDOs you will have microtonal tiers
              n.paletteNum = stepsForPalette;
              int microTonesMinor3 = stepsForPalette - spanMinor3;
              if (std::abs(microTonesMinor3) < std::abs(n.paletteNum)) {
                n.paletteNum = microTonesMinor3;
              }
              int microTonesMajor2 = stepsForPalette - spanMajor2;
              if (std::abs(microTonesMajor2) < std::abs(n.paletteNum)) {
                n.paletteNum = microTonesMajor2;
              }
              int microTonesMinor2 = stepsForPalette - spanMinor2;
              if (std::abs(microTonesMinor2)) < std::abs(n.paletteNum)) {
                n.paletteNum = microTonesMinor2;
              }
              int microTonesSharp1 = stepsForPalette - spanSharp;
              if (std::abs(microTonesSharp1) < std::abs(n.paletteNum)) {
                n.paletteNum = microTonesSharp1;
              }
              n.paletteNum += (n.paletteNum > 0 ? 1 : -1);
              break;
            }
          }
        }
        break;
      }
      
      case _tuneSys_lg_sm: {
        // defines pitch for playback
        n.midiPitch += (smStepCents / 100.0)
          * (refS[_smStepA].i * thisHex.a + refS[_smStepB].i * thisHex.b);
        n.midiPitch += (lgStepCents / 100.0)
          * (refS[_lgStepA].i * thisHex.a + refS[_lgStepB].i * thisHex.b);

        break;
      }
      case _tuneSys_just: {
        n.midiPitch += (JIcentsA / 100) * thisHex.a;
        n.midiPitch += (JIcentsB / 100) * thisHex.b;
        n.scaleDegree = 0;
        n.scaleEquave = 0;
        n.paletteNum = 0;
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