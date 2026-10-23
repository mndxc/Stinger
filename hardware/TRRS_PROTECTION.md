This is a great component question. Let me search for the specific datasheet to give you accurate guidance.Quick clarification before diving in: I'm reading "TPRCLAMP0524S" as **RClamp0524S** (Semtech RailClamp, 5V working voltage, 4-channel array in SOT-23-6). That's the part the search results match — let me know if you actually meant a different part.

## What you're working with

The RClamp0524S is a 4-channel unidirectional TVS array in SOT-23-6. Internally it's two independent two-channel circuits — each circuit has steering diodes that shunt transients up to VCC or down to GND, with a shared TVS clamp on the rail. It protects up to four data lines with low maximum capacitance of 5pF per line and a low operating voltage of 5.0V, which is fine for line-level audio swings (typically <1Vrms, well under the 5V standoff).

For a 4-conductor TRRS jack (Tip = L, Ring1 = R, Ring2 = GND, Ring3 = Mic — CTIA — or with GND/Mic swapped for OMTP), you have **3 signal lines** that need protection: L, R, and Mic. Sleeve/GND doesn't get clamped — it *is* the reference.

## Topology

Place the part **immediately adjacent to the jack**, on the connector side of any series resistors or AC-coupling caps. ESD energy must hit the clamp before it reaches the codec.

```
              RClamp0524S (SOT-23-6)
              
   Jack TIP  ──────●──────[series R / DC block]────► ES8388 LOUT/HPL
                   │
                   ├── pin (I/O 1)
                   
   Jack R1   ──────●──────[series R / DC block]────► ES8388 ROUT/HPR
                   │
                   ├── pin (I/O 2)
                   
   Jack R3   ──────●──────[bias network]──────────► ES8388 MICIN
   (Mic)           │
                   ├── pin (I/O 3)
                   
   Jack R2   ──────●──── AGND (chassis-tied near jack)
   (GND/Sleeve)    │
                   └── GND pin of RClamp
                   
                   VCC pin of RClamp ── 5V (or VBUS) ── 100nF ── GND
```

The fourth I/O channel goes unused — just leave that pin floating or tie it to GND per the datasheet's recommendation (check the specific RClamp0524S datasheet section on unused channels; some Semtech parts prefer GND, others tolerate float).

## Critical layout details for Stinger

A few things matter more than the schematic itself for ESD parts:

**Ground return.** The RClamp's GND pin needs the shortest, fattest path to the same ground node the jack sleeve uses. If the clamp dumps 8kV into a ground that has any meaningful inductance back to the codec's AGND, the codec sees the voltage difference. Tie the jack sleeve, the RClamp GND, and a stitching via to the AGND plane all within a few mm of each other.

**Place before any series element.** Series resistors (often 22Ω–100Ω on headphone outs) and DC-blocking caps belong *between* the TVS and the ES8388 — never between the jack and the TVS. The clamp must see the strike first.

**VCC pin.** The RClamp0524S is unidirectional, so it needs a rail to clamp to. Tie VCC to your 5V (or whatever rail is ≥ the maximum signal swing) with a local 100nF decoupling cap. If you're worried about the clamp injecting transient current into your 5V rail during a hit, some designers tie it through a small ferrite, but for a soundboard at low audio currents this is usually overkill.

**Mic bias consideration.** The mic line on a CTIA jack carries a DC bias (typically 2.0–2.7V from the codec). The RClamp0524S's 5V standoff is fine for this, but make sure your bias network (the resistor + cap from MICBIAS to the jack) sits *between* the TVS node and the codec, so the TVS sees the jack node directly.

## One thing worth flagging

5pF per line is low for digital interfaces but not zero for analog audio. On the headphone outputs at audio frequencies (20Hz–20kHz) into 32Ω loads, 5pF is completely inaudible. On the mic line, 5pF combined with a high-impedance bias resistor could form a very mild RC rolloff, but for typical 2.2kΩ bias resistors the corner is well above 14MHz — fine.

If you want a cleaner alternative specifically tuned for headset/mic jacks, Semtech also makes the **RClamp3304N** and similar parts marketed for audio jacks with even lower capacitance and built-in arrangements for the GND-noise-pumping problem that some headset configurations exhibit. Worth a glance before you commit, but the RClamp0524S will absolutely do the job.

Want me to sketch this out in KiCad-friendly form once you confirm the part number and which jack pinout (CTIA vs OMTP) you're targeting?