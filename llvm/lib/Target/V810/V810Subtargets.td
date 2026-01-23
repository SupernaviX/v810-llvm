def IsNintendo : Predicate<"Subtarget->isNintendo()">,
                 AssemblerPredicate<(all_of FeatureNintendo)>;

def IsV810 : Predicate<"!Subtarget->isV830()">,
             AssemblerPredicate<(all_of (not FeatureV830))>;

def IsV830 : Predicate<"Subtarget->isV830()">,
             AssemblerPredicate<(all_of FeatureV830)>;

def IsV830orNintendo : Predicate<"Subtarget->isV830() || Subtarget->isNintendo()">,
             AssemblerPredicate<(any_of FeatureNintendo, FeatureV830)>;
             
             