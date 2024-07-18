#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################################################################################
# Authors: Marcel Breyer                                                                                               #
# Copyright (C): 2024-today All Rights Reserved                                                                        #
# License: This file is released under the MIT license.                                                                #
#          See the LICENSE.md file in the project root for full license information.                                   #
########################################################################################################################

import HardwareSampling
import numpy as np

sampler = HardwareSampling.CpuHardwareSampler()
# could also be, e.g.,
# sampler = HardwareSampling.GpuNvidiaHardwareSampler()
sampler.start()

sampler.add_event("init")
A = np.random.rand(2**14, 2**14)
B = np.random.rand(2**14, 2**14)

sampler.add_event("matmul")
C = A @ B

sampler.stop()
sampler.dump_yaml("track.yaml")