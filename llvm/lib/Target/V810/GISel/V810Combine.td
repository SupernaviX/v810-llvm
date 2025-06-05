include "llvm/Target/GlobalISel/Combine.td"

def V810PreLegalizerCombiner: GICombiner<
  "V810PreLegalizerCombinerImpl", [all_combines]> {
}

def V810O0PreLegalizerCombiner: GICombiner<
  "V810O0PreLegalizerCombinerImpl", [optnone_combines]> {
}

def V810PostLegalizerCombiner: GICombiner<
  "V810PostLegalizerCombinerImpl", [all_combines]> {
}