# INTRODUCTION

This is the official repository of the resources related to the paper "A low-cost wireless multi-node vibration monitoring system for civil structures", by 
Renan Rocha Ribeiro, Rafael de Almeida Sobral, Ian Barreto Cavalcante, Luís Augusto Conte Mendes Veloso, and Rodrigo de Melo Lameiras, from University of Brasília, published in the journal Structural Control and Health MOnitoring (DOI to come).

This work refers to the proposal and investigation of a minimal and simple functional system, based on Arduino and ready-to-use components found in any electronics supply store,
modal identification of civil structures (e.g., slabs, beams, bridges, buildings, etc) in an Operation Modal Analysis framework.

The main motivation of this work was to answer: what performance can we expect from a low-cost SHM system for vibration monitoring?

While this answer depends on to what extent the interested reader is willing to dive into advanced electronic topics, we believe a starting point could be assuming that
a good baseline would be a system developed with ready-to-use components normally found in any physical or online electronics supply store. While this theme has been explored 
in other works already, no clear understanding of the baseline performance of a low cost system, implemented as a simplest system as possible 
(e.g., a hobbyist level, so no specialized skills and knowledge in fabrication of sensors and electronics), could be found in the literature. 
Attempt was made to provide as much information as possible on the developments of the system (e.g., source code and schematics) and performance of the system (e.g., providing raw acceleration measurement
files, presenting power spectral densities, comparing the proposed system to a professional system in the same condition) so more optimized systems can be developed upon 
and compared against what it is consider as a baseline performance of a monitoring system built with simple and cheap components.

# WHAT THIS REPOSITORY CONTAINS

This repository contains three folders:
* Experimental data: folder containing acceration time series obtained from the proposed low cost system and the professional system used as benchmark:
  * LowCostSystem: folder containing data from the proposed system
    * InterpolatedData: folder containing the data from each measurement node interpolatedm as described in the paper
    * RawData: folder containing the raw data from each measurement node
    * interpolationGenerator_20221216.m: MATLAB code used to generate the interpolated data, as described in the paper
  * ProfessionalSystem: folder containing data from the professional system
* SamplingFrequencyStability: folder containing the data used in the sampling frequency stability analysis and the code used to generate the results shown in the paper
* Source codes & schematics:
  * Arduino_Required_libraries: folder containing the libraries used in the Arduino source code. Check [arduino.cc](https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries) to see how to install libraries to your Arduino IDE.
  * Version_A-isolatedNodes: folder with information about Version A, as described in the paper
    *  Schematics: a Fritzing (.fzz) schematics of the system
    *  Source codes: a Arduino source code (.ino) file of the system
  * Version_B-cabledNodes: folder with information about Version B, as described in the paper
    *  Schematics: a Fritzing (.fzz) schematics of the system
    *  Source codes: a Arduino source code (.ino) file of the systemm, for each type of node
  * Version_C-wirelessNodes: folder with information about Version C, as described in the paper
    *  Schematics: a Fritzing (.fzz) schematics of the system
    *  Source codes: a Arduino source code (.ino) file of the systemm, for each type of node 
