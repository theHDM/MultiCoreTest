#pragma once
/*  
 *  This header generates hexBoard
 *  layouts based on menu selections.
 *  Lots of dependencies but they're
 *  all common to the main .ino file
 *  so rather than list them all, just
 *  make sure this header occurs at the
 *  end after other declarations.
 */

template <typename T1, typename T2>
void update_if_closer_to_zero(T1& LHS, const T2& RHS) {
  if (std::abs(RHS) < std::abs(LHS)) {LHS = RHS;}
}

void generate_and_apply_EDO_layout(double octave, int EDO, int A_span, int B_span) {
  int spanFifth = (EDO == 13 ? 7 : (EDO == 18 ? 10 : 
                  round(EDO * log2f(1.5f))));
  int spanFourth = EDO - spanFifth;
  int spanSharp  = 7 * spanFifth - 4 * EDO;
  int spanMajor2 = 2 * spanFifth - EDO;
  int spanMinor2 = spanMajor2 - spanSharp;
  int spanMinor3 = spanFourth - spanMajor2;
  int spanMinor7 = spanFifth - spanMinor3;
  for (auto& n : hexBoard.btn) {
    if (!n.isBtn) continue;
    if (!n.isNote) continue; // for now assuming cmd btns are unchanged
    n.scaleDegree = A_span * n.A_steps + B_span * n.B_steps; 
    n.midiPitch += n.scaleDegree * (octave / 100.0 / EDO);
    // scale by octave
    n.scaleEquave = 0;
    while (n.scaleDegree < 0) {
      --(n.scaleEquave);
      n.scaleDegree += EDO;
    }
    while (n.scaleDegree >= EDO) {
      ++(n.scaleEquave);
      n.scaleDegree -= EDO;
    }
    if (EDO < 10) { // algorithm doesn't really work below 10 EDO
      n.paletteNum = n.scaleDegree;
    } else {
      int stepsForPalette = n.scaleDegree;
      
           if (  n.scaleDegree >= spanMinor7) { stepsForPalette -= spanMinor7; }
      else if (  n.scaleDegree >= spanFifth)  { stepsForPalette -= spanFifth;  }
      else if (  n.scaleDegree >= spanFourth) { stepsForPalette -= spanFourth; }
      else if (  n.scaleDegree >= spanMinor3) { stepsForPalette -= spanMinor3; }

           if (stepsForPalette == 0)                      { n.paletteNum =  0; }
      else if (stepsForPalette == spanMajor2)             { n.paletteNum =  0; }
      else if (stepsForPalette == spanSharp)              { n.paletteNum =  1; }
      else if (stepsForPalette == spanMinor2)             { n.paletteNum =  1; }
      else if (stepsForPalette == spanMajor2 + spanSharp) { n.paletteNum = -1; }
      else if (stepsForPalette == spanMinor3 - spanSharp) { n.paletteNum = -1; }
      else { // for larger EDOs you will have microtonal tiers
        n.paletteNum = stepsForPalette;
        update_if_closer_to_zero(n.paletteNum, stepsForPalette - spanMinor3);
        update_if_closer_to_zero(n.paletteNum, stepsForPalette - spanMajor2);
        update_if_closer_to_zero(n.paletteNum, stepsForPalette - spanMinor2);
        update_if_closer_to_zero(n.paletteNum, stepsForPalette - spanSharp);
        n.paletteNum += (n.paletteNum > 0 ? 1 : -1);
      }
    }
    // anything else?
  }
  // that should be it for all the keys.
}


// recursive formula to generate MOS scale from large/small steps
// outputs a vector of booleans
// large step = TRUE/1   small step = FALSE/0
#include <vector>

uint gcd(const uint& _a, const uint& _b) {
  for (uint _f = _a - 1; _f > 1; --_f) {
    if (!((_a % _f) || (_b % _f))) {
      return _f;
    }
  }
  return 1;
}

std::vector<bool> generate_MOS_scale(uint8_t L, uint8_t S) {
  std::vector<bool> result;
  if ((L == 1) || (S == 1)) {
    for (uint8_t i = 0; i < L; ++i) {
      result.emplace_back(true);
    }
    for (uint8_t i = 0; i < S; ++i) {
      result.emplace_back(false);
    }
    return result;
  } else {
    int K = gcd(L,S);
    if (K > 1) {
      result = generate_MOS_scale(L/K, S/K);
      std::vector<bool> repeatKtimes;
      for (uint8_t i = 0; i < K; ++i) {
        for (uint8_t j = 0; j < result.size(); ++j) {
          repeatKtimes.emplace_back(result[j]);
        }
      }
      return repeatKtimes;
    } else {
      uint8_t Mn = (L < S ? L : S);
      uint8_t Mx = (L > S ? L : S);
      uint8_t z = Mx % Mn;
      uint8_t w = Mn - z;
      uint8_t v = Mx / Mn; // floor, integer division
      std::vector<bool> preScale = generate_MOS_scale(z, w);
      if (L < S) {
        std::reverse(preScale.begin(), preScale.end());
        for (uint8_t i = 0; i < preScale.size(); ++i) {
          result.emplace_back(true);
          for (uint8_t j = 0; j < v + preScale[i]; ++j) {
            result.emplace_back(false);
          }
        }
      } else {
        for (uint8_t i = 0; i < preScale.size(); ++i) {
          for (uint8_t j = 0; j < v + preScale[i]; ++j) {
            result.emplace_back(true);
          }
          result.emplace_back(false);
        }
      }
    }
  }
  return result;
}

std::vector<bool> generate_mode(const std::vector<bool>& baseScale, int mode) {
  uint8_t T = baseScale.size();
  // determine # of small steps directly from scale
  uint8_t S = T;
  for (const auto& d : baseScale) {
    S -= d;
  }
  // calculate size of bright generator
  uint8_t G = 0;
  for (uint8_t m = 1; m < T; ++m) {
    if ((S * m) % T == 1) {
      G = m;
      break;
    }
  }
  // left shift the scale by generator * mode#
  std::vector<bool> result;
  for (uint8_t i = 0; i < T; ++i) {
    result.emplace_back(baseScale[(i + (G * mode)) % T]);
  }
  return result;
}

void mode_shift(std::vector<bool>& RHS) {
  RHS = generate_mode(RHS, 1);
}

#include <map>
void generate_MOS_layout(int L, int S, int mode, 
                       std::map<Hex, int8_t>& paletteTier,
                       std::vector<bool>& layout ) {
  layout = generate_mode(generate_MOS_scale(L, S), mode);
  // using the Hex struct as a sortable pair
  // the X dimension = large steps, Y = small steps
  // start with the mode after the selected one
  // set notes in that mode to be the "black keys" (1)
  // cycle through modes until back to the selected
  // and that mode gets "white keys" (0)
  // doing it in this order prevents having to check
  // if the map entry is already full -- can naively
  // overwrite safely
  std::vector<bool> currMode = layout;
  uint8_t modeCount = (L + S) / gcd(L,S);
  Hex Zero(0,0);
  Hex Lg(1,0);
  Hex Sm(0,1);
  Hex iH;
  bool blackKey;
  for (uint8_t m = 0; m < modeCount; ++m) {
    mode_shift(currMode);
    iH = Zero;
    blackKey = (m != modeCount - 1);
    for (uint8_t i = 0; i < S + L; ++i) {
      paletteTier[iH] = blackKey;
      iH = iH + (currMode[i] ? Lg : Sm);
    }
  }
}

void apply_MOS_palette( Hex    A_span, // X = lg, Y = sm
                        Hex    B_span, // X = lg, Y = sm
                        Hex    Enharm,
                        const  std::map<Hex, int8_t>& tier) {
  // find the largest lg and sm step stored in the palette map
  // so you don't have to pass S and L by a value
  int S = 0;
  int L = 0;
  for (const auto& [ls, t] : tier) {
    L = (ls.x > L ? ls.x : L);
    S = (ls.y > S ? ls.y : S);
  }
  for (auto& n : hexBoard.btn) {
    if (!n.isBtn) continue;
    if (!n.isNote) continue; // for now assuming cmd btns are unchanged
    n.smallDegree = A_span.y * n.A_steps + B_span.y * n.B_steps;
    n.largeDegree = A_span.x * n.A_steps + B_span.x * n.B_steps;
    // scale by octave
    n.scaleEquave = 0;
    while ((n.largeDegree >= L) && (n.smallDegree >= S)) {
      ++(n.scaleEquave);
      n.largeDegree -= L;
      n.smallDegree -= S;
    }
    while ((n.largeDegree < 0) || (n.smallDegree < 0)) {
      --(n.scaleEquave);
      n.largeDegree += L;
      n.smallDegree += S;
    }
    // reduce to enharmonic equivalent, if you're in an EDO subset
    if (Enharm.x && Enharm.y) {
      while ((n.largeDegree > L) && (n.smallDegree <= S)) {
        n.largeDegree -= Enharm.x;
        n.smallDegree += Enharm.y;
      }
      while ((n.largeDegree <= L) && (n.smallDegree > S)) {
        n.largeDegree += Enharm.x;
        n.smallDegree -= Enharm.y;
      }
    }
    //
    auto pt = tier.find({n.largeDegree, n.smallDegree});  
    if (pt != tier.end()) {
      n.paletteNum = tier.at({n.largeDegree, n.smallDegree});
    } else {
      n.paletteNum = -1;
    }
  }
}

void apply_MOS_pitches(double period, int L, int S, double R) {
  for (auto& n : hexBoard.btn) {
    if (!n.isBtn) continue;
    if (!n.isNote) continue; // for now assuming cmd btns are unchanged
    n.midiPitch += period / 100.0 
        * (n.scaleEquave + (n.largeDegree * R + n.smallDegree) / (L + S));
  }
}

bool generate_layout(hexBoard_Setting_Array& refS) {
  Hex anchorHex(refS[_anchorX].i, refS[_anchorY].i);
  if (!hexBoard.in_bounds(anchorHex)) {
    return false; // 1) anchor hex must be valid -- in range and not a Cmd
  }
  Hex hexA = unitHex[refS[_axisA].i];
  Hex hexB = unitHex[refS[_axisB].i];
  if ((hexA == hexB) || (hexA == hexB * -1)) {
    return false; // 2) axes cannot be parallel
  }
  // express the root pitch as MIDI (note + cents/100)
  double anchorPitch = (double)refS[_anchorN].i + refS[_anchorC].d / 100.0;  
  double equaveCents = (refS[_equaveJI].b
      ? intervalToCents(refS[_equaveN].d / refS[_equaveD].d)
                      : refS[_equaveC].d);
  if (equaveCents <= 0.0) {
    return false; // 3) equave must be valid
  }
  for (auto& n : hexBoard.btn) {
    // cache the number of A and B steps for each button
    // relative to anchor hex
    Hex h = n.coord - anchorHex;
    if ((hexA == unitHex[dir_e]) || (hexA == unitHex[dir_w])) {
      n.B_steps  =  h.y                       / hexB.y;
      n.A_steps  = (h.x - n.B_steps * hexB.x) / hexA.x;
    } else if ((hexB == unitHex[dir_e]) || (hexB == unitHex[dir_w])) {
      n.A_steps  =  h.y                       / hexA.y;
      n.B_steps  = (h.x - n.A_steps * hexA.x) / hexB.x;
    } else {
      n.A_steps  =  h.x                           / hexA.x;
      n.B_steps  = (h.y - n.A_steps * hexA.y) / 2 / hexB.y;
      n.A_steps += (h.y - n.A_steps * hexA.y) / 2 / hexA.y;
    }
    // preload anchor pitch
    n.midiPitch = anchorPitch;
  }    
  // run the next steps based on what tuning system we're in
  switch (refS[_tuneSys].i) {
    case _tuneSys_normal: {
      generate_and_apply_EDO_layout(1200.0, 12, 
        refS[_eqStepA].i, refS[_eqStepB].i);
      break;
    }
    case _tuneSys_equal: {
      generate_and_apply_EDO_layout(equaveCents, refS[_eqDivs].i, 
        refS[_eqStepA].i, refS[_eqStepB].i);
      break;
    }
    case _tuneSys_lg_sm: {
      std::map<Hex, int8_t> palette;
      std::vector<bool> MOS;
      generate_MOS_layout(refS[_lgSteps].i, refS[_smSteps].i, 
          refS[_modeLgSm].i, palette, MOS);
      Hex A_axis(refS[_lgStepA].i, refS[_smStepA].i);
      Hex B_axis(refS[_lgStepB].i, refS[_smStepB].i);
      Hex Lg_Sm(refS[_lgToSmN].i, refS[_lgToSmD].i);
      Hex Discard(0,0);
      apply_MOS_palette(A_axis,B_axis,
                  refS[_lgToSmND].b ? Lg_Sm : Discard, palette);
      apply_MOS_pitches(equaveCents, refS[_lgSteps].i, refS[_smSteps].i,
        refS[_lgToSmND].b ? refS[_lgToSmN].i / (double)refS[_lgToSmD].i
                          : refS[_lgToSmR].d);
    }
    case _tuneSys_just: {
      float JIcentsA = intervalToCents(refS[_JInumA].d / refS[_JIdenA].d);
      float JIcentsB = intervalToCents(refS[_JInumB].d / refS[_JIdenB].d);
      for (auto& n : hexBoard.btn) {
        if (!n.isBtn) continue;
        if (!n.isNote) continue; // for now assuming cmd btns are unchanged
        n.midiPitch += JIcentsA * n.A_steps;
        n.midiPitch += JIcentsB * n.B_steps;
      }
    }
  }
  for (auto& n : hexBoard.btn) {
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