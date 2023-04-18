#!/usr/bin/env python
import io
import sys, os
sys.path.append(os.getcwd())

import pandas as pd
import numpy as np

import fit_factory
        
# Skeleton fit parameters
# param_1 = 1
# param_2 = 1,
# param_3 = 10

class SkelFit:
    def __init__(self, param_1, param_2, param_3):
        self.param_1 = param_1
        self.param_2 = param_2
        self.param_3 = param_3        

    # implementation of the algorithm, the argument are mandatory even if not used
    def start(self, x, y, xmin, xmax, axis, fit_results):
        print("Skeleton example. Implementation is all yours.")
        
class SkelFitBuilder:
    def __init__(self):
        self._instance = None

    def __call__(self, param_1 = 1, param_2 = 2, param_3 = 10, **_ignored):
        if not self._instance:
            self._instance = SkelFit(param_1, param_2, param_3)
        return self._instance
